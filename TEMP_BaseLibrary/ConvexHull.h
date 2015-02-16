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


#include  "BasicTypes.h"


namespace KKU
{
  #ifndef  _POINT_
  class  Point;
  typedef  Point*  PointPtr;
  class  PointList;
  typedef  PointList*  PointListPtr;
  #endif


  #ifndef _RASTER_
  class  Raster;
  typedef  Raster*  RasterPtr;
  #endif


  class  ConvexHull
  {
  public:
    ConvexHull  ();

    ~ConvexHull ();


    RasterPtr  Filter (Raster&  src);

    int32   ConvexArea ();

    void    Draw (Raster& output);

    void    Store (Raster&  input);


  private:
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
      
    int32   RelativeCCW (Point&  sp,
                         Point&  ep,
                         Point&  p);

    static 
    double TriangleArea (Point& a, 
                         Point& b, 
		                 Point& c);


    int32         convexArea;

    PointListPtr  upperPoints;
    PointListPtr  lowerPoints;
    PointListPtr  upper;
    PointListPtr  lower;
  };

  typedef  ConvexHull*  ConvexHullPtr;
}  /* namespace KKU; */

#endif
