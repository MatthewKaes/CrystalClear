#ifndef CRYSTAL_OBSCURE
#define CRYSTAL_OBSCURE

#include "Crystal.h"

//Standard
void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Subtraction(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Multiplication(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Division(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Modulo(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Power(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Equal(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Diffrence(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Less(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Greater(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Less_Equal(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_Greater_Equal(Crystal_Symbol* dest, Crystal_Symbol* source);
//Reverse Variants
void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_SubtractionR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_DivisionR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_ModuloR(Crystal_Symbol* dest, Crystal_Symbol* source);
void Obscure_PowerR(Crystal_Symbol* dest, Crystal_Symbol* source);

#endif