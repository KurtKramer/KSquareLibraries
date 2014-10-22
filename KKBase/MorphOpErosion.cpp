/* MorphOpErosion.cpp -- Morphological operators use to perform erosion.
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

#include "MorphOp.h"
#include "MorphOpErosion.h"
#include "Raster.h"
using namespace KKB;



MorphOpErosion::MorphOpErosion (StructureType  _structure,
                                kkuint16       _structureSize
                               ):
  MorphOp (),
  structure      (_structure),
  structureSize  (_structureSize)
{
}

    
    
MorphOpErosion::~MorphOpErosion ()
{
}



kkint32  MorphOpErosion::MemoryConsumedEstimated ()
{
  return  sizeof (*this);
}



bool  MorphOpErosion::Fit (kkint32  row,
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

  if  (structure == stSquare)
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



RasterPtr   MorphOpErosion::PerformOperation (RasterConstPtr  _image)
{
  this->SetSrcRaster (_image);

  kkint32  r = 0;
  kkint32  c = 0;

  kkint32  erodedForegroundPixelCount = srcRaster->ForegroundPixelCount ();

  RasterPtr   erodedRaster = new Raster (*srcRaster);

  uchar*      srcRow    = NULL;
  uchar**     destGreen  = erodedRaster->Green ();
  uchar*      destRow    = NULL;

  uchar       backgroundPixelValue = srcRaster->BackgroundPixelTH ();

  for  (r = 0;  r < srcHeight;  r++)
  {
    destRow = destGreen[r];
    srcRow  = srcGreen[r];

    for  (c = 0; c < srcWidth; c++)
    {
      if  (ForegroundPixel (srcRow[c]))
      {
        if  (!Fit (r, c))
        {
          destRow[c] = backgroundPixelValue;
          erodedForegroundPixelCount--;
        }
      }
    }  /* for (c) */
  }  /* for (r) */

  erodedRaster->ForegroundPixelCount (erodedForegroundPixelCount);

  return  erodedRaster;
}  /* PerformOperation */




