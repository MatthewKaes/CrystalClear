#ifndef CRYSTAL_GENERATOR
#define CRYSTAL_GENERATOR

#include "Compiler.h"
#include "Operations.h"

typedef bool (*GENERATOR_CODE)(Crystal_Compiler*, Crystal_Data*, std::vector<Crystal_Data>*, Crystal_Data*);

GENERATOR_CODE Resolve_Genorator(Crystal_Data* sym);
GENERATOR_CODE Resolve_Operator(Crystal_Data* sym);

bool Null_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);

//System Generators
bool Library_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);

#endif