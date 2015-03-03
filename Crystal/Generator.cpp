#include "Generator.h"
#include "Function.h"

GENERATOR_CODE Resolve_Generator(Crystal_Data* sym)
{
  switch(sym->type)
  {
  case DAT_FUNCTION:
    return Function_Gen;
  case DAT_BIFUNCTION:
    return Library_Gen;
  case DAT_CLASS:
    return Class_Gen;
  case DAT_OP:
    return Resolve_Operator(sym);
  case DAT_STATEMENT:
    return Resolve_Statement(sym);
  }
  return Null_Gen;
}

GENERATOR_CODE Resolve_Operator(Crystal_Data* sym)
{
  switch(sym->str.c_str()[0])
  {
  case '<':
    if(sym->str.c_str()[1] == '\0')
      return Less_Gen;
    if(sym->str.c_str()[1] == '>')
      return Swap_Gen;
    else
      return Less_Equal_Gen;
    break;
  case '>':
    if(sym->str.c_str()[1] == '\0')
      return Greater_Gen;
    else
      return Greater_Equal_Gen;
    break;
  case '&':
    switch(sym->str.c_str()[1])
    {
    case '&':
      return And_Gen;
    }
    break;
  case '|':
    switch(sym->str.c_str()[1])
    {
    case '|':
      return Or_Gen;
    }
    break;
  case '=':
    switch(sym->str.c_str()[1])
    {
    case '\0':
      return Assignment_Gen;
    case '=':
      return Equal_Gen;
    }
    break;
  case '.':
    if(sym->str.c_str()[1] == '.' && sym->str.c_str()[2] == '.')
    {
      return Range_Gen;
    }
    break;
  case '!':
    switch(sym->str.c_str()[1])
    {
    case '=':
      return Diffrent_Gen;
    }
    break;
  case '+':
    if(sym->str.c_str()[1] == '=')
      return Additive_Gen;
    else
      return Addition_Gen;
    break;
  case '-':
    if(sym->str.c_str()[1] == '=')
      return Subtractive_Gen;
    else
      return Subtraction_Gen;
    break;
  case '*':
    if(sym->str.c_str()[1] == '=')
      return Multiplicative_Gen;
    else
      return Multiplication_Gen;
    break;
  case '/':
    if(sym->str.c_str()[1] == '=')
      return Divisional_Gen;
    else
      return Division_Gen;
    break;
  case '%':
    if(sym->str.c_str()[1] == '=')
      return Remainder_Gen;
    else
      return Modulo_Gen;
    break;
  case '^':
    if(sym->str.c_str()[1] == '=')
      return Exponent_Gen;
    else
      return Power_Gen;
    break;
  case '[':
    return Array_Gen;
    break;
  }
  return Null_Gen;
}

GENERATOR_CODE Resolve_Statement(Crystal_Data* sym)
{
  switch(sym->str.c_str()[0])
  {
  case 'r':
    return Return_Gen;
  case 'i':
    return If_Gen;
  case 'w':
    return While_Gen;
  case 'e':
    if(!sym->str.compare("end"))
      return End_Gen;
    if(!sym->str.compare("elsif"))
      return ElseIf_Gen;
    return Else_Gen;
  }
  return Null_Gen;
}

bool Null_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{ 
  return false;
}

bool Function_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  for(int i = static_cast<int>(syms->size() / 2) - 1; i >= 0; i--)
  {
    if((*syms)[i].type != DAT_LOCAL && (*syms)[i].type != DAT_REGISTRY)
      target->Load(Mem_Conv(target, &(*syms)[i + syms->size() / 2]), &(*syms)[i]);
    target->Push(Mem_Conv(target, &(*syms)[i + syms->size() / 2]));
  }
  if(result->type == DAT_NIL)
  {
    target->Call(base->str.c_str());
    target->Pop(static_cast<int>(syms->size() / 2));
  }
  else
  {
    target->Call(base->str.c_str(), MEM(*result));
    target->Pop(static_cast<int>(syms->size() / 2) + 1);
  }

  return true;
}

bool Library_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  for(int i = static_cast<int>(syms->size() / 2) - 1; i >= 0; i--)
  {
    if((*syms)[i].type != DAT_LOCAL && (*syms)[i].type != DAT_REGISTRY)
      target->Load(Mem_Conv(target, &(*syms)[i + syms->size() / 2]), &(*syms)[i]);
    target->Push(Mem_Conv(target, &(*syms)[i + syms->size() / 2]));
  }

  target->Call(base->external, MEMR(result));
  target->Pop(static_cast<int>(syms->size() / 2) + 1);
  
  return true;
}

bool Class_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->Push_C(base->i32);
  target->Push(MEMR(result));

  target->Call(Construct_Class);
  target->Pop(2);
  
  return true;
}

bool Return_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{    
  if((*syms)[0].type != DAT_LOCAL && (*syms)[0].type != DAT_REGISTRY)
    target->Load(Mem_Conv(target, &(*syms)[1]), &(*syms)[0]);
  target->Return(Mem_Conv(target, &(*syms)[1]));

  return true;
}

bool End_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->End();
  return true;
}

bool Loop_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->Loop();
  return true;
}

bool If_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[0].type != DAT_LOCAL && (*syms)[0].type != DAT_REGISTRY)
    target->Load(Mem_Conv(target, &(*syms)[1]), &(*syms)[0]);
  target->If(Mem_Conv(target, &(*syms)[1]));
  return true;
}

bool Else_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->Else();
  return true;
}

bool ElseIf_Preface_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  target->ElseIf_Pre();
  return true;
}

bool ElseIf_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[0].type != DAT_LOCAL && (*syms)[0].type != DAT_REGISTRY)
    target->Load(Mem_Conv(target, &(*syms)[1]), &(*syms)[0]);
  target->ElseIf(Mem_Conv(target, &(*syms)[1]));
  return true;
}

bool While_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result)
{
  if((*syms)[0].type != DAT_LOCAL && (*syms)[0].type != DAT_REGISTRY)
    target->Load(Mem_Conv(target, &(*syms)[1]), &(*syms)[0]);
  target->While(Mem_Conv(target, &(*syms)[1]));
  return true;
}
