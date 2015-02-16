/* Sobel.cpp -- Performs Sobel Edge Detection on a Raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#include  <math.h>
#include  <vector>
#include  <iostream>

#include  "MemoryDebug.h"
#include  "BasicTypes.h"

using namespace std;
using namespace KKU;

#include "Sobel.h"


#include "Raster.h"



Sobel::Sobel (const RasterPtr  _raster):
    edgeAngles     (NULL),
    edgeMagnitudes (NULL),
    height         (-1),
    maxMagnitude   (0),
    raster         (_raster),
    width          (-1)

{
  if  (!raster)
    return;

  bool  weOwnRaster = false;
  if  (raster->Color ())
  {
    raster = raster->CreateGrayScaleKLT ();
    weOwnRaster = true;
  }

  height = raster->Height ();
  width  = raster->Width  ();

  uchar*  lastRow = NULL;
  uchar*  thisRow = NULL;
  uchar*  nextRow = NULL;

  int32  magnitude;
  
  int32  r, c;
  int32  v, h;

  maxMagnitude = 0;

  edgeMagnitudes = new int32*[height];

  for  (r = 1;  r < (height - 1);  r++)
  {
    lastRow = raster->Green ()[r - 1];
    thisRow = raster->Green ()[r];
    nextRow = raster->Green ()[r + 1];

    edgeMagnitudes[r] = new int32[width];

    for  (c = 1;  c < (width - 1);  c++)
    {
      v = (-1 * lastRow[c - 1]) + (-2 * lastRow[c]) + (-1 * lastRow[c + 1]) +
          ( 1 * nextRow[c - 1]) + ( 2 * nextRow[c]) + ( 1 * nextRow[c + 1]);

      h = (-1 * lastRow[c - 1]) + (1 * lastRow[c + 1]) +
          (-2 * thisRow[c - 1]) + (2 * thisRow[c + 1]) +
          (-1 * nextRow[c - 1]) + (1 * nextRow[c + 1]);

      magnitude = (v * v + h * h);
      if  (magnitude > maxMagnitude)
        maxMagnitude = magnitude;

      edgeMagnitudes[r][c] = magnitude;
    }
  }
  if  (weOwnRaster)
  {
    delete  raster;
    raster = NULL;
  }
}



Sobel::~Sobel(void)
{
  if  (edgeMagnitudes)
  {
    for  (int32  r = 1;  r < (height - 1);  r++)
    {
      delete  edgeMagnitudes[r];
    }
    delete  edgeMagnitudes;
  }

  if  (edgeAngles)
  {
    for  (int32  r = 1;  r < (height - 1);  r++)
    {
      delete  edgeAngles    [r];
    }
    delete  edgeAngles;
  }
}


void  Sobel::BuildAngleTable ()
{
  uchar*  lastRow = NULL;
  uchar*  thisRow = NULL;
  uchar*  nextRow = NULL;

  float  angle;

  int32  r, c;
  int32  v, h;

  edgeAngles = new float*[height];

  for  (r = 1;  r < (height - 1);  r++)
  {
    lastRow = raster->Green ()[r - 1];
    thisRow = raster->Green ()[r];
    nextRow = raster->Green ()[r + 1];

    edgeAngles [r] = new float[width];

    for  (c = 1;  c < (width - 1);  c++)
    {
      v = (-1 * lastRow[c - 1]) + (-2 * lastRow[c]) + (-1 * lastRow[c + 1]) +
          ( 1 * nextRow[c - 1]) + ( 2 * nextRow[c]) + ( 1 * nextRow[c + 1]);

      h = (-1 * lastRow[c - 1]) + (1 * lastRow[c + 1]) +
          (-2 * thisRow[c - 1]) + (2 * thisRow[c + 1]) +
          (-1 * nextRow[c - 1]) + (1 * nextRow[c + 1]);

      if  (v == 0)
      {
        if  (h == 0)
          angle = 0.0f;

        else if  (v > 0)
          angle = (float)PIE / 2.0f;

        else
          angle = -(float)(PIE / 2.0);
      } 
      else
      {
        angle = atan2 ((float)v, (float)h);
        while  (angle < -PIE)
           angle = angle + (2.0f * (float)PIE);

        while  (angle > PIE)
           angle = angle - (2.0f * (float)PIE);
      }

      edgeAngles[r][c] = angle;
    }
  }
}  /* BuildAngleTable */







