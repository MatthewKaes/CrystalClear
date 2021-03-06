#ifndef CRYSTAL_FUNCTION
#define CRYSTAL_FUNCTION

#include "Crystal.h"

bool Crystal_Compare(Crystal_Symbol* left, Crystal_Symbol* right);

int Crystal_And(Crystal_Symbol* left, Crystal_Symbol* right);
int Crystal_Or(Crystal_Symbol* left, Crystal_Symbol* right);
int Crystal_Elements(Crystal_Symbol* sym);

//No BIP (built in package) functions.
void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Text_Append_Rev(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Text_Append_Loc(Crystal_Symbol* symd, Crystal_Symbol* syms, Crystal_Symbol* result);

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length);
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length);

void Crystal_Array_Append(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Array_Append_Rev(Crystal_Symbol* symd, Crystal_Symbol* syms);
void Crystal_Array_Append_Loc(Crystal_Symbol* symd, Crystal_Symbol* syms, Crystal_Symbol* result);

void Construct_Array(Crystal_Symbol* symd, unsigned size, unsigned capacity, Crystal_Symbol* ary);
void Construct_Range(Crystal_Symbol* symd, int left, int right);
void Construct_String(Crystal_Symbol* symd,  char* str, unsigned size);
void Construct_Class(int id, Crystal_Symbol* symd);
void Clone_Class(Crystal_Symbol* symd, Crystal_Symbol* sym, Crystal_Symbol* data);

#endif