#ifndef CRYSTAL_MAIN
#define CRYSTAL_MAIN

#include "Config.h"
#include <string>
#include <vector>
#include <unordered_map>

//Crystal Macros
#define NO_SUPPORT(type) \
    case type: \
      resolve = CRY_NIL; \
      break;

//Crystal Types
typedef int (*FuncPtr)(void *);
typedef unsigned char BYTE;


//Controls Higherarchy
enum Symbol_Type : char { CRY_NIL = 0, CRY_BOOL, CRY_INT, CRY_INT64, CRY_DOUBLE, CRY_TEXT, 
                           CRY_POINTER, CRY_STRING, CRY_ARRAY,  CRY_CLASS_OBJ, CRY_SYMS };

enum Data_Type : char { DAT_NIL, DAT_LOOKUP, DAT_INT, DAT_INT64, DAT_DOUBLE, DAT_BOOL, 
                        DAT_LOCAL, DAT_STRING, DAT_OP, DAT_FUNCTION, DAT_BIFUNCTION, DAT_ARRAY, 
                        DAT_REGISTRY, DAT_REF, DAT_STATEMENT };

enum PACKAGE_TYPE { PGK_EXE, PGK_ATR, PKG_FUC };

union funcptr {
  FuncPtr call;
  BYTE* load;
};

union CryProg {
  FuncPtr call;
  BYTE* load;
};

struct Package_Info {
  PACKAGE_TYPE pt;
  union{
    unsigned attributes;
    unsigned arguments;
  } info;
  void* function;
};

struct Class_Info {
  std::string name;
  std::vector<std::string> attributes;
  std::unordered_map<unsigned, Package_Info> lookup;
};

struct CryLookup
{
  int lable_id;
  int lable_block_id;
  int loop_back_lable;
  std::vector<bool> corruptions;

  static const int NO_LABLE = -1;
};

class Crystal_Symbol
{
public:
  Symbol_Type type;
  bool sweep;
  unsigned short UNUSED;
  union {
    Crystal_Symbol* sym;
    char* str;
  };
  union {
    int i32;
    __int64 i64;
    double d;
    bool b;
    struct {
      union {
        unsigned size;
        unsigned LOWWER;
      };
      union {
        const char* text;
        Class_Info* klass;
        unsigned capacity;
        unsigned UPPER;
      };
    };
  };
};

class Crystal_Data
{
public:
  union {
    int i32;
    __int64 i64;
    double d;
    bool b;
  };
  void* external;
  Data_Type type;
  std::string str;
};

#endif
