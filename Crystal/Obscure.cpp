#include "Obscure.h"
#include "Helper.h"
#include "Library.h"

void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
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
  default:
    dest->type = CRY_NIL;
    return;
  }
}
void Obscure_Subtraction(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b && source->b;
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
  default:
    dest->type = CRY_NIL;
    return;
  }
}
void Obscure_Multiplication(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = !(dest->b ^ source->b);
    dest->type = resolve;
    return;
  case CRY_INT:
    dest->i32 = dest->i32 * source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      dest->d = dest->i32 * source->d;
    else if(source->type != CRY_DOUBLE)
      dest->d *= source->i32;
    else
      dest->d *= source->d;
    dest->type = resolve;
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}
void Obscure_Power(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b ^ source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    {
      int temp = dest->i32;
      for(int i = 1; i < source->i32; i++)
      {
        dest->i32 *= temp;
      }
      dest->type = resolve;
    }
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      dest->d = pow(dest->i32,source->d);
    else if(source->type != CRY_DOUBLE)
      dest->d = pow(dest->d,source->i32);
    else
      dest->d = pow(dest->d,source->d);
    dest->type = resolve;
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}
void Obscure_Equal(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  switch(dest->type)
  {
  case CRY_NIL:
    dest->b = source->type == CRY_NIL;
    dest->type = CRY_BOOL;
    return;
  case CRY_BOOL:
    if(dest->type != source->type)
    {
      dest->b = false;
      return;
    }
    dest->b = (dest->b == source->b);
    dest->type = CRY_BOOL;
    return;
  case CRY_INT:
    if(dest->type != source->type)
    {
      dest->b = false;
      return;
    }
    dest->b = (dest->i32 == source->i32);
    dest->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type != source->type)
    {
      dest->b = false;
      dest->type = CRY_BOOL;
      return;
    }
    dest->b = (dest->d == source->d);
    dest->type = CRY_BOOL;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    dest->b = Fast_strcmp(dest, source);
    dest->type = CRY_BOOL;
    return;
  default:
    dest->b = false;
    dest->type = CRY_BOOL;
    return;
  }
}
void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
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
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
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
void Obscure_PowerR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->b = dest->b ^ source->b;
    dest->type = resolve;
    return;
  case CRY_INT:
    {
      int temp = dest->i32;
      dest->i32 = source->i32;
      for(int i = 1; i < temp; i++)
      {
        dest->i32 *= source->i32;
      }
    }
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      dest->d = pow(source->i32,dest->d);
    else if(source->type != CRY_DOUBLE)
      dest->d = pow(source->d,dest->i32);
    else
      dest->d = pow(source->d,dest->d);
    dest->type = resolve;
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}