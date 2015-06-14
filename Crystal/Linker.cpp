#include "Linker.h"
#include <windows.h>

Crystal_Linker::Crystal_Linker()
{
  read_only_memory = NULL;
  code_size = 0;
}

Crystal_Linker::~Crystal_Linker()
{
  if(read_only_memory)
  {
    VirtualFreeEx(GetCurrentProcess(), read_only_memory, code_size, MEM_RELEASE);
  }
}

void Crystal_Linker::Add_Double(double value, unsigned offset)
{
  doubles[value].push_back(offset);
}

void Crystal_Linker::Set_Doubles(const std::unordered_map<double, std::vector<unsigned>>* values)
{
  doubles = *values;
}

void Crystal_Linker::Add_String(const char* str, unsigned offset)
{
  strings[str].push_back(offset);
}

void Crystal_Linker::Set_Strings(const std::unordered_map<std::string, std::vector<unsigned>>* values)
{
  strings = *values;
}

void Crystal_Linker::Add_Function(unsigned func, unsigned offset)
{
  functions[func].push_back(offset);
}

void Crystal_Linker::Set_Functions(const std::unordered_map<std::string, PackageLinks>* values)
{
  // Copy over all the locations that need to be fixed up by the
  // linker for user defined functions.
  for(auto iter = values->begin(); iter != values->end(); iter++)
  {
    // Discard the name. We only actually care about the offset pairs.
    functions[iter->second.package_offset] = iter->second.links;
  }
}

void Crystal_Linker::Add_Internal(unsigned address, unsigned offset)
{
  internals[address].push_back(offset);
}

BYTE* Crystal_Linker::Link(BYTE* code)
{
  unsigned double_index = 0;
  unsigned string_index = double_index + doubles.size() * sizeof(double);
  unsigned index;

  // Size of strings
  code_size = string_index;
  for(auto iter = strings.begin(); iter != strings.end(); iter++)
  {
    code_size += iter->first.size() + 1;
  }

  // Allocate the memory
  read_only_memory = (BYTE*)VirtualAllocEx( GetCurrentProcess(), 0, code_size, MEM_COMMIT | MEM_RESERVE , PAGE_EXECUTE_READWRITE);
  
  // Copy over executable code
  //strcpy((char *)code, input.c_str());

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
      (int&)code[iter->second[j]] = (int)code + iter->first;
    }
  }
    
  return code;
}