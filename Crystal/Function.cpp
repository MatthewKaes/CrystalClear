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
  
  //Garbage Collect the old string
  Construct_String(symd, new_buffer, val_left.size() + val_right.size());
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

  //Garbage Collect the old string
  Construct_String(symd, new_buffer, val_left.size() + val_right.size());
}

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  //Garbage Collect the old string
  char* new_buffer = static_cast<char*>(malloc(symd->ptr.sym->size + length + 1));
  strcpy(new_buffer, symd->ptr.sym->ptr.str);
  strcpy(new_buffer + symd->ptr.sym->size, str);
  free(symd->ptr.sym->ptr.str);
  symd->ptr.sym->ptr.str = new_buffer;
  symd->ptr.sym->size = symd->ptr.sym->size + length;
}
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->ptr.sym->size + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, symd->ptr.sym->ptr.str);
  free(symd->ptr.sym->ptr.str);
  symd->ptr.sym->ptr.str = new_buffer;
  symd->ptr.sym->size = symd->ptr.sym->size + length;
}

void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), str);
  
  //Garbage Collect the old string
  Construct_String(symd, new_buffer, val_left.size() + length);
}
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, val_left.c_str());
  
  //Garbage Collect the old string
  Construct_String(symd, new_buffer, val_left.size() + length);
}
void Construct_Array(Crystal_Symbol* symd, unsigned size, Crystal_Symbol* ary)
{
  Garbage_Collection(symd);

  symd->ptr.sym = reinterpret_cast<Crystal_Symbol*>(calloc(1, sizeof(Crystal_Symbol)));
  symd->ptr.sym->type = CRY_ARRAY;
  symd->ptr.sym->size = size;
  symd->ptr.sym->ptr.sym = ary;
  symd->ptr.sym->ref_cnt = 1;
  if(size < 0x20)
    symd->ptr.sym->capacity = 0x20;
  else
    symd->ptr.sym->capacity = size;
  symd->type = CRY_POINTER;
}
void Construct_Range(Crystal_Symbol* symd, int left, int right)
{
  Garbage_Collection(symd);

  int size = right - left;
  if(size < 0)
    size = 0;

  Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(calloc(size, sizeof(Crystal_Symbol)));
  for(int size = left; size < right; size++)
  {
    ary[size - left].type = CRY_INT;
    ary[size - left].i32 = size;
  }
  symd->ptr.sym = reinterpret_cast<Crystal_Symbol*>(calloc(1, sizeof(Crystal_Symbol)));
  symd->ptr.sym->type = CRY_ARRAY;
  symd->ptr.sym->size = size;
  symd->ptr.sym->ptr.sym = ary;
  symd->ptr.sym->ref_cnt = 1;
  if(size < 0x20)
    symd->ptr.sym->capacity = 0x20;
  else
    symd->ptr.sym->capacity = size;
  symd->type = CRY_POINTER;
}
void Construct_String(Crystal_Symbol* symd,  char* str, unsigned size)
{
  Garbage_Collection(symd);

  symd->ptr.sym = reinterpret_cast<Crystal_Symbol*>(calloc(1, sizeof(Crystal_Symbol)));
  symd->ptr.sym->type = CRY_STRING;
  symd->ptr.sym->size = size;
  symd->ptr.sym->ptr.str = str;
  symd->ptr.sym->ref_cnt = 1;
  symd->type = CRY_POINTER;
}
