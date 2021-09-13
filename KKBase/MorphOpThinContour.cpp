/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
* Copyright (C) 1994-2014 Kurt Kramer
* For conditions of distribution and use, see copyright notice in KKB.h
*/

#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "MorphOp.h"
#include "MorphOpThinContour.h"
#include "Raster.h"
using namespace KKB;


MorphOpThinContour::MorphOpThinContour () :
  MorphOp ()
{
}



MorphOpThinContour::~MorphOpThinContour ()
{
}



size_t  MorphOpThinContour::MemoryConsumedEstimated ()  const
{
  size_t  result = MorphOp::MemoryConsumedEstimated () + sizeof (*this);
  return  result;
}



//****************************************************************
//* The following ThinningCode was lifted out of a IPL98 
//* Library and modified to conform with this object.
//***************************************************************
//
//   The Image Processing Library 98, IPL98    
//   by Ren Dencker Eriksen - edr@mip.sdu.dk
//
//  from module   "~\ipl98\source\ipl98\kernel_c\algorithms\kernel_morphology.c"
//

RasterPtr   MorphOpThinContour::PerformOperation (RasterConstPtr  _image)
{
  SetSrcRaster (_image);

#if  defined(DEBUG_ThinContour)
  cout << std::endl << std::endl
    << "Raster::ThinContour" << std::endl
    << std::endl;

  rasterGlobalHeight = height;
  rasterGlobalWidth = width;
#endif


  bool   PointsRemoved = false;
  uchar  m_Matrix22[3][3];

  kkint32  Iter = 0;
  kkint32  prem1 = 0;
  kkint32  prem2 = 0;
  kkint32  iCountX = 0;
  kkint32  iCountY = 0;
  kkint32  pntinpic = 0;

  PointList  pointList (true);
  PointList  removeList (true);

  //ContourFollower contourFollwer (*this);
  //PointListPtr  borderPixs = contourFollower.GenerateContourList (blob);

  RasterPtr workRaster = srcRaster->AllocateARasterInstance (*srcRaster);
  workRaster->ConnectedComponent (3);

  uchar**   workGreen = workRaster->Green ();
  ErodeSpurs (workRaster);
  // workRaster->Dilation ();

  //   k_SetBorder(1,1,pImg);
  PointsRemoved = false;
  Iter++;

  /* step 1 Collecting the Black point in a list */
  prem1 = prem2 = 0;

  kkint32  minCol, maxCol, minRow, maxRow;

  workRaster->FindBoundingBox (minRow, minCol, maxRow, maxCol);

  if ((minRow > maxRow) || (minRow < 0) || (minCol < 0))
  {
#if  defined(DEBUG_ThinContour)
    cout << std::endl << std::endl
      << "Raster::ThinContour    'FindBoundingBox'" << std::endl
      << "                        minRow[" << minRow << "]  maxRow[" << maxRow << "]" << std::endl
      << std::endl;
    cout.flush ();
#endif
    // We must have a empty raster.  In this case there is nothing else we can do.
    return  workRaster;
  }

  for (iCountY = minRow; iCountY <= maxRow; iCountY++)
  {
    minCol = 999999;
    maxCol = -1;

    for (kkint32 x = 0; x < srcWidth; x++)
    {
      if (ForegroundPixel (workGreen[iCountY][x]))
      {
        maxCol = Max (maxCol, x);
        minCol = Min (minCol, x);
      }
    }

    for (iCountX = minCol; iCountX <= maxCol; ++iCountX)
    {
      if (ForegroundPixel (workGreen[iCountY][iCountX]))
      {
        PointPtr tempPoint = new Point (iCountY, iCountX);
        pntinpic++;

        if (ThinningSearchNeighbors (iCountX, iCountY, workGreen, &m_Matrix22[0]) &&
            k_ThinningCheckTransitions (&m_Matrix22[0]) &&
            k_ThinningStep1cdTests (&m_Matrix22[0])
           )
        {
          prem1++;
          PointsRemoved = true;
          removeList.PushOnBack (tempPoint);
        }
        else
        {
          pointList.PushOnBack (tempPoint);
        }
      }
    }
  }

#if  defined(DEBUG_ThinContour)
  cout << "Total black points:" << pntinpic << "\n";
  cout.flush ();
#endif


  /* Set all pixels positions in RemoveList in image to white */
  {
    PointPtr  pixel = removeList.PopFromFront ();
    while (pixel)
    {
      workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
      delete pixel;
      pixel = removeList.PopFromFront ();
    }
  }

  removeList.DeleteContents ();


  /* step 2 after step 1 which inserted points in list */
  if (PointsRemoved)
  {
#if  defined(DEBUG_ThinContour)
    cout << "PointsRemoved = true" << pntinpic << "\n";
    cout.flush ();
#endif

    PointPtr  tempPoint = NULL;

    for (kkuint32 pointListIdx = 0;  pointListIdx < pointList.QueueSize ();  pointListIdx++)
    {
      tempPoint = pointList.IdxToPtr (pointListIdx);
      if (tempPoint == NULL)
        continue;

      if (ThinningSearchNeighbors (tempPoint->Col (), tempPoint->Row (), workGreen, &m_Matrix22[0]) &&
        k_ThinningCheckTransitions (&m_Matrix22[0]) &&
        k_ThinningStep2cdTests (&m_Matrix22[0])
        )
      {
        prem2++;
        PointsRemoved = true;

        //pointList.DeleteEntry (pointListIdx);
        pointList.SetIdxToPtr (pointListIdx, NULL);
        removeList.PushOnBack (tempPoint);
        //pointListIdx--; /* Must decrease pointListIdx when a point has been removed */
      }
    }
  }

  /* Set all pixels positions in RemoveList in image to white */
  {
    PointPtr  pixel = removeList.PopFromFront ();
    while (pixel)
    {
      workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
      delete pixel;
      pixel = removeList.PopFromFront ();
    }
  }

  removeList.DeleteContents ();
#if  defined(DEBUG_ThinContour)
  cout << "Iteration " << Iter << ": Points removed: " << prem1 << " + " << prem2 << " = " << prem1 + prem2 << "\n";
  cout.flush ();
#endif

#if  defined(DEBUG_ThinContour)
  cout << std::endl << "ThinContour  Starting Step 1    PointsRemoved[" << (PointsRemoved ? "True" : "False") << "]" << "\n";
  cout.flush ();
#endif
  /* step 1 */
  while (PointsRemoved)
  {
    PointPtr  tempPoint = NULL;

    prem1 = prem2 = 0;

    Iter++;
    PointsRemoved = false;

    for (kkuint32 pointListIdx = 0;  pointListIdx < pointList.QueueSize ();  pointListIdx++)
    {
      tempPoint = pointList.IdxToPtr (pointListIdx);
      if (tempPoint == NULL)
        continue;

      if ((ThinningSearchNeighbors (tempPoint->Col (), tempPoint->Row (), workGreen, &m_Matrix22[0])) &&
        (k_ThinningCheckTransitions (&m_Matrix22[0])) &&
        (k_ThinningStep1cdTests (&m_Matrix22[0]))
        )
      {
        prem1++;
        PointsRemoved = true;

        /*k_RemovePosFromGroupSlow(pointListIdx,&PointList);*/

        //pointList.DeleteEntry (pointListIdx);
        pointList.SetIdxToPtr (pointListIdx, NULL);
        removeList.PushOnBack (tempPoint);
        //pointListIdx--;  /* Must decrease pointListIdx when a point has been removed */
      }
    }

    /* Set all pixels positions in Remove List in image to white */
#if  defined(DEBUG_ThinContour)
    cout << "Set all pixels positions in Remove List in image to white.   removeList.size()=[" << removeList.size () << "]" << "\n";
    cout.flush ();
#endif

    {
      PointPtr  pixel = removeList.PopFromFront ();
      while (pixel)
      {
        workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
        delete pixel;
        pixel = removeList.PopFromFront ();
      }

      removeList.DeleteContents ();
    }


#if  defined(DEBUG_ThinContour)
    cout << "ThinContour  Starting Step 2" << "\n";
    cout.flush ();
#endif
    /* step 2 */
    for (kkuint32 pointListIdx = 0; pointListIdx < pointList.QueueSize (); pointListIdx++)
    {
      tempPoint = pointList.IdxToPtr (pointListIdx);
      if (tempPoint == NULL)
        continue;

      if (ThinningSearchNeighbors (tempPoint->Col (), tempPoint->Row (), workGreen, &m_Matrix22[0]) &&
        k_ThinningCheckTransitions (&m_Matrix22[0]) &&
        k_ThinningStep2cdTests (&m_Matrix22[0])
        )
      {
        prem2++;
        PointsRemoved = true;

        /*k_RemovePosFromGroupSlow(pointListIdx,&PointList);*/
        //pointList.DeleteEntry (pointListIdx);
        pointList.SetIdxToPtr (pointListIdx, NULL);
        removeList.PushOnBack (tempPoint);
        //pointListIdx--; /* Must decrease pointListIdx when a point has been removed */
      }
    }


#if  defined(DEBUG_ThinContour)
    cout << "ThinContour  LastStep in loop" << "\n";
    cout.flush ();
#endif

    /* Set all pixels positions in RemoveList in image to white */
    {
      PointPtr  pixel = removeList.PopFromFront ();
      while (pixel)
      {
        workGreen[pixel->Row ()][pixel->Col ()] = backgroundPixelValue;
        delete pixel;
        pixel = removeList.PopFromFront ();
      }

      removeList.DeleteContents ();
    }

#if  defined(DEBUG_ThinContour)
    cout << "Iteration " << Iter << ": Points removed: " << prem1 << " + " << prem2 << " = " << prem1 + prem2 << "\n";
    cout.flush ();
#endif

  }

#if  defined(DEBUG_ThinContour)
  cout << "ThinContour   Ready to Exit;  going to DeleteContents of 'pointList' and 'removeList'." << "\n";
  cout.flush ();
#endif


  pointList.DeleteContents ();
  removeList.DeleteContents ();

#if  defined(DEBUG_ThinContour)
  cout << "ThinContour   Exiting'." << "\n";
  cout.flush ();
#endif

  return  workRaster;
}  /* PerformOperation */



