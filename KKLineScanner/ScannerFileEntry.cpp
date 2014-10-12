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

#include "ScannerFileEntry.h"
#include "ScannerFile.h"
using  namespace  KKLSC;


ScannerFileEntry::ScannerFileEntry (const KKStr&  _rootName):
     description       (),
     fullName          (),
     pixelsPerScanLine (0),
     rootName          (_rootName),
     scanRate          (0.0f)
{

}



ScannerFileEntry::ScannerFileEntry (const ScannerFileEntry&  entry):
     description       (entry.description),
     fullName          (entry.fullName),
     pixelsPerScanLine (entry.pixelsPerScanLine),
     rootName          (entry.rootName),
     scanRate          (entry.scanRate)
{
}




ScannerFileEntry::~ScannerFileEntry ()
{
}



int32  ScannerFileEntry::MemoryConsumedEstimated ()
{
  int32  mem = sizeof (*this) + 
               description.MemoryConsumedEstimated () +
               fullName.MemoryConsumedEstimated ()    +
               rootName.MemoryConsumedEstimated ();
  return  mem;
}



ScannerFileEntryListPtr  ScannerFileEntry::globalList = NULL;


void  ScannerFileEntry::CleanUp ()
{
  delete  globalList;
  globalList  = NULL;
}


KKStr  ScannerFileEntry::ToTabDelStr ()  const
{
  KKStr  r (256);
  r << "Description"       << "\t" << description        << "\t"
    << "FullName"          << "\t" << fullName           << "\t"
    << "PixelsPerScanLine" << "\t" << pixelsPerScanLine  << "\t"
    << "RootName"          << "\t" << rootName           << "\t"
    << "ScanRate"          << "\t" << scanRate;

  return r;
}  /* ToTabDelStr */


void  ScannerFileEntry::ParseTabDelStr (KKStr  ln)
{
  KKStr  fieldName = ln.ExtractToken2 ("\t");
  while  (!ln.Empty ())
  {
    KKStr  fieldName = ln.ExtractToken2 ("\t");

    if  (fieldName.EqualIgnoreCase ("Description"))
      description = ln.ExtractToken2 ("\t");

    else if  (fieldName.EqualIgnoreCase ("FullName"))
      fullName = ln.ExtractToken2 ("\t");

    else if  (fieldName.EqualIgnoreCase ("PixelsPerScanLine"))
      pixelsPerScanLine = ln.ExtractTokenUint ("\t");

    else if  (fieldName.EqualIgnoreCase ("RootName"))
      rootName = ln.ExtractToken2 ("\t");

    else if  (fieldName.EqualIgnoreCase ("scanRate"))
      scanRate = (float)ln.ExtractTokenDouble ("\t");
  }

  return;
}  /* ParseTabDelStr */



void   ScannerFileEntry::Assign (const ScannerFileEntry&  entry)
{
  description       = entry.description;
  fullName          = entry.fullName;
  pixelsPerScanLine = entry.pixelsPerScanLine;
  rootName          = entry.rootName;
  scanRate          = entry.scanRate;
}  /* ReFresh */



ScannerFileEntryPtr    ScannerFileEntry::GetOrCreateScannerFileEntry (const KKStr&  rootName)
{
  ScannerFileEntryPtr  entry = NULL;

  KKB::GlobalGoalKeeper::StartBlock ();
  if  (globalList == NULL)
  {
    globalList = new ScannerFileEntryList ();
    atexit (CleanUp);
  }

  entry = globalList->LookUpByRootName (rootName);
  if  (entry == NULL)
  {
    entry = new ScannerFileEntry (rootName);
    globalList->AddEntry (entry);
  }

  KKB::GlobalGoalKeeper::EndBlock ();

  return  entry;
}



ScannerFileEntryPtr  ScannerFileEntry::GetOrCreateScannerFileEntry (KKLSC::ScannerFilePtr  scannerFile)
{
  ScannerFileEntryPtr  entry = NULL;

  KKB::GlobalGoalKeeper::StartBlock ();
  if  (globalList == NULL)
  {
    globalList = new ScannerFileEntryList ();
    atexit (CleanUp);
  }

  KKStr rootName = osGetRootName (scannerFile->FileName ());
  entry = globalList->LookUpByRootName (rootName);
  if  (entry == NULL)
  {
    entry = new ScannerFileEntry (rootName);
    entry->FullName (scannerFile->FileName ());
    entry->PixelsPerScanLine (scannerFile->PixelsPerScanLine ());
    entry->ScanRate (scannerFile->ScanRate ());
    globalList->AddEntry (entry);
  }

  KKB::GlobalGoalKeeper::EndBlock ();

  return  entry;
}






ScannerFileEntryList::ScannerFileEntryList ():
    map<KKStr, ScannerFileEntryPtr> ()
{
}


ScannerFileEntryList::~ScannerFileEntryList ()
{
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    ScannerFileEntryPtr  sfe = idx->second;
    delete  sfe;
  }
}


int32  ScannerFileEntryList::MemoryConsumedEstimated ()
{
  int32 mem = sizeof (*this);

  for  (idx = begin ();  idx != end ();  ++idx)
  {
    ScannerFileEntryPtr  sfe = idx->second;
    mem += sfe->MemoryConsumedEstimated ();
  }

  return  mem;
}




void  ScannerFileEntryList::AddEntry (ScannerFileEntryPtr  entry)
{
  insert (KeyPair (entry->RootName (), entry));
}



ScannerFileEntryPtr  ScannerFileEntryList::LookUpByRootName (const KKStr&  rootName)
{
  idx = find (rootName);
  if  (idx == end ())
    return NULL;
  else
    return idx->second;
}


