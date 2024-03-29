#include  "FirstIncludes.h"

//**********************************************************************
//* Originally Developed by Tong Luo as a Java Applet.                 *
//*                                                                    *
//* Feb-28-03  Ported to c++ by Kurt Kramer to work with Raster class  *
//*                                                                    *
//**********************************************************************

/*
 * ConvexHull.java
 *
 * Created on April 7, 2002, 10:52 PM
 */

#include <math.h>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"

#include "ConvexHull.h"
#include "KKException.h"
#include "Point.h"
#include "Raster.h"
using namespace KKB;




ConvexHull::ConvexHull ():
  MorphOp (),
  convexArea  (0),
  upperPoints (NULL),
  lowerPoints (NULL),
  upper       (NULL),
  lower       (NULL)
{
  AllocateMemory ();
}


ConvexHull::~ConvexHull ()
{
  CleanUpMemory ();
}



void  ConvexHull::AllocateMemory ()
{
  delete  upperPoints;  upperPoints = new PointList (true);
  delete  lowerPoints;  lowerPoints = new PointList (true);
  delete  upper;        upper       = new PointList (true);
  delete  lower;        lower       = new PointList (true);
}



void  ConvexHull::CleanUpMemory ()
{
  delete  upperPoints;   upperPoints = NULL;
  delete  lowerPoints;   lowerPoints = NULL;
  delete  upper;         upper       = NULL;
  delete  lower;         lower       = NULL;
}



RasterPtr  ConvexHull::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);
  AllocateMemory ();
  RasterPtr  result = Filter (*srcRaster);
  return  result;
}  /* PerformOperation */



/**
 *@brief  Returns a image that represents the convex-hull of the 'src' image.
 *param[in]  src  Source image that convex-hull is to be found for.
 *@returns A new raster that contains the convex-hull of the 'src' image; the caller will be responsible for deleting.
 */
RasterPtr  ConvexHull::Filter (const Raster&  src)
{
  SetSrcRaster (&src);

  kkint32 w = src.Width ();
  kkint32 h = src.Height ();
        
  Store (src);
  
  RasterPtr  dest = new Raster (h, w);

  if  ((upperPoints->QueueSize () == 1)  &&  
       (lowerPoints->QueueSize () == 1))
  {
    // We must have a Vertical  Line
    DrawLine (*dest, (*upperPoints)[0], (*lowerPoints)[0], 255);
    CalcConvexArea (dest);
    return  dest;
  }

  else if  ((upperPoints->QueueSize () < 1)  ||  (lowerPoints->QueueSize () < 1))
  {
    // We have Nothing
    return dest;
  }

  BuildUpperLink ();
  BuildLowerLink ();

  Merge ();

  Draw (*dest);
  
  CalcConvexArea (dest);

  return dest;
}  /* Filter */



RasterPtr  ConvexHull::Filter (const Raster&  src,
                               RasterPtr      dest
                              )
{
  SetSrcRaster (&src);
  //srcHeight = src.Height ();
  //srcWidth  = src.Width  ();

//  kkint32 w = src.Width ();
//  kkint32 h = src.Height ();

  if  ((dest->Height () != srcHeight)  ||  (dest->Width () != srcWidth))
    dest->ReSize (srcHeight, srcWidth, false);
        
  Store (src);

  if  ((upperPoints->QueueSize () == 1)  &&  
       (lowerPoints->QueueSize () == 1))
  {
    // We must have a Vertical  Line
    DrawLine (*dest, (*upperPoints)[0], (*lowerPoints)[0], 255);
    CalcConvexArea (dest);
    return  dest;
  }

  else if  ((upperPoints->QueueSize () < 1)  ||  (lowerPoints->QueueSize () < 1))
  {
    // We have Nothing
    return dest;
  }

  BuildUpperLink ();
  BuildLowerLink ();

  Merge ();

  Draw (*dest);
  
  CalcConvexArea (dest);

  return dest;
}  /* Filter */





inline
double  ConvexHull::Distance (Point& p1,
                              Point& p2
                             )
{
  double  deltaY = 1.0 + p1.Row () - p2.Row ();
  double  deltaX = 1.0 + p1.Col () - p2.Col ();

  return sqrt (deltaX * deltaX + deltaY * deltaY);
}  /* Distance */
 



kkint32  ConvexHull::ConvexArea ()
{
  return  convexArea;
}




