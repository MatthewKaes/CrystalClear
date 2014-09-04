#ifndef CRYSTAL_MAIN
#define CRYSTAL_MAIN

#include <string>

//Controls Higherarchy
enum Symbol_Type : char { CRY_NIL = 0, CRY_BOOL, CRY_INT, CRY_INT64, CRY_DOUBLE, CRY_TEXT, 
                           CRY_STRING, CRY_ARRAY, CRY_POINTER, CRY_CLASS_OBJ, CRY_SYMS };

enum Data_Type : char { DAT_NIL, DAT_LOOKUP, DAT_INT, DAT_INT64, DAT_DOUBLE, DAT_BOOL, 
                        DAT_LOCAL, DAT_STRING, DAT_OP, DAT_FUNCTION, DAT_BIFUNCTION, DAT_ARRAY, 
                        DAT_REGISTRY };

class Crystal_Symbol
{
public:
  Symbol_Type type;
  unsigned char ref_cnt;
  unsigned short ex_dat;
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
