#include "Compiler.h"
#include "Lexicon.h"
#include "Obscure.h"
#include "Helper.h"

int CRY_ARG::poolindex = 0;
char CRY_ARG::strpool[STRING_POOL] = {0};

CRY_ARG::CRY_ARG(const char* str) : type(CRY_TEXT), filt(CRY_TEXT)
{
  str_ = strpool + poolindex;
  strcpy(str_, str);
  poolindex += strlen(str) + 2;
  strpool[poolindex - 1] = '\0';
}
CRY_ARG::CRY_ARG(Crystal_Data* sym)
{
  switch(sym->type)
  {
  case DAT_NIL:
    type = CRY_NIL;
    break;
  case DAT_INT:
    type = CRY_INT;
    num_ = sym->i32;
    break;
  case DAT_BOOL:
    type = CRY_BOOL;
    bol_ = sym->b;
    break;
  case DAT_DOUBLE:
    type = CRY_DOUBLE;
    dec_ = sym->d;
    break;
  case DAT_STRING:
    type = CRY_TEXT;
    str_ = strpool + poolindex;
    strcpy(str_, sym->str.c_str());
    poolindex += sym->str.size() + 2;
    strpool[poolindex - 1] = '\0';
    break;
  }
  filt.Set(type);
}

Crystal_Compiler::Crystal_Compiler(AOT_Compiler* target)
{
  Machine = target;
}

Crystal_Compiler::~Crystal_Compiler()
{
  VirtualFreeEx(GetCurrentProcess(), program.load, 1<<16, MEM_RELEASE);
  delete Machine;
}

