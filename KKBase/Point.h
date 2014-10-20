/* Point.h -- Represents the coordinates in a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _POINT_
#define  _POINT_

#include  "KKBaseTypes.h"
#include  "KKQueue.h"
#include  "KKStr.h"

namespace KKB
{
  /**
   *@class  Point  Point.h
   *@brief  Used by Raster class methods to denote a single pixel location in Raster image.
   *@see Raster
   */
  class  Point
  {
  public:
    Point ();

    Point  (const Point&  point);

    Point  (kkint16  _row,
            kkint16  _col
           );

    Point  (int32  _row,
            int32  _col
           );

    Point  (float  _row,
            float  _col
           );

    inline int32  Row () const {return row;}
    inline int32  Col () const {return col;}

    void   Row (int32 _row)  {row = _row;}
    void   Col (int32 _col)  {col = _col;}

    Point  UpOne    ()  const  {return Point (row - 1, col    );}
    Point  DownOne  ()  const  {return Point (row + 1, col    );}
    Point  LeftOne  ()  const  {return Point (row    , col - 1);}
    Point  RightOne ()  const  {return Point (row    , col + 1);}

    bool  operator== (Point&  r)  const;

    bool  operator!= (const Point&  r) const;

    Point  operator+ (const Point&  r) const;

    Point  operator- (const Point&  r) const;

    Point&  operator= (const Point&  r);

  private:
    int32  row;
    int32  col;
  }; /* Point */


  typedef  Point*  PointPtr;


  /**
   *@class  PointList  Point.h
   *@brief  Container object used to maintaining a list of pixel locations.
   */
  class  PointList:  public KKQueue<Point>
  {
  public:
    PointList (const PointList&  pointList);


    PointList (bool _owner);

    void  BoxCoordinites (int32&  minRow,
                          int32&  minCol,
                          int32&  maxRow,
                          int32&  maxCol
                         );

    Point  CalculateCenterPoint ();

    float  ComputeSegmentLens (float  heightFactor,
                               float  widthFactor
                              )  const;

  };  /* PointList */
    
  typedef  PointList*  PointListPtr;

  typedef  PointList::iterator  PointListIterator;



  KKStr&  operator<< (KKStr&        left,
                      const Point&  right
                     );

  std::ostream&  operator<< (std::ostream&  left,
                             const Point&   right
                            );
} /* namespace KKB; */

#endif