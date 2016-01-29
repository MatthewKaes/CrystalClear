#include "Linker.h"
#include <windows.h>

#include "Native.h"
#include "Function.h"
#include "Filesystems.h"
#include "Helper.h"
#include "Garbage_Collector.h"
#include "Obscure.h"
#include "Core.h"

extern const char* CRY_ROOT;

#define CRYSTAL_HEADER "MK15CC"
#define CRYSTAL_PADDING 0xCFCFCFCF

#define WRITE_HEADER(string) fwrite(string, 1, strlen(string), output);
#define WRITE_BLOCK(object) fwrite(&object, sizeof(object), 1, output);
#define WRITE_BLOCKS(block, length) fwrite(block, sizeof(*block), length, output);
#define WRITE_STRING(string) { \
    unsigned len = strlen(string); \
    WRITE_BLOCK(len); \
    fwrite(string, 1, len, output); \
  }
#define WRITE_VALUE(object) { \
    unsigned obj = object; \
    WRITE_BLOCK(obj); \
  }

#define READ_HEADER(string) { \
    char val[sizeof(CRYSTAL_HEADER)] = {0}; \
    fread(val, 1, strlen(CRYSTAL_HEADER), input); \
    string.assign(val); \
  }
#define READ_BLOCK(object) fread(&object, sizeof(object), 1, input);
#define READ_BLOCKS(block, length) fread(block, sizeof(*block), length, input);
#define READ_STRING(string) { \
    unsigned len; \
    READ_BLOCK(len); \
    string.resize(len, '\0'); \
    fread(const_cast<char*>(string.data()), 1, len, input); \
  }

Crystal_Linker::Crystal_Linker()
{
  executable = NULL;
  read_only_memory = NULL;
  code_size = 0;
  entry_point = 0;
}

Crystal_Linker::~Crystal_Linker()
{
  if(read_only_memory)
  {
    VirtualFreeEx(GetCurrentProcess(), read_only_memory, code_size, MEM_RELEASE);
  }
}

void Crystal_Linker::Set_Doubles(const std::unordered_map<double, std::vector<unsigned>>* values)
{
  doubles = *values;
}

void Crystal_Linker::Set_Strings(const std::unordered_map<std::string, std::vector<unsigned>>* values)
{
  strings = *values;
}

void Crystal_Linker::Set_Functions(const std::unordered_map<std::string, PackageLinks>* values)
{
  // Copy over all the locations that need to be fixed up by the
  // linker for user defined functions.
  for(auto iter = values->begin(); iter != values->end(); iter++)
  {
    unsigned offset = iter->second.package_offset;

    // Get the entry point.
    if (!iter->first.compare("main"))
      entry_point = offset;

    // Grab all of the offset pairs.
    functions[offset] = iter->second.links;
  }

  // Save the raw functions that were passed in.
  raw_functions = *values;
}

void Crystal_Linker::Set_Internal(const std::unordered_map<std::string, std::vector<unsigned>>* values)
{
  // Linking the Crystal Built in Library
  for (auto itr = Crystal_Linker::library.built_in.begin(); itr != Crystal_Linker::library.built_in.end(); itr++)
  {
    library.supported_functions[itr->second.name] = itr->second.function;
  }

  // Copy over all the locations that need to be fixed up by the
  // linker for user defined functions.
  for (auto iter = values->begin(); iter != values->end(); iter++)
  {
    if (library.supported_functions.find(iter->first) == library.supported_functions.end())
    {
      printf("Failed to link function '%s'.\n", iter->first.c_str());
      continue;
    }

    // Grab all of the offset pairs.
    for (unsigned i = 0; i < iter->second.size(); i++)
      internals[reinterpret_cast<unsigned>(library.supported_functions[iter->first])].push_back(iter->second[i]);
  }

  // Save the raw internals that were passed in.
  raw_internals = *values;
}

