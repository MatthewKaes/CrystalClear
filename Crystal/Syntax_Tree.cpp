#include "Syntax_Tree.h"

Syntax_Node::Syntax_Node(std::vector<Syntax_Node*>* pool, Crystal_Compiler* compiler)
{
  pool_ = pool;
  compiler_ = compiler;
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
  for(int i = 0; i < MAX_ARGS; i++)
    if(params[i])
      if(!params[i]->Evaluate())
        return false;

  if(!code_gen(compiler_))
    return false;

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

  code_gen = Resolve_Genorator(sym.type);
}
Syntax_Tree::Syntax_Tree(Crystal_Compiler* compiler)
{
  compiler_ = compiler;
  root = NULL;
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
    return new Syntax_Node(&nodepool, compiler_);
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
  return result;
}