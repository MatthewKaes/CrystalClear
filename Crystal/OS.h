#ifndef CRYSTAL_OS
#define CRYSTAL_OS

#include "Crystal.h"

void Crystal_CrystalCopyright(Crystal_Symbol* ret_sym);
void Crystal_CrystalVersion(Crystal_Symbol* ret_sym);
void Crystal_CrystalTarget(Crystal_Symbol* ret_sym);

void Crystal_Environ(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);

#endif
