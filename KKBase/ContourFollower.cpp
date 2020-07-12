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
//#else
//#  include  "kku_fftw.h"
#endif
#include "kku_fftw.h"


#include "ContourFollower.h"
#include "Point.h"
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
  backgroundPixelTH (_raster.BackgroundPixelTH ()),
  curCol            (-1),
  curRow            (-1),
  fromDir           (),
  height            (_raster.Height ()),
  lastDir           (0),
  log               (_log),
  raster            (_raster),
  rows              (_raster.Rows   ()),
  width             (_raster.Width  ())
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
  if  (PixelValue (row - 1, col - 1) > backgroundPixelTH)  ++count;
  if  (PixelValue (row - 1, col    ) > backgroundPixelTH)  ++count;
  if  (PixelValue (row - 1, col + 1) > backgroundPixelTH)  ++count;
  if  (PixelValue (row    , col - 1) > backgroundPixelTH)  ++count;
  if  (PixelValue (row    , col    ) > backgroundPixelTH)  ++count;
  if  (PixelValue (row    , col + 1) > backgroundPixelTH)  ++count;
  if  (PixelValue (row + 1, col - 1) > backgroundPixelTH)  ++count;
  if  (PixelValue (row + 1, col    ) > backgroundPixelTH)  ++count;
  if  (PixelValue (row + 1, col + 1) > backgroundPixelTH)  ++count;
  return  count;
}



Point  ContourFollower::GetFirstPixel ()
{
  kkint32 row, col;
  GetFirstPixel (row, col);
  return Point (row, col);
}



