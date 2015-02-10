#ifndef CRYSTAL_MARSHAL
#define CRYSTAL_MARSHAL

#include "Crystal.h"
#include <stdio.h>

const unsigned MARSHAL_SIG = 0xAF2184BC;

class Marshaler {
public:
  Marshaler() : handle(NULL) {};
  ~Marshaler() { Close(); };

  void Open(const char* filename);
  void Dump(Crystal_Symbol* sym);
  void Load(const char* filename, Crystal_Symbol* dest);
  void Close();

private:
  unsigned count;
  FILE* handle;
};

#endif