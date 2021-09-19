/* RandomNumGenerator.cpp -- Class that represents one Random Number Generator.
 * Copyright (C) 1994-2014 Kurt Kramer  &   Daniel J. Garcia
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"

#include  <stdlib.h>
#include  <stdio.h>
#include  <iostream>
#include  <vector>

#include  "MemoryDebug.h"


#include  "KKBaseTypes.h"

using namespace std;
using namespace KKB;


#include "RandomNumGenerator.h"

using namespace  KKB;

RandomNumGenerator::RandomNumGenerator ()
{
#if  defined(WIN32)
  _lrand48_sequence = 0x1234ABCD330E;
#else
  _lrand48_sequence = 0x1234ABCD330ELLU;
#endif
}



RandomNumGenerator::RandomNumGenerator (long  _seed): RandomNumGenerator()
{
  kkint64 seedMask = 65535;
  _lrand48_sequence &= seedMask;
  kkint64 upperBits = _seed;
  upperBits <<= 16;
  _lrand48_sequence |= upperBits;
}



RandomNumGenerator::~RandomNumGenerator ()
{
}



const  kkint64 RandomNumGenerator::c     = 0xB; 
#if  defined(WIN32)
const  kkint64 RandomNumGenerator::a     = 0x5DEECE66D; 
const  kkint64 RandomNumGenerator::m     = 281474976710656;
const  kkint64 RandomNumGenerator::mask  = 281474976710655;
#else
const  kkint64 RandomNumGenerator::a     = 0x5DEECE66DLLU; 
const  kkint64 RandomNumGenerator::m     = 281474976710656LLU;
const  kkint64 RandomNumGenerator::mask  = 281474976710655LLU;
#endif



long  RandomNumGenerator::Next ()
{
  _lrand48_sequence = ( ((a * _lrand48_sequence) & mask) + c ) % m;
  return static_cast<long> (_lrand48_sequence >> 17);
}
