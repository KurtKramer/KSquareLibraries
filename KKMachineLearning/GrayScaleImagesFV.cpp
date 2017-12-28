#include "FirstIncludes.h"

#include <ctype.h>
#include <math.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string.h>
#include <typeinfo>

#include "MemoryDebug.h"

using namespace  std;


#include "KKBaseTypes.h"
#include "Blob.h"
#include "BMPImage.h"
#include "ContourFollower.h"
#include "ConvexHull.h"
#include "DateTime.h"
#include "ImageIO.h"
#include "KKException.h"
#include "OSservices.h"
#include "Raster.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "FactoryFVProducer.h"
#include "FeatureNumList.h"
#include "FeatureFileIO.h"
#include "FeatureFileIOC45.h"
#include "GrayScaleImagesFVProducer.h"
#include "MLClass.h"
#include "ImageDirTree.h"
using namespace  KKMLL;


#include "GrayScaleImagesFV.h"
using namespace  KKMLL;



GrayScaleImagesFV::GrayScaleImagesFV (kkint32  _numOfFeatures):
       FeatureVector (_numOfFeatures),

        centroidCol     (-1),
        centroidRow     (-1),
        numOfEdgePixels (-1)
{
}



GrayScaleImagesFV::GrayScaleImagesFV (const GrayScaleImagesFV&  _fv):
  FeatureVector (_fv),

  centroidCol     (_fv.centroidCol),
  centroidRow     (_fv.centroidRow),
  numOfEdgePixels (_fv.numOfEdgePixels)

{
}



GrayScaleImagesFV::GrayScaleImagesFV (const FeatureVector&  featureVector):
   FeatureVector    (featureVector),
   centroidCol      (-1),
   centroidRow      (-1),
   numOfEdgePixels  (-1)
{
  //if  (strcmp (featureVector.UnderlyingClass (), "GrayScaleImagesFV") == 0)
  if  (typeid (featureVector) == typeid(GrayScaleImagesFV))
  {
    // The underlying class is another GrayScaleImagesFV object.
    const GrayScaleImagesFV&  example = dynamic_cast<const GrayScaleImagesFV&>(featureVector);

    centroidCol      = example.CentroidCol     ();
    centroidRow      = example.CentroidRow     ();
    numOfEdgePixels  = example.NumOfEdgePixels ();
  }
}



GrayScaleImagesFV::~GrayScaleImagesFV ()
{
}



GrayScaleImagesFVPtr  GrayScaleImagesFV::Duplicate ()  const
{
  return new GrayScaleImagesFV (*this);
}



GrayScaleImagesFVList::GrayScaleImagesFVList (FileDescConstPtr  _fileDesc,
                                              bool              _owner
                                             ):

    FeatureVectorList (_fileDesc, _owner)
{

}



GrayScaleImagesFVList::GrayScaleImagesFVList (FactoryFVProducerPtr  _fvProducerFactory,
                                              MLClassPtr           _mlClass,
                                              KKStr                _dirName,
                                              KKStr                _fileName,
                                              RunLog&              _log
                                             ):

   FeatureVectorList (_fvProducerFactory->FileDesc (), true)

{
  FeatureExtraction (_fvProducerFactory, _dirName, _fileName, _mlClass, _log);
}



GrayScaleImagesFVList::GrayScaleImagesFVList (const GrayScaleImagesFVList&  examples):
   FeatureVectorList (examples.FileDesc (), examples.Owner ())
{
  for  (auto imageExample: examples)
  {
    if  (Owner ())
      PushOnBack (new GrayScaleImagesFV (*imageExample));
    else
      PushOnBack (imageExample);
  }
}



GrayScaleImagesFVList::GrayScaleImagesFVList (const GrayScaleImagesFVList&  examples,
                                              bool                          _owner
                                             ):
   FeatureVectorList (examples.FileDesc (), _owner)
{
  for  (auto imageExample: examples)
  {
    if  (Owner ())
      PushOnBack (new GrayScaleImagesFV (*imageExample));
    else
      PushOnBack (imageExample);
  }
}



