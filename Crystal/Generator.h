#ifndef CRYSTAL_GENERATOR
#define CRYSTAL_GENERATOR

#include "Compiler.h"

typedef bool (*GENERATOR_CODE)(Crystal_Compiler*);

GENERATOR_CODE Resolve_Genorator(Data_Type type);

bool Null_Gen(Crystal_Compiler* target);

#endif