#include "Reductions.h"
#include "Lexicon.h"
#include <unordered_map>

#define REDUCTION(val, func) case val: \
    if(sym->str.c_str()[1] == '\0') \
    { \
       Reduce_##func(sym, left, right); \
       return true; \
    }
#define REDUCTION_EX(val, val2, func) case val: \
    if(sym->str.c_str()[1] == val2) \
    { \
       Reduce_##func(sym, left, right); \
       return true; \
    }
#define APPEND_REDUCTION(val, func) else if(sym->str.c_str()[1] == val) \
    { \
       Reduce_##func(sym, left, right); \
       return true; \
    }

bool Can_Reduce(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  static std::unordered_map<Data_Type, bool> reduceable;
  if(!reduceable.size())
  {
    reduceable[DAT_NIL] = true;
    reduceable[DAT_BOOL] = true;
    reduceable[DAT_INT] = true;
    reduceable[DAT_DOUBLE] = true;
    reduceable[DAT_STRING] = true;
  }

  if(sym->type != DAT_OP)
    return false;

  if(reduceable.find(left->type) == reduceable.end())
    return false;
  if(reduceable.find(right->type) == reduceable.end())
    return false;

  return true;
}

bool Reduction(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  if(!Can_Reduce(sym, left, right))
    return false;

  switch(sym->str.c_str()[0])
  { 
    REDUCTION('+', Addition)
    REDUCTION('-', Subtraction)
    REDUCTION('*', Multiplication)
    REDUCTION('^', Power)
    REDUCTION('<', Less)
    APPEND_REDUCTION('=', Less_Equal)
    REDUCTION('>', Greater)
    APPEND_REDUCTION('=', Greater_Equal)
    REDUCTION('/', Division)
    REDUCTION('%', Modulo)
    REDUCTION_EX('&', '&', And)
    REDUCTION_EX('=', '=', Equal)
    REDUCTION_EX('!', '=', Diffrence)
  }

  return false;
}

void Reduce_Addition(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_BOOL:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->b = left->b || right->b;
    break;
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->i32 = left->i32 + right->i32;
    break;
  case DAT_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->d = l + r;
    }
    break;
  case DAT_STRING:
    if(left->type == DAT_NIL)
    {
      sym->str.assign("nil");
      sym->str.append(right->str);
    }
    else if(left->type == DAT_BOOL)
    {
      sym->str.clear();
      b_to_str(left->b, &sym->str);
      sym->str.append(right->str);
    }
    else if(left->type == DAT_INT)
    {
      sym->str.clear();
      i_to_str(left->i32, &sym->str);
      sym->str.append(right->str);
    }
    else if(left->type == DAT_DOUBLE)
    {
      sym->str.clear();
      d_to_str(left->d, &sym->str);
      sym->str.append(right->str);
    }
    else
    {
      sym->str.assign(left->str);
      if(right->type == DAT_NIL)
      {
        sym->str.append("nil");
      }
      else if(right->type == DAT_BOOL)
      {
        std::string r_str;
        b_to_str(right->b, &r_str);
        sym->str.append(right->str);
      }
      else if(right->type == DAT_INT)
      {
        std::string r_str;
        i_to_str(right->i32, &r_str);
        sym->str.append(right->str);
      }
      else if(right->type == DAT_DOUBLE)
      {
        std::string r_str;
        d_to_str(right->d,  &r_str);
        sym->str.append(r_str);
      }
      else
      {
        sym->str.append(right->str);
      }
    } 
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}
void Reduce_Subtraction(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{  
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_BOOL:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->b = left->b ^ right->b;
    break;
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->i32 = left->i32 - right->i32;
    break;
  case DAT_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->d = l - r;
    }
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}

void Reduce_Multiplication(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_BOOL:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->b = !(left->b ^ right->b);
    break;
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->i32 = left->i32 * right->i32;
    break;
  case DAT_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->d = l * r;
    }
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}
void Reduce_Division(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL ||
       left->type == DAT_BOOL || right->type == DAT_BOOL)
      resolve = DAT_NIL;
    else
      sym->i32 = left->i32 / right->i32;
    break;
  case DAT_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->d = l / r;
    }
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}
void Reduce_Modulo(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{  
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL ||
       left->type == DAT_BOOL || right->type == DAT_BOOL)
      resolve = DAT_NIL;
    else
      sym->i32 = left->i32 % right->i32;
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}

