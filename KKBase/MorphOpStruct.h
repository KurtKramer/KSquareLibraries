/* MorphOpStruct.cpp -- Morphological operators use to perform erosion.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_MORPHOPSTRUCT_)
#define  _MORPHOPSTRUCT_

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
  class  MorphOpStruct :  public MorphOp
  {
  public:
    MorphOpStruct (StructureType  _structure,
                   kkuint16       _structureSize
                  );
    
    virtual ~MorphOpStruct ();

    virtual  OperationType   Operation ()  const  {return  OperationType::Erosion;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image) = 0;

    kkint32  MemoryConsumedEstimated ();

    void  BackgroundCountTH (kkint32 _backgroundCountTH)  {backgroundCountTH = _backgroundCountTH;}
    void  ForegroundCountTH (kkint32 _foregroundCountTH)  {foregroundCountTH = _foregroundCountTH;}


  protected:
    bool  Fit (kkint32  row, 
               kkint32  col
              )  const;

    bool  FitBackgroundCount (kkint32  row,
                              kkint32  col
                             )  const;

    //uchar  Hit (kkint32  row,
    //            kkint32  col
    //           )  const;

    uchar  HitForegroundCount (kkint32  row,
                               kkint32  col
                              )  const;




    StructureType  structure;
    kkint32        backgroundCountTH;  /**<  If greater than zero; then pixel must have at least that many neighbors to be considered a fit. */
    kkint32        foregroundCountTH;
    kkuint16       structureSize;

  };  /* MorphOpStruct */

  typedef  MorphOpStruct*  MorphOpStructPtr;
};  /* KKB */



#endif
