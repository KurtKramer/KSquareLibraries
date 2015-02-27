/* ContourFollower.cpp -- Used to find the contour of image in a Raster object.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"
#include  <stdlib.h>
#include  <memory>
#include  <math.h>
#include  <complex>
#include  <fstream>
#include  <iostream>
#include  <string>
#include  <vector>
#include  "MemoryDebug.h"
using namespace std;

//#include  <fftw3.h>



#include "ContourFollower.h"
#include "kku_fftw.h"
#include "Raster.h"
#include "Str.h"
using namespace KKU;



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



uchar  ContourFollower::PixelValue (int32 row,
                                    int32 col
                                   )
{
  if  ((row < 0)  ||  (row >= height))  return 0;
  if  ((col < 0)  ||  (col >= width))   return 0;
  return  rows[row][col];
}


int32  ContourFollower::PixelCountIn9PixelNeighborhood (int32  row, 
                                                      int32  col
                                                     )
{
  int32  count = 0;
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



void  ContourFollower::GetFirstPixel (int32&  startRow,
                                      int32&  startCol
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
    int32  row;
    int32  col;

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



void  ContourFollower::GetNextPixel (int32&  nextRow,
                                     int32&  nextCol
                                    )
{
  fromDir = lastDir + 4;

  if  (fromDir > 7)
    fromDir = fromDir - 8;

  bool  nextPixelFound = false;

  int32  nextDir = fromDir + 2;

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
                        int32           index
                       )
  {
    float  mag = 0.0f;
    mag = (float)sqrt (dest[index][0] * dest[index][0] + dest[index][1] * dest[index][1]);
    return  mag;
  }  /* CalcMagnitude */
#else
  float  CalcMagnitude (KK_DFT1D_Float::DftComplexType* dest,
                        int32                           index
                       )
  {
    double rp = (float)dest[index].real ();
    double ip = (float)dest[index].imag ();
    return (float)sqrt (rp * rp + ip * ip);
  }  /* CalcMagnitude */
  
#endif






int32  ContourFollower::FollowContour (float*  countourFreq,
                                       float   fourierDescriptors[15],
                                       int32   totalPixels,
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


  int32  numOfBuckets = 5;

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  absoluteMaximumEdgePixels = totalPixels * 3;

  int32  maxNumOfBorderPoints = 3 * (height + width);
  int32  numOfBorderPixels = 0;

  int32 x;

  successful = true;

  int32  totalRow = 0;
  int32  totalCol = 0;


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
      int32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;

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

      int32  x;
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

  int32  numOfedgePixels = numOfBorderPixels;

  int32*  count = new int32 [numOfBuckets];

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



int32  ContourFollower::FollowContour2 (float*  countourFreq,
                                        bool&   successful
                                       )
{
  // A diff Version of FollowContour

  // startRow and startCol is assumed to come from the left (6)

  int32  numOfBuckets = 16;

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  maxNumOfBorderPoints = 3 * (height + width);
  int32  numOfBorderPixels = 0;

  int32 x;

  successful = true;

  int32  totalRow = 0;
  int32  totalCol = 0;


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
      int32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;

      #if  defined(FFTW_AVAILABLE)
        fftwf_complex*  newSrc = (fftwf_complex*)fftwf_malloc (sizeof (fftwf_complex) * newMaxNumOfAngles);
      #else
        KK_DFT1D_Float::DftComplexType*  newSrc = new KK_DFT1D_Float::DftComplexType[newMaxNumOfAngles];
      #endif

      int32  x;
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

  int32  numOfedgePixels = numOfBorderPixels;

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

  int32*  count = new int32 [numOfBuckets];

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

  int32  region = 0;

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

    if  (Fabs (deltaX) < r0)
      continue;

    if  (deltaX < 0)
    {
      // We are on the Left half.
      deltaX = Fabs (deltaX);

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

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

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




int32  ContourFollower::CreateFourierDescriptorBySampling (int32    numOfBuckets,
                                                           float*  countourFreq,
                                                           bool&    successful
                                                          )
{
  // startRow and startCol is assumed to come from the left (6)
  // int32  numOfBuckets = 8;
  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  numOfBorderPixels = 0;

  int32 x;

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

  int32  totalRow = 0;
  int32  totalCol = 0;

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    int32  borderPixelIdx = (int32)(((float)x * (float)numOfBorderPixels) /  (float)numOfBuckets);

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
                                                          int32   numOfBuckets,
                                                          int32*  buckets,
                                                          float&  minDistance,
                                                          float&  maxDistance,
                                                          int32&  numOfEdgePixels
                                                         )
{
  PointListPtr  points = GenerateContourList ();

  numOfEdgePixels = points->QueueSize ();
  
  int32  x;

  minDistance = FFLOAT_MAX;
  maxDistance = FFLOAT_MIN;

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
      int32 bucketIDX = (int32)((distances[x] - minDistance) / bucketSize);
      buckets[bucketIDX]++;
    }
  }

  delete  points;     points    = NULL;
  delete  distances;  distances = NULL;

  return;
}  /* HistogramDistanceFromAPoint */


