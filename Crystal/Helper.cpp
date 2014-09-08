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
  return 0.0;
}
void Parse_String(Crystal_Symbol* sym, std::string* str)
{
  str->clear();
  if(sym->type == CRY_STRING || sym->type == CRY_TEXT)
  {
    if(sym->type == CRY_STRING)
      sym->ptr.str[sym->size - 1] = 0;
    str->assign(sym->ptr.str);
    return;
  }
  if(sym->type == CRY_INT)
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