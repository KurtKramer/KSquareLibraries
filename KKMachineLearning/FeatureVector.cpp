#include "FirstIncludes.h"
#include <ctype.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "FeatureVector.h"
#include "ClassProb.h"
#include "DuplicateImages.h"
#include "FeatureNumList.h"
#include "FeatureFileIO.h"
#include "MLClass.h"
using namespace  KKMLL;



FeatureVector::FeatureVector (kkuint32  _numOfFeatures):
        featureData      (NULL),
        numOfFeatures    (_numOfFeatures),
        breakTie         (0.0f),
        mlClass          (NULL),
        exampleFileName  (),
        missingData      (false),
        origSize         (0.0f),
        predictedClass   (NULL),
        probability      (-1.0),
        trainWeight      (1.0f),
        validated        (false),
        version          (-1)
{
  AllocateFeatureDataArray ();
}



FeatureVector::FeatureVector (const FeatureVector&  _example):
  featureData      (NULL),
  numOfFeatures    (_example.numOfFeatures),
  breakTie         (_example.breakTie),
  mlClass          (_example.mlClass),
  exampleFileName  (_example.exampleFileName),
  missingData      (false),
  origSize         (_example.origSize),
  predictedClass   (_example.predictedClass),
  probability      (_example.probability),
  trainWeight      (_example.trainWeight),
  validated        (_example.validated),
  version          (-1)
{
  if  (_example.featureData)
  {
    AllocateFeatureDataArray ();
    for  (kkuint32 x = 0; x < numOfFeatures;  ++x)
      featureData[x] = _example.featureData[x];
  }
}



FeatureVector::~FeatureVector ()
{
  delete[] featureData;  featureData = NULL;
}



