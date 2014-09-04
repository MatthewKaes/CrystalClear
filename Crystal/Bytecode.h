#ifndef CRYSTAL_BYTECODE
#define CRYSTAL_BYTECODE

#include "Generator.h"

class Bytecode{
public:
  bool Execute(Crystal_Compiler* comp);

  Crystal_Data result;
  std::vector<Crystal_Data> elements;
  GENERATOR_CODE code_gen;
};

#endif