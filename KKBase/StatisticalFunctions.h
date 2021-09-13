/* StatisticalFunctions.h -- Basic Statistical Functions
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _KKU_STATISTICALFUNCTIONS_
#define  _KKU_STATISTICALFUNCTIONS_
#include  <math.h>
#include  "KKBaseTypes.h"

namespace  KKB
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

    T  total = T(0);

    for  (size_t idx = 0;  idx < v.size ();  idx++)
      total = total + v[idx];

    mean = total / T (v.size ());

    uld   deltaSqrTotal = 0.0;
    uld   deltaSquared  = 0.0;
    uld   delta         = 0.0;
     
    for  (size_t idx = 0;  idx < v.size ();  idx++)
    {
      delta = static_cast<uld> (fabs (mean - v[idx]));
      deltaSquared = delta * delta;
      deltaSqrTotal += deltaSquared;
    }

    uld  varience = deltaSqrTotal / static_cast<uld> (v.size ());

    stdDev = static_cast<T> (sqrt (varience));

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

    total = static_cast<T> (0);

    min = max = v[0];

    for  (size_t idx = 0;  idx < v.size ();  ++idx)
    {
      total = total + v[idx];
      if  (v[idx] < min)  min = v[idx];
      if  (v[idx] > max)  max = v[idx];
    }

    mean = total / static_cast<T> (v.size ());

    uld   deltaSqrTotal = 0.0;
    uld   deltaSquared  = 0.0;
    uld   delta         = 0.0;
    
    for  (size_t idx = 0;  idx < v.size ();  ++idx)
    {
      delta = static_cast<uld> (fabs (mean - v[idx]));
      deltaSquared = delta * delta;
      deltaSqrTotal += deltaSquared;
    }

    uld  varience = deltaSqrTotal / static_cast<uld> (v.size ());

    stdDev = static_cast<T> (sqrt (varience));

    return;
  }  /* CalcMeanAndStdDev */



  //  Quadrat-Valance Methods,  page 44
  template<typename T >
  T  CalcBQV (const vector<T>&  v,
              size_t            blockSize
             )
  {
    //typedef long double  uld;

    size_t zed = 0;

    double  totalSquare = static_cast<T> (0.0);

    double  divisorFactor = 1.0 / pow(2.0, blockSize);

    while  (zed < (v.size () - 2 * blockSize))
    {
      T  plusSide  = static_cast<T> (0.0);
      T  minusSide = static_cast<T> (0.0);

      double  delta = 0.0;

      for  (size_t x = 0;  x < blockSize;  x++)
      {
        delta += v[zed];
        ++zed;
      }
     
      for  (size_t x = 0;  x < blockSize;  x++)
      {
        delta -= v[zed];
        ++zed;
      }

      double  deltaSquared = delta * delta;

      totalSquare += divisorFactor * deltaSquared;
    }

    double  result  = pow (2.0, blockSize) * totalSquare / zed;

    return  static_cast<T> (result);
  }  /* CalcBQV */



  //  Quadrat-Valance Methods,  page 44
  template<typename T >
  T  CalcPQV (const vector<T>&  v,
              size_t            distance
             )
  {
    //typedef long double  uld;

    size_t x = 0;
    size_t y = x + distance;

    double  totalDeltaSquared = 0.0;

    while  (y < v.size ())
    {
      T delta = v[x] - v[y];
      double deltaSquared = delta * delta;
      totalDeltaSquared += deltaSquared;

      ++x;
      ++y;
    }

    double result = totalDeltaSquared / (2 * (v.size () - distance));

    return  static_cast<T> (result);
  }  /* CalcPQV */



  //  Quadrat-Valance Methods,  page 113
  template<typename T >
  T  CalcTTLQC (const vector<T>&  v,
                size_t            b
               )
  {
    size_t   n = v.size ();
    
    double  squareSun = 0.0;

    vector<T>  sumArray (v.size (), static_cast<T> (0.0));
    sumArray[0] = v[0];
    for  (size_t i = 1;  i < n;  ++i)
      sumArray[i] = sumArray[i - 1] + v[i];

    size_t  end = n + 1 - (2 * b);

    for  (size_t i = 0;  i < end;  ++i)
    {
      T plusSide  = sumArray[i + b - 1];
      T minusSide = sumArray[i + b + b - 1] - sumArray[i + b - 1];
      T delta = plusSide - minusSide;
      double  deltaSquared = delta * delta;
      squareSun += deltaSquared;
    }

    double  divisor = 2.0 * scDOUBLE (b) * (scDOUBLE (n) + 1.0 - 2.0 * scDOUBLE (b));

    double  result = squareSun / divisor;

    return  static_cast<T> (result);
  }  /* CalcTTLQC */



  //  Quadrat-Valance Methods,  page 113
  template<typename T >
  T  Calc3TTLQC (const vector<T>&  v,
                 size_t            b
                )
  {
    size_t  n = v.size ();
    
    double  squareSun = 0.0;

    vector<T>  sumArray (v.size (), 0.0);
    sumArray[0] = v[0];
    for  (size_t i = 1;  i < n;  i++)
      sumArray[i] = sumArray[i - 1] + v[i];

    size_t  end = n + 1 - (3 * b);

    for  (size_t i = 0;  i < end;  ++i)
    {
      T plusSide  = sumArray[i + b - 1];
      T minusSide = 2 * sumArray[i + b + b - 1];
      T plusSide2 = sumArray[i + 2 * b];

      T delta = plusSide - minusSide + plusSide2;
      double  deltaSquared = delta * delta;
      squareSun += deltaSquared;
    }

    double  divisor = 8.0 * scDOUBLE (b) * (scDOUBLE (n) + 1.0 - 3.0 * scDOUBLE (b));

    double  result = squareSun / divisor;

    return  static_cast<T> (result);
  }  /* Calc3TTLQC */



  // As defined by Andrew Remsen
  // also look at http://www.pmel.noaa.gov/pubs/outstand/stab1646/statistics.shtml
  float   LLoydsIndexOfPatchiness (const KKB::VectorInt& bins);



  float  McNemarsTest (kkint32      size,
                       const bool*  classedCorrectly1,
                       const bool*  classedCorrectly2
                      );


  float  PairedTTest (const KKB::VectorFloat&  set1,
                      const KKB::VectorFloat&  set2
                     );

}

#endif