RasterPtr  Sobel::MagnitudeImage () const
{
  int32  r, c;

  RasterPtr  magImage = new Raster (height, width, false);

  for  (r = 0;  r < height;  r++)
  {
    magImage->SetPixelValue (r, 0, 0);
    magImage->SetPixelValue (r, width - 1, 0);
  }

  for  (c = 0;  c < width;  c++)
  {
    magImage->SetPixelValue (0, c, 0);
    magImage->SetPixelValue (height - 1, c, 0);
  }

  float  adjMag = 0.0f;

  float  maxMagnitudeFloat = sqrt ((float)maxMagnitude);

  for  (r = 1;  r < (height - 1);  r++)
  {
    for  (c = 1;  c < (width - 1);  c++)
    {
       if  (maxMagnitude > 0.0f)
         adjMag = 255.0f * sqrt ((float)edgeMagnitudes[r][c]) / maxMagnitudeFloat;
       else
         adjMag = 0.0f;
        
       float  pixelWorkVal =   adjMag + (FFLOAT)0.5;
       if  (pixelWorkVal > 255.0f)
         pixelWorkVal = 255.0f;

       uchar  pixelVal = (uchar)pixelWorkVal;
       magImage->SetPixelValue (r, c, pixelVal);
    }
  }


  return  magImage;
}  /* SobelEdgeDetector */






RasterPtr  Sobel::AngleImage () const
{
  int32  r, c;

  RasterPtr  angImage = new Raster (height, width, true);

  float  maxMagnitudeFloat = sqrt ((float)maxMagnitude);

  for  (r = 0;  r < height;  r++)
  {
    angImage->SetPixelValue (r, 0, PixelValue::Black);
    angImage->SetPixelValue (r, width - 1, PixelValue::Black);
  }

  for  (c = 0;  c < width;  c++)
  {
    angImage->SetPixelValue (0, c, PixelValue::Black);
    angImage->SetPixelValue (height - 1, c, PixelValue::Black);
  }

  float  h, s, i;
  float  red, green, blue;


  for  (r = 1;  r < (height - 1);  r++)
  {
    for  (c = 1;  c < (width - 1);  c++)
    {
      h = this->edgeAngles[r][c];
      while  (h < 0.0f)
        h += (float)TwoPie;
      
      i = s = sqrt ((float)edgeMagnitudes[r][c]) / maxMagnitudeFloat;

      if  ((s < 0.00001) || (i < 0.00001))
      {
        red = green = blue = i;          
      }   

      else if  ((h > 0.0)  &&  (h <= (float)TwoThirdsPie))
      {
        red = i * (1 + ((s * cos (h)) / cos ((float)OneThirdPie - h)));
        blue = i * (1 - s);
        green = 3 * i * (1 - (red + blue) / (3 * i));
      }

      else if  ((h > (float)TwoThirdsPie)  &&   (h <= ((float)FourThirdsPie)))
      {
        h = h - (float)TwoThirdsPie;
          
        green = i * (1 + ((s * cos (h)) / cos ((float)OneThirdPie - h)));
        red = i * (1 - s);
        blue = 3 * i * (1 - ((red + green) / (3 * i)));
      }

      else if  ((h > (float)FourThirdsPie) && (h <= (float)TwoPie))
      {
        h = h - (float)FourThirdsPie;
        blue = i * (1 + ((s * cos (h)) / cos ((float)OneThirdPie - h)));
        green = i * (1 - s);
        red = 3 * i * (1 - ((green + blue) / (3 * i)));
      }

      else
      {
        red = green = blue = 0;
      }

      angImage->SetPixelValue (r, c, PixelValue ((uchar)(red   * 255 + 0.5),
                                                 (uchar)(green * 255 + 0.5),
                                                 (uchar)(blue  * 255 + 0.5)
                                                )
                              );
    }
  }

  return  angImage;
}  /* AngleImage */
