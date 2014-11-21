#include "Math.h"
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
void Crystal_Log(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = log10(Parse_Double(sym));
}
void Crystal_NatrualLog(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = log(Parse_Double(sym));
}
void Crystal_Abs(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if(sym->type == CRY_DOUBLE)
  {
    ret_sym->d =  abs(sym->d);
  }
  else if(sym->type == CRY_INT)
  {
    ret_sym->i32 = abs(sym->i32);
  }
  else
  {
    ret_sym->d = abs(Parse_Double(sym));
  }
}