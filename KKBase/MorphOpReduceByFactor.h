/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
* Copyright (C) 1994-2014 Kurt Kramer
* For conditions of distribution and use, see copyright notice in KKB.h
*/

#ifndef  _MORPHOPREDUCEBYFACTOR_
#define  _MORPHOPREDUCEBYFACTOR_

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
  class  MorphOpReduceByFactor : public MorphOp
  {
  public:
    MorphOpReduceByFactor (float  _factor);

    virtual ~MorphOpReduceByFactor ();

    float  Factor ()  const { return factor; }

    virtual  OperationType   Operation ()  const { return  OperationType::Stretcher; }

    virtual  RasterPtr  PerformOperation (RasterConstPtr  _image);

    virtual  size_t  MemoryConsumedEstimated () const;

  private:
    float  factor;
  };  /* MorphOpReduceByFactor */

  typedef  MorphOpReduceByFactor*  MorphOpReduceByFactorPtr;

}  /* KKB */


#endif
