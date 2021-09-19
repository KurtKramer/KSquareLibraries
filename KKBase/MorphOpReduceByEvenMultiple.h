/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
* Copyright (C) 1994-2014 Kurt Kramer
* For conditions of distribution and use, see copyright notice in KKB.h
*/

#ifndef  _MORPHOPREDUCEBYEVENMULTIPLE_
#define  _MORPHOPREDUCEBYEVENMULTIPLE_

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
  class  MorphOpReduceByEvenMultiple : public MorphOp
  {
  public:
    MorphOpReduceByEvenMultiple (kkint32  _multiple);

    virtual ~MorphOpReduceByEvenMultiple ();

    kkint32  Multiple ()  const { return multiple; }

    virtual  OperationType   Operation ()  const { return  OperationType::Stretcher; }

    virtual  RasterPtr  PerformOperation (RasterConstPtr  _image);

    virtual  size_t  MemoryConsumedEstimated ()  const;

  private:
    kkint32  multiple;
  };  /* MorphOpReduceByFactor */

  typedef  MorphOpReduceByEvenMultiple*  MorphOpReduceByEvenMultiplePtr;

}  /* KKB */


#endif
