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



FlatFieldCorrection::FlatFieldCorrection (kkuint32      _numSampleLines,
                                          kkuint32      _lineWidth,
                                          const uchar*  _compensationTable,
                                          kkuint32      _startCol,
                                          bool          _color
                                         ):
    color                (_color),
    compensationTable    (_compensationTable),
    enabled              (true),
    lastHistoryIdxAdded  (_numSampleLines - 1),
    lineWidth            (_lineWidth),
    numSampleLines       (_numSampleLines),
    numSampleLinesAdded  (0),
    startCol             (_startCol)
{
  KKCheck (_numSampleLines > 0,  "_numSampleLines: " + StrFromUint32 (_numSampleLines) + " > 0")
}



FlatFieldCorrection::~FlatFieldCorrection ()
{
}