size_t  FeatureVector::MemoryConsumedEstimated ()  const
{
  size_t  memoryConsumedEstimated = sizeof (FeatureVector)
    +  exampleFileName.MemoryConsumedEstimated ();

  if  (featureData)
    memoryConsumedEstimated += sizeof (float) * numOfFeatures;
  
  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



FeatureVectorPtr  FeatureVector::Duplicate ()  const
{
  return new FeatureVector (*this);
}



void  FeatureVector::ResetNumOfFeatures (kkuint32  newNumOfFeatures)
{
  if  (newNumOfFeatures < 1)  
  {
    KKStr errMsg (128U);
    errMsg << "FeatureVector::ResetNumOfFeatures   ***ERROR***   NewNumOfFeatures[" << newNumOfFeatures << "] is invalid.";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  float*  newFeatureData = new float[newNumOfFeatures];
  for  (kkuint32 x = 0;  x < newNumOfFeatures;  ++x)
  {
    if  (x < numOfFeatures)
      newFeatureData[x] = featureData[x];
    else
      newFeatureData[x] = 0.0f;
  }

  delete  featureData;
  featureData   = newFeatureData;
  numOfFeatures = newNumOfFeatures;
}  /* ResetNumOfFeatures */



void  FeatureVector::AllocateFeatureDataArray ()
{
  if  (featureData)
    delete  featureData;

  featureData = new float [numOfFeatures];

  for  (kkuint32 x = 0;  x < numOfFeatures;  ++x)
    featureData[x] = 0;
}  /* AllocateFeatureDataArray */



float  FeatureVector::FeatureData (kkuint32 featureNum)  const
{
  if  (featureNum >= NumOfFeatures ())
  {
    cerr << endl
         << "*** ERROR ***   FeatureVector::FeatureData(" << featureNum << ")  Index out of bounds." << endl
         << endl;
    return 0.0f;
  }

  return featureData[featureNum];
}  /* FeatureData */



void  FeatureVector::FeatureData (kkuint32  _featureNum,
                                  float     _featureValue
                                 )
{
  KKCheck (_featureNum < NumOfFeatures (), "FeatureVector::FeatureData  _featureNum: " << _featureNum << " index out of bounds.")
  featureData[_featureNum] = _featureValue;
}  /* FeatureData */



float  FeatureVector::TotalOfFeatureData ()  const
{
  float  totalOfFeatureData = 0.0f;
  for  (kkuint32 x = 0;  x < NumOfFeatures ();  ++x)
    totalOfFeatureData += featureData[x];
  return  totalOfFeatureData;
}



void  FeatureVector::AddFeatureData (kkuint32  _featureNum,
                                     float     _featureData
                                    )
{
  KKCheck (_featureNum < numOfFeatures, "FeatureVector::AddFeatureData  _featureNum: " << _featureNum << " index out of bounds;  numOfFeatures: " << numOfFeatures)
  featureData[_featureNum] = _featureData;
}  /* AddFeatureData */



bool  FeatureVector::FeatureDataValid ()
{
  for  (kkuint32 featureNum = 0;  featureNum < numOfFeatures;  featureNum++)
  {
    if  ((featureData[featureNum] == KKB::FloatMin)  ||  (featureData[featureNum] == KKB::FloatMax))
      return  false;
  }

  return true;
}  /* FeatureDataValid */



const KKStr&  FeatureVector::PredictedClassName ()  const
{
  if  (predictedClass)
    return  predictedClass->Name ();
  else
    return  KKStr::EmptyStr ();
}  /* PredictedClasseName */



const KKStr&  FeatureVector::MLClassName  ()  const
{
  if  (mlClass)
    return mlClass->Name ();
  else
    return KKStr::EmptyStr ();
}  /* MLClassName */



const KKStr&  FeatureVector::MLClassNameUpper ()  const
{
  if  (mlClass)
    return mlClass->UpperName ();
  else
    return KKStr::EmptyStr ();
}  /* MLClassNameUpper */



KKStr  FeatureVector::ExampleRootName () const
{
  return KKB::osGetRootName (exampleFileName);
}



bool FeatureVector::operator== (FeatureVector &other_image)  const
{
  if (numOfFeatures != other_image.numOfFeatures)
    return false;

  for (kkuint32 i = 0; i < numOfFeatures; i++)
  {
    if (featureData[i] != other_image.featureData[i])
    {
      return false;
    }
  }

  return true;
}  /* operator== */



FeatureVectorList::FeatureVectorList ():
  KKQueue<FeatureVector> (true),
  curSortOrder  (IFL_SortOrder::IFL_UnSorted),
  fileDesc      (NULL),
  fileName      (),
  numOfFeatures (0),
  version       (-1)
{
}



FeatureVectorList::FeatureVectorList (FileDescConstPtr  _fileDesc,
                                      bool              _owner
                                     ):
  KKQueue<FeatureVector> (_owner),

  curSortOrder  (IFL_SortOrder::IFL_UnSorted),
  fileDesc      (_fileDesc),
  fileName      (),
  numOfFeatures (0),
  version       (-1)
{
  KKCheck (fileDesc, "FeatureVectorList::FeatureVectorList    *** ERROR ***      FileDesc == NULL")
  numOfFeatures = fileDesc->NumOfFields ();
}



FeatureVectorList::FeatureVectorList (FeatureVectorList&  examples):
     KKQueue<FeatureVector> (examples),

     curSortOrder  (IFL_SortOrder::IFL_UnSorted),
     fileDesc      (examples.fileDesc),
     fileName      (examples.fileName),
     numOfFeatures (examples.numOfFeatures),
     version       (examples.version)
{
}



/**
 *@brief  Constructor that create a duplicate list of FeatureVectors;  
 *@details 
 *if (owner == true)  then will also
 *   -# duplicate the contents; that is each example in the list will be 
 *      duplicated by calling the copy constructor.
 *   -# And the new list will own these newly created examples.
 */
FeatureVectorList::FeatureVectorList (const FeatureVectorList&  examples,
                                      bool                      _owner
                                     ):
   KKQueue<FeatureVector> (examples, _owner),

   curSortOrder  (IFL_SortOrder::IFL_UnSorted),
   fileDesc      (examples.fileDesc),
   fileName      (examples.fileName),
   numOfFeatures (examples.numOfFeatures),
   version       (examples.version)
{
}



/**
 *@brief  Constructs a list of examples that are a subset of the ones in _examples as dictated by _mlClasses
 *@details
 * The subset will consist of the examples who's mlClass is one of the ones in mlClasses. The new 
 * instance created will not own the contents;  it will just point to the existing examples that were in
 * '_examples'.
 *
 *@param[in] _mlClasses  List of classes that the subset of examples will come from.
 *@param[in] _examples  Examples that the subset will be derived from.
 */
FeatureVectorList::FeatureVectorList (MLClassList&        _mlClasses,
                                      FeatureVectorList&  _examples
                                     ):
  KKQueue<FeatureVector> (false),

  curSortOrder  (IFL_SortOrder::IFL_UnSorted),
  fileDesc      (_examples.fileDesc),
  fileName      (_examples.fileName),
  numOfFeatures (_examples.numOfFeatures),
  version       (_examples.version)
{
  MLClassIndexList  classIdx  (_mlClasses);
  for  (auto idx:  _examples)
  {
    if  (classIdx.GetClassIndex (idx->MLClass ()) >= 0)
      PushOnBack (idx);
  }
}



FeatureVectorList::~FeatureVectorList ()
{
}



size_t  FeatureVectorList::MemoryConsumedEstimated ()  const
{
  size_t  memoryConsumedEstimated = sizeof (FeatureVectorList) + fileName.MemoryConsumedEstimated ();
  FeatureVectorList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    FeatureVectorPtr  fv = *idx;
    memoryConsumedEstimated += fv->MemoryConsumedEstimated ();
  }
  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



FeatureVectorListPtr  FeatureVectorList::Duplicate (bool _owner)  const
{
  return new FeatureVectorList (*this, _owner);
}



FeatureVectorListPtr  FeatureVectorList::ManufactureEmptyList (bool _owner)  const
{
  return new FeatureVectorList (fileDesc, _owner);
}



/**
 *@details   
 *  Determines if the other FeatreVectorList has the same underlining layout;  that is each
 *  field is of the same type and meaning. This way we can determine if one list contains
 *  Apples while the other contains Oranges.
 */
bool  FeatureVectorList::SameExceptForSymbolicData (const FeatureVectorList&  otherData,
                                                    RunLog&                   log
                                                   )  const
{
  return  fileDesc->SameExceptForSymbolicData (*(otherData.FileDesc ()), log);
}



void  FeatureVectorList::RemoveEntriesWithMissingFeatures (RunLog&  log)
{
  log.Level (50) << "FeatureVectorList::RemoveEntriesWithMissingFeatures" << endl;

  vector<FeatureVectorPtr>  entriesToBeDeleted;

  for  (iterator idx = begin ();  idx != end ();  idx++)
  {
    FeatureVectorPtr example = *idx;
    if  (example->MissingData ())
      entriesToBeDeleted.push_back (example);
  }

  for  (kkint32 x = 0;  x < (kkint32)entriesToBeDeleted.size ();  x++)
  {
    FeatureVectorPtr example = entriesToBeDeleted[x];
    DeleteEntry (example);
    if  (Owner ())
      delete  example;
  }

}  /* RemoveEntriesWithMissingFeatures */



void  FeatureVectorList::ValidateFileDescAndFieldNum (kkint32      fieldNum,
                                                      const char*  funcName
                                                     )  const
{
  if  (!fileDesc)
  {
    // This should never ever be able to happen,  but will check 
    // any way.  If missing something has gone very wrong.
    KKStr  msg (200U);
    msg << "FeatureVectorList::" << funcName << "      *** ERROR ***  'fileDesc == NULL'";
    throw KKException (msg);
  }

  if  ((fieldNum < 0)  ||  (fieldNum >= (kkint32)fileDesc->NumOfFields ()))
  {
    KKStr  msg (200U);
    msg << "FeatureVectorList::" << funcName << "    *** ERROR ***    FeatureNum[" << fieldNum << "] is out of range.";
    throw KKException (msg);
  }
} /* ValidateFileDescAndFieldNum */



bool   FeatureVectorList::AllFieldsAreNumeric ()  const
{
  return  fileDesc->AllFieldsAreNumeric ();
}



const KKStr&  FeatureVectorList::FieldName (kkint32 featureNum) const
{
  ValidateFileDescAndFieldNum (featureNum, "FieldName");
  return  fileDesc->FieldName (featureNum);
}  /* FeatureName */



AttributeType  FeatureVectorList::FeatureType (kkint32 featureNum) const
{
  ValidateFileDescAndFieldNum (featureNum, "FeatureType");
  return  fileDesc->Type (featureNum);
}  /* FeatureType */



KKStr  FeatureVectorList::FeatureTypeStr (kkint32 featureNum) const
{
  ValidateFileDescAndFieldNum (featureNum, "FeatureTypeStr");
  return  AttributeTypeToStr (fileDesc->Type (featureNum));
}  /* FeatureType */



kkint32 FeatureVectorList::FeatureCardinality (kkint32 featureNum)  const
{
  ValidateFileDescAndFieldNum (featureNum, "FeatureCardinality");
  return  fileDesc->Cardinality (featureNum);
}



AttributeTypeVector  FeatureVectorList::CreateAttributeTypeTable ()  const
{
  ValidateFileDescAndFieldNum (0, "CreateAttributeTypeTable");
  return  fileDesc->CreateAttributeTypeTable ();
}  /* CreateAttributeTypeTable */



vector<kkint32>  FeatureVectorList::CreateCardinalityTable ()  const
{
  ValidateFileDescAndFieldNum (0, "CreateCardinalityTable");
  return  fileDesc->CreateCardinalityTable ();
}  /* CreateAttributeTypeTable */



FeatureNumList  FeatureVectorList::AllFeatures ()
{
  return FeatureNumList::AllFeatures (fileDesc);
}



void  FeatureVectorList::ResetNumOfFeaturs (kkint32 newNumOfFeatures)
{
  numOfFeatures = newNumOfFeatures;

  for  (iterator idx = begin ();  idx != end ();  idx++)
  {
    FeatureVectorPtr  i = *idx;
    i->ResetNumOfFeatures (newNumOfFeatures);
  }
}  /* ResetNumOfFeaturs */



void  FeatureVectorList::ResetFileDesc (FileDescConstPtr  newFileDesc)
{
  KKCheck (newFileDesc, "FeatureVector::ResetFileDesc   ***ERROR***   newFileDesc == NULL.")

  fileDesc = newFileDesc;

  numOfFeatures = fileDesc->NumOfFields ();

  for  (iterator idx = begin ();  idx != end ();  idx++)
  {
    FeatureVectorPtr  i = *idx;
    i->ResetNumOfFeatures (numOfFeatures);
  }
}  /* ResetFileDesc */



void  FeatureVectorList::PushOnBack (FeatureVectorPtr  example)
{
  KKCheck (example->NumOfFeatures () == numOfFeatures, "FeatureVectorList::PushOnBack   Mismatch numOfFeatures: " << numOfFeatures << " example->NumOfFeaturess: " << example->NumOfFeatures ())
  KKQueue<FeatureVector>::PushOnBack (example);
  curSortOrder = IFL_SortOrder::IFL_UnSorted;
}  /* Push On Back */



void  FeatureVectorList::PushOnFront (FeatureVectorPtr  example)
{
  KKCheck (example->NumOfFeatures () == numOfFeatures, "FeatureVectorList::PushOnFront   Mismatch numOfFeatures: " << numOfFeatures << " example->NumOfFeaturess: " << example->NumOfFeatures ())
  KKQueue<FeatureVector>::PushOnFront (example);
  curSortOrder = IFL_SortOrder::IFL_UnSorted;
}  /* PushOnFront */



MLClassListPtr  FeatureVectorList::ExtractListOfClasses ()  const
{
  MLClassPtr  lastClass = NULL;
  map<MLClassPtr,MLClassPtr>  ptrIndex;
  map<MLClassPtr,MLClassPtr>::iterator  ptrIndexItr;
  FeatureVectorList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    FeatureVectorPtr example = *idx;
    MLClassPtr  newClass = example->MLClass ();
    if  (newClass == lastClass)
      continue;

    lastClass  = newClass;
    ptrIndexItr = ptrIndex.find (newClass);
    if  (ptrIndexItr == ptrIndex.end ())
    {
      lastClass = newClass;
      ptrIndex.insert (pair<MLClassPtr,MLClassPtr> (newClass, newClass));
    }
  }

  MLClassListPtr  classes = new MLClassList ();
  for  (ptrIndexItr = ptrIndex.begin ();  ptrIndexItr != ptrIndex.end ();  ++ptrIndexItr)
    classes->PushOnBack (ptrIndexItr->first);

  return   classes;
}  /* ExtractListOfClasses */



void  FeatureVectorList::AddSingleExample (FeatureVectorPtr  _imageFeatures)
{
  PushOnBack (_imageFeatures);
  curSortOrder = IFL_SortOrder::IFL_UnSorted;
}



void  FeatureVectorList::RemoveDuplicateEntries (bool     allowDupsInSameClass,
                                                 RunLog&  runLog
                                                )
{
  DuplicateImages  dupDetector (this, runLog);
  dupDetector.PurgeDuplicates (this, allowDupsInSameClass, NULL);
}  /* RemoveDuplicateEntries */



void  FeatureVectorList::AddQueue (const FeatureVectorList&  examplesToAdd)
{
  KKCheck (numOfFeatures == examplesToAdd.NumOfFeatures (),
           "AddQueue   'examplesToAdd' different numOfFeatures: " << numOfFeatures << " examplesToAdd.numOfFeatures: " << examplesToAdd.numOfFeatures)

  if  (QueueSize () < 1)
  {
    // Since we don't have any examples in the existing queue then the version number
    // of the queue that we are adding will be our version number.
    Version (examplesToAdd.Version ());
  }
  else
  {
    // Since there are examples in both queues, then they should have the same version number
    // otherwise the version number of the resulting KKQueue will be undefined (-1).
    if  (examplesToAdd.Version () != Version ())
      Version (-1);
  }

  KKQueue<FeatureVector>::AddQueue (examplesToAdd);
}  /* AddQueue */



kkint32  FeatureVectorList::GetClassCount (MLClassPtr  c)  const
{
  kkint32  count =0;
  FeatureVectorList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    if  ((*idx)->MLClass () == c)
      count++;
  }

  return  count;
}  /* GetClassCount */



