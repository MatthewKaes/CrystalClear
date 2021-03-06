#ifndef CRYSTAL_FILESYSTEM
#define CRYSTAL_FILESYSTEM

#include "Crystal.h"

void Crystal_MakeDirectory(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_RemovePath(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_CopyPath(Crystal_Symbol* ret_sym, Crystal_Symbol* to, Crystal_Symbol* from);
void Crystal_PathExists(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_IsFile(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_IsDir(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_FileSize(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_FileList(Crystal_Symbol* ret_sym, Crystal_Symbol* sym);
void Crystal_CryRoot(Crystal_Symbol* ret_sym);

#endif