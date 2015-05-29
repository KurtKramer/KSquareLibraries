#ifndef  _CONVEXHULL_
#define  _CONVEXHULL_

//**********************************************************************
//* Originally Developed by Tong Luo as a Java Applet.                 *
//*                                                                    *
//* Feb-28-03  Ported to c++ by Kurt Kramer to work with Raster class. *
//*                                                                    *
//**********************************************************************

/*
 * ConvexHull.java
 *
 * Created on April 7, 2002, 10:52 PM
 */


#include "KKBaseTypes.h"
#include "MorphOp.h"


namespace KKB
{
  #ifndef  _POINT_
  class  Point;
  typedef  Point*  PointPtr;
  class  PointList;
  typedef  PointList*  PointListPtr;
  #endif


  #ifndef _RASTER_
  class  Raster;
  typedef  Raster*        RasterPtr;
  typedef  Raster const*  RasterConstPtr;
  #endif


  /**
   *@brief  Operator that will create a Convex Hull of a supplied image.
   */
  class  ConvexHull: public  MorphOp
  {
  public:
    ConvexHull ();

    virtual  ~ConvexHull ();

    virtual  OperationType   Operation ()  const  {return OperationType::ConvexHull;}

    virtual  RasterPtr  PerformOperation (RasterConstPtr  _image);

    RasterPtr  Filter (const Raster&  src);


    /*
     *@brief Will perform Convex-hull on 'src' and place result into 'dest'.
     *@returns  Returns dest.
     */
    RasterPtr  Filter (const Raster&  src,
                       RasterPtr      dest
                      );


    kkint32 ConvexArea ();

    void    Draw (Raster& output);


    /**
     *@brief  Build list of the upper and lower points in the image.
     *@details  for each column in the image where there is at least one foreground pixel will add
     * one point to 'upperPoints' for the pixel with the smallest row and one point to 'lowerPoints'
     * for the pixel with the largest row.
     *@param[in] input  Source image that we are to generate a Convex Hull for.
     */
    void    Store (const Raster&  input);


  private:
    void    AllocateMemory ();
    void    CleanUpMemory ();

    inline  double  Distance (Point& p1,
                              Point& p2
                             );

    void    BuildLowerLink ();

    void    BuildUpperLink ();

    void    CalcConvexArea (RasterPtr   raster);

    double  ConvexArea2 ();

    void    DrawLine (Raster&  raster,
                      Point&   p1, 
                      Point&   p2,
                      uchar    pixVal
                     );

    void    Merge ();
      
    kkint32 RelativeCCW (Point&  sp,
                         Point&  ep,
                         Point&  p
                        );

    static 
    double TriangleArea (Point& a, 
                         Point& b, 
                         Point& c
                        );


    kkint32       convexArea;
    PointListPtr  upperPoints;
    PointListPtr  lowerPoints;
    PointListPtr  upper;
    PointListPtr  lower;
  };

  typedef  ConvexHull*  ConvexHullPtr;
}  /* namespace KKB; */

#endif