FeatureVectorListPtr   FeatureVectorList::ExtractExamplesForHierarchyLevel (kkuint32 level)
{
  FeatureVectorListPtr  examples = ManufactureEmptyList (true);
  FeatureVectorList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    const FeatureVectorPtr  fv = *idx;

    FeatureVectorPtr  newFV = new FeatureVector (*fv);
    newFV->MLClass (fv->MLClass ()->MLClassForGivenHierarchialLevel ((kkuint16)level));
    examples->PushOnBack (newFV);
  }

  return  examples;
}  /* ExtractExamplesForHierarchyLevel */



FeatureVectorListPtr  FeatureVectorList::ExtractExamplesForAGivenClass (MLClassPtr  _mlClass,
                                                                        kkuint32    _maxToExtract,
                                                                        float       _minSize
                                                                       )  const
{
  if  (_maxToExtract < 1)
    _maxToExtract = uint32_max;

  // Create a new list structure that does not own the Images it contains.  This way when 
  // this structure is deleted.  The example it contains are not deleted.
  FeatureVectorListPtr  extractedImages = this->ManufactureEmptyList (false);
  KKCheck (extractedImages, "ExtractExamplesForAGivenClass,  Could not allocate more space.")

  kkuint32 qSize = QueueSize ();
  for  (kkuint32 idx = 0;  ((idx < qSize));  idx++)
  {
    FeatureVectorPtr example = IdxToPtr (idx);
    if  (((example->MLClass () == _mlClass)  ||  (!_mlClass)))
    {
      if  (example->OrigSize () >= _minSize)
      {
        extractedImages->AddSingleExample (example);
        if  (extractedImages->QueueSize () >= _maxToExtract)
          break;
      }
    }
  }

  return  extractedImages;
}  /*  ExtractExamplesForAGivenClass  */



