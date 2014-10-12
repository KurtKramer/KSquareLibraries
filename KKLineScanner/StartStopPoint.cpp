#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "MemoryDebug.h"
using namespace std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKStrParser.h"
#include "OSservices.h"
#include "KKStr.h"
using namespace KKB;

#include "Variables.h"

#include "StartStopPoint.h"
using  namespace  KKLSC;



KKB::KKStr                      StartStopPoint::startStopPointStrs [] = {KKStr ("NULL"),  KKStr ("StartPoint"),  KKStr ("Start"),  KKStr ("StopPoint"),  KKStr ("Stop"),  KKStr ("Invalid"), ""};
StartStopPoint::StartStopType   StartStopPoint::startStopPointTypes[] = {sspNULL,         sspStartPoint,         sspStartPoint,    sspStopPoint,         sspStopPoint,    sspInvalid};

const KKStr&  StartStopPoint::StartStopTypeToStr (StartStopType  t)
{
  if  (t <= sspNULL)     return (startStopPointStrs[sspNULL]);
  if  (t >= sspInvalid)  return (startStopPointStrs[sspInvalid]);
  return  (startStopPointStrs[t]);
}



StartStopPoint::StartStopType  StartStopPoint::StartStopTypeFromStr (const KKStr&   s)
{
  for  (int32 x = 0;  x < sspInvalid;  ++x)
    if  (s.EqualIgnoreCase (startStopPointStrs[x]))
      return  (StartStopType)x;
  return  sspNULL;
}


StartStopPoint::StartStopPoint (int32          _scanLineNum,
                                StartStopType  _type
                                ):
   scanLineNum (_scanLineNum),
   type        (_type)
{
}



StartStopPoint::StartStopPoint (const StartStopPoint&  entry):
   scanLineNum (entry.scanLineNum),
   type        (entry.type)
{
}



   StartStopPoint::StartStopPoint (const KKStr&  s):
   scanLineNum (-1),
   type        (sspInvalid)
{
  ParseTabDelStr (s);
}


   
StartStopPoint::~StartStopPoint ()
{
}


int32  StartStopPoint::MemoryConsumedEstimated ()  const
{
  return  sizeof (*this);
}




KKStr  StartStopPoint::ToTabDelStr ()  const
{
  KKStr s (48);
  s << scanLineNum << "\t" << TypeStr ();
  return  s;
}



void   StartStopPoint::ParseTabDelStr (KKStr  parser)
{
  scanLineNum = parser.ExtractTokenInt ("\t");
  KKStr  typeStr = parser.ExtractToken2 ("\t");
  type = StartStopTypeFromStr (typeStr);
}




StartStopPointList::StartStopPointList ():
    vector<StartStopPointPtr> ()
{
}


StartStopPointList::~StartStopPointList ()
{
  Clear ();
}


void  StartStopPointList::Clear ()
{
  while  (size () > 0)
  {
    StartStopPointPtr e = back ();
    pop_back ();
    delete  e;
    e = NULL;
  }
}



int32  StartStopPointList::MemoryConsumedEstimated ()  const
{
  const_iterator  idx2;

  int32 mem = sizeof (*this);

  for  (idx2 = begin ();  idx2 != end ();  ++idx2)
  {
    StartStopPointPtr  sp = *idx2;
    mem += sp->MemoryConsumedEstimated ();
  }

  return  mem;
}




StartStopPointPtr  StartStopPointList::AddEntry (int32                          _scanLineNum,
                                                 StartStopPoint::StartStopType  _type
                                                )
{
  StartStopPointPtr  newEntry = new StartStopPoint (_scanLineNum, _type);
  return  AddEntry (newEntry);
}  /* AddEntry */



StartStopPointPtr  StartStopPointList::AddEntry (StartStopPointPtr&  _entry)
{
  if  (size () < 1)
  {
    push_back (_entry);
    return  _entry;
  }

  StartStopPointPtr  entryAdded = _entry;

  int32 m = FindGreaterOrEqual (_entry->ScanLineNum ());
  if  (m < 0)
    push_back (_entry);

  else 
  {
    StartStopPointPtr  existingEntry = (*this)[m];
    if  (existingEntry->ScanLineNum () == _entry->ScanLineNum ())
    {
      entryAdded = existingEntry;
      existingEntry->Type (_entry->Type ());
      delete  _entry;
      _entry = NULL;
    }
    else
    {
      entryAdded = _entry;
      idx = begin () + m;
      insert (idx, _entry);
    }
  }

  return  entryAdded;
}  /* AddEntry */



void  StartStopPointList::DeleteEntry (int32  _scanLineNum)
{
  int32  m = FindEqual (_scanLineNum);
  if  (m < 0)
    return;

  StartStopPointPtr  entry = (*this)[m];
  erase (begin () + m);
  delete  entry;
}


int32  StartStopPointList::FindEqual (int32 _scanLineNum)  const
{
  if  (size () < 1)
    return -1;

  uint32 b = 0;
  uint32 e = size () - 1;
  uint32 m = 0;

  int32  entryScanLineNum = 0;

  do
  {
    m = (b + e) / 2;

    entryScanLineNum = (*this)[m]->ScanLineNum ();

    if  (entryScanLineNum == _scanLineNum)
      return  m;

    if  (entryScanLineNum > _scanLineNum)
      e = m - 1;

    else
      b = m + 1;

  }  while  (b <= e);
  return  -1;
}  /* FindGreaterOrEqual */




