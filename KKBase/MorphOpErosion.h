/* MorphOpErosion.cpp -- Morphological operators use to perform erosion.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_MORPHOPEREROSION_)
#define  _MORPHOPEREROSION_

#include "KKBaseTypes.h"
#include "MorphOpStruct.h"


#if  !defined(_RASTER_)
namespace  KKB
{
  class  Raster;
  typedef  Raster*  RasterPtr;
}
#endif



namespace  KKB
{
  class  MorphOpErosion :  public MorphOpStruct
  {
  public:
    MorphOpErosion (StructureType  _structure,
                    kkuint16       _structureSize
                   );
    
    virtual ~MorphOpErosion ();

    virtual  OperationType   Operation ()  const  {return  OperationType::Erosion;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    kkMemSize  MemoryConsumedEstimated ();

  private:
  };  /* MorphOpErosion */

  typedef  MorphOpErosion*  MorphOpErosionPtr;
}  /* KKB */



#endif