FeatureVectorListPtr  FeatureVectorList::ExtractExamplesForClassList (MLClassListPtr  classes)
{
  FeatureVectorListPtr  subSetForClassList =  ManufactureEmptyList (false);
  for  (auto idx: *classes)
  {
    FeatureVectorListPtr  examplesForClass = ExtractExamplesForAGivenClass (idx);
    if  (examplesForClass)
      subSetForClassList->AddQueue (*examplesForClass);
  }
  return  subSetForClassList;
}  /* ExtractExamplesForClassList */



KKStrListPtr   FeatureVectorList::ExtractDuplicatesByExampleFileName () 
{
  SortByImageFileName ();

  KKStrListPtr  duplicateExamples = new KKStrList (true);

  if  (QueueSize () < 2)
    return  duplicateExamples;

  FeatureVectorList::iterator  iIDX = this->begin ();

  while  (iIDX != end ())
  {
    FeatureVectorPtr  example = *iIDX;  ++iIDX;
    if  (!example)
      continue;

    KKStr  lastFileName = example->ExampleFileName ();

    if  (example->ExampleFileName () == lastFileName)
    {
      duplicateExamples->PushOnBack (new KKStr (example->ExampleFileName ()));

      if  (iIDX != end ())
      {
        example = *iIDX;
        ++iIDX; 
      }
      else
      {
        example = NULL;
      }

      while  ((example != NULL)   &&   (example->ExampleFileName () == lastFileName))
      {
        if  (iIDX != end ())
        {
          example = *iIDX;
          ++iIDX;
        }
        else
        {
          example = NULL;
        }
      }
    }
  }

  return  duplicateExamples;
}  /*  ExtractDuplicateImageFileNames  */



FeatureVectorPtr  FeatureVectorList::BinarySearchByName (const KKStr&  _imageFileName)  const
{
  KKCheck ((curSortOrder == IFL_SortOrder::IFL_ByName)  ||  (curSortOrder == IFL_SortOrder::IFL_ByRootName),
           "FeatureVectorList::BinarySearchByName    Invalid Sort Order.");

  if  (QueueSize () < 1)
    return NULL;

  kkuint32  low  = 0;
  kkuint32  high = QueueSize () - 1;
  kkuint32  mid;

  FeatureVectorPtr  example = NULL;

  while  (true)
  {
    mid = (low + high) / 2;

    example = IdxToPtr (mid);

    if  (example->ExampleFileName () < _imageFileName)
    {
      if  (mid >= high)  break;
      low = mid + 1;
    }

    else if  (example->ExampleFileName () > _imageFileName)
    {
      if  (mid <= low)  break;
      high = mid - 1;
    }

    else
      return  example;
  }

  return  NULL;
}  /* BinarySearchByName */



FeatureVectorPtr  FeatureVectorList::LookUpByRootName (const KKStr&  _rootName)
{
  FeatureVectorPtr  example = NULL;

  if  (curSortOrder != IFL_SortOrder::IFL_ByRootName)
  {
    cerr << endl
         << "FeatureVectorList::LookUpByRootName   ***WARNING***  List is NOT SORTED by RootName."  << endl
         << endl;

    FeatureVectorList::iterator  idx;
    for  (idx = begin ();  idx != end ();  idx++)
    {
      example = *idx;
      if  (_rootName == osGetRootName (example->ExampleFileName ()))
        return example;
    }
    return NULL;
  }
  else
  {
    kkint32  low  = 0;
    kkint32  high = QueueSize () - 1;
    kkint32  mid;

    while  (low <= high)
    {
      mid = (low + high) / 2;

      example = IdxToPtr (mid);

      KKStr  tempName = osGetRootName (example->ExampleFileName ());

      if  (tempName < _rootName)
      {
        low = mid + 1;
      }

      else if  (tempName > _rootName)
      {
        high = mid - 1;
      }

      else
      {
        return  example;
      }
    }
  }

  return  NULL;
}  /* LookUpByRootName */



FeatureVectorPtr  FeatureVectorList::LookUpByImageFileName (const KKStr&  _imageFileName)  const
{
  if  (curSortOrder == IFL_SortOrder::IFL_ByName)
    return  BinarySearchByName (_imageFileName);
  else
  {
    FeatureVectorPtr   example = NULL;
    for  (auto tempExample:  *this)
    {
      if  (tempExample->ExampleFileName () == _imageFileName)
         example = tempExample;
    }
    return  example;
  }
}  /* LookUpByImageFileName */



FeatureVectorListPtr  FeatureVectorList::OrderUsingNamesFromAFile (const KKStr&  orderedFileName,
                                                                   RunLog&       log
                                                                  )
{
  FILE*  in = osFOPEN (orderedFileName.Str (), "r");
  if  (!in)
  {
    log.Level (-1) << endl
                   << "FeatureVectorList::OrderUsingNamesFromAFile   *** ERROR ***" << endl
                   << "                               Could not open file[" << orderedFileName << "]." << endl
                   << endl;
    return NULL;
  }

  FeatureVectorPtr      example = NULL;
  FeatureVectorListPtr  orderedImages = ManufactureEmptyList (false);

  char buff[1024];
  while  (fgets (buff, sizeof (buff), in))
  {
    KKStr  txtLine (buff);

    if  (txtLine.StartsWith ("//"))
    {
      // Comment line, will ignore.
      continue;
    }

    KKStr exampleFileName = txtLine.ExtractToken ("\n\r\t");
    if  (orderedImages->LookUpByImageFileName (exampleFileName))
    {
      // Image file name used more than once, will treat as error
      log.Level (-1) << endl
                     << "FeatureVectorList::OrderUsingNamesFromAFile   *** ERROR ***" << endl
                     << "                      ExampleFileName[" << exampleFileName << "] occurred more than once in file." << endl
                     << endl;
      fclose (in);
      delete  orderedImages;
      return NULL;
    }

    example = LookUpByImageFileName (exampleFileName);
    if  (!example)
    {
      // Image file name not in list, will treat as error.
      log.Level (-1) << endl
                     << "FeatureVectorList::OrderUsingNamesFromAFile   *** ERROR ***" << endl
                     << "                      ExampleFileName[" << exampleFileName << "] Not in list." << endl
                     << endl;
      fclose (in);
      delete  orderedImages;
      return NULL;
    }

    orderedImages->PushOnBack (example);
  }

  fclose (in);

  return  orderedImages;
}  /* OrderUsingNamesFromAFile */




