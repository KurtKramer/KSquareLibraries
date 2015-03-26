/* MorphOpMaskExclude.cpp -- Used to exclude specified region.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#include  "FirstIncludes.h"
#include  <stdlib.h>
#include  <iostream>
#include  <map>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;

#include "MorphOp.h"
#include "MorphOpMaskExclude.h"
#include "Raster.h"
using namespace KKB;



MorphOpMaskExclude::MorphOpMaskExclude (MaskTypes  _mask):
    MorphOp (),
    mask (_mask)
{
}

    
    
MorphOpMaskExclude::~MorphOpMaskExclude ()
{
}



kkint32  MorphOpMaskExclude::MemoryConsumedEstimated ()
{
  kkint32  result = sizeof (*this);
  return  result;
}



RasterPtr   MorphOpMaskExclude::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);

  kkint32  totalPixels = srcHeight * srcWidth;

  RasterPtr  maskImage = new Raster (*_image);

  maskImage->Opening ((KKB::MaskTypes)mask);
  //maskImage->ConnectedComponent();
  maskImage->Dilation ((KKB::MaskTypes)mask);

  RasterPtr  result = new Raster (srcHeight, srcWidth, srcColor);

  uchar const*  srcImageRedPtr   = srcRedArea;
  uchar const*  srcImageGreenPtr = srcGreenArea;
  uchar const*  srcImageBluePtr  = srcBlueArea;

  uchar*  resultRedPtr   = result->RedArea ();
  uchar*  resultGreenPtr = result->GreenArea ();
  uchar*  resultBluePtr  = result->BlueArea ();

  uchar*  maskPtr = maskImage->GreenArea ();
  for  (kkint32 x = 0;  x < totalPixels;  ++x)
  {
    if  (*maskPtr == 0)
    {
      *resultGreenPtr = *srcImageGreenPtr;
      if  (srcColor)
      {
        *resultRedPtr  = *srcImageRedPtr;
        *resultBluePtr = *srcImageBluePtr;
      }
    }
    ++maskPtr;
    ++srcImageGreenPtr;
    ++resultGreenPtr;
    if  (srcColor)
    {
      ++srcImageRedPtr;
      ++resultRedPtr;
      ++srcImageBluePtr;
      ++resultBluePtr;
    }
  }

  delete  maskImage;
  maskImage = NULL;

  return  result;
}  /* PerformOperation */

