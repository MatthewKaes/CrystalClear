#ifndef CRYSTAL_REDUCTIONS
#define CRYSTAL_REDUCTIONS

#include "Crystal.h"

bool Can_Reduce(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);

bool Reduction(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);

void Reduce_Addition(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Subtraction(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Multiplication(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Power(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Diffrence(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Less(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Less_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Greater(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
void Reduce_Greater_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right);
  
#endif