double  ConvexHull::ConvexArea2 ()
//effects: if upper.size()<3 throw IllegalStateException
//        else calculate the area for the convex area

{
  if  (upper->QueueSize () == 2)
  {
    // We must have some kind of line.
    return  Distance ((*upper)[0], (*upper)[1]);
  }


  if  (upper->QueueSize () == 1)
  {
    // We have a dot
    return static_cast<double> (1.0);
  }


  if  (upper->QueueSize () == 0)
    return 0.0;

  PointList::iterator  it;
  it = upper->begin ();
       
  double area = 0.0;

  PointPtr p = *it;  ++it;
  PointPtr m = *it;  ++it;

  while  (it != upper->end ())
  {
    PointPtr current = *it;
    area = area + TriangleArea (*p, *m, *current);
    m = current;
    ++it;
  }
        
  return area;
}  /* ConvexArea */


 

void  ConvexHull::CalcConvexArea (RasterPtr   raster)
{
  convexArea = 0;

  kkint32 w = raster->Width ();
  kkint32 h = raster->Height ();

  uchar**  rows = raster->Rows ();
        
  for (kkint32 col = 0; col < w; col++)
  {
    kkint32  topRow;
    kkint32  botRow;

    for  (topRow = h - 1;  topRow >= 0;  topRow--)
    {
      if  (ForegroundPixel (rows[topRow][col]))
        break;
    }

    if  (topRow >= 0)
    {

      for  (botRow = 0; botRow < h; botRow++)
      {
        if  (ForegroundPixel (rows[botRow][col]))
          break;
      }

      convexArea += 1 + topRow - botRow;
    }
  }

  return;
}  /* CalcConvexArea */




inline
double  DistanceSquare (Point& p1,
                        Point& p2)
{
  double  deltaY = 1.0 + fabs (scDOUBLE (p1.Row () - p2.Row ()));
  double  deltaX = 1.0 + fabs (scDOUBLE (p1.Col () - p2.Col ()));

  return deltaX * deltaX + deltaY * deltaY;
}  /* Distance */
 




/*
double ConvexHull::TriangleArea (Point& a, 
                                 Point& b, 
		                         Point& c)

//effects: if a or b or c==null, throw NullPointerException
//          else return the positive area of a,b,c

{
  double  distBetweenAB = Distance (a, b);
  if  (distBetweenAB == 0)
    return  0;

  double  h;  // Height of Triangle.

  double  area;

  h = (c.Row () - a.Row ()) * (b.Row () - a.Row ())  +  (c.Col () - a.Col ()) * (b.Col () - a.Col ());
  h = h / (distBetweenAB * distBetweenAB);

  area =   distBetweenAB * h * 0.5;

  return  area;
} 
*/



double ConvexHull::TriangleArea (Point& a, 
                                 Point& b, 
		                         Point& c)
{
  double  abSqr = DistanceSquare (a, b);
  double  bcSqr = DistanceSquare (b, c);
  double  acSqr = DistanceSquare (a, c);
  double  ac = sqrt (acSqr);

  double  x = (abSqr - bcSqr + acSqr) / (2 * ac);

  double  h = sqrt (abSqr - (x * x));

  double  area = (h * ac) / 2.0;

  return  area;
}





void  ConvexHull::DrawLine (Raster&  raster,
                            Point&   p1, 
                            Point&   p2,
                            uchar    pixVal
                           )
{
  kkint32  col;
  kkint32  row;



  if  (p1.Col () == p2.Col ())
  {
    kkint32  startRow;
    kkint32  endRow;

    col = p2.Col ();

    if  (p1.Row() < p2.Row ())
    {
      startRow = p1.Row ();
      endRow   = p2.Row ();
    }
    else
    {
      startRow = p2.Row ();
      endRow   = p1.Row ();
    }

    for  (row = startRow; row <= endRow; row++)
    {
      raster.SetPixelValue (row, col, pixVal);
    }

    return;
  }


  // If we made it here then we are not a vertical line.


  double  m = scDOUBLE (p1.Row () - p2.Row ()) / scDOUBLE (p1.Col () - p2.Col ());
  double  c = scDOUBLE (p2.Row ()) - m * scDOUBLE (p2.Col ());

  if  (fabs (m) < 1)
  {
    kkint32  startCol;
    kkint32  endCol;

    if  (p1.Col () < p2.Col ())
    {
      startCol = p1.Col ();
      endCol   = p2.Col ();
    }
    else
    {
      startCol = p2.Col ();
      endCol   = p1.Col ();
    }

    for  (col = startCol; col <= endCol; col++)
    {
      row = scINT32 (m * scDOUBLE (col) + c + 0.5);  // The Extract 0.5 is for rounding.
      raster.SetPixelValue (row, col, pixVal);
    }
  }
  else
  {
    kkint32  startRow;
    kkint32  endRow;

    if  (p1.Row () < p2.Row ())
    {
      startRow = p1.Row ();
      endRow   = p2.Row ();
    }
    else
    {
      startRow = p2.Row ();
      endRow   = p1.Row ();
    }

    for  (row = startRow; row <= endRow; row++)
    {
      col = scINT32 (((scDOUBLE (row) - c) / m) + 0.5);  // The Extract 0.5 is for rounding.
      raster.SetPixelValue (row, col, pixVal);
    }
  }
}  /* DrawLine */





