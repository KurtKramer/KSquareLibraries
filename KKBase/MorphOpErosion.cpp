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
  MorphOpStruct (_structure, _structureSize)
{
}

    
    
MorphOpErosion::~MorphOpErosion ()
{
}



kkMemSize  MorphOpErosion::MemoryConsumedEstimated ()
{
  return  sizeof (*this);
}





RasterPtr   MorphOpErosion::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);

  kkint32  r = 0;
  kkint32  c = 0;

  kkint32  erodedForegroundPixelCount = 0;

  RasterPtr erodedRaster = new Raster (*srcRaster);

  uchar*    srcRow    = NULL;
  uchar**   destGreen = erodedRaster->Green ();
  uchar*    destRow   = NULL;

  uchar     srcBackgroundPixelValue = srcRaster->BackgroundPixelValue ();

  for  (r = 0;  r < srcHeight;  r++)
  {
    destRow = destGreen[r];
    srcRow  = srcGreen[r];

    for  (c = 0; c < srcWidth; c++)
    {
      if  (ForegroundPixel (srcRow[c]))
      {
        bool  fit = false;
        if  (backgroundCountTH > 0)
          fit = FitBackgroundCount (r, c);
        else
          fit = Fit (r, c);

        if  (!fit)
        {
          destRow[c] = srcBackgroundPixelValue;
        }
        else
        {
          ++erodedForegroundPixelCount;
        }
      }
    }  /* for (c) */
  }  /* for (r) */

  erodedRaster->ForegroundPixelCount (erodedForegroundPixelCount);

  return  erodedRaster;
}  /* PerformOperation */
