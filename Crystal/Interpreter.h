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

  struct Package_Info {
    PACKAGE_TYPE pt;
    union{
      unsigned attributes;
      unsigned arguments;
    } info;
    void* function;
  };

  //Hidden constructor
  Crystal_Interpreter();

  //Populate the data for built in packages.
  void Populate_BIP();
  
  //Interpret Process under the hood
  void Format_Code();
  void Lookup_Packages();
  void Process_Code();
  void Process_Package(const char* code);
  unsigned Get_Precedence(const char*  sym);
  void Special_Processing(Crystal_Data* sym);

  //Private members
  Crystal_Compiler* comp;
  std::string code_cache;
  std::string code_out;
  Syntax_Tree stree;
  std::unordered_map<std::string, Package_Info> packages;
  std::unordered_map<std::string, Package_Info> built_in;
};

#endif