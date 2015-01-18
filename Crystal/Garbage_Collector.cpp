#include <boost\date_time.hpp>

#include "Garbage_Collector.h"
#include "Config.h"

GC global_gc;

GC::GC() : generation(0), last_cleanup(0) {}

GC::~GC()
{ 
  auto mem_walker = used_blocks.begin();
  
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
  sym->generation = generation;
  used_blocks.push_back(sym);

  return sym;
}

void GC::Collect()
{
  // Check if we need to do any collection
  generation--;
  unsigned curr_time = static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds());
  if(last_cleanup < curr_time - COLLECTION_DELAY)
  {
    // Mark
    auto mem_walker = used_blocks.begin();

    while(mem_walker != used_blocks.end())
    {
      // Only clear newer generations as old memory most likely
      // needs to be persisted.
      if((*mem_walker)->generation <= generation)
        Mark(*mem_walker);

      mem_walker++;
    }

    // Sweep
    mem_walker = used_blocks.begin();

    while(mem_walker != used_blocks.end())
    {
      if(!(*mem_walker)->sweep)
      {
        if((*mem_walker)->type == CRY_STRING)
          free((*mem_walker)->str);
        else
          free((*mem_walker)->sym);

        if(free_blocks.size() < MAX_FREE_LIST)
        {
          (*mem_walker)->sym = 0;
          free_blocks.push(*mem_walker);
        }
        else
        {
          free(*mem_walker);
        }

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

void GC::Branch()
{
  generation++;
}

void GC::Mark(Crystal_Symbol* ptr)
{
  if(ptr->sweep == true)
    return;

  ptr->sweep = true;
  if(ptr->type != CRY_STRING)
  {
    for(unsigned i = 0; i < ptr->size; i++)
    {
      if(ptr->sym[i].type == CRY_POINTER)
        Mark(ptr->sym[i].sym);
    }
  }
}

Crystal_Symbol* GC_Allocate()
{
  return global_gc.Allocate();
}

void GC_Collect()
{
  global_gc.Collect();
}

void GC_Branch()
{
  global_gc.Branch();
}

void GC_Extend_Generation(Crystal_Symbol* root)
{
  root->sym->generation--;
}