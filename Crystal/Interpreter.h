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

  enum PACKAGE_TYPE { PGK_EXE, PGK_OBJ };
  //Hidden constructor
  Crystal_Interpreter();

  //Interpret Process under the hood
  void Format_Code();
  void Lookup_Packages();
  void Process_Code();
  void Process_Package(const char* code);
  unsigned Get_Precedence(const char*  sym);

  //Private members
  Crystal_Compiler* comp;
  std::string code_cache;
  std::string code_out;
  Syntax_Tree stree;
  std::unordered_map<std::string, PACKAGE_TYPE> packages;
};

#endif