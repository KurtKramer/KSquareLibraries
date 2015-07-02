/* MorphOpStruct.cpp -- Morphological operators use to perform erosion.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>

#include "MemoryDebug.h"
using namespace std;

#include "MorphOpStruct.h"
#include "Raster.h"
using namespace KKB;



MorphOpStruct::MorphOpStruct (StructureType  _structure,
                              kkuint16       _structureSize
                             ):
  MorphOp           (),
  backgroundCountTH (0),
  foregroundCountTH (1),
  structure         (_structure),
  structureSize     (_structureSize)
{
}

    
    
MorphOpStruct::~MorphOpStruct ()
{
}



kkint32  MorphOpStruct::MemoryConsumedEstimated ()
{
  return  sizeof (*this);
}





bool  MorphOpStruct::Fit (kkint32  row,
                          kkint32  col
                         )  const
{
  kkint32  r, c;
  kkint32  rStart = row - structureSize;
  kkint32  rEnd   = row + structureSize;
  kkint32  cStart = col - structureSize;
  kkint32  cEnd   = col + structureSize;

  if  (rStart  < 0)           rStart = 0;
  if  (rEnd    >= srcHeight)  rEnd = srcHeight - 1;
  if  (cStart  < 0)           cStart = 0;
  if  (cEnd    >= srcWidth)   cEnd = srcWidth - 1;

  if  (structure == StructureType::stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar const* rowPtr = srcGreen[r];

      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (BackgroundPixel (rowPtr[c]))
          return  false;
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (BackgroundPixel (srcGreen[r][col]))
        return  false;
    }

    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (BackgroundPixel (srcGreen[row][c]))
        return  false;
    }
  }

  return  true;
}  /* Fit */




bool  MorphOpStruct::FitBackgroundCount (kkint32  row,
                                         kkint32  col
                                        )  const
{
  kkint32  r, c;
  kkint32  rStart = row - structureSize;
  kkint32  rEnd   = row + structureSize;
  kkint32  cStart = col - structureSize;
  kkint32  cEnd   = col + structureSize;

  if  (rStart  < 0)           rStart = 0;
  if  (rEnd    >= srcHeight)  rEnd = srcHeight - 1;
  if  (cStart  < 0)           cStart = 0;
  if  (cEnd    >= srcWidth)   cEnd = srcWidth - 1;

  int  backgroundCount = 0;

  if  (structure == StructureType::stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar const* rowPtr = srcGreen[r];

      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (BackgroundPixel (rowPtr[c]))
          ++backgroundCount;
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (BackgroundPixel (srcGreen[r][col]))
        ++backgroundCount;
    }

    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (BackgroundPixel (srcGreen[row][c]))
        ++backgroundCount;
    }
  }

  return  backgroundCount < backgroundCountTH;
}  /* FitBackgroundCount */





uchar  MorphOpStruct::HitForegroundCount (kkint32  row,
                                          kkint32  col
                                         )  const
{
  kkint32  r, c;
  kkint32  rStart = row - structureSize;
  kkint32  rEnd   = row + structureSize;
  kkint32  cStart = col - structureSize;
  kkint32  cEnd   = col + structureSize;

  if  (rStart  < 0)           rStart = 0;
  if  (rEnd    >= srcHeight)  rEnd = srcHeight - 1;
  if  (cStart  < 0)           cStart = 0;
  if  (cEnd    >= srcWidth)   cEnd = srcWidth - 1;

  kkint32  pixelValueTotal = 0;
  kkint32  neighborCount = 0;

  if  (structure == StructureType::stSquare)
  {
    for  (r = rStart;  r <= rEnd;  r++)
    {
      uchar const* rowPtr = srcGreen[r];

      for  (c = cStart;  c <= cEnd;  c++)
      {
        if  (ForegroundPixel (rowPtr[c]))
        {
          ++neighborCount;
          pixelValueTotal += rowPtr[c];
        }
      }
    }
  }

  else
  {  
    for  (r = rStart;  r <= rEnd;  r++)
    {
      if  (ForegroundPixel (srcGreen[r][col]))
      {
        ++neighborCount;
        pixelValueTotal += srcGreen[r][col];
      }
    }

    for  (c = cStart;  c <= cEnd;  c++)
    {
      if  (ForegroundPixel (srcGreen[r][c]))
      {
        ++neighborCount;
        pixelValueTotal += srcGreen[r][col];
      }
    }
  }

  if  (neighborCount < foregroundCountTH)
    return (uchar)0;
  else
    return (uchar)(0.5f + (float)pixelValueTotal / (float)neighborCount);
}  /* HitForegroundCount */
