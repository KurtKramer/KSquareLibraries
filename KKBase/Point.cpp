/* Point.cpp -- Represents the coordinates in a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;


#include  "Point.h"
using namespace KKB;


Point::Point ():
  row (0),
  col (0)
{
}



Point::Point (const Point&  point):
   row (point.row),
   col (point.col)
{
}



Point::Point  (kkint16  _row,
               kkint16  _col
              ):
  row (_row),
  col (_col)
{}



Point::Point  (int32  _row,
               int32  _col
              ):
  row (_row),
  col (_col)
{}


  
Point::Point  (float  _row,
               float  _col
              ):
   row ((int32)(_row + 0.5)),
   col ((int32)(_col + 0.5))
{}


bool  Point::operator== (Point&  r)  const
{
  return ((row == r.row)  &&  (col == r.col));
}


bool  Point::operator!= (const Point&  r)  const
{
  return ((row != r.row)  ||  (col != r.col));
}


Point  Point::operator+ (const Point&  r) const
{
  return  Point (row + r.row, col + r.col);
}


Point  Point::operator- (const Point&  r) const
{
  return  Point (row - r.row, col - r.col);
}


Point&  Point::operator= (const Point&  r)
{
  row = r.row;
  col = r.col;
  return *this;
}



PointList::PointList (const PointList&  pointList):
    KKQueue<Point> (true)
{
  PointList::const_iterator  idx;
  for  (idx = pointList.begin ();  idx != pointList.end ();  idx++)
  {
    PushOnBack (new Point (*(*idx)));
  }
}



PointList::PointList (bool _owner):
    KKQueue<Point> (_owner)
{
}


void  PointList::BoxCoordinites (int32&  minRow,
                                 int32&  minCol,
                                 int32&  maxRow,
                                 int32&  maxCol
                                )
{
  minRow = minCol = 999999;
  maxRow = maxCol = -1;

  for  (iterator x = begin ();  x != end ();  x++)
  {
    PointPtr p = *x;
    if  (p->Row () < minRow)
      minRow = p->Row ();

    if  (p->Row () > maxRow)
      maxRow = p->Row ();

    if  (p->Col () < minCol)
      minCol = p->Col ();

    if  (p->Col () > maxCol)
      maxCol = p->Col ();
  }
}


Point  PointList::CalculateCenterPoint ()
{
  int32  totalRow = 0;
  int32  totalCol = 0;
  for  (iterator x = begin ();  x != end ();  x++)
  {
    PointPtr p = *x;
    totalRow += p->Row ();
    totalCol += p->Col ();
  }

  int32 centerRow = (int32)((double)totalRow / (double)size () + 0.5);
  int32 centerCol = (int32)((double)totalCol / (double)size () + 0.5);
  return  Point (centerRow, centerCol);
}




float  PointList::ComputeSegmentLens (float  heightFactor,
                                      float  widthFactor
                                     )  const
{
  if  (QueueSize () < 1)
    return 0.0f;

  float totalLen = 0.0f;

  const_iterator  idx;
  idx = begin ();
  PointPtr lastPoint = *idx;
  ++idx;

  while  (idx != end ())
  {
    PointPtr  nextPoint = *idx;  ++idx;

    float  deltaHeight = (float)(nextPoint->Row () - lastPoint->Row ()) * heightFactor;
    float  deltaWidth  = (float)(nextPoint->Col () - lastPoint->Col ()) * widthFactor;

    float  segmentLen = sqrt (deltaHeight * deltaHeight + deltaWidth * deltaWidth);

    totalLen += segmentLen;
    lastPoint = nextPoint;
  }

  return  totalLen;
}  /* ComputeSegmentLens */



KKStr&  KKB::operator<< (KKStr&       left,
                         const Point&  right
                        )
{
  left << "[" << right.Row () << ", " <<  right.Col () << "]";
  return  left;
}



ostream&  KKB::operator<< (std::ostream& left,
                           const Point&  right
                          )
{
  left << "[" << right.Row () << ", " <<  right.Col () << "]";
  return  left;
}
