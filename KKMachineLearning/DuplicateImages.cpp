#include "FirstIncludes.h"
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace KKB;


#include "DuplicateImages.h"

                
//#include "FeatureFileIOKK.h"
//#include "FeatureFileIOPices.h"
#include "FeatureVector.h"
#include "ImageFeaturesNameIndexed.h"
#include "ImageFeaturesDataIndexed.h"
using namespace  KKMLL;


DuplicateImages::DuplicateImages (FeatureVectorListPtr  _examples,
                                  RunLog&               _log
                                 ):
   duplicateCount     (0),
   duplicateDataCount (0),
   duplicateNameCount (0),
   dupExamples        (new DuplicateImageList (true)),
   featureDataTree    (new ImageFeaturesDataIndexed ()),
   fileDesc           (NULL),
   log                (_log),
   nameTree           (new ImageFeaturesNameIndexed ())

{
  if  (!_examples)
  {
    log.Level (-1) << endl << endl << "DuplicateImages::DuplicateImages   ***ERROR***   '_examples == NULL'" << endl << endl;
    return;
  }
  fileDesc = _examples->FileDesc ();
  FindDuplicates (_examples);
}




DuplicateImages::DuplicateImages (FileDescPtr _fileDesc,
                                  RunLog&     _log
                                 ):
   duplicateCount     (0),
   duplicateDataCount (0),
   duplicateNameCount (0),
   dupExamples        (new DuplicateImageList (true)),
   featureDataTree    (new ImageFeaturesDataIndexed ()),
   fileDesc           (_fileDesc),
   log                (_log),
   nameTree           (new ImageFeaturesNameIndexed ())

{
}




DuplicateImages::~DuplicateImages(void)
{
  delete  nameTree;         nameTree        = NULL;
  delete  featureDataTree;  featureDataTree = NULL;
  delete  dupExamples;      dupExamples     = NULL;
}



bool  DuplicateImages::ExampleInDetector (FeatureVectorPtr  fv)
{
  if  (nameTree->GetEqual (fv->ExampleFileName ()) != NULL)
    return true;

  if  (featureDataTree->GetEqual (fv) != NULL)
    return true;

  return false;
}  /* ExampleInDetector */



bool   DuplicateImages::AddExamples (FeatureVectorListPtr  examples)
{
  bool  dupsDetected = false;
  FeatureVectorList::iterator idx;
  for (idx = examples->begin ();  idx != examples->end ();  idx++)
  {
    DuplicateImagePtr dupExample = AddSingleExample (*idx);
    if  (dupExample)
      dupsDetected = true;
  }

  return dupsDetected;
}  /* AddExamples */




/**
 *@brief Will add one more example to list and if it turns out to be a duplicate will return pointer to a "DuplicateImage" structure
 * that will contain a list of all images that it is duplicate to. If no duplicate found will then return a NULL pointer.
 */
DuplicateImagePtr  DuplicateImages::AddSingleExample (FeatureVectorPtr  example)
{
  DuplicateImagePtr dupExample = NULL;

  FeatureVectorPtr    existingNameExample = NULL;

  const KKStr&  imageFileName = example->ExampleFileName ();
  if  (!imageFileName.Empty ())
  {
    existingNameExample = nameTree->GetEqual (osGetRootName (example->ExampleFileName ()));
    if  (!existingNameExample)
      nameTree->RBInsert (example);
  }

  FeatureVectorPtr  existingDataExample = featureDataTree->GetEqual (example);
  if  (!existingDataExample)
    featureDataTree->RBInsert (example);
  
  if  ((existingNameExample)  ||  (existingDataExample))
  {
    duplicateCount++;
    if  (existingNameExample)
    {
      duplicateNameCount++;
      dupExample = dupExamples->LocateByImage (existingNameExample);
      if  (!dupExample)
      {
        dupExample = new DuplicateImage (fileDesc, existingNameExample, example, log);
        dupExamples->PushOnBack (dupExample);
      }
      else
      {
        dupExample->AddADuplicate (example);
      }
    }
    
    if  (existingDataExample) 
    {
      duplicateDataCount++;
      if  (existingDataExample != existingNameExample)
      {
        dupExample = dupExamples->LocateByImage (existingDataExample);
        if  (!dupExample)
        {
          dupExample = new DuplicateImage (fileDesc, existingDataExample, example, log);
          dupExamples->PushOnBack (dupExample);
        }
        else
        {
          dupExample->AddADuplicate (example);
        }
      }
    }
  }

  return dupExample;
}  /* AddSingleExample */





void  DuplicateImages::FindDuplicates (FeatureVectorListPtr  examples)
{
  if  (!examples)
    return;

  for  (auto idx: *examples)
  {
    FeatureVectorPtr   example = idx;
    AddSingleExample (example);
  }
}  /* FindDuplicates */



/**
 *@brief Delete duplicate examples from FeatureVectorList structure provided in constructor.
 *@details
 *       If duplicates are in more than one class then all will be deleted.
 *       if duplicates are in a single class then one with smallest scan line will be kept
 *       while all others will be deleted.
 */
