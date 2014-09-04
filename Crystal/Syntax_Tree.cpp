#include "Syntax_Tree.h"

Syntax_Node::Syntax_Node(std::vector<Syntax_Node*>* pool, Syntax_Tree* tree)
{
  tree_ = tree;
  pool_ = pool;
}
void Syntax_Node::Process(Syntax_Node* node)
{
  
  if((sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION) &&
    node->sym.type == DAT_OP && node->sym.str[0] == ',' && node->priority == priority + 1)
  {
    index += 1;
    if(index >= sym.i32)
    {
      printf("WARNING: \"%s\" does not take %d arguments.", sym.str.c_str(), index + 1);
    }
    return;
  }
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
  bool evaluation = false;
  Bytecode new_code;
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      evaluation = true;
      new_code.elements.push_back(*params[i]->Acquire());
      if(!params[i]->Evaluate())
      {
        return false;
      }
    }
  }
  if(!evaluation)
    return true;

  new_code.code_gen = Resolve_Genorator(&sym);

  if((sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION) && index < sym.i32)
  {
    printf("ERROR: \"%s\" requires %d argument%s.", sym.str.c_str(), sym.i32, sym.i32 == 1 ? "" : "s");
  }

  std::vector<bool>* regptr = tree_->Get_Registers();
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      new_code.elements.push_back(*params[i]->Acquire());
      if(params[i]->Acquire()->type == DAT_REGISTRY)
        (*regptr)[params[i]->Acquire()->i32] = false;
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
  
  //Finalize Bytecode
  new_code.result = sym;
  tree_->Get_Bytecodes()->push_back(new_code);

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

  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION)
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
  return Get_Registers()->size();
}