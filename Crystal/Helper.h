#ifndef CRYSTAL_HELPER
#define CRYSTAL_HELPER

#include "Compiler.h"

int Parse_Int(Crystal_Symbol* sym);
bool Parse_Bool(Crystal_Symbol* sym);
double Parse_Double(Crystal_Symbol* sym);
void Parse_String(Crystal_Symbol* sym, std::string* str);

#endif