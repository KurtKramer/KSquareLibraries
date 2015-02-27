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

#include  <math.h>
#include  <iostream>
#include  <vector>
#include  "MemoryDebug.h"
using namespace std;


#include  "BasicTypes.h"

#include  "ConvexHull.h"
#include  "Point.h"
#include  "Raster.h"
using namespace KKU;





ConvexHull::ConvexHull ()
{
  upperPoints = new PointList (true);
  lowerPoints = new PointList (true);
  upper       = new PointList (true);
  lower       = new PointList (true);
}


ConvexHull::~ConvexHull ()
{
  if  (upperPoints)
    delete  upperPoints;

  if  (lowerPoints)
    delete  lowerPoints;

  if  (upper)
    delete  upper;
  
  if (lower)
    delete  lower;
}




RasterPtr  ConvexHull::Filter (Raster&  src)
//requires: the src!=null ;
//modifies: it may modify dest
//effects: Finding the convex hull points on the src
//if src.getType!=TYPE_BYTE_BINARY, throw ImagingOpException
//if dest!=null store the Convex Hull Point processed image in the dest
//dest=null return the erosion processed image    

{
  int32 w = src.Width ();
  int32 h = src.Height ();
        
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





inline
double  Distance (Point& p1,
                  Point& p2)
{
  double  deltaY = 1.0 + p1.Row () - p2.Row ();
  double  deltaX = 1.0 + p1.Col () - p2.Col ();

  return sqrt (deltaX * deltaX + deltaY * deltaY);
}  /* Distance */
 



int32  ConvexHull::ConvexArea ()
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
    return (double)1.0;
  }


  if  (upper->QueueSize () == 0)
  {
    return 0.0;
  }

  PointList::iterator  it;

  double area = 0.0;

  it = upper->begin ();
  PointPtr  p = *it;  ++it;
  PointPtr  m = *it;  ++it;

  while  (it != upper->end ())
  {
    PointPtr  current = *it;  ++it;

    area = area + TriangleArea (*p, *m, *current);
    m = current;
  }
        
  return area;
}  /* ConvexArea */


 

void  ConvexHull::CalcConvexArea (RasterPtr   raster)
{
  convexArea = 0;

  int32 w = raster->Width ();
  int32 h = raster->Height ();

  uchar**  rows = raster->Rows ();
        
  for (int32 col = 0; col < w; col++)
  {
    int32  topRow;
    int32  botRow;

    for  (topRow = h - 1;  topRow >= 0;  topRow--)
    {
      if  (rows[topRow][col] > 0)
        break;
    }

    if  (topRow >= 0)
    {

      for  (botRow = 0; botRow < h; botRow++)
      {
        if  (rows[botRow][col] > 0)
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
  double  deltaY = 1.0 + fabs ((float)(p1.Row () - p2.Row ()));
  double  deltaX = 1.0 + fabs ((float)(p1.Col () - p2.Col ()));

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
  int32  col;
  int32  row;



  if  (p1.Col () == p2.Col ())
  {
    int32  startRow;
    int32  endRow;

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


  // If we made it here then we are not a verticle line.


  double  m = (double)(p1.Row () - p2.Row ()) / (double)(p1.Col () - p2.Col ());
  double  c = (double)p2.Row () - m * (double)p2.Col ();

  if  (fabs (m) < 1)
  {
    int32  startCol;
    int32  endCol;

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
      row = (int32)(m * (double)col + c + 0.5);  // The Extract 0.5 is for rounding.
      raster.SetPixelValue (row, col, pixVal);
    }
  }
  else
  {
    int32  startRow;
    int32  endRow;

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
      col = (int32)((((double)row - c) / m) + 0.5);  // The Extract 0.5 is for rounding.
      raster.SetPixelValue (row, col, pixVal);
    }
  }
}  /* DrawLine */





void  ConvexHull::Draw (Raster& output)
//effects: if output==null throw NullPointerException
//        else draw the points in the upper

{
  // When output was created, would default to backGround.
  //  Arrays.fill (d, backGround);
  //  output.setSamples (0, 0, w, h, 0, d);
  
  if  (upper->size () < 2)
    return;

  PointList::iterator it = upper->begin ();

  PointPtr  lastPoint  = *it;  ++it;
  PointPtr  nextPoint  = *it;  ++it;
  PointPtr  firstPoint = lastPoint;
    
  while (nextPoint)
  {
    DrawLine (output, *lastPoint, *nextPoint, 255);
    lastPoint = nextPoint;
    if  (it != upper->end ())
    {
      nextPoint = *it;  ++it;
    }
    else
    {
      nextPoint = NULL;
    }
  }

  DrawLine (output, *lastPoint, *firstPoint, 255);
}  /* Draw */
    




void  ConvexHull::Merge ()
//modifies: this
//effects: if upper=null || lower==null throws NullPointerException
//          else remove the first and the last points in the lower 
//              and link the lower to the upper

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
    



int32 ConvexHull::RelativeCCW (Point&  sp,
                               Point&  ep,
                               Point&  p)
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
      //  This is a verticle Line Pointing Down.
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


  // If we made it here then we are not a verticle line.


  double  m = (double)(sp.Row () - ep.Row ()) / (double)(sp.Col () - ep.Col ());
  double  c = (double)ep.Row () - m * (double)ep.Col ();

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





void  ConvexHull::BuildUpperLink ()
//modifies: this
//effects: if upperPoints.size()<2 throw IllegalStateException
//          else use the incremental method to add the convex points into the upper
//              and make the upperPoints=null

{
  if  (upperPoints->QueueSize () < 2)
  {
    cerr << "Raster::BuildUpperLink *** ERROR ***, Not Enough Points to Build Upper Link." << std::endl;
    return;
  }
        
  PointList::iterator it;
  it = upperPoints->begin ();

  //  upper.add (it.next());
  upper->PushOnBack (new Point (**it));
  ++it;

  PointPtr  middle = new Point (**it);
  ++it;
         
  PointPtr  current = NULL;
  PointPtr  old     = NULL;

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
//modifies: this
//effects: if lowerPoints.size()<=2 throw IllegalStateException
//          else use the incremental method to add the convex points into the lower
//              and make the lowerPoints=null

{
  if  (lowerPoints->QueueSize () < 2)
  {
    cerr << "Raster::BuildLowerLink *** ERROR ***, Not Enough Points to Build LowerLink." << std::endl;
    return;
  }

        
  PointList::iterator  it = lowerPoints->begin ();

  lower->PushOnBack (new Point (**it));  ++it;
        

  PointPtr  middle = new Point (**it);   ++it;   
  PointPtr  current = NULL;
  PointPtr  old     = NULL;

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
    



void  ConvexHull::Store (Raster&  input)
//modifies: this
//effects: if input==null, throw NullPointerException
//      else store the foreground points of the input into points set
//      in such a way that from the left to the right, 
//      and in the column just save the uppermost and lower most points
{

  int32 w = input.Width ();
  int32 h = input.Height ();

  uchar**  rows = input.Rows ();
        
  for (int32 col = 0; col < w; col++)
  {
    int32 row;
    for  (row = h - 1;  row >= 0;  row--)
    {
      if  (rows[row][col] > 0)
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
        if  (rows[row][col] > 0)
        {
          PointPtr l = new Point (row, col);
          lowerPoints->AddFirst (l);
          break;
        }
      }
    }
  }
}  /* Store */
