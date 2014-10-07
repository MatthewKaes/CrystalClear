#include "Crystal_Math.h"
#include "Helper.h"

void Crystal_Cos(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = cos(Parse_Double(sym));
}
void Crystal_Sin(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = sin(Parse_Double(sym));
}