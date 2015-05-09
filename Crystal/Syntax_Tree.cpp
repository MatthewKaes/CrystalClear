#include "Syntax_Tree.h"
#include "Reductions.h"
#include "Lexicon.h"

Syntax_Node::Syntax_Node(std::vector<Syntax_Node*>* pool, Syntax_Tree* tree)
{
  tree_ = tree;
  pool_ = pool;

  // Store default left and right
  params.push_back(0);
  params.push_back(0);
}

void Syntax_Node::Process(Syntax_Node* node)
{
  if((sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION || sym.str[0] == '[' || 
    (parent && !parent->Acquire()->str.compare("."))) && node->sym.type == DAT_OP && 
    node->sym.str[0] == ',' && node->priority < priority + Get_Precedence(NULL))
  {
    index += 1;
    if(index >= params.size())
    {
      params.push_back(0);
    }

    if((sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION) && index >= static_cast<unsigned>(sym.i32))
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
      // If an array is a ternimating leaf we need to
      // treat the array as an object.
      if(!node->sym.str.compare("["))
      {
        node->index = 0;
        node->sym.i32 = GETTER_ID;
      }
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
    // If we are doing an assignment to a special operator
    // we need to set it as a reference.
    if(!sym.str.compare("[") && !node->sym.str.compare("="))
    {
      node->sym.i32 = REFRENCE_ID;
      sym.i32 = REFRENCE_ID;
    }
  }
}

void Syntax_Node::Reduce()
{
  bool eval = false;
  for(unsigned i = 0; i < params.size(); i++)
  {
    if(params[i])
    {
      params[i]->Reduce();
      eval = true;
    }
  }
  if(!eval || !params[0] || !params[1])
    return;

  if(Reduction(&sym, params[0]->Acquire(), params[1]->Acquire()))
  {  
    for(unsigned i = 0; i < params.size(); i++)
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
  // Setup labels for prefaced control statements
  if(sym.type == DAT_STATEMENT)
  {
    if(!sym.str.compare("while"))
    {
      Bytecode loop_code;
      loop_code.code_gen = Loop_Gen;
      tree_->Get_Bytecodes()->push_back(loop_code); 
    }
    
    if(!sym.str.compare("elsif"))
    {
      Bytecode loop_code;
      loop_code.code_gen = ElseIf_Preface_Gen;
      tree_->Get_Bytecodes()->push_back(loop_code); 
    }
  }

  // Evaluate any children before this node is evaluated.
  bool evaluation = false;
  Bytecode new_code;
  for(unsigned i = 0; i < params.size(); i++)
  {
    // Since vectors aren't shrunk ignore empty slots
    if(params[i])
    {
      evaluation = true;
      if(!params[i]->Evaluate())
      {
        return false;
      }
    }
  }
  
  // Check to see if this symbol is an evaluable type.
  if(!Evaluable_Type(&sym) && !evaluation)
    return true;

  
  // Get the generator for the bytecode.
  new_code.code_gen = Resolve_Generator(&sym);
  new_code.base = sym;
  new_code.result.type = DAT_NIL;
  
  // Force arguments to be moved onto the stack for functions
  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION || sym.type == DAT_OBJFUNCTION)
  {
    Force_Memory(&new_code);
  }


  // Processing of global and built in functions.
  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION)
  { 
    // Check the argument count for functions.
    if((index != sym.i32 && sym.i32 == 0) || (index != sym.i32 - 1 && sym.i32 > 0))
      printf("ERROR: \"%s\" requires %d argument%s.\n", sym.str.c_str(), sym.i32, sym.i32 == 1 ? "" : "s");
    
    // Built in functions have to have a symbol to return to
    if(sym.type == DAT_BIFUNCTION)
    {
      new_code.result.type = DAT_REGISTRY;
      new_code.result.i32 = 0;
    }
  }
  else 

  // Force constants to be in a register or memory location for
  // certain control statements.
  if(sym.type == DAT_STATEMENT)
  {
    if(!sym.str.compare("return") || !sym.str.compare("if") ||
      !sym.str.compare("while") || !sym.str.compare("elsif"))
    {
      Force_Memory(&new_code);
    }
  }

  // Construct all the elements that this bytecode uses.
  std::vector<bool>* regptr = tree_->Get_Registers();

  for(unsigned i = 0; i < params.size(); i++)
  {
    if(params[i])
    {
      new_code.elements.push_back(*params[i]->Acquire());

      // Free up registers for future use.
      if(params[i]->Acquire()->type == DAT_REGISTRY)
        (*regptr)[params[i]->Acquire()->i32] = false;
    }
  }

  // Pull up arguments for the dot operator
  if(sym.type == DAT_OP && !sym.str.compare("."))
  {
    // All of our arguments are aggragated into our function
    params[1]->Force_Memory(&new_code);
    for(unsigned i = 0; i < params[1]->params.size(); i++)
    {
      if(params[1]->params[i])
      {
        new_code.elements.push_back(*params[1]->params[i]->Acquire());

        // Free up registers for future use.
        if(params[1]->params[i]->Acquire()->type == DAT_REGISTRY)
          (*regptr)[params[1]->params[i]->Acquire()->i32] = false;
      }
    }
  }

  // Stacking assignment operator without using
  // uneccesary registers.
  if(parent != NULL && Assignment_Op(&sym))
  {
    sym = new_code.result = *params[0]->Acquire();
  }
  else
  {
    // Move the result into a registry.
    sym.type = DAT_REGISTRY;
    sym.i32 = tree_->Get_Open_Reg();
    if(sym.i32 == -1)
    {
      sym.i32 = regptr->size();
      regptr->push_back(true);
    }
    new_code.result = sym;
  }
  
  //Finalize Bytecode
  tree_->Get_Bytecodes()->push_back(new_code);

  return true;
}

void Syntax_Node::Remove()
{
  // Remove all children of this node as well
  for(unsigned i = 0; i < params.size(); i++)
  {
    if(params[i])
    {
      params[i]->Remove();
      params[i] = NULL;
    }
  }

  // Add this node to the pool
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
  for(unsigned i = 0; i < params.size(); i++)
  {
    params[i] = NULL;
  }

  if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION || sym.type == DAT_STATEMENT)
    index = 0;
  else
    index = 1;

  if(Assignment_Op(&sym))
  {
    R_Assoc = true;
  }
}

void Syntax_Node::Force_Memory(Bytecode* code)
{
  for(unsigned i = 0; i < params.size(); i++)
  {
    if(params[i])
    {
      code->elements.push_back(*params[i]->Acquire());
      Data_Type t = params[i]->Acquire()->type;
      
      // For all types not already in a memory address
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

bool Syntax_Node::Evaluable_Type(Crystal_Data* sym)
{  
  // List of evaluable types
  switch(sym->type)
  {
  case DAT_FUNCTION:
  case DAT_BIFUNCTION: 
  case DAT_STATEMENT:
  case DAT_OP: 
  case DAT_CLASS:
  case DAT_OBJFUNCTION:
  case DAT_ATTRIBUTE:
    return true;
  default:
    return false;
  }
}

bool Syntax_Node::Assignment_Op(Crystal_Data* sym)
{
  if(sym->type != DAT_OP)
    return false;

  if(!sym->str.compare("=") || !sym->str.compare("+=")
    || !sym->str.compare("-=")  || !sym->str.compare("*=")
    || !sym->str.compare("^=")  || !sym->str.compare("%=")
    || !sym->str.compare("<>"))
    return true;

  return false;
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