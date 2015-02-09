#include "Compiler.h"
#include "Lexicon.h"
#include "Obscure.h"
#include "Helper.h"
#include "Function.h"
#include "Garbage_Collector.h"

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
  program.load = (byte*)VirtualAllocEx( GetCurrentProcess(), 0, 1<<16, MEM_COMMIT | MEM_RESERVE , PAGE_EXECUTE_READWRITE);
}

Crystal_Compiler::~Crystal_Compiler()
{
  VirtualFreeEx(GetCurrentProcess(), program.load, 1<<16, MEM_RELEASE);
  delete Machine;
}

void Crystal_Compiler::Start_Encode(std::string name, unsigned locals_used, unsigned stack_count, unsigned arguments)
{
  Machine->Setup(name, program.load);

  //Local and stack depth.
  locals_count = locals_used;
  stack_depth = stack_count;
  if(stack_depth == 0)
    stack_depth = 1;

  stack_size = (locals_count + stack_depth + 1) * VAR_SIZE + 4;
  Machine->Allocate_Stack(stack_size);
  states.resize(locals_count + stack_depth + 1);
  
  for(unsigned i = 0; i < arguments; i++)
  {
    Machine->Push_Stk(i * 4 + 0x14);
    Push(i);
    Call(Stack_Copy);
    Pop(2);
    states[i].Obscurity();
  }

  //set up a new generation for the garbage collector.
  Machine->Push(static_cast<int>(locals_count + stack_depth + 1));
  Push(0);
  Machine->Call(GC_Branch);
  Pop(2);

  //tidy up lookup data.
  lookups.clear();
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
  program.load = Machine->Location() + 1;
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
  if(var != CRY_NULL)
  {
    states[var].Obscurity();
  }
}
void Crystal_Compiler::Convert(unsigned reg, Symbol_Type type)
{
  Push(reg);
  switch(type)
  {
  case CRY_INT:
    Call(Parse_Int);
    Pop(1);
    Machine->Push(EAX);
    break;
  case CRY_DOUBLE:
    Call(Parse_Double);
    Pop(1);
    Machine->FPU_Store();
    break;
  }
}
void Crystal_Compiler::Allocate(unsigned sym_count)
{
  //Calloc Memory
  Machine->Push(static_cast<int>(VAR_SIZE));
  //Allocation is expensive. Get a somewhat useful
  //block size
  if(sym_count < MIN_BLOCK_SIZE)
    Machine->Push(static_cast<int>(MIN_BLOCK_SIZE));
  else
    Machine->Push(static_cast<int>(sym_count));
  Machine->Call(calloc);
  Machine->Pop(sizeof(int) * 2);
  //Push it for later use
  Machine->Push(EAX);
}
void Crystal_Compiler::Make_Array(unsigned var, unsigned size, unsigned capacity)
{
  //Creation of the array object
  Machine->Push(static_cast<int>(capacity));
  Machine->Push(static_cast<int>(size));
  Push(var);
  Machine->Call(Construct_Array);
  Machine->Pop(sizeof(int) * 4);
  //Set up types for compiler use
  states[var].Set(CRY_ARRAY);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}
void Crystal_Compiler::Make_Range(unsigned var)
{
  //Creation of the array object
  Push(var);
  Machine->Call(Construct_Range);
  Machine->Pop(sizeof(int) * 3);
  //Set up types for compiler use
  states[var].Set(CRY_ARRAY);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}
