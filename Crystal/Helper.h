#ifndef CRYSTAL_HELPER
#define CRYSTAL_HELPER

#include "Crystal.h"

int Parse_Int(Crystal_Symbol* sym);
int Parse_Bool(Crystal_Symbol* sym);
double Parse_Double(Crystal_Symbol* sym);
void Parse_String(Crystal_Symbol* sym, std::string* str);

bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr);
int Fast_pointercmp(Crystal_Symbol* aryl, Crystal_Symbol* aryr);
int Fast_arraycmp(Crystal_Symbol* aryl, Crystal_Symbol* aryr);

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

//Refrence functions
void Push_Nil(Crystal_Symbol* sym);
void Push_Bool(int num, Crystal_Symbol* sym);
void Push_Int(int num, Crystal_Symbol* sym);
void Push_Double(double dec, Crystal_Symbol* sym);
void Push_Text(const char* text, Crystal_Symbol* sym);

void Val_Binding(Crystal_Symbol* dest, Crystal_Symbol* src, int index);
void Cry_Assignment(Crystal_Symbol* dest, Crystal_Symbol* src);

void* Late_Func_Binding(int id, Crystal_Symbol* symd);
void* Late_Func_Binding_Ref(int id, Crystal_Symbol* symd);
void Late_Attr_Binding(int id, Crystal_Symbol* symd, Crystal_Symbol* ret);
void Late_Attr_Binding_Ref(int id, Crystal_Symbol* symd, Crystal_Symbol* ret);

void Copy_Ref(Crystal_Symbol* dest, Crystal_Symbol* src);

#endif