void  MorphOpThinContour::ErodeSpurs (RasterPtr  src)
{
  Raster  origRaster (*src);

  uchar**  origGreen = origRaster.Green ();

  kkint32  firstRow = 1;
  kkint32  firstCol = 1;
  kkint32  lastRow = src->Height () - 1;
  kkint32  lastCol = src->Width  () - 1;

  for (kkint32 r = firstRow; r < lastRow; r++)
  {
    for (kkint32 c = firstCol; c < lastCol; c++)
    {
      if (src->ForegroundPixel(r, c))
      {
        // We have a foreground Pixel.

        if ((BackgroundPixel (origGreen[r - 1][c - 1]))  && 
            (BackgroundPixel (origGreen[r - 1][c]))      &&
            (BackgroundPixel (origGreen[r - 1][c + 1]))  &&
            (BackgroundPixel (origGreen[r    ][c - 1]))  &&
            (BackgroundPixel (origGreen[r    ][c + 1])))
        {
          // Top Spur
          src->SetPixelValue(r, c, backgroundPixelValue);
        }

        else if (
              (BackgroundPixel (origGreen[r - 1][c - 1])) &&
              (BackgroundPixel (origGreen[r    ][c - 1])) &&
              (BackgroundPixel (origGreen[r + 1][c + 1])) &&
              (BackgroundPixel (origGreen[r - 1][c    ])) &&
              (BackgroundPixel (origGreen[r + 1][c    ])))
        {
            // Left Spur
            src->SetPixelValue(r, c, backgroundPixelValue);
        }

        else if (
              (BackgroundPixel (origGreen[r + 1][c - 1])) &&
              (BackgroundPixel (origGreen[r + 1][c    ])) &&
              (BackgroundPixel (origGreen[r + 1][c + 1])) &&
              (BackgroundPixel (origGreen[r    ][c - 1])) &&
              (BackgroundPixel (origGreen[r    ][c + 1])))
        {
              // Bottom Spur
          src->SetPixelValue (r, c, backgroundPixelValue);
        }

        else if (
              (BackgroundPixel (origGreen[r - 1][c + 1])) &&
              (BackgroundPixel (origGreen[r    ][c + 1])) &&
              (BackgroundPixel (origGreen[r + 1][c + 1])) &&
              (BackgroundPixel (origGreen[r - 1][c    ])) &&
              (BackgroundPixel (origGreen[r + 1][c    ])))
        {
                // Right Spur
          src->SetPixelValue (r, c, backgroundPixelValue);
        }
      }
    }
  }
}  /* ErodeSpurs */



