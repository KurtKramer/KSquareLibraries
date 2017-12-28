/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
* Copyright (C) 1994-2014 Kurt Kramer
* For conditions of distribution and use, see copyright notice in KKB.h
*/

#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <map>
#include <math.h>
#include <memory>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "MorphOp.h"
#include "MorphOpReduceByFactor.h"
#include "Raster.h"
using namespace KKB;


MorphOpReduceByFactor::MorphOpReduceByFactor (float _factor):
  MorphOp (),
  factor (_factor)
{
}



MorphOpReduceByFactor::~MorphOpReduceByFactor ()
{
}



kkMemSize  MorphOpReduceByFactor::MemoryConsumedEstimated ()
{
  kkMemSize  result = MorphOp::MemoryConsumedEstimated () + sizeof (*this);
  return  result;
}



RasterPtr   MorphOpReduceByFactor::PerformOperation (RasterConstPtr  _image)
{
  this->SetSrcRaster (_image);

  if (factor <= 0.0f)
    factor = 0.1f;

  else if (factor > 1.0f)
    factor = 1.0f;

  kkint32  newHeight = (kkint32)((float)(srcRaster->Height ()) * factor + 0.5f);
  kkint32  newWidth  = (kkint32)((float)(srcRaster->Width ())  * factor + 0.5f);
  if (newHeight < 2)
    newHeight = 2;

  if (newWidth < 2)
    newWidth = 2;

  kkint32  newTotal = newHeight * newWidth;

  float*  accumulatorAreaGreen = new float[newTotal];
  float*  accumulatorAreaRed = NULL;
  float*  accumulatorAreaBlue = NULL;
  if (srcColor)
  {
    accumulatorAreaRed  = new float[newTotal];
    accumulatorAreaBlue = new float[newTotal];
    std::memset (accumulatorAreaRed,  0, newTotal * sizeof (float));
    std::memset (accumulatorAreaBlue, 0, newTotal * sizeof (float));
  }

  float*  divisorArea = new float[newTotal];

  std::memset (accumulatorAreaGreen, 0, newTotal * sizeof (float));
  std::memset (divisorArea, 0, newTotal * sizeof (float));

  float** accumulatorRed = NULL;
  float** accumulatorGreen = new float*[newHeight];
  float** accumulatorBlue = NULL;

  if (srcColor)
  {
    accumulatorRed  = new float*[newHeight];
    accumulatorBlue = new float*[newHeight];
  }

  float** divisor = new float*[newHeight];

  float*  rowFactor = new float[srcRaster->Height () + 1];
  float*  colFactor = new float[srcRaster->Width  () + 1];

  for (kkint32 r = 0; r < srcRaster->Height (); ++r)
    rowFactor[r] = (float)r * factor;
  rowFactor[srcRaster->Height ()] = (float)newHeight;

  for (kkint32 c = 0; c < srcRaster->Width (); ++c)
    colFactor[c] = (float)c * factor;
  colFactor[srcRaster->Width ()] = (float)newWidth;

  float*  arPtr = accumulatorAreaRed;
  float*  agPtr = accumulatorAreaGreen;
  float*  abPtr = accumulatorAreaBlue;
  float*  daPtr = divisorArea;
  for (kkint32 newR = 0; newR < newHeight; ++newR)
  {
    accumulatorGreen[newR] = agPtr;
    divisor[newR] = daPtr;
    agPtr += newWidth;
    daPtr += newWidth;

    if (srcRaster->Color ())
    {
      accumulatorRed [newR] = arPtr;
      accumulatorBlue[newR] = abPtr;
      arPtr += newWidth;
      abPtr += newWidth;
    }
  }

  uchar  rValue = 0, gValue = 0, bValue = 0;

  for (kkint32 r = 0; r < srcHeight; ++r)
  {
    kkint32  thisRow = (kkint32)rowFactor[r];
    if (thisRow >= newHeight)
      thisRow = newHeight - 1;

    kkint32  nextRow = (kkint32)rowFactor[r + 1];
    if (nextRow >= newHeight)
      nextRow = newHeight - 1;

    float  amtThisRow = 1.0f;
    float  amtNextRow = 0.0f;

    if (nextRow > thisRow)
    {
      amtThisRow = (float)nextRow - rowFactor[r];
      amtNextRow = 1.0f - amtThisRow;
    }

    for (kkint32 c = 0; c < srcWidth; c++)
    {
      gValue = srcGreen[r][c];
      if (srcRaster->Color ())
      {
        rValue = srcRed[r][c];
        bValue = srcBlue[r][c];
      }

      kkint32  thisCol = (kkint32)colFactor[c];
      if (thisCol >= newWidth)
        thisCol = newWidth - 1;

      kkint32  nextCol = (kkint32)colFactor[c + 1];
      if (nextCol >= newWidth)
        nextCol = newWidth - 1;

      float  amtThisCol = 1.0f;
      float  amtNextCol = 0.0f;

      if (nextCol > thisCol)
      {
        amtThisCol = (float)nextCol - colFactor[c];
        amtNextCol = 1.0f - amtThisCol;
      }

      accumulatorGreen[thisRow][thisCol] += gValue * amtThisRow * amtThisCol;
      if (srcColor)
      {
        accumulatorRed [thisRow][thisCol] += rValue * amtThisRow * amtThisCol;
        accumulatorBlue[thisRow][thisCol] += bValue * amtThisRow * amtThisCol;
      }

      divisor[thisRow][thisCol] += amtThisRow * amtThisCol;

      if (nextRow > thisRow)
      {
        accumulatorGreen[nextRow][thisCol] += gValue * amtNextRow * amtThisCol;
        if (srcColor)
        {
          accumulatorRed[nextRow][thisCol] += rValue * amtNextRow * amtThisCol;
          accumulatorBlue[nextRow][thisCol] += bValue * amtNextRow * amtThisCol;
        }
        divisor[nextRow][thisCol] += amtNextRow * amtThisCol;

        if (nextCol > thisCol)
        {
          accumulatorGreen[nextRow][nextCol] += gValue * amtNextRow * amtNextCol;
          if (srcColor)
          {
            accumulatorRed[nextRow][nextCol] += rValue * amtNextRow * amtNextCol;
            accumulatorBlue[nextRow][nextCol] += bValue * amtNextRow * amtNextCol;
          }
          divisor[nextRow][nextCol] += amtNextRow * amtNextCol;
        }
      }
      else
      {
        if (nextCol > thisCol)
        {
          accumulatorGreen[thisRow][nextCol] += gValue * amtThisRow * amtNextCol;
          if (srcColor)
          {
            accumulatorRed[thisRow][nextCol] += rValue * amtThisRow * amtNextCol;
            accumulatorBlue[thisRow][nextCol] += bValue * amtThisRow * amtNextCol;
          }
          divisor[thisRow][nextCol] += amtThisRow * amtNextCol;
        }
      }
    }  /*  for (c)  */
  }  /*  for (r)  */

  kkint32  x;
  RasterPtr  reducedRaster = srcRaster->AllocateARasterInstance (newHeight, newWidth, srcColor);
  uchar*  newRedArea   = reducedRaster->RedArea ();
  uchar*  newGreenArea = reducedRaster->GreenArea ();
  uchar*  newBlueArea  = reducedRaster->BlueArea ();
  for (x = 0; x < newTotal; x++)
  {
    if (divisorArea[x] == 0.0f)
    {
      newGreenArea[x] = 0;
      if (srcColor)
      {
        newRedArea[x] = 0;
        newBlueArea[x] = 0;
      }
    }
    else
    {
      newGreenArea[x] = (uchar)(accumulatorAreaGreen[x] / divisorArea[x] + 0.5f);
      if (srcColor)
      {
        newRedArea [x] = (uchar)(accumulatorAreaRed[x]  / divisorArea[x] + 0.5f);
        newBlueArea[x] = (uchar)(accumulatorAreaBlue[x] / divisorArea[x] + 0.5f);
      }
    }
  }

  delete[]  accumulatorAreaRed;    accumulatorAreaRed = NULL;
  delete[]  accumulatorAreaGreen;  accumulatorAreaGreen = NULL;
  delete[]  accumulatorAreaBlue;   accumulatorAreaBlue = NULL;

  delete[]  divisorArea;       divisorArea = NULL;

  delete[]  accumulatorRed;    accumulatorRed = NULL;
  delete[]  accumulatorGreen;  accumulatorGreen = NULL;
  delete[]  accumulatorBlue;   accumulatorBlue = NULL;
  delete[]  divisor;           divisor = NULL;

  delete[]  rowFactor;       rowFactor = NULL;
  delete[]  colFactor;       colFactor = NULL;

  return  reducedRaster;
}  /* PerformOperation */


