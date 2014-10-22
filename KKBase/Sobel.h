/* Sobel.h -- Performs Sobel Edge Detection on a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _SOBEL_
#define  _SOBEL_

namespace KKB
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

    kkint32**    edgeMagnitudes;   /**< Will be stored as square,  to save the time of computing floating point operations. */

    kkint32    height;
    kkint32    maxMagnitude;     /**< Like edgeMagnitudes will be stored as a Square */
    RasterPtr  raster;
    kkint32    width;
  };  /* Sobel */

} /* namespace KKB; */

#endif
