/* ContourFollower.cpp -- Used to find the contour of image in a Raster object.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory>
#include <math.h>
#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#if defined(FFTW_AVAILABLE)
#  include  <fftw3.h>
#else
#  include  "kku_fftw.h"
#endif



#include "ContourFollower.h"
#include "Raster.h"
#include "KKStr.h"
using namespace KKB;



const 
MovDir   movements[8]  = {{-1,  0}, // Up
                          {-1,  1}, // Up-Right
                          { 0,  1}, // Right
                          { 1,  1}, // Down-Right
                          { 1,  0}, // Down
                          { 1, -1}, // Down-Left
                          { 0, -1}, // Left,
                          {-1, -1}  // Up-Left
                         };




ContourFollower::ContourFollower (Raster&  _raster,
                                  RunLog&  _log
                                 ):
  curCol  (-1),
  curRow  (-1),
  fromDir (),
  height  (_raster.Height ()),
  lastDir (0),
  log     (_log),
  raster  (_raster),
  rows    (_raster.Rows   ()),
  width   (_raster.Width  ())
{
}



uchar  ContourFollower::PixelValue (kkint32 row,
                                    kkint32 col
                                   )
{
  if  ((row < 0)  ||  (row >= height))  return 0;
  if  ((col < 0)  ||  (col >= width))   return 0;
  return  rows[row][col];
}


kkint32  ContourFollower::PixelCountIn9PixelNeighborhood (kkint32  row, 
                                                      kkint32  col
                                                     )
{
  kkint32  count = 0;
  if  (PixelValue (row - 1, col - 1) > 0)  ++count;
  if  (PixelValue (row - 1, col    ) > 0)  ++count;
  if  (PixelValue (row - 1, col + 1) > 0)  ++count;
  if  (PixelValue (row    , col - 1) > 0)  ++count;
  if  (PixelValue (row    , col    ) > 0)  ++count;
  if  (PixelValue (row    , col + 1) > 0)  ++count;
  if  (PixelValue (row + 1, col - 1) > 0)  ++count;
  if  (PixelValue (row + 1, col    ) > 0)  ++count;
  if  (PixelValue (row + 1, col + 1) > 0)  ++count;
  return  count;
}



void  ContourFollower::GetFirstPixel (kkint32&  startRow,
                                      kkint32&  startCol
                                     )
{
  lastDir = 1;
  fromDir = 0;

  startRow = height / 2;
  startCol = 0;

  while  ((startCol < width)  &&  (rows[startRow][startCol] == 0))
  {
    startCol++;
  }

  if  (startCol >= width)
  {
    // We did not find a Pixel in the Middle Row(height / 2) so now we will
    // scan the image row, by row, Left to Right until we find an occupied 
    // pixel.

    bool  found = false;
    kkint32  row;
    kkint32  col;

    for  (row = 0; ((row < height)  &&  (!found));  row++)
    {
      for  (col = 0; ((col < width)  &&  (!found)); col++)
      {
        if  (rows[row][col] != 0)
        {
          found = true;
          startRow = row;
          startCol = col;
        }
      }
    }

    if  (!found)
    {
      startRow = startCol = -1;
      return;
    }
  }

  curRow = startRow;
  curCol = startCol;

}  /* GetFirstPixel */



void  ContourFollower::GetNextPixel (kkint32&  nextRow,
                                     kkint32&  nextCol
                                    )
{
  fromDir = lastDir + 4;

  if  (fromDir > 7)
    fromDir = fromDir - 8;

  bool  nextPixelFound = false;

  kkint32  nextDir = fromDir + 2;

  while  (!nextPixelFound)
  {
    if  (nextDir > 7)
      nextDir = nextDir - 8;

    nextRow = curRow + movements[nextDir].row;
    nextCol = curCol + movements[nextDir].col;

    if  ((nextRow <  0)       ||  
         (nextRow >= height)  ||
         (nextCol <  0)       ||
         (nextCol >= width) 
        )
    {
      nextDir++;
    }
    else if  (rows[nextRow][nextCol] > 0)
    {
      nextPixelFound = true;
    }
    else
    {
      nextDir++;
    }
  }

  curRow = nextRow;
  curCol = nextCol;

  lastDir  = nextDir;
}  /* GetNextPixel */



