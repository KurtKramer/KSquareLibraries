/* StatisticalFunctions.cpp -- Basic Statistical Functions
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"

#include  <stdio.h>
#include  <math.h>
#include  <string>
#include  <iostream>
#include  <fstream>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;


#include  "StatisticalFunctions.h"
#include  "KKBaseTypes.h"
#include  "OSservices.h"
using namespace KKB;


float  KKB::PairedTTest (const VectorFloat&  set1,
                         const VectorFloat&  set2
                        )
{
  // Formula was taken from MathWorld
  // http://mathworld.wolfram.com/Pairedt-Test.html

  KKCheck (set1.size () == set2.size (), "PairedTTest  two sets not the same length;  set1.size: " + StrFromUint64 (set1.size ()) + " set2.size: " << StrFromUint64 (set2.size ()))

  double  numOfPairs = (double)set1.size ();
  double xTotal = 0.0f;
  double yTotal = 0.0f;

  kkint32  foldNum = 0;

  for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  {
    xTotal += set1[foldNum];
    yTotal += set2[foldNum];
  }

  double  xMean = xTotal / numOfPairs;
  double  yMean = yTotal / numOfPairs;

  double  totalDeltaSquared = 0.0f;

  for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  {
    double  xDelta = set1[foldNum] - xMean;
    double  yDelta = set2[foldNum] - yMean;

    double  deltaDelta = xDelta - yDelta;
    totalDeltaSquared += (deltaDelta * deltaDelta);
  }

  double  tValue = 0.0f;
  if  (totalDeltaSquared != 0.0)
    tValue = fabs ((xMean - yMean) * sqrt ((numOfPairs * (numOfPairs - 1.0)) / totalDeltaSquared));

  return  float (tValue);
}  /* PairedTTest */



float  KKB::McNemarsTest (kkint32      size,
                          const bool*  classedCorrectly1,
                          const bool*  classedCorrectly2
                         )
{
  kkint32  n00 = 0;
  kkint32  n01 = 0;
  kkint32  n10 = 0;
  kkint32  n11 = 0;

  kkint32  x;

  for  (x = 0;  x < size;  x++)
  {
    bool  ca = classedCorrectly1[x];
    bool  cb = classedCorrectly2[x];

    if  (!ca && !cb)
      n00++;   // true neg.

    else if  (!ca && cb)
      n01++;   // false pos.

    else if  (ca && !cb)
      n10++;   // false neg.

    else if  (ca && cb)
      n11++;   // true pos
  }

  float y = (float)fabs ((float)(n01 - n10)) - 1.0f;    // (false pos - false neg) - 1.0;

  float  mcNemars = 0.0;
  float  divisor = (float)n01 + (float)n10;

  if  (divisor != 0.0f)
    mcNemars =  y * y / divisor;

  return  mcNemars;
}  /* McNemarsTest */



/*
template <class T> T  Min (T  a, 
                           T  b
                          )
{
  if  (a <= b)
    return  a;
  else
    return  b;
}
*/



float   KKB::LLoydsIndexOfPatchiness (const VectorInt& bins)
{
  // As defined by Andrew Remsen
  // also look at http://www.pmel.noaa.gov/pubs/outstand/stab1646/statistics.shtml

  if  (bins.size () < 1)
    return  0.0f;

  kkuint32  x;
  double  total = 0.0;
  double  mean  = 0.0;

  for  (x = 0;  x < bins.size ();  x++)
    total += bins[x];

  mean = total / double (bins.size ());

  double  totalDeltaSquared = 0.0;

  for  (x = 0;  x < bins.size ();  x++)
  {
    double  delta = double (bins[x]) - mean;
    totalDeltaSquared += delta * delta;
  }

  double  var = totalDeltaSquared / (double)bins.size ();

  double  varDivMean = var / mean;
  double  oneDivMean = 1.0 / mean;

  float p =  float ((varDivMean - 1.0 + mean) * oneDivMean);

  return  p;
}  /* LLoydsIndexOfPatchiness */





