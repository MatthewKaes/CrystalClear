#ifndef CRYSTAL_MACHINE
#define CRYSTAL_MACHINE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#define DEBUG_PROGRAM -1
#define NON_EXECUTABLE -2
#define COMPILER_ERRORS -3

enum RETURN_TYPES { VOID_RETURN, CONST_RETURN, LOCAL_RETURN };
enum REGISTERS : unsigned char { EAX = 0x85, EBX = 0x9D, ECX = 0x8D, EDX = 0x95, ESI = 0xB5, EDI = 0xBD };
enum LOAD_TYPES { LOCAL_LOAD, CONST_LOAD, REG_LOAD };
enum ARG_TYPES { AOT_MEMORY, AOT_INT, AOT_BOOL, AOT_STRING, AOT_FLOAT, AOT_DOUBLE, AOT_CHAR, AOT_REG };
enum VAR_TYPES { _INT, _FLOAT, _DOUBLE };
enum FPU_REGISTERS { ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7 };

typedef int (*FuncPtr)(void *);
typedef unsigned char BYTE;


struct LINKER_Data
{
  std::string name;
  std::vector<BYTE*> refrence_list;
};

class ARG
{
public:
  ARG(int num) : num_(num), type(AOT_INT){};
  ARG(bool bol) : bol_(bol), type(AOT_BOOL){};
  ARG(REGISTERS reg) : reg_(reg), type(AOT_REG){};
  ARG(const char* str) : str_(str), type(AOT_STRING){};
  ARG(float flt) : flt_(flt), type(AOT_FLOAT){};
  ARG(double dec) : dec_(dec), type(AOT_DOUBLE){};
  ARG(char chr) : chr_(chr), type(AOT_CHAR){};
  ARG(void* mem) : mem_((unsigned)mem), type(AOT_MEMORY){};
  union{
    unsigned mem_;
    int num_;
    bool bol_;
    float flt_;
    double dec_;
    char chr_;
    REGISTERS reg_;
    const char* str_;
  };
  ARG_TYPES type;
private:
  ARG();
};

class AOT_Compiler
{
public:
  //==========================
  // Compiler Functionallity
  //==========================
  //Get the hooks
  virtual ~AOT_Compiler(){};
  virtual void Setup(std::string name, BYTE* program) = 0;
  //Compiler info
  virtual const char* Get_Version() = 0;
  virtual const char* Get_Name() = 0;
  virtual std::vector<LINKER_Data> Get_Links() = 0;

  //==========================
  // Assembler Functionallity
  //==========================

  //System Functions
  virtual void Allocate_Stack(unsigned bytes) = 0;
  virtual void Make_Label(unsigned label) = 0;
  virtual unsigned New_Label() = 0;
  virtual unsigned Reserve_Label() = 0;
  virtual unsigned Last_Label() = 0;
  //System calls
  virtual void Print(ARG argument) = 0;
  //Stack Operations
  virtual void Push(ARG argument) = 0;
  virtual void Push_Adr(unsigned address) = 0;
  virtual void Push_LD(unsigned address, ARG_TYPES type) = 0;
  virtual void Pop(unsigned bytes = 0) = 0;
  //By name addressing
  virtual void Load_Mem(unsigned address, ARG argument) = 0;
  //Register Addressing
  virtual void Load_Register(REGISTERS reg, ARG argument) = 0;
  //Memory Addressing
  virtual void Load_Ptr(unsigned ptr) = 0;
  virtual void MovP(unsigned addr, unsigned offset = 0, bool byte = false) = 0;
  //Instructions
  virtual void Mov(REGISTERS dest, unsigned address, bool byte = false) = 0;
  virtual void Mov(unsigned address, REGISTERS source, bool byte = false) = 0;
  virtual void Lea(REGISTERS dest, unsigned address) = 0;
  virtual void Xchg_Register(REGISTERS dest, REGISTERS source) = 0;
  virtual void Add(REGISTERS dest, REGISTERS source) = 0;
  virtual void Add(unsigned address, REGISTERS source) = 0;
  virtual void Sub(REGISTERS dest, REGISTERS source) = 0;
  virtual void Sub(unsigned address, REGISTERS source) = 0;
  virtual void Mul(REGISTERS dest, REGISTERS source) = 0;
  virtual void Imul(unsigned address) = 0;
  virtual void Imul(REGISTERS dest, unsigned address) = 0;
  virtual void Dec(REGISTERS dest) = 0;
  virtual void Inc(REGISTERS dest) = 0;
  virtual void Or(unsigned address, REGISTERS source) = 0;
  virtual void And(unsigned address, REGISTERS source) = 0;
  virtual void Cmp(unsigned address, ARG argument) = 0;
  virtual void CmpF(unsigned address, ARG argument) = 0;
  virtual void Jmp(unsigned label) = 0;
  virtual void Je(unsigned label) = 0;
  virtual void Jne(unsigned label) = 0;
  virtual void Jle(unsigned label) = 0;
  virtual void Jl(unsigned label) = 0;
  virtual void Jg(unsigned label) = 0;
  virtual void Jge(unsigned label) = 0;
  virtual void Call(void* function) = 0;
  virtual void Call(const char* function) = 0;
  virtual void Return(ARG argument = 0) = 0;

  //--------------------------
  // Floating Point
  //--------------------------
  //Stack Operations
  virtual void FPU_Load(ARG argument) = 0;
  virtual void FPU_Loadf(unsigned address) = 0;
  virtual void FPU_Loadd(unsigned address) = 0;
  virtual void FPU_Loadi(unsigned address) = 0;
  virtual void FPU_Store(unsigned address) = 0;
  //Pop instructions
  virtual void FPU_Add(FPU_REGISTERS reg = ST1) = 0;
  virtual void FPU_Sub(FPU_REGISTERS reg = ST1) = 0;
  virtual void FPU_Mul(FPU_REGISTERS reg = ST1) = 0;
  virtual void FPU_Div(FPU_REGISTERS reg = ST1) = 0;
  //Non Pop ST(0) Instructions
  virtual void FPU_Sqr() = 0;
  virtual void FPU_Abs() = 0;
  virtual void FPU_Root() = 0;
  //Special x86 extensions
  virtual void FPU_Xchg() = 0;
  virtual void FPU_Invert() = 0;
  virtual void FPU_Neg() = 0;
  virtual void FPU_Round() = 0;
  //Special Constants
  virtual void FPU_PI() = 0;
  virtual void FPU_Zero() = 0;
  virtual void FPU_One() = 0;
  //Trig Functions
  virtual void FPU_Sin() = 0;
  virtual void FPU_Cos() = 0;
  
  //--------------------------
  // Special Functions
  //--------------------------
  virtual void Strcpy(REGISTERS dest, unsigned address, int length, bool raw_address = false) = 0;
  
  //--------------------------
  // Memory Functions
  //--------------------------
  virtual int String_Address(const char* str) = 0;
  virtual int Double_Address(double dec) = 0;
  virtual int Float_Address(float dec) = 0;

  //==========================
  // Compiler Components
  //==========================
  struct AOT_Var
  {
    //std::string name;
    unsigned lab;
    unsigned adr;
    BYTE* loc;
    VAR_TYPES type;
  };
  struct LINKER_Var
  {
    std::string name;
    double dec;
    float flt;
    std::vector<BYTE*> refrence_list;
  };
};

#endif
