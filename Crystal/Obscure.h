#ifndef CRYSTAL_OBSCURE
#define CRYSTAL_OBSCURE

#include "Crystal.h"

//Standard
void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Subtraction(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Multiplication(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Power(Crystal_Symbol* dest, Crystal_Symbol* source);
//Reverse Variants
void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_SubtractionR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_PowerR(Crystal_Symbol* dest, Crystal_Symbol* source);

#endif