/* Histogram.cpp -- Class that represents a single Histogram.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <float.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "Histogram.h"
#include "ImageIO.h"
#include "KKBaseTypes.h"
#include "Raster.h"
using namespace KKB;



Histogram::Histogram (float    _minValue,
                      kkint32  _numOfBuckets,
                      float    _bucketSize,
                      bool     _wrapArround
                     ):

  bucketSize        (_bucketSize),
  buckets           (NULL),
  equalizedMapTable (NULL),
  minValue          (_minValue),
  numOfBuckets      (_numOfBuckets),
  totalCount        (0.0),
  totalVal          (0.0),
  wrapArround       (_wrapArround)

{
  KKCheck (numOfBuckets > 0, "Histogram, Negative _numOfBuckets: " << numOfBuckets)
  KKCheck (_bucketSize > 0.0f, "Histogram,  _bucketSize: " << _bucketSize << " must be greaterthan zero.")

  range = bucketSize * (float)numOfBuckets;

  buckets      = new float[numOfBuckets];
  bucketTotals = new float[numOfBuckets];

  for  (kkint32 x = 0;  x < numOfBuckets;  x++)
  {
    buckets     [x] = (float)0.0;
    bucketTotals[x] = (float)0.0;
  }
}



Histogram::~Histogram ()
{
  delete  buckets;
  delete  bucketTotals;
  if  (equalizedMapTable)
    delete  equalizedMapTable;
}



kkint32  Histogram::MaxBucketIdx ()  const
{
  kkint32  idx;

  float  maxCount = -1;
  kkint32  maxIdx   = -1;

  for  (idx = 0;  idx < numOfBuckets;  idx++)
  {
    if  (buckets[idx] > maxCount)
    {
      maxCount = buckets[idx];
      maxIdx   = idx;
    }
  }

  return  maxIdx;
}  /* MaxBucketIdx */



float  Histogram::AverageOfMaxBucket ()  const
{
  kkint32 idx = MaxBucketIdx ();

  if  (buckets[idx] <= (float)0.0)
    return  (float)0.0;
  else
    return  bucketTotals[idx] / buckets[idx];

}  /* AverageOfMaxBucket */




float   Histogram::CountOfMaxBucket ()  const
{
  kkint32 idx = MaxBucketIdx ();

  return  buckets[idx];
}  /* CountOfMaxBucket */



float  Histogram::Bucket (kkint32  bucket)  const
{
  KKCheck ((bucket >= 0)  &&  (bucket < numOfBuckets), "Histogram::Bucket,  bucket: " << bucket << " out of range.")
  return  buckets[bucket];
}  /* Bucket */



void  Histogram::Increment (float  val)
{
  KKCheck ((val >= minValue), "Histogram::Increment,  val: " << val << " out of range.")

  kkint32 bucket  = (kkint32) ((val - minValue) / bucketSize);
  if  (bucket >= numOfBuckets)
    bucket = numOfBuckets - 1;
  else if  (bucket < 0)
    bucket = 0;

  buckets[bucket] = buckets[bucket] + (float)1.0;
  bucketTotals[bucket] += val;

  totalCount += (float)1.0;
  totalVal += val;
}  /* Increment */



