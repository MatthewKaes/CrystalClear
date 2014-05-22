#include "x86_Machine.h"

#pragma warning(disable: 4996)

BYTE x86_Machine::two_complement_8(unsigned char id)
{
  return static_cast<BYTE>((0xFF - id) + 1);
}
unsigned x86_Machine::two_complement_32(unsigned id)
{
  return (0xFFFFFFFF - id) + 1;
}

x86_Machine::x86_Machine()
{
  //Obviously since these are used in direct addressing
  //we can't have them go and reallocating themselves.
  //When they grow we will get a copy and we don't want that.
  esp.reserve(STACK_SIZE);
  efp.reserve(STACK_SIZE);
  edp.reserve(STACK_SIZE);
}
void x86_Machine::Setup(std::string name, BYTE* program)
{
  p = program;
  prg_id = name;
  call_links.clear();
}
void x86_Machine::Make_Label(unsigned label)
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
unsigned x86_Machine::New_Label()
{
  return cls.size();
}
void x86_Machine::Cmp(unsigned address, ARG argument)
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
void x86_Machine::CmpF(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    if(argument.num_ > ADDER_S)
    {
      *p++ = STK_POP;
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
void x86_Machine::Jmp(unsigned label)
{
  *p++ = CODE_JMP;  
  Label_Management(label);
}
void x86_Machine::Je(unsigned label)
{
  *p++ = CODE_JE;
  Label_Management(label);
}
void x86_Machine::Jne(unsigned label)
{
  *p++ = CODE_JNE;
  Label_Management(label);
}
void x86_Machine::Jle(unsigned label)
{
  *p++ = CODE_JLE;
  Label_Management(label);
}
void x86_Machine::Jl(unsigned label)
{
  *p++ = CODE_JL;
  Label_Management(label);
}
void x86_Machine::Jg(unsigned label)
{
  *p++ = CODE_JG;
  Label_Management(label);
}
void x86_Machine::Allocate_Stack(unsigned bytes)
{
  //Strack preamble
  Push(EDX);
  *p++ = PSH_EBP; //push ebp

  *p++ = MEM_MOV;
  *p++ = EBP_ESP; //mov ebp, esp

  stack_size = bytes;

  if(stack_size < ADDER_S)
  {
    *p++ = STK_POP;
    *p++ = SUB_ESP;
    *p++ = (unsigned char)(stack_size);
    *p++ = 0x6A;
    *p++ = (unsigned char)(stack_size);
  }
  else
  {
    *p++ = STK_PP4;
    *p++ = SUB_ESP; //sub esp
    (int&)p[0] = stack_size; p+= sizeof(int);
    *p++ = 0x68;
    (int&)p[0] = stack_size; p+= sizeof(int);
  }

  //Get address
  *p++ = REG_ADR; *p++ = REG_LEA; //LEA
  *p++ = LEA_EAX; //EAX
  *p++ = BYTES_4; // 4 bytes pushed already
  //Clear it up
  *p++ = 0x6A;
  *p++ = MC_ZERO;
  //Push the address last
  *p++ = 0x50;
  //Construct the symbols
  Call(memset);
  //Restore stack frame
  *p++ = STK_POP; *p++ = ADD_ESP; *p++ = BYTES_12; //12 bytes
  stack_allocated = true;
}  
void x86_Machine::Print(ARG argument)
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
void x86_Machine::Push(ARG argument)
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
    if(abs(argument.num_) < ADDER_S)
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
    *p++ = FPU_DOUBLE_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = (int)Double_Address(argument.dec_); p+= sizeof(int);

    //Make room
    *p++ = STK_POP;
    *p++ = SUB_ESP;
    *p++ = BYTES_8;

    //store on stack
    //double
    *p++ = FPU_DOUBLE_OP;
    *p++ = 0x1C;
    *p++ = LEA_EAX;
    pushed_bytes += BYTES_4;
    return;
  case AOT_FLOAT:
    //Float
    *p++ = FPU_FLOAT_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = (int)Float_Address(argument.flt_); p+= sizeof(int);

    //Make room
    *p++ = STK_POP;
    *p++ = SUB_ESP;
    *p++ = BYTES_4;

    //store on stack
    //Float
    *p++ = FPU_FLOAT_OP;
    *p++ = 0x1C;
    *p++ = 0x24;

    pushed_bytes += BYTES_4;
    return;
  }
}
void x86_Machine::Push_Adr(unsigned address)
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
void x86_Machine::Pop(unsigned bytes)
{
  if(!bytes)
  {
    return;
  }
  else
  {
    *p++ = STK_POP;
    *p++ = 0xC4;
    *p++ = (unsigned char)(bytes);
    pushed_bytes -= bytes;
  }
}
void x86_Machine::Load_Mem(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    *p++ = 0xC7;
    Put_Addr(address);
    (int&)p[0] = argument.num_; p+= sizeof(int);
    return;
  case AOT_STRING:
    *p++ = 0xC7;
    Put_Addr(address);
    (int&)p[0] = (int)argument.str_; p+= sizeof(int);
    return;
  case AOT_BOOL:
  case AOT_CHAR:
    *p++ = 0xC6;
    Put_Addr(address);
    *p++ = static_cast<BYTE>(argument.chr_);
    return;
  case AOT_FLOAT:
    Load_Mem(address, static_cast<double>(argument.flt_));
    return;
  case AOT_DOUBLE:
    *p++ = FPU_DOUBLE_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = Double_Address(argument.dec_); p+= sizeof(int);
    *p++ = FPU_DOUBLE_OP;
    Put_Addr(address, FPU_PUSH);
    return;
  }
}