void  DuplicateImages::PurgeDuplicates (FeatureVectorListPtr  examples,
                                        bool                  allowDupsInSameClass,
                                        ostream*              report
                                       )
{
  log.Level (10) << "DuplicateImageList::PurgeDuplicates" << endl;

  
  // To make sure that we do not delete the same example Twice I added 'deletedDictionary' below.
  // if will track all examples by address that have been deleted.  I did this because a bug in
  // the duplicate detector routine had the same example added to to different groups of duplicates.
  map<FeatureVectorPtr,KKStr>  deletedDictionary;  // List of examples already deleted.
  map<FeatureVectorPtr,KKStr>::iterator  deletedDictionaryIdx;
  
  DuplicateImageListPtr  dupExamples = DupExamples ();

  kkint32  dupSetCount = 0;  
  DuplicateImageList::iterator  dIDX = dupExamples->begin ();

  for  (dIDX = dupExamples->begin ();   dIDX != dupExamples->end ();  ++dIDX, ++dupSetCount)
  {
    DuplicateImagePtr dupSet = *dIDX;

    log.Level (20) << "PurgeDuplicates  Duplicate Set[" << dupSet->FirstExampleAdded ()->ExampleFileName () << "]" << endl;

    FeatureVectorListPtr  examplesInSet = dupSet->DuplicatedImages ();
    FeatureVectorPtr exampleToKeep = NULL;

    if  (dupSet->AllTheSameClass ())
    {
      if  (allowDupsInSameClass)
        continue;
      else
        exampleToKeep = dupSet->ExampleWithSmallestScanLine ();
    }
    
    FeatureVectorList::iterator iIDX = examplesInSet->begin ();

    for  (iIDX = examplesInSet->begin ();  iIDX != examplesInSet->end ();  ++iIDX)
    {
      FeatureVectorPtr example = *iIDX;
      if  (!example)
        continue;

      if  (example == exampleToKeep)
      {
        log.Level (30) << "PurgeDuplicates  Keeping [" << exampleToKeep->ExampleFileName () << "]." << endl;
        if  (report)
          *report << example->ExampleFileName () << "\t" << "Class" << "\t" << example->MLClassName () << "\t" << "Duplicate retained." << endl;
      }
      else
      {
        bool  alreadyDeleted = false;
        deletedDictionaryIdx = deletedDictionary.find (example);
        if  (deletedDictionaryIdx != deletedDictionary.end ())
        {
          // AHA We are getting ready to delete an entry we have already deleted ????
          KKStr  errMsg (1024);
          errMsg << "Example: " << deletedDictionaryIdx->second << "  Already Been Deleted.";
          log.Level (-1) << endl << "DuplicateImages::PurgeDuplicates   ***ERROR***  " << errMsg << endl <<endl;
          alreadyDeleted = true;
        }

        if  (!alreadyDeleted)
        {
          deletedDictionary.insert (pair<FeatureVectorPtr,KKStr> (example, example->ExampleFileName ()));

          log.Level (30) << "PurgeDuplicates  Deleting [" << example->ExampleFileName () << "]." << endl;
          if  (report)
            *report << example->ExampleFileName () << "\t" << "Class" << "\t" << example->MLClassName () << "\t" << "Duplicate deleted." << endl;
          examples->DeleteEntry (example);
          if  (examples->Owner ())
            delete  example;
        }
      }
    }
  }
}  /* PurgeDuplicates */




FeatureVectorListPtr  DuplicateImages::ListOfExamplesToDelete ()
{
  FeatureVectorListPtr  examplesToDelete = new FeatureVectorList (fileDesc, false);

  log.Level (10) << "DuplicateImages::ListOfExamplesToDelete" << endl;

  DuplicateImageListPtr  dupExamples = DupExamples ();

  DuplicateImageList::iterator  dIDX = dupExamples->begin ();

  for  (dIDX = dupExamples->begin ();   dIDX != dupExamples->end ();  ++dIDX)
  {
    DuplicateImagePtr dupSet = *dIDX;

    log.Level (20) << "ListOfExamplesToDelete   Duplicate Set[" << dupSet->FirstExampleAdded ()->ExampleFileName () << "]" << endl;

    FeatureVectorListPtr  examplesInSet = dupSet->DuplicatedImages ();
    FeatureVectorPtr exampleToKeep = NULL;

    if  (dupSet->AllTheSameClass ())
    {
      exampleToKeep = dupSet->ExampleWithSmallestScanLine ();
    }
    
    FeatureVectorList::iterator iIDX = examplesInSet->begin ();

    for  (iIDX = examplesInSet->begin ();  iIDX != examplesInSet->end ();  ++iIDX)
    {
      FeatureVectorPtr example = *iIDX;
      if  (!example)
        continue;

      if  (example == exampleToKeep)
      {
        log.Level (30) << "ListOfExamplesToDelete  Keeping [" << exampleToKeep->ExampleFileName () << "]." << endl;
      }
      else
      {
        log.Level (30) << "ListOfExamplesToDelete  Deleting [" << example->ExampleFileName () << "]." << endl;
        examplesToDelete->PushOnBack (example);
      }
    }
  }

  return  examplesToDelete;
}  /* ListOfExamplesToDelete */