RasterPtr  Histogram::CreateGraph ()  const
{
  kkint32 bucket        = 0;
  kkint32 col           = 10;
  kkint32 barWidth      = 2;
  kkint32 interBarWidth = 5;
  kkint32 colHeight     = 0;
  float   maxCount      = (float)0.0;
  kkint32 row           = 0;
  kkint32 x             = 0;
  kkint32 y             = 0;

  uchar  borderColor  = 100;
  uchar  colorAxis    = 150;
  uchar  colorBar     = 200;
  uchar  colorBarMax  = 255;
  uchar  colorBarPeak = 255;
  uchar  gridColor    = 180;
  uchar  hashColor    =  70;
  uchar  hashColor2   =  45;

  kkint32  graphWidth  = numOfBuckets * (barWidth + interBarWidth) + 20; //  3 pixels per bar,  +
                                                                         //  3 pixels space     +
                                                                         // 10 pixel Padding on both sides.

  // Set up where Grid lines should go
  bool*  vertGridLines = new bool[numOfBuckets];
  bool*  peakBarLines  = new bool[numOfBuckets];
  for  (x = 0;  x < numOfBuckets;  x++)
  {
    vertGridLines[x] = false;
    peakBarLines [x] = IsBucketAPeak (x, 2);  // Make sure that we are greater than 
                                              // our 2 left and right neighbors
  }

  // Look for the Zero Grid Line should go

  kkint32  middleIdx = 0;
  if  (minValue < 0)
  {
    kkint32  numOfBucketsToZero = (kkint32)(0.5 + fabs (minValue) / bucketSize);

    if  (numOfBucketsToZero < numOfBuckets)
      middleIdx = numOfBucketsToZero;
  }

  x = middleIdx;
  while  (x < numOfBuckets)
  {
    vertGridLines[x] = true;
    x += 100;
  }

  x = middleIdx;
  while  (x > 0)
  {
    vertGridLines[x] = true;
    x -= 100;
  }

  for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    if  (buckets[bucket] > maxCount)
      maxCount = buckets[bucket];
  }

  kkint32  maxColHeight = Min ((kkint32)(maxCount * (float)(barWidth + interBarWidth) + 0.5f), (kkint32)512);
  kkint32  graphHeight = maxColHeight + 20;


  RasterPtr  graph = new Raster (graphHeight, graphWidth);

  graph->DrawLine (              10,              10,               10, graphWidth - 10, borderColor);  // TopLeft -> TopRight

  graph->DrawLine (              10, graphWidth - 10, graphHeight - 10, graphWidth - 10, borderColor);  // TopRight -> BotRight

  graph->DrawLine (graphHeight - 10, graphWidth - 10, graphHeight - 10,              10, borderColor);  // BotRight -> BotLeft

  graph->DrawLine (graphHeight - 10,              10,               10,              10, borderColor);  // BotLeft -> TopLeft

  x = 0;
  row = graphHeight - 10;
  
  // Draw Y Axis
  while  (x < maxColHeight)
  {
    if  ((x % 10) == 0)
    {
      if  ((x % 50) == 0)
      {
        graph->SetPixelValue (row, 4, hashColor2);
        graph->SetPixelValue (row, 5, hashColor2);
        graph->SetPixelValue (row, 6, hashColor2);

        if  ((x % 100) == 0)
        {
          graph->DrawLine (row, 9, row, graphWidth - 11, gridColor);
        }
      }

      graph->SetPixelValue (row, 7, hashColor);
      graph->SetPixelValue (row, 8, hashColor);
      graph->SetPixelValue (row, 9, hashColor);
    }

    graph->SetPixelValue (row, 9, colorAxis);
    row--;
    x++;
  }

  col = 10;

  for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    for  (x = 0;  x < (barWidth + interBarWidth);  x++)
    {
      graph->SetPixelValue (graphHeight - 9, col + x, colorAxis);
    }

    if  ((bucket % 10) == 0)
    {
      // Lets add a Hash Mark
      graph->SetPixelValue (graphHeight - 8, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 7, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 6, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 5, col + 1, hashColor);
    }

    if  (vertGridLines[bucket] == true)
    {
      graph->DrawLine (graphHeight - 10, col + 1, 10, col + 1, gridColor);
    }

    colHeight = (kkint32)ceil ((((float)maxColHeight * buckets[bucket]) / maxCount));

    if  ((colHeight < 1)  &&  (buckets[bucket] > (float)0.0))
    {
      cout << "I want to know about this." << std::endl;
    }

    row = graphHeight - 10;

    uchar  barCodeToUse = colorBar;
    if  (buckets[bucket] >= maxCount)
      barCodeToUse = colorBarMax;

    else if  (peakBarLines[bucket])
      barCodeToUse = colorBarPeak;

    for  (kkint32  counter = 0;  counter < colHeight;  counter++)
    {
      for  (y = 0;  y < barWidth;  y++)
      {
        graph->SetPixelValue (row, col + y, barCodeToUse);
      }

      row--;
    }

    col += (barWidth + interBarWidth);
  }  

  delete[]  peakBarLines;
  delete[]  vertGridLines;

  graph->ReverseImage ();

  return  graph;
}  /* CreateGraph */