int32  StartStopPointList::FindGreaterOrEqual (int32 _scanLineNum)  const
{
  if  (size () < 1)
    return -1;

  if  ((*this)[size () - 1]->ScanLineNum () < _scanLineNum)
    return -1;

  int32 b = 0;
  int32 e = (int32)size () - 1;
  int32 m = 0;

  int32  entryScanLineNum = 0;

  do
  {
    m = (b + e) / 2;

    entryScanLineNum = (*this)[m]->ScanLineNum ();

    if  (entryScanLineNum == _scanLineNum)
      return  m;

    if  (entryScanLineNum > _scanLineNum)
      e = m - 1;

    else
      b = m + 1;

  }  while  (b <= e);

  if  (entryScanLineNum > _scanLineNum)
    return  m;

  if  (m < ((int32)size () - 1))
    return m + 1;
  else
    return  -1;
}  /* FindGreaterOrEqual */



int32  StartStopPointList::FindLessOrEqual (int32 _scanLineNum)  const
{
  if  (size () < 1)
    return -1;

  if  ((*this)[0]->ScanLineNum () > _scanLineNum)
    return -1;

  int32 b = 0;
  int32 e = (int32)size () - 1;
  int32 m = 0;

  int32  entryScanLineNum = 0;

  do
  {
    m = (b + e) / 2;

    entryScanLineNum = (*this)[m]->ScanLineNum ();

    if  (entryScanLineNum == _scanLineNum)
      return  m;

    if  (entryScanLineNum > _scanLineNum)
      e = m - 1;

    else
      b = m + 1;

  }  while  (b <= e);

  if  (entryScanLineNum < _scanLineNum)
    return  m;

  if  (m > 0)
    return m - 1;
  else
    return -1;
}  /* FindLessOrEqual */



StartStopPointPtr  StartStopPointList::PrevEntry (int32  _scanLineNum)  const
{
  int32  m = FindLessOrEqual (_scanLineNum);
  if  (m >= 0)
    return  (*this)[m];
  else
    return NULL;
}  /* PrevEntry */



StartStopPointPtr  StartStopPointList::SuccEntry (int32  _scanLineNum)  const
{
  int32  m = FindGreaterOrEqual (_scanLineNum);
  if  (m >= 0)
    return  (*this)[m];
  else
    return NULL;
}  /* SuccEntry */



StartStopPointPtr  StartStopPointList::NearestEntry (int32  _scanLineNum)  const
{
  if  (size () < 1)
    return NULL;

  else if  (size () == 1)
    return  (*this)[0];

  int32  m = FindLessOrEqual (_scanLineNum);
  if  (m < 0)
    return  (*this)[0];

  if  (m >= ((int32)size () - 1))
    return  (*this)[m];

  int32  deltaBefore = (_scanLineNum - (*this)[m]->ScanLineNum ());
  int32  deltaAfter  = (*this)[m + 1]->ScanLineNum () - _scanLineNum;
  if  (deltaBefore <= deltaAfter)
    return (*this)[m];
  else
    return (*this)[m + 1];
}




StartStopRegion::StartStopRegion (int32  _start,  
                                  int32  _end
                                 ):
    start (_start),
    end   (_end)
{
}



StartStopRegionList::StartStopRegionList (bool  _owner):
    KKQueue<StartStopRegion> (_owner)
{
}



StartStopRegionList::StartStopRegionList (const StartStopPointList&  startStopPoints):
    KKQueue<StartStopRegion> (true)
{
  if  (startStopPoints.size () < 1)
  {
    PushOnBack (new StartStopRegion (0, int32_max));
  }
  else
  {
    int32  prevLineNum = 0;
    StartStopPoint::StartStopType  prevType = StartStopPoint::sspStartPoint;

    StartStopPointList::const_iterator  idx = startStopPoints.begin ();
    while  (idx != startStopPoints.end ())
    {
      int32  nextLineNum = (*idx)->ScanLineNum ();
      StartStopPoint::StartStopType  nextType = (*idx)->Type ();

      if  (prevType == StartStopPoint::sspStartPoint)
      {
        if  (nextType == StartStopPoint::sspStopPoint)
        {
          PushOnBack (new StartStopRegion (prevLineNum, nextLineNum));
          prevType = nextType;
          prevLineNum = nextLineNum;
        }
        else
        {
          // Since we appear to have two start points in a row we will ignore the second one.
        }
      }
      else
      {
        // previous point was StopPoint.
        if  (nextType == StartStopPoint::sspStopPoint)
        {
          // We have two stop points in a row;  will ignore
        }
        else
        {
          // We have the beginnings of a new StartStopregion.
          prevType = nextType;
          prevLineNum = nextLineNum;
        }
      }

      ++idx;
    }

    if  (prevType == StartStopPoint::sspStartPoint)
    {
      PushOnBack (new StartStopRegion (prevLineNum, int32_max));
    }
  }
}



StartStopRegionList::~StartStopRegionList ()
{
}
