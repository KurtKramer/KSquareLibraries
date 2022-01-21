/* FlatFieldCorrectionColor.cpp --
 * Copyright (C) 2011-2013  Kurt Kramer
 * For conditions of distribution and use, see copyright notice in CounterUnManaged.txt
 */
#include "FirstIncludes.h"

#include <errno.h>
#include <istream>
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include  "KKBaseTypes.h"
#include  "OSservices.h"
using namespace KKB;


#include "FlatFieldCorrectionColor.h"
using  namespace  KKLSC;


/**
 * @param  _numSampleLines  Number sample lines to track for computinmg parameters.
 * @param  _lineWidth  Width of scan line in Pixels.
 * @param  _compensationTable ScannerFile adjustment required as scanner file usually saves the complement of the pixel value.
 * @param  _startCol  Pixel column to start flat fielding from, prev columns will be ignored, typically used for flow meter data.
 */
FlatFieldCorrectionColor::FlatFieldCorrectionColor (kkuint32      _numSampleLines,
                                                    kkuint32      _lineWidth,
                                                    const uchar*  _compensationTable,
                                                    kkuint32      _startCol
                                                   ):
    FlatFieldCorrection (_numSampleLines, _lineWidth, _compensationTable, _startCol, true),
    highPoint            (NULL),
    highPointLastSeen    (NULL),
    history              (NULL),
    lineWidthBytes       (_lineWidth * 3),
    totalLine            (NULL)
{
  KKCheck (_numSampleLines > 0,  "_numSampleLines: " + StrFromUint32 (_numSampleLines) + " > 0")
  highPoint         = new uchar[lineWidthBytes];
  highPointLastSeen = new kkuint32[lineWidthBytes];
  for  (kkuint32 x = 0; x < lineWidthBytes;  ++x)
  {
    highPoint        [x] = 255u;
    highPointLastSeen[x] = 0;
  }

  history = new uchar*[numSampleLines];
  for  (kkuint32 x = 0;  x < numSampleLines;  ++x)
  {
    history[x] = new uchar[lineWidthBytes];
    for  (kkuint32 y = 0;  y < lineWidthBytes;  ++y)
      history[x][y] = (uchar)255;
  }

  totalLine = new kkint32[lineWidthBytes];

  lookUpTable = new uchar*[lineWidthBytes];
  for  (kkuint32 x = 0;  x < lineWidthBytes;  ++x)
  {
    lookUpTable[x] = new uchar[256u];
    for  (kkuint32 y = 0;  y < 256u;  ++y)
      lookUpTable[x][y] = (uchar)y;
  }

  for  (kkuint32 col = 0;  col < lineWidthBytes;  ++col)
    ReComputeLookUpForColumn (col);
}



FlatFieldCorrectionColor::~FlatFieldCorrectionColor ()
{
  delete  highPoint;          highPoint         = NULL;
  delete  highPointLastSeen;  highPointLastSeen = NULL;

  for (kkuint32  x = 0;  x < numSampleLines;  ++x)
  {
    delete  history[x];
    history[x] = NULL;
  }
  delete  history;
  history = NULL;

  for (kkuint32 x = 0;  x < lineWidthBytes;  ++x)
  {
    delete  lookUpTable[x];
    lookUpTable[x] = NULL;
  }
  
  delete  lookUpTable;  lookUpTable = NULL;
  delete  totalLine;    totalLine   = NULL;
}



void  FlatFieldCorrectionColor::CompensationTable (const uchar*  _compensationTable)
{
  compensationTable = _compensationTable;
  for  (kkuint32  x = 0;  x < lineWidthBytes;  ++x)
    ReComputeLookUpForColumn (x);
}



void  FlatFieldCorrectionColor::AddSampleLine (const uchar*  sampleLine)
{
  ++lastHistoryIdxAdded;
  if  (lastHistoryIdxAdded >= numSampleLines)
    lastHistoryIdxAdded = 0;

  uchar*  historyLine = history[lastHistoryIdxAdded];
  for  (kkuint32 x = 0;  x < lineWidthBytes;  ++x)
  {
    historyLine[x] = sampleLine[x];
    if  (sampleLine[x] < highPoint[x])
    {
      highPointLastSeen[x]++;
      if  (highPointLastSeen[x] > numSampleLines)
        ReComputeLookUpForColumn (x);
    }
    else if  ((sampleLine[x] - 5) > highPoint[x])
    {
      ReComputeLookUpForColumn (x);
      highPointLastSeen[x] = 0u;
    }
    else
    {
      highPointLastSeen[x] = 0u;
    }
  }

  ++numSampleLinesAdded;
}  /* AddSampleLine */




/**
 *@details  Unlike the Mono version where the parameter refers to a specific pixel;  this version refres to the specific byte in th escan column, keep inm mind the data is organized as "RGBRGBRGB....RGB" 
 */