void  ConvexHull::Draw (Raster& output)
{
  PointList::iterator  it;  // (*upper);
  it = upper->begin ();

  PointPtr  lastPoint = NULL;
  PointPtr  nextPoint = NULL;

  if  (it != upper->end ())
  {lastPoint = *it;  ++it;}

  if  (it != upper->end ())
  {nextPoint = *it;  ++it;}

  PointPtr  firstPoint = lastPoint;

  if  ((!nextPoint)  &&  (!lastPoint))
    return;

  if ((nextPoint != NULL)  &&  (lastPoint == NULL))
  {
    output.SetPixelValue (nextPoint->Row (), nextPoint->Col (), 255);
    return;
  }
    
  while (nextPoint)
  {
    DrawLine (output, *lastPoint, *nextPoint, 255);
    lastPoint = nextPoint;
    if  (it == upper->end ())
      nextPoint = NULL;
    else
    {
      nextPoint = *it;
      ++it;
    }
  }

  DrawLine (output, *lastPoint, *firstPoint, 255);
}  /* Draw */
    



void  ConvexHull::Merge ()
{
  PointPtr  p;

  
  if  (*(lower->LookAtFront ()) == *(upper->LookAtFront ()))
  {
    p = lower->PopFromFront ();
    delete p;
  }


  if  (*(lower->LookAtBack ()) == *(upper->LookAtBack ()))
  {
    p = lower->PopFromBack ();
    delete p;
  }

  if  (lower->QueueSize () > 0)
  {
    p = lower->PopFromFront ();
    while  (p)
    {
      upper->PushOnBack (p);
      p = lower->PopFromFront ();
    }
  }
}  /* Merge */
    



kkint32 ConvexHull::RelativeCCW (Point&  sp,
                                 Point&  ep,
                                 Point&  p
                                )
{
  if  (sp.Col () == ep.Col ())
  {
    // We are looking at a Vertical Line

    if  (sp.Row () < ep.Row ())
    {
      //  This is a vertical Line Pointing Up.
      
      if  (p.Col () > ep.Col ())
      {
        return -1;  // Clockwise Turn(Right)
      }

      else if  (p.Col () < ep.Col ())
      {
        return  1;  // Counter-Clockwise Turn(Left).
      }

      else
      {
        //  Next Point is on Same Line
        if  (p.Row () < ep.Row ())
        {
          return  1;  // In front of Line
        }

        else if  (p.Row () > sp.Row ())
        {
          return  -1; // Behind Line
        }

        else
        {
          return  0;  // On Line
        }
      }
    }
    else
    {
      //  This is a vertical Line Pointing Down.
      if  (p.Col () > ep.Col ())
      {
        return  1;  // Counter-Clockwise Turn(Left).
      }

      else if  (p.Col () < ep.Col ())
      {
        return -1;  // Clockwise Turn(Right)
      }

      else
      {
        //  Next Point is on Same Line
        if  (p.Row () > ep.Row ())
        {
          return   1;  // In front of Line
        }

        else if  (p.Row () < sp.Row ())
        {
          return  -1;  // Behind Line
        }

        else
        {
          return  0;   // On Line.
        }
      }
    }
  }


  // If we made it here then we are not a vertical line.


  double  m = scDOUBLE (sp.Row () - ep.Row ()) / scDOUBLE (sp.Col () - ep.Col ());
  double  c = scDOUBLE (ep.Row ()) - m * scDOUBLE (ep.Col ());

  double  extendedY = m * p.Col () + c;  // This is where the line segment will be 
                                         // if extended to same col as "p".   

  if  (extendedY == p.Row ())
    return 0;

  
  if  (sp.Col () < ep.Col ())
  {
    // we are heading to the right

    if  (extendedY < p.Row ())
      return 1;

    else
      return -1;
  }
  else
  {
    // We are heading to the left.
    if  (extendedY > p.Row ())
      return 1;

    else
      return -1;
  }
}  /* RelativeCCW */