#if  defined(FFTW_AVAILABLE)
  float  CalcMagnitude (fftwf_complex*  dest,
                        kkint32         index
                       )
  {
    float  mag = 0.0f;
    mag = (float)sqrt (dest[index][0] * dest[index][0] + dest[index][1] * dest[index][1]);
    return  mag;
  }  /* CalcMagnitude */
#else
  float  CalcMagnitude (KK_DFT1D_Float::DftComplexType* dest,
                        kkint32                         index
                       )
  {
    double rp = (float)dest[index].real ();
    double ip = (float)dest[index].imag ();
    return (float)sqrt (rp * rp + ip * ip);
  }  /* CalcMagnitude */
  
#endif






kkint32  ContourFollower::FollowContour (float*  countourFreq,
                                         float   fourierDescriptors[15],
                                         kkint32 totalPixels,
                                         bool&   successful
                                        )
{
  // startRow and startCol is assumed to come from the left (6)

  // I am having a problem;  
  //     We sometimes locate a separate piece of the image and end of with a contour of only that
  //     one piece.  This results in two problems.
  //                    1) The Fourier Descriptors do not even begin to represent the image
  //                    2) If the number of border pixels are small enough; then we could end
  //                       up causing a memory access fault when we access a negative index.


  kkint32  numOfBuckets = 5;

  kkint32  startRow;
  kkint32  startCol;

  kkint32  scndRow;
  kkint32  scndCol;

  kkint32  lastRow;
  kkint32  lastCol;

  kkint32  nextRow;
  kkint32  nextCol;

  kkint32  absoluteMaximumEdgePixels = totalPixels * 3;

  kkint32  maxNumOfBorderPoints = 3 * (height + width);
  kkint32  numOfBorderPixels = 0;

  kkint32 x;

  successful = true;

  kkint32  totalRow = 0;
  kkint32  totalCol = 0;

  #if  defined(FFTW_AVAILABLE)
  fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
  #else
  KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[maxNumOfBorderPoints];
  #endif


  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;


  while  (true)  
  {
    if  (numOfBorderPixels > absoluteMaximumEdgePixels)
    {
      // Something must be wrong;  somehow we missed the starting point.

      log.Level (-1) << endl << endl << endl
                     << "ContourFollower::FollowContour   ***ERROR***" << endl
                     << endl
                     << "     We exceeded the absolute maximum number of possible edge pixels." << endl
                     << endl
                     << "     FileName          [" << raster.FileName () << "]" << endl
                     << "     numOfBorderPixels [" << numOfBorderPixels  << "]" << endl
                     << endl;

      ofstream r ("c:\\temp\\ContourFollowerFollowContour.log", std::ios_base::ate);
      r << "FileName  [" << raster.FileName ()        << "]  ";  
      r << "totalPixels [" << totalPixels             << "]  ";
      r << "numOfBorderPixels [" << numOfBorderPixels << "]";
      r << std::endl;
      r.close ();
      break;
    }

    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }

    if  (numOfBorderPixels >= maxNumOfBorderPoints)
    {
      kkint32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;

      #if  defined(FFTW_AVAILABLE)
        fftwf_complex*  newSrc = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * newMaxNumOfAngles);
      #else
        KK_DFT1D_Float::DftComplexType*  newSrc = new KK_DFT1D_Float::DftComplexType[newMaxNumOfAngles];
      #endif

      if  (newSrc == NULL)
      {
        log.Level (-1) << endl << endl
                       << "FollowContour     ***ERROR***" << endl
                       << endl
                       << "Could not allocate memory needed for contour points." << endl
                       << endl;
        successful = false;
        return 0;
      }

      kkint32  x;
      for  (x = 0; x < maxNumOfBorderPoints; x++)
      {
        #if  defined(FFTW_AVAILABLE)
          newSrc[x][0] = src[x][0];
          newSrc[x][1] = src[x][1];
        #else
          newSrc[x].real (src[x].real ());
          newSrc[x].imag (src[x].imag ());
        #endif
      }


      #if  defined(FFTW_AVAILABLE)
        fftwf_free (src);
        src = newSrc;
        newSrc = NULL;
      #else
        delete  src;
        src = newSrc;
        newSrc = NULL;
      #endif

      maxNumOfBorderPoints = newMaxNumOfAngles;
    }

    #if  defined(FFTW_AVAILABLE)
      src[numOfBorderPixels][0] = (float)nextRow;
      src[numOfBorderPixels][1] = (float)nextCol;
    #else
      src[numOfBorderPixels].real ((float)nextRow);
      src[numOfBorderPixels].imag ((float)nextCol);
    #endif

    totalRow += nextRow;
    totalCol += nextCol;

    numOfBorderPixels++;
  }

  float  centerRow = (float)totalRow / (float)numOfBorderPixels;
  float  centerCol = (float)totalCol / (float)numOfBorderPixels;

  float  totalRe = 0.0;
  float  totalIm = 0.0;

  for  (x = 0;  x < numOfBorderPixels;  x++)
  {
    #if  defined(FFTW_AVAILABLE)
      src[x][0] = (src[x][0] - centerRow);
      src[x][1] = (src[x][1] - centerCol);
      totalRe += src[x][0];
      totalIm += src[x][1];
    #else
      src[x].real ((src[x].real () - centerRow));
      src[x].imag ((src[x].imag () - centerCol));
      totalRe += src[x].real ();
      totalIm += src[x].imag ();
    #endif
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*  dest = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
    fftwf_plan plan = fftwCreateOneDPlan (numOfBorderPixels, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
    plan = NULL;
  #else
    KK_DFT1D_Float  plan (numOfBorderPixels, true);
    KK_DFT1D_Float::DftComplexType*  dest = new KK_DFT1D_Float::DftComplexType[numOfBorderPixels];
    plan.Transform (src, dest);
  #endif

  kkint32  numOfedgePixels = numOfBorderPixels;

  kkint32*  count = new kkint32 [numOfBuckets];

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    countourFreq[x] = 0.0f;
    count[x] = 0;
  }

  // Original Region Areas,  as reflected in ICPR paper
  float  middle = numOfBorderPixels / (float)2.0;
  float  r1 = middle / (float)2.0;
  float  r2 = middle * ( (float)3.0  /   (float)4.0);
  float  r3 = middle * ( (float)7.0  /   (float)8.0);
  float  r4 = middle * ((float)15.0  /  (float)16.0);

  float  deltaX = 0.0f;
  float  mag    = 0.0f;

  if  (numOfBorderPixels < 8)
  {
    successful = false;
  }
  else
  {
    float normalizationFactor = CalcMagnitude (dest, 1);

    fourierDescriptors[ 0] = CalcMagnitude (dest, numOfBorderPixels - 1) / normalizationFactor;

    fourierDescriptors[ 1] = CalcMagnitude (dest, 2)                     / normalizationFactor;
    fourierDescriptors[ 2] = CalcMagnitude (dest, numOfBorderPixels - 2) / normalizationFactor;

    fourierDescriptors[ 3] = CalcMagnitude (dest, 3)                     / normalizationFactor;
    fourierDescriptors[ 4] = CalcMagnitude (dest, numOfBorderPixels - 3) / normalizationFactor;

    fourierDescriptors[ 5] = CalcMagnitude (dest, 4)                     / normalizationFactor;
    fourierDescriptors[ 6] = CalcMagnitude (dest, numOfBorderPixels - 4) / normalizationFactor;

    fourierDescriptors[ 7] = CalcMagnitude (dest, 5)                     / normalizationFactor;
    fourierDescriptors[ 8] = CalcMagnitude (dest, numOfBorderPixels - 5) / normalizationFactor;

    fourierDescriptors[ 9] = CalcMagnitude (dest, 6)                     / normalizationFactor;
    fourierDescriptors[10] = CalcMagnitude (dest, numOfBorderPixels - 6) / normalizationFactor;

    fourierDescriptors[11] = CalcMagnitude (dest, 7)                     / normalizationFactor;
    fourierDescriptors[12] = CalcMagnitude (dest, numOfBorderPixels - 7) / normalizationFactor;

    fourierDescriptors[13] = CalcMagnitude (dest, 8)                     / normalizationFactor;
    fourierDescriptors[14] = CalcMagnitude (dest, numOfBorderPixels - 8) / normalizationFactor;
  }

  for  (x = 1; x < (numOfBorderPixels - 1); x++)
  {
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im));
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im) + 1.0);
 
    mag = CalcMagnitude (dest, x);

    deltaX = (float)fabs ((float)x - middle);

    if  (deltaX < r1)
    {
      countourFreq[0] = countourFreq[0] + mag;
      count[0]++;
    }

    else if  (deltaX < r2)
    {
      countourFreq[1] = countourFreq[1] + mag;
      count[1]++;
    }

    else if  (deltaX < r3)
    {
      countourFreq[2] = countourFreq[2] + mag;
      count[2]++;
    }

    else if  (deltaX < r4)
    {
      countourFreq[3] = countourFreq[3] + mag;
      count[3]++;
    }

    else
    {
      countourFreq[4] = countourFreq[4] + mag;
      count[4]++;
    }
  }

  for  (x = 0; x < numOfBuckets; x++)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / (float)count[x];
    }
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);   src   = NULL;
    fftwf_free (dest);  dest  = NULL;
  #else
    delete  src;  src  = NULL;
    delete  dest; dest = NULL;
  #endif
  delete  count;      count = NULL;

  return  numOfedgePixels;
}  /* FollowContour */



