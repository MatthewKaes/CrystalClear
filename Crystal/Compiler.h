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
#define RETURN_ADDRESS 0x10
#define CRY_NULL static_cast<unsigned>(-1)

//Memory layouts
#define VAR_SIZE sizeof(Crystal_Symbol)
#define DATA_LOWER (unsigned)&(((Crystal_Symbol *)0)->LOWWER)
#define DATA_UPPER (unsigned)&(((Crystal_Symbol *)0)->UPPER)
#define DATA_TYPE (unsigned)&(((Crystal_Symbol *)0)->type)
#define DATA_PNTR (unsigned)&(((Crystal_Symbol *)0)->ptr)

//Crystal Macros
#define NO_SUPPORT(type) \
    case type: \
      resolve = CRY_NIL; \
      break;

union funcptr {
  FuncPtr call;
  BYTE* load;
};

union CryProg {
  FuncPtr call;
  BYTE* load;
};

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
  void Push(unsigned var);
  void Pop(unsigned args);
  void Return(unsigned var);
  void Return();
  
  void Load(unsigned var, CRY_ARG val = CRY_ARG());
  void Copy(unsigned dest, unsigned source);
  void Add(unsigned dest, unsigned source, bool left = true);
  void AddC(unsigned dest, CRY_ARG const_, bool left = true);
  void Sub(unsigned dest, unsigned source, bool left = true);
  void SubC(unsigned dest, CRY_ARG const_, bool left = true);
  void Mul(unsigned dest, unsigned source, bool left = true);
  void MulC(unsigned dest, CRY_ARG const_, bool left = true);
  void Pow(unsigned dest, unsigned source, bool left = true);
  void PowC(unsigned dest, CRY_ARG const_, bool left = true);
  void Eql(unsigned dest, unsigned source, bool left = true);
  void EqlC(unsigned dest, CRY_ARG const_, bool left = true);
  void Dif(unsigned dest, unsigned source, bool left = true);
  void DifC(unsigned dest, CRY_ARG const_, bool left = true);
  void Les(unsigned dest, unsigned source, bool left = true);
  void LesC(unsigned dest, CRY_ARG const_, bool left = true);

  unsigned Addr_Reg(unsigned reg);
private:
  //==========================
  // Helper Functions
  //==========================
  bool Null_Op(Clarity_Filter& l, Clarity_Filter& r, unsigned dest, Clarity_Filter Exceptions = Clarity_Filter(CRY_NIL));
  void Runtime_Resovle(unsigned dest, Symbol_Type resolve);
  void Safe_Set_Pointer(unsigned var, CRY_ARG val);
  void Garbage_Collection(unsigned var);

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
};

#endif
