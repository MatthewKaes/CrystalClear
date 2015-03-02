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

  static std::vector<HINSTANCE> Extension_Libs;

private:

  //Hidden constructor
  Crystal_Interpreter();

  //Populate the data for built in packages.
  void Populate_BIP();
  
  //Interpret Process under the hood
  void Format_Code();
  void Process_Lookups();
  void Process_Logic();
  void Process_Package(const char* code);


  //Processing symbols
  void Special_Processing(Crystal_Data* sym);
  void Lookup_Processing(Crystal_Data* sym, std::unordered_map<std::string, unsigned>* local_map);

  unsigned Late_Binding(Crystal_Data* sym);

  //Private members
  Crystal_Compiler* comp;
  std::string code_cache;
  std::string code_out;
  Syntax_Tree stree;
  std::unordered_map<std::string, Package_Info> packages;
  std::unordered_map<std::string, Package_Info> built_in;
};

#endif