BYTE* Crystal_Linker::Link(BYTE* code, unsigned code_length)
{
  unsigned double_index = 0;
  unsigned string_index = double_index + doubles.size() * sizeof(double);
  unsigned index;

  executable = code;

  // Size of strings
  code_size = string_index;
  for(auto iter = strings.begin(); iter != strings.end(); iter++)
  {
    code_size += iter->first.size() + 1;
  }

  // Allocate the memory
  read_only_memory = (BYTE*)VirtualAllocEx( GetCurrentProcess(), 0, code_size, MEM_COMMIT | MEM_RESERVE , PAGE_EXECUTE_READWRITE);

  // Write and link all the doubles
  index = double_index;
  for(auto iter = doubles.begin(); iter != doubles.end(); iter++)
  {
    (double&)read_only_memory[index] = iter->first;
    
    for(unsigned j = 0; j < iter->second.size(); j++)
    {
      (int&)code[iter->second[j]] = (int)read_only_memory + index;
    }
    index += sizeof(double);
  }
  
  // Write and link all the strings
  index = string_index;
  for(auto iter = strings.begin(); iter != strings.end(); iter++)
  {
    strcpy((char *)read_only_memory + index, iter->first.c_str());
    read_only_memory[index + iter->first.size()] = '\0';

    for(unsigned j = 0; j < iter->second.size(); j++)
    {
      (int&)code[iter->second[j]] = (int)read_only_memory + index;
    }
    index += iter->first.size() + 1;
  }
  
  for(auto iter = functions.begin(); iter != functions.end(); iter++)
  {    
    for(unsigned j = 0; j < iter->second.size(); j++)
    {
      (int&)code[iter->second[j]] = (int)code + iter->first;
    }
  }
  
  for(auto iter = internals.begin(); iter != internals.end(); iter++)
  {    
    for(unsigned j = 0; j < iter->second.size(); j++)
    {
      (int&)code[iter->second[j]] = iter->first;
    }
  }
    
  return code;
}

BYTE* Crystal_Linker::Entry()
{
  return executable + entry_point;
}

void Crystal_Linker::Write(std::string output, BYTE* code, unsigned code_length)
{
  FILE* exe = fopen(output.c_str(), "wb");
  Write_Header(exe);
  Write_Internals(exe);
  Write_Doubles(exe);
  Write_Strings(exe);
  Write_Functions(exe);
  Write_Code(exe, code, code_length);
  fclose(exe);
}

void Crystal_Linker::Write_Header(FILE* output)
{
  WRITE_HEADER(CRYSTAL_HEADER);
  WRITE_STRING(CRY_VERSION);
  WRITE_VALUE(CRYSTAL_PADDING);
}

void Crystal_Linker::Write_Internals(FILE* output)
{
  WRITE_STRING(".internals");
  WRITE_VALUE(internals.size());

  for (auto iter = raw_internals.begin(); iter != raw_internals.end(); iter++)
  {
    WRITE_STRING(iter->first.c_str());
    WRITE_VALUE(iter->second.size());
    for (unsigned j = 0; j < iter->second.size(); j++)
    {
      WRITE_BLOCK(iter->second[j]);
    }
  }
}

void Crystal_Linker::Write_Doubles(FILE* output)
{
  WRITE_STRING(".numbers");
  WRITE_VALUE(doubles.size());

  for (auto iter = doubles.begin(); iter != doubles.end(); iter++)
  {
    WRITE_BLOCK(iter->first);
    WRITE_VALUE(iter->second.size());
    for (unsigned j = 0; j < iter->second.size(); j++)
    {
      WRITE_BLOCK(iter->second[j]);
    }
  }
}

void Crystal_Linker::Write_Strings(FILE* output)
{
  WRITE_STRING(".strings");
  WRITE_VALUE(strings.size());

  for (auto iter = strings.begin(); iter != strings.end(); iter++)
  {
    WRITE_STRING(iter->first.c_str());
    WRITE_VALUE(iter->second.size());
    for (unsigned j = 0; j < iter->second.size(); j++)
    {
      WRITE_BLOCK(iter->second[j]);
    }
  }
}