RasterPtr  Histogram::CreateGraph (kkint32  barSize)  const
{
  kkint32 bucket        = 0;
  kkint32 col           = 10;
  kkint32 barWidth      = barSize;
  kkint32 interBarWidth = 5;
  kkint32 colHeight     = 0;
  float   maxCount      = (float)0.0;
  kkint32 row           = 0;
  kkint32 x;
  kkint32 y;

  uchar  borderColor  = 100;
  uchar  colorAxis    = 150;
  uchar  colorBar     = 200;
  uchar  colorBarMax  = 255;
  uchar  colorBarPeak = 255;
  uchar  gridColor    = 180;
  uchar  hashColor    =  70;
  uchar  hashColor2   =  45;

  kkint32  graphWidth  = numOfBuckets * (barWidth + interBarWidth) + 20; //  3 pixels per bar,  +
                                                                         //  3 pixels space     +
                                                                         // 10 pixels Padding on both sides.

  // Set up where Grid lines should go
  bool*  vertGridLines = new bool[numOfBuckets];
  bool*  peakBarLines  = new bool[numOfBuckets];
  for  (x = 0;  x < numOfBuckets;  x++)
  {
    vertGridLines[x] = false;
    peakBarLines [x] = IsBucketAPeak (x, 2);  // Make sure that we are greater than 
                                              // our 2 left and right neighbors
  }

  // Look for the Zero Grid Line should go

  kkint32  middleIdx = 0;
  if  (minValue < 0)
  {
    kkint32  numOfBucketsToZero = (kkint32)(0.5 + fabs (minValue) / bucketSize);

    if  (numOfBucketsToZero < numOfBuckets)
      middleIdx = numOfBucketsToZero;
  }

  x = middleIdx;
  while  (x < numOfBuckets)
  {
    vertGridLines[x] = true;
    x += 100;
  }

  x = middleIdx;
  while  (x > 0)
  {
    vertGridLines[x] = true;
    x -= 100;
  }
  

  for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    if  (buckets[bucket] > maxCount)
      maxCount = buckets[bucket];
  }
  
  kkint32  maxColHeight = Min ((kkint32)(maxCount * (float)(barWidth + interBarWidth) + 0.5f), (kkint32)512);
  kkint32  graphHeight = maxColHeight + 20;

  RasterPtr  graph = new Raster (graphHeight, graphWidth);

  graph->DrawLine (              10,              10,               10, graphWidth - 10, borderColor);  // TopLeft  -> TopRight
  graph->DrawLine (              10, graphWidth - 10, graphHeight - 10, graphWidth - 10, borderColor);  // TopRight -> BotRight
  graph->DrawLine (graphHeight - 10, graphWidth - 10, graphHeight - 10,              10, borderColor);  // BotRight -> BotLeft
  graph->DrawLine (graphHeight - 10,              10,               10,              10, borderColor);  // BotLeft  -> TopLeft

  x = 0;
  row = graphHeight - 10;

  // Draw Y Axis
  while  (x < maxColHeight)
  {
    if  ((x % 10) == 0)
    {
      if  ((x % 50) == 0)
      {
        graph->SetPixelValue (row, 4, hashColor2);
        graph->SetPixelValue (row, 5, hashColor2);
        graph->SetPixelValue (row, 6, hashColor2);

        if  ((x % 100) == 0)
        {
          graph->DrawLine (row, 9, row, graphWidth - 11, gridColor);
        }
      }

      graph->SetPixelValue (row, 7, hashColor);
      graph->SetPixelValue (row, 8, hashColor);
      graph->SetPixelValue (row, 9, hashColor);
    }

    graph->SetPixelValue (row, 9, colorAxis);
    row--;
    x++;
  }

  col = 10;

  for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    for  (x = 0;  x < (barWidth + interBarWidth);  x++)
    {
      graph->SetPixelValue (graphHeight - 9, col + x, colorAxis);
    }

    if  ((bucket % 10) == 0)
    {
      // Lets add a Hash Mark
      graph->SetPixelValue (graphHeight - 8, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 7, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 6, col + 1, hashColor);
      graph->SetPixelValue (graphHeight - 5, col + 1, hashColor);
    }

    if  (vertGridLines[bucket] == true)
    {
      graph->DrawLine (graphHeight - 10, col + 1, 10, col + 1, gridColor);
    }

    colHeight = (kkint32)ceil ((((float)maxColHeight * buckets[bucket]) / maxCount));

    if  ((colHeight < 1)  &&  (buckets[bucket] > (float)0.0))
    {
      cout << "I want to know about this." << std::endl;
    }

    row = graphHeight - 10;

    uchar  barCodeToUse = colorBar;
    if  (buckets[bucket] >= maxCount)
      barCodeToUse = colorBarMax;

    else if  (peakBarLines[bucket])
      barCodeToUse = colorBarPeak;

    for  (kkint32  counter = 0;  counter < colHeight;  counter++)
    {
      for  (y = 0;  y < barWidth;  y++)
        graph->SetPixelValue (row, col + y, barCodeToUse);
      row--;
    }

    col += (barWidth + interBarWidth);
  }  

  delete[]  peakBarLines;
  delete[]  vertGridLines;

  graph->ReverseImage ();

  return  graph;
}  /* CreateGraph */



