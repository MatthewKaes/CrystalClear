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
void Crystal_Sum(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Mean(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Min(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Max(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Var(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Sd(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Step(Crystal_Symbol* ret_sym, Crystal_Symbol* start, Crystal_Symbol* end, Crystal_Symbol* step);

#endif