void  ContourFollower::GetFirstPixel (kkint32&  startRow,
                                      kkint32&  startCol
                                     )
{
  lastDir = 1;
  fromDir = 0;

  startRow = height / 2;
  startCol = 0;

  while  (startCol < width)
  {
    if  ((rows[startRow][startCol] > backgroundPixelTH)  &&  (PixelCountIn9PixelNeighborhood (startRow, startCol) > 1))
      break;
    startCol++;
  }

  if  (startCol >= width)
  {
    // We did not find a Pixel in the Middle Row(height / 2) so now we will
    // scan the image row, by row, Left to Right until we find an occupied 
    // pixel.

    bool  found = false;

    for  (kkint32 row = 0;  ((row < height)  &&  (!found));  ++row)
    {
      for  (kkint32 col = 0;  ((col < width)  &&  (!found));  ++col)
      {
        if  ((rows[row][col] > backgroundPixelTH)  &&  (PixelCountIn9PixelNeighborhood (row, col) > 1))
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



Point  ContourFollower::GetNextPixel ()
{
  kkint32  row, col;
  GetNextPixel (row, col);
  return Point (row, col);
}



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
    else if  (rows[nextRow][nextCol] > backgroundPixelTH)
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
    double rp = todouble (dest[index].real ());
    double ip = todouble (dest[index].imag ());
    return tofloat (sqrt (rp * rp + ip * ip));
  }  /* CalcMagnitude */
  
#endif



kkint32  ContourFollower::FollowContour (float*  countourFreq,
                                         float   fourierDescriptors[15],
                                         kkint32 totalPixels,
                                         bool&   successful
                                        )
{
  kkint32  numOfBuckets = 5;

  kkint32  absoluteMaximumEdgePixels = totalPixels * 3;

  kkint32  maxNumOfBorderPoints = 3 * (height + width);
  kkint32  numOfBorderPixels = 0;

  successful = true;

  Point total = {0, 0};

  #if  defined(FFTW_AVAILABLE)
  fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
  #else
  KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[maxNumOfBorderPoints];
  #endif

  Point start = GetFirstPixel ();
  if  ((start.Row () < 0)  ||  (start.Col () < 0)  ||  (PixelCountIn9PixelNeighborhood (start.Row (), start.Col ()) < 2))
  {
    std::cerr << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  Point scnd = GetNextPixel ();
  Point next = scnd;
  Point last = start;

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

    last = next;
    next = GetNextPixel ();
    if  ((next == scnd)   &&  (last == start))
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

      for  (kkint32 x = 0;  x < maxNumOfBorderPoints;  ++x)
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
      src[numOfBorderPixels][0] = next.RowF ();
      src[numOfBorderPixels][1] = next.ColF ();
    #else
      src[numOfBorderPixels].real (next.RowF ());
      src[numOfBorderPixels].imag (next.ColF ());
    #endif

    total += next;

    ++numOfBorderPixels;
  }

  float  centerRow = total.RowF () / tofloat (numOfBorderPixels);
  float  centerCol = total.ColF () / tofloat (numOfBorderPixels);

  float  totalRe = 0.0f;
  float  totalIm = 0.0f;

  for  (kkint32  x = 0;  x < numOfBorderPixels;  ++x)
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

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
  {
    countourFreq[x] = 0.0f;
    count[x] = 0;
  }

  // Original Region Areas,  as reflected in ICPR paper
  float  middle = tofloat (numOfBorderPixels) / 2.0f;
  float  r1 = middle /  2.0f;
  float  r2 = middle *  3.0f  / 4.0f;
  float  r3 = middle *  7.0f  / 8.0f;
  float  r4 = middle * 15.0f  / 16.0f;

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

  for  (kkint32 x = 1; x < (numOfBorderPixels - 1); x++)
  {
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im));
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im) + 1.0);
 
    mag = CalcMagnitude (dest, x);

    deltaX = fabs (tofloat (x) - middle);

    if  (deltaX < r1)
    {
      countourFreq[0] = countourFreq[0] + mag;
      ++count[0];
    }

    else if  (deltaX < r2)
    {
      countourFreq[1] = countourFreq[1] + mag;
      ++count[1];
    }

    else if  (deltaX < r3)
    {
      countourFreq[2] = countourFreq[2] + mag;
      ++count[2];
    }

    else if  (deltaX < r4)
    {
      countourFreq[3] = countourFreq[3] + mag;
      ++count[3];
    }

    else
    {
      countourFreq[4] = countourFreq[4] + mag;
      ++count[4];
    }
  }

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / tofloat (count[x]);
    }
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);   src   = NULL;
    fftwf_free (dest);  dest  = NULL;
  #else
    delete  src;  src  = NULL;
    delete  dest; dest = NULL;
  #endif
  delete[]  count;      count = NULL;

  return  numOfedgePixels;
}  /* FollowContour */



