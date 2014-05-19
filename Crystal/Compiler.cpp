#include "Compiler.h"

Crystal_Compiler::Crystal_Compiler()
{
}

Crystal_Compiler::~Crystal_Compiler()
{
  VirtualFreeEx(GetCurrentProcess(), program.load, 1<<16, MEM_RELEASE);
}

void Crystal_Compiler::Start_Encode(std::string name, unsigned locals_used, unsigned arguments)
{
  program.load = (byte*)VirtualAllocEx( GetCurrentProcess(), 0, 1<<10, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
  x86_Machine.Setup(name, program.load);
  locals_count = locals_used;
  stack_size = (locals_count + CRY_NA) * VAR_SIZE + 4;
  x86_Machine.Allocate_Stack(stack_size);
  states.resize(locals_count + CRY_NA);
}
void Crystal_Compiler::End_Encode()
{
  //Resolve 
  Return();
  //Construct Package
  CryPackage pack;
  pack.name = x86_Machine.Get_Name();
  pack.links = x86_Machine.Get_Links();
  pack.program = program;
  packages.push_back(pack);
}
void Crystal_Compiler::Linker()
{
  for(unsigned i = 0; i < packages.size(); i++)
  {
    for(unsigned j = 0; j < packages[i].links.size(); j++)
    {
      for(unsigned k = 0; k < packages.size(); k++)
      {
        if(!packages[i].links[j].name.compare(packages[k].name))
        {
          for(unsigned w = 0; w < packages[i].links[j].refrence_list.size(); w++)
          {
            int* adder = (int*)packages[i].links[j].refrence_list[w];
            int result = (int)packages[k].program.load;
            *adder = result;
          }
          break;
        }
      }
    }
  }
}
int Crystal_Compiler::Execute(Crystal_Symbol* ret)
{
  for(unsigned i = 0; i < packages.size(); i++)
  {
    if(!packages[i].name.compare("main"))
    {
      return packages[i].program.call(ret);
    }
  }
  return -1;
}
void Crystal_Compiler::Print(unsigned var)
{
  Push(var);
  Call(Crystal_Print);
  Pop(1);
}
void Crystal_Compiler::Call(void* function)
{
  x86_Machine.Call(function);
}
void Crystal_Compiler::Call(const char* cry_function, unsigned var)
{
  if(var != CRY_NULL)
  {
    unsigned offset = stack_size - VAR_SIZE * var;
    x86_Machine.Lea(EAX, offset);
  }
  else
  {
    x86_Machine.Load_Register(EAX, 0);
  }
  x86_Machine.Push(EAX);
  //Call Crystal function
  x86_Machine.Call(cry_function);
  x86_Machine.Pop(4);
  states[var].Obscurity();
}
void Crystal_Compiler::Push(unsigned var)
{
  unsigned offset = stack_size - var * VAR_SIZE;
  x86_Machine.Push_Adr(offset);
}
void Crystal_Compiler::Pop(unsigned args)
{
  x86_Machine.Pop(args * 4);
}
void Crystal_Compiler::Return(unsigned var)
{
  unsigned label = x86_Machine.New_Label();
  unsigned offset = stack_size - VAR_SIZE * var;
  //Check to see if eax was loaded with something
  x86_Machine.CmpF(12, 0);
  x86_Machine.Je(label);
  //Load the data into the ptr value
  x86_Machine.Load_Ptr(12);
  if(states[var].Test(CRY_TEXT) || states[var].Test(CRY_STRING) || 
     states[var].Test(CRY_ARRAY))
  {
    x86_Machine.MovP(offset - DATA_PNTR, DATA_PNTR);
  }
  x86_Machine.MovP(offset - DATA_LOWER, DATA_LOWER);
  x86_Machine.MovP(offset - DATA_UPPER, DATA_UPPER);
  x86_Machine.MovP(offset - DATA_TYPE, DATA_TYPE, true);
  //Finalize return
  x86_Machine.Make_Label(label);
  x86_Machine.Return();
}
void Crystal_Compiler::Return()
{
  unsigned label = x86_Machine.New_Label();
  //Check to see if eax was loaded with something
  x86_Machine.CmpF(12, 0);
  x86_Machine.Je(label);
  Load(Addr_Reg(CRY_R0));
  x86_Machine.Load_Ptr(12);
  x86_Machine.MovP(stack_size - VAR_SIZE * Addr_Reg(CRY_R0) - DATA_TYPE, DATA_TYPE, true);
  x86_Machine.Make_Label(label);
  x86_Machine.Return();
}
void Crystal_Compiler::Load(unsigned var, CRY_ARG val)
{
  unsigned offset = stack_size - VAR_SIZE * var;
  switch(val.type)
  {
  case CRY_INT:
    x86_Machine.Load_Mem(offset - DATA_LOWER, val.num_);
    //x86_Machine.Load_Mem(offset - DATA_UPPER, 0);
    break;
  case CRY_INT64:
    x86_Machine.Load_Mem(offset - DATA_LOWER, static_cast<int>(val.lrg_));
    x86_Machine.Load_Mem(offset - DATA_UPPER, static_cast<int>(val.lrg_ / 0x100000000));
    break;
  case CRY_BOOL:
    x86_Machine.Load_Mem(offset - DATA_LOWER, val.bol_ ? 1 : 0);
    //x86_Machine.Load_Mem(offset - DATA_UPPER, 0);
    break;
  case CRY_DOUBLE:
    x86_Machine.Load_Mem(offset - DATA_LOWER, val.dec_);
    break;
  case CRY_TEXT:
    x86_Machine.Load_Mem(offset - DATA_PNTR, val.str_);
    x86_Machine.Load_Mem(offset - DATA_LOWER, static_cast<int>(strlen(val.str_)));
    x86_Machine.Load_Mem(offset - DATA_UPPER, EAX);
    break;
  }
  x86_Machine.Load_Mem(offset - DATA_TYPE, static_cast<char>(val.type));
  states[var].Set(val.type);
}
void Crystal_Compiler::Copy(unsigned dest, unsigned source)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;
  if(!(states[source].Size() == 1 && states[source].Test(CRY_NIL)))
  {
    x86_Machine.Mov(EAX, offset_source - DATA_LOWER);
    x86_Machine.Mov(offset_dest - DATA_LOWER, EAX);
    if(states[source].Test(CRY_DOUBLE) || states[source].Test(CRY_INT64))
    {
      x86_Machine.Mov(EAX, offset_source - DATA_UPPER);
      x86_Machine.Mov(offset_dest - DATA_UPPER, EAX);
    }
  }
  x86_Machine.Mov(EAX, offset_source - DATA_TYPE, true);
  x86_Machine.Mov(offset_dest - DATA_TYPE, EAX, true);
  states[dest] = states[source];
}
void Crystal_Compiler::Add(unsigned dest, unsigned source)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || states[source].Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER, true);
      x86_Machine.Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER);
      x86_Machine.Add(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_source - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_source - DATA_LOWER);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
      }
      x86_Machine.FPU_Add();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