void  FlatFieldCorrectionColor::ReComputeLookUpForColumn (kkuint32 byteCol)
{
  if  (enabled)
  {
    highPointLastSeen[byteCol] = 1u;

    kkuint32 historyIdx = lastHistoryIdxAdded;

    kkint32 age = 1;
    highPoint[byteCol] = 0;

    kkint32 hp0 = 0, hp0Age = 0;
    kkint32 hp1 = 0, hp1Age = 0;
    kkint32 hp2 = 0, hp2Age = 0;

    while  (true)
    {
      uchar  hv = history[historyIdx][byteCol];
      if  (hv > hp0)
      {
        hp2 = hp1;  hp2Age = hp1Age;
        hp1 = hp0;  hp1Age = hp0Age;
        hp0 = hv;   hp0Age = age;
      }
      else if  (hv > hp1)
      {
        hp2 = hp1;  hp2Age = hp1Age;
        hp1 = hv;   hp1Age = age;
      }
      else if  (hv > hp2)
      {
        hp2 = hv;   hp2Age = age;
      }

      if  (historyIdx < 1)
        historyIdx = numSampleLines - 1;
      else
        --historyIdx;

      if  (historyIdx == lastHistoryIdxAdded)
        break;

      age++;
    }


    //highPoint[byteCol] = (uchar)(((uint16)hp0 + (uint16)hp1 + (uint16)hp2) / (uint16)3);
    //highPoint[byteCol] = hp0;
    //highPointLastSeen[byteCol] = hp0Age;

    highPoint[byteCol] = (uchar)hp2;
    highPointLastSeen[byteCol] = hp2Age;


    // We now know the high point value;  lets scale the look-up-table for this column now.
    kkuint32 hp = highPoint[byteCol];

    if  (hp < 28u)
    {
      hp = 0;
      kkint32  newPixelValue = 255;
      if  (compensationTable)
        newPixelValue = compensationTable[newPixelValue];
      newPixelValue = 255 - newPixelValue;
      for  (kkuint32 row = 0;  row < 256u;  ++row)
        lookUpTable[byteCol][row] = (uchar)newPixelValue;
    }
    else
    {
      kkint32 newPixelValue = 0;

      for  (kkuint32 row = 0;  row < 256u;  ++row)
      {
        if  (row  <  hp)
          newPixelValue = (kkint32)(0.5f + (255.0f * (float)row) / (float)hp);
        else
          newPixelValue = 255;

        if  ((newPixelValue < 0)  ||  (newPixelValue > 255))
        {
          cout << "Woaaaaa!!!!! it went beyond 255  or less that 0  ["  << newPixelValue << "]." << endl;
          newPixelValue = Max (0, Min (newPixelValue, 255));
        }

        //lookUpTable[byteCol][row] = (uchar)(255 - newPixelValue);
        if  (compensationTable)
          newPixelValue = compensationTable[newPixelValue];

        newPixelValue = 255 - newPixelValue;

        lookUpTable[byteCol][row] = (uchar)newPixelValue;
      }
    }
  }
  else
  {
    for  (kkuint32  row = 0;  row < 256u;  ++row)
    {
      kkint32  newPixelValue = row;
      if  (compensationTable)
        newPixelValue = compensationTable[newPixelValue];

      newPixelValue = 255 - newPixelValue;

      lookUpTable[byteCol][row] = (uchar)newPixelValue;
    }
  }
}  /* ReComputeLookUpForColumn */



void  FlatFieldCorrectionColor::ApplyFlatFieldCorrection (uchar*  scanLine)
{
  for  (kkuint32 col = startCol;  col < lineWidthBytes;  ++col)
    scanLine[col] = lookUpTable[col][scanLine[col]];
}  /* ApplyFlatFieldCorrection */



void  FlatFieldCorrectionColor::ApplyFlatFieldCorrection (uchar*  srcScanLine,  uchar* destScanLine)
{
  if  (enabled)
  {
    for  (kkuint32 col = 0;  col < lineWidthBytes;  ++col)
      destScanLine[col] = lookUpTable[col][srcScanLine[col]];
  }
  else
  {
    for  (kkuint32 col = 0;  col < lineWidthBytes;  ++col)
      destScanLine[col] = srcScanLine[col];
  }
}  /* ApplyFlatFieldCorrection */



VectorUcharPtr  FlatFieldCorrectionColor::CameraHighPoints ()  const
{
  vector<uchar>*  results = new vector<uchar> ();
  for  (kkuint32 x = 0;  x < lineWidthBytes;  x++)
    results->push_back (highPoint[x]);
  return  results;
}  /* CameraHighPoints */



VectorUcharPtr  FlatFieldCorrectionColor::CameraHighPointsFromLastNSampleLines (kkuint32 n)  const
{
  n = Min (n, numSampleLines);

  vector<uchar>*  highPoints = new vector<uchar>(lineWidthBytes, 0);

  kkint32  row = lastHistoryIdxAdded;
  for  (kkuint32  x = 0;  x < n;  ++x)
  {
    if  (row < 0)
      row = numSampleLines - 1;
    uchar*  sampleRow = history[row];

    for  (kkuint32 col = 0;  col < lineWidthBytes;  ++col)
    {
      if  (sampleRow[col] > (*highPoints)[col])
        (*highPoints)[col] = sampleRow[col];
    }
    --row;
  }
  return  highPoints;
}  /* CameraHighPointsFromLastNSampleLines */
