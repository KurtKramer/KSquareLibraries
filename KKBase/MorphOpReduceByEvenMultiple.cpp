/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
* Copyright (C) 1994-2014 Kurt Kramer
* For conditions of distribution and use, see copyright notice in KKB.h
*/

#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "MorphOp.h"
#include "MorphOpReduceByEvenMultiple.h"
#include "Raster.h"
using namespace KKB;


MorphOpReduceByEvenMultiple::MorphOpReduceByEvenMultiple (kkint32  _multiple) :
  MorphOp (),
  multiple (_multiple)
{
}



MorphOpReduceByEvenMultiple::~MorphOpReduceByEvenMultiple ()
{
}



size_t  MorphOpReduceByEvenMultiple::MemoryConsumedEstimated () const
{
  size_t  result = MorphOp::MemoryConsumedEstimated () + sizeof (*this);
  return  result;
}



RasterPtr   MorphOpReduceByEvenMultiple::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);

  // We will pad one extra pixel top, bot, left, and right.
  // This is necessary because some feature calculations assume that there edge rows are empty.

  kkint32  nHeight = scINT32 (srcHeight / multiple) + 2;
  kkint32  nWidth  = scINT32 (srcWidth  / multiple) + 2;

  kkuint32**  workRaster  = new kkuint32*[nHeight];
  uchar**     workDivisor = new uchar*[nHeight];
  kkuint32*   workRow = NULL;

  uchar*  workDivisorRow = NULL;

  for  (kkint32 nRow = 0;  nRow < nHeight;  ++nRow)
  {
    workRow = new kkuint32[nWidth];
    workRaster[nRow] = workRow;

    workDivisorRow = new uchar[nWidth];
    workDivisor[nRow] = workDivisorRow;

    for  (kkint32 nCol = 0;  nCol < nWidth;  ++nCol)
    {
      workRow[nCol] = 0;
      workDivisorRow[nCol] = 0;
    }
  }

  kkint32  intermediateRow = 0;
  kkint32  intermediateCol = 0;
  uchar*  srcRow = NULL;

  {
    kkint32 nRow = 1;
    for  (kkint32 row = 0;  row < srcHeight;  ++row)
    {
      srcRow = srcGreen[row];
      intermediateCol = 0;
      kkint32 nCol = 1;
      workRow = workRaster[nRow];
      workDivisorRow = workDivisor[nRow];

      for  (kkint32 col = 0;  col < srcWidth;  ++col)
      {
        workRow[nCol] += srcRow[col];
        workDivisorRow[nCol]++;

        intermediateCol++;
        if  (intermediateCol >= multiple)
        {
          intermediateCol = 0;
          nCol++;
        }
      }

      intermediateRow++;
      if  (intermediateRow >= multiple)
      {
        intermediateRow = 0;
        nRow++;
      }
    }
  }

  RasterPtr  reducedRaster = srcRaster->AllocateARasterInstance (nHeight, nWidth, false);

  uchar*  destRow = NULL;

  kkint32  newPixelVal = 0;
  kkint32  nMaxPixVal = 0;
  kkint32  nForegroundPixelCount = 0;

  for  (kkint32 nRow = 0;  nRow < nHeight;  ++nRow)
  {
    destRow = (reducedRaster->Green ())[nRow];
    workRow = workRaster[nRow];
    workDivisorRow = workDivisor[nRow];

    for  (kkint32 nCol = 0;  nCol < nWidth;  ++nCol)
    {
      newPixelVal = workRow[nCol];
      if  (newPixelVal > 0)
      {
        nForegroundPixelCount++;
        newPixelVal = scINT32 (0.5f + scFLOAT (newPixelVal) / scFLOAT (workDivisorRow[nCol]));
        destRow[nCol] = scUCHAR (newPixelVal);
        if  (newPixelVal > nMaxPixVal)
          nMaxPixVal = newPixelVal;
      }
    }

    delete  workRaster[nRow];
    delete  workDivisor[nRow];
    workRaster [nRow] = NULL;
    workDivisor[nRow] = NULL;
  }
  delete[]  workRaster;   workRaster  = NULL;
  delete[]  workDivisor;  workDivisor = NULL;

  reducedRaster->ForegroundPixelCount (nForegroundPixelCount);
  reducedRaster->MaxPixVal (scUCHAR (nMaxPixVal));

  return  reducedRaster;
}  /* PerformOperation */


