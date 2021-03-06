#include "Compiler.h"
#include "Lexicon.h"
#include "Obscure.h"
#include "Helper.h"
#include "Function.h"
#include "Garbage_Collector.h"
#include "Linker.h"

extern std::unordered_map<std::string, unsigned> late_bindings;
Crystal_Library Crystal_Linker::library;

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
    str_ = sym->str;
    break;
  }
  filt.Set(type);
}

Crystal_Compiler::Crystal_Compiler(AOT_Compiler* target)
{
  Machine = target;
  program.load = (byte*)VirtualAllocEx( GetCurrentProcess(), 0, 1<<16, MEM_COMMIT | MEM_RESERVE , PAGE_EXECUTE_READWRITE);
  program.base = program.load;
  Machine->Setup(program.load);
}

Crystal_Compiler::~Crystal_Compiler()
{
  VirtualFreeEx(GetCurrentProcess(), program.base, 1<<16, MEM_RELEASE);
  delete Machine;
}

void Crystal_Compiler::Start_Encode(std::string name, unsigned locals_used, unsigned stack_count, unsigned arguments, Class_Info* obj, unsigned id)
{
  Machine->Function(name, program.load);

  // Set lookups for the class object if we are in one.
  if(obj)
  {
    obj->lookup[id].function = program.load;
    class_encoding = true;

    // Include the implicit "this" object
    arguments++;
  }
  else
  {
    class_encoding = false;
  }

  //Local and stack depth.
  locals_count = locals_used;
  stack_depth = stack_count;
  if(stack_depth == 0)
    stack_depth = 1;

  // Setup the local stack.
  stack_size = (locals_count + stack_depth + 1) * VAR_SIZE + 4;
  Machine->Allocate_Stack(stack_size);
  states.resize(locals_count + stack_depth + 1);
  
  // If it's an object member function then copy over
  // the "this" object as a reference.
  unsigned i = 0;
  if(obj)
  {
    Machine->Push_Stk(i * 4 + 0x14);
    Push(i);
    Runtime("This_Copy");
    Pop(2);
    states[0].Set(CRY_REFERENCE);
    i++;
  }

  // Copy over all the function objects onto the stack
  // TODO:
  //  Optimize with with a block copy to increase speed
  //  and reduce code bloat.
  for(; i < locals_count + stack_depth + 1; i++)
  {
    if(i < arguments)
    {
      Machine->Push_Stk(i * 4 + 0x14);
      Push(i);
      Runtime("Stack_Copy");
      Pop(2);
    }
    states[i].Obscurity();
  }

  //set up a new generation for the garbage collector.
  Machine->Push(static_cast<int>(locals_count + stack_depth + 1));
  Push(0);
  Machine->Runtime("GC_Branch");
  Pop(2);

  //tidy up lookup data.
  lookups.clear();

  // Set the "this" object to a reference for member functions.
  if(obj)
    states[0].Set(CRY_REFERENCE);
}

void Crystal_Compiler::End_Encode()
{
  //Resolve 
  Return();

  //Construct Package
  if(!class_encoding)
  {
    std::unordered_map<std::string, PackageLinks>* links = Machine->Get_Links();
    (*links)[Machine->Get_Name()].package_offset = program.load - program.base;
  }

  // Move to the next segment of memory so we can start loading
  // our next function.
  program.load = Machine->Location() + 1;
}

void Crystal_Compiler::Linker()
{
  // Grab the functions from the machine.
  linker.Set_Functions(Machine->Get_Links());

  // Grab all double locations from the machine.
  linker.Set_Doubles(Machine->Get_Doubles());
  
  // Grab all string locations from the machine.
  linker.Set_Strings(Machine->Get_Strings());

  // Grab all of the internal lookups
  linker.Set_Internal(Machine->Get_Internals());

  // Link the program directly so it can be executed later.
  linker.Link(program.base, program.load - program.base);
}

void Crystal_Compiler::Read_Binary(const char* exe_name)
{
  linker.Read(exe_name);
}

void Crystal_Compiler::Write_Binary(const char* exe_name)
{
  linker.Write(exe_name, program.base, program.load - program.base);
}

