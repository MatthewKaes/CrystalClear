#include "Obscure.h"
#include "Helper.h"
#include "Library.h"
#include "Function.h"

void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    dest->i32 = dest->i32 || source->i32;
    dest->type = resolve;
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
    return;
  case CRY_INT:
    dest->i32 = dest->i32 + source->i32;
    dest->type = resolve;    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
    return;
  case CRY_DOUBLE:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
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
    Crystal_Text_Append(dest, source);
    return;
  case CRY_POINTER:
    if((dest->type == CRY_POINTER && dest->sym->type == CRY_STRING) ||
       (source->type == CRY_POINTER && source->sym->type == CRY_STRING))
    {
      Crystal_Text_Append(dest, source);
      return;
    }
    else
    {
      Crystal_Array_Append(dest, source);
      return;
    }
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
    dest->i32 = dest->i32 && source->i32;
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
    dest->i32 = !(dest->i32 ^ source->i32);
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

void Obscure_Division(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    dest->i32 = dest->i32 / source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      dest->d = dest->i32 / source->d;
    else if(source->type != CRY_DOUBLE)
      dest->d /= source->i32;
    else
      dest->d /= source->d;
    dest->type = resolve;
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}

void Obscure_Modulo(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    dest->i32 %= source->i32;
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
    dest->i32 = dest->i32 ^ source->i32;
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
    dest->i32 = source->type == CRY_NIL;
    dest->type = CRY_BOOL;
    return;
  case CRY_BOOL:
    if(dest->type != source->type)
    {
      dest->i32 = 0;
      dest->type = CRY_BOOL;
      return;
    }
    dest->i32 = (dest->b == source->b);
    dest->type = CRY_BOOL;
    return;
  case CRY_INT:
    if(dest->type != source->type)
    {
      dest->i32 = 0;
      dest->type = CRY_BOOL;
      return;
    }
    dest->i32 = (dest->i32 == source->i32);
    dest->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type != source->type)
    {
      dest->i32 = 0;
      dest->type = CRY_BOOL;
      return;
    }
    dest->i32 = (dest->d == source->d);
    dest->type = CRY_BOOL;
    return;
  case CRY_TEXT:
    if(source->type == CRY_TEXT)
    {
      dest->i32 = Fast_strcmp(dest, source);
      dest->type = CRY_BOOL;
      return;
    }
    else if(source->type == CRY_POINTER && source->sym->type == CRY_STRING)
    {
      dest->i32 = Fast_strcmp(dest, source->sym);
      dest->type = CRY_BOOL;
      return;
    }
    dest->i32 = 0;
    dest->type = CRY_BOOL;
    return;
  case CRY_POINTER:
    if(source->type == CRY_TEXT)
    {
      if(dest->sym->type != CRY_STRING)
      {
        dest->i32 = 0;
        dest->type = CRY_BOOL;
        return;
      }
      dest->i32 = Fast_strcmp(dest->sym, source);
      dest->type = CRY_BOOL;
      return;
    }
    else if(source->type != CRY_POINTER || source->sym->type != dest->sym->type)
    {
      dest->i32 = 0;
      dest->type = CRY_BOOL;
      return;
    }

    if(dest->sym == source->sym)
    {
      dest->i32 = 1;
      dest->type = CRY_BOOL;
      return;
    }

    if(dest->sym->type == CRY_STRING)
    {
      dest->i32 = Fast_strcmp(dest->sym, source->sym);
      dest->type = CRY_BOOL;
      return;
    }
    else
    {
      dest->i32 = Fast_arraycmp(dest->sym, source->sym);
      dest->type = CRY_BOOL;
      return;
    }
    break;

  default:
    dest->i32 = 0;
    dest->type = CRY_BOOL;
    return;
  }
}

void Obscure_Diffrence(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  Obscure_Equal(dest, source);
  dest->i32 = !dest->i32;
}

void Obscure_Less(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  switch(dest->type)
  {
  case CRY_INT:
    if(dest->type != source->type)
    {
      dest->type = CRY_NIL;
      return;
    }
    dest->i32 = (dest->i32 < source->i32);
    dest->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type > CRY_DOUBLE || source->type > CRY_DOUBLE)
    {
      dest->type = CRY_NIL;
      return;
    }
    else
    {
      double l = dest->type == CRY_DOUBLE ? dest->d : dest->i32;
      double r = source->type == CRY_DOUBLE ? source->d : source->i32;
      dest->i32 = l < r;
      dest->type = CRY_BOOL;
    }
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}

void Obscure_Greater(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  switch(dest->type)
  {
  case CRY_INT:
    if(dest->type != source->type)
    {
      dest->type = CRY_NIL;
      return;
    }
    dest->i32 = (dest->i32 > source->i32);
    dest->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type > CRY_DOUBLE || source->type > CRY_DOUBLE)
    {
      dest->type = CRY_NIL;
      return;
    }
    else
    {
      double l = dest->type == CRY_DOUBLE ? dest->d : dest->i32;
      double r = source->type == CRY_DOUBLE ? source->d : source->i32;
      dest->i32 = l > r;
      dest->type = CRY_BOOL;
    }
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}

void Obscure_Less_Equal(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  Obscure_Greater(dest, source);
  dest->i32 = !dest->i32;
}

void Obscure_Greater_Equal(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  Obscure_Less(dest, source);
  dest->i32 = !dest->i32;
}

//Reversals
void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
    dest->i32 = dest->i32 || source->i32;
    dest->type = resolve;
    return;
  case CRY_INT:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
    dest->i32 = dest->i32 + source->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      dest->type = CRY_NIL;
      return;
    }
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
    Crystal_Text_AppendR(dest, source);
    return;
  case CRY_POINTER:
    if((dest->type == CRY_POINTER && dest->sym->type == CRY_STRING) ||
       (source->type == CRY_POINTER && source->sym->type == CRY_STRING))
    {
      Crystal_Text_AppendR(dest, source);
      return;
    }
    else
    {
      Crystal_Array_AppendR(dest, source);
    }
  default:
    dest->type = CRY_NIL;
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

void Obscure_DivisionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    dest->i32 = source->i32 / dest->i32;
    dest->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      dest->d = source->i32 / dest->d;
    else if(source->type != CRY_DOUBLE)
      dest->d = source->d / dest->i32;
    else
      dest->d = source->d / dest->d;
    dest->type = resolve;
    return;
  default:
    dest->type = CRY_NIL;
    return;
  }
}

void Obscure_ModuloR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  if(dest->type == CRY_NIL || source->type == CRY_NIL)
  {
    dest->type = CRY_NIL;
    return;
  }
  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    dest->i32 = source->i32 % dest->i32;
    dest->type = resolve;
    return;
  default:
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
    dest->i32 = dest->i32 ^ source->i32;
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