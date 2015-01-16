#ifndef CRYSTAL_GC
#define CRYSTAL_GC

#include "Crystal.h"
#include "Config.h"

#include <stack>
#include <list>

// Grabage Collector
class GC {
public:
  GC();
  ~GC();

  Crystal_Symbol* Allocate();
  void Collect();
  void Branch();

private:
  void Mark(Crystal_Symbol* ptr);

  unsigned generation;
  unsigned last_cleanup;
  std::stack<Crystal_Symbol*> free_blocks;
  std::list<Crystal_Symbol*> used_blocks;
  std::stack<unsigned> generation_markers;
};

// Exposed Global Garbage Collector Operations
Crystal_Symbol* GC_Allocate();
void GC_Collect();
void GC_Branch();

void GC_Extend_Generation(Crystal_Symbol* root);

#endif