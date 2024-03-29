/* Point.cpp -- Represents the coordinates in a Raster image.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>
#include <sstream>
#include "MemoryDebug.h"
using namespace std;

#include "Option.h"

#include "Point.h"
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



Point::Point  (kkint32  _row,
               kkint32  _col
              ):
  row (_row),
  col (_col)
{}


  
Point::Point  (float  _row,
               float  _col
              ):
   row (scINT32 (_row + 0.5f)),
   col (scINT32 (_col + 0.5f))
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



Point&  Point::operator+= (const Point&  r)
{
  row += r.row;
  col += r.col;
  return *this;
}



Point&  Point::operator-= (const Point&  r)
{
  row -= r.row;
  col -= r.col;
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


void  PointList::BoxCoordinites (kkint32&  minRow,
                                 kkint32&  minCol,
                                 kkint32&  maxRow,
                                 kkint32&  maxCol
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
  kkint32  totalRow = 0;
  kkint32  totalCol = 0;
  for  (iterator x = begin ();  x != end ();  x++)
  {
    PointPtr p = *x;
    totalRow += p->Row ();
    totalCol += p->Col ();
  }

  kkint32 centerCol = scINT32 (scDOUBLE (totalCol) / scDOUBLE (size ()) + 0.5);
  kkint32 centerRow = scINT32 (scDOUBLE (totalRow) / scDOUBLE (size ()) + 0.5);
  return  Point (centerRow, centerCol);
}



KKStr  PointList::ToDelStr (char del)  const
{
  if  (QueueSize () < 1)
    return "[]";

  KKStr  result (QueueSize () * 10);

  int  count = 0;
  PointList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx, ++count)
  {
    PointPtr  p = *idx;
    if  (count > 0)
      result << del;
    result << p->Row () << del << p->Col ();
  }
  return  result;
}  /* ToDelStr */


PointListPtr  PointList::FromDelStr (const KKStr&  _s)
{
  PointListPtr  result = new PointList (true);

  KKStr  s (_s);
  s.TrimLeft ();

  while  (s.Len () > 0)
  {
    char nextCh = s.FirstChar ();
    char endPairChar = 0;
    if  (nextCh == '[')
      endPairChar = ']';

    else if  (nextCh == '(')
      endPairChar = ')';

    else
    {
      // Not Bracketed.
      endPairChar = 0;
      kkint16  row = scINT16 (s.ExtractTokenInt (",\t\n\t"));
      kkint16  col = scINT16 (s.ExtractTokenInt (",\t\n\t"));
      result->PushOnBack (new Point (row, col));
    }

    if  (endPairChar != 0)
    {
      KKStr pairStr = "";
      auto  idx = s.Find (endPairChar);
      if  (idx)
      {
        pairStr = s.SubStrSeg (0, idx);
        s = s.SubStrPart (idx + 1);
      }
      else
      {
        pairStr = s;
        s = "";
      }

      kkint16  row = scINT16 (pairStr.ExtractTokenInt (","));
      kkint16  col = scINT16 (pairStr.ExtractTokenInt (","));
      result->PushOnBack (new Point (row, col));
      nextCh = s.FirstChar ();
      if  ((nextCh == ',')  ||  (nextCh == '\n')  ||  (nextCh == '\r')  || (nextCh == '\t'))
        s.ChopFirstChar ();
    }
    s.TrimLeft ();
  }

  return  result;
}  /* FromDelStr */



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

    float  deltaHeight = scFLOAT (nextPoint->Row () - lastPoint->Row ()) * heightFactor;
    float  deltaWidth  = scFLOAT (nextPoint->Col () - lastPoint->Col ()) * widthFactor;

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
