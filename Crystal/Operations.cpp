#include "Operations.h"
#include <functional>

typedef void (Crystal_Compiler::*OPERATION)(unsigned, unsigned, bool);
typedef void (Crystal_Compiler::*OPERATION_C)(unsigned, CRY_ARG, bool);
#define PREFORM_OPERATION(op) return Generic_Operation(target, base, syms, result, &Crystal_Compiler::##op, &Crystal_Compiler::##op ## C)
#define PREFORM_ASSIGNMENT(op) return Generic_Assignment(target, base, syms, result, &Crystal_Compiler::##op, &Crystal_Compiler::##op ## C)

int Mem_Conv(Crystal_Compiler* target, Crystal_Data* sym)
{
  if(sym->type == DAT_LOCAL)
    return sym->i32;
  return target->Addr_Reg(sym->i32);
}

bool Generic_Operation(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result, OPERATION func, OPERATION_C func_const)
{
  if(((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY) &&
    ((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY))
  {
    if(MEM((*syms)[0]) == MEMR(result))
      (target->*func)(MEM((*syms)[0]), MEM((*syms)[1]), true);
    else if(MEM((*syms)[1]) == MEMR(result))
      (target->*func)(MEM((*syms)[1]), MEM((*syms)[0]), false);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      (target->*func)(MEMR(result), MEM((*syms)[1]), true);
    }
    return true;
  }
  if((*syms)[0].type == DAT_LOCAL || (*syms)[0].type == DAT_REGISTRY)
  {
    if(MEM((*syms)[0]) == MEMR(result))
      (target->*func_const)(MEM((*syms)[0]), &(*syms)[1], true);
    else
    {
      target->Copy(MEMR(result), MEM((*syms)[0]));
      (target->*func_const)(MEMR(result), &(*syms)[1], true);
    }
    return true;
  }

  target->Copy(MEMR(result), MEM((*syms)[1]));
  (target->*func_const)(MEMR(result), &(*syms)[0], false);

  return true;
}

bool Addition_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Add);
}
bool Subtraction_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Sub);
}
bool Multiplication_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Mul);
}
bool Power_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Pow);
}
bool Equal_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Eql);
}
bool Diffrent_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Dif);
}

bool Generic_Assignment(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result, OPERATION func, OPERATION_C func_const)
{
  if((*syms)[1].type == DAT_LOCAL || (*syms)[1].type == DAT_REGISTRY)
    (target->*func)(MEM((*syms)[0]), MEM((*syms)[1]), true);
  else
    (target->*func_const)(MEM((*syms)[0]), &(*syms)[1], true);
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
  PREFORM_ASSIGNMENT(Add);
}
bool Subtractive_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_ASSIGNMENT(Sub);
}
bool Multiplicative_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_ASSIGNMENT(Mul);
}