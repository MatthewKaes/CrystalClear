#include "Function.h"
#include "Helper.h"
#include "Garbage_Collector.h"

extern std::vector<Class_Info*> Class_Listing;

bool Crystal_Compare(Crystal_Symbol* left, Crystal_Symbol* right)
{
  Cry_Derefrence(&left);
  Cry_Derefrence(&right);

  switch (left->type)
  {
  case CRY_NIL:
    return right->type == CRY_NIL;
  case CRY_BOOL:
    if (right->type != left->type)
    {
      return false;
    }
    return (right->b == left->b);
  case CRY_INT:
    if (right->type != left->type)
    {
      return false;
    }
    return (right->i32 == left->i32);
  case CRY_DOUBLE:
    if (right->type != left->type)
    {
      return false;
    }
    return (right->d == left->d);
  case CRY_TEXT:
  case CRY_STRING:
    if (right->type != CRY_TEXT && right->type != CRY_STRING)
    {
      return false;
    }
    return Fast_strcmp(right, left);
  case CRY_CLASS_OBJ:
    if (right->klass != left->klass)
    {
      return false;
    }
  case CRY_ARRAY:
    if (right->type != left->type)
    {
      return false;
    }
    return Fast_arraycmp(right, left) == 1;
  default:
    return false;
  }
}

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

void Crystal_Text_Append_Rev(Crystal_Symbol* symd, Crystal_Symbol* syms)
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

void Crystal_Text_Append_Loc(Crystal_Symbol* symd, Crystal_Symbol* syms, Crystal_Symbol* result)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), val_right.c_str());
  
  // Create a new Garbage Controled string.
  Construct_String(result, new_buffer, val_left.size() + val_right.size());
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
  if(symd->type == CRY_POINTER && symd->sym->type == CRY_ARRAY)
  {
    if(symd->sym->size + 1 >= symd->sym->capacity)
    {
      symd->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(symd->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary, symd->sym->sym, sizeof(Crystal_Symbol) * symd->sym->size);
    new_ary[symd->sym->size] = *syms;

    Construct_Array(symd, symd->sym->size + 1, symd->sym->capacity, new_ary);
  }
  else
  {
    if(syms->sym->size + 1 >= syms->sym->capacity)
    {
      syms->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(syms->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary + 1, syms->sym->sym, sizeof(Crystal_Symbol) * syms->sym->size);
    new_ary[0] = *symd;

    Construct_Array(symd, syms->sym->size + 1, syms->sym->capacity, new_ary);
  }
}

void Crystal_Array_Append_Rev(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  if(syms->type == CRY_POINTER && syms->sym->type == CRY_ARRAY)
  {
    if(syms->sym->size + 1 >= syms->sym->capacity)
    {
      syms->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(syms->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary, syms->sym->sym, sizeof(Crystal_Symbol) * syms->sym->size);
    new_ary[symd->sym->size] = *symd;

    Construct_Array(symd, syms->sym->size + 1, syms->sym->capacity, new_ary);
  }
  else
  {
    if(symd->sym->size + 1 >= symd->sym->capacity)
    {
      symd->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(symd->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary + 1, symd->sym->sym, sizeof(Crystal_Symbol) * symd->sym->size);
    new_ary[0] = *syms;

    Construct_Array(symd, symd->sym->size + 1, symd->sym->capacity, new_ary);
  }
}

void Crystal_Array_Append_Loc(Crystal_Symbol* symd, Crystal_Symbol* syms, Crystal_Symbol* result)
{
  if(symd->type == CRY_POINTER && symd->sym->type == CRY_ARRAY)
  {
    if(symd->sym->size + 1 >= symd->sym->capacity)
    {
      symd->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(symd->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary, symd->sym->sym, sizeof(Crystal_Symbol) * symd->sym->size);
    new_ary[symd->sym->size] = *syms;

    Construct_Array(result, symd->sym->size + 1, symd->sym->capacity, new_ary);
  }
  else
  {
    if(syms->sym->size + 1 >= syms->sym->capacity)
    {
      syms->sym->capacity *= 2;
    }

    Crystal_Symbol* new_ary = reinterpret_cast<Crystal_Symbol*>(calloc(syms->sym->capacity, sizeof(Crystal_Symbol)));
    
    memcpy(new_ary + 1, syms->sym->sym, sizeof(Crystal_Symbol) * syms->sym->size);
    new_ary[0] = *symd;

    Construct_Array(result, syms->sym->size + 1, syms->sym->capacity, new_ary);
  }
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

void Construct_Class(int id, Crystal_Symbol* symd)
{
  Class_Info* klass = Class_Listing[id];
  symd->sym = GC_Allocate();
  symd->sym->type = CRY_CLASS_OBJ;
  symd->sym->sym = reinterpret_cast<Crystal_Symbol*>(calloc(klass->attributes.size(), sizeof(Crystal_Symbol)));
  symd->sym->size = klass->attributes.size();
  symd->sym->klass = klass;
  symd->type = CRY_POINTER;
}

void Clone_Class(Crystal_Symbol* symd, Crystal_Symbol* sym, Crystal_Symbol* data)
{
  symd->sym = GC_Allocate();
  symd->sym->type = CRY_CLASS_OBJ;
  *symd->sym = *sym->sym;
  symd->sym->sym = data;
  symd->type = CRY_POINTER;
}
