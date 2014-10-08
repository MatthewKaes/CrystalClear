#include "Operations.h"

int Mem_Conv(Crystal_Compiler* target, Crystal_Data* sym)
{
  if(sym->type == DAT_LOCAL)
    return sym->i32;
  return target->Addr_Reg(sym->i32);
}

bool Addition_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Add(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Add(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Add(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->AddC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->AddC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->AddC(MEMR(result), &(*syms)[0], false);

  return true;
}
bool Subtraction_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Sub(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Sub(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Sub(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->SubC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->SubC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->SubC(MEMR(result), &(*syms)[0], false);

  return true;
}
bool Multiplication_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Mul(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Mul(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Mul(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->MulC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->MulC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->MulC(MEMR(result), &(*syms)[0], false);

  return true;
}
bool Power_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Pow(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Pow(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Pow(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->PowC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->PowC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->PowC(MEMR(result), &(*syms)[0], false);

  return true;
}
bool Equal_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Eql(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Eql(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Eql(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->EqlC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->EqlC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->EqlC(MEMR(result), &(*syms)[0], false);

  return true;
}
bool Diffrent_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->Dif(MEM((*syms)[0]), MEM((*syms)[1]));
    else if(MEM((*syms)[1]) == MEMR(result))
      target->Dif(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->Dif(MEMR(result), MEM((*syms)[1]));
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      target->DifC(MEM((*syms)[0]), &(*syms)[1]);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      target->DifC(MEMR(result), &(*syms)[1]);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  target->DifC(MEMR(result), &(*syms)[0], false);

  return true;
}

bool Assignment_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Copy(MEM((*syms)[0]), MEM((*syms)[1]));
  else
    target->Load(MEM((*syms)[0]), &(*syms)[1]);
  return true;
}
bool Additive_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Add(MEM((*syms)[0]), MEM((*syms)[1]));
  else
    target->AddC(MEM((*syms)[0]), &(*syms)[1]);
  return true;
}
bool Subtractive_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Sub(MEM((*syms)[0]), MEM((*syms)[1]));
  else
    target->SubC(MEM((*syms)[0]), &(*syms)[1]);
  return true;
}
bool Multiplicative_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    target->Mul(MEM((*syms)[0]), MEM((*syms)[1]));
  else
    target->MulC(MEM((*syms)[0]), &(*syms)[1]);
  return true;
}