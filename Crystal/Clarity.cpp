#include "Clarity.h"

Clarity_Filter::Clarity_Filter(Symbol_Type flag)
{
  Set(flag);
}
void Clarity_Filter::Set(Symbol_Type flag)
{
  flags = (0x1 << (flag));
  dilution = 1;
  Dynamic(flag);
}
void Clarity_Filter::Dilute(Symbol_Type flag)
{
  flags |= (0x1 << (flag));
  dilution++;
  Dynamic(flag);
}
void Clarity_Filter::Obscurity()
{
  flags = 0xffffffff;
  dilution = static_cast<unsigned>(-1);
}
bool Clarity_Filter::Test(Symbol_Type flag)
{
  if(flags & (0x1 << (flag)))
  {
    return true;
  }
  return false;
}
bool Clarity_Filter::Collection()
{
  return collection;
}
void Clarity_Filter::Collected()
{
  collection = false;
}
unsigned Clarity_Filter::Size()
{
  return dilution;
}
Symbol_Type Clarity_Filter::Reduce(Clarity_Filter l, Clarity_Filter r)
{
  unsigned flag = l.flags > r.flags ? l.flags : r.flags;
  unsigned reduction = 0;
  while(flag)
  {
    reduction++;
    flag >>= 1;
  }
  return static_cast<Symbol_Type>(reduction - 1);
}
void Clarity_Filter::Combind(Clarity_Filter& l, Clarity_Filter& r)
{
  l.flags = r.flags |= l.flags;
}
void Clarity_Filter::Dynamic(Symbol_Type flag)
{
  //Text is the highest non collected object.
  if(flag > CRY_TEXT)
  {
    collection = true;
  }
}
