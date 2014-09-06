#include "Bytecode.h"

bool Bytecode::Execute(Crystal_Compiler* comp)
{
  return code_gen(comp, &base, &elements, &result);
}