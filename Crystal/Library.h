#ifndef CRYSTAL_LIBRARY
#define CRYSTAL_LIBRARY

#include "Crystal.h"

//BIP (built in package) functions.
void Crystal_Time(Crystal_Symbol* ret_sym);
void Crystal_Input(Crystal_Symbol* ret_sym);
void Crystal_Convert(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* conversion);
void Crystal_Boolean(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Integer(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Double(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_String(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Type(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Rand(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Print(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_PrintColor(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* color);
void Crystal_Size(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);

//Python
void Crystal_Python(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* func);

#endif