void  Histogram::SaveGraphImage (const KKStr&  fileName)  const
{
  RasterPtr  graphImage = CreateGraph ();
  SaveImage (*graphImage, fileName);
  delete  graphImage;
}  /* SaveGraphImage */



void  Histogram::SaveGraphImage (const KKStr&  fileName,
                                 kkint32       barSize
                                )  const
{
  RasterPtr  graphImage = CreateGraph (barSize);
  SaveImage (*graphImage, fileName);
  delete  graphImage;
}



void  Histogram::Save (KKStr  fileName)  const
{
  ofstream o (fileName.Str ());

  kkint32  bucketIDX;
  
  float  avgVal = totalVal / totalCount;

  o << "MinValue" << "\t" << "\t" << "BucketSize"  << "\t"  << "NumOfBuckets" << "\t" << "TotalCount" << "\t" << "Average"  << std::endl;
  o <<  minValue  << "\t" << "\t" <<  bucketSize   << "\t"  <<  numOfBuckets  << "\t" <<  totalCount  << "\t" <<  avgVal    << std::endl;

  o << std::endl;
  o << std::endl;

  float  avg;
  float  percentage;
  float  bucketVal = minValue + bucketSize;

  o << "BucketValue"  << "\t"
    << "Count"        << "\t"
    << "AvgValue"     << "\t"
    << "%OfTotalCount"
    << std::endl
    << std::endl;

  for  (bucketIDX = 0;  bucketIDX < numOfBuckets;  bucketIDX++)
  {
    o << bucketVal;

    o << "\t" << buckets[bucketIDX];

    if  (buckets[bucketIDX] > (float)0.0)
      avg = bucketTotals[bucketIDX] / buckets[bucketIDX];
    else
      avg = 0.0;
    o << "\t" << avg;


    percentage = (float)100.0 * buckets[bucketIDX] / totalCount;
    o << "\t" << percentage << "%";

    bucketVal += bucketSize;
    o << std::endl;
  }

  o << std::endl;
  o << std::endl;

  o.close ();
}  /* Save */



