#ifndef  _MORPHOPTHINCONTOUR_
#define  _MORPHOPTHINCONTOUR_

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
  class  MorphOpThinContour : public MorphOp
  {
  public:
    MorphOpThinContour ();

    virtual ~MorphOpThinContour ();

    virtual  OperationType   Operation ()  const { return  OperationType::Stretcher; }

    virtual  RasterPtr  PerformOperation (RasterConstPtr  _image);

    virtual  kkMemSize  MemoryConsumedEstimated ();

  private:
    void  ErodeSpurs (RasterPtr  src);


    bool  ThinningSearchNeighbors (kkint32 x,
                                   kkint32 y,   // row
                                   uchar** g,
                                   uchar   m_Matrix22[][3]
                                  ) const;

    bool  k_ThinningCheckTransitions (uchar  m_Matrix22[][3]);

    bool  k_ThinningStep1cdTests (uchar  m_Matrix22[][3]);

    bool  k_ThinningStep2cdTests (uchar m_Matrix22[][3]);

  };  /* MorphOpReduceByFactor */

  typedef  MorphOpThinContour*  MorphOpThinContourPtr;

}  /* KKB */


#endif
