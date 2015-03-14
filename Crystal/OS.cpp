#include "OS.h"
#include "Helper.h"
#include <cstdlib>

extern const char* CRY_ROOT;

void Crystal_CrystalCopyright(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_COPYRIGHT;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalVersion(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_VERSION;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalTarget(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_MACHINE;
  ret_sym->type = CRY_TEXT;
}

void Crystal_Environ(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string var;
  Parse_String(sym, &var);
  
  ret_sym->text = std::getenv(var.c_str());
  ret_sym->type = CRY_TEXT;
}

void Crystal_RuntimePath(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_ROOT;
  ret_sym->type = CRY_TEXT;
}