#include "Helper.h"
#include "Lexicon.h"

int Parse_Int(Crystal_Symbol* sym)
{
  if(sym->type == CRY_INT)
  {
    return sym->i32;
  }
  return 0;
}
bool Parse_Bool(Crystal_Symbol* sym)
{
  if(sym->type == CRY_BOOL)
  {
    return sym->b;
  }
  return false;
}
double Parse_Double(Crystal_Symbol* sym)
{
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
    sym->ptr.str[sym->size - 1] = 0;
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