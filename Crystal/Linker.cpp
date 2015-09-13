#include "Linker.h"
#include <windows.h>

#include "Library.h"
#include "Function.h"
#include "Filesystems.h"
#include "Helper.h"
#include "Garbage_Collector.h"
#include "Obscure.h"
#include "Core.h"

extern std::unordered_map<std::string, Package_Info> built_in;

#define REGISTER_SUPPORT(function) supported_functions[#function] = function;

Crystal_Linker::Crystal_Linker()
{
  executable = NULL;
  read_only_memory = NULL;
  code_size = 0;
  entry_point = 0;

  // Standard Library
  REGISTER_SUPPORT(memset);
  REGISTER_SUPPORT(calloc);

  // Linking Support Functionallity
  REGISTER_SUPPORT(Copy_Ref);
  REGISTER_SUPPORT(This_Copy);
  REGISTER_SUPPORT(Stack_Copy);
  REGISTER_SUPPORT(Parse_Int);
  REGISTER_SUPPORT(Parse_Bool);
  REGISTER_SUPPORT(Parse_Double);
  REGISTER_SUPPORT(Parse_String);
  REGISTER_SUPPORT(Fast_strcmp);
  REGISTER_SUPPORT(Fast_pointercmp);
  REGISTER_SUPPORT(Late_Func_Binding);
  REGISTER_SUPPORT(Late_Attr_Binding);
  REGISTER_SUPPORT(Late_Func_Binding_Ref);
  REGISTER_SUPPORT(Late_Attr_Binding_Ref);

  // Linking Complex Support Functionallity
  REGISTER_SUPPORT(Construct_Class);
  REGISTER_SUPPORT(Construct_Array);
  REGISTER_SUPPORT(Array_Add_Nil);
  REGISTER_SUPPORT(Push_Int);
  REGISTER_SUPPORT(Push_Double);
  REGISTER_SUPPORT(Push_Text);
  REGISTER_SUPPORT(Val_Binding);

  // Linking Garbage Collection
  REGISTER_SUPPORT(GC_Branch);
  REGISTER_SUPPORT(GC_Extend_Generation);
  REGISTER_SUPPORT(GC_Collect);

  // Linking Dynamic Math Support
  REGISTER_SUPPORT(Obscure_Addition);
  REGISTER_SUPPORT(Obscure_Subtraction);
  REGISTER_SUPPORT(Obscure_Multiplication);
  REGISTER_SUPPORT(Obscure_Division);
  REGISTER_SUPPORT(Obscure_Modulo);
  REGISTER_SUPPORT(Obscure_Power);
  REGISTER_SUPPORT(Obscure_Equal);
  REGISTER_SUPPORT(Obscure_Diffrence);
  REGISTER_SUPPORT(Obscure_Less);
  REGISTER_SUPPORT(Obscure_Greater);
  REGISTER_SUPPORT(Obscure_Less_Equal);
  REGISTER_SUPPORT(Obscure_Greater_Equal);
  REGISTER_SUPPORT(Obscure_AdditionR);
  REGISTER_SUPPORT(Obscure_SubtractionR);
  REGISTER_SUPPORT(Obscure_DivisionR);
  REGISTER_SUPPORT(Obscure_ModuloR);
  REGISTER_SUPPORT(Obscure_PowerR);
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
  // When loading the first function is the entry point
  if(entry_point == static_cast<unsigned>(-1))
    entry_point = func;

  functions[func].push_back(offset);
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
}

void Crystal_Linker::Add_Internal(unsigned address, unsigned offset)
{
  internals[address].push_back(offset);
}

void Crystal_Linker::Set_Internal(const std::unordered_map<std::string, std::vector<unsigned>>* values)
{
  // Linking the Crystal Built in Library
  for (auto itr = built_in.begin(); itr != built_in.end(); itr++)
  {
    supported_functions[itr->second.name] = itr->second.function;
  }

  // Copy over all the locations that need to be fixed up by the
  // linker for user defined functions.
  for (auto iter = values->begin(); iter != values->end(); iter++)
  {
    if (supported_functions.find(iter->first) == supported_functions.end())
    {
      printf("Failed to link function '%s'.\n", iter->first.c_str());
      continue;
    }

    // Grab all of the offset pairs.
    for (unsigned i = 0; i < iter->second.size(); i++)
      internals[reinterpret_cast<unsigned>(supported_functions[iter->first])].push_back(iter->second[i]);
  }
}

BYTE* Crystal_Linker::Link(BYTE* code)
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
      (int&)code[iter->second[j]] = iter->first;
    }
  }
    
  return code;
}

BYTE* Crystal_Linker::Entry()
{
  return executable + entry_point;
}
