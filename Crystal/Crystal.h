#ifndef CRYSTAL_MAIN
#define CRYSTAL_MAIN

#include "Config.h"
#include <string>
#include <vector>

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
                           CRY_STRING, CRY_POINTER, CRY_ARRAY,  CRY_CLASS_OBJ, CRY_SYMS };

enum Data_Type : char { DAT_NIL, DAT_LOOKUP, DAT_INT, DAT_INT64, DAT_DOUBLE, DAT_BOOL, 
                        DAT_LOCAL, DAT_STRING, DAT_OP, DAT_FUNCTION, DAT_BIFUNCTION, DAT_ARRAY, 
                        DAT_REGISTRY, DAT_OBJ, DAT_STATEMENT };

enum PACKAGE_TYPE { PGK_EXE, PGK_OBJ };

union funcptr {
  FuncPtr call;
  BYTE* load;
};

union CryProg {
  FuncPtr call;
  BYTE* load;
};


class Crystal_Symbol
{
public:
  Symbol_Type type;
  unsigned char ex_data;
  unsigned short ref_cnt;
  union {
    Crystal_Symbol* sym;
    char* str;
  } ptr;
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

struct CryLookup
{
  int lable_id;
  int loop_back_lable;
  std::vector<bool> corruptions;
};

struct Package_Info {
  PACKAGE_TYPE pt;
  union{
    unsigned attributes;
    unsigned arguments;
  } info;
  void* function;
};

#endif
