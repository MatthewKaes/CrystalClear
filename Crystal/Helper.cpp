#include "Helper.h"
#include "Lexicon.h"

int Parse_Int(Crystal_Symbol* sym)
{
  if(sym->type == CRY_STRING)
  {
    std::string str(sym->ptr.str);
    return str_to_i(&str);
  }
  if(sym->type == CRY_TEXT)
  {
    std::string str(sym->text);
    return str_to_i(&str);
  }
  if(sym->type == CRY_INT)
  {
    return sym->i32;
  }
  if(sym->type == CRY_BOOL)
  {
    return sym->b;
  }
  if(sym->type == CRY_DOUBLE)
  {
    return static_cast<int>(sym->d);
  }
  return 0;
}
int Parse_Bool(Crystal_Symbol* sym)
{
  if(sym->type == CRY_NIL)
    return 0;
  return sym->i32 != 0;
}
double Parse_Double(Crystal_Symbol* sym)
{
  if(sym->type == CRY_STRING)
  {
    std::string str(sym->ptr.str);
    return str_to_d(&str);
  }
  if(sym->type == CRY_TEXT)
  {
    std::string str(sym->text);
    return str_to_d(&str);
  }
  if(sym->type == CRY_BOOL)
  {
    return sym->b;
  }
  if(sym->type == CRY_DOUBLE)
  {
    return sym->d;
  }
  if(sym->type == CRY_INT)
  {
    return sym->i32;
  }
  return 0.0;
}
void Parse_String(Crystal_Symbol* sym, std::string* str)
{
  str->clear();
  if(sym->type == CRY_STRING)
  {
    str->assign(sym->ptr.str);
  }
  else if(sym->type == CRY_TEXT)
  {
    str->assign(sym->text);
  }
  else if(sym->type == CRY_INT)
  {
    i_to_str(sym->i32, str);
  }
  else if(sym->type == CRY_INT64)
  {
    l_to_str(sym->i64, str);
  }
  else if(sym->type == CRY_DOUBLE)
  {
    d_to_str(sym->d, str);
  }
  else if(sym->type == CRY_BOOL)
  {
    b_to_str(sym->b, str);
  }
  else if(sym->type == CRY_NIL)
  {
    str->assign("nil");
  }
}
bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  int i = 0;
  char* l = syml->type == CRY_TEXT ? syml->text : syml->ptr.str;
  char* r = symr->type == CRY_TEXT ? symr->text : symr->ptr.str;
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
  if(sym_from->type == CRY_STRING)
  {
    sym_stack->size = sym_from->size;
    sym_stack->text = sym_from->ptr.str;
    sym_from->type = CRY_TEXT;
    return;
  }
  else
  {
    sym_stack->i64 = sym_from->i64;
    sym_stack->ptr = sym_from->ptr;
    sym_stack->type = sym_from->type;
    //Up ref count for symbols that need it.
    if(sym_stack->type >= CRY_ARRAY)
    {
      sym_stack->ptr.sym->ref_cnt += 1;
    }
  }
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
void Garbage_Collection(Crystal_Symbol* sym)
{
  if(sym->ptr.str != 0)
  {
    if(sym->type > CRY_STRING)
    {
      sym->ptr.sym->ref_cnt -= 1;
      if(sym->ptr.sym->ref_cnt == 0)
      {
      free(sym->ptr.sym);
      }
    }
    else
      free(sym->ptr.str);
    sym->ptr.sym = 0;
  }
}