void  FeatureVectorList::SaveOrderingOfImages (const KKStr&  _fileName,
                                               bool&          _successful
                                              )
{
  _successful = true;

  ofstream o (_fileName.Str ());
  if  (!o.is_open ())
  {
    _successful = false;
    return;
  }

  o << "// " << "Time Written  [" << osGetLocalDateTime () << "]" << endl;
  o << "// " << "File Name     [" << _fileName             << "]" << endl;
  o << "// " << "Size          [" << QueueSize ()          << "]" << endl;
  o << "//"                                                       << endl;


  FeatureVectorList::iterator  idx;

  for  (idx = begin ();  idx != end ();  idx++)
    o << (*idx)->ExampleFileName () << endl;

  o.close ();

  return;
}  /* OrderUsingNamesFormAFile */



/**
 *@brief  Creates a duplicate of list and also duplicates it contents.
 *@return Duplicated list with hard copy of its contents.
 */
FeatureVectorListPtr  FeatureVectorList::DuplicateListAndContents ()  const
{
  FeatureVectorListPtr  copyiedList = ManufactureEmptyList (true);
  for  (kkuint32 idx = 0;  idx < QueueSize ();  idx++)
  {
    FeatureVectorPtr  curImage = IdxToPtr (idx);
    copyiedList->AddSingleExample (new FeatureVector (*curImage));
  }
  
  return  copyiedList;
}  /* DuplicateListAndContents */



KKStr  FeatureVectorList::ClassStatisticsStr ()  const
{
  ClassStatisticListPtr  stats = GetClassStatistics ();

  KKStr  s (stats->QueueSize () * 30 + 100);

  s << "Total_Images\t"  << QueueSize ()    << endl;
  s << endl; 
  s << "Total_Classes\t" << stats->QueueSize () << endl;
  s << endl;

  s << "Class_Name" << "\t" << "Count" << endl;
  ClassStatisticList::iterator  statsIDX;
  for  (statsIDX = stats->begin (); statsIDX != stats->end ();  statsIDX++)
  {
    ClassStatisticPtr  cs = *statsIDX;
    s << cs->Name () << "\t" << cs->Count () << endl;
  }

  delete  stats;
  return  s;
}  /* ClassStatisticsStr */



ClassStatisticListPtr  FeatureVectorList::GetClassStatistics ()  const
{
  ClassStatisticListPtr  classStatistics = new ClassStatisticList (true);

  MLClassPtr        mlClass = NULL;
  FeatureVectorPtr  example = NULL;

  FeatureVectorList::const_iterator  idx;

  for  (idx = begin ();  idx != end ();  ++idx)
  {
    example = *idx;
    mlClass = example->MLClass ();
    
    ClassStatisticPtr  classStatistic = classStatistics->LookUpByMLClass (mlClass);
    if  (classStatistic == NULL)
    {
      classStatistic = new ClassStatistic (mlClass, 0);
      classStatistics->PushOnBack (classStatistic);
    }

    classStatistic->Increment ();
  }


  classStatistics->SortByClassName ();

  return  classStatistics;
}  /* GetClassStatistics */



ClassProbListPtr  FeatureVectorList::GetClassDistribution () const
{
  ClassStatisticListPtr  stats = GetClassStatistics ();
  ClassProbListPtr  distribution = new ClassProbList (true);

  kkuint32 countTotal = 0;
  ClassStatisticList::const_iterator  idx;
  for  (idx = stats->begin ();  idx != stats->end ();  ++idx)
  {
    ClassStatisticPtr cs = *idx;
    countTotal += cs->Count ();
  }

  if  (countTotal > 0)
  {
    for  (idx = stats->begin ();  idx != stats->end ();  ++idx)
    {
      ClassStatisticPtr cs = *idx;
      distribution->PushOnBack (new ClassProb (cs->MLClass (), ((float)(cs->Count ()) / (float)countTotal), 0));
    }
  }

  delete  stats;
  stats = NULL;

  return  distribution;
}  /* GetClassDistribution */



FeatureVectorListPtr  FeatureVectorList::ExtractDuplicatesByRootImageFileName ()
{
  FeatureVectorListPtr  duplicateList = this->ManufactureEmptyList (false);

  if  (QueueSize () < 2)
  {
    // Since there are less than 2 elements in the list,  
    // There is no way that there can be any duplicates in the list.
    return  duplicateList;
  }

  FeatureVectorList  workList (*this, 
                               false    // owner = false,  only create a list of pointers to existing instances
                              );  // 

  workList.SortByRootName (false);
  
  FeatureVectorList::iterator  idx;
  idx = workList.begin ();

  FeatureVectorPtr  lastExample = *idx;  ++idx;
  FeatureVectorPtr  example   = *idx;  ++idx;

  KKStr  lastRootName = osGetRootName (lastExample->ExampleFileName ());
  KKStr  rootName;

  while  (example)
  {
    rootName = osGetRootName (example->ExampleFileName ());
    if  (rootName != lastRootName)
    {
      lastRootName = rootName;
      lastExample    = example;
      if  (idx == workList.end ())
        example = NULL;
      else
      {
        example = *idx;
        ++idx;
      }
    }
    else
    {
      duplicateList->PushOnBack (lastExample);
      while  ((example != NULL)  &&  (rootName == lastRootName))
      {
        duplicateList->PushOnBack (example);
        if  (idx == workList.end ())
          example = NULL;
        else
        {
          example = *idx;  
          ++idx;
        }

        if  (example)
          rootName = osGetRootName (example->ExampleFileName ());
      }
    }
  }

  return  duplicateList;
}  /* ExtractDuplicatesByRootImageFileName */



void  SplitImagesAmongstFolds (kkint32                numOfFolds,
                               kkint32                maxImagesPerClass,
                               FeatureVectorListPtr*  folds,
                               FeatureVectorListPtr   src
                              )
{
  src->RandomizeOrder ();
  src->RandomizeOrder ();

  kkint32  imagesInThisList = src->QueueSize ();
  if  (maxImagesPerClass > 0)
    imagesInThisList = Min (imagesInThisList, maxImagesPerClass);

  kkint32  x;

  for  (x = 0; x < imagesInThisList; x++)
  {
    folds[x % numOfFolds]->AddSingleExample (src->IdxToPtr (x));
  }
}  /* SplitImagesAmongstFolds */



