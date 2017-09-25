#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <errno.h>
#include  <string.h>
#include  <ctype.h>

#include  <iostream>
#include  <fstream>
#include  <map>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;

#include "GoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "KKStr.h"
using namespace  KKB;

#include "ScannerHeaderFields.h"
using namespace  KKLSC;


ScannerHeaderFields::ScannerHeaderFields ():
  map<KKStr,KKStr> (),
  goalie (NULL)
{
  GoalKeeper::Create ("ScannerHeaderFields", goalie);
}

  
  
ScannerHeaderFields::ScannerHeaderFields (const ScannerHeaderFields&  fields):
  map<KKStr,KKStr> (),
  goalie (NULL)
{
  GoalKeeper::Create ("ScannerHeaderFields", goalie);
  ScannerHeaderFields::const_iterator  idx;
  for  (idx = fields.begin ();  idx != fields.end ();  ++idx)
    Add (idx->first, idx->second);
}



ScannerHeaderFields::~ScannerHeaderFields ()
{
  GoalKeeper::Destroy (goalie);
  goalie = NULL;
}



kkMemSize  ScannerHeaderFields::MemoryConsumedEstimated () const
{
  goalie->StartBlock ();

  kkMemSize  mem = sizeof (*this);

  if  (goalie)   mem += goalie->MemoryConsumedEstimated ();
  
  for  (idx2 = begin ();  idx2 != end ();  ++idx2)
  {
    mem = mem + (idx2->first.MemoryConsumedEstimated () + idx2->second.MemoryConsumedEstimated ());
  }

  goalie->EndBlock ();

  return  mem;
}



void  ScannerHeaderFields::Add (ScannerHeaderFieldsPtr  fields)
{
  if  (!fields)
    return;
  ScannerHeaderFields::const_iterator  idx;
  for  (idx = fields->begin ();  idx != fields->end ();  ++idx)
    Add (idx->first, idx->second);
}



void  ScannerHeaderFields::Add (const KKB::KKStr&  fieldName,
                                const KKB::KKStr&  fieldValue
                               )
{
  goalie->StartBlock ();
  
  idx1 = this->find (fieldName);
  if  (idx1 == end ())
    insert (pair<KKStr,KKStr>(fieldName, fieldValue));
  else
    idx1->second = fieldValue;

  goalie->EndBlock ();
}


void  ScannerHeaderFields::Add (const KKB::KKStr&  fieldName,
                                bool               fieldValue
                               )
{
  KKStr fieldValueStr = (fieldValue ? "Yes" : "No");
  Add (fieldName, fieldValueStr);
}


void  ScannerHeaderFields::Add (const KKStr&  fieldName,
                                kkint32       fieldValue
                               )
{
  KKStr fieldValueStr = StrFromInt64 (fieldValue);
  Add (fieldName, fieldValueStr);
}



void  ScannerHeaderFields::Add (const KKStr&  fieldName,
                                kkint64       fieldValue
                               )
{
  KKStr fieldValueStr = StrFromInt64 (fieldValue);
  Add (fieldName, fieldValueStr);
}



void  ScannerHeaderFields::Add (const KKStr&  fieldName,
                                double        fieldValue
                               )
{
  KKStr  fieldValueStr (20);
  fieldValueStr << fieldValue;
  Add (fieldName, fieldValueStr);
}




void  ScannerHeaderFields::Add (const KKStr&  fieldName,
                                DateTime      fieldValue
                               )
{
  KKStr s (20);
  s << fieldValue;
  Add (fieldName, s);
}




void  ScannerHeaderFields::Clear ()
{
  goalie->StartBlock ();
  clear ();
  goalie->EndBlock ();
}



bool  ScannerHeaderFields::FieldExists (const KKStr&  fieldName)  const
{
  bool  exists = false;
  goalie->StartBlock ();
  idx2 = this->find (fieldName);
  exists = (idx2 != end ());
  goalie->EndBlock ();
  return exists;
}



const KKStr&  ScannerHeaderFields::GetValue (const KKStr&  fieldName)  const
{
  const KKStr*  value = NULL;

  goalie->StartBlock ();
  idx2 = this->find (fieldName);
  if  (idx2 == end ())
    value = &(KKStr::EmptyStr ());
  else
    value = &(idx2->second);
  goalie->EndBlock ();

  return  *value;
}



float  ScannerHeaderFields::GetValueFloat (const KKStr&  fieldName)  const
{
  KKStr  s = GetValue (fieldName);
  return s.ToFloat ();
}



kkint32  ScannerHeaderFields::GetValueInt32 (const KKStr&  fieldName)  const
{
  KKStr  s = GetValue (fieldName);
  return s.ToInt32 ();
}




void  ScannerHeaderFields::StartBlock ()
{
  goalie->StartBlock ();
}



void  ScannerHeaderFields::EndBlock ()
{
  goalie->EndBlock ();
}
