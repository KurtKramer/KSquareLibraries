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
#include "FeatureVector.h"
#include "ImageFeaturesNameIndexed.h"
#include "ImageFeaturesDataIndexed.h"
using namespace  KKMachineLearning;


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
  if  (nameTree->GetEqual (fv->ImageFileName ()) != NULL)
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
    DuplicateImagePtr dupExample = AddSingleImage (*idx);
    if  (dupExample)
      dupsDetected = true;
  }

  return dupsDetected;
}  /* AddExamples */




/**
 *@brief Will add one more image to list and if it turns out to be a duplicate will return pointer to a "DuplicateImage" structure
 * that will contain a list of all images that it is duplicate to. If no duplicate found will then return a NULL pointer.
 */
DuplicateImagePtr  DuplicateImages::AddSingleImage (FeatureVectorPtr  image)
{
  DuplicateImagePtr dupImage = NULL;

  FeatureVectorPtr  existingNameImage = NULL;

  const KKStr&  imageFileName = image->ImageFileName ();
  if  (!imageFileName.Empty ())
  {
    existingNameImage = nameTree->GetEqual (osGetRootName (image->ImageFileName ()));
    if  (!existingNameImage)
      nameTree->RBInsert (image);
  }

  FeatureVectorPtr  existingDataImage = featureDataTree->GetEqual (image);
  if  (!existingDataImage)
    featureDataTree->RBInsert (image);

  if  ((existingNameImage)  ||  (existingDataImage))
  {
    duplicateCount++;
    if  (existingNameImage)
    {
    duplicateNameCount++;
    dupImage = dupExamples->LocateByImage (existingNameImage);
      if  (!dupImage)
    {
        dupImage = new DuplicateImage (fileDesc, existingNameImage, image, log);
        dupExamples->PushOnBack (dupImage);
    }
    else
    {
        dupImage->AddADuplicate (image);
    }
  }
    
    if  (existingDataImage) 
    {
      duplicateDataCount++;
      if  (existingDataImage != existingNameImage)
      {
      dupImage = dupExamples->LocateByImage (existingDataImage);
        if  (!dupImage)
      {
          dupImage = new DuplicateImage (fileDesc, existingDataImage, image, log);
          dupExamples->PushOnBack (dupImage);
      }
      else
      {
          dupImage->AddADuplicate (image);
        }
      }
    }
  }

  return dupImage;
}  /* AddSingleImage */





