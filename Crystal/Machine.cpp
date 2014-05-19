#include "Machine.h"
#pragma warning(disable: 4996)

BYTE AOT_Compiler::two_complement_8(unsigned char id)
{
  return static_cast<BYTE>((0xFF - id) + 1);
}
unsigned AOT_Compiler::two_complement_32(unsigned id)
{
  return (0xFFFFFFFF - id) + 1;
}

AOT_Compiler::AOT_Compiler()
{
  //Obviously since these are used in direct addressing
  //we can't have them go and reallocating themselves.
  //When they grow we will get a copy and we don't want that.
  esp.reserve(STACK_SIZE);
  efp.reserve(STACK_SIZE);
  edp.reserve(STACK_SIZE);
}
void AOT_Compiler::Setup(std::string name, BYTE* program)
{
  p = program;
  prg_id = name;
  call_links.clear();
}
void AOT_Compiler::Make_Label(unsigned label)
{
  AOT_Var l;
  l.lab = label;
  l.adr = (unsigned)p;
  cls.push_back(l);
  std::vector<AOT_Var>::iterator walker = cjs.begin();
  //search backwards for foward jumps
  while(walker != cjs.end())
  {
    if(walker->lab == label)
    {
      unsigned distance = l.adr - walker->adr - 1;
      (walker->loc)[0] = distance;
    }
    walker++;
  }
}
unsigned AOT_Compiler::New_Label()
{
  return cls.size();
}
void AOT_Compiler::Cmp(unsigned address, ARG argument)
{
  //switch(argument.type)
  //{
  //case AOT_INT:
  //  if(argument.num_ > 0x7F)
  //  {
  //    *p++ = 0x83;
  //    *p++ = 0x7D;
  //    *p++ = two_complement_8(address);
  //    *p++ = (unsigned char)argument.num_;
  //  }
  //  else
  //  {
  //    *p++ = 0x81;
  //    *p++ = 0x7D;
  //    *p++ = two_complement_8(address);
  //    (int&)p[0] = argument.num_; p+= sizeof(int);
  //  }
  //  break;
  //}
}
void AOT_Compiler::CmpF(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    if(argument.num_ > 0x7F)
    {
      *p++ = 0x83;
      *p++ = 0x7D;
      *p++ = static_cast<char>(address);
      *p++ = (unsigned char)argument.num_;
    }
    else
    {
      *p++ = 0x81;
      *p++ = 0x7D;
      *p++ = static_cast<char>(address);
      (int&)p[0] = argument.num_; p+= sizeof(int);
    }
    break;
  }
}
void AOT_Compiler::Jmp(unsigned label)
{
  *p++ = 0xEB;  
  Label_Management(label);
}
void AOT_Compiler::Je(unsigned label)
{
  *p++ = 0x74;
  Label_Management(label);
}
void AOT_Compiler::Jne(unsigned label)
{
  *p++ = 0x75;
  Label_Management(label);
}
void AOT_Compiler::Jle(unsigned label)
{
  *p++ = 0x7E;
  Label_Management(label);
}
void AOT_Compiler::Jl(unsigned label)
{
  *p++ = 0x7C;
  Label_Management(label);
}
void AOT_Compiler::Jg(unsigned label)
{
  *p++ = 0x7F;
  Label_Management(label);
}
void AOT_Compiler::Allocate_Stack(unsigned bytes)
{
  //Strack preamble
  Push(EDX);
  *p++ = 0x55; //push ebp

  *p++ = 0x8B;
  *p++ = 0xEC; //mov ebp, esp

  stack_size = bytes;

  if(stack_size < 0x7F)
  {
    *p++ = 0x83;
    *p++ = 0xEC;
    *p++ = (unsigned char)(stack_size);
    *p++ = 0x6A;
    *p++ = (unsigned char)(stack_size);
  }
  else
  {
    *p++ = 0x81;
    *p++ = 0xEC; //sub esp
    (int&)p[0] = stack_size; p+= sizeof(int);
    *p++ = 0x68;
    (int&)p[0] = stack_size; p+= sizeof(int);
  }

  //Get address
  *p++ = 0x8D; *p++ = 0x44; //LEA
  *p++ = 0x24; //EAX
  *p++ = 0x04; // 4 bytes pushed already
  //Clear it up
  *p++ = 0x6A;
  *p++ = 0x00;
  //Push the address last
  *p++ = 0x50;
  //Construct the symbols
  Call(memset);
  //Restore stack frame
  *p++ = 0x83; *p++ = 0xC4; *p++ = 0x0C; //12 bytes
  stack_allocated = true;
}  
void AOT_Compiler::Print(ARG argument)
{
  if(argument.type == AOT_FLOAT)
  {
    Push((double)argument.flt_);
  }
  else
  {
    Push(argument);
  }
  *p++ = 0x68;
  switch(argument.type)
  {
  case AOT_FLOAT:
  case AOT_DOUBLE:
    (int&)p[0] = (int)"%f\n"; p+= sizeof(int);
    break;
  case AOT_INT:
  default:
    (int&)p[0] = (int)"%d\n"; p+= sizeof(int);
    break;
  }
  Call(printf);
  Pop();
}
void AOT_Compiler::Push(ARG argument)
{
  pushed_bytes += 4;
  switch(argument.type)
  {
  case AOT_REG:
    switch(argument.reg_)
    {
    case EAX:
      *p++ = 0x50;
      return;
    case EBX:
      *p++ = 0x53;
      return;
    case ECX:
      *p++ = 0x51;
      return;
    case EDX:
      *p++ = 0x52;
      return;
    }
    break;
  case AOT_INT:
    if(abs(argument.num_) < 0x7F)
    {
      *p++ = 0x6A;
      *p++ = (unsigned char)(argument.num_);
    }
    else
    {
      *p++ = 0x68;
      (int&)p[0] = (unsigned)(argument.num_); p+= sizeof(int);
    }
    return;
  case AOT_DOUBLE:

    //Double
    *p++ = 0xDD;
    *p++ = 0x05;
    (int&)p[0] = (int)Double_Address(argument.dec_); p+= sizeof(int);

    //Make room
    *p++ = 0x83;
    *p++ = 0xEC;
    *p++ = 0x08;

    //store on stack
    //double
    *p++ = 0xDD;
    *p++ = 0x1C;
    *p++ = 0x24;
    pushed_bytes += 4;
    return;
  case AOT_FLOAT:
    //Float
    *p++ = 0xD9;
    *p++ = 0x05;
    (int&)p[0] = (int)Float_Address(argument.flt_); p+= sizeof(int);

    //Make room
    *p++ = 0x83;
    *p++ = 0xEC;
    *p++ = 0x04;

    //store on stack
    //Float
    *p++ = 0xD9;
    *p++ = 0x1C;
    *p++ = 0x24;

    //pushed_bytes += 4;
    return;
  }
}
void AOT_Compiler::Push_Adr(unsigned address)
{
  *p++ = 0x8D;
  if(address < 0x7F)
  {
    *p++ = EAX - WORD_VARIANT;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = EAX;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
  *p++ = 0x50; 
  pushed_bytes += 4;   
}
void AOT_Compiler::Pop(unsigned bytes)
{
  if(!bytes)
  {
    return;
  }
  else
  {
    *p++ = 0x83;
    *p++ = 0xC4;
    *p++ = (unsigned char)(bytes);
    pushed_bytes -= bytes;
  }
}
void AOT_Compiler::Load_Mem(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    *p++ = 0xC7;
    if(address < 0x7F)
    {
      *p++ = 0x45;
      *p++ = two_complement_8(address);
    }
    else
    {
      *p++ = 0x85;
      (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
    }
    (int&)p[0] = argument.num_; p+= sizeof(int);
    return;
  case AOT_STRING:
    *p++ = 0xC7;
    if(address < 0x7F)
    {
      *p++ = 0x45;
      *p++ = two_complement_8(address);
    }
    else
    {
      *p++ = 0x85;
      (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
    }
    (int&)p[0] = (int)argument.str_; p+= sizeof(int);
    return;
  case AOT_BOOL:
  case AOT_CHAR:
    *p++ = 0xC6;
    if(address < 0x7F)
    {
      *p++ = 0x45;
      *p++ = two_complement_8(address);
    }
    else
    {
      *p++ = 0x85;
      (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
    }
    *p++ = static_cast<BYTE>(argument.chr_);
    return;
  case AOT_FLOAT:
    Load_Mem(address, static_cast<double>(argument.flt_));
    return;
  case AOT_DOUBLE:
    *p++ = 0xDD;
    *p++ = 0x05;
    (int&)p[0] = Double_Address(argument.dec_); p+= sizeof(int);
    
    *p++ = 0xDD;
    if(address < 0x7F)
    {
      *p++ = 0x5D;
      *p++ = two_complement_8(address);
    }
    else
    {
      *p++ = 0x9D;
      (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
    }
    return;
  }
}

void AOT_Compiler::Load_Register(REGISTERS reg, ARG argument)
{
  switch(argument.type)
  {
  case AOT_REG:
    Move_Register(reg, argument.reg_);
    return;
  case AOT_MEMORY:
  case AOT_INT:
    switch(reg)
    {
    case EAX:
      if(argument.num_ == 0)
      {
        //xor eax, eax
        *p++ = 0x33;
        *p++ = 0xC0;
        return;
      }
      else
      {
        *p++ = 0xB8; // mov eax
      }
      break;
    case EBX:
      *p++ = 0xBB; // mov ebx
      break;
    case ECX:
      *p++ = 0xB9; // mov ecx
      break;
    case EDX:
      *p++ = 0xBA; // mov edx
      break;
    }
    (int&)p[0] = argument.num_; p+= sizeof(int);
    return;
  }
}
void AOT_Compiler::Load_Ptr(unsigned ptr)
{
  *p++ = 0x8B;  
  if(ptr < 0x7F)
  {
    *p++ = 0x45;
    *p++ = static_cast<char>(ptr);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = static_cast<int>(ptr); p+= sizeof(int);
  }

}
void AOT_Compiler::MovP(unsigned addr, unsigned offset, bool byte)
{
  *p++ = 0x8B - byte;
  if(addr < 0x7F)
  {
    *p++ = 0x4D;
    *p++ = two_complement_8(addr);
  }
  else
  {
    *p++ = 0x8D;
    (int&)p[0] = (int)two_complement_32(addr); p+= sizeof(int);
  }
  
  *p++ = 0x89 - byte;
  *p++ = 0x48;
  *p++ = static_cast<char>(offset);
}
void AOT_Compiler::Mov(REGISTERS dest, unsigned address, bool byte)
{
  *p++ = 0x8B - byte;
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Mov(unsigned address, REGISTERS source, bool byte)
{
  *p++ = 0x89 - byte;
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Lea(REGISTERS dest, unsigned address)
{
  *p++ = 0x8D;
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Move_Register(REGISTERS dest, REGISTERS source)
{
  *p++ = 0x8B;
  *p++ = Reg_to_Reg(dest, source);
}
void AOT_Compiler::Xchg_Register(REGISTERS dest, REGISTERS source)
{
  if(source == dest)
  {
    //nop
    return;
  }
  //Super fast EAX xchgs
  if(dest == EAX)
  {
    switch(source)
    {
    case EBX:
      *p++ = 0x93;
      break;
    case ECX:
      *p++ = 0x91;
      break;
    case EDX:
      *p++ = 0x92;
    }
  }
  else if(source == EAX)
  {
    switch(dest)
    {
    case EBX:
      *p++ = 0x93;
      break;
    case ECX:
      *p++ = 0x91;
      break;
    case EDX:
      *p++ = 0x92;
    }
  }
  else
  {
    *p++ = 0x87;
    *p++ = Reg_to_Reg(dest, source);
  }
}
void AOT_Compiler::Add(REGISTERS dest, REGISTERS source)
{
  *p++ = 0x03;
  *p++ = Reg_to_Reg(dest, source);
}
void AOT_Compiler::Add(unsigned address, REGISTERS source)
{
  *p++ = 0x01;
  if(address < 0x7F)
  {
    *p++ = source - WORD_VARIANT;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = source;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Sub(REGISTERS dest, REGISTERS source)
{
  *p++ = 0x2B;
  *p++ = Reg_to_Reg(dest, source);
}
void AOT_Compiler::Sub(unsigned address, REGISTERS source)
{
  *p++ = 0x29;
  if(address < 0x7F)
  {
    *p++ = source - WORD_VARIANT;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = source;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Mul(REGISTERS dest, REGISTERS source)
{
  *p++ = 0x0F;
  *p++ = 0xAF;
  *p++ = Reg_to_Reg(dest, source);
}
void AOT_Compiler::Imul(unsigned address)
{
  *p++ = 0xF7;
  if(address < 0x7F)
  {
    *p++ = 0x6D;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0xAD;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Inc(REGISTERS dest)
{
  switch(dest)
  {
  case EAX:
    *p++ = 0x40;
    return;
  case EBX:
    *p++ = 0x43;
    return;
  case ECX:
    *p++ = 0x41;
    return;
  case EDX:
    *p++ = 0x42;
    return;
  }
}
void AOT_Compiler::Or(unsigned address, REGISTERS source)
{
  *p++ = 0x08;
  if(address < 0x7F)
  {
    *p++ = source - WORD_VARIANT;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = source;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::And(unsigned address, REGISTERS source)
{
  *p++ = 0x21;
  if(address < 0x7F)
  {
    *p++ = source - WORD_VARIANT;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = source;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::Dec(REGISTERS dest)
{
  switch(dest)
  {
  case EAX:
    *p++ = 0x48;
    return;
  case EBX:
    *p++ = 0x4B;
    return;
  case ECX:
    *p++ = 0x49;
    return;
  case EDX:
    *p++ = 0x4A;
    return;
  }
}
void AOT_Compiler::Call(void* function)
{
  Load_Register(ECX, function);
  *p++ = 0xFF;
  *p++ = 0xD1;
}
void AOT_Compiler::Call(const char* function)
{
  Load_Register(ECX, 0);
  bool found = false;
  for(unsigned i = 0; i < call_links.size();i++)
  {
    if(!call_links[i].name.compare(function))
    {
      found = true;
      call_links[i].refrence_list.push_back(p);
      break;
    }
  }
  if(found == false)
  {
    LINKER_Data dat;
    dat.name = function;
    dat.refrence_list.push_back(p - 4);
    call_links.push_back(dat);
  }
  *p++ = 0xFF;
  *p++ = 0xD1;
}
void AOT_Compiler::Return(ARG argument)
{
  //Floats and doubles are returned on the FPU
  //x86 primitives are passed via EAX.
  if(argument.type == AOT_FLOAT || argument.type == AOT_DOUBLE)
  {
    FPU_Load(argument);
  }
  else
  {
    Load_Register(EAX, argument);
  }
  *p++ = 0x8B;
  *p++ = 0xE5; // mov esp,ebp 
  *p++ = 0x5D; // pop ebp
  *p++ = 0x5A; // pop edx
  *p++ = 0xC3; // ret
}

const char* AOT_Compiler::Get_Version()
{
  return COMPILER_VERSION;
}
const char* AOT_Compiler::Get_Name()
{
  return prg_id.c_str();
}
std::vector<LINKER_Data> AOT_Compiler::Get_Links()
{
  return call_links;
}
unsigned char AOT_Compiler::Reg_to_Reg(REGISTERS dest, REGISTERS source)
{
  unsigned char base;
  switch(dest)
  {
  case EAX:
    base = 0xC0;
    break;
  case EBX:
    base = 0xD8;
    break;
  case ECX:
    base = 0xC8;
    break;
  case EDX:
    base = 0xD0;
    break;
  }
  switch(source)
  {
  case EAX:
    return base; // mov eax
  case EBX:
    return base + 0x03; // mov ebx
  case ECX:
    return base + 0x01; // mov ecx
  case EDX:
    return base + 0x02; // mov edx
  }
  return NULL;
}
int AOT_Compiler::String_Address(std::string& str)
{
  int address;
  for(unsigned i = 0; i < esp.size(); i++)
  {
    if(!esp[i].name.compare(str))
    {
      address = (int)esp[i].name.c_str();
      return address;
    }
  }
  return Add_String(str);
}
int AOT_Compiler::Add_String(std::string& str)
{
  LINKER_Var var;
  var.name.assign(str);
  esp.push_back(var);
  return (int)(esp.back().name.c_str());
}
void AOT_Compiler::Label_Management(unsigned label)
{
   std::vector<AOT_Var>::iterator walker;
  //search lables for backwards jumps
  for(walker = cls.begin(); walker != cls.end(); walker++)
  {
    if(walker->lab == label)
    {
      *p++ = two_complement_8((unsigned)p - walker->adr + 1);
      return;
    }
  }
  //didn't find a matching label
  AOT_Var var;
  var.lab = label;
  var.adr = (unsigned)p;
  var.loc = p;
  cjs.push_back(var);
  p++;
}
int AOT_Compiler::Float_Address(float dec)
{
  for(unsigned i = 0; i < efp.size(); i++)
  {
    if(efp[i].flt == dec)
    {
      return (int)&(efp[i].flt);
    }
  }
  return Add_Float(dec);
}
int AOT_Compiler::Add_Float(float dec)
{
  LINKER_Var var;
  var.flt = dec;
  efp.push_back(var);
  return (int)&(efp.back().flt);
}
int AOT_Compiler::Double_Address(double dec)
{
  for(unsigned i = 0; i < edp.size(); i++)
  {
    if(edp[i].dec == dec)
    {
      return (int)&(edp[i].dec);
    }
  }
  return Add_Double(dec);
}
int AOT_Compiler::Add_Double(double dec)
{
  LINKER_Var var;
  var.dec = dec;
  edp.push_back(var);
  return (int)&(edp.back().dec);
}
void AOT_Compiler::FPU_Load(ARG argument)
{
  //AOT_Var* var;
  switch(argument.type)
  {
  case AOT_FLOAT:    
    *p++ = 0xD9;
    *p++ = 0x05;
    (int&)p[0] = (int)Float_Address(argument.flt_); p+= sizeof(int);
    return;
  case AOT_DOUBLE:    
    *p++ = 0xDD;
    *p++ = 0x05;
    (int&)p[0] = (int)Double_Address(argument.dec_); p+= sizeof(int);
    return;
    //FPU can't actually load integer values
    //so we need to load it as an "address":
  case AOT_INT:    
    *p++ = 0xD9;
    *p++ = 0x05;
    (int&)p[0] = (int)Float_Address((float)argument.num_); p+= sizeof(int);
    return;
  }
}
void AOT_Compiler::FPU_Loadf(unsigned address)
{
  *p++ = 0xD9;  
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::FPU_Loadd(unsigned address)
{
  *p++ = 0xDD;
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::FPU_Loadi(unsigned address)
{
  *p++ = 0xDB;
  if(address < 0x7F)
  {
    *p++ = 0x45;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x85;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void AOT_Compiler::FPU_Store(unsigned address)
{
  *p++ = 0xDD;
  if(address < 0x7F)
  {
    *p++ = 0x55;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0x95;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
  //AOT_Var* var = Local_Direct(name);
  //switch(var->type)
  //{
  //case _FLOAT:
  //  *p++ = 0xD9;
  //  *p++ = 0x5D;
  //  *p++ = two_complement_8(Local_Address(name) + 4);
  //  return;
  //case _INT:
  //  *p++ = 0xDB;
  //  *p++ = 0x5D;
  //  *p++ = two_complement_8(Local_Address(name) + 4);
  //  return;
  //case _DOUBLE:
  //  *p++ = 0xDD;
  //  *p++ = 0x5D;
  //  *p++ = two_complement_8(Local_Address(name) + 4);
  //  return;
  //}
}
void AOT_Compiler::FPU_Add(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xC0 + (unsigned)reg;
}
void AOT_Compiler::FPU_Sub(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xE8 + (unsigned)reg;
}
void AOT_Compiler::FPU_Mul(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xC8 + (unsigned)reg;
}
void AOT_Compiler::FPU_Div(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xF8 + (unsigned)reg;
}
void AOT_Compiler::FPU_Sqr()
{
  *p++ = 0xDC;
  *p++ = 0xC8;
}
void AOT_Compiler::FPU_Abs()
{
  *p++ = 0xD9;
  *p++ = 0xE1;
}
void AOT_Compiler::FPU_Root()
{
  *p++ = 0xD9;
  *p++ = 0xFA;
}
void AOT_Compiler::FPU_One()
{
  *p++ = 0xD9;
  *p++ = 0xE8;
}
void AOT_Compiler::FPU_Zero()
{
  *p++ = 0xD9;
  *p++ = 0xEE;
}
void AOT_Compiler::FPU_PI()
{
  *p++ = 0xD9;
  *p++ = 0xEB;
}
void AOT_Compiler::FPU_Xchg()
{
  *p++ = 0xD9;
  *p++ = 0xC9;
}
void AOT_Compiler::FPU_Invert()
{
  *p++ = 0xD9;
  *p++ = 0xE0;
}
void AOT_Compiler::FPU_Neg()
{
  FPU_Abs();
  FPU_Invert();
}
void AOT_Compiler::FPU_Round()
{
  *p++ = 0xD9;
  *p++ = 0xFC;
}
void AOT_Compiler::FPU_Sin()
{
  *p++ = 0xD9;
  *p++ = 0xFE;
}
void AOT_Compiler::FPU_Cos()
{
  *p++ = 0xD9;
  *p++ = 0xFF;
}