void   DuplicateImages::ReportDuplicates (ostream&  o)
{
  o << "Number of Duplicate Groups [" << dupExamples->QueueSize () << "]" << endl;
  kkint32  groupNum = 0;

  //for  (DuplicateImageList::iterator  idx = dupExamples->begin ();  idx != dupExamples->end ();  idx++)
  for  (auto  dupExampleSet:  *dupExamples)
  {
    const FeatureVectorListPtr  dupList = dupExampleSet->DuplicatedImages ();

    o << "Group[" << groupNum << "] Contains [" << dupList->QueueSize () << "] Duplicates." << endl;

    kkint32  numOnLine = 0;
    //FeatureVectorList::const_iterator  fvIDX;
    for  (auto fvIDX: *dupList)   //  = dupList->begin ();  fvIDX != dupList->end ();  ++fvIDX)
    {
      if  (numOnLine > 8)
      {
        o << endl;
        numOnLine = 0;
      }

      if  (numOnLine > 0)
        o << "\t";
      o << fvIDX->ExampleFileName () << "[" << fvIDX->MLClassName () << "]";

      numOnLine++;
    }
    o << endl << endl;;

    groupNum++;
  }
}  /* ReportDuplicates */



bool   DuplicateImages::DuplicatesFound ()  const 
{
  return  (dupExamples->QueueSize () > 0);
}



DuplicateImage::DuplicateImage (FileDescPtr       _fileDesc,
                                FeatureVectorPtr  _image1, /**< image1, will be the one that we was already in the index structures. */
                                FeatureVectorPtr  _image2,
                                RunLog&           _log
                               ):
   fileDesc         (_fileDesc),
   duplicatedImages (_fileDesc, false),
   firstImageAdded  (_image1)
{
  duplicatedImages.PushOnBack (_image1);
  duplicatedImages.PushOnBack (_image2);
}


DuplicateImage::~DuplicateImage ()
{
}


void  DuplicateImage::AddADuplicate (FeatureVectorPtr  example)
{
  duplicatedImages.PushOnBack (example);
}  /* AddADuplicate */





bool  DuplicateImage::AllTheSameClass ()
{
  bool  allTheSameClass = true;
  
  MLClassPtr  mlClass = duplicatedImages.IdxToPtr (0)->MLClass ();

  FeatureVectorList::iterator iIDX = duplicatedImages.begin ();

  while ((iIDX != duplicatedImages.end ()) &&  (allTheSameClass))
  {
    allTheSameClass = (*iIDX)->MLClass () == mlClass;
    iIDX++;
  }

  return  allTheSameClass;
}  /* AllTheSameClass */





bool  DuplicateImage::AlreadyHaveExample (FeatureVectorPtr example)
{
  return  (duplicatedImages.PtrToIdx (example) >= 0);
}




FeatureVectorPtr  DuplicateImage::ExampleWithSmallestScanLine ()
{
  kkint32  smallestScanLine = 99999999;
  FeatureVectorPtr  imageWithSmallestScanLine = NULL;


  for  (FeatureVectorList::iterator  iIDX = duplicatedImages.begin ();  iIDX != duplicatedImages.end (); iIDX++)
  {
    FeatureVectorPtr i = *iIDX;
    // First lets derive scan line from example file name
   
    KKStr  rootName = osGetRootName (i->ExampleFileName ());
    rootName.Upper ();

    kkint32  scanLine = 9999999;

    if  (rootName.SubStrPart (0, 4) == "FRAME")
    {
      // Scan line will be last seq number in name.
      kkint32 x = rootName.LocateLastOccurrence ('_');
      if  (x > 0)
      {
        KKStr  scanLineStr = rootName.SubStrPart (x + 1);
        scanLine = atoi (scanLineStr.Str ());
      }
    }
    else
    {
      // Scan should be 2nd to last seq number in name.
      kkint32 x = rootName.LocateLastOccurrence ('_');
      if  (x > 0)
      {
        KKStr  workStr = rootName.SubStrPart (0, x - 1);
        kkint32 x = workStr.LocateLastOccurrence ('_');
        KKStr  scanLineStr = workStr.SubStrPart (x + 1);
        scanLine = atoi (scanLineStr.Str ());
      }
    }

    if  ((scanLine < smallestScanLine)   ||  
         (imageWithSmallestScanLine == NULL)
        )
    {
      smallestScanLine = scanLine;
      imageWithSmallestScanLine = i;
    }
  }

  return  imageWithSmallestScanLine;
}  /* ImageWithSmallestScalLine */




DuplicateImageList::DuplicateImageList (bool _owner):
  KKQueue<DuplicateImage> (_owner)
{
}



DuplicateImageList::~DuplicateImageList ()
{
}



DuplicateImagePtr  DuplicateImageList::LocateByImage (FeatureVectorPtr  example)
{
  for  (DuplicateImageList::iterator  idx = begin ();  idx != end ();  idx++)
  {
    DuplicateImagePtr dupExampleSet = *idx;
    if  (dupExampleSet->AlreadyHaveExample (example))
      return  dupExampleSet;
  }

  return NULL;
}  /* LocateByImage */
