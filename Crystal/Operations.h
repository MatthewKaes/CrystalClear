#ifndef CRYSTAL_OPERATIONS
#define CRYSTAL_OPERATIONS

#include "Compiler.h"

//Operator Generators
bool Addition_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Subtraction_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);
bool Assignment_Gen(Crystal_Compiler* target, Crystal_Data* base, std::vector<Crystal_Data>* syms, Crystal_Data* result);

#endif