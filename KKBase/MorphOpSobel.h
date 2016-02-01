/* Sobel.h -- Performs Sobel Edge Detection on a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _MORPHOPSOBEL_
#define  _MORPHOPSOBEL_

#include "MorphOp.h"

namespace KKB
{
  #ifndef  _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  class MorphOpSobel: public MorphOp
  {
  public:
    MorphOpSobel ();
    ~MorphOpSobel(void);

    virtual  RasterPtr  PerformOperation (Raster const* _image);
    virtual  OperationType   Operation ()  const  {return OperationType::SobelEdgeDetection;}

  private:
    void       AllocateMagnitudeSqrTable ();
    void       DeleteMagnitudeSqrTable ();
    void       BuildMagnitudeSqrTable ();
    RasterPtr  BuildMagnitudeImage () const;

    kkint32**  magnitudeSqrTable;    /**< Will be stored as square,  to save the time of computing floating point operations. */
    kkuint32   magnitudeSqrTableHeight;
    kkuint32   magnitudeSqrTableWidth;
    kkint32    maxMagnitude;     /**< Like edgeMagnitudes will be stored as a Square */
  };  /* Sobel */

} /* namespace KKB; */

#endif
