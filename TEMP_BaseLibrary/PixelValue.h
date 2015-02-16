/* PixelValue.h -- Class that represents one pixel;  works in conjunction with the Raster class.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef _PIXELVALUE_
#define _PIXELVALUE_

#include  "BasicTypes.h"
#include  "Str.h"


namespace KKU
{
  /**
   *@class  PixelValue  PixelValue.h
   *@brief  Used by the Raster Class to represent the contents of one pixel.  
   *@details  There are several predefined colors to pick from or you can 
   *          work with the individual color channels separately.
   *@see Raster
   */
  class  PixelValue
  {
  public:

    /** @brief  Constructs a 'PixelValue' instance from the the three provided values. */
    PixelValue  ();

    /** @brief Constructs a 'PixelValue' instance using the provided values for the color components. */
    PixelValue  (uchar _r, uchar _g,  uchar _b);

    PixelValue  (const PixelValue&  pixelValue);

    static  PixelValue  Aqua;
    static  PixelValue  Black;
    static  PixelValue  Blue;
    static  PixelValue  Brown;
    static  PixelValue  Cyan;
    static  PixelValue  FireBrick;
    static  PixelValue  Green;
    static  PixelValue  Indigo;
    static  PixelValue  Magenta;
    static  PixelValue  Orange;
    static  PixelValue  Pink;
    static  PixelValue  Purple;
    static  PixelValue  Red;
    static  PixelValue  Teal;
    static  PixelValue  Violet;
    static  PixelValue  White;
    static  PixelValue  Yellow;
  
    uchar  r;
    uchar  g;
    uchar  b;

    /**
     *@brief  Will create an instance of PixelValue from the HSI values provided (HSI -> RGB).
     *@details Used "http://www.codeguru.com/forum/archive/index.php/t-134892.html"  for inspiration.
     *@param[in] hue Angle between 0 and (2 x Pi).
     *@param[in] sat Saturation of color,  a number between 0.0 and 1.0.
     *@param[in] intensity Intensity.
     */
    static  
    PixelValue  FromHSI (float  hue,
                         float  sat,
                         float  intensity
                        );

    /**
     *@brief  Computes the equivalent HSI values;  RGB -> HSI.
     *@details Used "http://www.codeguru.com/forum/archive/index.php/t-134892.html"  for inspiration.
     *@param[out]  hue  Angle in radians.
     *@param[out]  sat  Saturation between 0.0 and 1.0.
     *@param[out]  intensity  Intensity between 0.0 and 1.0.
     */
    void  ToHSI (float&  hue, 
                 float&  sat,
                 float&  intensity
                )  const;

    /** @brief  Creates a displayable string reflecting the values of the three RGB components. */
    KKStr  ToStr ()  const;

    PixelValue&  operator=  (const PixelValue& right);

    PixelValue   operator*  (double  fact)  const;

    bool         operator== (const PixelValue& right)  const;
    bool         operator!= (const PixelValue& right)  const;
  };
}  /* KKU */

#endif