void Crystal_Compiler::Push(unsigned var)
{
  unsigned offset = stack_size - var * VAR_SIZE;
  Machine->Push_Adr(offset);
}
void Crystal_Compiler::Push_C(CRY_ARG var)
{
  switch(var.type)
  {
  case CRY_INT:
    Machine->Push(var.num_);
    return;
  case CRY_DOUBLE:
    Machine->Push(var.dec_);
    return;
  case CRY_TEXT:
    Machine->Push(reinterpret_cast<int>(var.str_));
    return;
  }
}
void Crystal_Compiler::Push_Reg()
{
  Machine->Push(EAX);
}
void Crystal_Compiler::Pop(unsigned args)
{
  Machine->Pop(args * 4);
}
void Crystal_Compiler::Return(unsigned var)
{
  unsigned label = Machine->Reserve_Label();
  unsigned offset = stack_size - VAR_SIZE * var;

  //Check to see if eax was loaded with something
  Machine->CmpF(RETURN_ADDRESS, 0);
  Machine->Je(label);
  //Load the data into the ptr value
  Machine->Load_Ptr(RETURN_ADDRESS);

  //Load return pointer with all the proper data.
  Machine->MovP(offset - DATA_PNTR, DATA_PNTR);

  Machine->MovP(offset - DATA_LOWER, DATA_LOWER);
  Machine->MovP(offset - DATA_UPPER, DATA_UPPER);
  Machine->MovP(offset - DATA_TYPE, DATA_TYPE, true);

  //Set up garbage Collection
  if(states[var].Order(CRY_POINTER))
  {
    Machine->Cmp(offset - DATA_TYPE, static_cast<char>(CRY_POINTER));
    Machine->Jne(label);

    Push(var);
    Machine->Call(GC_Extend_Generation);
    Pop(1);
  }

  //Finalize return
  Machine->Make_Label(label);

  //Collect garbage
  Machine->Call(GC_Collect);

  Machine->Return();
}
void Crystal_Compiler::Return()
{
  unsigned label = Machine->Reserve_Label();

  //Check to see if eax was loaded with something
  Machine->CmpF(RETURN_ADDRESS, 0);
  Machine->Je(label);
  CRY_ARG reg;
  Load(Addr_Reg(0));
  Machine->Load_Ptr(RETURN_ADDRESS);
  Machine->MovP(stack_size - VAR_SIZE * Addr_Reg(0) - DATA_TYPE, DATA_TYPE, true);
  Machine->Make_Label(label);
  
  //Clean up accumulated garbage
  Machine->Call(GC_Collect);

  Machine->Return();
}
void Crystal_Compiler::Loop()
{
  CryLookup new_lookup;
  //setup lookups
  new_lookup.corruptions.reserve(locals_count + stack_depth + 1);
  for(unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    new_lookup.corruptions.push_back(true);
  }
  new_lookup.loop_back_lable = Machine->Reserve_Label();
  new_lookup.lable_id = Machine->Reserve_Label();
  new_lookup.lable_block_id = new_lookup.lable_id;

  //While procedure
  Machine->Make_Label(new_lookup.loop_back_lable);
  lookups.push_back(new_lookup);
}
void Crystal_Compiler::While(unsigned var)
{
  unsigned offset_dest = stack_size - var * VAR_SIZE;
  CryLookup new_lookup = lookups.back();
  //Loops have to obscure all their symbols because we don't know what state they
  //will be in when we jump back
  for(unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    states[i].Obscurity();
  }
  //No special checking because a symbol could be nil or whatever
  //at any possible time.
  Machine->Cmp(offset_dest - DATA_LOWER, 0);
  Machine->Je(new_lookup.lable_id);
  Machine->Cmp(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
  Machine->Je(new_lookup.lable_id);
  
}
void Crystal_Compiler::If(unsigned var)
{
  unsigned offset_dest = stack_size - var * VAR_SIZE;
  CryLookup new_lookup;
  //setup lookups
  new_lookup.corruptions.reserve(locals_count + stack_depth + 1);
  for(unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    new_lookup.corruptions.push_back(false);
  }
  new_lookup.loop_back_lable = CryLookup::NO_LABLE;
  new_lookup.lable_id = Machine->Reserve_Label();
  new_lookup.lable_block_id = Machine->Reserve_Label();
  //IF procedure
  if(!(states[var].Test(CRY_NIL) && states[var].Size() == 1))
  {
    Machine->Cmp(offset_dest - DATA_LOWER, 0);
    Machine->Je(new_lookup.lable_id);
  }
  if(states[var].Test(CRY_NIL))
  {
    Machine->Cmp(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
    Machine->Je(new_lookup.lable_id);
  }
  lookups.push_back(new_lookup);
}
void Crystal_Compiler::Else()
{
  CryLookup new_lookup;
  CryLookup from_lookup = lookups.back();
  lookups.pop_back();

  //setup lookups
  new_lookup.corruptions.reserve(locals_count + stack_depth + 1);
  for(unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    new_lookup.corruptions.push_back(false);
  }
  new_lookup.loop_back_lable = CryLookup::NO_LABLE;
  new_lookup.lable_block_id = from_lookup.lable_block_id;
  new_lookup.lable_id = from_lookup.lable_block_id;

  //Else procedure
  Machine->Jmp(new_lookup.lable_block_id);
  Machine->Make_Label(from_lookup.lable_id);

  lookups.push_back(new_lookup);
}
void Crystal_Compiler::ElseIf_Pre()
{
  CryLookup new_lookup;
  CryLookup from_lookup = lookups.back();
  lookups.pop_back();

  //setup lookups
  new_lookup.corruptions.reserve(locals_count + stack_depth + 1);
  for(unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    new_lookup.corruptions.push_back(false);
  }
  new_lookup.loop_back_lable = CryLookup::NO_LABLE;
  new_lookup.lable_block_id = from_lookup.lable_block_id;
  new_lookup.lable_id = Machine->Reserve_Label();

  ////Else procedure
  Machine->Jmp(new_lookup.lable_block_id);
  Machine->Make_Label(from_lookup.lable_id);

  lookups.push_back(new_lookup);
}
void Crystal_Compiler::ElseIf(unsigned var)
{
  CryLookup new_lookup = lookups.back();

  //Else-IF procedure
  unsigned offset_dest = stack_size - var * VAR_SIZE;
  if(!(states[var].Test(CRY_NIL) && states[var].Size() == 1))
  {
    Machine->Cmp(offset_dest - DATA_LOWER, 0);
    Machine->Je(new_lookup.lable_id);
  }
  if(states[var].Test(CRY_NIL))
  {
    Machine->Cmp(offset_dest - DATA_TYPE, static_cast<char>(CRY_NIL));
    Machine->Je(new_lookup.lable_id);
  }
}

void Crystal_Compiler::End()
{
  // Create Loop backs for loops
  if(lookups.back().loop_back_lable != CryLookup::NO_LABLE)
  {
    Machine->Jmp(lookups.back().loop_back_lable);
  }
  
  // Create block ends for multi block statement.
  if(lookups.back().lable_block_id != lookups.back().lable_id)
  {
    Machine->Make_Label(lookups.back().lable_block_id);
  }

  Machine->Make_Label(lookups.back().lable_id);
  
  //obscure corruptions
  for(unsigned i = 0; i < (locals_count + stack_depth); i++)
  {
    if(lookups.back().corruptions[i])
    {
      states[i].Obscurity();
    }
  }
  lookups.pop_back();
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
    Machine->Load_Mem(offset - DATA_UPPER, val.str_);
    Machine->Load_Mem(offset - DATA_LOWER, static_cast<int>(strlen(val.str_)));
    //Machine->Load_Mem(offset - DATA_UPPER, EAX);
    break;
  }
  Machine->Load_Mem(offset - DATA_TYPE, static_cast<char>(val.type));
  states[var].Set(val.type);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}
void Crystal_Compiler::Copy(unsigned dest, unsigned source)
{
  if(dest == source)
    return;

  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  if(!(states[source].Size() == 1 && states[source].Test(CRY_NIL)))
  {
    Machine->Mov(EAX, offset_source - DATA_LOWER);
    Machine->Mov(offset_dest - DATA_LOWER, EAX);
    if(states[source].Test(CRY_DOUBLE) || states[source].Test(CRY_INT64) || 
       states[source].Test(CRY_TEXT))
    {
      Machine->Mov(EAX, offset_source - DATA_UPPER);
      Machine->Mov(offset_dest - DATA_UPPER, EAX);
    }
    
    if(states[source].Order(CRY_POINTER))
    {
      Machine->Mov(EAX, offset_source - DATA_PNTR);
      Machine->Mov(offset_dest - DATA_PNTR, EAX);
    }
    else if(states[dest].Order(CRY_POINTER))
    {
      Machine->Load_Mem(offset_dest - DATA_PNTR, NULL);
    }
  }
  Machine->Mov(EAX, offset_source - DATA_TYPE, true);
  Machine->Mov(offset_dest - DATA_TYPE, EAX, true);
  states[dest] = states[source];
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[dest] = true;
}
void Crystal_Compiler::Swap(unsigned dest, unsigned source)
{  
  //It should be noted that no refrence counters are changed
  //during a sawp unlike a copy
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  Machine->Mov(EAX, offset_source - DATA_LOWER);
  Machine->Mov(EBX, offset_dest - DATA_LOWER);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Machine->Mov(offset_source - DATA_LOWER, EBX);

  if(states[dest].Order(CRY_INT64) || states[source].Order(CRY_INT64) ||
     states[dest].Order(CRY_TEXT))
  {
    Machine->Mov(EAX, offset_source - DATA_UPPER);
    Machine->Mov(EBX, offset_dest - DATA_UPPER);
    Machine->Mov(offset_dest - DATA_UPPER, EAX);
    Machine->Mov(offset_source - DATA_UPPER, EBX);
  }
  
  if(states[dest].Order(CRY_POINTER) || states[source].Order(CRY_POINTER))
  {
    Machine->Mov(EAX, offset_source - DATA_PNTR);
    Machine->Mov(EBX, offset_dest - DATA_PNTR);
    Machine->Mov(offset_dest - DATA_PNTR, EAX);
    Machine->Mov(offset_source - DATA_PNTR, EBX);
  }

  Machine->Mov(EAX, offset_source - DATA_TYPE, true);
  Machine->Mov(EBX, offset_dest - DATA_TYPE, true);
  Machine->Mov(offset_dest - DATA_TYPE, EAX, true);
  Machine->Mov(offset_source - DATA_TYPE, EBX, true);
  Clarity_Filter temp = states[dest];
  states[dest] = states[source];
  states[source] = temp;
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
    except.Dilute(CRY_POINTER);
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
        Machine->Strcpy(EAX, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER);
        Machine->Strcpy(EDI, offset_source - DATA_UPPER, offset_source - DATA_LOWER, false, true);
        Machine->Push(EBX);
        Machine->Push(EAX);
        Push(dest);
        Machine->Call(Construct_String);
        resolve = CRY_STRING;
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
    case CRY_ARRAY:
      Push(source);
      Push(dest);
      if(left)
        Call(Crystal_Array_Append);
      else
        Call(Crystal_Array_AppendR);
      Pop(2);
      break;
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
    except.Dilute(CRY_POINTER);
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
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Load_Register(EBX, static_cast<int>(converted.length()) + 1);
        Machine->Add(EBX, EAX);
        Machine->Push(EBX);
        Machine->Call(malloc);
        Machine->Pop(4);
        if(left)
        {
          Machine->Strcpy(EAX, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER);
          Machine->Strcpy(EDI, static_cast<unsigned>(Machine->String_Address(converted.c_str())), converted.length(), true, true);
        }
        else
        {
          Machine->Strcpy(EAX, static_cast<unsigned>(Machine->String_Address(converted.c_str())), converted.length(), true);
          Machine->Strcpy(EDI, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER, false, true);
        }
        Machine->Dec(EBX);
        Machine->Push(EBX);
        Machine->Push(EAX);
        Push(dest);
        Machine->Call(Construct_String);
      }
      else
      {
        Machine->Push(static_cast<int>(strlen(const_.str_)));
        Machine->Push(Machine->String_Address(const_.str_));
        Push(dest);
        if(left)
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
      Call(left ? Crystal_Text_Append_C : Crystal_Text_Append_CR);
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
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Sub(offset_source - DATA_LOWER, EAX);
        Machine->Mov(offset_dest - DATA_LOWER, EAX);
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
        Machine->Sub(EAX, offset_dest - DATA_LOWER);
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
void Crystal_Compiler::Div(unsigned dest, unsigned source, bool left)
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
    case CRY_INT:
      Machine->Mov(EAX, (left ? offset_dest : offset_source) - DATA_LOWER);
      Machine->Idiv((left ? offset_source : offset_dest) - DATA_LOWER);
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
      Machine->FPU_Div();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Division : Obscure_DivisionR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::DivC(unsigned dest, CRY_ARG const_, bool left)
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
    case CRY_INT:
      if(left)
      {
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Load_Register(EBX, const_.num_);
      }
      else
      {
        Machine->Load_Register(EAX, const_.num_);
        Machine->Mov(EBX, offset_dest - DATA_LOWER);
      }
      Machine->Idiv(EBX);
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
      Machine->FPU_Div();
      Machine->FPU_Store(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Division : Obscure_DivisionR);
    Pop(2);
    //Copy(dest, Addr_Reg(stack_depth));
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Mod(unsigned dest, unsigned source, bool left)
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
    case CRY_INT:
      Machine->Mov(EAX, (left ? offset_dest : offset_source) - DATA_LOWER);
      Machine->Idiv((left ? offset_source : offset_dest) - DATA_LOWER);
      Machine->Mov(offset_dest - DATA_LOWER, EDX);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_DOUBLE);
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Modulo : Obscure_ModuloR);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::ModC(unsigned dest, CRY_ARG const_, bool left)
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
    case CRY_INT:
      if(left)
      {
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Load_Register(EBX, const_.num_);
      }
      else
      {
        Machine->Load_Register(EAX, const_.num_);
        Machine->Mov(EBX, offset_dest - DATA_LOWER);
      }
      Machine->Idiv(EBX);
      Machine->Mov(offset_dest - DATA_LOWER, EDX);
      break;
    NO_SUPPORT(CRY_DOUBLE);
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Modulo : Obscure_ModuloR);
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
      Machine->Jge(lablebottom, true);

      Machine->Imul(EBX, (left ? offset_dest : offset_source) - DATA_LOWER);

      Machine->Jmp(labletop, true);
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
      Machine->Jge(lablebottom, true);

      Machine->Imul(EBX, (left ? offset_dest : stack_size - Addr_Reg(stack_depth) * VAR_SIZE) - DATA_LOWER);

      Machine->Jmp(labletop, true);
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
void Crystal_Compiler::And(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Push(source);
  Push(dest);
  Machine->Call(Crystal_And);
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}
void Crystal_Compiler::AndC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Load(Addr_Reg(stack_depth), const_);
  Push(Addr_Reg(stack_depth));
  Push(dest);
  Machine->Call(Crystal_And);
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}
void Crystal_Compiler::Or(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Push(source);
  Push(dest);
  Machine->Call(Crystal_Or);
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}
void Crystal_Compiler::OrC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Load(Addr_Reg(stack_depth), const_);
  Push(Addr_Reg(stack_depth));
  Push(dest);
  Machine->Call(Crystal_Or);
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}
void Crystal_Compiler::Eql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || states[source].Test(CRY_NIL))
    {
      if(states[dest].Test(CRY_NIL) && states[source].Test(CRY_NIL))
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }
    if(!states[dest].Compare(states[source]))
    {
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
    case CRY_BOOL:
      if(states[dest].Test(CRY_INT) == states[source].Test(CRY_INT))
      {
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Cmp(offset_source - DATA_LOWER, EAX);
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
        Machine->Sete(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      break;
    case CRY_DOUBLE:
      Machine->FPU_Loadd(offset_source - DATA_LOWER);
      Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      Machine->Sete(offset_dest - DATA_LOWER);
      break;
    case CRY_TEXT:
    case CRY_STRING:
      if((states[dest].Test(CRY_TEXT) || states[source].Test(CRY_TEXT)) &&
         (states[dest].Test(CRY_STRING) || states[source].Test(CRY_STRING)))
      {
        Push(source);
        Push(dest);
        Machine->Call(Fast_strcmp);
        Pop(2);
        Machine->Mov(offset_dest - DATA_LOWER, EAX);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      break;
    //Lacking Support
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(Obscure_Equal);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::EqlC(unsigned dest, CRY_ARG const_, bool left)
{  
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || const_.filt.Test(CRY_NIL))
    {
      if(states[dest].Test(CRY_NIL) && const_.filt.Test(CRY_NIL))
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }
    if(!states[dest].Compare(const_.filt))
    {
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      const_.num_ = const_.bol_;
    case CRY_INT:
      if(states[dest].Test(CRY_INT) == const_.filt.Test(CRY_INT))
      {
        Machine->Load_Register(EAX, const_.num_);
        Machine->Cmp(offset_dest - DATA_LOWER, EAX);
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
        Machine->Sete(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      break;
    case CRY_DOUBLE:
      Machine->FPU_Load(const_.dec_);
      Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      Machine->Sete(offset_dest - DATA_LOWER);
      break;
    case CRY_TEXT:
    case CRY_STRING:
      Load(Addr_Reg(stack_depth), const_);
      Push(Addr_Reg(stack_depth));
      Push(dest);
      Machine->Call(Fast_strcmp);
      Pop(2);
      Machine->Mov(offset_dest - DATA_LOWER, EAX);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(Obscure_Equal);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }

}
void Crystal_Compiler::Dif(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || states[source].Test(CRY_NIL))
    {
      if(states[dest].Test(CRY_NIL) && states[source].Test(CRY_NIL))
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }
    if(!states[dest].Compare(states[source]))
    {
      Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
    case CRY_BOOL:
      if(states[dest].Test(CRY_INT) == states[source].Test(CRY_INT))
      {
        Machine->Mov(EAX, offset_dest - DATA_LOWER);
        Machine->Cmp(offset_source - DATA_LOWER, EAX);
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
        Machine->Setne(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      break;
    case CRY_DOUBLE:
      Machine->FPU_Loadd(offset_source - DATA_LOWER);
      Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      Machine->Setne(offset_dest - DATA_LOWER);
      break;
    case CRY_TEXT:
    case CRY_STRING:
      if((states[dest].Test(CRY_TEXT) || states[source].Test(CRY_TEXT)) &&
         (states[dest].Test(CRY_STRING) || states[source].Test(CRY_STRING)))
      {
        Push(source);
        Push(dest);
        Machine->Call(Fast_strcmp);
        Pop(2);
        Machine->Mov(offset_dest - DATA_LOWER, EAX);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      break;
    //Lacking Support
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Push(source);
    Push(dest);
    Machine->Call(Obscure_Diffrence);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::DifC(unsigned dest, CRY_ARG const_, bool left)
{  
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    //null operation
    if(states[dest].Test(CRY_NIL) || const_.filt.Test(CRY_NIL))
    {
      if(states[dest].Test(CRY_NIL) && const_.filt.Test(CRY_NIL))
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }
    
    if(!states[dest].Compare(const_.filt))
    {
      Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      Runtime_Resovle(dest, CRY_BOOL);
      return;
    }

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_BOOL:
      const_.num_ = const_.bol_;
    case CRY_INT:
      if(states[dest].Test(CRY_INT) == const_.filt.Test(CRY_INT))
      {
        Machine->Load_Register(EAX, const_.num_);
        Machine->Cmp(offset_dest - DATA_LOWER, EAX);
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
        Machine->Setne(offset_dest - DATA_LOWER);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 1);
      }
      break;
    case CRY_DOUBLE:
      Machine->FPU_Load(const_.dec_);
      Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      Machine->Setne(offset_dest - DATA_LOWER);
      break;
    case CRY_TEXT:
    case CRY_STRING:
      Load(Addr_Reg(stack_depth), const_);
      Push(Addr_Reg(stack_depth));
      Push(dest);
      Machine->Call(Fast_strcmp);
      Pop(2);
      Machine->Mov(offset_dest - DATA_LOWER, EAX);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(Obscure_Diffrence);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Les(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(!left)
        Machine->Setl(offset_dest - DATA_LOWER);
      else
        Machine->Setg(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT))
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setb(offset_dest - DATA_LOWER);
      else
        Machine->Seta(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Less : Obscure_Greater);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::LesC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(!left)
        Machine->Setl(offset_dest - DATA_LOWER);
      else
        Machine->Setg(offset_dest - DATA_LOWER);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT))
        Machine->FPU_Load(const_.num_);
      else
        Machine->FPU_Load(const_.dec_);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setb(offset_dest - DATA_LOWER);
      else
        Machine->Seta(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(left ? Obscure_Less : Obscure_Greater);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::LesEql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(!left)
        Machine->Setle(offset_dest - DATA_LOWER);
      else
        Machine->Setge(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT))
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setbe(offset_dest - DATA_LOWER);
      else
        Machine->Setae(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(left ? Obscure_Less_Equal : Obscure_Greater_Equal);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::LesEqlC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(!left)
        Machine->Setle(offset_dest - DATA_LOWER);
      else
        Machine->Setge(offset_dest - DATA_LOWER);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT))
        Machine->FPU_Load(const_.num_);
      else
        Machine->FPU_Load(const_.dec_);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setbe(offset_dest - DATA_LOWER);
      else
        Machine->Setae(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(left ? Obscure_Less_Equal : Obscure_Greater_Equal);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Gtr(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(left)
        Machine->Setl(offset_dest - DATA_LOWER);
      else
        Machine->Setg(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT))
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setb(offset_dest - DATA_LOWER);
      else
        Machine->Seta(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(!left ? Obscure_Less : Obscure_Greater);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::GtrC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(left)
        Machine->Setl(offset_dest - DATA_LOWER);
      else
        Machine->Setg(offset_dest - DATA_LOWER);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT))
        Machine->FPU_Load(const_.num_);
      else
        Machine->FPU_Load(const_.dec_);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setb(offset_dest - DATA_LOWER);
      else
        Machine->Seta(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(!left ? Obscure_Less : Obscure_Greater);
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::GtrEql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1)
  {

    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], states[source]);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Mov(EAX, offset_source - DATA_LOWER);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(left)
        Machine->Setle(offset_dest - DATA_LOWER);
      else
        Machine->Setge(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    case CRY_DOUBLE:
      if(states[source].Test(CRY_INT))
        Machine->FPU_Loadi(offset_source - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_source - DATA_LOWER);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setbe(offset_dest - DATA_LOWER);
      else
        Machine->Setae(offset_dest - DATA_LOWER);
      resolve = CRY_BOOL;
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
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
    Machine->Call(!left ? Obscure_Less_Equal : Obscure_Greater_Equal);
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::GtrEqlC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1)
  {
    Symbol_Type resolve = Clarity_Filter::Reduce(states[dest], const_.filt);
    switch(resolve)
    {
    case CRY_INT:
      Machine->Load_Register(EAX, const_.num_);
      Machine->Cmp(offset_dest - DATA_LOWER, EAX);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      if(left)
        Machine->Setle(offset_dest - DATA_LOWER);
      else
        Machine->Setge(offset_dest - DATA_LOWER);
      break;
    case CRY_DOUBLE:
      if(const_.filt.Test(CRY_INT))
        Machine->FPU_Load(const_.num_);
      else
        Machine->FPU_Load(const_.dec_);
      if(states[dest].Test(CRY_INT))
        Machine->FPU_Loadi(offset_dest - DATA_LOWER);
      else
        Machine->FPU_Loadd(offset_dest - DATA_LOWER);
      Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      Machine->FPU_Cmp();
      if(left)
        Machine->Setbe(offset_dest - DATA_LOWER);
      else
        Machine->Setae(offset_dest - DATA_LOWER);
      break;
    //Lacking Support
    NO_SUPPORT(CRY_BOOL);
    NO_SUPPORT(CRY_TEXT);
    NO_SUPPORT(CRY_STRING);
    NO_SUPPORT(CRY_ARRAY);
    NO_SUPPORT(CRY_POINTER);
    }
    Runtime_Resovle(dest, CRY_BOOL);
  }
  //Clarity Handling
  else
  {
    Load(Addr_Reg(stack_depth), const_);
    Push(Addr_Reg(stack_depth));
    Push(dest);
    Machine->Call(!left ? Obscure_Less_Equal : Obscure_Greater_Equal);
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
  if(states[dest].Size() != 1 || !states[dest].Test(resolve))
  {
    states[dest].Set(resolve);
    if(resolve > CRY_POINTER)
      resolve = CRY_POINTER;
    Machine->Load_Mem(stack_size - VAR_SIZE * dest - DATA_TYPE, static_cast<char>(resolve));
  }
}