/**
 *@brief Builds the upper list of points that make the top have of the convex-hull.
 *@details if upperPoints.size()<2 throw IllegalStateException else use the incremental method to
 * add the convex points into the upper and make the upperPoints=null.
 */
void  ConvexHull::BuildUpperLink ()
{
  if  (upperPoints->QueueSize () < 2)
  {
    KKStr  msg = "ConvexHull::BuildUpperLink *** ERROR ***, Not Enough Points to Build Upper Link.";
    cerr << msg << std::endl;
    throw KKException (msg);
  }
        
  PointList::iterator  it;  // (*upperPoints);
  it = upperPoints->begin ();

  //  upper.add (it.next());
  upper->PushOnBack (new Point (**it));  ++it;

   //  Point middle = (Point)(it.next ());
  PointPtr  middle = new Point (**it);   ++it;
         
  PointPtr  current;
  PointPtr  old;

  while  (it != upperPoints->end ())
  {
    current = *it;  ++it;

    old = upper->BackOfQueue ();
            
    while (RelativeCCW (*old, *middle, *current) >= 0)
    {
      delete  middle;
      middle = upper->PopFromBack ();

      if  (upper->QueueSize () > 0)
         old = upper->BackOfQueue ();
      else 
         break;
    }
           
    upper->PushOnBack (middle);
    middle = new Point (*current);
  }

  upper->PushOnBack (middle);

  delete  upperPoints;
  upperPoints = NULL;
}  /* BuildUpperLink */
    


void   ConvexHull::BuildLowerLink ()
{
  if  (lowerPoints->QueueSize () < 2)
  {
    KKStr  msg = "ConvexHull::BuildLowerLink *** ERROR ***, Not Enough Points to Build Upper Link.";
    cerr << msg << std::endl;
    throw KKException (msg);
  }
        
  PointList::iterator it;
  it  = lowerPoints->begin ();

  lower->PushOnBack (new Point (**it));  ++it;

  PointPtr  middle = new Point (**it);   ++it;   
  PointPtr current = NULL;
  PointPtr old     = NULL;

  while  (it != lowerPoints->end ())
  {
    current = *it;  ++it;

    old = lower->GetLast ();
            
    while  (RelativeCCW (*old, *middle, *current) >= 0)
    {
      delete  middle;
      middle = lower->RemoveLast ();  // Same as PopFromBack

      if  (lower->QueueSize () > 0)
         old = lower->GetLast ();
      else 
         break;
    }
            
    lower->Add (middle);
    middle = new Point (*current);
  }

  lower->Add (middle);
        
  delete  lowerPoints;
  lowerPoints = NULL;
}  /* BuildLowerLink */
    


/**
 *@brief  Build list of the upper and lower points in the image.
 *@details  for each column in the image where there is at least one foreground pixel will add
 * one point to 'upperPoints' for the pixel with the smallest row and one point to 'lowerPoints'
 * for the pixel with the largest row.
 *@param[in] input  Source image that we are to generate a convex-hull for.
 */
void  ConvexHull::Store (const Raster&  input)
{
  kkint32 w = input.Width ();
  kkint32 h = input.Height ();

  for (kkint32 col = 0;  col < w;  ++col)
  {
    kkint32 row;
    for  (row = h - 1;  row >= 0;  --row)
    {
      if  (ForegroundPixel (row, col))
      {
        PointPtr u = new Point (row, col);
        upperPoints->Add (u);
        break;
      }
    }

    if  (row >= 0)
    {
      for  (row = 0; row < h; row++)
      {
        if  (ForegroundPixel (row, col))
        {
          PointPtr l = new Point (row, col);
          lowerPoints->AddFirst (l);
          break;
        }
      }
    }
  }
}  /* Store */