kkint32  ContourFollower::FollowContour2 (float*  countourFreq,
                                          bool&   successful
                                         )
{
  // A different Version of FollowContour

  // startRow and startCol is assumed to come from the left (6)

  kkint32  numOfBuckets = 16;

  kkint32  maxNumOfBorderPoints = 3 * (height + width);
  kkint32  numOfBorderPixels = 0;

  successful = true;

  #if  defined(FFTW_AVAILABLE)
  fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * maxNumOfBorderPoints);
  #else
  KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[maxNumOfBorderPoints];
  #endif

  auto start = GetFirstPixel ();
  if  ((start.Row () < 0)  ||  (start.Col () < 0)  ||  (PixelCountIn9PixelNeighborhood (start.Row (), start.Col ()) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  Point scnd = GetNextPixel ();
  Point next = scnd;
  Point last = start;
  Point total = {0, 0};

  while  (true)  
  {
    last = next;

    next = GetNextPixel ();

    if  ((next == scnd)   &&  (last == start))
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

      for  (kkint32 x = 0; x < maxNumOfBorderPoints; x++)
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
      src[numOfBorderPixels][0] = next.RowF ();
      src[numOfBorderPixels][1] = next.ColF ();
    #else
      src[numOfBorderPixels].real (next.RowF ());
      src[numOfBorderPixels].imag (next.ColF ());
    #endif

    total += next;

    numOfBorderPixels++;
  }

  float  centerRow = total.RowF () / tofloat (numOfBorderPixels);
  float  centerCol = total.ColF () / tofloat (numOfBorderPixels);

  float  totalRe = 0.0f;
  float  totalIm = 0.0f;

  for  (kkint32  x = 0;  x < numOfBorderPixels;  ++x)
  {
    #if  defined(FFTW_AVAILABLE)
      src[x][0] = src[x][0] - centerRow;
      src[x][1] = src[x][1] - centerCol;
      totalRe+= (float)src[x][0];
      totalIm+= (float)src[x][1];
    #else
      src[x].real (src[x].real () - centerRow);
      src[x].imag (src[x].imag () - centerCol);
      totalRe+= src[x].real ();
      totalIm+= src[x].imag ();
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
  for  (kkint32  x = 0;  x < numOfBuckets;  ++x)
  {
    countourFreq[x] = 0.0f;
    count[x] = 0;
  }

  float  middle = tofloat (numOfBorderPixels) / 2.0f;

  float  r0 = middle * ( 8.0f  /  16.0f);
  float  r1 = middle * (10.0f  /  16.0f);
  float  r2 = middle * (12.0f  /  16.0f);
  float  r3 = middle * (13.0f  /  16.0f);

  kkint32  region = 0;

  for  (kkint32 x = 1; x < numOfBorderPixels;  ++x)
  {
    #if  defined(FFTW_AVAILABLE)
      float mag = (float)sqrt (dest[x][0] * dest[x][0] + dest[x][1] * dest[x][1]);
    #else
      float mag = sqrt (dest[x].real () * dest[x].real () + dest[x].imag () * dest[x].imag ());
    #endif

    float deltaX = tofloat (x) - middle;

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

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0f;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / tofloat (count[x]);
    }
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_free (src);   src   = NULL;
    fftwf_free (dest);  dest  = NULL;
  #else
    delete[] src;   src  = nullptr;
    delete[] dest;  dest = nullptr;
  #endif
  
  delete[]  count;
  count = nullptr;

  return  numOfedgePixels;
}  /* FollowContour2 */



PointListPtr  ContourFollower::GenerateContourList ()
{
  // startRow and startCol is assumed to come from the left (6)
  PointListPtr  points = new PointList (true);

  Point start = GetFirstPixel ();
  if  ((start.Row () < 0)  ||  (start.Col () < 0)  ||  (PixelCountIn9PixelNeighborhood (start.Row (), start.Col ()) < 2))
  {
    delete  points;
    points = nullptr;
    std::cerr << "Very Bad Starting Point" << std::endl;
    return  nullptr;
  }

  Point scnd = GetNextPixel ();
  Point next = scnd;
  Point last = start;

  while  (true)  
  {
    last = next;
    next = GetNextPixel ();
    if  ((next == scnd)   &&  (last == start))
    {
      break;
    }
    points->PushOnBack (new Point (next));
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

  if  (fourierMask)
  {
    for  (kkint32  x = 0;  x < curMaskSize;  ++x)
    {
      delete  fourierMask[x];
      fourierMask[x] = nullptr;
    }

    delete[]  fourierMask;
    fourierMask = nullptr;
  }

  fourierMask = new ComplexDouble*[size];
  for  (kkint32 x = 0;  x < size;  ++x)
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

  for  (kkint32 m = 0;  m < size;  ++m)
  {
    complex<double>  mc (m, 0);

    for  (kkint32 k = 0;  k < size;  ++k)
    {
      complex<double>  kc (k, 0);

      // double  xxx = 2 * 3.14159265359 * (double)k * (double)m / (double)size;

      // fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);
      fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);

      double  exponetPart = 2.0 * 3.14159265359 * todouble (k) * todouble (m) / todouble (size);
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
  
  if  (revFourierMask)
  {
    for  (kkint32 x = 0;  x < curRevMaskSize;  ++x)
    {
      delete  revFourierMask[x];
      revFourierMask[x] = nullptr;
    }

    delete[]  revFourierMask;
    revFourierMask = nullptr;
  }

  revFourierMask = new ComplexDouble*[size];
  for  (kkint32 x = 0;  x < size;  ++x)
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

  for  (kkint32 m = 0;  m < size;  ++m)
  {
    complex<double>  mc (m, 0);

    for  (kkint32 k = 0;  k < size;  ++k)
    {
      complex<double>  kc (k, 0);

      revFourierMask[m][k] = exp (PositiveOne * j * Two * Pi * kc * mc / M);

      double  exponetPart = 2.0 * 3.14159265359 * todouble (k) * todouble (m) / todouble (size);
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
                                                             float*   countourFreq,
                                                             bool&    successful
                                                          )
{
  // startRow and startCol is assumed to come from the left (6)
  // kkint32  numOfBuckets = 8;

  kkint32  numOfBorderPixels = 0;

  successful = true;

  PointListPtr  points = new PointList (true);

  Point start = GetFirstPixel ();
  if  ((start.Row () < 0)  ||  (start.Col () < 0)  ||  (PixelCountIn9PixelNeighborhood (start.Row (), start.Col ()) < 2))
  {
    std::cerr << "Vary Bad Starting Point" << std::endl;
    successful = false;
    return  0;
  }
   
  Point scnd = GetNextPixel ();
  Point next = scnd;
  Point last = start;

  while  (true)  
  {
    last = next;
    next = GetNextPixel ();
    if  ((next == scnd)   &&  (last == start))
    {
      break;
    }
 
    points->PushOnBack (new Point (next));
    numOfBorderPixels++;
  }

  #if  defined(FFTW_AVAILABLE)
    fftwf_complex*  src = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * numOfBuckets);
  #else
    KK_DFT1D_Float::DftComplexType*  src = new KK_DFT1D_Float::DftComplexType[numOfBuckets];
  #endif

  Point  total = {0, 0};

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
  {
    size_t  borderPixelIdx = tosize_t ((todouble (x) * todouble (numOfBorderPixels)) /  todouble (numOfBuckets));

    const Point&  point = points->at (borderPixelIdx);  // (*points)[borderPixelIdx];

    #if  defined(FFTW_AVAILABLE)
      src[x][0] = point.RowF ();
      src[x][1] = point.ColF ();
    #else
      src[x].real (point.RowF ());
      src[x].imag (point.ColF ());
    #endif

    total += point;
  }

  delete  points;
  points = NULL;

  float  centerRow = total.RowF () / tofloat (numOfBuckets);
  float  centerCol = total.ColF () / tofloat (numOfBuckets);

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
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

  for  (kkint32 x = 0;  x < numOfBuckets;  ++x)
  {
    #if  defined(FFTW_AVAILABLE)
      float real = dest[x][0];
      float imag = dest[x][1];
    #else
      float real = dest[x].real ();
      float imag = dest[x].imag ();
    #endif

    countourFreq[x] = (sqrt (real * real + imag * imag));
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



void  ContourFollower::HistogramDistanceFromAPointOfEdge (float     pointRow,
                                                          float     pointCol,
                                                          kkuint32  numOfBuckets,
                                                          kkint32*  buckets,
                                                          float&    minDistance,
                                                          float&    maxDistance,
                                                          kkint32&  numOfEdgePixels
                                                         )
{
  PointListPtr  points = GenerateContourList ();

  numOfEdgePixels = toint32_t (points->QueueSize ());
  
  minDistance = FloatMax;
  maxDistance = FloatMin;

  for  (kkuint32 x = 0;  x < numOfBuckets;  x++)
    buckets[x] = 0;

  float*  distances = new float[points->QueueSize ()];

  for  (kkuint32 x = 0;  x < points->QueueSize ();  ++x)
  {
    Point& point = (*points)[x];

    float  deltaCol = point.ColF () - pointCol;
    float  deltaRow = point.RowF () - pointRow;

    float  distance = sqrt (deltaCol * deltaCol  +  deltaRow * deltaRow);

    distances[x] = distance;

    minDistance = Min (minDistance, distance);
    maxDistance = Max (maxDistance, distance);
  }

  float  bucketSize = (maxDistance - minDistance) / tofloat (numOfBuckets);

  if  (bucketSize == 0.0f)
  {
     buckets[numOfBuckets / 2] = toint32_t (points->QueueSize ());
  }
  else
  {
    for  (kkuint32 x = 0;  x < points->QueueSize ();  ++x)
    {
      kkint32 bucketIDX = toint32_t ((distances[x] - minDistance) / bucketSize);
      buckets[bucketIDX]++;
    }
  }

  delete    points;     points    = nullptr;
  delete[]  distances;  distances = nullptr;

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

  Point  total = {0, 0};

  for  (kkuint32 x = 0;  x < points.QueueSize ();  ++x)
  {
    Point&  point (points[x]);

    //src[x] = ComplexDouble ((double)point.Row (), (double)point.Col ());

    #if  defined(FFTW_AVAILABLE)
      src[x][0] = point.RowD ();
      src[x][1] = point.ColD ();
    #else
      src[x].real (point.RowD ());
      src[x].imag (point.ColD ());
    #endif

    total += point;
  }

  #if  defined(FFTW_AVAILABLE)
    fftw_complex*   destFFTW = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * points.QueueSize());
    fftw_plan       plan;
    plan = fftw_plan_dft_1d (points.QueueSize (), src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);  
#else
    KK_DFT1D_Double::DftComplexType*  destFFTW = new KK_DFT1D_Double::DftComplexType[points.QueueSize()];
    KK_DFT1D_Double  plan (toint32_t (points.QueueSize ()), true);
    plan.Transform (src, destFFTW);
  #endif

  vector<ComplexDouble>  dest;

  for  (kkuint32   l = 0;  l < points.QueueSize ();  ++l)
  {
    #if  defined(FFTW_AVAILABLE)
    dest.push_back (ComplexDouble (destFFTW[l][0] / todouble (points.QueueSize ()), destFFTW[l][1] / todouble (points.QueueSize ())));
    #else
    dest.push_back (ComplexDouble (destFFTW[l].real () / todouble (points.QueueSize ()), destFFTW[l].imag () / todouble (points.QueueSize ())));
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

  for  (size_t  l = 0;  l < fourier.size ();  ++l)
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
    plan = fftw_plan_dft_1d ((int)numOfEdgePixels, src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);  
  #else
    KK_DFT1D_Double::DftComplexType*  destFFTW = new KK_DFT1D_Double::DftComplexType[numOfEdgePixels];
    KK_DFT1D_Double  plan (toint32_t (numOfEdgePixels), false);
    plan.Transform(src, destFFTW);
  #endif

  kkint32  largestRow  = -1;
  kkint32  largestCol  = -1;
  kkint32  smallestRow = 999999;
  kkint32  smallestCol = 999999;

  for  (size_t  l = 0;  l < fourier.size ();  ++l)
  {

    #if  defined(FFTW_AVAILABLE)
       const double  realPart = destFFTW[l][0];
       const double  imagPart = destFFTW[l][1];
       ComplexDouble  z (realPart, imagPart);
    #else
       const double  realPart = destFFTW[l].real ();
       const double  imagPart = destFFTW[l].imag ();
       ComplexDouble  z (realPart, imagPart);
    #endif
    
    kkint32  row = toint32_t (z.real () + 0.5);
    if  (row > largestRow)
      largestRow = row;
    if  (row < smallestRow)
      smallestRow = row;

    kkint32  col = toint32_t (z.imag () + 0.5);
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
