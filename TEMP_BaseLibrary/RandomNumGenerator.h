/* RandomNumGenerator.h -- Class that represents one Random Number Generator.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _KKU_RANDOMNUMGENERATOR_
#define  _KKU_RANDOMNUMGENERATOR_

#include  "BasicTypes.h"

namespace  KKU
{
  /**
   *@brief  Represents one single random number generator.
   *@details  Rather than using the two global functions "lrand48" and "srand48" you can create an
   *          instance of this class that will provide the random sequence. The advantage is that
   *          you can control who has access to this sequence. That is you do not need to worry that
   *          some other code that you are calling is not also getting numbers from this sequence.
   *          This will allow you to guarantee the same sequence of numbers for the same seed.
   */
  class  RandomNumGenerator
  {
  public:
    RandomNumGenerator ();
    RandomNumGenerator (long  _seed);
  
    ~RandomNumGenerator ();
  
    long  Next ();
  
  
  private:
    int64 _lrand48_sequence;
  
    static const int64 a; 
    static const int64 c; 
    static const int64 m;
    static const int64 mask;
  };  /* RandomNumGenerator */
}

#endif

