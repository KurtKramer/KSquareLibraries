/* PixelValue.cpp -- Class that represents one pixel;  works in conjunction with the Raster class.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
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
using namespace KKB;



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
  const uchar newR = scUCHAR (scDOUBLE (r) * fact + 0.5);
  const uchar newG = scUCHAR (scDOUBLE (g) * fact + 0.5);
  const uchar newB = scUCHAR (scDOUBLE (b) * fact + 0.5);
  return  PixelValue (newR, newG, newB);
}



bool  PixelValue::operator== (const PixelValue& right)  const  noexcept
{
  return  ((r == right.r)  &&  (g == right.b)  &&  (b == right.b));
}



bool  PixelValue::operator!= (const PixelValue& right)  const  noexcept
{
  return  ((r != right.r)  ||  (g != right.b)  ||  (b != right.b));
}



void  PixelValue::ToHSI (float&  hue, 
                         float&  sat,
                         float&  intensity
                        )  const noexcept
{
  float  greenF  = scFLOAT (g) / 255.0f;
  float  blueF   = scFLOAT (b) / 255.0f;
  float  redF    = scFLOAT (r) / 255.0f;

  if (blueF <= greenF)
  {
    // compute hue with this formula
    hue = static_cast<float> (acos ((0.5f * ((redF - greenF) + (redF - blueF))) / sqrtf ((pow ((redF - greenF), 2.0f) + (redF - blueF) * (greenF - blueF)))));
  }

  // Otherwise
  else
  {
    // compute hue with this formula
    hue = static_cast<float> (2.0f * scFLOAT (PIE) - acos ((0.5f * ((redF - greenF) + (redF - blueF)))) / sqrtf ((pow ((redF - greenF), 2.0f) + (redF - blueF) * (greenF - blueF))));
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
    // gray-scale image
    const kkint32  greyValue = static_cast<kkint32> (0.5f + intensity * 255.0f);
    uchar  gv = static_cast<uchar> (greyValue);
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

    // yellow domain; red ascending
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
    // cyan domain, green ascending
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
    // Magenta domain, blue descending
    domainOffset = hue - (5.0f / 6.0f);
    red = intensity;
    green = intensity * (1.0f - sat);
    blue = red - (intensity - green) * domainOffset * 6.0f;
  }

  return  PixelValue (static_cast<uchar> (0.5f + red   * 255.0f), 
                      static_cast<uchar> (0.5f + green * 255.0f), 
                      static_cast<uchar> (0.5f + blue  * 255.0f)
                     );
}  /* FromHSI */



KKStr  PixelValue::ToStr ()  const
{
  KKStr  s = "(" + KKB::StrFormatInt (scINT32 (r), "ZZ0") + ","
    		         + KKB::StrFormatInt (scINT32 (g), "ZZ0") + ","
	               + KKB::StrFormatInt (scINT32 (b), "ZZ0") + ")";
  return s;
}