void  FeatureVectorList::CalcStatsForFeatureNum (kkuint32 _featureNum,
                                                 kkint32& _count,
                                                 float&   _total,
                                                 float&   _mean,
                                                 float&   _var,
                                                 float&   _stdDev
                                                )
{
  _count  = 0;
  _total  = 0.0f;
  _mean   = 0.0f;
  _var    = 0.0f;
  _stdDev = 0.0f;

  if  (_featureNum >= NumOfFeatures ())
  {
    cerr << endl
         << "FeatureVectorList::CalcStatsForFeatureNum    *** ERROR ***  Invalid FeatureNum[" << _featureNum << "]" << endl
         << "                                            FeatureNum   [" << _featureNum      << "]" << endl
         << "                                            NumOfFeatures[" << NumOfFeatures () << "]" << endl
         << endl;
    _count = -1;
    return;
  }

  if  (QueueSize () == 0)
    return;
    
  iterator  idx;

  for  (idx = begin (); idx != end ();  idx++)
  {
    FeatureVectorPtr  i = *idx;
    _total += i->FeatureData (_featureNum);
    _count++;
  }

  if  (_count < 1)
  {
    _mean   = 0.0f;
    _var    = 0.0f;
    _stdDev = 0.0f;
    return;
  }

  _mean = _total / (float)_count;

  float  totalSquareDelta = 0.0f;
  for  (idx = begin (); idx != end ();  idx++)
  {
    FeatureVectorPtr  i = *idx;
    float  delta = i->FeatureData (_featureNum) - _mean;
    totalSquareDelta += delta * delta;
  }

  _var     = totalSquareDelta / (float)_count;
  _stdDev  = sqrt (_var);
}  /* CalcStatsForFeatureNum */



FeatureVectorListPtr  FeatureVectorList::StratifyAmoungstClasses (kkint32  numOfFolds,
                                                                  RunLog&  log
                                                                 )
{
  MLClassListPtr  classes = ExtractListOfClasses ();

  FeatureVectorListPtr stratifiedExamples = StratifyAmoungstClasses (classes, -1, numOfFolds, log);
  delete  classes;  classes = NULL;

  return  stratifiedExamples;
}  /* StratifyAmoungstClasses */



FeatureVectorListPtr  FeatureVectorList::StratifyAmoungstClasses (MLClassListPtr  mlClasses,
                                                                  kkint32         maxImagesPerClass,
                                                                  kkuint32        numOfFolds,
                                                                  RunLog&         log
                                                                 )
{
  log.Level (10) << "FeatureVectorList::StratifyAmoungstClasses" << endl;

  FeatureVectorListPtr*  folds = new FeatureVectorListPtr[numOfFolds];
  for  (kkuint32 x = 0;  x < numOfFolds;  ++x)
    folds[x] =  ManufactureEmptyList (false);

  MLClassPtr  mlClass = NULL;
  MLClassList::iterator  icIDX;

  for  (icIDX = mlClasses->begin ();  icIDX != mlClasses->end ();  ++icIDX)
  {
    mlClass = *icIDX;
    FeatureVectorListPtr  imagesInClass = ExtractExamplesForAGivenClass (mlClass);

    if  (imagesInClass->QueueSize () < numOfFolds)
    {
      log.Level (-1) << endl
                     << "FeatureVectorList::DistributesImagesRandomlyWithInFolds    ***ERROR***" << endl
                     << endl
                     << "*** ERROR ***,  Not enough examples to split amongst the different folds." << endl
                     << "                Class           [" << mlClass->Name ()         << "]."  << endl
                     << "                Number of Images[" << imagesInClass->QueueSize () << "]."  << endl
                     << "                Number of Folds [" << numOfFolds                  << "]."  << endl
                     << endl;

      KKStr  msg;
      msg << "Not enough Images[" << imagesInClass->QueueSize ()  << "] "
          << "for Class["         << mlClass->Name ()          << "] "
          << "to distribute in Folds.";

      if  (!osIsBackGroundProcess ())
         osDisplayWarning (msg);
    }

    imagesInClass->RandomizeOrder ();
    imagesInClass->RandomizeOrder ();

    SplitImagesAmongstFolds (numOfFolds, maxImagesPerClass, folds, imagesInClass);
    delete  imagesInClass;
    imagesInClass = NULL;
  }

  FeatureVectorListPtr stratafiedImages = ManufactureEmptyList (false);

  for  (kkuint32 foldNum = 0; foldNum < numOfFolds;  foldNum++)
  {
    folds[foldNum]->RandomizeOrder ();
    folds[foldNum]->RandomizeOrder ();
    stratafiedImages->AddQueue (*folds[foldNum]);
    delete  folds[foldNum];
    folds[foldNum] = NULL;
  }

  delete[]  folds;
  folds = NULL;
  return  stratafiedImages;
}  /* StratifyAmoungstClasses */



float    FeatureVectorList::MajorityClassFraction ()  const
{
  ClassStatisticListPtr  allClassStats = GetClassStatistics ();

  if  (!allClassStats)
    return 0.0f;

  kkuint32 largestClassSize = 0;

  ClassStatisticList::const_iterator  idx;
  for  (auto classStats: *allClassStats)
  {
    if  (classStats->Count () > largestClassSize)
      largestClassSize = classStats->Count ();
  }

  float  fraction = float (largestClassSize) / float (size ());

  delete  allClassStats;
  allClassStats = nullptr;

  return  fraction;
}  /* MajorityClassFraction */




void  FeatureVectorList::PrintClassStatistics (ostream&  o)  const
{
  ClassStatisticListPtr  stats = GetClassStatistics ();

  o << "Total_Images\t"  << QueueSize ()    << endl;
  o << endl; 
  o << "Total_Classes\t" << stats->QueueSize () << endl;
  o << endl;

  ClassStatisticList::const_iterator  statsIDX;
  kkint32  index = 0;

  o << "Class_Name" << "\t" << "Index" << "\t" << "Count" << endl;
  for  (statsIDX = stats->begin (); statsIDX != stats->end ();  ++statsIDX)
  {
    o << (*statsIDX)->Name () << "\t" << index << "\t" << (*statsIDX)->Count () << endl;
    index++;
  }

  delete  stats;
  stats = NULL;
  return;
}  /* PrintClassStatistics */



