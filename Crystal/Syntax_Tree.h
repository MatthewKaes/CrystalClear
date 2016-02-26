#ifndef CRYSTAL_TREE
#define CRYSTAL_TREE

#include "Bytecode.h"
#include "Clarity_Filter.h"
#include <vector>

#define LEFT_CHILD params[0]
#define RIGHT_CHILD params[index]

class Syntax_Tree;

class Syntax_Node {
public:
  Syntax_Node(std::vector<Syntax_Node*>* pool, Syntax_Tree* tree);
  void ProcessRoot();
  void Process(Syntax_Node* node);
  void Reduce();
  bool Evaluate();
  void Remove();
  Crystal_Data* Acquire();
  void Finalize();

  static bool Evaluable_Type(Crystal_Data* sym);
  static bool Assignment_Op(Crystal_Data* sym);

  unsigned priority;

private:
  void Process_Parameters(Bytecode* code, Syntax_Node* node);
  void Map_Parameters(Bytecode* code, Syntax_Node* node);
  void Clear_Registry(Syntax_Node* node);
  void Force_Memory(Bytecode* code);
  bool Evaluate_Children();

  Syntax_Node* parent;
  std::vector<Syntax_Node*> params;
  std::vector<Syntax_Node*>* pool_;

  Syntax_Tree* tree_;
  Crystal_Data sym;
  unsigned index;
  bool R_Assoc;
};

class Syntax_Tree {
public:
  Syntax_Tree();
  ~Syntax_Tree();
  Syntax_Node* Acquire_Node();
  void Process(Syntax_Node* node);
  bool Evaluate();
  void Reset();

  void Set_Root(Syntax_Node* new_root);

  std::vector<Bytecode>* Get_Bytecodes();
  std::vector<bool>* Get_Registers();
  unsigned Get_Depth();
  Syntax_Node* Get_Root();
  int Get_Open_Reg();

private:
  std::vector<Syntax_Node*> nodepool;
  Syntax_Node* root;
  std::vector<bool> registers;
  std::vector<Bytecode> bytecodes;
};

#endif
