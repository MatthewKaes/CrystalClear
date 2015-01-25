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
  void Prune();
  void Branch(Crystal_Symbol* root, int blocks);

private:
  struct Root {
    Root(Crystal_Symbol* _base, int _size) : base(_base), block_size(_size) {};
    Crystal_Symbol* base;
    int block_size;
  };

  void Mark(Crystal_Symbol* ptr);

  std::stack<Crystal_Symbol*> free_blocks;
  std::list<Crystal_Symbol*> used_blocks;

  unsigned gen_cap;
  std::vector<Root> generations;
};

// Exposed Global Garbage Collector Operations
Crystal_Symbol* GC_Allocate();
void GC_Collect();
void GC_Branch(Crystal_Symbol* base, int size);

void GC_Extend_Generation(Crystal_Symbol* root);

#endif