HistogramPtr  Histogram::Smooth (kkint32 smoothWidth)
{
  kkint32  bucket;
  kkint32  endBuckOffset;
  kkint32  startBuckOffset;
  float sumOfbuckets      = (float)0.0;
  float sumOfBucketTotals = (float)0.0;

  HistogramPtr hist = new Histogram (minValue, 
                                     numOfBuckets, 
                                     bucketSize,
                                     wrapArround
                                    );


  if  (wrapArround)
  {
    startBuckOffset = numOfBuckets - (smoothWidth / 2);
    endBuckOffset   = startBuckOffset + smoothWidth - 1 - numOfBuckets;


    // Lets Build Initial Totals

    bucket = startBuckOffset;

    while  (true)
    {
      sumOfbuckets      += buckets      [bucket];
      sumOfBucketTotals += bucketTotals [bucket];

      if  (bucket == endBuckOffset)
        break;

      bucket++;
      if  (bucket >= numOfBuckets)
        bucket = bucket - numOfBuckets;
    }

    for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
    {
      hist->buckets      [bucket] = sumOfbuckets      / (float)smoothWidth;
      hist->bucketTotals [bucket] = sumOfBucketTotals / (float)smoothWidth;

      sumOfbuckets      -= buckets      [startBuckOffset];
      sumOfBucketTotals -= bucketTotals [startBuckOffset];

      startBuckOffset++;
      if  (startBuckOffset >= numOfBuckets)
        startBuckOffset = startBuckOffset - numOfBuckets;

      endBuckOffset++;
      if  (endBuckOffset >= numOfBuckets)
        endBuckOffset = endBuckOffset - numOfBuckets;

      sumOfbuckets      += buckets      [endBuckOffset];
      sumOfBucketTotals += bucketTotals [endBuckOffset];
    }
  }  /* End of WrapArround Version */

  else
  {
    // No WrapArround.

    startBuckOffset = -(smoothWidth / 2);
    endBuckOffset   = startBuckOffset + smoothWidth - 1;

    kkint32  spreadSize = endBuckOffset + 1;

    // Lets Build Initial Totals
    for  (bucket = 0;  bucket <= endBuckOffset;  bucket++)
    {
      sumOfbuckets      += buckets      [bucket];
      sumOfBucketTotals += bucketTotals [bucket];
    }


    for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
    {
      hist->buckets [bucket] = sumOfbuckets / (float)spreadSize;

      if  (sumOfbuckets > (float)0.0)
      {
        hist->bucketTotals [bucket] = hist->buckets [bucket] * sumOfBucketTotals / sumOfbuckets;
      }
      else
      {
        hist->bucketTotals [bucket] = (float)0.0;
      }

      if  (startBuckOffset >= 0)
      {
        sumOfbuckets      = sumOfbuckets      - buckets      [startBuckOffset];
        sumOfBucketTotals = sumOfBucketTotals - bucketTotals [startBuckOffset];
        spreadSize--;
      }

      startBuckOffset++;

      endBuckOffset++;
      if  (endBuckOffset < numOfBuckets)
      {
        sumOfbuckets      = sumOfbuckets      + buckets      [endBuckOffset];
        sumOfBucketTotals = sumOfBucketTotals + bucketTotals [endBuckOffset];
        spreadSize++;
      }
    }
  }

  hist->totalCount = totalCount;
  hist->totalVal   = totalVal;

  return  hist;
}  /*  Smooth */



bool  Histogram::IsBucketAPeak (kkint32  bucket,
                                kkint32  tolerance
                              )  const
{
  kkint32  beforeIDX     = bucket;
  kkint32  afterIDX      = bucket;
  kkint32  lastBeforeIDX = bucket;
  kkint32  lastAfterIDX  = bucket;

  if  (buckets[bucket] == (float)0.0)
    return  false;

  for  (kkint32 x = 1; x <= tolerance;  x++)
  {
    beforeIDX--;
    afterIDX++;

    if  (beforeIDX < 0)
    {
      if  (wrapArround)
        beforeIDX = beforeIDX + numOfBuckets;
    }

    if  (afterIDX < 0)
    {
      if  (wrapArround)
        afterIDX = afterIDX - numOfBuckets;
    }

    if  (beforeIDX >= 0)
    {
      if  (buckets [beforeIDX] > buckets[lastBeforeIDX])
        return  false;
    }

    if  (afterIDX < numOfBuckets)
    {
      if  (buckets [afterIDX] > buckets[lastAfterIDX])
        return  false;
    }

    lastBeforeIDX = beforeIDX;
    lastAfterIDX  = afterIDX;
  }

  return  true;
}  /* IsBucketAPeak */



void  Histogram::CalculatePeaks (kkint32  threshold)
{
  kkint32  bucket;

  peaks.erase (peaks.begin (), peaks.end ());

  for  (bucket = 0;  bucket < numOfBuckets;  bucket++)
  {
    if  (IsBucketAPeak (bucket, threshold))
      peaks.push_back (bucket);
  }
}  /* CalculatePeaks */



kkint32  Histogram::GetPeakBucket (kkint32 peakNum) 
{
  if  (peaks.empty ())
    CalculatePeaks (2);

  if  (peakNum > (kkint32)peaks.size ())
    return -1;

  else
    return  peaks[peakNum];
}



