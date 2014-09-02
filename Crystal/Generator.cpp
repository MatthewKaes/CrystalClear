#include "Generator.h"

GENERATOR_CODE Resolve_Genorator(Crystal_Data* sym)
{
  return Null_Gen;
}

bool Null_Gen(Crystal_Compiler* target, std::vector<Crystal_Data>* syms)
{ 
  return false; 
}