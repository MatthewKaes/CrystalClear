#ifndef CRYSTAL_CONTAINERS
#define CRYSTAL_CONTAINERS

#include "Function.h"

void Crystal_Contains(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* val);
void Crystal_Index(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* val);
void Crystal_Size(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Clone(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_Fork(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);

#endif