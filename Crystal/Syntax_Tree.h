#ifndef CRYSTAL_TREE
#define CRYSTAL_TREE

#include "Bytecode.h"
#include "Clarity.h"
#include <vector>

#define MAX_ARGS 6
#define LEFT_CHILD params[0]
#define RIGHT_CHILD params[index]

class Syntax_Tree;

class Syntax_Node {
public:
  Syntax_Node(std::vector<Syntax_Node*>* pool, Syntax_Tree* tree);
  void Process(Syntax_Node* node);
  bool Evaluate();
  void Remove();
  Crystal_Data* Acquire();
  void Finalize();

  void Force_Memory(Bytecode* code);

  int priority;

private:
  Syntax_Node* parent;
  Syntax_Node* params[MAX_ARGS];
  std::vector<Syntax_Node*>* pool_;

  Syntax_Tree* tree_;
  Crystal_Data sym;
  int index;
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
