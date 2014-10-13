#include "Syntax_Tree.h"
#include "Reductions.h"

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
  if((!node->R_Assoc && node->priority > priority) || (node->R_Assoc && node->priority >= priority))
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
      parent->params[parent->index] = node;
    parent = node;

    if(tree_->Get_Root() == this)
    {
      tree_->Set_Root(node);
    }
  }
}

void Syntax_Node::Reduce()
{
  bool eval = false;
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      params[i]->Reduce();
      eval = true;
    }
  }
  if(!eval)
    return;

  if(Reduction(&sym, params[0]->Acquire(), params[1]->Acquire()))
  {  
    for(int i = 0; i < MAX_ARGS; i++)
    {
      if(params[i])
      {
        params[i]->Remove();
        params[i] = NULL;
      }
    }
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
      if(!params[i]->Evaluate())
      {
        return false;
      }
    }
  }
  
  if(sym.type != DAT_FUNCTION && sym.type != DAT_BIFUNCTION && !evaluation)
    return true;

  new_code.code_gen = Resolve_Genorator(&sym);
  new_code.base = sym;
  new_code.result.type = DAT_NIL;
  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION)
  { 
    if((index != sym.i32 && sym.i32 == 0) || (index != sym.i32 - 1 && sym.i32 > 0))
      printf("ERROR: \"%s\" requires %d argument%s.\n", sym.str.c_str(), sym.i32, sym.i32 == 1 ? "" : "s");
   
    Force_Memory(&new_code);
    if(sym.type == DAT_BIFUNCTION)
    {
      new_code.result.type = DAT_REGISTRY;
      new_code.result.i32 = 0;
    }
  }
  if(sym.type == DAT_STATEMENT)
  {
    if(!sym.str.compare("return"))
      Force_Memory(&new_code);
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

  if(parent != NULL)
  {
    //Stacking assignment operator without using
    //uneccesary registers.
    if(sym.type == DAT_OP && (!sym.str.compare("=") || !sym.str.compare("+=")
       || !sym.str.compare("-=")  || !sym.str.compare("*=")
       || !sym.str.compare("^=")  || !sym.str.compare("%=")
       || !sym.str.compare("<>")))
    {
      sym = new_code.result = *params[0]->Acquire();
    }
    else
    {
      sym.type = DAT_REGISTRY;
      sym.i32 = tree_->Get_Open_Reg();
      if(sym.i32 == -1)
      {
        sym.i32 = regptr->size();
        regptr->push_back(true);
      }
      new_code.result = sym;
    }
  }
  
  //Finalize Bytecode
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
  R_Assoc = false;
  parent = NULL;
  for(int i = 0; i < MAX_ARGS; i++)
  {
    params[i] = NULL;
  }

  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION || sym.type == DAT_STATEMENT)
    index = 0;
  else
    index = 1;

  if(sym.type == DAT_OP)
  {
    if(!sym.str.compare("=") || !sym.str.compare("+=")
       || !sym.str.compare("-=")  || !sym.str.compare("*=")
       || !sym.str.compare("^=")  || !sym.str.compare("%=")
       || !sym.str.compare("<>"))
    {
      R_Assoc = true;
    }
  }
}
void Syntax_Node::Force_Memory(Bytecode* code)
{
  for(int i = 0; i < MAX_ARGS; i++)
  {
    if(params[i])
    {
      code->elements.push_back(*params[i]->Acquire());
      Data_Type t = params[i]->Acquire()->type;
      if(t != DAT_LOCAL && t != DAT_REGISTRY)
      {
        params[i]->Acquire()->type = DAT_REGISTRY;
        int index = tree_->Get_Open_Reg();
        if(index == -1)
        {
          index = tree_->Get_Registers()->size();
          tree_->Get_Registers()->push_back(true);
        }
        params[i]->Acquire()->i32 = index;
      }
    }
  }
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

  root->Reduce();
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
void Syntax_Tree::Set_Root(Syntax_Node* new_root)
{
  root = new_root;
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
Syntax_Node* Syntax_Tree::Get_Root()
{
  return root;
}
int Syntax_Tree::Get_Open_Reg()
{
  for(unsigned i = 0; i < registers.size(); i++)
  {
    if(!registers[i])
    {
      registers[i] = true;
      return i;
    }
  }
  return -1;
}