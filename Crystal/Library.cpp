#include "Library.h"
#include "Lexicon.h"

void Crystal_Print(Crystal_Symbol* sym)
{
  if(sym->type == CRY_STRING || sym->type == CRY_TEXT)
  {
    if(sym->type == CRY_STRING)
      sym->ptr.str[sym->size - 1] = 0;
    printf("%s\n", sym->ptr.str);
    return;
  }
  std::string val;
  if(sym->type == CRY_INT)
  {
    i_to_str(sym->i32, &val);
  }
  else if(sym->type == CRY_INT64)
  {
    l_to_str(sym->i64, &val);
  }
  else if(sym->type == CRY_DOUBLE)
  {
    d_to_str(sym->d, &val);
  }
  else if(sym->type == CRY_BOOL)
  {
    b_to_str(sym->b, &val);
  }
  else if(sym->type == CRY_NIL)
  {
    val.assign("nil");
  }
  printf("%s\n", val.c_str());
}
