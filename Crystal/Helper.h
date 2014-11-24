#ifndef CRYSTAL_HELPER
#define CRYSTAL_HELPER

#include "Crystal.h"

int Parse_Int(Crystal_Symbol* sym);
int Parse_Bool(Crystal_Symbol* sym);
double Parse_Double(Crystal_Symbol* sym);
void Parse_String(Crystal_Symbol* sym, std::string* str);

bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr);

void Stack_Copy(Crystal_Symbol* sym_stack, Crystal_Symbol* sym_from);

void Power_Syms(Crystal_Symbol* syml, Crystal_Symbol* symr);
void Power_SymsR(Crystal_Symbol* syml, Crystal_Symbol* symr);

//Derefrence
void Cry_Derefrence(Crystal_Symbol** sym);

//Array functions
void Array_Add_Nil(int index, Crystal_Symbol* ary);
void Array_Add_Bool(int num, int index, Crystal_Symbol* ary);
void Array_Add_Int(int num, int index, Crystal_Symbol* ary);
void Array_Add_Double(double dec, int index, Crystal_Symbol* ary);
void Array_Add_Text(const char* text, int index, Crystal_Symbol* ary);
void Array_Add_Var(Crystal_Symbol* sym, int index, Crystal_Symbol* ary);
void Array_Add_Stack(Crystal_Symbol* sym_stack, int index, Crystal_Symbol* ary);

void Garbage_Collection(Crystal_Symbol* sym);
void Crystal_Free(Crystal_Symbol* sym);
int Printer(Crystal_Symbol* sym);

#endif