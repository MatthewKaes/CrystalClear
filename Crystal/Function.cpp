#include "Function.h"
#include "Helper.h"

int Crystal_And(Crystal_Symbol* left, Crystal_Symbol* right)
{
  return Parse_Bool(left) && Parse_Bool(right);
}
int Crystal_Or(Crystal_Symbol* left, Crystal_Symbol* right)
{
  return Parse_Bool(left) || Parse_Bool(right);
}
void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), val_right.c_str());
  if(symd->ptr.str)
    free(symd->ptr.str);
  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + val_right.size() + 1;
}
void Crystal_Text_AppendR(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_right.c_str());
  strcpy(new_buffer + val_right.size(), val_left.c_str());
  if(symd->ptr.str)
    free(symd->ptr.str);
  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + val_right.size() + 1;
}

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length + 1));
  strcpy(new_buffer, symd->ptr.str);
  strcpy(new_buffer + strlen(symd->ptr.str), str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, symd->ptr.str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}

void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), str);

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length;
}
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, val_left.c_str());

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length;
}