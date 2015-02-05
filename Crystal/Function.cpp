#include "Function.h"
#include "Helper.h"
#include "Garbage_Collector.h"

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
  
  // Create a new Garbage Controled string.
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
  
  // Create a new Garbage Controled string.
  Construct_String(symd, new_buffer, val_left.size() + val_right.size());
}

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->sym->size + length + 1));
  strcpy(new_buffer, symd->sym->str);
  strcpy(new_buffer + symd->sym->size, str);
  
  // Create a new Garbage Controled string.
  Construct_String(symd, new_buffer, symd->sym->size + length);
}

void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->sym->size + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, symd->sym->str);
  
  // Create a new Garbage Controled string.
  Construct_String(symd, new_buffer, symd->sym->size + length);
}

void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), str);
  
  // Create a new Garbage Controled string.
  Construct_String(symd, new_buffer, val_left.size() + length);
}

void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, val_left.c_str());
  
  // Create a new Garbage Controled string.
  Construct_String(symd, new_buffer, val_left.size() + length);
}

void Crystal_Array_Append(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  if(symd->sym->size + 1 < symd->sym->capacity)
  {
    symd->sym->capacity *= 2;
  }

  Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(symd->sym->capacity, sizeof(Crystal_Symbol)));
    
  memcpy(new_ary, symd->sym->sym, sizeof(Crystal_Symbol) * symd->sym->size);
  new_ary[symd->sym->size] = *syms;

  Construct_Array(symd, symd->sym->size + 1, symd->sym->capacity, new_ary);

}

void Construct_Array(Crystal_Symbol* symd, unsigned size, unsigned capacity, Crystal_Symbol* ary)
{
  symd->sym = GC_Allocate();
  symd->sym->type = CRY_ARRAY;
  symd->sym->size = size;
  symd->sym->sym = ary;
  symd->sym->capacity = capacity;
  symd->type = CRY_POINTER;
}

void Construct_Range(Crystal_Symbol* symd, int left, int right)
{
  int size = right - left;
  int capacity = right - left;

  if(size < 0)
    size = 0;

  if(size * 2 < 0x20)
    capacity = 0x20;
  else
    capacity = size * 2;

  Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(calloc(capacity, sizeof(Crystal_Symbol)));
  for(int size = left; size < right; size++)
  {
    ary[size - left].type = CRY_INT;
    ary[size - left].i32 = size;
  }

  symd->sym = GC_Allocate();
  symd->sym->type = CRY_ARRAY;
  symd->sym->size = size;
  symd->sym->capacity = capacity;
  symd->sym->sym = ary;
  symd->type = CRY_POINTER;
}

void Construct_String(Crystal_Symbol* symd,  char* str, unsigned size)
{
  symd->sym = GC_Allocate();
  symd->sym->type = CRY_STRING;
  symd->sym->size = size;
  symd->sym->str = str;
  symd->type = CRY_POINTER;
}
