/* FlatFieldCorrectionMono.cpp --
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

#include "FlatFieldCorrection.h"
#include "FlatFieldCorrectionMono.h"
using  namespace  KKLSC;



FlatFieldCorrectionMono::FlatFieldCorrectionMono (kkuint32      _numSampleLines,
                                                  kkuint32      _lineWidth,
                                                  const uchar*  _compensationTable,
                                                  kkuint32      _startCol
                                                 ):
    FlatFieldCorrection (_numSampleLines, _lineWidth, _compensationTable, _startCol, false),
    highPoint            (NULL),
    highPointLastSeen    (NULL),
    history              (NULL),
    totalLine            (NULL)
{
  KKCheck (_numSampleLines > 0,  "_numSampleLines: " + StrFromUint32 (_numSampleLines) + " > 0")
  highPoint         = new uchar[lineWidth];
  highPointLastSeen = new kkuint32[lineWidth];
  for  (kkuint32 x = 0; x < lineWidth;  ++x)
  {
    highPoint        [x] = 255u;
    highPointLastSeen[x] = 0;
  }

  history = new uchar*[numSampleLines];
  for  (kkuint32 x = 0;  x < numSampleLines;  ++x)
  {
    history[x] = new uchar[lineWidth];
    for  (kkuint32 y = 0;  y < lineWidth;  ++y)
      history[x][y] = (uchar)255;
  }

  totalLine = new kkint32[lineWidth];

  lookUpTable = new uchar*[lineWidth];
  for  (kkuint32 x = 0;  x < lineWidth;  ++x)
  {
    lookUpTable[x] = new uchar[256u];
    for  (kkuint32 y = 0;  y < 256u;  ++y)
      lookUpTable[x][y] = (uchar)y;
  }

  for  (kkuint32 col = 0;  col < lineWidth;  ++col)
    ReComputeLookUpForColumn (col);
}



FlatFieldCorrectionMono::~FlatFieldCorrectionMono ()
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

  for (kkuint32 x = 0;  x < lineWidth;  ++x)
  {
    delete  lookUpTable[x];
    lookUpTable[x] = NULL;
  }
  
  delete  lookUpTable;  lookUpTable = NULL;
  delete  totalLine;    totalLine   = NULL;
}



void  FlatFieldCorrectionMono::CompensationTable (const uchar*  _compensationTable)
{
  compensationTable = _compensationTable;
  for  (kkuint32  x = 0;  x < lineWidth;  ++x)
    ReComputeLookUpForColumn (x);
}



void  FlatFieldCorrectionMono::AddSampleLine (const uchar*  sampleLine)
{
  ++lastHistoryIdxAdded;
  if  (lastHistoryIdxAdded >= numSampleLines)
    lastHistoryIdxAdded = 0;

  uchar*  historyLine = history[lastHistoryIdxAdded];
  for  (kkuint32 x = 0;  x < lineWidth;  ++x)
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



void  FlatFieldCorrectionMono::ReComputeLookUpForColumn (kkuint32 col)
{
  if  (enabled)
  {
    highPointLastSeen[col] = 1u;

    kkuint32 historyIdx = lastHistoryIdxAdded;

    kkint32 age = 1;
    highPoint[col] = 0;

    kkint32 hp0 = 0, hp0Age = 0;
    kkint32 hp1 = 0, hp1Age = 0;
    kkint32 hp2 = 0, hp2Age = 0;

    while  (true)
    {
      uchar  hv = history[historyIdx][col];
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


    //highPoint[col] = (uchar)(((uint16)hp0 + (uint16)hp1 + (uint16)hp2) / (uint16)3);
    //highPoint[col] = hp0;
    //highPointLastSeen[col] = hp0Age;

    highPoint[col] = (uchar)hp2;
    highPointLastSeen[col] = hp2Age;


    // We now know the high point value;  lets scale the look-up-table for this column now.
    kkuint32 hp = highPoint[col];

    if  (hp < 28u)
    {
      hp = 0;
      kkint32  newPixelValue = 255;
      if  (compensationTable)
        newPixelValue = compensationTable[newPixelValue];
      newPixelValue = 255 - newPixelValue;
      for  (kkuint32 row = 0;  row < 256u;  ++row)
        lookUpTable[col][row] = (uchar)newPixelValue;
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

        //lookUpTable[col][row] = (uchar)(255 - newPixelValue);
        if  (compensationTable)
          newPixelValue = compensationTable[newPixelValue];

        newPixelValue = 255 - newPixelValue;

        lookUpTable[col][row] = (uchar)newPixelValue;
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

      lookUpTable[col][row] = (uchar)newPixelValue;
    }
  }
}  /* ReComputeLookUpForColumn */



void  FlatFieldCorrectionMono::ApplyFlatFieldCorrection (uchar*  scanLine)
{
  for  (kkuint32 col = startCol;  col < lineWidth;  ++col)
    scanLine[col] = lookUpTable[col][scanLine[col]];
}  /* ApplyFlatFieldCorrection */



void  FlatFieldCorrectionMono::ApplyFlatFieldCorrection (uchar*  srcScanLine,  uchar*  destScanLine)
{
  if  (enabled)
  {
    for  (kkuint32 col = 0;  col < lineWidth;  col++)
      destScanLine[col] = lookUpTable[col][srcScanLine[col]];
  }
  else
  {
    for  (kkuint32 col = 0;  col < lineWidth;  col++)
      destScanLine[col] = srcScanLine[col];
  }
}  /* ApplyFlatFieldCorrection */



VectorUcharPtr  FlatFieldCorrectionMono::CameraHighPoints ()  const
{
  vector<uchar>*  results = new vector<uchar> ();
  for  (kkuint32 x = 0;  x < lineWidth;  x++)
    results->push_back (highPoint[x]);
  return  results;
}  /* CameraHighPoints */



VectorUcharPtr  FlatFieldCorrectionMono::CameraHighPointsFromLastNSampleLines (kkuint32 n)  const
{
  n = Min (n, numSampleLines);

  vector<uchar>*  highPoints = new vector<uchar>(lineWidth, 0);

  kkint32  row = lastHistoryIdxAdded;
  for  (kkuint32  x = 0;  x < n;  ++x)
  {
    if  (row < 0)
      row = numSampleLines - 1;
    uchar*  sampleRow = history[row];

    for  (kkuint32 col = 0;  col < lineWidth;  ++col)
    {
      if  (sampleRow[col] > (*highPoints)[col])
        (*highPoints)[col] = sampleRow[col];
    }
    --row;
  }
  return  highPoints;
}  /* CameraHighPointsFromLastNSampleLines */
