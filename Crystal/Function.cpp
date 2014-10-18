#include "Function.h"
#include "Helper.h"

int Crystal_And(Crystal_Symbol* left, Crystal_Symbol* right)
{
  return Parse_Bool(left) && Parse_Bool(right);
}