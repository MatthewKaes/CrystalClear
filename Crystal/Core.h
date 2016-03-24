#ifndef CRYSTAL_CORE
#define CRYSTAL_CORE

#include "Crystal.h"

void Crystal_CrystalCopyright(Crystal_Symbol* ret_sym);
void Crystal_CrystalVersion(Crystal_Symbol* ret_sym);
void Crystal_CrystalLinker(Crystal_Symbol* ret_sym);
void Crystal_CrystalCompiler(Crystal_Symbol* ret_sym);
void Crystal_CrystalTarget(Crystal_Symbol* ret_sym);
void Crystal_Help(Crystal_Symbol* ret_sym, Crystal_Symbol* command);
void Crystal_Null_Init(Crystal_Symbol* ret_sym);

#endif