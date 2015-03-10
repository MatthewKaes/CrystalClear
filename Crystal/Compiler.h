#ifndef CRYSTAL_COMPILER
#define CRYSTAL_COMPILER

#include <windows.h>
#include <vector>
#include <unordered_map>

#include "Crystal.h"
#include "Clarity_Filter.h"
#include "Library.h"
#include "Machines\Machine.h"

//Crystal Constants
#define STRING_POOL 0xFFFF
#define STACK_RESERVE 0xFF
#define MIN_BLOCK_SIZE 0x20
//Return Address should be the side of a symbol
//This allows us to have cache alignment on the stack.
//(stack frames are always multiples of 16)
#define RETURN_ADDRESS 0x10
#define CRY_NULL static_cast<unsigned>(-1)

//Memory layouts
#define VAR_SIZE sizeof(Crystal_Symbol)
#define DATA_LOWER (unsigned)&(((Crystal_Symbol *)0)->LOWWER)
#define DATA_UPPER (unsigned)&(((Crystal_Symbol *)0)->UPPER)
#define DATA_TYPE (unsigned)&(((Crystal_Symbol *)0)->type)
#define DATA_PNTR (unsigned)&(((Crystal_Symbol *)0)->sym)

struct CryPackage
{
  std::vector<LINKER_Data> links;
  CryProg program;
  std::string name;
};

class CRY_ARG
{
public:
  CRY_ARG() : type(CRY_NIL){};
  CRY_ARG(int num) : num_(num), type(CRY_INT), filt(CRY_INT){};
  CRY_ARG(__int64 lrg) : lrg_(lrg), type(CRY_INT64), filt(CRY_INT64){};
  CRY_ARG(bool bol) : bol_(bol), type(CRY_BOOL), filt(CRY_BOOL){};
  CRY_ARG(double dec) : dec_(dec), type(CRY_DOUBLE), filt(CRY_DOUBLE){};
  CRY_ARG(const char* str);
  CRY_ARG(Crystal_Data* sym);
  union{
    int num_;
    __int64 lrg_;
    bool bol_;
    double dec_;
    char* str_;
  };
  Symbol_Type type;
  Clarity_Filter filt;

  static char strpool[STRING_POOL];
  static int poolindex;
};

class Crystal_Compiler 
{
public:
  Crystal_Compiler(AOT_Compiler* target);
  ~Crystal_Compiler();
  void Start_Encode(std::string name, unsigned locals_used, unsigned stack_size, unsigned arguments = 0);
  void End_Encode();
  void Linker();
  int Execute(Crystal_Symbol* ret = 0);

  //System calls
  void Print(unsigned var);

  //System Functions
  void Call(void* function);
  void Call(void* function, unsigned reg);
  void Call(const char* cry_function, unsigned reg = CRY_NULL);
  void Call(const char* binding, unsigned op, unsigned ret);
  void Get(const char* binding, unsigned op, unsigned ret);
  void Convert(unsigned reg, Symbol_Type type);
  void Allocate(unsigned sym_count);
  void Make_Array(unsigned var, unsigned size, unsigned capacity);
  void Make_Class(unsigned var, unsigned ID);
  void Make_Range(unsigned var);
  void Push(unsigned var);
  void Push_C(CRY_ARG var);
  void Push_Reg();
  void Pop(unsigned args);
  void Return(unsigned var);
  void Return();
  void Loop();
  void While(unsigned var);
  void If(unsigned var);
  void Else();
  void ElseIf_Pre();
  void ElseIf(unsigned var);
  void End();
  
  void Load(unsigned var, CRY_ARG val = CRY_ARG());
  void Ref_Load(unsigned var, CRY_ARG val = CRY_ARG());
  void Copy(unsigned dest, unsigned source);
  void Swap(unsigned dest, unsigned source);
  void Add(unsigned dest, unsigned source, bool left = true);
  void AddC(unsigned dest, CRY_ARG const_, bool left = true);
  void Sub(unsigned dest, unsigned source, bool left = true);
  void SubC(unsigned dest, CRY_ARG const_, bool left = true);
  void Mul(unsigned dest, unsigned source, bool left = true);
  void MulC(unsigned dest, CRY_ARG const_, bool left = true);
  void Div(unsigned dest, unsigned source, bool left = true);
  void DivC(unsigned dest, CRY_ARG const_, bool left = true);
  void Mod(unsigned dest, unsigned source, bool left = true);
  void ModC(unsigned dest, CRY_ARG const_, bool left = true);
  void Pow(unsigned dest, unsigned source, bool left = true);
  void PowC(unsigned dest, CRY_ARG const_, bool left = true);
  void And(unsigned dest, unsigned source, bool left = true);
  void AndC(unsigned dest, CRY_ARG const_, bool left = true);
  void Or(unsigned dest, unsigned source, bool left = true);
  void OrC(unsigned dest, CRY_ARG const_, bool left = true);
  void Eql(unsigned dest, unsigned source, bool left = true);
  void EqlC(unsigned dest, CRY_ARG const_, bool left = true);
  void Dif(unsigned dest, unsigned source, bool left = true);
  void DifC(unsigned dest, CRY_ARG const_, bool left = true);
  void Les(unsigned dest, unsigned source, bool left = true);
  void LesC(unsigned dest, CRY_ARG const_, bool left = true);
  void LesEql(unsigned dest, unsigned source, bool left = true);
  void LesEqlC(unsigned dest, CRY_ARG const_, bool left = true);
  void Gtr(unsigned dest, unsigned source, bool left = true);
  void GtrC(unsigned dest, CRY_ARG const_, bool left = true);
  void GtrEql(unsigned dest, unsigned source, bool left = true);
  void GtrEqlC(unsigned dest, CRY_ARG const_, bool left = true);

  unsigned Addr_Reg(unsigned reg);
private:
  //==========================
  // Helper Functions
  //==========================
  bool Null_Op(Clarity_Filter& l, Clarity_Filter& r, unsigned dest, Clarity_Filter Exceptions = Clarity_Filter(CRY_NIL));
  void Runtime_Resovle(unsigned dest, Symbol_Type resolve);
  void Safe_Set_Pointer(unsigned var, CRY_ARG val);

  //==========================
  // Machine Code
  //==========================
  AOT_Compiler* Machine;

  //==========================
  // Optimization Management
  //==========================
  std::vector<Clarity_Filter> states;

  //==========================
  // Program Management
  //==========================
  CryProg program;
  unsigned locals_count;
  unsigned stack_size;
  unsigned stack_depth;
  std::vector<CryPackage> packages;
  std::unordered_map<std::string, CryProg> package_lookup;
  
  //==========================
  // Function Management
  //==========================
  std::vector<CryLookup> lookups;
};

#endif
