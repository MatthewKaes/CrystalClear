#ifndef CRYSTAL_LIBRARY
#define CRYSTAL_LIBRARY

#include "Crystal.h"

//BIP (built in package) functions.
void Crystal_Print(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_PrintColor(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* color);

//No BIP (built in package) functions.
void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length);

#endif
