/* FlatFieldCorrection.cpp --
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
using  namespace  KKLSC;



FlatFieldCorrection::FlatFieldCorrection (kkint32       _numSampleLines,
                                          kkint32       _lineWidth,
                                          const uchar*  _compensationTable
                                         ):
    compensationTable    (_compensationTable),
    enabled              (true),
    highPoint            (NULL),
    highPointLastSeen    (NULL),
    history              (NULL),
    lastHistoryIdxAdded  (_numSampleLines - 1),
    lineWidth            (_lineWidth),
    numSampleLines       (_numSampleLines),
    numSampleLinesAdded  (0),
    totalLine            (NULL)
{
  kkint32 x, y;

  highPoint         = new uchar[lineWidth];
  highPointLastSeen = new kkint32[lineWidth];
  for  (x = 0; x < lineWidth;  x++)
  {
    highPoint        [x] = 255;
    highPointLastSeen[x] = 0;
  }

  history = new uchar*[numSampleLines];
  for  (x = 0;  x < numSampleLines;  x++)
  {
    history[x] = new uchar[lineWidth];
    for  (y = 0;  y < lineWidth;  y++)
      history[x][y] = 255;
  }

  totalLine = new kkint32[lineWidth];

  lookUpTable = new uchar*[lineWidth];
  for  (x = 0;  x < lineWidth;  x++)
  {
    lookUpTable[x] = new uchar[256];
    for  (y = 0;  y < 256;  y++)
      lookUpTable[x][y] = (uchar)y;
  }

  for  (kkint32 col = 0;  col < lineWidth;  col++)
    ReComputeLookUpForColumn (col);
}



FlatFieldCorrection::~FlatFieldCorrection ()
{
  kkint32 x;

  delete  highPoint;          highPoint         = NULL;
  delete  highPointLastSeen;  highPointLastSeen = NULL;

  for (x = 0;  x < numSampleLines;  x++)
  {
    delete  history[x];
    history[x] = NULL;
  }
  delete  history;
  history = NULL;

  for (x = 0;  x < lineWidth;  x++)
  {
    delete  lookUpTable[x];
    lookUpTable[x] = NULL;
  }
  
  delete  lookUpTable;  lookUpTable = NULL;
  delete  totalLine;    totalLine   = NULL;
}


void  FlatFieldCorrection::CompensationTable (const uchar*  _compensationTable)
{
  compensationTable = _compensationTable;
  for  (kkint32  x = 0;  x < lineWidth;  ++x)
    ReComputeLookUpForColumn (x);
}




void  FlatFieldCorrection::AddSampleLine (const uchar*  sampleLine)
{
  lastHistoryIdxAdded++;
  if  (lastHistoryIdxAdded >= numSampleLines)
    lastHistoryIdxAdded = 0;

  uchar*  historyLine = history[lastHistoryIdxAdded];
  for  (kkint32 x = 0;  x < lineWidth;  ++x)
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
      highPointLastSeen[x] = 0;
    }
    else
    {
      highPointLastSeen[x] = 0;
    }
  }

  numSampleLinesAdded++;
}  /* AddSampleLine */




void  FlatFieldCorrection::ReComputeLookUpForColumn (kkint32 col)
{
  if  (enabled)
  {
    highPointLastSeen[col] = 1;

    kkint32 historyIdx = lastHistoryIdxAdded;

    kkint32 age = 1;
    highPoint[col] = 0;

    kkint32 hp0 = 0, hp0Age =0;
    kkint32 hp1 = 0, hp1Age =0;
    kkint32 hp2 = 0, hp2Age =0;

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

      //if  (hv > highPoint[col])
      //{
      //  highPoint[col]         = history[historyIdx][col];
      // highPointLastSeen[col] = age;
      //}
      historyIdx--;
      if  (historyIdx < 0)
        historyIdx = numSampleLines - 1;

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
    kkint32 row = 0;
    kkint32 hp = highPoint[col];

    if  (hp < 28)
    {
      hp = 0;
      kkint32  newPixelValue = 255;
      if  (compensationTable)
        newPixelValue = compensationTable[newPixelValue];
      newPixelValue = 255 - newPixelValue;
      for  (row = 0;  row < 256;  ++row)
        lookUpTable[col][row] = (uchar)newPixelValue;
    }
    else
    {
      kkint32 newPixelValue = 0;
      //kkint32 hpThreshold = (kkint32)(0.5f + (float)hp * 15.0f / 16.0f);
      //kkint32 hpThreshold = hp - 15;
      //hp = Max (0, (hp - 10));
      //hp = Max (0, (hp - 15));

      for  (row = 0;  row < 256;  ++row)
      {
        if  (row  <  hp)
          newPixelValue = (kkint32)(0.5f + (255.0f * (float)row) / (float)hp);
        else
          newPixelValue = 255;

        if  ((newPixelValue < 0)  ||  (newPixelValue > 255))
        {
          cout << "Woaaaaa!!!!! it went beyond 255  or less that 0  ["  << newPixelValue << "]." << endl;
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
    for  (kkint32  row = 0;  row < 256;  ++row)
    {
      kkint32  newPixelValue = row;
      if  (compensationTable)
        newPixelValue = compensationTable[newPixelValue];

      newPixelValue = 255 - newPixelValue;

      lookUpTable[col][row] = (uchar)newPixelValue;
    }
  }
}  /* ReComputeLookUpForColumn */



void  FlatFieldCorrection::ApplyFlatFieldCorrection (uchar*  scanLine)
{
  for  (kkint32 col = 0;  col < lineWidth;  col++)
    scanLine[col] = lookUpTable[col][scanLine[col]];
}  /* ApplyFlatFieldCorrection */



void  FlatFieldCorrection::ApplyFlatFieldCorrection (uchar*  srcScanLine,
                                                     uchar*  destScanLine
                                                    )
{
  if  (enabled)
  {
    for  (kkint32 col = 0;  col < lineWidth;  col++)
      destScanLine[col] = lookUpTable[col][srcScanLine[col]];
  }
  else
  {
    for  (kkint32 col = 0;  col < lineWidth;  col++)
      destScanLine[col] = srcScanLine[col];
  }
}  /* ApplyFlatFieldCorrection */



VectorUcharPtr  FlatFieldCorrection::CameraHighPoints ()  const
{
  vector<uchar>*  results = new vector<uchar> ();
  for  (kkint32 x = 0;  x < lineWidth;  x++)
    results->push_back (highPoint[x]);
  return  results;
}  /* CameraHighPoints */



VectorUcharPtr  FlatFieldCorrection::CameraHighPointsFromLastNSampleLines (kkint32 n)  const
{
  n = Min (n, numSampleLines);

  vector<uchar>*  highPoints = new vector<uchar>(lineWidth, 0);

  kkint32  row = lastHistoryIdxAdded;
  for  (kkint32  x = 0;  x < n;  ++x)
  {
    if  (row < 0)
      row = numSampleLines - 1;
    uchar*  sampleRow = history[row];

    for  (kkint32 col = 0;  col < lineWidth;  ++col)
    {
      if  (sampleRow[col] > (*highPoints)[col])
        (*highPoints)[col] = sampleRow[col];
    }
    --row;
  }
  return  highPoints;
}  /* CameraHighPointsFromLastNSampleLines */
