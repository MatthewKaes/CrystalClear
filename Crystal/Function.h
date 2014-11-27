#ifndef CRYSTAL_FUNCTION
#define CRYSTAL_FUNCTION

#include "Crystal.h"

int Crystal_And(Crystal_Symbol* left, Crystal_Symbol* right);
int Crystal_Or(Crystal_Symbol* left, Crystal_Symbol* right);

//No BIP (built in package) functions.
void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Text_AppendR(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length);

void Construct_Array(Crystal_Symbol* symd, unsigned size, Crystal_Symbol* ary);
void Construct_Range(Crystal_Symbol* symd, int left, int right);
void Construct_String(Crystal_Symbol* symd,  char* str, unsigned size);

#endif