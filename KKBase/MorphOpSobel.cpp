/* MorphOpSobel.cpp -- Performs MorphOpSobel Edge Detection on a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"

#include  <math.h>
#include  <vector>
#include  <iostream>

#include  "MemoryDebug.h"
#include  "KKBaseTypes.h"

using namespace std;
using namespace KKB;

#include "MorphOpSobel.h"


#include "Raster.h"


MorphOpSobel::MorphOpSobel ():
    magnitudeSqrTable (NULL),
    maxMagnitude      (0)
{
}

MorphOpSobel::~MorphOpSobel(void)
{
  DeleteMagnitudeSqrTable ();
}

void  MorphOpSobel::DeleteMagnitudeSqrTable () 
{
  if  (magnitudeSqrTable)
  {
    for  (kkuint32  r = 0;  r < magnitudeSqrTableHeight;  ++r)
      delete  magnitudeSqrTable[r];
    delete  magnitudeSqrTable;
    magnitudeSqrTable = NULL;
  }
  magnitudeSqrTableHeight = 0;
  magnitudeSqrTableWidth = 0;
}

void  MorphOpSobel::AllocateMagnitudeSqrTable ()
{
  DeleteMagnitudeSqrTable ();

  kkuint32  r = 0;

  magnitudeSqrTableHeight = srcHeight;
  magnitudeSqrTableWidth  = srcWidth;

  magnitudeSqrTable = new kkint32*[magnitudeSqrTableHeight];

  for  (r = 0;  r < magnitudeSqrTableHeight;  ++r)
    magnitudeSqrTable[r] = new kkint32[magnitudeSqrTableWidth];
}

RasterPtr  MorphOpSobel::PerformOperation (Raster const* _image)
{
  if  (!_image)
    return NULL;

  bool  weOwnRaster = false;
  if  (_image->Color ())
  {
    _image = _image->CreateGrayScaleKLT ();
    weOwnRaster = true;
  }

  SetSrcRaster (_image);

  BuildMagnitudeSqrTable ();

  if  (weOwnRaster)
    delete  _image;
  _image = NULL;

  return  BuildMagnitudeImage();
}  /* PerformOperation */

void  MorphOpSobel::BuildMagnitudeSqrTable ()
{
  if  ((srcHeight != magnitudeSqrTableHeight)  ||  (srcWidth != magnitudeSqrTableWidth))
    AllocateMagnitudeSqrTable ();

  uchar*  lastRow = NULL;
  uchar*  thisRow = NULL;
  uchar*  nextRow = NULL;

  kkint32  magnitude;
  
  kkint32  r, c;
  kkint32  v, h;

  maxMagnitude = 0;

  for  (r = 1;  r < (srcHeight - 1);  ++r)
  {
    lastRow = srcGreen[r - 1];
    thisRow = srcGreen[r];
    nextRow = srcGreen[r + 1];

    for  (c = 1;  c < (srcWidth - 1);  c++)
    {
      v = (-1 * lastRow[c - 1]) + (-2 * lastRow[c]) + (-1 * lastRow[c + 1]) +
          ( 1 * nextRow[c - 1]) + ( 2 * nextRow[c]) + ( 1 * nextRow[c + 1]);

      h = (-1 * lastRow[c - 1]) + (1 * lastRow[c + 1]) +
          (-2 * thisRow[c - 1]) + (2 * thisRow[c + 1]) +
          (-1 * nextRow[c - 1]) + (1 * nextRow[c + 1]);

      magnitude = (v * v + h * h);
      if  (magnitude > maxMagnitude)
        maxMagnitude = magnitude;

      magnitudeSqrTable[r][c] = magnitude;
    }
  }
}  /* BuildMagnitudeTable */


RasterPtr  MorphOpSobel::BuildMagnitudeImage () const
{
  kkint32  r = 0, c = 0;

  RasterPtr  magImage = new Raster (srcHeight, srcWidth, false);

  for  (r = 0;  r < srcHeight;  ++r)
  {
    magImage->SetPixelValue (r, 0, 0);
    magImage->SetPixelValue (r, srcWidth - 1, 0);
  }

  for  (c = 0;  c < srcWidth;  ++c)
  {
    magImage->SetPixelValue (0, c, 0);
    magImage->SetPixelValue (srcHeight - 1, c, 0);
  }

  float  adjMag = 0.0f;

  float  maxMagnitudeFloat = sqrt ((float)maxMagnitude);

  for  (r = 1;  r < (srcHeight - 1);  ++r)
  {
    for  (c = 1;  c < (srcWidth - 1);  ++c)
    {
       if  (maxMagnitude > 0.0f)
         adjMag = 255.0f * sqrt ((float)magnitudeSqrTable[r][c]) / maxMagnitudeFloat;
       else
         adjMag = 0.0f;
        
       float  pixelWorkVal = adjMag + (float)0.5;
       if  (pixelWorkVal > 255.0f)
         pixelWorkVal = 255.0f;

       uchar  pixelVal = (uchar)pixelWorkVal;
       magImage->SetPixelValue (r, c, pixelVal);
    }
  }

  return  magImage;
}  /* BuildMagnitudeImage */


