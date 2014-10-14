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
  std::vector<AOT_Var>::iterator walker = cls.begin();
  while(walker != cls.end())
  {
    if(walker->lab == label)
    {
      walker->adr = l.adr;
      break;
    }
    walker++;
  }
  if(walker == cls.end())
  {
    cls.push_back(l);
  }

  walker = cjs.begin();
  //search backwards for foward jumps
  while(walker != cjs.end())
  {
    if(walker->lab == label)
    {
      unsigned distance = l.adr - walker->adr - BYTES_1;
      (walker->loc)[0] = distance;
    }
    walker++;
  }
}
unsigned x86_Machine::New_Label()
{
  return cls.size();
}
unsigned x86_Machine::Reserve_Label()
{
  AOT_Var new_lab;
  new_lab.adr = 0;
  new_lab.lab = cls.size();
  cls.push_back(new_lab);
  return new_lab.lab;
}
unsigned x86_Machine::Last_Label()
{
  return cls.size() - 1;
}
void x86_Machine::Cmp(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_CHAR:
    if(address < ADDER_S)
    {
      *p++ = CMP_BYTE;
      *p++ = CMP_MEM;
      *p++ = two_complement_8(address);
      *p++ = (unsigned char)argument.chr_;
    }
    else
    {
      *p++ = CMP_WORDL;
      *p++ = CMP_MEM + WORD_VARIANT;
      (int&)p[0] = two_complement_32(address); p+= sizeof(int);
      *p++ = (unsigned char)argument.chr_;
    }
    break;
  case AOT_INT:
    if(argument.num_ > ADDER_S)
    {
      *p++ = CMP_WORD;
      if(address < ADDER_S)
      {
        *p++ = CMP_MEM;
        *p++ = two_complement_8(address);
      }
      else
      {
        *p++ = CMP_MEM + WORD_VARIANT;
        (int&)p[0] = two_complement_32(address); p+= sizeof(int);
      }
      *p++ = (unsigned char)argument.num_;
    }
    else
    {
      *p++ = CMP_WORDL;
      if(address < ADDER_S)
      {
        *p++ = CMP_MEM;
        *p++ = two_complement_8(address);
      }
      else
      {
        *p++ = CMP_MEM + WORD_VARIANT;
        (int&)p[0] = two_complement_32(address); p+= sizeof(int);
      }
      (int&)p[0] = argument.num_; p+= sizeof(int);
    }
    break;
  case AOT_REG:
    *p++ = CMP_REG;
    Reg_Op(address, EAX);
    break;
  }
}
void x86_Machine::CmpF(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    if(argument.num_ < ADDER_S)
    {
      *p++ = CMP_WORD;
      *p++ = CMP_MEM;
      *p++ = static_cast<char>(address);
      *p++ = (unsigned char)argument.num_;
    }
    else
    {
      *p++ = CMP_WORDL;
      *p++ = CMP_MEM;
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
void x86_Machine::Jge(unsigned label)
{
  *p++ = CODE_JGE;
  Label_Management(label);
}
void x86_Machine::Sete(unsigned address)
{
  *p++ = SET_TYP;  
  *p++ = 0x94;  
  Put_Addr(address);
}
void x86_Machine::Setne(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x95;
  Put_Addr(address);
}
void x86_Machine::Setl(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x9C;
  Put_Addr(address);
}
void x86_Machine::Setle(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x9E;
  Put_Addr(address);
}
void x86_Machine::Setg(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x9F;
  Put_Addr(address);
}
void x86_Machine::Setge(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x9D;
  Put_Addr(address);
}
void x86_Machine::Seta(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x97;
  Put_Addr(address);
}
void x86_Machine::Setae(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x93;
  Put_Addr(address);
}
void x86_Machine::Setb(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x92;
  Put_Addr(address);
}
void x86_Machine::Setbe(unsigned address)
{
  *p++ = SET_TYP;
  *p++ = 0x96;
  Put_Addr(address);
}
void x86_Machine::Allocate_Stack(unsigned bytes)
{
  //Strack preamble
  Push(ESI);
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
    *p++ = PSH_INT_B;
    *p++ = (unsigned char)(stack_size);
  }
  else
  {
    *p++ = STK_PP4;
    *p++ = SUB_ESP; //sub esp
    (int&)p[0] = stack_size; p+= sizeof(int);
    *p++ = PSH_INT_W;
    (int&)p[0] = stack_size; p+= sizeof(int);
  }

  //Get address
  *p++ = REG_ADR; *p++ = REG_LEA; //LEA
  *p++ = LEA_EAX; //EAX
  *p++ = BYTES_4; // 4 bytes pushed already
  //Clear it up
  *p++ = PSH_INT_B;
  *p++ = MC_ZERO;
  //Push the address last
  *p++ = REG_PSH;
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
  *p++ = PSH_INT_W;
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
  pushed_bytes += BYTES_4;
  switch(argument.type)
  {
  case AOT_REG:
    *p++ = REG_PSH + Reg_Id(argument.reg_);
    return;
  case AOT_INT:
    if(abs(argument.num_) < ADDER_S)
    {
      *p++ = PSH_INT_B;
      *p++ = (unsigned char)(argument.num_);
    }
    else
    {
      *p++ = PSH_INT_W;
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
    *p++ = FPU_ADR;
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
    *p++ = FPU_ADR;
    *p++ = LEA_EAX;

    pushed_bytes += BYTES_4;
    return;
  }
}
void x86_Machine::Push_Adr(unsigned address)
{
  Lea(EAX, address);
  *p++ = REG_PSH; 
  pushed_bytes += BYTES_4;   
}
void x86_Machine::Push_LD(unsigned address, ARG_TYPES type)
{
  switch(type)
  {
  case AOT_INT:
    *p++ = STK_POP;
    *p++ = SUB_ESP;
    *p++ = BYTES_8;
    FPU_Loadi(address);
    *p++ = FPU_DOUBLE_OP;
    *p++ = FPU_ADR;
    *p++ = FPU_ESP;
    return;
  case AOT_DOUBLE:
    *p++ = STK_POP;
    *p++ = SUB_ESP;
    *p++ = BYTES_8;
    FPU_Loadd(address);
    *p++ = FPU_DOUBLE_OP;
    *p++ = FPU_ADR;
    *p++ = FPU_ESP;
    return;
  default:
    return;
  }
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
    *p++ = ADD_ESP;
    *p++ = (unsigned char)(bytes);
    pushed_bytes -= bytes;
  }
}
void x86_Machine::Load_Mem(unsigned address, ARG argument)
{
  switch(argument.type)
  {
  case AOT_INT:
    *p++ = ESP_WORD;
    Put_Addr(address);
    (int&)p[0] = argument.num_; p+= sizeof(int);
    return;
  case AOT_STRING:
    *p++ = ESP_WORD;
    Put_Addr(address);
    (int&)p[0] = (int)argument.str_; p+= sizeof(int);
    return;
  case AOT_BOOL:
  case AOT_CHAR:
    *p++ = ESP_BYTE;
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
  Reg_Op(address, dest);
}
void x86_Machine::Mov(unsigned address, REGISTERS source, bool byte)
{
  *p++ = MOV_TYP - byte;
  Reg_Op(address, source);
}
void x86_Machine::Lea(REGISTERS dest, unsigned address)
{
  *p++ = REG_ADR;
  Reg_Op(address, dest);
}
void x86_Machine::Move_Register(REGISTERS dest, REGISTERS source)
{
  if(source == dest)
    return;
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
  Reg_Op(address, source);
}
void x86_Machine::Sub(REGISTERS dest, REGISTERS source)
{
  *p++ = REG_SUB;
  *p++ = Reg_to_Reg(dest, source);
}
void x86_Machine::Sub(unsigned address, REGISTERS source)
{
  *p++ = REG_SUB;
  Reg_Op(address, source);
}
void x86_Machine::Sub(REGISTERS source, unsigned address)
{
  *p++ = 0x29;
  Reg_Op(address, source);
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
  Put_Addr(address, WID_ADR);
}
void x86_Machine::Imul(REGISTERS dest, unsigned address)
{
  *p++ = REG_MUL_L;
  *p++ = REG_MUL_H;
  Reg_Op(address, dest);
}
void x86_Machine::Idiv(unsigned address)
{  
  //cdq
  *p++ = 0x99;
  //idiv
  *p++ = 0xF7;
  if(address < ADDER_S)
  {
    *p++ = 0x7D;
    *p++ = two_complement_8(address);
  }
  else
  {
    *p++ = 0xBD;
    (int&)p[0] = (int)two_complement_32(address); p+= sizeof(int);
  }
}
void x86_Machine::Idiv(REGISTERS dest)
{
  //cdq
  *p++ = 0x99;
  //idiv
  *p++ = 0xF7;
  *p++ = 0xF8 + Reg_Id(dest);
}
void x86_Machine::Inc(REGISTERS dest)
{
  *p++ = REG_INC + Reg_Id(dest);
}
void x86_Machine::Or(unsigned address, REGISTERS source)
{
  *p++ = REG_OR;
  Reg_Op(address, source);
}
void x86_Machine::And(unsigned address, REGISTERS source)
{
  *p++ = REG_AND;
  Reg_Op(address, source);
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
      call_links[i].refrence_list.push_back(p - BYTES_4);
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
  *p++ = POP_ESI;
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
  return 0xC0 + BYTES_8 * Reg_Id(dest) + Reg_Id(source);
}
unsigned x86_Machine::Reg_Id(REGISTERS reg)
{
  switch(reg)
  {
  case EAX: return 0x00;
  case EBX: return 0x03;
  case ECX: return 0x01;
  case EDX: return 0x02;
  case ESI: return 0x06;
  case EDI: return 0x07;
  }
  return 0;
} 
void x86_Machine::Reg_Op(unsigned address, REGISTERS source)
{
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
int x86_Machine::String_Address(const char* str)
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
int x86_Machine::Add_String(const char* str)
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
      if(walker->adr == 0)
        break;
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
  Put_Addr(address);
}
void x86_Machine::FPU_Loadd(unsigned address)
{
  *p++ = FPU_DOUBLE_OP;
  Put_Addr(address);
}
void x86_Machine::FPU_Loadi(unsigned address)
{
  *p++ = FPU_INT_OP;
  Put_Addr(address);
}
void x86_Machine::FPU_Store(unsigned address)
{
  *p++ = FPU_DOUBLE_OP;
  Put_Addr(address, FPU_STORE);
}
void x86_Machine::FPU_Cmp(FPU_REGISTERS reg)
{
  *p++ = FPU_COMIP;
  *p++ = 0xE8 + (unsigned)reg;
}
void x86_Machine::FPU_Add(FPU_REGISTERS reg)
{
  *p++ = FPU_MATH;
  *p++ = FPU_ADD + (unsigned)reg;
}
void x86_Machine::FPU_Sub(FPU_REGISTERS reg)
{
  *p++ = FPU_MATH;
  *p++ = FPU_SUB + (unsigned)reg;
}
void x86_Machine::FPU_Mul(FPU_REGISTERS reg)
{
  *p++ = FPU_MATH;
  *p++ = FPU_MUL + (unsigned)reg;
}
void x86_Machine::FPU_Div(FPU_REGISTERS reg)
{
  *p++ = FPU_MATH;
  *p++ = FPU_DIV + (unsigned)reg;
}
void x86_Machine::FPU_Sqr()
{
  *p++ = FPU_80P_OP;
  *p++ = FPU_SQRT;
}
void x86_Machine::FPU_Abs()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_ABS;
}
void x86_Machine::FPU_Root()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_ROOT;
}
void x86_Machine::FPU_One()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_PUSH_ONE;
}
void x86_Machine::FPU_Zero()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_PUSH_ZERO;
}
void x86_Machine::FPU_PI()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_PUSH_PI;
}
void x86_Machine::FPU_Xchg()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_XCHNG;
}
void x86_Machine::FPU_Invert()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_INVERT;
}
void x86_Machine::FPU_Neg()
{
  FPU_Abs();
  FPU_Invert();
}
void x86_Machine::FPU_Round()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_ROUND;
}
void x86_Machine::FPU_Sin()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_SIN;
}
void x86_Machine::FPU_Cos()
{
  *p++ = FPU_FLOAT_OP;
  *p++ = FPU_COS;
}
void x86_Machine::Strcpy(REGISTERS dest, unsigned address, int length, bool raw_address, bool extra_byte)
{
  if(raw_address)
    Load_Register(ECX, length);
  else
    Mov(ECX, length);

  if(extra_byte)
    Inc(ECX);

  if(raw_address)
    Load_Register(ESI, static_cast<int>(address));
  else
    Mov(ESI, address);

  Move_Register(EDI, dest);
  *p++ = REP;
  *p++ = MOVSB;
}