GrayScaleImagesFVList::GrayScaleImagesFVList (const FeatureVectorList&  featureVectorList,
                                              bool                      _owner
                                             ):

  FeatureVectorList (featureVectorList.FileDesc (),
                     _owner
                    )
{
  if  (_owner)
  {
    for  (auto featureVector: featureVectorList)
    {
      // The constructor below will detect what the underlying type of 'featureVector' is.  
      // If (underlying type is a 'GrayScaleImagesFV' object)  then
      //   | Information that is particular to a 'GrayScaleImagesFV' object will be extracted
      //   | from the 'FeatureVector' object being passed in.
      // else
      //   | info that is particular to a 'GrayScaleImagesFV' object will be set to
      //   | default values.
      GrayScaleImagesFVPtr  example = new GrayScaleImagesFV (*featureVector);
      PushOnBack (example);
    }
  }
  else
  {
    // Since we will not own the contents but just point to an existing instances we will 
    // have to make sure that the existing instances of 'FeatureVector' objects have a 
    // underlying type of 'GrayScaleImagesFV'.
    FeatureVectorList::const_iterator  idx;
    for  (idx = featureVectorList.begin ();  idx != featureVectorList.end ();  idx++)
    {
      FeatureVectorPtr featureVector = *idx;
      //if  (strcmp (featureVector->UnderlyingClass (), "GrayScaleImagesFV") == 0)
      if  (typeid (*featureVector) == typeid (GrayScaleImagesFV))
      {
        GrayScaleImagesFVPtr example = dynamic_cast<GrayScaleImagesFVPtr>(featureVector);
        PushOnBack (example);
      }
      else
      {
        // ****    ERROR  ****
        KKStr  errMSsg = "GrayScaleImagesFVList   One of the elements in 'featureVectorList' is not of 'GrayScaleImagesFV'  type.";
        cerr << endl
             << "GrayScaleImagesFVList::GrayScaleImagesFVList (const FeatureVectorList&  featureVectorList)   ***ERROR***" << endl
             << "        " << errMSsg  << endl
             << "       FileName[" << featureVector->ExampleFileName () << "]"  << endl
             << endl;
        throw KKException (errMSsg);
      }
    }
  }
}



//****************************************************************************
//*  Will Create a list of examples that are a subset of the ones in _examples.  *
//* The subset will consist of the examples who's mlClass is one of the     *
//* ones in mlClasses.                                                    *
//****************************************************************************
GrayScaleImagesFVList::GrayScaleImagesFVList (MLClassList&            _mlClasses,
                                              GrayScaleImagesFVList&  _examples
                                             ):
  FeatureVectorList (_mlClasses, _examples)
  
{
}



GrayScaleImagesFVList::GrayScaleImagesFVList (const FeatureVectorList&  featureVectorList):
  FeatureVectorList (featureVectorList.FileDesc (),
                     featureVectorList.Owner ()
                    )
{
  if  (featureVectorList.Owner ())
  {
    for  (auto featureVector : featureVectorList)
    {
      // The constructor below will detect what the underlying type of 'featureVector' is.  
      // If (underlying type is a 'GrayScaleImagesFV' object)  then
      //   | Information that is particular to a 'GrayScaleImagesFV' object will be extracted
      //   | from the 'FeatureVector' object being passed in.
      // else
      //   | info that is particular to a 'GrayScaleImagesFV' object will be set to
      //   | default values.
      GrayScaleImagesFVPtr  example = new GrayScaleImagesFV (*featureVector);
      PushOnBack (example);
    }
  }
  else
  {
    // Since we will not own the contents but just point to existing instances we will 
    // have to make sure that the existing instances of 'FeatureVector' objects have a 
    // underlying type of 'GrayScaleImagesFV'.
    for  (auto featureVector: featureVectorList)
    {
      if  (typeid (*featureVector) == typeid (GrayScaleImagesFV))
      {
        GrayScaleImagesFVPtr example = dynamic_cast<GrayScaleImagesFVPtr>(featureVector);
        PushOnBack (example);
      }
      else
      {
        // ****    ERROR  ****
        KKStr errMsg;
        errMsg << "GrayScaleImagesFVList   ***ERROR***   One of the elements in 'featureVectorList' is not of 'GrayScaleImagesFV'  type.  We can not recast this element"
             << " FileName[" << featureVector->ExampleFileName () << "]";

        cerr << endl << errMsg << endl << endl;
        throw KKException (errMsg);
      }
    }
  }
}



GrayScaleImagesFVList::~GrayScaleImagesFVList ()
{
}



GrayScaleImagesFVListPtr  GrayScaleImagesFVList::Duplicate (bool _owner)  const
{
  return new GrayScaleImagesFVList (*this, _owner);
}



GrayScaleImagesFVPtr  GrayScaleImagesFVList::IdxToPtr (kkint32 idx)  const
{
  return  (GrayScaleImagesFVPtr)FeatureVectorList::IdxToPtr (idx);
}  /* IdxToPtr */



