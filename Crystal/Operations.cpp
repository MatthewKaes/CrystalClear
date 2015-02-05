#include "Operations.h"
#include "Helper.h"
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
bool Division_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Div);
}
bool Modulo_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Mod);
}
bool Power_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Pow);
}
bool And_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(And);
}
bool Or_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Or);
}
bool Equal_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Eql);
}
bool Diffrent_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Dif);
}
bool Less_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Les);
}
bool Less_Equal_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(LesEql);
}
bool Greater_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(Gtr);
}
bool Greater_Equal_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_OPERATION(GtrEql);
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
  if(base->i32 == REFRENCE_ID)
  {    
    switch((*syms)[1].type)
    {
    case DAT_NIL:
      target->Push_Reg();
      target->Call(Ref_Nil);
      target->Pop(1);
      break;
    case DAT_BOOL:
      target->Push_C(static_cast<int>((*syms)[1].b));
      target->Call(Ref_Bool);
      target->Pop(2);
      break;
    case DAT_INT:
      target->Push_C((*syms)[1].i32);
      target->Call(Ref_Int);
      target->Pop(2);
      break;
    case DAT_DOUBLE:
      target->Push_C((*syms)[1].d);
      target->Call(Ref_Double);
      target->Pop(3);
      break;
    case DAT_STRING:
      target->Push_C((*syms)[1].str.c_str());
      target->Call(Ref_Text);
      target->Pop(2);
      break;
    case DAT_REGISTRY:
    case DAT_LOCAL:
      target->Push(MEM((*syms)[1]));
      target->Call(Cry_Assignment);
      target->Pop(2);
      break;
    }
    return true;
  }

  //Normal Assignment
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
bool Divisional_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_ASSIGNMENT(Div);
}
bool Remainder_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_ASSIGNMENT(Mod);
}
bool Exponent_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  PREFORM_ASSIGNMENT(Pow);
}

bool Swap_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->Swap(MEM((*syms)[0]), MEM((*syms)[1]));
  return true;
}

bool Array_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if(base->i32 == GETTER_ID)
  {
    //Create array
    unsigned capacity = syms->size() * 2;
    if(capacity < 0x20)
      capacity = 0x20;

    target->Allocate(capacity);
    for(unsigned i = 0; i < syms->size(); i++)
    {
      target->Push_C(static_cast<int>(i));
      switch((*syms)[i].type)
      {
      case DAT_NIL:
        target->Call(Array_Add_Nil);
        target->Pop(1);
        break;
      case DAT_BOOL:
        target->Push_C(static_cast<int>((*syms)[i].b));
        target->Call(Array_Add_Bool);
        target->Pop(2);
        break;
      case DAT_INT:
        target->Push_C((*syms)[i].i32);
        target->Call(Array_Add_Int);
        target->Pop(2);
        break;
      case DAT_DOUBLE:
        target->Push_C((*syms)[i].d);
        target->Call(Array_Add_Double);
        target->Pop(3);
        break;
      case DAT_STRING:
        target->Push_C((*syms)[i].str.c_str());
        target->Call(Array_Add_Text);
        target->Pop(2);
        break;
      case DAT_REGISTRY:
      case DAT_LOCAL:
        target->Push(MEM((*syms)[i]));
        target->Call(Array_Add_Stack);
        target->Pop(2);
        break;
      }
    }
    target->Make_Array(MEMR(result), syms->size(), capacity);
  }
  else
  {
    //Getter
    switch((*syms)[1].type)
    {
    case DAT_NIL:
      target->Push_C(0);
      break;
    case DAT_BOOL:
      target->Push_C(static_cast<int>((*syms)[1].b));
      break;
    case DAT_INT:
      target->Push_C((*syms)[1].i32);
      break;
    case DAT_DOUBLE:
      target->Push_C(static_cast<int>((*syms)[1].d));
      break;
    case DAT_STRING:
      target->Push_C(atoi((*syms)[1].str.c_str()));
      break;
    case DAT_REGISTRY:
    case DAT_LOCAL:
      target->Convert(MEM((*syms)[1]), CRY_INT);
      break;
    }
    target->Push(MEM((*syms)[0]));

    if(base->i32 == REFRENCE_ID)
    {
      target->Call(Get_Ptr);
      target->Pop(2);
      target->Push_Reg();
    }
    else
    {
      target->Push(MEMR(result));
      target->Call(Copy_Ptr);
      target->Pop(3);
    }
  }
  return true;
}
bool Range_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  //Value Right
  switch((*syms)[1].type)
  {
  case DAT_NIL:
    target->Push_C(0);
    break;
  case DAT_BOOL:
    target->Push_C(static_cast<int>((*syms)[1].b));
    break;
  case DAT_INT:
    target->Push_C((*syms)[1].i32);
    break;
  case DAT_DOUBLE:
    target->Push_C(static_cast<int>((*syms)[1].d));
    break;
  case DAT_STRING:
    target->Push_C(atoi((*syms)[1].str.c_str()));
    break;
  case DAT_REGISTRY:
  case DAT_LOCAL:
    target->Convert(MEM((*syms)[1]), CRY_INT);
    break;
  }

  //Value Left
  switch((*syms)[0].type)
  {
  case DAT_NIL:
    target->Push_C(0);
    break;
  case DAT_BOOL:
    target->Push_C(static_cast<int>((*syms)[0].b));
    break;
  case DAT_INT:
    target->Push_C((*syms)[0].i32);
    break;
  case DAT_DOUBLE:
    target->Push_C(static_cast<int>((*syms)[0].d));
    break;
  case DAT_STRING:
    target->Push_C(atoi((*syms)[0].str.c_str()));
    break;
  case DAT_REGISTRY:
  case DAT_LOCAL:
    target->Convert(MEM((*syms)[0]), CRY_INT);
    break;
  }

  target->Make_Range(MEMR(result));
  return true;
}