void Crystal_Linker::Write_Functions(FILE* output)
{
  WRITE_STRING(".functions");
  WRITE_VALUE(functions.size());

  for (auto iter = raw_functions.begin(); iter != raw_functions.end(); iter++)
  {
    WRITE_STRING(iter->first.c_str());
    WRITE_VALUE(iter->second.package_offset);
    WRITE_VALUE(iter->second.links.size());
    for (unsigned j = 0; j < iter->second.links.size(); j++)
    {
      WRITE_BLOCK(iter->second.links[j]);
    }
  }
}

void Crystal_Linker::Write_Code(FILE* output, BYTE* code, unsigned code_length)
{
  WRITE_STRING(".code");
  WRITE_VALUE(code_length);
  WRITE_BLOCKS(code, code_length);
}

void Crystal_Linker::Read(std::string input)
{
  FILE* exe = fopen(input.c_str(), "rb");
  Read_Header(exe);
  Read_Internals(exe);
  Read_Doubles(exe);
  Read_Strings(exe);
  Read_Functions(exe);
  Read_Code(exe);
  fclose(exe);
}

void Crystal_Linker::Read_Header(FILE* input)
{
  std::string str;
  unsigned padding;
  READ_HEADER(str);
  READ_STRING(str);
  READ_BLOCK(padding);
}

void Crystal_Linker::Read_Internals(FILE* input)
{
  std::string str;
  std::unordered_map<std::string, std::vector<unsigned>> values;
  unsigned block_count;
  READ_STRING(str);
  READ_BLOCK(block_count);

  for (unsigned i = 0; i < block_count; i++)
  {
    unsigned lookups;
    READ_STRING(str);
    READ_BLOCK(lookups);
    for (unsigned j = 0; j < lookups; j++)
    {
      unsigned value;
      READ_BLOCK(value);
      values[str].push_back(value);
    }
  }

  Set_Internal(&values);
}

void Crystal_Linker::Read_Doubles(FILE* input)
{
  std::string str;
  std::unordered_map<double, std::vector<unsigned>> values;
  unsigned block_count;
  READ_STRING(str);
  READ_BLOCK(block_count);

  for (unsigned i = 0; i < block_count; i++)
  {
    unsigned lookups;
    double block;
    READ_BLOCK(block);
    READ_BLOCK(lookups);
    for (unsigned j = 0; j < lookups; j++)
    {
      unsigned value;
      READ_BLOCK(value);
      values[block].push_back(value);
    }
  }

  Set_Doubles(&values);
}

void Crystal_Linker::Read_Strings(FILE* input)
{
  std::string str;
  std::unordered_map<std::string, std::vector<unsigned>> values;
  unsigned block_count;
  READ_STRING(str);
  READ_BLOCK(block_count);

  for (unsigned i = 0; i < block_count; i++)
  {
    unsigned lookups;
    READ_STRING(str);
    READ_BLOCK(lookups);
    for (unsigned j = 0; j < lookups; j++)
    {
      unsigned value;
      READ_BLOCK(value);
      values[str].push_back(value);
    }
  }

  Set_Strings(&values);
}

void Crystal_Linker::Read_Functions(FILE* input)
{
  std::string str;
  std::unordered_map<std::string, PackageLinks> values;
  unsigned block_count;
  unsigned link_count;
  READ_STRING(str);
  READ_BLOCK(block_count);

  for (unsigned i = 0; i < block_count; i++)
  {
    READ_STRING(str);
    READ_BLOCK(values[str].package_offset);
    READ_BLOCK(link_count);
    for (unsigned j = 0; j < link_count; j++)
    {
      unsigned value;
      READ_BLOCK(value);
      values[str].links.push_back(value);
    }
  }

  Set_Functions(&values);
}


void Crystal_Linker::Read_Code(FILE* input)
{
  std::string str;
  BYTE* code = (byte*)VirtualAllocEx(GetCurrentProcess(), 0, 1 << 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  unsigned code_lenght;

  READ_STRING(str);
  READ_BLOCK(code_lenght);
  READ_BLOCKS(code, code_lenght);

  Link(code, code_lenght);
}