void Crystal_Compiler::Start_Encode(std::string name, unsigned locals_used, unsigned stack_count, unsigned arguments)
{
  program.load = (byte*)VirtualAllocEx( GetCurrentProcess(), 0, 1<<10, MEM_COMMIT | MEM_RESERVE , PAGE_EXECUTE_READWRITE);
  Machine->Setup(name, program.load);

  //Local and stack depth.
  locals_count = locals_used;
  stack_depth = stack_count;
  if(stack_depth == 0)
    stack_depth = 1;

  stack_size = (locals_count + stack_depth + 1) * VAR_SIZE + 4;
  Machine->Allocate_Stack(stack_size);
  states.resize(locals_count + stack_depth + 1);
}
void Crystal_Compiler::End_Encode()
{
  //Resolve 
  Return();
  //Construct Package
  CryPackage pack;
  pack.name = Machine->Get_Name();
  pack.links = Machine->Get_Links();
  pack.program = program;
  packages.push_back(pack);
  package_lookup[pack.name] = pack.program;
}
void Crystal_Compiler::Linker()
{
  for(unsigned i = 0; i < packages.size(); i++)
  {
    for(unsigned j = 0; j < packages[i].links.size(); j++)
    {
      BYTE* call = package_lookup[packages[i].links[j].name].load;
      for(unsigned w = 0; w < packages[i].links[j].refrence_list.size(); w++)
      {
        int* adder = (int*)packages[i].links[j].refrence_list[w];
        *adder = (int)call;
      }
    }
  }
}
int Crystal_Compiler::Execute(Crystal_Symbol* ret)
{
  if(package_lookup.find("main") == package_lookup.end())
  {
    //Error
    return -1;
  }
  CryProg entry = package_lookup["main"];
  return entry.call(ret);
}
void Crystal_Compiler::Print(unsigned var)
{
  Push(var);
  Call(Crystal_Print);
  Pop(1);
}
void Crystal_Compiler::Call(void* function)
{
  Machine->Call(function);
}
void Crystal_Compiler::Call(void* function, unsigned var)
{
  Push(var);
  Machine->Call(function);
  states[var].Obscurity();
}
void Crystal_Compiler::Call(const char* cry_function, unsigned var)
{
  if(var != CRY_NULL)
  {
    unsigned offset = stack_size - VAR_SIZE * var;
    Machine->Lea(EAX, offset);
  }
  else
  {
    Machine->Load_Register(EAX, 0);
  }
  Machine->Push(EAX);
  //Call Crystal function
  Machine->Call(cry_function);
  Machine->Pop(4);
  if(var != CRY_NULL)
  {
    states[var].Obscurity();
  }
}
void Crystal_Compiler::Push(unsigned var)
{
  unsigned offset = stack_size - var * VAR_SIZE;
  Machine->Push_Adr(offset);
}
void Crystal_Compiler::Pop(unsigned args)
{
  Machine->Pop(args * 4);
}
void Crystal_Compiler::Return(unsigned var)
{
  unsigned label = Machine->New_Label();
  unsigned offset = stack_size - VAR_SIZE * var;
  //Check to see if eax was loaded with something
  Machine->CmpF(RETURN_ADDRESS, 0);
  Machine->Je(label);
  //Load the data into the ptr value
  Machine->Load_Ptr(RETURN_ADDRESS);
  //Mark memory as collected and pass it back for use.
  if(states[var].Test(CRY_TEXT) || states[var].Test(CRY_STRING) || 
     states[var].Test(CRY_ARRAY))
  {
    states[var].Collected();
    Machine->MovP(offset - DATA_PNTR, DATA_PNTR);
  }
  Machine->MovP(offset - DATA_LOWER, DATA_LOWER);
  Machine->MovP(offset - DATA_UPPER, DATA_UPPER);
  Machine->MovP(offset - DATA_TYPE, DATA_TYPE, true);
  //Finalize return
  Machine->Make_Label(label);
  //cleanup 
  for(unsigned i = 0; i < (locals_count + stack_depth); i++)
  {
    Garbage_Collection(i);
  }

  Machine->Return();
}
void Crystal_Compiler::Return()
{
  unsigned label = Machine->New_Label();
  //Check to see if eax was loaded with something
  Machine->CmpF(RETURN_ADDRESS, 0);
  Machine->Je(label);
  CRY_ARG reg;
  Load(Addr_Reg(0));
  Machine->Load_Ptr(RETURN_ADDRESS);
  Machine->MovP(stack_size - VAR_SIZE * Addr_Reg(0) - DATA_TYPE, DATA_TYPE, true);
  Machine->Make_Label(label);
  //cleanup 
  for(unsigned i = 0; i < (locals_count + stack_depth); i++)
  {
    Garbage_Collection(i);
  }
  Machine->Return();
}
void Crystal_Compiler::Load(unsigned var, CRY_ARG val)
{
  unsigned offset = stack_size - VAR_SIZE * var;
  switch(val.type)
  {
  case CRY_INT:
    Machine->Load_Mem(offset - DATA_LOWER, val.num_);
    break;
  case CRY_INT64:
    Machine->Load_Mem(offset - DATA_LOWER, static_cast<int>(val.lrg_));
    Machine->Load_Mem(offset - DATA_UPPER, static_cast<int>(val.lrg_ / 0x100000000));
    break;
  case CRY_BOOL:
    Machine->Load_Mem(offset - DATA_LOWER, val.bol_ ? 1 : 0);
    break;
  case CRY_DOUBLE:
    Machine->Load_Mem(offset - DATA_LOWER, val.dec_);
    break;
  //Text can be loaded as constanst since strings cant.
  case CRY_TEXT:
    Garbage_Collection(var);
    Machine->Load_Mem(offset - DATA_PNTR, val.str_);
    Machine->Load_Mem(offset - DATA_LOWER, static_cast<int>(strlen(val.str_)));
    //Machine->Load_Mem(offset - DATA_UPPER, EAX);
    break;
  }
  Machine->Load_Mem(offset - DATA_TYPE, static_cast<char>(val.type));
  states[var].Set(val.type);
}
void Crystal_Compiler::Copy(unsigned dest, unsigned source)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;
  if(!(states[source].Size() == 1 && states[source].Test(CRY_NIL)))
  {
    Machine->Mov(EAX, offset_source - DATA_LOWER);
    Machine->Mov(offset_dest - DATA_LOWER, EAX);
    if(states[source].Test(CRY_DOUBLE) || states[source].Test(CRY_INT64))
    {
      Machine->Mov(EAX, offset_source - DATA_UPPER);
      Machine->Mov(offset_dest - DATA_UPPER, EAX);
    }
    if(states[source].Test(CRY_STRING))
    {
      unsigned label = Machine->New_Label();
      //If we can be more then just a string then we need to check
      //our current state as a string.
      if(states[source].Size() > 1)
      {
        Machine->Cmp(offset_source - DATA_TYPE, static_cast<char>(CRY_STRING));
        Machine->Jne(label);
      }
      //Copy over if we are currently a string.
      Machine->Mov(EBX, offset_source - DATA_LOWER);
      Machine->Push(EBX);
      Machine->Call(malloc);
      Machine->Pop(4);
      Machine->Strcpy(EAX, offset_source - DATA_PNTR, offset_source - DATA_LOWER);
      Machine->Mov(offset_dest - DATA_PNTR, EAX);
      Machine->Mov(offset_dest - DATA_LOWER, EBX);
      //Create end label for jumping
      if(states[source].Size() > 1)
      {
        Machine->Make_Label(label);
      }
    }
    else if(states[source].Test(CRY_TEXT))
    {
      Garbage_Collection(dest);
      Machine->Mov(EAX, offset_source - DATA_PNTR);
      Machine->Mov(offset_dest - DATA_PNTR, EAX);
    }
  }
  Machine->Mov(EAX, offset_source - DATA_TYPE, true);
  Machine->Mov(offset_dest - DATA_TYPE, EAX, true);
  states[dest] = states[source];
}
void Crystal_Compiler::Add(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    Clarity_Filter except(CRY_TEXT);
    except.Dilute(CRY_STRING);
    except.Dilute(CRY_ARRAY);
    if(Null_Op(states[dest], states[source], dest, except))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Mov(EAX, offset_source - DATA_LOWER, true);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Add(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      }
      Machine->FPU_Add();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    case CRY_TEXT:
      //Faster way of creating a string from two text objects
      if(states[dest].Test(CRY_TEXT) && states[source].Test(CRY_TEXT))
      {
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Mov(EBX, offset_source - DATA_LOWER);
        Machine->Add(EBX, EAX);
        //Make room for zero
        Machine->Inc(EBX);
        Machine->Push(EBX);
        Machine->Call(malloc);
        Machine->Pop(4);
        Machine->Strcpy(EAX, offset_dest - DATA_PNTR, offset_dest - DATA_LOWER);
        Machine->Strcpy(EDI, offset_source - DATA_PNTR, offset_source - DATA_LOWER);
        Machine->Mov(offset_dest - DATA_PNTR, EAX);
        Machine->Mov(offset_dest - DATA_LOWER, EBX);
      }
      //complex slower text handling:
      else
      {
        Push(source);
        Push(dest);
        if(left)
          Call(Crystal_Text_Append);
        else
          Call(Crystal_Text_AppendR);
        Pop(2);
      }
      //we now have a string.
      resolve = CRY_STRING;
      break;
    case CRY_STRING:
      Push(source);
      Push(dest);
      if(left)
        Call(Crystal_Text_Append);
      else
        Call(Crystal_Text_AppendR);
      Pop(2);
      break;
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(left ? Obscure_Addition : Obscure_AdditionR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::AddC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation 
    Clarity_Filter except(CRY_TEXT);
    except.Dilute(CRY_STRING);
    except.Dilute(CRY_ARRAY);
    if(Null_Op(states[dest], const_.filt, dest, except))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Load_Register(EAX, const_.bol_);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Add(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
      {
        Machine->FPU_Load(const_.num_);
      }
      else
      {
        Machine->FPU_Load(const_.dec_);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      }
      Machine->FPU_Add();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
     case CRY_TEXT:
      //Faster way of creating a string from two text objects
      if(states[dest].Test(CRY_TEXT))
      {
        std::string converted;
        if(const_.filt.Test(CRY_BOOL))
        {
          if(const_.bol_)
          {
            converted.assign("true");
          }
          else
          {
            converted.assign("false");
          }
        }
        else if(const_.filt.Test(CRY_INT))
        {
          i_to_str(const_.num_, &converted);
        }
        else if(const_.filt.Test(CRY_DOUBLE))
        {
          d_to_str(const_.dec_, &converted);
        }
        else if(const_.filt.Test(CRY_TEXT))
        {
          converted.assign(const_.str_);
        }
        else
        {
          converted.assign("nil");
        }
        Machine->Mov(EBX, offset_dest - DATA_LOWER);
        Machine->Mov(EBX, converted.length() + 1);
        Machine->Add(EBX, EAX);
        Machine->Push(EBX);
        Machine->Call(malloc);
        Machine->Pop(4);
        Machine->Strcpy(EAX, offset_dest - DATA_PNTR, offset_dest - DATA_LOWER);
        Machine->Strcpy(EDI, static_cast<unsigned>(Machine->String_Address(converted.c_str())), converted.length() + 1, true);
        Machine->Mov(offset_dest - DATA_PNTR, EAX);
        Machine->Mov(offset_dest - DATA_LOWER, EBX);
      }
      else
      {
        Machine->Push(static_cast<int>(strlen(const_.str_)));
        Machine->Push(Machine->String_Address(const_.str_));
        Push(dest);
        if(left == true)
          Call(Crystal_Const_Append_T);
        else
          Call(Crystal_Const_Append_TL);
        Pop(3);
      }
      //we now have a string.
      resolve = CRY_STRING;
      break;
    case CRY_STRING:
      //Empty scope
      {
        //Preprocess so we don't have to do it at runtime.
        std::string converted;
        if(const_.filt.Test(CRY_BOOL))
        {
          if(const_.bol_)
          {
            converted.assign("true");
          }
          else
          {
            converted.assign("false");
          }
        }
        else if(const_.filt.Test(CRY_INT))
        {
          i_to_str(const_.num_, &converted);
        }
        else if(const_.filt.Test(CRY_DOUBLE))
        {
          d_to_str(const_.dec_, &converted);
        }
        else if(const_.filt.Test(CRY_TEXT))
        {
          converted.assign(const_.str_);
        }
        else
        {
          converted.assign("nil");
        }
        Machine->Push(static_cast<int>(converted.length()));
        Machine->Push(Machine->String_Address(converted.c_str()));
      }
      Push(dest);
      Call(Crystal_Text_Append_C);
      Pop(3);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    //Load const into temp.
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(left ? Obscure_Addition : Obscure_AdditionR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Sub(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation 
    if(Null_Op(states[dest], states[source], dest))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Mov(EAX, offset_source - DATA_LOWER, true);
      Machine->And(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      if(left)
      {
        Machine->Mov(EAX, offset_source - DATA_LOWER);
        Machine->Sub(offset_dest - DATA_LOWER, EAX);
      }
      else
      {
        Machine->Mov(EAX, offset_source - DATA_LOWER);
        Machine->Mov(EBX, offset_dest - DATA_LOWER);
        Machine->Sub(EAX, EBX);
        Machine->Mov(offset_dest - DATA_LOWER, EAX);
      }
      break;
    case CRY_DOUBLE:
      if(left)
      {
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
          Machine->FPU_Loadi(offset_dest - DATA_LOWER);
        else
          Machine->FPU_Loadd(offset_dest - DATA_LOWER);
        if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
          Machine->FPU_Loadi(offset_source - DATA_LOWER);
        else
          Machine->FPU_Loadd(offset_source - DATA_LOWER);
      }
      else
      {
        if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
          Machine->FPU_Loadi(offset_source - DATA_LOWER);
        else
          Machine->FPU_Loadd(offset_source - DATA_LOWER);
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
          Machine->FPU_Loadi(offset_dest - DATA_LOWER);
        else
          Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      }
      Machine->FPU_Sub();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(left ? Obscure_Subtraction : Obscure_SubtractionR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::SubC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation 
    if(Null_Op(states[dest], const_.filt, dest))
      return;
   
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Load_Register(EAX, const_.bol_);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      if(left)
      {
        Machine->Load_Register(EAX, const_.num_);
        Machine->Sub(offset_dest - DATA_LOWER, EAX);
      }
      else
      {        
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Load_Register(EBX, const_.num_);
        Machine->Sub(EBX, EAX);
        Machine->Mov(offset_dest - DATA_LOWER, EBX);
      }
      break;
    case CRY_DOUBLE:
      if(left)
      {
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
        {
          Machine->FPU_Loadi(offset_dest - DATA_LOWER);
        }
        else
        {
          Machine->FPU_Loadd(offset_dest - DATA_LOWER);
        }
        if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
        {
          Machine->FPU_Load(const_.num_);
        }
        else
        {
          Machine->FPU_Load(const_.dec_);
        }
      }
      else
      {
        if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
        {
          Machine->FPU_Load(const_.num_);
        }
        else
        {
          Machine->FPU_Load(const_.dec_);
        }
        if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
        {
          Machine->FPU_Loadi(offset_dest - DATA_LOWER);
        }
        else
        {
          Machine->FPU_Loadd(offset_dest - DATA_LOWER);
        }
      }
      Machine->FPU_Sub();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {    
    //Load const into temp.
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(left ? Obscure_Subtraction : Obscure_SubtractionR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Mul(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(Null_Op(states[dest], states[source], dest))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      //TO DO:
      Machine->Mov(EAX, offset_source - DATA_LOWER, true);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Imul(offset_dest - DATA_LOWER);
      Machine->Mov(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      }
      if(states[source].Test(CRY_INT) || states[source].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      }
      Machine->FPU_Mul();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(Obscure_Multiplication);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::MulC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(Null_Op(states[dest], const_.filt, dest))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Load_Register(EAX, const_.bol_);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Imul(offset_dest - DATA_LOWER);
      Machine->Mov(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT) || const_.filt.Test(CRY_BOOL))
      {
        Machine->FPU_Load(const_.num_);
      }
      else
      {
        Machine->FPU_Load(const_.dec_);
      }
      if(states[dest].Test(CRY_INT) || states[dest].Test(CRY_BOOL))
      {
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      }
      Machine->FPU_Mul();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    //Load const into temp.
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(Obscure_Multiplication);
    Pop(2);
    //Copy(dest, Addr_Reg(stack_depth));
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Pow(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;
  unsigned labletop, lablebottom;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(Null_Op(states[dest], states[source], dest))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_BOOL:
      //TO DO:
      Machine->Mov(EAX, offset_source - DATA_LOWER, true);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      labletop = Machine->Reserve_Label();
      lablebottom = Machine->Reserve_Label();
      Machine->Mov(EBX, (left ? offset_dest : offset_source) - DATA_LOWER);
      Machine->Load_Register(EAX, 0);
      Machine->Make_Label(labletop);
      Machine->Inc(EAX);
      Machine->Cmp((left ? offset_source : offset_dest) - DATA_LOWER, EAX);
      Machine->Jge(lablebottom);

      Machine->Imul(EBX, (left ? offset_dest : offset_source) - DATA_LOWER);

      Machine->Jmp(labletop);
      Machine->Make_Label(lablebottom);

      Machine->Mov(offset_dest - DATA_LOWER, EBX);
      break;
    case CRY_DOUBLE:
      Push(source);
      Push(dest);
      Machine->Call(left ? Power_Syms : Power_SymsR);
      Pop(2);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(left ? Obscure_Power : Obscure_PowerR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::PowC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned labletop, lablebottom;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(Null_Op(states[dest], const_.filt, dest))
      return;

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      Machine->Load_Register(EAX, const_.bol_);
      Machine->Or(offset_dest - DATA_LOWER, EAX);
      break;
    case CRY_INT:
      labletop = Machine->Reserve_Label();
      lablebottom = Machine->Reserve_Label();
      if(left)
      {
        Machine->Mov(EBX, offset_dest - DATA_LOWER);
        Load(Addr_Reg(stack_depth), const_);
      }
      else
        Machine->Load_Register(EBX, const_.num_);
      Machine->Load_Register(EAX, 0);
      Machine->Make_Label(labletop);
      Machine->Inc(EAX);
      if(left)
        Machine->Cmp((stack_size - Addr_Reg(stack_depth) * VAR_SIZE) - DATA_LOWER, EAX);
      else
        Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Jge(lablebottom);

      Machine->Imul(EBX, (left ? offset_dest : stack_size - Addr_Reg(stack_depth) * VAR_SIZE) - DATA_LOWER);

      Machine->Jmp(labletop);
      Machine->Make_Label(lablebottom);

      Machine->Mov(offset_dest - DATA_LOWER, EBX);
      break;
    case CRY_DOUBLE:
      Load(Addr_Reg(stack_depth), const_);
      Push(Addr_Reg(stack_depth));
      Push(dest);
      Machine->Call(left ? Power_Syms : Power_SymsR);
      Pop(2);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, resolve);
  }
  //Clarity Handling
  else
  {
    //Load const into temp.
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(left ? Obscure_Power : Obscure_PowerR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
unsigned Crystal_Compiler::Addr_Reg(unsigned reg)
{
  return locals_count + reg;
}


//==========================
// Helper Functions
//==========================
bool Crystal_Compiler::Null_Op(Clarity_Filter& l, Clarity_Filter& r, unsigned dest, Clarity_Filter Exceptions)
{
  if(!l.Test(CRY_NIL) && !r.Test(CRY_NIL))
    return false;

  for(unsigned type = CRY_NIL + 1; type < CRY_SYMS; type++)
  {
    Symbol_Type test_type = static_cast<Symbol_Type>(type);
    if(!Exceptions.Test(test_type))
    {
      continue;
    }
    if(l.Test(test_type) || r.Test(test_type))
    {
      return false;
    }
  }
  Machine->Load_Mem(stack_size - VAR_SIZE * dest - DATA_TYPE, static_cast<char>(CRY_NIL));
  l.Set(CRY_NIL);
  return true;
}
void Crystal_Compiler::Runtime_Resovle(unsigned dest, Symbol_Type resolve)
{
  if(!states[dest].Test(resolve))
  {
    Machine->Load_Mem(stack_size - VAR_SIZE * dest - DATA_TYPE, static_cast<char>(resolve));
    states[dest].Set(resolve);
  }
}
void Crystal_Compiler::Garbage_Collection(unsigned var)
{
  if(states[var].Collection())
  {
    unsigned offset_dest = stack_size - var * VAR_SIZE;
    unsigned label = Machine->New_Label();
    Machine->Cmp(offset_dest - DATA_PNTR, 0);
    Machine->Je(label);
    Machine->Mov(EAX, offset_dest - DATA_PNTR);
    Machine->Push(EAX);
    Call(free);
    Machine->Pop(4);
    Machine->Load_Mem(offset_dest - DATA_PNTR, 0);
    Machine->Make_Label(label);
    states[var].Collected();
  }
}