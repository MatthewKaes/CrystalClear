#ifndef CRYSTAL_IO
#define CRYSTAL_IO

#include "Marshal.h"

void Crystal_MarshalOpen(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_MarshalDump(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_MarshalLoad(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_MarshalClose(Crystal_Symbol* ret_sym);

void Crystal_WriteText(Crystal_Symbol* ret_sym, Crystal_Symbol* file, Crystal_Symbol* sym);
void Crystal_ReadText(Crystal_Symbol* ret_sym, Crystal_Symbol* file);

#endif