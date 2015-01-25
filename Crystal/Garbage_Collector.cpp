#include <boost\date_time.hpp>

#include "Garbage_Collector.h"
#include "Config.h"

GC global_gc;

GC::GC() : last_cleanup(0) 
{
  generations.reserve(MAX_STACK_DEPTH);
}

GC::~GC()
{ 
  auto mem_walker = ++used_blocks.begin();
  
  while(mem_walker != used_blocks.end())
  {
    if((*mem_walker)->type == CRY_STRING)
      free((*mem_walker)->str);
    else
      free((*mem_walker)->sym);
    free((*mem_walker));
    mem_walker++;
  }

  while(!free_blocks.empty())
  {
    free(free_blocks.top());
    free_blocks.pop();
  }
}

Crystal_Symbol* GC::Allocate()
{
  if(used_blocks.size() > gen_cap)
  {
    Collect();
    if(used_blocks.size() > gen_cap * 0.8)
    {
      gen_cap *= GROWTH_RATE;
    }
  }

  Crystal_Symbol* sym;
  if(free_blocks.size())
  {
    sym = free_blocks.top();
    free_blocks.pop();
  }
  else
  {
    sym = reinterpret_cast<Crystal_Symbol*>(calloc(1, sizeof(Crystal_Symbol)));
  }
  used_blocks.push_back(sym);

  return sym;
}

void GC::Collect()
{
  // Check if we have enought blocks to care collecting about.
  if(used_blocks.size() >= MINIMUM_BLOCKS)
    return;

  // Check if we need to do any collection
  unsigned curr_time = static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds());
  if(last_cleanup < curr_time - COLLECTION_DELAY)
  {
    // Mark
    for(unsigned i = 0; i < generations.size(); i++)
    {
      Root r = generations[i];
      for(int block = 0; block < r.block_size; block++)
      {
        (r.base + block)->sweep = false;
        Mark(r.base + block);
      }
    }

    // Sweep
    auto mem_walker = ++used_blocks.begin();

    while(mem_walker != used_blocks.end())
    {
      if(!(*mem_walker)->sweep)
      {
        // Free memory based on underlying type.
        // Strings have to be freed diffrently then all other types.
        if((*mem_walker)->type == CRY_STRING)
          free((*mem_walker)->str);
        else
          free((*mem_walker)->sym);

        // Add blocks to the free list.
        // If the free list is full then delete the block.
#if MAX_FREE_LIST > -1
        if(free_blocks.size() < MAX_FREE_LIST)
        {
          (*mem_walker)->sym = 0;
          free_blocks.push(*mem_walker);
        }
        else
        {
          free(*mem_walker);
        }
#else
        (*mem_walker)->sym = 0;
        free_blocks.push(*mem_walker);
#endif
        used_blocks.erase(mem_walker++);
      }
      else
      {
        (*mem_walker)->sweep = false;
        mem_walker++;
      }
    }

    // Reset cleanup time
    last_cleanup = curr_time;
  }
}

void GC::Prune()
{
  generations.pop_back();
}

void GC::Branch(Crystal_Symbol* root, int blocks)
{
  gen_cap = USAGE_LIMIT;
  generations.push_back(GC::Root(root, blocks));
}

void GC::Mark(Crystal_Symbol* ptr)
{
  if(ptr->sweep == true)
    return;

  ptr->sweep = true;
  if(ptr->type == CRY_ARRAY)
  {
    for(unsigned i = 0; i < ptr->size; i++)
    {
      if(ptr->sym[i].type == CRY_POINTER)
        Mark(ptr->sym[i].sym);
    }
  }
  else if(ptr->type == CRY_POINTER)
  {
    Mark(ptr->sym);
  }
}

Crystal_Symbol* GC_Allocate()
{
  return global_gc.Allocate();
}

void GC_Collect()
{
  // Pull out generation data.
  global_gc.Prune();
}

void GC_Branch(Crystal_Symbol* base, int block_size)
{
  global_gc.Branch(base, block_size);
}

void GC_Extend_Generation(Crystal_Symbol* root)
{
  root->sym->sweep = true;
}