kkint32  Histogram::GetPeakByHighestOrder (kkint32 peakNum)
{
  if  (peaks.empty ())
    CalculatePeaks (2);

  if  (peakNum > (kkint32)peaks.size ())
    return -1;

  VectorInt32 sortedPeaks (peaks);

  for  (size_t x = 1;  x < sortedPeaks.size ();  ++x)
  {
    for (size_t y = x;  y > 0;  --y)
    {
      if  (buckets[sortedPeaks[y - 1]] < buckets[sortedPeaks[y]])
      {
        kkint32 t = sortedPeaks[y];
        sortedPeaks[y] = sortedPeaks[y - 1];
        sortedPeaks[y - 1] = t;
      }
    }
  }

  kkint32  returnIdxNum = sortedPeaks[peakNum];
  
  return   returnIdxNum;
  
}  /* GetPeakByHighestOrder */



float  Histogram::GetPeakAvgByHighestOrder (kkint32 peakNum)
{
  kkint32  x = GetPeakByHighestOrder (peakNum);

  if  (x < 0)
  {
    cout << std::endl;
    cout << "GetPeakAvgByHighestOrder     GetPeakByHighestOrder returned[" << x << "]" << std::endl;
    return  0;
  }

  if  (x >= this->numOfBuckets)
  {
    cout << std::endl;
    cout << "GetPeakAvgByHighestOrder     GetPeakByHighestOrder returned[" << x << "]" << std::endl;
    x = 1;
    return 0;
  }

  if  (buckets[x] <= (float)0.0)
    return 0.0;

  return  (bucketTotals[x] / buckets[x]);
}  /* GetPeakAvgByHighestOrder */



HistogramPtr  Histogram::Equalized ()
{
  float*   histSumTable  = new float[numOfBuckets];
  kkint32  idx;

  HistogramPtr   newHist = new Histogram (minValue, numOfBuckets, bucketSize, wrapArround);

  newHist->equalizedMapTable = new kkint32[numOfBuckets];

  float  accumulatedCount = (float)0.0;
  for  (kkint32 x = 0; x < numOfBuckets; x++)
  {
    accumulatedCount += buckets[x];
    histSumTable[x] = accumulatedCount;
  }
   
  for  (kkint32 x = 0;  x < numOfBuckets;  x++)
  {
    idx = (kkint32)(0.5f + (((float)(histSumTable[x] * (float)(numOfBuckets - 1)) / totalCount)));
    if  (idx >= numOfBuckets)
      idx = numOfBuckets - 1;

    newHist->equalizedMapTable[x] = idx;
    newHist->buckets[idx]         += buckets[x];
    newHist->bucketTotals[idx]    += bucketTotals[x];
  }
  
  newHist->totalCount = totalCount;
  newHist->totalVal   = totalVal;

  delete[]  histSumTable;

  return  newHist;
} /* Equalize */



float   Histogram::AverageOfMaxBucketInRange (kkint32  minBucket,
                                              kkint32  maxBucket
                                             )  const
{
  KKCheck ((minBucket >= 0)  &&  (minBucket < numOfBuckets)  &&  (maxBucket >= 0)  &&  (maxBucket < numOfBuckets)  &&  (minBucket <= maxBucket),
           "Histogram::AverageOfMaxBucketInRange  Parameters out or range numOfBuckets: " << numOfBuckets << "  minBucket: " << minBucket << " maxBucket: " << maxBucket)

  kkint32 maxBucketIDX = minBucket;
  float maxBucketVal = buckets[minBucket];

  for  (kkint32 b = minBucket;  b <= maxBucket;  b++)
  {
    if  (buckets[b] > maxBucketVal)
    {
      maxBucketIDX = b;
      maxBucketVal = buckets[b];
    }
  }

  if  (maxBucketVal > (float)0.0)
    return  bucketTotals[maxBucketIDX] / buckets[maxBucketIDX];
  else
    return (float)0.0;
}  /* AverageOfMaxBucketInRange */



