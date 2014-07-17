#include "Generator.h"

GENERATOR_CODE Resolve_Genorator(Data_Type type)
{
  return Null_Gen;
}

bool Null_Gen(Crystal_Compiler* target)
{ 
  return false; 
}