void x86_Machine::Load_Register(REGISTERS reg, ARG argument)
{
  switch(argument.type)
  {
  case AOT_REG:
    Move_Register(reg, argument.reg_);
    return;
  case AOT_MEMORY:
  case AOT_INT:
    if(reg == EAX && argument.num_ == 0)
    {
      //xor eax, eax
      *p++ = REG_XOR;
      *p++ = OPR_EAX;
      return;
    }
    *p++ = REG_LOD + Reg_Id(reg); // mov eax
    (int&)p[0] = argument.num_; p+= sizeof(int);
    return;
  }
}
void x86_Machine::Load_Ptr(unsigned ptr)
{
  *p++ = MEM_MOV;  
  if(ptr < ADDER_S)
  {
    *p++ = BYT_ADR;
    *p++ = static_cast<char>(ptr);
  }
  else
  {
    *p++ = BYT_ADR + WRD_OFF;
    (int&)p[0] = static_cast<int>(ptr); p+= sizeof(int);
  }
}
void x86_Machine::MovP(unsigned addr, unsigned offset, bool byte)
{
  *p++ = MEM_MOV - byte;
  Put_Addr(addr, PTR_ADR);
  
  *p++ = MOV_TYP - byte;
  *p++ = PTR_MOV;
  *p++ = static_cast<char>(offset);
}
void x86_Machine::Mov(REGISTERS dest, unsigned address, bool byte)
{
  *p++ = MEM_MOV - byte;
  Put_Addr(address);
}
void x86_Machine::Mov(unsigned address, REGISTERS source, bool byte)
{
  *p++ = MOV_TYP - byte;
  Put_Addr(address);
}
void x86_Machine::Lea(REGISTERS dest, unsigned address)
{
  *p++ = REG_ADR;
  Put_Addr(address);
}
void x86_Machine::Move_Register(REGISTERS dest, REGISTERS source)
{
  *p++ = MEM_MOV;
  *p++ = Reg_to_Reg(dest, source);
}
void x86_Machine::Xchg_Register(REGISTERS dest, REGISTERS source)
{
  if(source == dest)
  {
    //nop
    return;
  }
  //Super fast EAX xchgs
  if(dest == EAX || source == EAX)
  {
    *p++ = EAX_XCH + Reg_Id(dest != EAX ? dest : source);
  }
  else
  {
    *p++ = REG_XCH;
    *p++ = Reg_to_Reg(dest, source);
  }
}
void x86_Machine::Add(REGISTERS dest, REGISTERS source)
{
  *p++ = REG_ADD;
  *p++ = Reg_to_Reg(dest, source);
}
void x86_Machine::Add(unsigned address, REGISTERS source)
{
  *p++ = MEM_ADD;
  if(address < ADDER_S)
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
void x86_Machine::Sub(REGISTERS dest, REGISTERS source)
{
  *p++ = REG_SUB;
  *p++ = Reg_to_Reg(dest, source);
}
void x86_Machine::Sub(unsigned address, REGISTERS source)
{
  *p++ = REG_SUB;
  if(address < ADDER_S)
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
void x86_Machine::Mul(REGISTERS dest, REGISTERS source)
{
  *p++ = REG_MUL_L;
  *p++ = REG_MUL_H;
  *p++ = Reg_to_Reg(dest, source);
}
void x86_Machine::Imul(unsigned address)
{
  *p++ = REG_IMUL;
  if(address < ADDER_S)
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
void x86_Machine::Inc(REGISTERS dest)
{
  *p++ = REG_INC + Reg_Id(dest);
}
void x86_Machine::Or(unsigned address, REGISTERS source)
{
  *p++ = REG_OR;
  if(address < ADDER_S)
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
void x86_Machine::And(unsigned address, REGISTERS source)
{
  *p++ = REG_AND;
  if(address < ADDER_S)
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
void x86_Machine::Dec(REGISTERS dest)
{
  *p++ = REG_DEC + Reg_Id(dest);
}
void x86_Machine::Call(void* function)
{
  Load_Register(ECX, function);
  *p++ = FAR_CAL;
  *p++ = FAR_ECX;
}
void x86_Machine::Call(const char* function)
{
  Load_Register(ECX, MC_ZERO);
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
    dat.refrence_list.push_back(p - BYTES_4);
    call_links.push_back(dat);
  }
  *p++ = FAR_CAL;
  *p++ = FAR_ECX;
}
void x86_Machine::Return(ARG argument)
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
  *p++ = MEM_MOV;
  *p++ = ESP_EBP; // mov esp,ebp 
  *p++ = POP_EBP; // pop ebp
  *p++ = POP_EDX; // pop edx
  *p++ = FNC_RET; // ret
}

const char* x86_Machine::Get_Version()
{
  return COMPILER_VERSION;
}
const char* x86_Machine::Get_Name()
{
  return prg_id.c_str();
}
std::vector<LINKER_Data> x86_Machine::Get_Links()
{
  return call_links;
}
unsigned char x86_Machine::Reg_to_Reg(REGISTERS dest, REGISTERS source)
{
  return 0xC0 + 0x08 * Reg_Id(source) + Reg_Id(source);
}
unsigned x86_Machine::Reg_Id(REGISTERS reg)
{
  switch(reg)
  {
  case EAX: return 0x00;
  case EBX: return 0x03;
  case ECX: return 0x01;
  case EDX: return 0x02;
  }
  return 0;
} 
void x86_Machine::Put_Addr(unsigned addr, int op_offset)
{
  if(addr < ADDER_S)
  {
    *p++ = op_offset;
    *p++ = two_complement_8(addr);
  }
  else
  {
    *p++ = op_offset + WRD_OFF;
    (int&)p[0] = (int)two_complement_32(addr); p+= sizeof(int);
  }
}
int x86_Machine::String_Address(std::string& str)
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
int x86_Machine::Add_String(std::string& str)
{
  LINKER_Var var;
  var.name.assign(str);
  esp.push_back(var);
  return (int)(esp.back().name.c_str());
}
void x86_Machine::Label_Management(unsigned label)
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
int x86_Machine::Float_Address(float dec)
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
int x86_Machine::Add_Float(float dec)
{
  LINKER_Var var;
  var.flt = dec;
  efp.push_back(var);
  return (int)&(efp.back().flt);
}
int x86_Machine::Double_Address(double dec)
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
int x86_Machine::Add_Double(double dec)
{
  LINKER_Var var;
  var.dec = dec;
  edp.push_back(var);
  return (int)&(edp.back().dec);
}
void x86_Machine::FPU_Load(ARG argument)
{
  //AOT_Var* var;
  switch(argument.type)
  {
  case AOT_FLOAT:    
    *p++ = FPU_FLOAT_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = (int)Float_Address(argument.flt_); p+= sizeof(int);
    return;
  case AOT_DOUBLE:    
    *p++ = FPU_DOUBLE_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = (int)Double_Address(argument.dec_); p+= sizeof(int);
    return;
    //FPU can't actually load integer values
    //so we need to load it as an "address":
  case AOT_INT:    
    *p++ = FPU_FLOAT_OP;
    *p++ = FPU_LOAD_ADDR;
    (int&)p[0] = (int)Float_Address((float)argument.num_); p+= sizeof(int);
    return;
  }
}
void x86_Machine::FPU_Loadf(unsigned address)
{
  *p++ = FPU_FLOAT_OP;  
  if(address < ADDER_S)
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
void x86_Machine::FPU_Loadd(unsigned address)
{
  *p++ = 0xDD;
  if(address < ADDER_S)
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
void x86_Machine::FPU_Loadi(unsigned address)
{
  *p++ = 0xDB;
  if(address < ADDER_S)
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
void x86_Machine::FPU_Store(unsigned address)
{
  *p++ = 0xDD;
  if(address < ADDER_S)
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
void x86_Machine::FPU_Add(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xC0 + (unsigned)reg;
}
void x86_Machine::FPU_Sub(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xE8 + (unsigned)reg;
}
void x86_Machine::FPU_Mul(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xC8 + (unsigned)reg;
}
void x86_Machine::FPU_Div(FPU_REGISTERS reg)
{
  *p++ = 0xDE;
  *p++ = 0xF8 + (unsigned)reg;
}
void x86_Machine::FPU_Sqr()
{
  *p++ = 0xDC;
  *p++ = 0xC8;
}
void x86_Machine::FPU_Abs()
{
  *p++ = 0xD9;
  *p++ = 0xE1;
}
void x86_Machine::FPU_Root()
{
  *p++ = 0xD9;
  *p++ = 0xFA;
}
void x86_Machine::FPU_One()
{
  *p++ = 0xD9;
  *p++ = 0xE8;
}
void x86_Machine::FPU_Zero()
{
  *p++ = 0xD9;
  *p++ = 0xEE;
}
void x86_Machine::FPU_PI()
{
  *p++ = 0xD9;
  *p++ = 0xEB;
}
void x86_Machine::FPU_Xchg()
{
  *p++ = 0xD9;
  *p++ = 0xC9;
}
void x86_Machine::FPU_Invert()
{
  *p++ = 0xD9;
  *p++ = 0xE0;
}
void x86_Machine::FPU_Neg()
{
  FPU_Abs();
  FPU_Invert();
}
void x86_Machine::FPU_Round()
{
  *p++ = 0xD9;
  *p++ = 0xFC;
}
void x86_Machine::FPU_Sin()
{
  *p++ = 0xD9;
  *p++ = 0xFE;
}
void x86_Machine::FPU_Cos()
{
  *p++ = 0xD9;
  *p++ = 0xFF;
}
