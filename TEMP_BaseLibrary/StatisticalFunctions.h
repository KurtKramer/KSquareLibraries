/* StatisticalFunctions.h -- Basic Statitical Functions
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _KKU_STATISTICALFUNCTIONS_
#define  _KKU_STATISTICALFUNCTIONS_
#include  <math.h>
#include  "BasicTypes.h"

namespace  KKU
{
  template<typename T >
  void  CalcMeanAndStdDev (const vector<T>&  v,
                           T&                mean,
                           T&                stdDev
                          )
  {
    typedef long double  uld;

    if  (v.size () < 1)
    {
      mean   = T (0);
      stdDev = T (0);
      return;
    }

    KKU::uint32  idx;
    T  total = T(0);

    for  (idx = 0;  idx < v.size ();  idx++)
      total = total + v[idx];

    mean = total / T (v.size ());

    uld   deltaSqrTotal = 0.0;
    uld   deltaSquared  = 0.0;
    uld   delta         = 0.0;
    
    
    for  (idx = 0;  idx < v.size ();  idx++)
    {
      delta = (uld)fabs (mean - v[idx]);
      deltaSquared = delta * delta;
      deltaSqrTotal += deltaSquared;
    }

    uld  varience = deltaSqrTotal / (uld)(v.size ());

    stdDev = (T)sqrt (varience);

    return;
  }  /* CalcMeanAndStdDev */



  template<typename T >
  void  CalcVectorStats (const vector<T>&  v,
                         T&                total,
                         T&                mean,
                         T&                stdDev,
                         T&                min,
                         T&                max
                        )
  {
    typedef long double  uld;

    total = 0;

    if  (v.size () < 1)
    {
      mean   = T (0);
      stdDev = T (0);
      min = 0.0;
      max = 0.0;
      return;
    }

    KKU::uint32  idx;
    total = T(0);

    min = max = v[0];

    for  (idx = 0;  idx < v.size ();  idx++)
    {
      total = total + v[idx];
      if  (v[idx] < min)  min = v[idx];
      if  (v[idx] > max)  max = v[idx];
    }

    mean = total / T (v.size ());

    uld   deltaSqrTotal = 0.0;
    uld   deltaSquared  = 0.0;
    uld   delta         = 0.0;
    
    for  (idx = 0;  idx < v.size ();  idx++)
    {
      delta = (uld)fabs (mean - v[idx]);
      deltaSquared = delta * delta;
      deltaSqrTotal += deltaSquared;
    }

    uld  varience = deltaSqrTotal / (uld)(v.size ());

    stdDev = (T)sqrt (varience);

    return;
  }  /* CalcMeanAndStdDev */







  //  Quadrat-Valance Methods,  page 44
  template<typename T >
  T  CalcBQV (const vector<T>&  v,
              int32             blockSize
             )
  {
    typedef long double  uld;

    int32 zed = 0;
    int32 x   = 0;

    double  totalSquare = (T)0.0;

    double  divisorFactor = 1.0 / pow(2.0, blockSize);

    while  (zed < (int32)(v.size () - 2 * blockSize))
    {
      T  plusSide  = (T)0.0;
      T  minusSide = (T)0.0;

      double  delta = 0.0;

      for  (x = 0;  x < blockSize;  x++)
      {
        delta += v[zed];
        zed++;
      }
     
      for  (x = 0;  x < blockSize;  x++)
      {
        delta -= v[zed];
        zed++;
      }

      double  deltaSquared = delta * delta;


      totalSquare += divisorFactor * deltaSquared;
    }


    double  result  = pow (2.0, blockSize) * totalSquare / zed;

    return  (T)result;
  }  /* CalcBQV */





  //  Quadrat-Valance Methods,  page 44
  template<typename T >
  T  CalcPQV (const vector<T>&  v,
              int32             distance
             )
  {
    typedef long double  uld;

    int32 x = 0;
    int32 y = x + distance;

    double  totalDeltaSquared = 0.0;

    while  (y < (int32)v.size ())
    {
      T delta = v[x] - v[y];
      double deltaSquared = delta * delta;
      totalDeltaSquared += deltaSquared;

      x++;
      y++;
    }

    double result = totalDeltaSquared / (2 * (v.size () - distance));

    return  (T)result;
  }  /* CalcPQV */





  //  Quadrat-Valance Methods,  page 113
  template<typename T >
  T  CalcTTLQC (const vector<T>&  v,
                int32             b
               )
  {
    int32  i = 0;
    int32  n = (int32)v.size ();
    
    double  squareSun = 0.0;

    vector<T>  sumArray (v.size (), (T)0.0);
    sumArray[0] = v[0];
    for  (i = 1;  i < n;  i++)
      sumArray[i] = sumArray[i - 1] + v[i];

    int32  end = n + 1 - (2 * b);

    for  (i = 0;  i < end;  i++)
    {
      T plusSide  = sumArray[i + b - 1];
      T minusSide = sumArray[i + b + b - 1] - sumArray[i + b - 1];
      T delta = plusSide - minusSide;
      double  deltaSquared = delta * delta;
      squareSun += deltaSquared;
    }

    double  divisor = 2.0 * (double)b * ((double)n + 1.0 - 2.0 * (double)b);

    double  result = squareSun / divisor;

    return  (T)result;
  }  /* CalcTTLQC */






  //  Quadrat-Valance Methods,  page 113
  template<typename T >
  T  Calc3TTLQC (const vector<T>&  v,
                 int32             b
                )
  {
    int32  i = 0;
    int32  n = (int32)v.size ();
    
    double  squareSun = 0.0;

    vector<T>  sumArray (v.size (), (T)0.0);
    sumArray[0] = v[0];
    for  (i = 1;  i < n;  i++)
      sumArray[i] = sumArray[i - 1] + v[i];

    int32  end = n + 1 - (3 * b);

    for  (i = 0;  i < end;  i++)
    {
      T plusSide  = sumArray[i + b - 1];
      T minusSide = 2 * sumArray[i + b + b - 1];
      T plusSide2 = sumArray[i + 2 * b];

      T delta = plusSide - minusSide + plusSide2;
      double  deltaSquared = delta * delta;
      squareSun += deltaSquared;
    }

    double  divisor = 8.0 * (double)b * ((double)n + 1.0 - 3.0 * (double)b);

    double  result = squareSun / divisor;

    return  (T)result;
  }  /* Calc3TTLQC */



  // As defined by andrew Remsen 
  // also look at http://www.pmel.noaa.gov/pubs/outstand/stab1646/statistics.shtml
  float   LLoydsIndexOfPatchiness (const KKU::VectorInt& bins);





  float  McNemarsTest (int32        size,
                       const bool*  classedCorrectly1,
                       const bool*  classedCorrectly2
                      );


  float  PairedTTest (const KKU::VectorFloat&  set1,
                      const KKU::VectorFloat&  set2
                     );

}

#endif