kkint32  ContourFollower::FollowContour2 (float*  countourFreq,
                                        bool&   successful
                                       )
{
  // A diff Version of FollowContour

  // startRow and startCol is assumed to come from the left (6)

  kkint32  numOfBuckets = 16;

  kkint32  startRow;
  kkint32  startCol;

  kkint32  scndRow;
  kkint32  scndCol;

  kkint32  lastRow;
  kkint32  lastCol;

  kkint32  nextRow;
  kkint32  nextCol;

  kkint32  maxNumOfBorderPoints = 3 * (height + width);
  kkint32  numOfBorderPixels = 0;

  kkint32 x;

  successful = true;

  kkint32  totalRow = 0;
  kkint32  totalCol = 0;


  #if  defined(FFTW_AVAILABLE)
  fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
  #else
  KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[maxNumOfBorderPoints];
  #endif

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;

  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }

    if  (numOfBorderPixels >= maxNumOfBorderPoints)
    {
      kkint32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;

      #if  defined(FFTW_AVAILABLE)
        fftwf_complex*  newSrc = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * newMaxNumOfAngles);
      #else
        KK_DFT1D_Float::DftComplexType*  newSrc = new KK_DFT1D_Float::DftComplexType[newMaxNumOfAngles];
      #endif

      kkint32  x;
      for  (x = 0; x < maxNumOfBorderPoints; x++)
      {
        #if  defined(FFTW_AVAILABLE)
          newSrc[x][0] = src[x][0];
          newSrc[x][1] = src[x][1];
        #else
          newSrc[x].real (src[x].real ());
          newSrc[x].imag (src[x].imag ());
        #endif
      }

      #if  defined(FFTW_AVAILABLE)
        fftwf_free (src);
        src = newSrc;
        newSrc = NULL;
      #else
        delete  src;
        src = newSrc;
        newSrc = NULL;
      #endif

      maxNumOfBorderPoints = newMaxNumOfAngles;
    }

    #if  defined(FFTW_AVAILABLE)
      src[numOfBorderPixels][0] = (float)nextRow;
      src[numOfBorderPixels][1] = (float)nextCol;
    #else
      src[numOfBorderPixels].real ((float)nextRow);
      src[numOfBorderPixels].imag ((float)nextCol);
    #endif

    totalRow += nextRow;
    totalCol += nextCol;

    numOfBorderPixels++;
  }

  float  centerRow = (float)totalRow / (float)numOfBorderPixels;
  float  centerCol = (float)totalCol / (float)numOfBorderPixels;

  float  totalRe = 0.0;
  float  totalIm = 0.0;

  for  (x = 0;  x < numOfBorderPixels;  x++)
  {
    #if  defined(FFTW_AVAILABLE)
      src[x][0] = src[x][0] - centerRow;
      src[x][1] = src[x][1] - centerCol;
      totalRe+= (float)src[x][0];
      totalIm+= (float)src[x][1];
    #else
      src[x].real (src[x].real () - centerRow);
      src[x].imag (src[x].imag () - centerCol);
      totalRe+= (float)src[x].real ();
      totalIm+= (float)src[x].imag ();
    #endif
  }

  kkint32  numOfedgePixels = numOfBorderPixels;

  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*  dest = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
    fftwf_plan plan = fftwCreateOneDPlan (numOfBorderPixels, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
    plan = NULL;
  #else
    KK_DFT1D_Float  plan (numOfBorderPixels, true);
    KK_DFT1D_Float::DftComplexType*  dest = new KK_DFT1D_Float::DftComplexType[numOfBorderPixels];
    plan.Transform (src, dest);
  #endif

  kkint32*  count = new kkint32 [numOfBuckets];

  for  (x = 0; x < numOfBuckets; x++)
  {
    countourFreq[x] = 0.0;
    count[x] = 0;
  }

  float  middle = (float)numOfBorderPixels / (float)2.0;

  float  r0 = middle * ( (float) 8.0  /   (float)16.0);
  float  r1 = middle * ( (float)10.0  /   (float)16.0);
  float  r2 = middle * ( (float)12.0  /   (float)16.0);
  float  r3 = middle * ( (float)13.0  /   (float)16.0);


  float  deltaX;
  float  mag;

  kkint32  region = 0;

  for  (x = 1; x < numOfBorderPixels; x++)
  {
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im));
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im) + 1.0);


    #if  defined(FFTW_AVAILABLE)
      mag = (float)sqrt (dest[x][0] * dest[x][0] + dest[x][1] * dest[x][1]);
    #else
      mag = (float)sqrt (dest[x].real () * dest[x].real () + dest[x].imag () * dest[x].imag ());
    #endif

    deltaX = (float)x - middle;

    if  (fabs (deltaX) < r0)
      continue;

    if  (deltaX < 0)
    {
      // We are on the Left half.
      deltaX = fabs (deltaX);

      if       (x == 1)        region = 0;
      else if  (x == 2)        region = 1;
      else if  (x == 3)        region = 2;
      else if  (x == 4)        region = 3;
      else if  (deltaX >= r3)  region = 4;
      else if  (deltaX >= r2)  region = 5;
      else if  (deltaX >= r1)  region = 6;
      else                     region = 7;
    }
    else
    {
      // We are on Right Half of array
      if       (x == (numOfBorderPixels - 1))  region = 15;
      else if  (x == (numOfBorderPixels - 2))  region = 14;
      else if  (x == (numOfBorderPixels - 3))  region = 13;
      else if  (x == (numOfBorderPixels - 4))  region = 12;
      else if  (deltaX >= r3)                  region = 11;
      else if  (deltaX >= r2)                  region = 10;
      else if  (deltaX >= r1)                  region =  9;
      else                                     region =  8;
    }

    countourFreq[region] = countourFreq[region] + mag;
    count[region]++;
  }

  for  (x = 0; x < numOfBuckets; x++)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / (float)count[x];
    }
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);   src   = NULL;
    fftwf_free (dest);  dest  = NULL;
  #else
    delete[] src;   src  = NULL;
    delete[] dest;  dest = NULL;
  #endif
  
  delete  count;
  count = NULL;
  
  
  return  numOfedgePixels;
}  /* FollowContour2 */



