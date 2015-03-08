#ifndef CRYSTAL_CLARITY
#define CRYSTAL_CLARITY

#include "Crystal.h"

/*========================================================//
    Clarity Filter

  Clarity Filters are used for type resolution at compile
  time. In essence they are a speical type of bloom filter
  used for testing type sets.
//=======================================================*/
class Clarity_Filter
{
public:
  Clarity_Filter() : flags(1), dilution(1){};
  Clarity_Filter(Symbol_Type flag);
  void Set(Symbol_Type flag);
  void Dilute(Symbol_Type flag);
  void Obscurity();
  bool Test(Symbol_Type flag);
  bool Only(Symbol_Type flag);
  bool Order(Symbol_Type flag);
  bool Compare(Clarity_Filter& filter);
  unsigned Size();
  static Symbol_Type Reduce(Clarity_Filter l, Clarity_Filter r);
  static void Combind(Clarity_Filter& l, Clarity_Filter& r);
private:
  unsigned flags;
  unsigned dilution;
};

#endif
