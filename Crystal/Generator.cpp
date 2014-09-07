#include "Generator.h"

GENERATOR_CODE Resolve_Genorator(Crystal_Data* sym)
{
  switch(sym->type)
  {
  case DAT_BIFUNCTION:
    return Library_Gen;
  case DAT_OP:
    return Resolve_Operator(sym);
  }
  return Null_Gen;
}
GENERATOR_CODE Resolve_Operator(Crystal_Data* sym)
{
  switch(sym->str.c_str()[0])
  {
  case '=':
    switch(sym->str.c_str()[1])
    {
    case '\0':
      return Assignment_Gen;
    }
    return Null_Gen;
  }
  return Null_Gen;
}

bool Null_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{ 
  return false; 
}
bool Library_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  for(unsigned i = 0; i < syms->size() - 1; i += 2)
  {
    if((*syms)[i].type != DAT_LOCAL && (*syms)[i].type != DAT_REGISTRY)
      target->Load((*syms)[i + 1].i32, &(*syms)[i]);
    target->Push((*syms)[i + 1].i32);
  }

  target->Call(base->external);
  
  return true;
}