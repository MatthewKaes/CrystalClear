#include "Operations.h"

bool Assignment_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Copy((*syms)[0].i32, (*syms)[1].i32);
  else
    target->Load((*syms)[0].i32, &(*syms)[1]);
  return true;
}