PointListPtr  ContourFollower::GenerateContourList ()
{
  // startRow and startCol is assumed to come from the left (6)

  kkint32  startRow;
  kkint32  startCol;

  kkint32  scndRow;
  kkint32  scndCol;

  kkint32  lastRow;
  kkint32  lastCol;

  kkint32  nextRow;
  kkint32  nextCol;

  PointListPtr  points = new PointList (true);

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    return  NULL;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;

  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }

    points->PushOnBack (new Point (nextRow, nextCol));
  }

  return  points;
}  /* GenerateContourList */






ComplexDouble**  GetFourierOneDimMask (kkint32  size)
{
  static  
  kkint32  curMaskSize = 0;

  static  
  ComplexDouble**  fourierMask = NULL;

  if  (size == curMaskSize)
    return  fourierMask;

  ComplexDouble  N(size, 0);
  ComplexDouble  M(size, 0);

  kkint32  x;

  if  (fourierMask)
  {
    for  (x = 0;  x < curMaskSize;  x++)
    {
      delete  fourierMask[x];
      fourierMask[x] = NULL;
    }

    delete  fourierMask;
    fourierMask = NULL;
  }

  fourierMask = new ComplexDouble*[size];
  for  (x = 0;  x < size;  x++)
  {
    fourierMask[x] = new ComplexDouble[size];
  }
  curMaskSize = size;


  ComplexDouble  j;
  ComplexDouble  MinusOne (-1, 0);
  j = sqrt (MinusOne);

  ComplexDouble  Pi  (3.14159265359, 0);
  ComplexDouble  One (1.0, 0);
  ComplexDouble  Two (2.0, 0);

  for  (kkint32 m = 0;  m < size;  m++)
  {
    complex<double>  mc (m, 0);

    for  (kkint32 k = 0; k < size; k++)
    {
      complex<double>  kc (k, 0);

      // double  xxx = 2 * 3.14159265359 * (double)k * (double)m / (double)size;

      // fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);
      fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);

      double  exponetPart = 2.0 * 3.14159265359 * (double)k * (double)m / (double)size;
      double  realPart = cos (exponetPart);
      double  imgPart  = -sin (exponetPart);

      if  (realPart != fourierMask[m][k].real ())
      {
        continue;
      }

      if  (imgPart != fourierMask[m][k].imag ())
      {
        continue;
      }
    }
  }

  return  fourierMask;
}  /* GetFourierOneDimMask */