void  DuplicateImages::FindDuplicates (FeatureVectorListPtr  examples)
{
  if  (!examples)
    return;

  FeatureVectorPtr   image = NULL;
  for  (FeatureVectorList::iterator  idx = examples->begin ();  idx != examples->end ();  idx++)
  {
    image = *idx;
    AddSingleImage (image);
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

  DuplicateImageListPtr  dupExamples = DupImages ();

  DuplicateImageList::iterator  dIDX = dupExamples->begin ();

  for  (dIDX = dupExamples->begin ();   dIDX != dupExamples->end ();  dIDX++)
  {
    DuplicateImagePtr dupSet = *dIDX;

    log.Level (20) << "PurgeDuplicates  Duplicate Set[" << dupSet->FirstImageAdded ()->ImageFileName () << "]" << endl;

    FeatureVectorListPtr  imagesInSet = dupSet->DuplicatedImages ();
    FeatureVectorPtr imageToKeep = NULL;

    if  (dupSet->AllTheSameClass ())
    {
      if  (allowDupsInSameClass)
        continue;
      else
        imageToKeep = dupSet->ImageWithSmallestScanLine ();
    }
    
    FeatureVectorList::iterator iIDX = imagesInSet->begin ();

    for  (iIDX = imagesInSet->begin ();  iIDX != imagesInSet->end ();  iIDX++)
    {
      FeatureVectorPtr image = *iIDX;

      if  (image == imageToKeep)
      {
        log.Level (30) << "PurgeDuplicates  Keeping [" << imageToKeep->ImageFileName () << "]." << endl;
        if  (report)
          *report << image->ImageFileName () << "\t" << "Class" << "\t" << image->MLClassName () << "\t" << "Duplicate retained." << endl;
      }
      else
      {
        log.Level (30) << "PurgeDuplicates  Deleting [" << image->ImageFileName () << "]." << endl;
        if  (report)
          *report << image->ImageFileName () << "\t" << "Class" << "\t" << image->MLClassName () << "\t" << "Duplicate deleted." << endl;
        examples->DeleteEntry (image);
        if  (examples->Owner ())
          delete  image;
      }
    }
  }
}  /* PurgeDuplicates */




FeatureVectorListPtr  DuplicateImages::ListOfExamplesToDelete ()
{
  FeatureVectorListPtr  examplesToDelete = new FeatureVectorList (fileDesc, false, log);

  log.Level (10) << "DuplicateImages::ListOfExamplesToDelete" << endl;

  DuplicateImageListPtr  dupExamples = DupImages ();

  DuplicateImageList::iterator  dIDX = dupExamples->begin ();

  for  (dIDX = dupExamples->begin ();   dIDX != dupExamples->end ();  dIDX++)
  {
    DuplicateImagePtr dupSet = *dIDX;

    log.Level (20) << "ListOfExamplesToDelete   Duplicate Set[" << dupSet->FirstImageAdded ()->ImageFileName () << "]" << endl;

    FeatureVectorListPtr  imagesInSet = dupSet->DuplicatedImages ();
    FeatureVectorPtr imageToKeep = NULL;

    if  (dupSet->AllTheSameClass ())
    {
      imageToKeep = dupSet->ImageWithSmallestScanLine ();
    }
    
    FeatureVectorList::iterator iIDX = imagesInSet->begin ();

    for  (iIDX = imagesInSet->begin ();  iIDX != imagesInSet->end ();  iIDX++)
    {
      FeatureVectorPtr image = *iIDX;

      if  (image == imageToKeep)
      {
        log.Level (30) << "ListOfExamplesToDelete  Keeping [" << imageToKeep->ImageFileName () << "]." << endl;
      }
      else
      {
        log.Level (30) << "ListOfExamplesToDelete  Deleting [" << image->ImageFileName () << "]." << endl;
        examplesToDelete->PushOnBack (image);
      }
    }
  }

  return  examplesToDelete;
}  /* ListOfExamplesToDelete */




void   DuplicateImages::ReportDuplicates (ostream&  o)
{
  o << "Number of Duplicate Groups [" << dupExamples->QueueSize () << "]" << endl;
  kkint32  groupNum = 0;

  for  (DuplicateImageList::iterator  idx = dupExamples->begin ();  idx != dupExamples->end ();  idx++)
  {
    DuplicateImagePtr dupImageSet = *idx;

    const FeatureVectorListPtr  dupList = dupImageSet->DuplicatedImages ();

    o << "Group[" << groupNum << "] Contains [" << dupList->QueueSize () << "] Duplicates." << endl;

    kkint32  numOnLine = 0;
    FeatureVectorList::const_iterator  imageIDX;
    for  (imageIDX = dupList->begin ();  imageIDX != dupList->end (); imageIDX++)
    {
      FeatureVectorPtr  i = *imageIDX;

      if  (numOnLine > 8)
      {
        o << endl;
        numOnLine = 0;
      }

      if  (numOnLine > 0)
        o << "\t";
      o << i->ImageFileName () << "[" << i->MLClassName () << "]";

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
   duplicatedImages (_fileDesc, false, _log),
   firstImageAdded  (_image1)
{
  duplicatedImages.PushOnBack (_image1);
  duplicatedImages.PushOnBack (_image2);
}


DuplicateImage::~DuplicateImage ()
{
}


void  DuplicateImage::AddADuplicate (FeatureVectorPtr  image)
{
  duplicatedImages.PushOnBack (image);
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





bool  DuplicateImage::AlreadyHaveImage (FeatureVectorPtr image)
{
  return  (duplicatedImages.PtrToIdx (image) >= 0);
}




FeatureVectorPtr  DuplicateImage::ImageWithSmallestScanLine ()
{
  kkint32  smallestScanLine = 99999999;
  FeatureVectorPtr  imageWithSmallestScanLine = NULL;


  for  (FeatureVectorList::iterator  iIDX = duplicatedImages.begin ();  iIDX != duplicatedImages.end (); iIDX++)
  {
    FeatureVectorPtr i = *iIDX;
    // First lets derive scan line from image file name
   
    KKStr  rootName = osGetRootName (i->ImageFileName ());
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



DuplicateImagePtr  DuplicateImageList::LocateByImage (FeatureVectorPtr  image)
{
  for  (DuplicateImageList::iterator  idx = begin ();  idx != end ();  idx++)
  {
    DuplicateImagePtr dupImageSet = *idx;
    if  (dupImageSet->AlreadyHaveImage (image))
      return  dupImageSet;
  }

  return NULL;
}  /* LocateByImage */
