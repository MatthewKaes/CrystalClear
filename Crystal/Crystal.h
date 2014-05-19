#ifndef CRYSTAL_MAIN
#define CRYSTAL_MAIN

#include <string>

//Controls Higherarchy
enum Symbol_Type : char { CRY_NIL = 0, CRY_BOOL, CRY_INT, CRY_INT64, CRY_DOUBLE, CRY_TEXT, 
                           CRY_STRING, CRY_ARRAY, CRY_POINTER, CRY_CLASS_OBJ };

enum Data_Type : char { DAT_NIL, DAT_INT, DAT_INT64, DAT_DOUBLE, DAT_BOOL, 
                        DAT_LOCAL, DAT_STRING, DAT_OP, DAT_ARRAY };

class Crystal_Symbol
{
public:
  union {
    Crystal_Symbol* sym;
    char* str;
  } ptr;
  Symbol_Type type;
  unsigned short ex_dat;
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
  Data_Type type;
  std::string str;
};

#endif