int Crystal_Compiler::Execute(Crystal_Symbol* ret, std::vector<Crystal_Symbol>* args)
{
  return Execute(ret, args, "main");
}

int Crystal_Compiler::Execute(Crystal_Symbol* ret, std::vector<Crystal_Symbol>* args, const char* entry_point)
{
  // Grab the entry point and run it.
  CryProg entry;
  entry.load = linker.Function(entry_point);

  if (entry.load == NULL)
  {
    ret->type = CRY_NIL;
    return 0;
  }

  // Load arguments for "main"
  for (int i = static_cast<int>(args->size()) - 1; i >= 0; i--)
  {
    Crystal_Symbol* sym = &(*args)[i];
    __asm{
      mov eax, sym
      push eax
    }
  }
 
  // Call "main"
  int return_value = entry.call(ret);

  // Pop off the added values.
  for (int i = static_cast<int>(args->size()) - 1; i >= 0; i--)
  {
    Crystal_Symbol* sym = &(*args)[i];
    __asm{
      pop eax
    }
  }

  return return_value;
}

void Crystal_Compiler::Print(unsigned var)
{
  Push(var);
  Runtime("Crystal_Print");
  Pop(1);
}

void Crystal_Compiler::Runtime(const char* runtime_function)
{
  Machine->Runtime(runtime_function);
}

