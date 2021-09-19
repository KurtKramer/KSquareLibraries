/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _MORPHOPSTRETCHER_
#define  _MORPHOPSTRETCHER_

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
  class  MorphOpStretcher :  public MorphOp
  {
  public:
    MorphOpStretcher (float  _rowFactor,
                      float  _colFactor
                     );
    
    virtual ~MorphOpStretcher ();

    float  ColFactor  ()  const  {return colFactor;}
    float  RowFactor  ()  const  {return rowFactor;}

    virtual  OperationType   Operation ()  const  {return  OperationType::Stretcher;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    size_t  MemoryConsumedEstimated () const;

  private:
    float  rowFactor;
    float  colFactor;

    class  CellFactor;
    typedef  CellFactor*  CellFactorPtr;

    CellFactorPtr  BuildCellFactors (float   factor,
                                     kkuint32  cellFactorsCount
                                    );

    void  UpdateFactors (kkuint32  height,
                         kkuint32  width
                        );

    kkuint32      rowFactorsCount;
    CellFactor*   rowFactors;

    kkuint32      colFactorsCount;
    CellFactor*   colFactors;

  };  /* MorphOpStretcher */

  typedef  MorphOpStretcher*  MorphOpStretcherPtr;

}  /* KKB */




#endif
