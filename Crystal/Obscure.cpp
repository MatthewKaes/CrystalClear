#include "Obscure.h"
#include "Helper.h"
#include "Native.h"
#include "Function.h"

void Obscure_Addition(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Crystal_Symbol* result = dest;
  Cry_Derefrence(&dest);
  Cry_Derefrence(&source);

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    result->i32 = dest->i32 || source->i32;
    result->type = resolve;
    return;
  case CRY_INT:   
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    dest->i32 = dest->i32 + source->i32;
    result->type = resolve; 
    return;
  case CRY_DOUBLE:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    if(result->type != CRY_DOUBLE)
    {
      result->d = dest->i32 + source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      result->d = dest->d + source->i32;
    }
    else
    {
      result->d = dest->d + source->d;
    }
    dest->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    Crystal_Text_Append_Loc(dest, source, result);
    return;
  case CRY_ARRAY:
    Crystal_Array_Append_Loc(dest, source, result);
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

  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    result->i32 = dest->i32 && source->i32;
    result->type = resolve;
    return;
  case CRY_INT:
    result->i32 = dest->i32 - source->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      result->d = dest->i32 - source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      result->d -= source->i32;
    }
    else
    {
      result->d -= source->d;
    }
    result->type = resolve;
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

  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    result->i32 = !(dest->i32 ^ source->i32);
    result->type = resolve;
    return;
  case CRY_INT:
    result->i32 = dest->i32 * source->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      result->d = dest->i32 * source->d;
    else if(source->type != CRY_DOUBLE)
      result->d *= source->i32;
    else
      result->d *= source->d;
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
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

  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    result->i32 = dest->i32 / source->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      result->d = dest->i32 / source->d;
    else if(source->type != CRY_DOUBLE)
      result->d /= source->i32;
    else
      result->d /= source->d;
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
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

  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    result->i32 %= source->i32;
    result->type = resolve;
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

  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    result->i32 = dest->i32 ^ source->i32;
    result->type = resolve;
    return;
  case CRY_INT:
    if(!source->i32)
    {
      result->i32 = 1;
    }
    else
    {
      result->i32 = dest->i32;
      for(int i = 1; i < source->i32; i++)
      {
        result->i32 *= dest->i32;
      }
    }
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      result->d = pow(dest->i32,source->d);
    else if(source->type != CRY_DOUBLE)
      result->d = pow(dest->d,source->i32);
    else
      result->d = pow(dest->d,source->d);
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
    return;
  }
}

void Obscure_Equal(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Crystal_Symbol* result = dest;
  Cry_Derefrence(&dest);
  Cry_Derefrence(&source);

  switch(dest->type)
  {
  case CRY_NIL:
    result->i32 = source->type == CRY_NIL;
    result->type = CRY_BOOL;
    return;
  case CRY_BOOL:
    if(dest->type != source->type)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
    result->i32 = (dest->b == source->b);
    result->type = CRY_BOOL;
    return;
  case CRY_INT:
    if(dest->type != source->type)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
    result->i32 = (dest->i32 == source->i32);
    result->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(result->type != source->type)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
    result->i32 = (dest->d == source->d);
    result->type = CRY_BOOL;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    if(source->type != CRY_TEXT && source->type != CRY_STRING)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
    result->i32 = Fast_strcmp(dest, source);
    result->type = CRY_BOOL;
    return;
  case CRY_CLASS_OBJ:
    if(dest->klass != source->klass)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
  case CRY_ARRAY:
    if(dest->type != source->type)
    {
      result->i32 = 0;
      result->type = CRY_BOOL;
      return;
    }
    result->i32 = Fast_arraycmp(dest, source);
    result->type = CRY_BOOL;
    return;
  default:
    result->i32 = 0;
    result->type = CRY_BOOL;
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
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  switch(dest->type)
  {
  case CRY_INT:
    if(dest->type != source->type)
    {
      result->type = CRY_NIL;
      return;
    }
    result->i32 = (dest->i32 < source->i32);
    result->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type > CRY_DOUBLE || source->type > CRY_DOUBLE)
    {
      result->type = CRY_NIL;
      return;
    }
    else
    {
      double l = dest->type == CRY_DOUBLE ? dest->d : dest->i32;
      double r = source->type == CRY_DOUBLE ? source->d : source->i32;
      result->i32 = l < r;
      result->type = CRY_BOOL;
    }
    return;
  default:
    result->type = CRY_NIL;
    return;
  }
}

