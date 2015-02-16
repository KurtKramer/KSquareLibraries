/* StatisticalFunctions.cpp -- Basic Statitical Functions
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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
#include  "BasicTypes.h"
#include  "OSservices.h"
using namespace KKU;


float  KKU::PairedTTest (const VectorFloat&  set1,
                         const VectorFloat&  set2
                        )
{
  // Forula was taken from Mathworld
  // http://mathworld.wolfram.com/Pairedt-Test.html

  //ofstream  out ("T-Test_Stats.txt", ios_base::app);
  double  numOfPairs = (double)set1.size ();

  if  (numOfPairs != set2.size ())
  {
    cerr << std::endl
         << std::endl
         << "PairedTTest     *** ERROR ***    The two sets are not the same length." << std::endl
         << std::endl
         << "                set1.size [" << set1.size () << "]   set2.size [" << set2.size () << "]" << std::endl
         << std::endl;
    osWaitForEnter ();
    exit (-1);
  }
                    

  double  xTotal = 0.0f;
  double yTotal = 0.0f;

  int32  foldNum = 0;


  //out << "X";
  //for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  //  out << "\t" << set1[foldNum];
  //out << endl;

  //out << "Y";
  //for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  //  out << "\t" << set2[foldNum];
  //out << endl;


  for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  {
    xTotal += set1[foldNum];
    yTotal += set2[foldNum];
  }

  double  xMean = xTotal / numOfPairs;
  double  yMean = yTotal / numOfPairs;

  //out  << endl
  //     << "X_Mean = " << xMean << endl
  //     << "Y_Mean = " << yMean << endl;

  double  totalDeltaSquared = 0.0f;

  for  (foldNum = 0;  foldNum < numOfPairs;  foldNum++)
  {
    double  xDelta = set1[foldNum] - xMean;
    double  yDelta = set2[foldNum] - yMean;

    double  deltaDelta = xDelta - yDelta;
    totalDeltaSquared += (deltaDelta * deltaDelta);
  }

  //out  << endl 
  //     << "totalDeltaSquared = " 
  //     << totalDeltaSquared << endl;


  double  tValue = 0.0f;
  if  (totalDeltaSquared != 0.0)
    tValue = fabs ((xMean - yMean) * sqrt ((numOfPairs * (numOfPairs - 1.0)) / totalDeltaSquared));

  //out  << endl 
  //     << "tValue = " << tValue << endl
  //     << endl;

  //out.close ();

  return  float (tValue);
}  /* PairedTTest */




float  KKU::McNemarsTest (int32        size,
                          const bool*  classedCorrectly1,
                          const bool*  classedCorrectly2
                         )
{
  int32  n00 = 0;
  int32  n01 = 0;
  int32  n10 = 0;
  int32  n11 = 0;

  int32  x;

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



float   KKU::LLoydsIndexOfPatchiness (const VectorInt& bins)
{
  // As defined by andrew Remsen 
  // also look at http://www.pmel.noaa.gov/pubs/outstand/stab1646/statistics.shtml

  if  (bins.size () < 1)
    return  0.0f;

  KKU::uint32  x;
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

  double  var = totalDeltaSquared / bins.size ();


  double  varDivMean = var / mean;
  double  oneDivMean = 1.0 / mean;

  float p =  float ((varDivMean - 1.0 + mean) * oneDivMean);

  return  p;
}  /* LLoydsIndexOfPatchiness */





