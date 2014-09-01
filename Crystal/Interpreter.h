#ifndef CRYSTAL_INTERPRETER
#define CRYSTAL_INTERPRETER

#include "Compiler.h"
#include "Lexicon.h"
#include "Syntax_Tree.h"

class Crystal_Interpreter
{
public:
  Crystal_Interpreter(Crystal_Compiler* compiler);
  ~Crystal_Interpreter();
  
  void Cache_Code(const char* filename);
  void Interpret();

private:
  //Hidden constructor
  Crystal_Interpreter();

  //Interpret Process under the hood
  void Format_Code();
  void Process_Code();
  void Process_Package();

  //Private members
  Crystal_Compiler* comp;
  std::string code_cache;
  Syntax_Tree stree;
};

#endif