void  FeatureVectorList::PrintClassStatisticsHTML (ostream&  o)  const
{
  ClassStatisticListPtr  stats = GetClassStatistics ();

  o << "<table align=\"center\"  border=\"2\" cellpadding=\"4\">"  << endl
    << "<thead>" << endl;

  o << "<tr>"
    << "<th align=\"left\">Class<br />Name</th>"
    << "<th align=\"center\">Index</th>"
    << "<th align=\"center\">Count</th>"
    << "</tr>" 
    << endl
    << "</thead>" << endl;

  o << "<tbody>" << endl;

  ClassStatisticList::iterator  statsIDX;
  kkint32  index = 0;
  for  (statsIDX = stats->begin (); statsIDX != stats->end ();  statsIDX++)
  {
    ClassStatistic& statistic = **statsIDX;
    o << "<tr>"
      << "<td align=\"left\">"    << statistic.Name ()  << "</td>"
      << "<td align=\"center\">"  << index              << "</td>" 
      << "<td align=\"center\">"  << statistic.Count () << "</td>" 
      << "</tr>"
      << endl;
    index++;
  }

  o << "<tr>"
    << "<td align=\"left\">"    << "Total"      << "</td>"
    << "<td align=\"center\">"  << "&nbsp"      << "</td>" 
    << "<td align=\"center\">"  << QueueSize () << "</td>" 
    << "</tr>"
    << endl;

  o << "</tbody>" << endl;
  o << "</table>" << endl;
  
  delete  stats;
  return;
}  /* PrintClassStatisticsHTML */



void  FeatureVectorList::PrintFeatureStatisticsByClass (ostream&  o)  const
{
  o << "Class"      << "\t"
    << "ClassIdx"   << "\t"
    << "FeatureNum" << "\t"
    << "FieldName"  << "\t"
    << "Type"       << "\t"
    << "Count"      << "\t"
    << "Total"      << "\t"
    << "Mean"       << "\t"
    << "Var"        << "\t"
    << "StdDev"
    << endl;

  MLClassListPtr  mlClasses = ExtractListOfClasses ();   

  MLClassList::const_iterator cIDX;

  kkint32  classIdx = 0;

  for  (cIDX = mlClasses->begin ();  cIDX != mlClasses->end ();  cIDX++)
  {
    MLClassPtr  mlClass = *cIDX;
    FeatureVectorListPtr  imagesThisClass = ExtractExamplesForAGivenClass (mlClass);
      
    for  (kkuint32 featureNum = 0;  featureNum < imagesThisClass->NumOfFeatures ();  featureNum++)
    {
      kkint32  count;
      float  total, mean, var, stdDev;
      imagesThisClass->CalcStatsForFeatureNum (featureNum, count, total, mean, var, stdDev);
      o << mlClass->Name ()          << "\t"
        << classIdx                     << "\t"
        << featureNum                   << "\t"
        << FieldName (featureNum)       << "\t"
        << FeatureTypeStr (featureNum)  << "\t"
        << count                        << "\t"
        << total                        << "\t"
        << mean                         << "\t"
        << var                          << "\t"
        << stdDev
        << endl;
    }

    delete  imagesThisClass;
    o << endl;

    classIdx++;
  }

  delete  mlClasses;
  mlClasses = NULL;
}  /* PrintFeatureStatisticsByClass */



VectorDouble  FeatureVectorList::ExtractMeanFeatureValues ()
{
  FeatureVectorList::const_iterator  idx;

  VectorDouble  totals (this->NumOfFeatures (), 0.0);
  VectorDouble  means  (this->NumOfFeatures (), 0.0);

  for  (idx = begin ();  idx != end ();  idx++)
  {
    const FeatureVectorPtr fv = *idx;
    const float*  fd = fv->FeatureDataConst ();

    for  (kkuint32 fn = 0;  fn < NumOfFeatures ();  fn++)
      totals[fn] += fd[fn];
  }

  for  (kkuint32 fn = 0;  fn < NumOfFeatures ();  fn++)
    means[fn] = totals[fn] / (double)this->QueueSize ();

  return  means;
}  /* ExtractMeanFeatureValues */



FeatureVectorListPtr   FeatureVectorList::ExtractRandomSampling (float      percentage,   // 0.0 -> 100.0
                                                                 kkuint32   minClassCount
                                                                )
{


  if  (percentage <= 0.0f)
  {
    percentage = 0.0f;
  }

  if  (percentage > 100.0f)
  {
    percentage = 1.0f;
  }

  FeatureVectorListPtr  randomSampled = ManufactureEmptyList (false);

  MLClassListPtr  classes = ExtractListOfClasses ();
  classes->SortByName ();
  MLClassList::iterator  idx;
  for  (idx = classes->begin ();  idx != classes->end ();  idx++)
  {
    MLClassPtr ic = *idx;
    FeatureVectorListPtr  examplesThisClass = ExtractExamplesForAGivenClass (ic);
    examplesThisClass->RandomizeOrder ();

    kkuint32  numExamplesThisClass = Max (minClassCount, ((kkuint32)(0.5f + (float)(examplesThisClass->QueueSize ()) * percentage / 100.0f)));
    if  (numExamplesThisClass > examplesThisClass->QueueSize ())
      numExamplesThisClass = examplesThisClass->QueueSize ();

    for  (kkuint32 zed = 0;  zed < numExamplesThisClass;  zed++)
      randomSampled->PushOnBack (examplesThisClass->IdxToPtr (zed));
    delete  examplesThisClass;  examplesThisClass = NULL;
  }

  delete  classes;   classes = NULL;

  randomSampled->RandomizeOrder ();
  return  randomSampled; 
}  /* ExtractRandomSampling */



bool  FeatureVectorList::MissingData () const  // Returns true if 1 or more entries have missing data.
{
  FeatureVectorList::const_iterator  idx;

  FeatureVectorPtr  i = NULL;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    i = *idx;
    if  (i->MissingData ())
      return true;
  }

  return  false;
}  /* MissingData */



void  FeatureVectorList::ReSyncSymbolicData (FileDescConstPtr  newFileDesc)

{
  kkint32  fieldNum;

  VectorInt  symbolicFields;
  vector<VectorInt>  lookUpTables;

  for  (fieldNum = 0;  fieldNum < (kkint32)newFileDesc->NumOfFields ();  fieldNum++)
  {
    if  (newFileDesc->Type (fieldNum) == AttributeType::Symbolic)
    {
      symbolicFields.push_back (fieldNum);
      VectorInt  lookUpTable;

      kkint32  x;
      for  (x = 0;  x < fileDesc->Cardinality (fieldNum);  x++)
      {
        const KKStr&  nominalValue = fileDesc->GetNominalValue (fieldNum, x);
        kkint32  newCd = newFileDesc->LookUpNominalCode (fieldNum, nominalValue);
        if  (newCd < 0)
        {
          KKStr  errMsg;
          errMsg << "FeatureVectorList::ReSyncSymbolicData  ***ERROR***    FieldNum[" << fieldNum << "]  FieldName[" << newFileDesc->FieldName (fieldNum) << "]  Nominal Value[" << nominalValue << "]    is missing.";
          cerr << errMsg;
          throw KKException (errMsg);
        }

        lookUpTable.push_back (newCd);
      }

      lookUpTables.push_back (lookUpTable);
    }
  }

  FeatureVectorList::iterator  idx;
  for  (idx = begin ();  idx !=  end ();  idx++)
  {
    FeatureVectorPtr i = *idx;

    kkuint32  x;

    for  (x = 0;  x < symbolicFields.size ();  x++)
    {
      fieldNum = symbolicFields[x];
      kkint32  oldCode = kkint32 (0.5f + i->FeatureData (fieldNum));
      kkint32  newCode = lookUpTables[x][oldCode];
      i->AddFeatureData (fieldNum, float (newCode));
    }
  }

  fileDesc = newFileDesc;
}  /* ReSyncSymbolicData */



