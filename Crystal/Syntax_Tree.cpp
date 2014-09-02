#include "Syntax_Tree.h"

Syntax_Node::Syntax_Node(std::vector<Syntax_Node*>* pool, Syntax_Tree* tree)
{
  tree_ = tree;
  pool_ = pool;
}
void Syntax_Node::Process(Syntax_Node* node)
{
  if(node->priority > priority)
  {
    if(RIGHT_CHILD)
    {
      RIGHT_CHILD->Process(node);
    }
    else
    {
      RIGHT_CHILD = node;
      node->parent = this;
    }
  }
  else
  {
    node->LEFT_CHILD = this;
    node->parent = parent;
    if(parent)
      parent->RIGHT_CHILD = node;
    parent = node;
  }
}
bool Syntax_Node::Evaluate()
{  
  bool evaluation;
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      evaluation = true;
      if(!params[i]->Evaluate())
      {
        return false;
      }
    }
  }
  if(!evaluation)
    return true;

  Bytecode new_code;
  new_code.code_gen = Resolve_Genorator(&sym);

  std::vector<bool>* regptr = tree_->Get_Registers();
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      new_code.elements.push_back(*params[i]->Acquire());
      if(params[i]->Acquire()->type == DAT_REGISTRY)
        (*regptr)[i] = false;
    }
  }

  sym.type = DAT_REGISTRY;
  sym.i32 = -1;
  for(unsigned i = 0; i < regptr->size(); i++)
  {
    if(!(*regptr)[i])
    {
      sym.i32 = static_cast<unsigned>(i);
      (*regptr)[i] = true;
      break;
    }
  }
  if(sym.i32 == -1)
  {
    sym.i32 = regptr->size();
    regptr->push_back(true);
  }
  return true;
}
void Syntax_Node::Remove()
{
  for(int i = 0; i < MAX_ARGS; i++)
    if(params[i])
    {
      params[i]->Remove();
      params[i] = NULL;
    }
  pool_->push_back(this);
}
Crystal_Data* Syntax_Node::Acquire()
{
  return &sym;
}
void Syntax_Node::Finalize()
{  
  parent = NULL;
  for(int i = 0; i < MAX_ARGS; i++)
  {
    params[i] = NULL;
  }

  if(sym.type == DAT_FUNCTION)
    index = 0;
  else
    index = 1;
}
Syntax_Tree::Syntax_Tree()
{
  Reset();
}
Syntax_Tree::~Syntax_Tree()
{
  if(root)
    root->Remove();

  for(unsigned i = 0; i < nodepool.size(); i++)
  {
    delete nodepool[i];
  }
}
Syntax_Node* Syntax_Tree::Acquire_Node()
{
  if(nodepool.empty())
  {
    return new Syntax_Node(&nodepool, this);
  }
  Syntax_Node* node = nodepool.back();
  nodepool.pop_back();
  return node;
}
void Syntax_Tree::Process(Syntax_Node* node)
{
  if(!node)
    return;
  node->Finalize();

  if(!root)
    root = node;
  else
    root->Process(node);
}
bool Syntax_Tree::Evaluate()
{
  if(!root)
    return false;

  bool result = root->Evaluate();
  root->Remove();
  root = NULL;
  for(unsigned i = 0; i < registers.size(); i++)
  {
    registers[i] = false;
  }
  return result;
}
void Syntax_Tree::Reset()
{
  package_depth = 0;
  root = NULL;
  bytecodes.clear();
}
std::vector<Bytecode>* Syntax_Tree::Get_Bytecodes()
{
  return &bytecodes;
}
std::vector<bool>* Syntax_Tree::Get_Registers()
{
  return &registers;
}
unsigned Syntax_Tree::Get_Depth()
{
  return package_depth;
}