#ifndef CRYSTAL_MATH
#define CRYSTAL_MATH

#include "Compiler.h"

#define PI 3.141592653589793
#define C_E 2.71828

void Crystal_Cos(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Sin(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Log(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_NatrualLog(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Abs(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);

#endif