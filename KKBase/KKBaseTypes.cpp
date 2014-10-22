/* KKStrParser.cpp -- Class used to parse string into tokens.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include <float.h>

#include "FirstIncludes.h"
#include <ctype.h>
#include <cmath>
#include <math.h>
#include <stdio.h>
using namespace  std;

#include  "KKBaseTypes.h"

using namespace KKB;



float   KKB::FloatMax  = FLT_MAX;
float   KKB::FloatMin  = FLT_MIN;
double  KKB::DoubleMax = DBL_MAX;
double  KKB::DoubleMin = DBL_MIN;

float  KKB::FloatAbs (float f)
{
  return (float)abs (f);
}


// Constant
kkint64         _lrand48_sequence = 0x1234ABCD330ELL;
//static KKB::int64  _lrand48_seed     = 305441741LL;

// lrand48 function
kkint32  KKB::LRand48 ()
{
  static const kkint64 a = 0x5DEECE66DLL;
  static const kkint64 c = 0xBLL;
  static const kkint64 m = 281474976710656LL;
  kkint64 mask = 281474976710655LL;

  _lrand48_sequence = ( ((a * _lrand48_sequence) & mask) + c ) % m;
  return (kkint32)(_lrand48_sequence >> 17);
}


// srand48 function
void  KKB::SRand48 (kkint64 _seed)
{
  kkint64 seedMask = 65535;
  _lrand48_sequence &= seedMask;
  kkint64 upperBits = _seed;
  upperBits <<= 16;
  _lrand48_sequence |= upperBits;
}



#if  defined(_WIN32)
bool   KKB::IsNaN (const float&  f)
{
  bool  notANum = _isnan (f) != 0;
  return  notANum;
}


bool   KKB::IsNaN (const double&  d)
{
  bool  notANum = _isnan (d) != 0;
  return  notANum;
}

bool  KKB::IsFinite (const float&  f)
{
  return (_finite (f) != 0);
}


bool  KKB::IsFinite (const double&  f)
{
  return (_finite (f) != 0);
}

#else
bool   KKB::IsNaN (const float&  f)
{
  bool  notANum = std::isnan (f) != 0;
  return  notANum;
}


bool   KKB::IsNaN (const double&  d)
{
  bool  notANum = std::isnan (d) != 0;
  return  notANum;
}


bool  IsFinite (const float&  f)
{
  return (std::isfinite(f) != 0);
}


bool  IsFinite (const double&  f)
{
  return (std::isfinite(f) != 0);
}
#endif




WordFormat32Bits::WordFormat32Bits ()
{
  unsigned32BitInt = 0;
}



WordFormat32Bits::WordFormat32Bits (kkuint32  d)
{
  unsigned32BitInt = d;
}



WordFormat32Bits::WordFormat32Bits (kkint32  d)
{
  signed32BitInt = d;
}



WordFormat32Bits::WordFormat32Bits (uchar b0,
                                    uchar b1,
                                    uchar b2,
                                    uchar b3
                                   )   
{
  fourBytes.byte0 = b0;
  fourBytes.byte1 = b1;
  fourBytes.byte2 = b2;
  fourBytes.byte3 = b3;
}