void Reduce_Power(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case DAT_BOOL:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
      sym->b = !(left->b ^ right->b);
    break;
  case DAT_INT:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      sym->i32 = left->i32;
      for(int i = 1; i < right->i32; i++)
      {
        sym->i32 *= left->i32;
      }
    }
    break;
  case DAT_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      resolve = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->d = pow(l, r);
    }
    break;
  default:
    resolve = DAT_NIL;
  }
  sym->type = resolve;
}
void Reduce_And(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  int l, r;
  if(left->type == CRY_NIL)
    l = 0;
  else
    l = left->i32 != 0;
  if(right->type == CRY_NIL)
    r = 0;
  else
    r = right->i32 != 0;
  sym->i32 = l && r;
  sym->type = DAT_BOOL;
}
void Reduce_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  if(left->type != right->type)
  {
    sym->b = false;
  }
  else
  {
    Data_Type resolve = left->type > right->type ? left->type : right->type;
    switch(resolve)
    {
    case DAT_NIL:
      sym->b = true;
      break;
    case DAT_BOOL:
    case DAT_INT:
      sym->b = (left->i32 == right->i32);
      break;
    case DAT_DOUBLE:
      sym->b = (left->d == right->d);
      break;
    case DAT_STRING:
      sym->b = !left->str.compare(right->str);
      break;
    default:
      sym->b = false;
    }
  }
  sym->type = DAT_BOOL;
}
void Reduce_Diffrence(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  if(left->type != right->type)
  {
    sym->b = true;
  }
  else
  {
    Data_Type resolve = left->type > right->type ? left->type : right->type;
    switch(resolve)
    {
    case DAT_NIL:
      sym->b = false;
      break;
    case DAT_BOOL:
    case DAT_INT:
      sym->b = (left->i32 != right->i32);
      break;
    case DAT_DOUBLE:
      sym->b = (left->d != right->d);
    case DAT_STRING:
      sym->b = !left->str.compare(right->str);
      sym->b = !sym->b;
      break;
    default:
      sym->b = true;
    }
  }
  sym->type = DAT_BOOL;
}
void Reduce_Less(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case CRY_INT:
    if(left->type != right->type)
    {
      sym->type = DAT_NIL;
      return;
    }
    sym->b = (left->i32 < right->i32);
    sym->type = DAT_BOOL;
    return;
  case CRY_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      sym->type = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->b = (l < r);
      sym->type = DAT_BOOL;
    }
    return;
  default:
    sym->type = DAT_NIL;
    return;
  }
}
void Reduce_Less_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case CRY_INT:
    if(left->type != right->type)
    {
      sym->type = DAT_NIL;
      return;
    }
    sym->b = (left->i32 <= right->i32);
    sym->type = DAT_BOOL;
    return;
  case CRY_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      sym->type = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->b = (l <= r);
      sym->type = DAT_BOOL;
    }
    return;
  default:
    sym->type = DAT_NIL;
    return;
  }
}
void Reduce_Greater(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case CRY_INT:
    if(left->type != right->type)
    {
      sym->type = DAT_NIL;
      return;
    }
    sym->b = (left->i32 > right->i32);
    sym->type = DAT_BOOL;
    return;
  case CRY_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      sym->type = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->b = (l > r);
      sym->type = DAT_BOOL;
    }
    return;
  default:
    sym->type = DAT_NIL;
    return;
  }
}
void Reduce_Greater_Equal(Crystal_Data* sym, Crystal_Data* left, Crystal_Data* right)
{
  Data_Type resolve = left->type > right->type ? left->type : right->type;
  switch(resolve)
  {
  case CRY_INT:
    if(left->type != right->type)
    {
      sym->type = DAT_NIL;
      return;
    }
    sym->b = (left->i32 >= right->i32);
    sym->type = DAT_BOOL;
    return;
  case CRY_DOUBLE:
    if(left->type == DAT_NIL || right->type == DAT_NIL)
      sym->type = DAT_NIL;
    else
    {
      double l, r;
      l = left->type == DAT_DOUBLE ? left->d : left->i32;
      r = right->type == DAT_DOUBLE ? right->d : right->i32;
      sym->b = (l >= r);
      sym->type = DAT_BOOL;
    }
    return;
  default:
    sym->type = DAT_NIL;
    return;
  }
}