ComplexDouble**   GetRevFourierOneDimMask (kkint32  size)  // For reverse Fourier
{
  static  
  kkint32  curRevMaskSize = 0;

  static  
  ComplexDouble**  revFourierMask = NULL;


  if  (size == curRevMaskSize)
    return  revFourierMask;

  ComplexDouble  N(size, 0);
  ComplexDouble  M(size, 0);

  kkint32  x;
  

  if  (revFourierMask)
  {
    for  (x = 0;  x < curRevMaskSize;  x++)
    {
      delete  revFourierMask[x];
      revFourierMask[x] = NULL;
    }

    delete  revFourierMask;
    revFourierMask = NULL;
  }


  revFourierMask = new ComplexDouble*[size];
  for  (x = 0;  x < size;  x++)
  {
    revFourierMask[x] = new ComplexDouble[size];
  }
  curRevMaskSize = size;

  ComplexDouble  j;
  ComplexDouble  MinusOne (-1, 0);
  ComplexDouble  PositiveOne (1, 0);
  j = sqrt (MinusOne);

  ComplexDouble  Pi  (3.1415926, 0);
  ComplexDouble  One (1.0, 0);
  ComplexDouble  Two (2.0, 0);

  for  (kkint32 m = 0;  m < size;  m++)
  {
    complex<double>  mc (m, 0);

    for  (kkint32 k = 0; k < size; k++)
    {
      complex<double>  kc (k, 0);

      // double  xxx = 2 * 3.14159265359 * (double)k * (double)m / (double)size;

      // fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);
      revFourierMask[m][k] = exp (PositiveOne * j * Two * Pi * kc * mc / M);


      double  exponetPart = 2.0 * 3.14159265359 * (double)k * (double)m / (double)size;
      double  realPart = cos (exponetPart);
      double  imgPart  = sin (exponetPart);

      if  (realPart != revFourierMask[m][k].real ())
      {
        continue;
      }

      if  (imgPart != revFourierMask[m][k].imag ())
      {
        continue;
      }
    }
  }

  return  revFourierMask;
}  /* GetRevFourierOneDimMask */




