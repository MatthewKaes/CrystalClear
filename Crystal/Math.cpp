#include "Math.h"
#include "Helper.h"
#include "Function.h"

void Crystal_Cos(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = cos(Parse_Double(sym));
}

void Crystal_Sin(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = sin(Parse_Double(sym));
}

void Crystal_Log(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = log10(Parse_Double(sym));
}

void Crystal_NatrualLog(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = log(Parse_Double(sym));
}

void Crystal_Abs(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if(sym->type == CRY_DOUBLE)
  {
    ret_sym->d =  abs(sym->d);
  }
  else if(sym->type == CRY_INT)
  {
    ret_sym->i32 = abs(sym->i32);
  }
  else
  {
    ret_sym->d = abs(Parse_Double(sym));
  }
}

void Crystal_Sum(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;

  if(sym->type != CRY_POINTER && sym->sym->type != CRY_ARRAY)
  {
    ret_sym->d = Parse_Double(sym);
    return;
  }

  Crystal_Symbol* ary = sym->sym;
  ret_sym->d = 0;
  for(unsigned i = 0; i < ary->size; i++)
  {
    ret_sym->d += Parse_Double(ary->sym + i);
  }
}

void Crystal_Mean(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;

  if(sym->type != CRY_POINTER && sym->sym->type != CRY_ARRAY)
  {
    ret_sym->d = Parse_Double(sym);
    return;
  }

  Crystal_Symbol* ary = sym->sym;
  ret_sym->d = 0;
  for(unsigned i = 0; i < ary->size; i++)
  {
    ret_sym->d += Parse_Double(ary->sym + i);
  }
  ret_sym->d /= ary->size;
}

void Crystal_Min(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if(sym->type != CRY_POINTER && sym->sym->type != CRY_ARRAY)
  {
    ret_sym->type = CRY_NIL;
    return;
  }

  Crystal_Symbol* ary = sym->sym;
  if(ary->size == 0)
  {
    ret_sym->type = CRY_NIL;
    return;
  }

  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = Parse_Double(ary->sym);

  for(unsigned i = 1; i < ary->size; i++)
  {
    double val = Parse_Double(ary->sym + i);
    if(val < ret_sym->d)
    {
      ret_sym->d = val;
    }
  }
}

void Crystal_Max(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if(sym->type != CRY_POINTER && sym->sym->type != CRY_ARRAY)
  {
    ret_sym->type = CRY_NIL;
    return;
  }

  Crystal_Symbol* ary = sym->sym;
  if(ary->size == 0)
  {
    ret_sym->type = CRY_NIL;
    return;
  }

  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = Parse_Double(ary->sym);

  for(unsigned i = 1; i < ary->size; i++)
  {
    double val = Parse_Double(ary->sym + i);
    if(val > ret_sym->d)
    {
      ret_sym->d = val;
    }
  }
}

void Crystal_Var(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;

  if(sym->type != CRY_POINTER && sym->sym->type != CRY_ARRAY)
  {
    ret_sym->d = Parse_Double(sym);
    return;
  }

  Crystal_Symbol* ary = sym->sym;
  double mean = 0;
  for(unsigned i = 0; i < ary->size; i++)
  {
    mean += Parse_Double(ary->sym + i);
  }
  mean /= ary->size;

  ret_sym->d = 0;
  for(unsigned i = 0; i < ary->size; i++)
  {
    ret_sym->d += pow(Parse_Double(ary->sym + i) - mean, 2);
  }
  ret_sym->d /= ary->size;
}

void Crystal_Sd(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  Crystal_Var(ret_sym, sym);

  ret_sym->d = sqrt(ret_sym->d);
}

void Crystal_Step(Crystal_Symbol* ret_sym, Crystal_Symbol* start, Crystal_Symbol* end, Crystal_Symbol* step)
{
  int start_v = Parse_Int(start);
  int end_v = Parse_Int(end);
  int step_v = Parse_Int(step);
  int index = 0;

  if(start_v < end_v && step_v > 0)
  {
    unsigned size = (end_v - start_v) / step_v + 1;
    Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(calloc(size, sizeof(Crystal_Symbol)));

    for(int i = start_v; i <= end_v; i += step_v)
    {
      ary[index].type = CRY_INT;
      ary[index].i32 = i;
      index++;
    }

    Construct_Array(ret_sym, size, size, ary);
  }
  else if(start_v > end_v && step_v < 0)
  {
    unsigned size = (end_v - start_v) / step_v + 1;
    Crystal_Symbol* ary = reinterpret_cast<Crystal_Symbol*>(calloc(size, sizeof(Crystal_Symbol)));

    for(int i = start_v; i >= end_v; i += step_v)
    {
      ary[index].type = CRY_INT;
      ary[index].i32 = i;
      index++;
    }

    Construct_Array(ret_sym, size, size, ary);
  }
  else
  {
    ret_sym->type = CRY_NIL;
  }
}