void Crystal_Compiler::AddC(unsigned dest, CRY_ARG const_)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || const_.filt.Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Load_Register(EAX, const_.bol_);
      x86_Machine.Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      x86_Machine.Load_Register(EAX, const_.num_);
      x86_Machine.Add(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
      {
        x86_Machine.FPU_Load(const_.num_);
      }
      else
      {
        x86_Machine.FPU_Load(const_.dec_);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
      }
      x86_Machine.FPU_Add();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
void Crystal_Compiler::Sub(unsigned dest, unsigned source)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || states[source].Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER, true);
      x86_Machine.And(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER);
      x86_Machine.Sub(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
      }
      if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_source - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_source - DATA_LOWER);
      }
      x86_Machine.FPU_Sub();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
void Crystal_Compiler::SubC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || const_.filt.Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Load_Register(EAX, const_.bol_);
      x86_Machine.Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      if(left)
      {
        x86_Machine.Load_Register(EAX, const_.num_);
        x86_Machine.Sub(offset_dest - DATA_LOWER, EAX);
      }
      else
      {        
        x86_Machine.Mov(EAX, offset_dest - DATA_LOWER);
        x86_Machine.Load_Register(EBX, const_.num_);
        x86_Machine.Sub(EBX, EAX);
        x86_Machine.Mov(offset_dest - DATA_LOWER, EBX);
      }
      break;
    case CRY_DOUBLE:
      if(left)
      {
        if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
        {
          x86_Machine.FPU_Load(const_.num_);
        }
        else
        {
          x86_Machine.FPU_Load(const_.dec_);
        }
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
        {
          x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
        }
        else
        {
          x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
        }
      }
      else
      {
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
        {
          x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
        }
        else
        {
          x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
        }
        if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
        {
          x86_Machine.FPU_Load(const_.num_);
        }
        else
        {
          x86_Machine.FPU_Load(const_.dec_);
        }
      }
      x86_Machine.FPU_Sub();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
void Crystal_Compiler::Mul(unsigned dest, unsigned source)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || states[source].Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER, true);
      x86_Machine.Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      x86_Machine.Mov(EAX, offset_source - DATA_LOWER);
      x86_Machine.Imul(offset_dest - DATA_LOWER);
      x86_Machine.Mov(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
      }
      if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_source - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_source - DATA_LOWER);
      }
      x86_Machine.FPU_Mul();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
void Crystal_Compiler::MulC(unsigned dest, CRY_ARG const_)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || const_.filt.Test(CRY_NIL))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
      states[dest].Set(CRY_NIL);
      return;
    }
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      x86_Machine.Load_Register(EAX, const_.bol_);
      x86_Machine.Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      x86_Machine.Load_Register(EAX, const_.num_);
      x86_Machine.Imul(offset_dest - DATA_LOWER);
      x86_Machine.Mov(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
      {
        x86_Machine.FPU_Load(const_.num_);
      }
      else
      {
        x86_Machine.FPU_Load(const_.dec_);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        x86_Machine.FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        x86_Machine.FPU_Loadd(offset_dest - DATA_LOWER);
      }
      x86_Machine.FPU_Mul();
      x86_Machine.FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    if(!states[dest].Test(resolve))
    {
      x86_Machine.Load_Mem(offset_dest - DATA_TYPE, static_cast<char>(resolve));
      states[dest].Set(resolve);
    }
  }
  //Clarity Handling
  else
  {
    //TO DO:
  }
}
unsigned Crystal_Compiler::Addr_Reg(CRY_REGISTER reg)
{
  return locals_count + reg;
}
