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

#include  "KKQueue.h"
#include  "KKStr.h"
#include  "RunLog.h"


#if  !defined(_RASTER_)
namespace  KKB
{
  class  Raster;
  typedef  Raster*        RasterPtr;
  typedef  Raster const*  RasterConstPtr;
}

#endif

namespace  KKB
{
  class  MorphOp
  {
  public:
    typedef  enum
    {
      moNULL,
      moBinarize,
      moConvexHull,
      moErosion,
      moMaskExclude,
      moStretcher
    }  OperationType;

    KKB::KKStr       OperationTypeToStr   (OperationType      _operation);
    OperationType    OperationTypeFromStr (const KKB::KKStr&  _operationStr);


    typedef  enum
    {
       stNULL,
       stCross,
       stSquare
    }  
      StructureType;

    typedef  enum  
    {
      CROSS3   = 0,
      CROSS5   = 1,
      SQUARE3  = 2,
      SQUARE5  = 3,
      SQUARE7  = 4,
      SQUARE9  = 5,
      SQUARE11 = 6
    }  MaskTypes;


    MorphOp ();

    virtual  ~MorphOp ();

    virtual  OperationType   Operation ()  const  = 0;

    virtual  RasterPtr  PerformOperation (Raster const* _image) = 0;

  protected:

    bool  BackgroundPixel (uchar  pixel)  const;


    bool  BackgroundPixel (int32  row,
                           int32  col
                         )  const;  

    /**
     *@brief Returns true if all the pixels covered by the specified mask are Foreground pixels.
     *@see  Erosion, Dialation, Closing, Opening, MaskType
     */
    bool  Fit (MaskTypes  mask,
               int32      row, 
               int32      col
              )  const;

    bool  ForegroundPixel (uchar  pixel)  const;

    bool  ForegroundPixel (int32  row,
                           int32  col
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
    int32            srcHeight;
    int32            srcWidth;

    static  int32  biases[];
    static  int32  maskShapes[];

  private:
  };  /* MorphOp */


  typedef  MorphOp*  MorphOpPtr;


  typedef  KKQueue<MorphOp>  MorphologicalOperatorList;


  typedef  MorphologicalOperatorList*  MorphologicalOperatorListPtr;

} /* namespace KKB; */

#endif