void  FeatureVectorList::SynchronizeSymbolicData (FeatureVectorList&  otherData,
                                                  RunLog&             log
                                                 )
{
  if  (!fileDesc->SameExceptForSymbolicData (*(otherData.FileDesc ()), log))
  {
    KKStr  errMsg;
    errMsg = "FeatureVectorList::SynchronizeSymbolicData   ***ERROR***    The two datasets have more than SymbolicData differences.";
    log.Level (-1) << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  FileDescConstPtr  newFileDesc = FileDesc::MergeSymbolicFields (*fileDesc, *(otherData.FileDesc ()), log);

  ReSyncSymbolicData (newFileDesc);
  otherData.ReSyncSymbolicData (newFileDesc);
}  /* SynchronizeSymbolicData */



KKStr  GetClassNameByHierarchyLevel (KKStr    className,   
                                     kkint32  level
                                    )
{
  kkint32  curLevel = 0;
  KKStr  fullLevelName = "";
  KKStr  nextLevelName = className.ExtractToken ("_");
  while  ((curLevel < level)  &&  (!nextLevelName.Empty ()))
  {
    curLevel++;
    if  (!fullLevelName.Empty ())
      fullLevelName << "_";
    fullLevelName << nextLevelName;
    nextLevelName = className.ExtractToken ("_");
  }

  return  fullLevelName;
}  /* GetClassNameByHierarchyLevel */



FeatureVectorListPtr  FeatureVectorList::CreateListForAGivenLevel (kkint32  level)
{
  FeatureVectorListPtr  examplesLabeledForAppropriateLevel = ManufactureEmptyList (true);

  MLClassListPtr  allClasses = ExtractListOfClasses ();
  allClasses->SortByName ();

  MLClassList::iterator  idx;
  idx = allClasses->begin ();

  while  (idx != allClasses->end ())
  {
    MLClassPtr  curClass = *idx;
    KKStr  curClassNameForThisLevel = GetClassNameByHierarchyLevel (curClass->Name (), level);
    if  (curClassNameForThisLevel.Empty ())
    {
      idx++;
      continue;
    }
    else
    {
      MLClassPtr  curClassForThisLevel  = MLClass::CreateNewMLClass (curClassNameForThisLevel);
      MLClassPtr  nextClassForThisLevel = curClassForThisLevel;

      while  ((idx != allClasses->end ())  &&  (nextClassForThisLevel == curClassForThisLevel))
      {
        {
          FeatureVectorListPtr  examplesForCurClass = ExtractExamplesForAGivenClass (curClass);
          FeatureVectorListPtr  reLabeledExamples = examplesForCurClass->DuplicateListAndContents ();
          delete  examplesForCurClass;  examplesForCurClass = NULL;

          FeatureVectorList::iterator  idx2;
          for  (idx2 = reLabeledExamples->begin ();  idx2 != reLabeledExamples->end ();  idx2++)
          {
            FeatureVectorPtr  fv = *idx2;
            fv->MLClass (curClassForThisLevel);
          }
          examplesLabeledForAppropriateLevel->AddQueue (*reLabeledExamples);
          reLabeledExamples->Owner (false);
          delete  reLabeledExamples;  reLabeledExamples = NULL;
        }

        // Get next Class to process.
        idx++;
        if  (idx != allClasses->end ())
        {
          curClass = *idx;
          curClassNameForThisLevel = GetClassNameByHierarchyLevel (curClass->Name (), level);
          nextClassForThisLevel = MLClass::CreateNewMLClass (curClassNameForThisLevel);
        }
      }
    }
  }

  delete  allClasses;  allClasses = NULL;

  return  examplesLabeledForAppropriateLevel;
}  /* CreateListForAGivenLevel */



void  FeatureVectorList::SortByRootName (bool  reversedOrder)
{
  auto f = [](FeatureVectorPtr p1, FeatureVectorPtr p2) -> bool{return p1->ExampleRootName () < p2->ExampleRootName ();};
  SortBy(reversedOrder, f);
  curSortOrder = IFL_SortOrder::IFL_ByRootName;
}  /* SortByRootName */



void  FeatureVectorList::SortByClass (bool  reversedOrder)
{

  auto f = [](FeatureVectorPtr l, FeatureVectorPtr r) -> bool {
    if  (l == NULL)
      return ((r == NULL) ? false : true);

    else if  (right == NULL)
      return false;

    else
     return l->MLClassNameUpper() < r->MLClassNameUpper();
  };

  SortBy(reversedOrder, f);
  return;
}



void  FeatureVectorList::SortByProbability (bool  reversedOrder)
{
  auto f = [](FeatureVectorPtr l, FeatureVectorPtr r) -> bool {return l->Probability () < r->Probability ();};
  SortBy(reversedOrder, f);
  curSortOrder = IFL_SortOrder::IFL_ByProbability;
}  /* SortByProbability */



void  FeatureVectorList::SortByBreakTie (bool  reversedOrder)
{
  auto f = [](FeatureVectorPtr l, FeatureVectorPtr r) -> bool {return l->BreakTie () < r->BreakTie ();};
  SortBy(reversedOrder, f);
  curSortOrder = IFL_SortOrder::IFL_ByBreakTie;
}  /* SortByProbability */



void  FeatureVectorList::SortByImageFileName (bool  reversedOrder)
{
  auto f = [](FeatureVectorPtr l, FeatureVectorPtr r) -> bool {return l->ExampleFileName() < r->ExampleFileName();};
  SortBy(reversedOrder, f);
  curSortOrder = IFL_SortOrder::IFL_ByName;
}  /* SortByImageFileName */



void  FeatureVectorList::SortBy (bool reversedOrder,  
                                 bool (*comp)(FeatureVectorPtr l, FeatureVectorPtr r)
                                )
 
{
  std::function<bool (FeatureVectorPtr, FeatureVectorPtr)> func;
  if  (reversedOrder)
    func = [comp](FeatureVectorPtr l, FeatureVectorPtr r) ->bool {return !comp(l, r);};
  else
    func = comp;

  sort (begin (), end (), [func](FeatureVectorPtr  p1, FeatureVectorPtr  p2)  {return func(p1, p2);});
}  /* SortBy */
