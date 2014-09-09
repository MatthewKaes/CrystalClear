#include "Operations.h"

bool Addition_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if((*syms)[0].i32 == result->i32)
      target->Add((*syms)[0].i32, (*syms)[1].i32);
    else if((*syms)[1].i32 == result->i32)
      target->Add((*syms)[1].i32, (*syms)[0].i32);
    else
    {
      target->Copy(result->i32, (*syms)[0].i32);
      target->Add(result->i32, (*syms)[1].i32);
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if((*syms)[0].i32 == result->i32)
      target->AddC((*syms)[0].i32, &(*syms)[1]);
    else
    {
      target->Copy(result->i32, (*syms)[0].i32);
      target->AddC(result->i32, &(*syms)[1]);
    }
    return true;
  }
  if((*syms)[1].i32 == result->i32)
    target->AddC((*syms)[1].i32, &(*syms)[0], false);
  else
  {
    target->Copy(result->i32, (*syms)[1].i32);
    target->AddC(result->i32, &(*syms)[1], false);
  }

  return true;
}
bool Assignment_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Copy((*syms)[0].i32, (*syms)[1].i32);
  else
    target->Load((*syms)[0].i32, &(*syms)[1]);
  return true;
}
