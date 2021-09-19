/* MorphOpDilation.cpp -- Morphological operators use to perform erosion.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_MORPHOPDILATION_)
#define  _MORPHOPDILATION_

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
  class  MorphOpDilation :  public MorphOpStruct
  {
  public:
    MorphOpDilation (StructureType  _structure,
                     kkuint16       _structureSize
                    );
    
    virtual ~MorphOpDilation ();

    virtual  OperationType   Operation ()  const  {return  OperationType::Dilation;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    size_t  MemoryConsumedEstimated ()  const;


  private:
  };  /* MorphOpDilation */

  typedef  MorphOpDilation*  MorphOpDialationPtr;
}  /* KKB */



#endif
