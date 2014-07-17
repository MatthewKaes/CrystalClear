#ifndef CRYSTAL_TREE
#define CRYSTAL_TREE

#include "Generator.h"
#include "Clarity.h"
#include <vector>

#define MAX_ARGS 6
#define LEFT_CHILD params[0]
#define RIGHT_CHILD params[index]

class Syntax_Node {
public:
  Syntax_Node(std::vector<Syntax_Node*>* pool, Crystal_Compiler* compiler);
  void Process(Syntax_Node* node);
  bool Evaluate();
  void Remove();
  Crystal_Data* Acquire();
  void Finalize();

private:
  Syntax_Node* parent;
  Syntax_Node* params[MAX_ARGS];
  std::vector<Syntax_Node*>* pool_;
  Crystal_Compiler* compiler_;
  GENERATOR_CODE code_gen;

  Crystal_Data sym;
  int priority;
  int index;
};

class Syntax_Tree {
public:
  Syntax_Tree(Crystal_Compiler* compiler);
  ~Syntax_Tree();
  Syntax_Node* Acquire_Node();
  void Process(Syntax_Node* node);
  bool Evaluate();
private:
  std::vector<Syntax_Node*> nodepool;
  Syntax_Node* root;
  Crystal_Compiler* compiler_;
};

#endif
