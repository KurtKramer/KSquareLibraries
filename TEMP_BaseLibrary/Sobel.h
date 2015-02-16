/* Sobel.h -- Performs Sobel Edge Detection on a Raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _SOBEL_
#define  _SOBEL_

namespace KKU
{
  #ifndef  _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  class Sobel
  {
  public:
    Sobel (const RasterPtr  _raster);
    ~Sobel(void);

    RasterPtr  AngleImage     () const;
    RasterPtr  MagnitudeImage () const;

  private:
    void  BuildAngleTable ();

    float**    edgeAngles;

    int32**    edgeMagnitudes;   /**< Will be stored as square,  to save the time of computing floating point operations. */

    int32      height;
    int32      maxMagnitude;     /**< Like edgeMagnitudes will be stored as a Square */
    RasterPtr  raster;
    int32      width;
  };  /* Sobel */

} /* namespace KKU; */

#endif
