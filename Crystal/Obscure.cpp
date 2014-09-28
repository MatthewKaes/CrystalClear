#include "Obscure.h"
#include "Helper.h"
#include "Library.h"

void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b || source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    dest->i32 = dest->i32 + source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      dest->d = dest->i32 + source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      dest->d += source->i32;
    }
    else
    {
      dest->d += source->d;
    }
    dest->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    Crystal_Text_Append(dest, source);
    dest->type = CRY_STRING;
    return;
  }
}
void Obscure_Subtraction(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b ^ source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    dest->i32 = dest->i32 - source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      dest->d = dest->i32 - source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      dest->d -= source->i32;
    }
    else
    {
      dest->d -= source->d;
    }
    dest->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    dest->type = CRY_NIL;
    return;
  }
}

void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b || source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    dest->i32 = dest->i32 + source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      dest->d = dest->i32 + source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      dest->d += source->i32;
    }
    else
    {
      dest->d += source->d;
    }
    dest->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    Crystal_Text_AppendR(dest, source);
    dest->type = CRY_STRING;
    return;
  }
}
void Obscure_SubtractionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b ^ source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    dest->i32 = source->i32 - dest->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      dest->d = source->d - dest->i32;
    }
    else if(source->type != CRY_DOUBLE)
    {
      dest->d = source->i32 - dest->d;
    }
    else
    {
      dest->d = source->d - dest->d;
    }
    dest->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    dest->type = CRY_NIL;
    return;
  }
}