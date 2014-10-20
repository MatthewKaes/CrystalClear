#ifndef CRYSTAL_x86
#define CRYSTAL_x86

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#include "Machine.h"
#include "x86_Codes.h"

#define COMPILER_VERSION "1.0.4"
#define WORD_VARIANT WRD_OFF
#define STACK_SIZE 0xFF

class x86_Machine : public AOT_Compiler
{
public:
  //==========================
  // Compiler Functionallity
  //==========================
  x86_Machine();
  //Get the hooks
  void Setup(std::string name, BYTE* program);
  //Compiler info
  const char* Get_Version();
  const char* Get_Name();
  std::vector<LINKER_Data> Get_Links();

  //==========================
  // Assembler Functionallity
  //==========================

  //--------------------------
  // Abstract x86 
  //--------------------------
  //System Functions
  void Allocate_Stack(unsigned bytes);
  void Make_Label(unsigned label);
  unsigned New_Label();
  unsigned Reserve_Label();
  unsigned Last_Label();
  //System calls
  void Print(ARG argument);
  //Stack Operations
  void Push(ARG argument);
  void Push_Adr(unsigned address);
  void Push_Stk(unsigned address);
  void Push_LD(unsigned address, ARG_TYPES type);
  void Pop(unsigned bytes = 0);
  //By name addressing
  void Load_Mem(unsigned address, ARG argument);
  //Register Addressing
  void Load_Register(REGISTERS reg, ARG argument);
  //Memory Addressing
  void Load_Ptr(unsigned ptr);
  void MovP(unsigned addr, unsigned offset = 0, bool byte = false);
  //Instructions
  void Mov(REGISTERS dest, unsigned address, bool byte = false);
  void Mov(unsigned address, REGISTERS source, bool byte = false);
  void Lea(REGISTERS dest, unsigned address);
  void Xchg_Register(REGISTERS dest, REGISTERS source);
  void Add(REGISTERS dest, REGISTERS source);
  void Add(unsigned address, REGISTERS source);
  void Sub(REGISTERS dest, REGISTERS source);
  void Sub(unsigned address, REGISTERS source);
  void Sub(REGISTERS source, unsigned address);
  void Mul(REGISTERS dest, REGISTERS source);
  void Imul(unsigned address);
  void Imul(REGISTERS dest, unsigned address);
  void Idiv(unsigned address);
  void Idiv(REGISTERS dest);
  void Dec(REGISTERS dest);
  void Inc(REGISTERS dest);
  void Or(unsigned address, REGISTERS source);
  void And(unsigned address, REGISTERS source);
  void Cmp(unsigned address, ARG argument);
  void CmpF(unsigned address, ARG argument);
  void Jmp(unsigned label);
  void Je(unsigned label);
  void Jne(unsigned label);
  void Jle(unsigned label);
  void Jl(unsigned label);
  void Jg(unsigned label);
  void Jge(unsigned label);
  void Sete(unsigned address);
  void Setne(unsigned address);
  void Setl(unsigned address);
  void Setle(unsigned address);
  void Setg(unsigned address);
  void Setge(unsigned address);
  void Seta(unsigned address);
  void Setae(unsigned address);
  void Setb(unsigned address);
  void Setbe(unsigned address);
  void Call(void* function);
  void Call(const char* function);
  void Return(ARG argument = 0);

  //--------------------------
  // Abstract x87
  //--------------------------
  //Stack Operations
  void FPU_Load(ARG argument);
  void FPU_Loadf(unsigned address);
  void FPU_Loadd(unsigned address);
  void FPU_Loadi(unsigned address);
  void FPU_Store(unsigned address);
  //OP instructions
  void FPU_Cmp(FPU_REGISTERS reg = ST1);
  //Pop instructions
  void FPU_Add(FPU_REGISTERS reg = ST1);
  void FPU_Sub(FPU_REGISTERS reg = ST1);
  void FPU_Mul(FPU_REGISTERS reg = ST1);
  void FPU_Div(FPU_REGISTERS reg = ST1);
  //Non Pop ST(0) Instructions
  void FPU_Sqr();
  void FPU_Abs();
  void FPU_Root();
  //Special x86 extensions
  void FPU_Xchg();
  void FPU_Invert();
  void FPU_Neg();
  void FPU_Round();
  //Special Constants
  void FPU_PI();
  void FPU_Zero();
  void FPU_One();
  //Trig Functions
  void FPU_Sin();
  void FPU_Cos();
  
  //--------------------------
  // Special Functions
  //--------------------------
  void Strcpy(REGISTERS dest, unsigned address, int length, bool raw_address = false, bool extra_byte = false);

  //--------------------------
  // Memory Functions
  //--------------------------
  int String_Address(const char* str);
  int Double_Address(double dec);
  int Float_Address(float dec);
private:
  //==========================
  // Compiler Components
  //==========================
  //Simple x86 register helper functions
  unsigned char Reg_to_Reg(REGISTERS dest, REGISTERS source);
  unsigned Reg_Id(REGISTERS reg);
  void Reg_Op(unsigned address, REGISTERS source);
  void Move_Register(REGISTERS dest, REGISTERS source); 
  void Put_Addr(unsigned addr, int op_offset = BYT_ADR); 
  unsigned char two_complement_8(unsigned char id);
  unsigned two_complement_32(unsigned id);
  //Addressing functions used for AOT execution mode.
  //Linker functions
  void Label_Management(unsigned label);
  int Add_Double(double dec);
  int Add_Float(float dec);
  int Add_String(const char* str);

  //Compiler Information Crawl
  //Compiler lable set
  std::vector<AOT_Var> cls;
  //Compiler jump set
  std::vector<AOT_Var> cjs;
  
  //==========================
  // Linker Components
  //==========================
  //Linker Information Crawl
  //Executable string pool
  std::vector<LINKER_Var> esp;
  //Executable float pool
  std::vector<LINKER_Var> efp;
  //Executable double pool
  std::vector<LINKER_Var> edp;
  //DATA Section mapping
  unsigned d_section_adr;

  //==========================
  // Crystal Components
  //==========================
  //function links
  std::vector<LINKER_Data> call_links;

  //==========================
  // State Management
  //==========================
  BYTE* p;
  unsigned stack_size;
  unsigned pushed_bytes;
  const char* name_;
  bool stack_allocated;
  bool compiler_error;
  std::string prg_id;
};

#endif