kkint32  ContourFollower::CreateFourierDescriptorBySampling (kkint32  numOfBuckets,
                                                           float*  countourFreq,
                                                           bool&    successful
                                                          )
{
  // startRow and startCol is assumed to come from the left (6)
  // kkint32  numOfBuckets = 8;
  kkint32  startRow;
  kkint32  startCol;

  kkint32  scndRow;
  kkint32  scndCol;

  kkint32  lastRow;
  kkint32  lastCol;

  kkint32  nextRow;
  kkint32  nextCol;

  kkint32  numOfBorderPixels = 0;

  kkint32 x;

  successful = true;

  PointListPtr  points = new PointList (true);

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Vary Bad Starting Point" << std::endl;
    successful = false;
    return  0;
  }
   
  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;


  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }
 
    points->PushOnBack (new Point (nextRow, nextCol));

    numOfBorderPixels++;
  }


  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * numOfBuckets);
  #else
    KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[numOfBuckets];
  #endif

  kkint32  totalRow = 0;
  kkint32  totalCol = 0;

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    kkint32  borderPixelIdx = (kkint32)(((double)x * (double)numOfBorderPixels) /  (double)numOfBuckets);

    Point&  point = (*points)[borderPixelIdx];

    #if  defined(FFTW_AVAILABLE)
      src[x][0] = (float)point.Row ();
      src[x][1] = (float)point.Col ();
    #else
      src[x].real ((float)point.Row ());
      src[x].imag ((float)point.Col ());
    #endif

    totalRow += point.Row ();
    totalCol += point.Col ();
  }

  delete[]  points;
  points = NULL;

  float  centerRow = (float)totalRow / (float)numOfBuckets;
  float  centerCol = (float)totalCol / (float)numOfBuckets;

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    #if  defined(FFTW_AVAILABLE)
      src[x][0] = src[x][0] - centerRow;
      src[x][1] = src[x][1] - centerCol;
    #else
      src[x].real (src[x].real () - centerRow);
      src[x].imag (src[x].imag () - centerCol);
    #endif
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*  dest = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * numOfBuckets);
    fftwf_plan  plan = fftwCreateOneDPlan (numOfBuckets, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute (plan);
    fftwDestroyPlan (plan);
    plan = NULL;
  #else
    KK_DFT1D_Float  plan (numOfBuckets, true);
    KK_DFT1D_Float::DftComplexType*  dest = new KK_DFT1D_Float::DftComplexType[numOfBuckets];
    plan.Transform (src, dest);
  #endif

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    #if  defined(FFTW_AVAILABLE)
      float real = dest[x][0];
      float imag = dest[x][1];
    #else
      float real = dest[x].real ();
      float imag = dest[x].imag ();
    #endif

    countourFreq[x] = (float)(sqrt (real * real + imag * imag));
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);
    fftwf_free (dest);
  #else
    delete[]  src;   src  = NULL;
    delete[]  dest;  dest = NULL;
  #endif

  return  numOfBorderPixels;
}  /* CreateFourierDescriptorBySampling */




