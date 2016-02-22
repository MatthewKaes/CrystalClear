#include "Containers.h"
#include "Obscure.h"

void Crystal_Contains(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* val)
{
  if (sym->type == CRY_POINTER || sym->type == CRY_REFERENCE)
  {
    sym = sym->sym;
    if (sym->type == CRY_ARRAY)
    {

      for (unsigned i = 0; i < sym->size; i++)
      {
        if (Crystal_Compare(sym->sym + i, val))
        {
          ret_sym->i32 = true;
          ret_sym->type = CRY_BOOL;
          return;
        }
      }

      ret_sym->i32 = false;
      ret_sym->type = CRY_BOOL;
    }
    else
    {
      ret_sym->type = CRY_NIL;
    }
  }
  else
  {
    ret_sym->type = CRY_NIL;
  }
}

void Crystal_Index(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* val)
{
  if (sym->type == CRY_POINTER || sym->type == CRY_REFERENCE)
  {
    sym = sym->sym;
    if (sym->type == CRY_ARRAY)
    {
      for (unsigned i = 0; i < sym->size; i++)
      {
        if (Crystal_Compare(sym->sym + i, val))
        {
          ret_sym->i32 = (int)i;
          ret_sym->type = CRY_INT;
          return;
        }
      }
    }
  }
  ret_sym->type = CRY_NIL;
}

void Crystal_Size(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if (sym->type == CRY_POINTER || sym->type == CRY_REFERENCE)
  {
    ret_sym->i32 = sym->sym->size;
    ret_sym->type = CRY_INT;
  }
  else
  {
    ret_sym->type = CRY_NIL;
  }
}

void Crystal_Clone(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if (sym->type == CRY_POINTER)
  {
    if (sym->sym->type == CRY_ARRAY)
    {
      unsigned size = sym->sym->size;
      Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(malloc(sizeof(Crystal_Symbol) * size));
      memcpy(ary, sym->sym->sym, sizeof(Crystal_Symbol) * size);
      Construct_Array(ret_sym, size, sym->sym->capacity, ary);
    }
    else if (sym->sym->type == CRY_CLASS_OBJ)
    {
      Class_Info* klass = sym->sym->klass;
      unsigned size = sym->sym->size;
      Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(malloc(sizeof(Crystal_Symbol) * size));
      memcpy(ary, sym->sym->sym, sizeof(Crystal_Symbol) * size);

      Clone_Class(ret_sym, sym, ary);
    }
    else
    {
      unsigned size = sym->sym->size;
      char* str = reinterpret_cast<char*>(malloc(sizeof(char) * size));
      memcpy(str, sym->sym->sym, sizeof(Crystal_Symbol) * size);
      Construct_String(ret_sym, str, size);
    }
  }
  else if (sym->type == CRY_REFERENCE)
  {
    Crystal_Clone(ret_sym, sym->sym);
  }
  else
  {
    *ret_sym = *sym;
  }
}

void Crystal_Fork(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if (sym->type == CRY_POINTER)
  {
    if (sym->sym->type == CRY_ARRAY)
    {
      unsigned size = sym->sym->size;
      Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(malloc(sizeof(Crystal_Symbol) * size));

      for (unsigned i = 0; i < size; i++)
      {
        Crystal_Fork(ary + i, sym->sym->sym + i);
      }

      Construct_Array(ret_sym, size, sym->sym->capacity, ary);
    }
    else if (sym->sym->type == CRY_CLASS_OBJ)
    {
      Class_Info* klass = sym->sym->klass;
      unsigned size = sym->sym->size;
      Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(malloc(sizeof(Crystal_Symbol) * size));

      for (unsigned i = 0; i < size; i++)
      {
        Crystal_Fork(ary + i, sym->sym->sym + i);
      }

      Clone_Class(ret_sym, sym, ary);
    }
    else
    {
      unsigned size = sym->sym->size;
      char* str = reinterpret_cast<char*>(malloc(sizeof(char) * size));
      memcpy(str, sym->sym->sym, sizeof(Crystal_Symbol) * size);
      Construct_String(ret_sym, str, size);
    }
  }
  else if (sym->type == CRY_REFERENCE)
  {
    Crystal_Clone(ret_sym, sym->sym);
  }
  else
  {
    *ret_sym = *sym;
  }
}