/* MorphOpDilation.cpp -- Morphological operators use to perform erosion.
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
#include "MorphOpDilation.h"
#include "Raster.h"
using namespace KKB;



MorphOpDilation::MorphOpDilation (StructureType  _structure,
                                  kkuint16       _structureSize
                                 ):
  MorphOpStruct (_structure, _structureSize)
{
}

    
    
MorphOpDilation::~MorphOpDilation ()
{
}



kkMemSize  MorphOpDilation::MemoryConsumedEstimated ()
{
  return  sizeof (*this);
}




RasterPtr   MorphOpDilation::PerformOperation (RasterConstPtr  _image)
{
  this->SetSrcRaster (_image);

  kkint32  r = 0;
  kkint32  c = 0;

  RasterPtr resultRaster = new Raster (*srcRaster);

  uchar*    srcRow     = NULL;
  uchar**   destGreen  = resultRaster->Green ();
  uchar*    destRow    = NULL;

  kkint32  forgroundPixelCount = srcRaster->ForegroundPixelCount ();

  for  (r = 0;  r < srcHeight;  r++)
  {
    destRow = destGreen[r];
    srcRow  = srcGreen[r];

    for  (c = 0; c < srcWidth; c++)
    {
      if  (BackgroundPixel (srcRow[c]))
      {
        uchar  pixel = HitForegroundCount (r, c);
        if  (ForegroundPixel (pixel))
          ++forgroundPixelCount;
        destRow[c] = pixel;
      }
    }  /* for (c) */
  }  /* for (r) */

  resultRaster->ForegroundPixelCount (forgroundPixelCount);

  return  resultRaster;
}  /* PerformOperation */