void  ContourFollower::HistogramDistanceFromAPointOfEdge (float   pointRow,
                                                          float   pointCol,
                                                          kkint32 numOfBuckets,
                                                          kkint32*  buckets,
                                                          float&  minDistance,
                                                          float&  maxDistance,
                                                          kkint32&  numOfEdgePixels
                                                         )
{
  PointListPtr  points = GenerateContourList ();

  numOfEdgePixels = points->QueueSize ();
  
  kkint32  x;

  minDistance = FloatMax;
  maxDistance = FloatMin;

  for  (x = 0;  x < numOfBuckets;  x++)
    buckets[x] = 0;

  float*  distances = new float[points->QueueSize ()];

  for  (x = 0;  x < points->QueueSize ();  x++)
  {
    Point& point = (*points)[x];

    float  deltaCol = (float)point.Col () - pointCol;
    float  deltaRow = (float)point.Row () - pointRow;

    float  distance = (float)sqrt (deltaCol * deltaCol  +  deltaRow  * deltaRow);

    distances[x] = distance;

    minDistance = Min (minDistance, distance);
    maxDistance = Max (maxDistance, distance);
  }


  float  bucketSize = (maxDistance - minDistance) / numOfBuckets;

  if  (bucketSize == 0.0)
  {
     buckets[numOfBuckets / 2] = points->QueueSize ();
  }
  else
  {
    for  (x = 0;  x < points->QueueSize ();  x++)
    {
      kkint32 bucketIDX = (kkint32)((distances[x] - minDistance) / bucketSize);

      buckets[bucketIDX]++;
    }
  }

  delete  points;     points    = NULL;
  delete  distances;  distances = NULL;

  return;
}  /* HistogramDistanceFromAPoint */



vector<ComplexDouble>  ContourFollower::CreateFourierFromPointList (const PointList&  points)
{
  //ComplexDouble*  src = new ComplexDouble[points.QueueSize ()];

  #if  defined(FFTW_AVAILABLE)
     fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * points.QueueSize ());
  #else
     KK_DFT1D_Double::DftComplexType*  src = new KK_DFT1D_Double::DftComplexType[points.QueueSize()];
  #endif


  kkint32  totalRow = 0;
  kkint32  totalCol = 0;

  kkint32  x = 0;

  for  (x = 0;  x < points.QueueSize ();  x++)
  {
    Point&  point (points[x]);

    //src[x] = ComplexDouble ((double)point.Row (), (double)point.Col ());

    #if  defined(FFTW_AVAILABLE)
      src[x][0] = (double)point.Row ();
      src[x][1] = (double)point.Col ();
    #else
      src[x].real ((double)point.Row ());
      src[x].imag ((double)point.Col ());
    #endif

    totalRow += point.Row ();
    totalCol += point.Col ();
  }

  //double  centerRow = (double)totalRow / (double)(points.QueueSize ());
  //double  centerCol = (double)totalCol / (double)(points.QueueSize ());