bool  MorphOpThinContour::ThinningSearchNeighbors (kkint32 x,   // column
                                                   kkint32 y,   // row
                                                   uchar** g,
                                                   uchar   m_Matrix22[][3]
                                                  ) const
/* As (a) in Gonzales and Woods, between 2 and 6 black neighbors */
{
#if  defined(DEBUG_ThinContour)
  if ((x < 1) || (x >= (rasterGlobalWidth - 1)))
  {
    cout << "\n"
      << "k_ThinningSearchNeighbors    x[" << x << "] is to close to the edge." << "\n"
      << "\n";
  }

  if ((y < 1) || (y >= (rasterGlobalHeight - 1)))
  {
    cout << "\n"
      << "k_ThinningSearchNeighbors    y[" << y << "] is to close to the edge." << "\n"
      << "\n";
  }
#endif


  kkint32  BlackNeighbor = 0;
  //added by baishali 
  if ((y == 0) || (x == 0) || (y >= srcHeight) || (x >= srcWidth))
  {
  }
  else
  {
    m_Matrix22[0][0] = (g[y - 1][x - 1] > 0) ? 0 : 1;
    m_Matrix22[1][0] = (g[y - 1][x] > 0) ? 0 : 1;
    m_Matrix22[2][0] = (g[y - 1][x + 1] > 0) ? 0 : 1;
    m_Matrix22[0][1] = (g[y][x - 1] > 0) ? 0 : 1;
    m_Matrix22[2][1] = (g[y][x + 1] > 0) ? 0 : 1;
    m_Matrix22[0][2] = (g[y + 1][x - 1] > 0) ? 0 : 1;
    m_Matrix22[1][2] = (g[y + 1][x] > 0) ? 0 : 1;
    m_Matrix22[2][2] = (g[y + 1][x + 1] > 0) ? 0 : 1;
    m_Matrix22[1][1] = (g[y][x] > 0) ? 0 : 1;
  }

  if (m_Matrix22[0][0] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[1][0] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[2][0] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[0][1] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[2][1] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[0][2] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[1][2] == 0) { ++BlackNeighbor; }
  if (m_Matrix22[2][2] == 0) { ++BlackNeighbor; }


  if ((BlackNeighbor >= 2) && (BlackNeighbor <= 6))
    return true;
  else
    return false;
}  /* ThinningSearchNeighbors */



   /* returns true if there is exactly one transition in the region around the actual pixel */
