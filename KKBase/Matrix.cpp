/* Matrix.cpp -- A simple two dimensional floating point matrix.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#if  defined(USE_MKL)
#include "mkl.h"
#endif

#include "Matrix.h"

#include "KKException.h"
#include "OSservices.h"
#include "KKStr.h"
using namespace KKB;



void  KKB::MultiplyMatrix (const Matrix <float> & a,  const Matrix<float>& b,  Matrix<float>& c)
{
  kkuint32 aNumRows = a.NumOfRows ();
  kkuint32 aNumCols = a.NumOfCols ();
  kkuint32 bNumRows = b.NumOfRows ();
  kkuint32 bNumCols = b.NumOfCols ();

  if (aNumCols != bNumRows)
  {
    KKStr  msg (160U);
    msg << "Matrix::operator*   **** ERROR ****,  Dimension Mismatch  Left[" << aNumRows << "," << aNumCols << "]  Right[" << bNumRows << "," << bNumCols << "].";
    std::cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

#if  defined(USE_MKL)
  cblas_sgemm (
    CBLAS_LAYOUT::CblasRowMajor,
    CBLAS_TRANSPOSE::CblasNoTrans,
    CBLAS_TRANSPOSE::CblasNoTrans,
    aNumRows,
    bNumCols,
    aNumCols,
    1.0,     // alpha
    a.DataAreaConst (),
    aNumCols,   // lda
    b.DataAreaConst (),
    1,       // incX
    0.0,     // beta
    c.DataArea (),
    1  // incY
  );

#else

  float**  resultData = c.DataNotConst ();
  float const * const *  leftData = a.Data ();
  float const * const *  rightData = b.Data ();

  for (kkuint32 row = 0; row < aNumRows; ++row)
  {
    for (kkuint32 col = 0; col < aNumCols; ++col)
    {
      float  val = 0.0;
      for (kkuint32 x = 0; x < bNumCols; ++x)
        val = val + leftData[row][x] * rightData[x][col];
      resultData[row][col] = val;
    }
  }
#endif

  return;
}  /* Matrix::operator */



void  KKB::MultiplyMatrix (const Matrix <double> & a, const Matrix<double>& b, Matrix<double>& c)
{
  kkuint32 aNumRows = a.NumOfRows ();
  kkuint32 aNumCols = a.NumOfCols ();
  kkuint32 bNumRows = b.NumOfRows ();
  kkuint32 bNumCols = b.NumOfCols ();

  if (aNumCols != bNumRows)
  {
    KKStr  msg (160U);
    msg << "Matrix::operator*   **** ERROR ****,  Dimension Mismatch  Left[" << aNumRows << "," << aNumCols << "]  Right[" << bNumRows << "," << bNumCols << "].";
    std::cerr << std::endl << msg << std::endl << std::endl;
    throw  KKException (msg);
  }

#if  defined(USE_MKL)
  cblas_dgemm (
    CBLAS_LAYOUT::CblasRowMajor,
    CBLAS_TRANSPOSE::CblasNoTrans,
    CBLAS_TRANSPOSE::CblasNoTrans,
    aNumRows,
    bNumCols,
    aNumCols,
    1.0,     // alpha
    a.DataAreaConst (),
    aNumCols,  
    b.DataAreaConst (),
    bNumCols,
    0.0,       // beta
    c.DataArea (),
    bNumCols 
  );

#else

  double**  resultData = c.DataNotConst ();
  double const * const *  leftData = a.Data ();
  double const * const *  rightData = b.Data ();

  for (kkuint32 row = 0; row < aNumRows; ++row)
  {
    for (kkuint32 col = 0; col < aNumCols; ++col)
    {
      double  val = 0.0;
      for (kkuint32 x = 0; x < aNumCols; ++x)
        val = val + leftData[row][x] * rightData[x][col];
      resultData[row][col] = val;
    }
  }
#endif

  return;
}  /* Matrix::operator */
