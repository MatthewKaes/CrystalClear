#ifndef CRYSTAL_GENERATOR
#define CRYSTAL_GENERATOR

#include "Compiler.h"

typedef bool (*GENERATOR_CODE)(Crystal_Compiler*, std::vector<Crystal_Data>*);

GENERATOR_CODE Resolve_Genorator(Crystal_Data* sym);

bool Null_Gen(Crystal_Compiler* target, std::vector<Crystal_Data>* syms);

#endif