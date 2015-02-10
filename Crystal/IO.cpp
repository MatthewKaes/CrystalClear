#include "IO.h"
#include <stdio.h>
#include "Helper.h"

static Marshaler global_marshal;
extern const char* CRY_ROOT;

void Crystal_MarshalOpen(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string path;
  Parse_String(sym, &path);

  if(path.c_str()[0] == '\0' || path.c_str()[1] != ':')
  {
    path.insert(0, CRY_ROOT);
  }

  global_marshal.Open(path.c_str());
  
  ret_sym->type = CRY_NIL;
}

void Crystal_MarshalDump(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  global_marshal.Dump(sym);

  ret_sym->type = CRY_NIL;
}

void Crystal_MarshalLoad(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string path;
  Parse_String(sym, &path);
  
  if(path.c_str()[0] == '\0' || path.c_str()[1] != ':')
  {
    path.insert(0, CRY_ROOT);
  }

  global_marshal.Load(path.c_str(), ret_sym);
}

void Crystal_MarshalClose(Crystal_Symbol* ret_sym)
{
  global_marshal.Close();

  ret_sym->type = CRY_NIL;
}