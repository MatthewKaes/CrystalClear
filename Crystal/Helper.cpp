#include "Helper.h"
#include "Lexicon.h"

int Parse_Int(Crystal_Symbol* sym)
{
  switch(sym->type)
  {
  case CRY_STRING:
    {
      std::string str(sym->str);
      return str_to_i(&str);
    }
  case CRY_TEXT:
    {
      std::string str(sym->text);
      return str_to_i(&str);
    }
  case CRY_INT:
    return sym->i32;
  case CRY_BOOL:
    return sym->b;
  case CRY_DOUBLE:
    return static_cast<int>(sym->d);
  case CRY_POINTER:
    return Parse_Int(sym->sym);
  default:
    return 0;
  }
}
int Parse_Bool(Crystal_Symbol* sym)
{
  if(sym->type == CRY_NIL)
    return 0;
  return sym->i32 != 0;
}
double Parse_Double(Crystal_Symbol* sym)
{
  switch(sym->type)
  {
  case CRY_STRING:
    {
      std::string str(sym->str);
      return str_to_d(&str);
    }
  case CRY_TEXT:
    {
      std::string str(sym->text);
      return str_to_d(&str);
    }
  case CRY_BOOL:
    return sym->b;
  case CRY_DOUBLE:
    return sym->d;
  case CRY_INT:
    return sym->i32;
  case CRY_POINTER:
    return Parse_Double(sym->sym);
  default:
    return 0.0;
  }
}
void Parse_String(Crystal_Symbol* sym, std::string* str)
{
  str->clear();
  switch(sym->type)
  {
  case CRY_STRING:
    str->assign(sym->str);
    return;
  case CRY_TEXT:
    str->assign(sym->text);
    return;
  case CRY_INT:
    i_to_str(sym->i32, str);
    return;
  case CRY_INT64:
    l_to_str(sym->i64, str);
    return;
  case CRY_DOUBLE:
    d_to_str(sym->d, str);
    return;
  case CRY_BOOL:
    b_to_str(sym->b, str);
    return;
  case CRY_POINTER:
    Parse_String(sym->sym, str);
    return;
  case CRY_NIL:
    str->assign("nil");
    return;
  }
}
bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  int i = 0;
  const char* l = syml->type == CRY_TEXT ? syml->text : syml->str;
  const char* r = symr->type == CRY_TEXT ? symr->text : symr->str;
  while(*(l + i) || *(r + i))
  {
    if(*(l + i) != *(r + i))
      return false;
    i++;
  }
  return true;
}
void Stack_Copy(Crystal_Symbol* sym_stack, Crystal_Symbol* sym_from)
{
  sym_stack->i64 = sym_from->i64;
  sym_stack->sym = sym_from->sym;
  sym_stack->type = sym_from->type;
}
void Power_Syms(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  double l = syml->type == CRY_DOUBLE ? syml->d : syml->i32;
  double r = symr->type == CRY_DOUBLE ? symr->d : symr->i32;
  syml->d = pow(l, r);
}
void Power_SymsR(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  double l = syml->type == CRY_DOUBLE ? syml->d : syml->i32;
  double r = symr->type == CRY_DOUBLE ? symr->d : symr->i32;
  syml->d = pow(r, l);
}
void Cry_Derefrence(Crystal_Symbol** sym)
{
  if((*sym)->type == CRY_POINTER)
  {
    *sym = (*sym)->sym;
  }
}
//Array functions
void Array_Add_Nil(int index, Crystal_Symbol* ary)
{
  ary[index].type = CRY_NIL;
}
void Array_Add_Bool(int num, int index, Crystal_Symbol* ary)
{
  ary[index].type = CRY_BOOL;
  ary[index].i32 = num;
}
void Array_Add_Int(int num, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_INT;
  ary[index].i32 = num;
}
void Array_Add_Double(double dec, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_DOUBLE;
  ary[index].d = dec;
}
void Array_Add_Text(const char* text, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_TEXT;
  ary[index].text = text;
}
void Array_Add_Var(Crystal_Symbol* sym, int index, Crystal_Symbol* ary)
{
  Stack_Copy(ary->sym + index, sym);
}
void Array_Add_Stack(Crystal_Symbol* sym_stack, int index, Crystal_Symbol* ary)
{
  if(sym_stack->type == CRY_STRING)
  {
    ary[index].size = sym_stack->size;
    ary[index].text = sym_stack->str;
    ary[index].type = CRY_TEXT;
    return;
  }
  else
  {
    ary[index].i64 = sym_stack->i64;
    ary[index].sym = sym_stack->sym;
    ary[index].type = sym_stack->type;
  }
}

//Refrence functions
void Ref_Nil(Crystal_Symbol* sym)
{
  sym->type = CRY_NIL;
}
void Ref_Bool(int num, Crystal_Symbol* sym)
{
  sym->type = CRY_BOOL;
  sym->i32 = num;
}
void Ref_Int(int num, Crystal_Symbol* sym)
{
  sym->type = CRY_INT;
  sym->i32 = num;
}
void Ref_Double(double dec, Crystal_Symbol* sym)
{
  sym->type = CRY_DOUBLE;
  sym->d = dec;
}
void Ref_Text(const char* text, Crystal_Symbol* sym)
{
  sym->type = CRY_TEXT;
  sym->text = text;
}
void Cry_Assignment(Crystal_Symbol* src, Crystal_Symbol* dest)
{
  *dest = *src;
}

int Printer(Crystal_Symbol* sym)
{
  int counter = 0;
  if(sym->type == CRY_POINTER)
  {
    counter += Printer(sym->sym);
  }
  else if(sym->type == CRY_STRING)
  {
    counter += printf("%s", sym->str);
  }
  else if(sym->type == CRY_ARRAY)
  {
    counter += printf("[");
    for(unsigned i = 0; i < sym->size; i++)
    {
      counter += Printer(&sym->sym[i]);
      if(i != sym->size - 1)
      {
        counter += printf(", ");
      }
    }
    counter += printf("]");
  }
  else
  {
    std::string val;
    Parse_String(sym, &val);
    counter += printf("%s", val.c_str());
  }
  return counter;
}
void Copy_Ptr(Crystal_Symbol* res,  Crystal_Symbol* src, int index)
{
  *res = src->sym->sym[index];
}
Crystal_Symbol* Get_Ptr(Crystal_Symbol* src, int index)
{
  return src->sym->sym + index;
}