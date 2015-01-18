#ifndef CRYSTAL_GENERATOR
#define CRYSTAL_GENERATOR

#include "Compiler.h"
#include "Operations.h"

typedef bool (*GENERATOR_CODE)(Crystal_Compiler*, Crystal_Data*, std::vector<Crystal_Data>*, Crystal_Data*);

GENERATOR_CODE Resolve_Generator(Crystal_Data* sym);
GENERATOR_CODE Resolve_Operator(Crystal_Data* sym);
GENERATOR_CODE Resolve_Statement(Crystal_Data* sym);

bool Null_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);

//System Generators
bool Function_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Library_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Return_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool End_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Loop_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool If_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Else_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool ElseIf_Preface_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool ElseIf_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool While_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);

#endif