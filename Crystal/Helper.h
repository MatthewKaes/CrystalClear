#ifndef CRYSTAL_HELPER
#define CRYSTAL_HELPER

#include "Compiler.h"

int Parse_Int(Crystal_Symbol* sym);
int Parse_Bool(Crystal_Symbol* sym);
double Parse_Double(Crystal_Symbol* sym);
void Parse_String(Crystal_Symbol* sym, std::string* str);

bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr);

void Power_Syms(Crystal_Symbol* syml, Crystal_Symbol* symr);
void Power_SymsR(Crystal_Symbol* syml, Crystal_Symbol* symr);

#endif