//  for  (x = 0;  x < points.QueueSize ();  x++)
//  {
//    src[x] = ComplexDouble ((double)(src[x].real () - centerRow), (double)(src[x].imag () - centerCol));
//  }
//


  #if  defined(FFTW_AVAILABLE)
    fftw_complex*   destFFTW = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * points.QueueSize());
    fftw_plan       plan;
    plan = fftw_plan_dft_1d (points.QueueSize (), src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);  
#else
    KK_DFT1D_Double::DftComplexType*  destFFTW = new KK_DFT1D_Double::DftComplexType[points.QueueSize()];
    KK_DFT1D_Double  plan(points.QueueSize(), true);
    plan.Transform (src, destFFTW);
  #endif

  vector<ComplexDouble>  dest;

  for  (kkint32  l = 0;  l < points.QueueSize ();  l++)
  {
    #if  defined(FFTW_AVAILABLE)
    dest.push_back (ComplexDouble (destFFTW[l][0] / (double)(points.QueueSize ()), destFFTW[l][1] / (double)(points.QueueSize ())));
    #else
    dest.push_back (ComplexDouble (destFFTW[l].real () / (double)(points.QueueSize ()), destFFTW[l].imag () / (double)(points.QueueSize ())));
    #endif
  }
  delete[]  destFFTW;
  delete[]  src;

  return  dest;
}  /* CreateFourierFromPointList */





PointListPtr  ContourFollower::CreatePointListFromFourier (vector<ComplexDouble>  fourier,
                                                           PointList&             origPointList
                                                          )
{
  kkint32  minRow, maxRow, minCol, maxCol;
  origPointList.BoxCoordinites (minRow, minCol, maxRow, maxCol);

  PointListPtr  points = new PointList (true);

  size_t  numOfEdgePixels = origPointList.size ();

  #if  defined(FFTW_AVAILABLE)
     fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * numOfEdgePixels);
  #else
     KK_DFT1D_Double::DftComplexType*  src = new KK_DFT1D_Double::DftComplexType[numOfEdgePixels];
  #endif

  for  (kkint32  l = 0;  l < (kkint32)fourier.size ();  l++)
  {
    #if  defined(FFTW_AVAILABLE)
    src[l][0] = fourier[l].real ();
    src[l][1] = fourier[l].imag ();
    #else
    src[l].real (fourier[l].real());
    src[l].imag (fourier[l].imag ());
    #endif
  }

  #if  defined(FFTW_AVAILABLE)
    fftw_complex*   destFFTW = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * numOfEdgePixels);
    fftw_plan       plan;
    plan = fftw_plan_dft_1d (numOfEdgePixels, src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);  
  #else
    KK_DFT1D_Double::DftComplexType*  destFFTW = new KK_DFT1D_Double::DftComplexType[numOfEdgePixels];
    KK_DFT1D_Double  plan(numOfEdgePixels, false);
    plan.Transform(src, destFFTW);
  #endif


  kkint32  largestRow  = -1;
  kkint32  largestCol  = -1;
  kkint32  smallestRow = 999999;
  kkint32  smallestCol = 999999;

  for  (kkint32  l = 0;  l < (kkint32)fourier.size ();  l++)
  {

    #if  defined(FFTW_AVAILABLE)
       double  realPart = (double)destFFTW[l][0];
       double  imagPart = (double)destFFTW[l][1];
       ComplexDouble  z (realPart, imagPart);
    #else
       double  realPart = (double)destFFTW[l].real ();
       double  imagPart = (double)destFFTW[l].imag ();
       ComplexDouble  z (realPart, imagPart);
    #endif

    
    //  z = z / (double)(fourier.size ());

    kkint32  row = (kkint32)(z.real () + 0.5);
    if  (row > largestRow)
      largestRow = row;
    if  (row < smallestRow)
      smallestRow = row;

    kkint32  col = (kkint32)(z.imag () + 0.5);
    if  (col > largestCol)
      largestCol = col;
    if  (col < smallestCol)
      smallestCol = col;

    PointPtr p = new Point (row, col);
    points->PushOnBack (p);
  }

  delete[]  src;
  delete[]  destFFTW;

  return  points;
}  /* CreatePointListFromFourier */
