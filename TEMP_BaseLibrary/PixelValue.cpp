/* PixelValue.cpp -- Class that represents one pixel;  works in conjunction with the Raster class.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include <memory>
#include <math.h>
#include <limits.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include  "PixelValue.h"
using namespace KKU;



//****************************************
//*        Predefined Pixel Colors       *
//****************************************
PixelValue  PixelValue::Aqua      (  0, 255, 255);
PixelValue  PixelValue::Black     (  0,   0,   0);
PixelValue  PixelValue::Blue      (  0,   0, 255);
PixelValue  PixelValue::Brown     (165,  42,  42);   // #A52A2A 
PixelValue  PixelValue::Cyan      (  0, 255, 255);
PixelValue  PixelValue::FireBrick (176,  34,  34);   // #B22222 
PixelValue  PixelValue::Green     (  0, 255,   0);
PixelValue  PixelValue::Indigo    ( 75,   0, 130);   // #4B0082 
PixelValue  PixelValue::Magenta   (255,   0, 255);
PixelValue  PixelValue::Orange    (255, 165,   0);   // #FFA500
PixelValue  PixelValue::Pink      (255, 105, 180);   // #FF69B4 
PixelValue  PixelValue::Purple    (128,   0, 128);
PixelValue  PixelValue::Red       (255,   0,   0);
PixelValue  PixelValue::Teal      (  0, 128, 128);   // #008080
PixelValue  PixelValue::Violet    (238, 130, 238);   // #EE82EE 
PixelValue  PixelValue::White     (255, 255, 255);
PixelValue  PixelValue::Yellow    (255, 255,   0);



PixelValue::PixelValue ():
  r (0),
  g (0),
  b (0)
{
}


PixelValue::PixelValue  (const PixelValue&  pixelValue):
    r(pixelValue.r), 
    g(pixelValue.g), 
    b(pixelValue.b)  
{
}


PixelValue::PixelValue  (uchar _r, uchar _g,  uchar _b): 
    r(_r), g(_g), b(_b)  
{
}


PixelValue&  PixelValue::operator=  (const PixelValue& right)  
{
  r = right.r;
  g = right.g;
  b = right.b;
  return *this;
}


/**
 *  Will multiply the individual color channels by the right operand and return the resultant new PixelValue.
 */
PixelValue  PixelValue::operator*  (double  fact)  const
{
  uchar newR = (uchar)((double)r * fact + 0.5);
  uchar newG = (uchar)((double)g * fact + 0.5);
  uchar newB = (uchar)((double)b * fact + 0.5);
  return  PixelValue (newR, newG, newB);
}



bool  PixelValue::operator== (const PixelValue& right)  const
{
  return  ((r == right.r)  &&  (g == right.b)  &&  (b == right.b));
}



bool  PixelValue::operator!= (const PixelValue& right)  const
{
  return  ((r != right.r)  ||  (g != right.b)  ||  (b != right.b));
}



void  PixelValue::ToHSI (float&  hue, 
                         float&  sat,
                         float&  intensity
                        )  const
{
  float  redF    = (float)r / 255.0f;
  float  greenF  = (float)g / 255.0f;
  float  blueF   = (float)b / 255.0f;

  if (blueF <= greenF)
  {
    // compute hue with this formula
    hue = (float)acos ((0.5 * ((redF - greenF) + (redF - blueF))) / sqrt ((pow ((redF - greenF), 2) + (redF - blueF) * (greenF - blueF))));
  }

  // Otherwise
  else
  {
    // compute hue with this formula
    hue = (float)(2.0f * 3.1415926535897932384626433832795 - acos ((0.5 * ((redF - greenF) + (redF - blueF)))) / sqrt ((pow ((redF - greenF), 2) + (redF - blueF) * (greenF - blueF))));
  }

  // compute saturation
  sat = 1 - 3 * min (min (redF, greenF), blueF) / (redF + greenF + blueF);

  // compute intensity
  intensity = (redF + greenF + blueF) / 3;
}  /* ToHSI */



PixelValue  PixelValue::FromHSI (float  hue,
                                 float  sat,
                                 float  intensity
                                )
{
  // Used "http://www.codeguru.com/forum/archive/index.php/t-134892.html"  for inspiration.
  if  (intensity == 0.0f) 
    return PixelValue::Black;

  if  (sat == 0.0f) 
  {
    // grayscale image
    int32  greyValue = (int32)(0.5f + intensity * 255.0f);
    uchar  gv = (uchar)greyValue;
    return  PixelValue (gv, gv, gv);
  }

  float  domainOffset = 0.0f;
  float  red = 0.0, green = 0.0, blue = 0.0;
  if  (hue < (1.0f / 6.0f)) 
  {	
//domainOffset = H;
//R = I;
//B = I * (1-S);
//G = B + (I-B)*domainOffset*6;
    domainOffset = hue;
    red  = intensity;
    blue = intensity * (1.0f - sat);
    green = blue + (intensity - blue) * domainOffset * 6.0f;
  }
  
  else if  (hue < (2.0f / 6.0f)) 
  {
//domainOffset = H - 1.0/6.0;
//G = I;
//B = I * (1-S);
//R = G - (I-B)*domainOffset*6;

    // yellow domain; red acending
    domainOffset = hue - 1.0f / 6.0f;
    green = intensity;
    blue = intensity * (1.0f - sat);
    red = green - (intensity - blue) * domainOffset * 6.0f;
  }

  else if  (hue < (3.0f / 6.0f))
  {
//domainOffset = H-2.0/6;
//G = I;
//R = I * (1-S);
//B = R + (I-R)*domainOffset * 6;    // green domain; blue descending
    domainOffset = hue - (2.0f / 6.0f);
    green = intensity;
    red = intensity * (1.0f - sat);
    blue = red - (intensity - red) * domainOffset * 6.0f;
  }

  else if  (hue < (4.0f / 6.0f))
  {
//domainOffset = H - 3.0/6;
//B = I;
//R = I * (1-S);
//G = B - (I-R) * domainOffset * 6;
    // cyan domain, green acsending
    domainOffset = hue - (3.0f / 6.0f);
    blue = intensity;
    red = intensity * (1.0f - sat);
    green = blue - (intensity - red) * domainOffset * 6.0f;
  }

  else if  (hue < (5.0f / 6.0f))
  {
//domainOffset = H - 4.0/6;
//B = I;
//G = I * (1-S);
//R = G + (I-G) * domainOffset * 6;
    // blue domain, red ascending
    domainOffset = hue - (4.0f / 6.0f);
    blue = intensity;
    green = intensity * (1.0f - sat);
    red = green + (intensity - green) * domainOffset * 6.0f;
  }

  else 
  {	
//domainOffset = H - 5.0/6;
//R = I;
//G = I * (1-S);
//B = R - (I-G) * domainOffset * 6;
    // magenta domain, blue descending
    domainOffset = hue - (5.0f / 6.0f);
    red = intensity;
    green = intensity * (1.0f - sat);
    blue = red - (intensity - green) * domainOffset * 6.0f;
  }

  return  PixelValue ((uchar)(0.5f + red   * 255.0f), 
                      (uchar)(0.5f + green * 255.0f), 
                      (uchar)(0.5f + blue  * 255.0f)
                     );
}  /* FromHSI */



KKStr  PixelValue::ToStr ()  const
{
  return "(" + StrFormatInt (r, "ZZ0") + "," + StrFormatInt (g, "ZZ0") + "," + StrFormatInt (b, "ZZ0") + ")"; 
}
