#ifndef CRYSTAL_CLARITY
#define CRYSTAL_CLARITY

#include "Crystal.h"

class Clarity_Filter
{
public:
  Clarity_Filter() : flags(1), dilution(1), collection(false){};
  Clarity_Filter(Symbol_Type flag);
  void Set(Symbol_Type flag);
  void Dilute(Symbol_Type flag);
  void Obscurity();
  bool Test(Symbol_Type flag);
  bool Collection();
  unsigned Size();
  static Symbol_Type Reduce(Clarity_Filter l, Clarity_Filter r);
  static void Combind(Clarity_Filter& l, Clarity_Filter& r);
private:
  void Dynamic(Symbol_Type flag);
  unsigned flags;
  unsigned dilution;
  bool collection;
};

#endif