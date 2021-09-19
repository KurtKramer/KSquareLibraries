/* KKStrMatrix.cpp -- 2D Matrix of Strings.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"
#include  <ctype.h>
#include  <limits.h>
#include  <math.h>
#include  <stdio.h>
#include  <iostream>
#include  <memory>
#include  <sstream>
#include  <string>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;

#include  "KKStrMatrix.h"
using namespace KKB;



KKStrList&  KKStrMatrix::operator[] (kkuint32 row)
{
  if  (row >= data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
      + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }
  return  data[row];
}



KKStrList&  KKStrMatrix::operator() (kkuint32 row)
{
  if  (row >= data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
      + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }
  return  data[row];
}



KKStr&  KKStrMatrix::operator() (kkuint32  row,
                                 kkuint32  col
                                )
{
  if  (row >= data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
                  + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw KKException (errMsg);
  }

  KKStrList&  rowOfData = data[row];
  if  (col >= rowOfData.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Col dimension[" + StrFormatInt (col, "0") + "] invalid;  Rows Available["
                  + StrFormatInt (rowOfData.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }

  return  rowOfData[col];
}



void  KKStrMatrix::AddRow (KKStrListPtr newRowData)
{
  if (!newRowData)
    throw KKException ("KKStrMatrix::AddRow   Cann not add nullptr.");

  while  (newRowData->QueueSize () < numCols)
    newRowData->PushOnBack (new KKStr ());
  data.PushOnBack (newRowData);
}



void  KKStrMatrix::AddRow ()  // Will add one row of empty Strings
{
  KKStrListPtr  newRowData = new KKStrList (true);
  while  (newRowData->QueueSize () < numCols)
    newRowData->PushOnBack (new KKStr ());
  data.PushOnBack (newRowData);
}