void Obscure_Greater(Crystal_Symbol* dest, Crystal_Symbol* source)
{  
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  switch(dest->type)
  {
  case CRY_INT:
    if(dest->type != source->type)
    {
      result->type = CRY_NIL;
      return;
    }
    result->i32 = (dest->i32 > source->i32);
    result->type = CRY_BOOL;
    return;
  case CRY_DOUBLE:
    if(dest->type > CRY_DOUBLE || source->type > CRY_DOUBLE)
    {
      result->type = CRY_NIL;
      return;
    }
    else
    {
      double l = dest->type == CRY_DOUBLE ? dest->d : dest->i32;
      double r = source->type == CRY_DOUBLE ? source->d : source->i32;
      result->i32 = l > r;
      result->type = CRY_BOOL;
    }
    return;
  default:
    result->type = CRY_NIL;
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

void Obscure_Not(Crystal_Symbol* dest)
{
  if (dest->type == CRY_NIL)
    return;

  dest->type = CRY_BOOL;
  dest->i32 = !dest->i32;
}

//Reversals
void Obscure_AdditionR(Crystal_Symbol* dest, Crystal_Symbol* source)
{
  Crystal_Symbol* result = dest;
  Cry_Derefrence(&dest);
  Cry_Derefrence(&source);

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    result->i32 = dest->i32 || source->i32;
    result->type = resolve;
    return;
  case CRY_INT:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    result->i32 = dest->i32 + source->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:    
    if(dest->type == CRY_NIL || source->type == CRY_NIL)
    {
      result->type = CRY_NIL;
      return;
    }
    if(dest->type != CRY_DOUBLE)
    {
      result->d = dest->i32 + source->d;
    }
    else if(source->type != CRY_DOUBLE)
    {
      result->d = dest->d + source->i32;
    }
    else
    {
      result->d = dest->d + source->d;
    }
    result->type = resolve;
    return;
  case CRY_TEXT:
  case CRY_STRING:
    Crystal_Text_Append_Loc(source, dest, result);
    return;
  case CRY_ARRAY:
    Crystal_Array_Append_Loc(source, dest, result);
    return;
  default:
    result->type = CRY_NIL;
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
  
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    result->b = dest->b ^ source->b;
    result->type = resolve;
    return;
  case CRY_INT:
    result->i32 = source->i32 - dest->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
    {
      result->d = source->d - dest->i32;
    }
    else if(source->type != CRY_DOUBLE)
    {
      result->d = source->i32 - dest->d;
    }
    else
    {
      result->d = source->d - dest->d;
    }
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
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
  
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    result->i32 = source->i32 / dest->i32;
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      result->d = source->i32 / dest->d;
    else if(source->type != CRY_DOUBLE)
      result->d = source->d / dest->i32;
    else
      result->d = source->d / dest->d;
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
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
  
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_INT:
    result->i32 = source->i32 % dest->i32;
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
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
  
  Crystal_Symbol* result = dest;
  dest = dest->type != CRY_REFERENCE ? dest : dest->sym; 
  source = source->type != CRY_REFERENCE ? source : source->sym; 

  Symbol_Type resolve = dest->type > source->type ? dest->type : source->type;
  switch(resolve)
  {
  case CRY_BOOL:
    result->i32 = dest->i32 ^ source->i32;
    result->type = resolve;
    return;
  case CRY_INT:
    if(!dest->i32)
    {
      result->i32 = 1;
    }
    else
    {
      result->i32 = source->i32;
      for(int i = 1; i < dest->i32; i++)
      {
        result->i32 *= source->i32;
      }
    }
    result->type = resolve;
    return;
  case CRY_DOUBLE:
    if(dest->type != CRY_DOUBLE)
      result->d = pow(source->i32,dest->d);
    else if(source->type != CRY_DOUBLE)
      result->d = pow(source->d,dest->i32);
    else
      result->d = pow(source->d,dest->d);
    result->type = resolve;
    return;
  default:
    result->type = CRY_NIL;
    return;
  }
}
