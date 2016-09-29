/* MorphOpBinarize.cpp -- Morphological operator used to Binarize image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_MORPHOPBINARIZE_)
#define  _MORPHOPBINARIZE_

#include "KKBaseTypes.h"
#include "MorphOp.h"


#if  !defined(_RASTER_)
namespace  KKB
{
  class  Raster;
  typedef  Raster*  RasterPtr;
}
#endif



namespace  KKB
{
  class  MorphOpBinarize :  public MorphOp
  {
  public:
    MorphOpBinarize (kkuint16  _pixelValueMin,
                     kkuint16  _pixelValueMax
                    );
    
    virtual ~MorphOpBinarize ();

    virtual  OperationType   Operation ()  const  {return  OperationType::Binarize;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    kkint32  MemoryConsumedEstimated ();

  private:
    bool  Fit (kkint32  row, 
               kkint32  col
              )  const;

    kkuint16  pixelValueMin;
    kkuint16  pixelValueMax;
  };

  typedef  MorphOpBinarize*  MorphOpBinarizePtr;
}  /* KKB */


#endif