GrayScaleImagesFVPtr  GrayScaleImagesFVList::BackOfQueue ()
{
  if  (size () < 1)
    return NULL;

  FeatureVectorPtr  fv = back ();
  //if  (strcmp (fv->UnderlyingClass (), "GrayScaleImagesFV") == 0)
  if  (typeid (*fv) == typeid (GrayScaleImagesFV))
    return  dynamic_cast<GrayScaleImagesFVPtr> (fv);

  KKStr errMsg = "GrayScaleImagesFVList::BackOfQueue ()    ***ERROR***        Entry at back of Queue is not a 'GrayScaleImagesFV' object.";
  cerr << endl << errMsg << endl << endl;
  throw  KKException (errMsg);
}  /* BackOfQueue */



GrayScaleImagesFVPtr  GrayScaleImagesFVList::PopFromBack ()
{
  if  (size () < 1)  return NULL;

  FeatureVectorPtr  fv = back ();
  //if  (strcmp (fv->UnderlyingClass (), "GrayScaleImagesFV") != 0)
  if  (typeid (*fv) == typeid (GrayScaleImagesFV))
  {
    KKStr errMsg = "GrayScaleImagesFVList::PopFromBack ()    ***ERROR***        Entry at back of Queue is not a 'GrayScaleImagesFV' object.";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return  dynamic_cast<GrayScaleImagesFVPtr> (FeatureVectorList::PopFromBack ());
}  /* PopFromBack */



void  GrayScaleImagesFVList::AddQueue (GrayScaleImagesFVList&  imagesToAdd)
{
  FeatureVectorList::AddQueue (imagesToAdd);
}  /* AddQueue */



GrayScaleImagesFVPtr  GrayScaleImagesFVList::BinarySearchByName (const KKStr&  _imageFileName)  const
{
  return  (GrayScaleImagesFVPtr)FeatureVectorList::BinarySearchByName (_imageFileName);
}  /* BinarySearchByName */



GrayScaleImagesFVPtr  GrayScaleImagesFVList::LookUpByRootName (const KKStr&  _rootName)
{
  return  (GrayScaleImagesFVPtr)FeatureVectorList::LookUpByRootName (_rootName);
}  /* LookUpByRootName */



GrayScaleImagesFVPtr  GrayScaleImagesFVList::LookUpByImageFileName (const KKStr&  _imageFileName)  const
{
  return  (GrayScaleImagesFVPtr)FeatureVectorList::LookUpByImageFileName (_imageFileName);
}  /* LookUpByImageFileName */



GrayScaleImagesFVListPtr   GrayScaleImagesFVList::ManufactureEmptyList (bool _owner)  const
{
  return new GrayScaleImagesFVList (FileDesc (), _owner);
}



GrayScaleImagesFVListPtr  GrayScaleImagesFVList::OrderUsingNamesFromAFile (const KKStr&  orderedFileName,
                                                                           RunLog&       log
                                                                          )
{
  FeatureVectorListPtr  examples = FeatureVectorList::OrderUsingNamesFromAFile (orderedFileName, log);
  examples->Owner (false);
  GrayScaleImagesFVListPtr  orderedImages = new GrayScaleImagesFVList (*examples);
  delete  examples;
  examples = NULL;
  return  orderedImages;
}  /* OrderUsingNamesFromAFile */



void   GrayScaleImagesFVList::FeatureExtraction (FactoryFVProducerPtr  _fvProducerFactory,
                                                 KKStr                 _dirName, 
                                                 KKStr                 _fileName, 
                                                 MLClassPtr            _mlClass,
                                                 RunLog&               _log
                                                )
{
  KKStr  className = _mlClass->Name ();
  _log.Level (10) << "FeatureExtraction,  dirName   [" << _dirName    << "]." << endl;
  _log.Level (10) << "                    fileName  [" << _fileName   << "]." << endl;
  _log.Level (10) << "                    className [" << className   << "]." << endl;

  bool  cancelFlag  = false;
  bool  successful  = false;

  if  (_dirName.LastChar () != DSchar)
    _dirName << DS;
  
  KKStr  fullFeatureFileName (_dirName);
  fullFeatureFileName << _fileName;

  KKStrListPtr   fileNameList;
  
  KKStr  fileSpec (_dirName);
  fileSpec << "*.*";

  fileNameList = osGetListOfFiles (fileSpec);
  if  (!fileNameList)
    return;

  FeatureVectorProducerPtr  fvProducer = _fvProducerFactory->ManufactureInstance (_log);

  KKStrList::iterator  fnIDX = fileNameList->begin ();

  KKStrPtr imageFileName = NULL;

  kkint32  numOfImages = fileNameList->QueueSize ();
  kkint32  count = 0;

  for  (fnIDX = fileNameList->begin ();   fnIDX != fileNameList->end ();  ++fnIDX)
  {
    if  ((count % 100) == 0)
      cout << className << " " << count << " of " << numOfImages << endl;

    imageFileName = *fnIDX;

    bool validImageFileFormat = SupportedImageFileFormat (*imageFileName);
    
    if  (!validImageFileFormat)
      continue;

    KKStr  fullFileName = osAddSlash (_dirName) + (*imageFileName);

    FeatureVectorPtr featureVector = fvProducer->ComputeFeatureVectorFromImage (fullFileName, _mlClass, NULL, _log);
    if  (!featureVector)
    {
      KKStr  msg (100);
      msg << "GrayScaleImagesFVList::FeatureExtraction   ***ERROR***   Could not Allocate GrayScaleImagesFV object" << endl
          << "for FileName[" << fullFileName << "].";
      _log.Level (-1) << endl << msg << endl << endl;
    }
    else
    {
      GrayScaleImagesFVPtr  larcosFeatureVector = NULL;
      if  (typeid(*featureVector) == typeid(GrayScaleImagesFV))
      {
        larcosFeatureVector = dynamic_cast<GrayScaleImagesFVPtr>(featureVector);
        featureVector = NULL;
      }
      else
      {
        larcosFeatureVector = new GrayScaleImagesFV (*featureVector);
        delete  featureVector;
        featureVector = NULL;
      }

      larcosFeatureVector->ExampleFileName (*imageFileName);
      _log.Level (30) << larcosFeatureVector->ExampleFileName () << "  " << larcosFeatureVector->OrigSize () << endl;
      PushOnBack (larcosFeatureVector);
      count++;
    }
  }

  Version (fvProducer->Version ());

  delete  fvProducer;  fvProducer = NULL;


  kkuint32  numExamplesWritten = 0;

  // WriteImageFeaturesToFile (fullFeatureFileName, RawFormat, FeatureNumList::AllFeatures (fileDesc));
  FeatureFileIOC45::Driver ()->SaveFeatureFile (fullFeatureFileName, 
                                                FeatureNumList::AllFeatures (FileDesc ()), 
                                                *this, 
                                                numExamplesWritten,
                                                cancelFlag,
                                                successful,
                                                _log
                                               );
  delete  fileNameList;
  fileNameList = NULL;
}  /* FeatureExtraction */



/**
 * @brief  Creates a duplicate of list and also duplicates it contents.
 * @return Duplicated list with hard copy of its contents.
 */
GrayScaleImagesFVListPtr  GrayScaleImagesFVList::DuplicateListAndContents ()  const
{
  GrayScaleImagesFVListPtr  copyiedList = new GrayScaleImagesFVList (FileDesc (), true);

  for  (kkint32 idx = 0;  idx < QueueSize ();  idx++)
  {
    GrayScaleImagesFVPtr  curImage = (GrayScaleImagesFVPtr)IdxToPtr (idx);
    copyiedList->PushOnBack (new GrayScaleImagesFV (*curImage));
  }
  
  copyiedList->Version (Version ());

  return  copyiedList;
}  /* DuplicateListAndContents */



void  GrayScaleImagesFVList::RecalcFeatureValuesFromImagesInDirTree (FactoryFVProducerPtr  fvProducerFactory,  
                                                                     const KKStr&          rootDir,
                                                                     bool&                 successful,
                                                                     RunLog&               log
                                                                    )
{
  log.Level (20) << "RecalcFeatureValuesFromImagesInDirTree   RootDir[" << rootDir << "]." << endl;

  successful = false;

  ImageDirTree   fileDirectory (rootDir);

  if  (QueueSize () < 1)
  {
    successful = true;
    return;
  }

  if  (fileDirectory.Size () < 1)
  {
    log.Level (10) << "RecalcFeatureValuesFromImagesInDirTree  No Image Files in[" << rootDir << "]" << endl;
    return;
  }

  FeatureVectorProducerPtr  fvProducer = fvProducerFactory->ManufactureInstance (log);

  KKStrConstPtr  dirPath = NULL;

  iterator  idx;
  GrayScaleImagesFVPtr  example = NULL;

  for  (idx = begin ();   idx != end ();  idx++)
  {
    example = *idx;
    dirPath = fileDirectory.LocateImage (example->ExampleFileName ());
    if  (!dirPath)
    {
      log.Level (10) << "RecalcFeatureValuesFromImagesInDirTree  Could not locate Image[" << example->ExampleFileName () << "] in Subdirectory Tree." << endl;
      return;
    }

    KKStr  fullFileName (*dirPath);
    osAddLastSlash (fullFileName);
    fullFileName << example->ExampleFileName ();

    bool   validFile;
    RasterPtr  raster = new Raster (fullFileName, validFile);
    if  (!validFile)
    {
      delete  raster;  raster = NULL;
      log.Level (-1) << "GrayScaleImagesFVList::RecalcFeatureValuesFromImagesInDirTree   ***ERROR***  Unable to load image: " << fullFileName << endl << endl;
    }
    else
    {
      FeatureVectorPtr fv = fvProducer->ComputeFeatureVector (*raster, example->MLClass (), NULL, 1.0f, log);
      if  (fv)
      {
        kkint32  x;
        kkint32  featureCount = Min (fv->NumOfFeatures (), example->NumOfFeatures ());
        for (x = 0; x < featureCount;  ++x)
          example->FeatureData (x, fv->FeatureData (x));

        example->OrigSize (fv->OrigSize ());
        example->Version (fv->Version ());

        if  (typeid(*fv) == typeid(GrayScaleImagesFV))
        {
          GrayScaleImagesFVPtr  lfv = dynamic_cast<GrayScaleImagesFVPtr>(fv);
          example->CentroidCol (lfv->CentroidCol ());
          example->CentroidRow (lfv->CentroidRow ());
          example->NumOfEdgePixels (lfv->NumOfEdgePixels ());
        }

        delete  fv;
        fv = NULL;
      }
      delete  raster;  raster = NULL;
    }
  }

  delete  fvProducer;
  fvProducer = NULL;
}  /* RecalcFeatureValuesFromImagesInDirTree */



GrayScaleImagesFVListPtr  GrayScaleImagesFVList::ExtractDuplicatesByRootImageFileName ()
{
  FeatureVectorListPtr  duplicateFeatureVectorObjects = FeatureVectorList::ExtractDuplicatesByRootImageFileName ();
  GrayScaleImagesFVListPtr  duplicateImageFeaturesObjects = new GrayScaleImagesFVList (*duplicateFeatureVectorObjects);
  duplicateFeatureVectorObjects->Owner (false);
  delete  duplicateFeatureVectorObjects;  duplicateFeatureVectorObjects = NULL;
  return  duplicateImageFeaturesObjects;
}  /* ExtractDuplicatesByRootImageFileName */



GrayScaleImagesFVListPtr   GrayScaleImagesFVList::ExtractExamplesForAGivenClass (MLClassPtr  _mlClass,
                                                                                   kkint32     _maxToExtract,
                                                                                   float       _minSize
                                                                                  )  const
{
  FeatureVectorListPtr  featureVectorList = FeatureVectorList::ExtractExamplesForAGivenClass (_mlClass, _maxToExtract, _minSize);
  GrayScaleImagesFVListPtr  imageFeaturesList = new GrayScaleImagesFVList (*featureVectorList);
  featureVectorList->Owner (false);
  delete  featureVectorList;  featureVectorList = NULL;
  return  imageFeaturesList;
}  /*  ExtractExamplesForAGivenClass  */



GrayScaleImagesFVListPtr  GrayScaleImagesFVList::StratifyAmoungstClasses (MLClassListPtr  mlClasses,
                                                                          kkint32         maxImagesPerClass,
                                                                          kkint32         numOfFolds,
                                                                          RunLog&         log
                                                                         )
{
  FeatureVectorListPtr  stratifiedFeatureVectors = FeatureVectorList::StratifyAmoungstClasses (mlClasses, maxImagesPerClass, numOfFolds, log);
  GrayScaleImagesFVListPtr  stratifiedImagefeatures  = new GrayScaleImagesFVList (*stratifiedFeatureVectors);
  stratifiedFeatureVectors->Owner (false);
  delete stratifiedFeatureVectors;  stratifiedFeatureVectors = NULL;
  return  stratifiedImagefeatures;
}  /* StratifyAmoungstClasses */



GrayScaleImagesFVListPtr  GrayScaleImagesFVList::StratifyAmoungstClasses (kkint32  numOfFolds,
                                                                          RunLog&  log
                                                                         )
{
  MLClassListPtr  classes = ExtractListOfClasses ();

  FeatureVectorListPtr  stratifiedFeatureVectors = FeatureVectorList::StratifyAmoungstClasses (classes, -1, numOfFolds, log);
  GrayScaleImagesFVListPtr  stratifiedImagefeatures  = new GrayScaleImagesFVList (*stratifiedFeatureVectors);
  
  stratifiedFeatureVectors->Owner (false);

  delete stratifiedFeatureVectors;  stratifiedFeatureVectors = NULL;
  delete  classes;                  classes                  = NULL;

  return  stratifiedImagefeatures;
}  /* StratifyAmoungstClasses */