bool  MorphOpThinContour::k_ThinningCheckTransitions (uchar  m_Matrix22[][3])
{
  kkint32 iTransitions = 0;

  if ((m_Matrix22[0][0] == 1) && (m_Matrix22[1][0] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[1][0] == 1) && (m_Matrix22[2][0] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[2][0] == 1) && (m_Matrix22[2][1] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[2][1] == 1) && (m_Matrix22[2][2] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[2][2] == 1) && (m_Matrix22[1][2] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[1][2] == 1) && (m_Matrix22[0][2] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[0][2] == 1) && (m_Matrix22[0][1] == 0)) {
    ++iTransitions;
  }

  if ((m_Matrix22[0][1] == 1) && (m_Matrix22[0][0] == 0)) {
    ++iTransitions;
  }

  if (iTransitions == 1)
    return true;
  else
    return false;
}  /* k_ThinningCheckTransitions */



/* performs the tests (c) and (d) in step 1 as explained in Gonzales and Woods page 492 */
bool MorphOpThinContour::k_ThinningStep1cdTests (uchar  m_Matrix22[][3])
{
  if ((m_Matrix22[1][0] + m_Matrix22[2][1] + m_Matrix22[1][2]) &&
      (m_Matrix22[2][1] + m_Matrix22[1][2] + m_Matrix22[0][1])
     )
    return true;
  else
    return false;
}


/* performs the tests (c') and (d') in step 2 as explained in Gonzales and Woods page 493 */

bool  MorphOpThinContour::k_ThinningStep2cdTests (uchar m_Matrix22[][3])
{
  if ((m_Matrix22[1][0] + m_Matrix22[2][1] + m_Matrix22[0][1]) &&
    (m_Matrix22[1][0] + m_Matrix22[1][2] + m_Matrix22[0][1])
    )
    return true;
  else
    return false;
}


