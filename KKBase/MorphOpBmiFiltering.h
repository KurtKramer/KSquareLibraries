/* MorphOpBmiFiltering.cpp -- Morphological operator used to Binarize image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if !defined(_MORPHOPBMIFILTERING_)
#define  _MORPHOPBMIFILTERING_

#include "KKBaseTypes.h"
#include "MorphOp.h"


#if  !defined(_RASTER_)
namespace  KKB
{
  class  Raster;
  typedef  Raster*  RasterPtr;
  typedef  Raster const *  RasterConstPtr;
}
#endif



namespace  KKB
{
  class  MorphOpBmiFiltering :  public MorphOp
  {
  public:
    MorphOpBmiFiltering ();
    
    virtual ~MorphOpBmiFiltering ();

    virtual  OperationType   Operation ()  const  {return  OperationType::Binarize;}

    virtual  RasterPtr   PerformOperation (RasterConstPtr  _image);

    size_t  MemoryConsumedEstimated ()  const;

    float  EigenRatio        () const  {return eigenRatio;}
    float  OrientationAngle  () const  {return orientationAngle;}

    kkint32 FirstQtrWeight   () const  {return firstQtrWeight;}
    kkint32 ForthQtrWeight   () const  {return forthQtrWeight;}

    kkint32 BeltWidth        () const  {return beltWidth;}
    kkint32 BoundingLength   () const  {return boundingLength;}
    kkint32 BoundingWidth    () const  {return boundingWidth;}

    float  LengthVsWidth     () const  {return lengthVsWidth;}

    float  HeadTailRatio     () const  {return headTailRatio;}

    float  EstimatedBmi      () const  {return estimatedBmi;}


  private:
    RasterPtr   ProcessBmi (uchar  minBackgroundTH);

    float  eigenRatio;
    float  orientationAngle;

    kkint32 firstQtrWeight;
    kkint32 forthQtrWeight;

    kkint32 beltWidth;
    kkint32 boundingLength;
    kkint32 boundingWidth;

    float  lengthVsWidth;

    float  headTailRatio;

    float  estimatedBmi;
  };

  typedef  MorphOpBmiFiltering*  MorphBmiFilteringPtr;

#define _MorphOpBmiFiltering_Defined_
}  /* KKB */



#endif
