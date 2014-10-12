/* MorphOpMaskExclude.cpp -- Used to exclude specified region.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _MORPHOPMASKEXCLUDE_
#define  _MORPHOPMASKEXCLUDE_
/**
 *@class KKB::MorphOp
 *@brief Creates a image where the only pixels that are passed thru are the 
 * ones that would be removed by the specified mask when a Open-Dialation operation
 * are performed.
 * 
 *@see KKB::Raster
 */                                                                     


#include "KKBaseTypes.h"
#include "MorphOp.h"


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
  class  MorphOpMaskExclude :  public MorphOp
  {
  public:
    MorphOpMaskExclude (MaskTypes _mask);
    
    virtual ~MorphOpMaskExclude ();

    virtual  OperationType   Operation ()  const  {return  moMaskExclude;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    int32  MemoryConsumedEstimated ();

  private:
    MaskTypes  mask;
  };  /* MorphOpMaskExclude */

  typedef  MorphOpMaskExclude*  MorphologicalOperatorMastPtr;

}  /* KKB */




#endif