void Crystal_Compiler::Runtime(const char* runtime_function, unsigned var)
{
  Push(var);
  Machine->Runtime(runtime_function);
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

void Crystal_Compiler::Call(const char* binding, unsigned op, unsigned ret)
{
  Push(op);
  Machine->Push(static_cast<int>(late_bindings[binding]));
  
  // Seperate calling for base and refrence objects
  if(states[op].Only(CRY_REFERENCE))
    Machine->Runtime("Late_Func_Binding_Ref");
  else
    Machine->Runtime("Late_Func_Binding");

  Pop(1);

  if(ret != CRY_NULL)
  {
    unsigned offset = stack_size - VAR_SIZE * ret;
    Machine->Lea(EBX, offset);
  }
  else
  {
    Machine->Load_Register(EBX, 0);
  }

  //Call Crystal function
  Machine->Push(EBX);
  Machine->Call(EAX);
  if(ret != CRY_NULL)
  {
    states[ret].Obscurity();
  }
  Pop(2);
}

void Crystal_Compiler::Get(const char* binding, unsigned op, unsigned ret)
{
  Push(ret);
  Push(op);
  Machine->Push(static_cast<int>(late_bindings[binding]));

  // Seperate calling for base and refrence objects
  if(states[op].Only(CRY_REFERENCE))
    Machine->Runtime("Late_Attr_Binding_Ref");
  else
    Machine->Runtime("Late_Attr_Binding");

  Pop(3);

  states[ret].Set(CRY_REFERENCE);
}

void Crystal_Compiler::Convert(unsigned reg, Symbol_Type type)
{
  Push(reg);
  switch(type)
  {
  case CRY_INT:
    Runtime("Parse_Int");
    Pop(1);
    Machine->Push(EAX);
    break;
  case CRY_DOUBLE:
    Runtime("Parse_Double");
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
  Machine->Runtime("calloc");
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
  Machine->Runtime("Construct_Array");
  Machine->Pop(sizeof(int) * 4);
  //Set up types for compiler use
  states[var].Set(CRY_ARRAY);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}

void Crystal_Compiler::Make_Class(unsigned var, unsigned ID, unsigned args)
{
  //Creation of the array object
  Push(var);
  Machine->Push(static_cast<int>(ID));
  Machine->Runtime("Construct_Class");
  Machine->Pop(sizeof(int));

  // Call Init function
  Machine->Push((int) late_bindings["init"]);
  
  // Seperate calling for base and refrence objects
  if(states[var].Only(CRY_REFERENCE))
    Machine->Runtime("Late_Func_Binding_Ref");
  else
    Machine->Runtime("Late_Func_Binding");
  Machine->Pop(sizeof(int));

  // Init functions can't return.
  unsigned label = Machine->Reserve_Label();
  Machine->CmpZero(EAX);
  Machine->Je(label, true);
  Machine->Call(EAX);
  Machine->Make_Label(label);
  Pop(2 + args);

  //Set up types for compiler use
  states[var].Set(CRY_CLASS_OBJ);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}

void Crystal_Compiler::Make_Range(unsigned var)
{
  //Creation of the array object
  Push(var);
  Machine->Runtime("Construct_Range");
  Machine->Pop(sizeof(int) * 3);
  //Set up types for compiler use
  states[var].Set(CRY_ARRAY);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}

void Crystal_Compiler::Array_Index(unsigned dest, unsigned var, unsigned index)
{
  Convert(index, CRY_INT);
  Push(var);
  Push(dest);
  Runtime("Val_Binding");
  Pop(3);

  states[dest].Set(CRY_REFERENCE);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[dest] = true;
}

void Crystal_Compiler::Array_Index_C(unsigned dest, unsigned var, CRY_ARG index)
{    
  //Reference
  switch(index.type)
  {
  case CRY_NIL:
    Push_C(0);
    break;
  case CRY_BOOL:
    Push_C(static_cast<int>(index.bol_));
    break;
  case CRY_INT:
    Push_C(index.num_);
    break;
  case CRY_DOUBLE:
    Push_C(static_cast<int>(index.dec_));
    break;
  case CRY_TEXT:
    Push_C(atoi(index.str_.c_str()));
    break;
  }

  Push(var);
  Push(dest);
  Runtime("Val_Binding");
  Pop(3);
  
  states[dest].Set(CRY_REFERENCE);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[dest] = true;
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
    Machine->Push(var.str_.c_str());
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
    Machine->Runtime("GC_Extend_Generation");
    Pop(1);
  }

  //Finalize return
  Machine->Make_Label(label);

  //Collect garbage
  Machine->Runtime("GC_Collect");

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
  Machine->Runtime("GC_Collect");

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
  new_lookup.indexed = false;

  //While procedure
  Machine->Make_Label(new_lookup.loop_back_lable);
  lookups.push_back(new_lookup);
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
  new_lookup.indexed = false;
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

void Crystal_Compiler::For()
{
  Machine->Load_Register(EBX, 0);
  Loop();
}

void Crystal_Compiler::In(unsigned target, unsigned container)
{
  CryLookup new_lookup = lookups.back();
  lookups.back().indexed = true;

  //Loops have to obscure all their symbols because we don't know what state they
  //will be in when we jump back
  for (unsigned i = 0; i < (locals_count + stack_depth + 1); i++)
  {
    states[i].Obscurity();
  }

  //No special checking because a symbol could be nil or whatever
  //at any possible time.
  Push(container);
  Machine->Runtime("Crystal_Elements");
  Pop(1);
  Machine->Cmp(EBX, EAX);
  Machine->Jge(new_lookup.lable_id);

  // Aquire the reference
  Machine->Push(EBX);
  Push(container);
  Push(target);
  Runtime("Val_Binding");
  Pop(3);

  states[target].Set(CRY_REFERENCE);
  //Corrupt the state of the object
  if (lookups.size())
    lookups.back().corruptions[target] = true;

  Machine->Inc(EBX);
  Machine->Push(EBX);
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
  new_lookup.indexed = false;

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
  new_lookup.indexed = false;

  //Else procedure
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
    // Pop the value for ECX for indexed loops to restore
    // the counter state of the loop.
    if (lookups.back().indexed)
    {
      Machine->Pop(EBX);
    }

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
  if(states[var].Only(CRY_REFERENCE))
  {
    Ref_Load(var, val);
    return;
  }

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
    Machine->Load_Mem(offset - DATA_UPPER, val.str_.c_str());
    Machine->Load_Mem(offset - DATA_LOWER, static_cast<int>(val.str_.size()));
    //Machine->Load_Mem(offset - DATA_UPPER, EAX);
    break;
  }
  Machine->Load_Mem(offset - DATA_TYPE, static_cast<char>(val.type));
  states[var].Set(val.type);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}

void Crystal_Compiler::Ref_Load(unsigned var, CRY_ARG val)
{
  Push(var);
  switch(val.type)
  {
  case CRY_INT:
    Push_C(val.num_);
    Runtime("Push_Int");
    Pop(2);
    break;
  case CRY_BOOL:
    Push_C(val.bol_);
    Runtime("Push_Bool");
    Pop(2);
    break;
  case CRY_DOUBLE:
    Push_C(val.dec_);
    Runtime("Push_Double");
    Pop(3);
    break;
  case CRY_TEXT:
    Push_C(val.str_.c_str());
    Runtime("Push_Text");
    Pop(2);
    break;
  }

  states[var].Set(val.type);
  //Corrupt the state of the object
  if(lookups.size())
    lookups.back().corruptions[var] = true;
}

void Crystal_Compiler::Copy(unsigned dest, unsigned source)
{
  if(dest == source)
    return;

  if(states[dest].Only(CRY_REFERENCE) || states[source].Only(CRY_REFERENCE))
  {
    Push(source);
    Push(dest);
    Runtime("Copy_Ref");
    Pop(2);
  }
  else 
  {
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
  }

  if(states[source].Only(CRY_REFERENCE))
  {
    states[dest].Obscurity();
  }
  else
  {
    states[dest] = states[source];
  }

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

void Crystal_Compiler::Not(unsigned dest)
{
  Push(dest);
  Machine->Runtime("Obscure_Not");
  Pop(1);
}

void Crystal_Compiler::Add(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 &&
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
        Machine->Runtime("malloc");
        Machine->Pop(4);
        Machine->Strcpy(EAX, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER);
        Machine->Strcpy(EDI, offset_source - DATA_UPPER, offset_source - DATA_LOWER, true);
        Machine->Push(EBX);
        Machine->Push(EAX);
        Push(dest);
        Machine->Runtime("Construct_String");
        resolve = CRY_STRING;
      }
      //complex slower text handling:
      else
      {
        Push(source);
        Push(dest);
        if(left)
          Runtime("Crystal_Text_Append");
        else
          Runtime("Crystal_Text_Append_Rev");
        Pop(2);
      }
      //we now have a string.
      resolve = CRY_STRING;
      break;
    case CRY_STRING:
      Push(source);
      Push(dest);
      if(left)
        Runtime("Crystal_Text_Append");
      else
        Runtime("Crystal_Text_Append_Rev");
      Pop(2);
      break;
    case CRY_ARRAY:
      Push(source);
      Push(dest);
      if(left)
        Runtime("Crystal_Array_Append");
      else
        Runtime("Crystal_Array_Append_Rev");
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
    Machine->Runtime(left ? "Obscure_Addition" : "Obscure_AdditionR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}

void Crystal_Compiler::AddC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
        Machine->Runtime("malloc");
        Machine->Pop(4);
        if(left)
        {
          Machine->Strcpy(EAX, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER);
          Machine->Strcpy(EDI, converted.c_str(), converted.length(), true);
        }
        else
        {
          Machine->Strcpy(EAX, converted.c_str(), converted.length());
          Machine->Strcpy(EDI, offset_dest - DATA_UPPER, offset_dest - DATA_LOWER, true);
        }
        Machine->Dec(EBX);
        Machine->Push(EBX);
        Machine->Push(EAX);
        Push(dest);
        Machine->Runtime("Construct_String");
      }
      else
      {
        Machine->Push(static_cast<int>(const_.str_.size()));
        Machine->Push(const_.str_.c_str());
        Push(dest);
        if(left)
          Runtime("Crystal_Const_Append_T");
        else
          Runtime("Crystal_Const_Append_TL");
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
        Machine->Push(Machine->String_Address(converted.c_str(), -1));
      }
      Push(dest);
      Call(left ? "Crystal_Text_Append_C" : "Crystal_Text_Append_CR");
      Pop(3);
      break;
    case CRY_ARRAY:
      Load(Addr_Reg(stack_depth), const_);
      Push(Addr_Reg(stack_depth));
      Push(dest);
      if(left)
        Call("Crystal_Array_Append");
      else
        Call("Crystal_Array_Append_Rev");
      Pop(2);
      break;
    //Lacking Support
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
    Machine->Runtime(left ? "Obscure_Addition" : "Obscure_AdditionR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Sub(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Subtraction" : "Obscure_SubtractionR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::SubC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Subtraction" : "Obscure_SubtractionR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Mul(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime("Obscure_Multiplication");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::MulC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime("Obscure_Multiplication");
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
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Division" : "Obscure_DivisionR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::DivC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Division" : "Obscure_DivisionR");
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
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Modulo" : "Obscure_ModuloR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::ModC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Modulo" : "Obscure_ModuloR");
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
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
      Machine->Runtime(left ? "Power_Syms" : "Power_SymsR");
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
    Machine->Runtime(left ? "Obscure_Power" : "Obscure_PowerR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::PowC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned labletop, lablebottom;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
      Machine->Runtime(left ? "Power_Syms" : "Power_SymsR");
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
    Machine->Runtime(left ? "Obscure_Power" : "Obscure_PowerR");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}

void Crystal_Compiler::And(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Push(source);
  Push(dest);
  Machine->Runtime("Crystal_And");
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
  Machine->Runtime("Crystal_And");
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}

void Crystal_Compiler::Or(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  Push(source);
  Push(dest);
  Machine->Runtime("Crystal_Or");
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
  Machine->Runtime("Crystal_Or");
  Pop(2);
  Machine->Mov(offset_dest - DATA_LOWER, EAX);
  Runtime_Resovle(dest, CRY_BOOL);
}

void Crystal_Compiler::Eql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
        Machine->Runtime("Fast_strcmp");
        Pop(2);
        Machine->Mov(offset_dest - DATA_LOWER, EAX);
      }
      else
      {
        Machine->Load_Mem(offset_dest - DATA_LOWER, 0);
      }
      break;
    case CRY_CLASS_OBJ:
      Push(source);
      Push(dest);
      Machine->Runtime("Fast_pointercmp");
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
    Push(source);
    Push(dest);
    Machine->Runtime("Obscure_Equal");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::EqlC(unsigned dest, CRY_ARG const_, bool left)
{  
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
      Machine->Runtime("Fast_strcmp");
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
    Machine->Runtime("Obscure_Equal");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }

}
void Crystal_Compiler::Dif(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
        Machine->Runtime("Fast_strcmp");
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
    Machine->Runtime("Obscure_Diffrence");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::DifC(unsigned dest, CRY_ARG const_, bool left)
{  
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
      Machine->Runtime("Fast_strcmp");
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
    Machine->Runtime("Obscure_Diffrence");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Les(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Less" : "Obscure_Greater");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::LesC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Less" : "Obscure_Greater");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::LesEql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Less_Equal" : "Obscure_Greater_Equal");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::LesEqlC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(left ? "Obscure_Less_Equal" : "Obscure_Greater_Equal");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::Gtr(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(!left ? "Obscure_Less" : "Obscure_Greater");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::GtrC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(!left ? "Obscure_Less" : "Obscure_Greater");
    Pop(2);
    Clarity_Filter::Combind(states[dest], const_.filt);
  }
}
void Crystal_Compiler::GtrEql(unsigned dest, unsigned source, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;
  unsigned offset_source = stack_size - source * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && states[source].Size() == 1 && 
    !states[dest].Only(CRY_REFERENCE) && !states[source].Only(CRY_REFERENCE))
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
    Machine->Runtime(!left ? "Obscure_Less_Equal" : "Obscure_Greater_Equal");
    Pop(2);
    Clarity_Filter::Combind(states[dest], states[source]);
  }
}
void Crystal_Compiler::GtrEqlC(unsigned dest, CRY_ARG const_, bool left)
{
  unsigned offset_dest = stack_size - dest * VAR_SIZE;

  //std static handling
  if(states[dest].Size() == 1 && const_.filt.Size() == 1 && !states[dest].Only(CRY_REFERENCE))
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
    Machine->Runtime(!left ? "Obscure_Less_Equal" : "Obscure_Greater_Equal");
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