float   Histogram::AverageOfMinBucketInRange (kkint32  firstBucket,
                                              kkint32  lastBucket
                                             )  const
{
  KKCheck ((firstBucket >= 0)  &&  (firstBucket < numOfBuckets)  &&  (lastBucket >= 0)  &&  (lastBucket < numOfBuckets)  &&  (firstBucket <= lastBucket),
           "Histogram::AverageOfMinBucketInRange  Parameters out or range numOfBuckets: " << numOfBuckets <<
           "  minBucket: " << firstBucket << " maxBucket: " << lastBucket)

  kkint32  minBucketIDX = firstBucket;
  float minBucketVal = buckets[firstBucket];

  for  (kkint32 b = firstBucket;  b <= lastBucket;  b++)
  {
    if  (buckets[b] <= minBucketVal)
    {
      minBucketIDX = b;
      minBucketVal = buckets[b];
    }
  }

  if  (minBucketVal > 0)
    return  bucketTotals[minBucketIDX] / buckets[minBucketIDX];
  else
    return (float)0.0;
}  /* AverageOfMaxBucketInRange */



float   Histogram::AverageOfMaxBucketExcludingRange (kkint32  minBucket,
                                                     kkint32  maxBucket
                                                    )  const
{
  KKCheck ((minBucket >= 0)  &&  (minBucket < numOfBuckets)  &&  (maxBucket >= 0)  &&  (maxBucket < numOfBuckets)  &&  (minBucket <= maxBucket),
           "Histogram::AverageOfMaxBucketExcludingRange  Parameters out or range numOfBuckets: " << numOfBuckets
           << "  minBucket: " << minBucket << " maxBucket: " << maxBucket)

  kkint32  maxBucketIDX = -1;
  float maxBucketVal = 0;

  for  (kkint32 b = 0;  b < numOfBuckets;  b++)
  {
    if  ((b < minBucket)  ||  (b > maxBucket))
    {
      if  (buckets[b] > maxBucketVal)
      {
        maxBucketIDX = b;
        maxBucketVal = buckets[b];
      }
    }
  }

  if  (maxBucketIDX < 0)
    return 0;

  if  (maxBucketVal > (float)0.0)
    return  bucketTotals[maxBucketIDX] / buckets[maxBucketIDX];
  else
    return (float)0.0;
}  /* AverageOfMaxBucketExcludingRange */



kkint32  Histogram::AreaInRange (kkint32  minBucket,
                                 kkint32  maxBucket
                                )  const
{
  KKCheck ((minBucket >= 0)  &&  (minBucket < numOfBuckets)  &&  (maxBucket >= 0)  &&  (maxBucket < numOfBuckets)  &&  (minBucket <= maxBucket),
           "Histogram::AreaInRange  Parameters out or range numOfBuckets: " << numOfBuckets << " minBucket: " << minBucket << " maxBucket: " << maxBucket)

  float  area = (float)0.0;
  for  (kkint32 b = minBucket;  b <= maxBucket;  b++)
  {
    area += buckets[b];
  }

  return  (kkint32)(area + (float)0.5);
}   /* AreaInRange */



float  Histogram::AreaInRangePercent (kkint32  minBucket,
                                      kkint32  maxBucket
                                     )  const
{
  if  (totalCount <= (float)0.0)
    return (float)0.0;

  return  (float)100.0 * (float)((float)AreaInRange (minBucket, maxBucket) / totalCount);
}



void  Histogram::PrintTable (ostream&  o)
{
  kkint32  x;
  float  l = minValue;
  
  o << l;
  for (x = 1;  x < numOfBuckets; x++)
  {
    l += bucketSize;
    o << "\t" << l;
  }
  o << std::endl;
  
  o << buckets[0];
  for (x = 1;  x < numOfBuckets; x++)
  {
    o << "\t" << buckets[x];
  }
  o << std::endl;
}  /* PrintTable */



void  Histogram::GetStats (float&  min,       // smallest amt in a bucket.
                           float&  max,       // largest amt in a bucket.
                           float&  mean,      // average number in each bucket.
                           float&  variance   // variance.
                          )
{
  min      = FLT_MAX;
  max      = FLT_MIN;
  mean     =  totalCount / (float)numOfBuckets;
  variance = 0.0f;

  float  deltaSquareTotal = 0.0f;
  for  (kkint32 idx = 0;  idx < numOfBuckets;  idx++)
  {
    if  (buckets[idx] < min)  min = buckets[idx];
    if  (buckets[idx] > max)  max = buckets[idx];

    float  delta = buckets[idx] - mean;
    float  deltaSquare = delta * delta;
    deltaSquareTotal += deltaSquare;
  }
  variance = deltaSquareTotal / (float)numOfBuckets;
}  /* GetStats */
