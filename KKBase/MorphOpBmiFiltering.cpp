/* MorphOpBmiFiltering.cpp -- Morphological operator used to Binarize image.
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
#include "MorphOpBmiFiltering.h"
#include "MorphOpMaskExclude.h"
#include "Raster.h"
using namespace KKB;



MorphOpBmiFiltering::MorphOpBmiFiltering ():
  MorphOp ()
{
}

    
    
MorphOpBmiFiltering::~MorphOpBmiFiltering ()
{
}



kkint32  MorphOpBmiFiltering::MemoryConsumedEstimated ()  const
{
  return  sizeof (*this);
}




RasterPtr   MorphOpBmiFiltering::ProcessBmi (uchar  minBackgroundTH)
{
  RasterPtr  erodedImage = NULL;
  
  RasterPtr  connectedImage = new Raster (*srcRaster);
  connectedImage->ConnectedComponent (2);

  erodedImage = connectedImage->BinarizeByThreshold (minBackgroundTH, 255);
  erodedImage->Opening (SQUARE3);
  uchar*  xxx = erodedImage->GreenArea ();
  uchar*  yyy = srcRaster->GreenArea ();
  kkint32  totPixels = srcRaster->TotPixels ();
  uchar  backgroundTH = erodedImage->BackgroundPixelTH ();
  for  (kkint32 x = 0;  x < totPixels;  ++x)
  {
    if  (xxx[x] > backgroundTH)
      xxx[x] = yyy[x];
  }

  eigenRatio       = 0.0f;
  orientationAngle = 0.0f;
  firstQtrWeight   = 0;
  forthQtrWeight   = 0;
  beltWidth        = 0;
  boundingLength   = 0;
  boundingWidth    = 0;
  lengthVsWidth    = 0.0f;
  headTailRatio    = 0.0f;
  estimatedBmi     = 0.0f;

  erodedImage->CalcOrientationAndEigerRatio (eigenRatio,
                                             orientationAngle
                                            );
  if  ((orientationAngle > TwoPie)  ||  (orientationAngle < -TwoPie))
  {
    orientationAngle = 0.0;
    eigenRatio = 1.0;
  }
  
  RasterPtr  rotatedImage = erodedImage->Rotate (orientationAngle);

  kkint32 tlRow;
  kkint32 tlCol;
  kkint32 brRow;
  kkint32 brCol;

  rotatedImage->FindBoundingBox (tlRow, tlCol, brRow, brCol);
  if  (tlRow < 0)
  {
    headTailRatio = 9999.99f;  
  }
  else
  {
    firstQtrWeight = 0;
    forthQtrWeight = 0;

    boundingLength = brCol - tlCol + 1;
    boundingWidth  = brRow - tlRow + 1;

    lengthVsWidth = (float)boundingWidth / (float)boundingLength;  // Prior 2 Format 18
    
    kkint32 firstQtr = tlCol +  (boundingLength / 4);
    kkint32 thirdQtr = tlCol + ((boundingLength * 3) / 4);

    kkint32 row;
    kkint32 col;
    
    uchar** grey = rotatedImage->Green ();

    for  (row = tlRow;  row <= brRow;  row++)
    {
      for (col = tlCol;  col <= firstQtr;  col++)
      {
        if  (grey[row][col] > 0)
        {
          firstQtrWeight++;
        }
      }

      for (col = thirdQtr;  col <= brCol;  col++)
      {
        if  (grey[row][col] > 0)
        {
          forthQtrWeight++;
        }
      }
    }

    beltWidth = 0;
    for (col = tlCol;  col <= brCol;  col++)
    {
      kkint32 rowFirst = -1;
      kkint32 rowLast  = 0;

      for  (row = tlRow;  row <= brRow;  ++row)
      {
        if  (grey[row][col] > 0)
        {
          rowLast = row;
          if  (rowFirst < 0)
            rowFirst = row;
        }
      }

      kkint32 colBeltWidth = 1+ (rowLast - rowFirst);
      if  (colBeltWidth > beltWidth)
        beltWidth = colBeltWidth;
    }

    grey = NULL;

    if  ((firstQtrWeight < 1)  || (forthQtrWeight < 1))
    {
      // This should not happen;  but if it does will assign '0' to EigenHead feature.
      headTailRatio = 0.0f;
    }
    else
    {
      if  (firstQtrWeight > forthQtrWeight)
      {
        headTailRatio = (float)firstQtrWeight / (float)forthQtrWeight;
      }
      else
      {
        headTailRatio = (float)forthQtrWeight / (float)firstQtrWeight;
      }
    }
  }

  //float  area = (float)rotatedImage->CalcArea ();
  //float  r = sqrt (area / (float)PIE);
  //float  estVolume = (4.0f / 3.0f) * (float)PIE * r * r * r;

  float  estVolume = (float)rotatedImage->CalcArea ();
  float  boundingLengthSquare = (float)(boundingLength * boundingLength);
  if  (boundingLengthSquare > 0.0f)
    estimatedBmi = estVolume / (boundingLength * boundingLength);
  else
    estimatedBmi = 0.0f;

  delete  erodedImage;     erodedImage    = NULL;
  delete  connectedImage;  connectedImage = NULL;

  return  rotatedImage;
}  /* ProcessBmi */




RasterPtr   MorphOpBmiFiltering::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);

  RasterPtr  rotatedImage = NULL;

  // 1) Remove Apendages.
  // 2) Perform Length vs Width

  uchar  binarizeTHmin = srcRaster->BackgroundPixelTH () + 1;
  uchar  binarizeTHmax = 125;

  rotatedImage = ProcessBmi (binarizeTHmax);
  kkint32    rectangularAreaMax = boundingLength * boundingWidth;
  delete  rotatedImage;  rotatedImage = NULL;

  rotatedImage = ProcessBmi (binarizeTHmin);
  kkint32    rectangularAreaMin = boundingLength * boundingWidth;

  if  ((rectangularAreaMin > 0)  &&  (rectangularAreaMax > 0))
  {
    if  ((rectangularAreaMax / rectangularAreaMin) > 0.90)
    {
      // Not a significant amount of low intensity forround pixels.
      delete  rotatedImage;
      rotatedImage = ProcessBmi (binarizeTHmin);
      return  rotatedImage;
    }
  }

  kkint32  rectangularAreaTH = (kkint32)(0.5f + (float)(rectangularAreaMax + rectangularAreaMin) * (1.0f / 2.0f));

  uchar  binarizeTHminLast = binarizeTHmin;
  while  (binarizeTHmin < binarizeTHmax)
  {
    if  (boundingWidth < 5)
      break;

    delete  rotatedImage;
    rotatedImage = ProcessBmi (binarizeTHmin);
    rectangularAreaMin = boundingLength * boundingWidth;
    if  (boundingWidth < 3)
    {
      binarizeTHmin = binarizeTHminLast;
      delete  rotatedImage;
      rotatedImage = ProcessBmi (binarizeTHmin);
      break;
    }

    binarizeTHminLast = binarizeTHmin;

    if  (rectangularAreaMin < rectangularAreaTH)
      break;

    binarizeTHmin += 5;
  }


  return  rotatedImage;
}  /* PerformOperation */




