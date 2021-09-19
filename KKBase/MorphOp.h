/* MorphOp.h -- Works with Raster class to track individual connected component in Raster.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _MORPHOP_
#define  _MORPHOP_
/**
 *@class KKB::MorphOp
 *@brief Meant to be the base class to all Morphological operators.
 *
 *@details 
 * 
 *@see KKB::Raster
 */



#include "KKQueue.h"
#include "KKStr.h"
#include "RunLog.h"


#if  !defined(_Raster_Defined_)
namespace  KKB
{
  class  Raster;
  typedef  Raster*        RasterPtr;
  typedef  Raster const*  RasterConstPtr;
}

#endif

namespace  KKB
{
  /**  
   *@brief  Base class for all Morphological operations.
   *@details
   *  It is assumed that all morphologocal operations will be working with a source image and returning a new
   * modified image; the atcual operation is to be perfomed by the "PerformOperation" metho where you pass in
   * a poiter to the source Image/ Raster. The the derived class would then call "SetSrcRaster" to intialize
   * the base class "MorphOp" with pointers to the source image.
   */
  class  MorphOp
  {
  public:
    enum  class  OperationType
    {
      Null,
      Binarize,
      BmiFiltering,
      ConvexHull,
      Dilation,
      Erosion,
      MaskExclude,
      ReduceByFactor,
      ReduceByEvenMultiple,
      SobelEdgeDetection,
      Stretcher,
      ThinContour
    };

    KKB::KKStr     OperationTypeToStr   (OperationType      _operation);
    OperationType  OperationTypeFromStr (const KKB::KKStr&  _operationStr);


    enum class  StructureType: int
    {
       Null,
       stCross,
       stSquare
    };

    
    enum class  MaskTypes: int
    {
      CROSS3   = 0,
      CROSS5   = 1,
      SQUARE3  = 2,
      SQUARE5  = 3,
      SQUARE7  = 4,
      SQUARE9  = 5,
      SQUARE11 = 6
    };

    MorphOp ();

    virtual  ~MorphOp ();

    virtual  size_t  MemoryConsumedEstimated ()  const;

    virtual  OperationType   Operation ()  const  = 0;

    virtual  RasterPtr  PerformOperation (Raster const* _image) = 0;

    static  kkint32        Biases     (MaskTypes  mt);
    static  StructureType  MaskShapes (MaskTypes  mt);

  protected:
    bool  BackgroundPixel (uchar  pixel)  const;

    bool  BackgroundPixel (kkint32  row,
                           kkint32  col
                          )  const;  

    bool  ForegroundPixel (uchar  pixel)  const;

    bool  ForegroundPixel (kkint32  row,
                           kkint32  col
                          )  const;

    void  SetSrcRaster (RasterConstPtr  _srcRaster);

    uchar            backgroundPixelTH;
    uchar            backgroundPixelValue;

    RasterConstPtr   srcRaster;

    uchar const*     srcRedArea;
    uchar const*     srcGreenArea;
    uchar const*     srcBlueArea;

    uchar* const*    srcRed;
    uchar* const*    srcGreen;
    uchar* const*    srcBlue;

    bool             srcColor;
    kkint32          srcHeight;
    kkint32          srcWidth;

    static  kkint32        biases[];
    static  StructureType  maskShapes[];

  private:
  };  /* MorphOp */

#define  _MorphOp_Defined_

  typedef  MorphOp*  MorphOpPtr;

  typedef  KKQueue<MorphOp>  MorphologicalOperatorList;

  typedef  MorphologicalOperatorList*  MorphologicalOperatorListPtr;
} /* namespace KKB; */

#endif
