#include "FirstIncludes.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;


#include "FeatureEncoder2.h"
#include "BinaryClassParms.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "SvmWrapper.h"
using namespace  KKMLL;

/**
  *@brief Constructs a Feature Encoder object.
  *@param[in] _param 
  *@param[in] _fileDesc
  *@param[in] _log A log file stream. All important events will be output to this stream
  */
FeatureEncoder2::FeatureEncoder2 (const ModelParam&  _param,
                                  FileDescConstPtr   _fileDesc
                                 ):
    attributeVector     (_fileDesc->AttributeVector ()),
    cardinalityDest     (NULL),
    cardinalityVector   (_fileDesc->CardinalityVector ()),
    codedNumOfFeatures  (0),
    destFeatureNums     (NULL),
    destWhatToDo        (NULL),
    encodedFileDesc     (NULL),
    encodingMethod      (ModelParam::EncodingMethodType::NoEncoding),
    fileDesc            (_fileDesc),
    numOfFeatures       (0),
    srcFeatureNums      (NULL),
    param               (_param)
    
{
  FeatureNumListConstPtr  selectedFeatures = param.SelectedFeatures ();
  numOfFeatures = param.SelectedFeatures ()->NumOfFeatures ();

  encodingMethod   = param.EncodingMethod ();

  srcFeatureNums   = new kkuint16  [numOfFeatures];
  cardinalityDest  = new kkint32   [numOfFeatures];
  destFeatureNums  = new kkint32   [numOfFeatures];
  destWhatToDo     = new FeWhatToDo[numOfFeatures];

  VectorKKStr   destFieldNames;

  kkint32  x;

  for  (x = 0;  x < numOfFeatures;  x++)
  {
    kkuint16  srcFeatureNum = (*selectedFeatures)[x];
    srcFeatureNums   [x] = srcFeatureNum;
    destFeatureNums  [x] = codedNumOfFeatures;
    cardinalityDest  [x] = 1;
    destWhatToDo     [x] = FeWhatToDo::FeAsIs;

    Attribute  srcAttribute = (fileDesc->Attributes ())[srcFeatureNum];

    switch (encodingMethod)
    {
      case  ModelParam::EncodingMethodType::Binary:
        if  ((attributeVector[srcFeatureNum] == AttributeType::Nominal)  ||
             (attributeVector[srcFeatureNum] == AttributeType::Symbolic)
            )
        {
          destWhatToDo    [x] = FeWhatToDo::FeBinary;
          cardinalityDest [x] = cardinalityVector[srcFeatureNums [x]];
          codedNumOfFeatures   += cardinalityDest[x];
          for  (kkint32 zed = 0;  zed < cardinalityDest[x];  zed++)
          {
            KKStr  fieldName = srcAttribute.Name () + "_" + srcAttribute.GetNominalValue (zed);
            destFieldNames.push_back (fieldName);
          }
        }
        else
        {
          codedNumOfFeatures++;
          destWhatToDo [x] = FeWhatToDo::FeAsIs;
          destFieldNames.push_back (srcAttribute.Name ());
        }
        break;


      case  ModelParam::EncodingMethodType::Scaled:
        codedNumOfFeatures++;
        if  ((attributeVector[srcFeatureNums[x]] == AttributeType::Nominal)  ||
             (attributeVector[srcFeatureNums[x]] == AttributeType::Symbolic)
            )
          destWhatToDo [x] = FeWhatToDo::FeScale;
        else
          destWhatToDo [x] = FeWhatToDo::FeAsIs;

        destFieldNames.push_back (srcAttribute.Name ());
        break;


      case    ModelParam::EncodingMethodType::NoEncoding:
      default:
        codedNumOfFeatures++;
        destWhatToDo [x] = FeWhatToDo::FeAsIs;
        destFieldNames.push_back (srcAttribute.Name ());
        break;
    }
  }

  encodedFileDesc = FileDesc::NewContinuousDataOnly (destFieldNames);
}




FeatureEncoder2::FeatureEncoder2 (const FeatureEncoder2&  _encoder):
    attributeVector    (_encoder.attributeVector),
    cardinalityDest    (NULL),
    cardinalityVector  (_encoder.cardinalityVector),
    codedNumOfFeatures (_encoder.codedNumOfFeatures),
    destFeatureNums    (NULL),
    destWhatToDo       (NULL),
    encodedFileDesc    (_encoder.encodedFileDesc),
    encodingMethod     (_encoder.encodingMethod),
    fileDesc           (_encoder.fileDesc),
    numOfFeatures      (_encoder.numOfFeatures),
    srcFeatureNums     (NULL),
    param              (_encoder.param)
{
  cardinalityDest  = new kkint32[numOfFeatures];
  destFeatureNums  = new kkint32[numOfFeatures];
  destWhatToDo     = new FeWhatToDo[numOfFeatures];
  srcFeatureNums   = new kkuint16[numOfFeatures];

  kkint32  x;
  for  (x = 0;  x < numOfFeatures;  x++)
  {
    srcFeatureNums   [x] = _encoder.srcFeatureNums [x];
    destFeatureNums  [x] = _encoder.destFeatureNums[x];
    cardinalityDest  [x] = _encoder.cardinalityDest[x];
    destWhatToDo     [x] = _encoder.destWhatToDo   [x];
    srcFeatureNums   [x] = _encoder.srcFeatureNums [x];
  }
}



/**
  * @brief Frees any memory allocated by, and owned by the FeatureEncoder2
  */
FeatureEncoder2::~FeatureEncoder2 ()
{
  delete  srcFeatureNums;
  delete  destFeatureNums;
  delete  cardinalityDest;
  delete  destWhatToDo;
}


kkint32  FeatureEncoder2::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (FeatureEncoder2)
    +  attributeVector.size ()   * sizeof (AttributeType)
    +  cardinalityVector.size () * sizeof (kkint32);

  if  (cardinalityDest)   memoryConsumedEstimated += 2 * numOfFeatures * sizeof (kkint32);  // For 'cardinalityDest', 'destFeatureNums'
  if  (destFeatureNums)   memoryConsumedEstimated += numOfFeatures * sizeof (kkint32);
  if  (destWhatToDo)      memoryConsumedEstimated += numOfFeatures * sizeof (FeWhatToDo);
  if  (srcFeatureNums)    memoryConsumedEstimated += numOfFeatures * sizeof (kkuint16);
  
  return  memoryConsumedEstimated;
}



kkint32  FeatureEncoder2::NumEncodedFeatures ()  const
{
  return  encodedFileDesc->NumOfFields ();
}




FileDescPtr  FeatureEncoder2::CreateEncodedFileDesc (ostream*  o,
                                                     RunLog&   log
                                                    )  const
{
  log.Level (40) << "FeatureEncoder2::CreateEncodedFileDesc" << endl;
  FileDescPtr  newFileDesc = new FileDesc ();

  if  (o)
  {
    *o << endl 
        << "Orig"    << "\t" << "Orig"      << "\t" << "Field" << "\t" << "Encoded"  << "\t" << "Encoded"   << endl;
    *o << "FieldNum" << "\t" << "FieldName" << "\t" << "Type"  << "\t" << "FieldNum" << "\t" << "FieldName" << endl;
  }

  kkint32  x;

  bool  alreadyExist;
  
  for  (x = 0;  x < numOfFeatures; x++)
  {
    kkuint16  srcFeatureNum = srcFeatureNums[x];
    kkint32  y = destFeatureNums[x];

    if  (y >= codedNumOfFeatures)
    {
      log.Level(-1) 
            << endl
            << "FeatureEncoder2::CreateEncodedFileDesc     ***ERROR***"                                   << endl
            << "             overriding number of encoded features. This should never be able to happen." << endl
            << "             Something is wrong with object."                                             << endl
            << endl;
      osWaitForEnter ();
      exit (-1);
    }

    KKStr  origFieldDesc = StrFormatInt (srcFeatureNum, "zz0") + "\t" +
                            fileDesc->FieldName (srcFeatureNum) + "\t" +
                            fileDesc->TypeStr   (srcFeatureNum);

    switch (destWhatToDo[x])
    {
    case  FeWhatToDo::FeAsIs:
      {
        newFileDesc->AddAAttribute (fileDesc->FieldName (x), AttributeType::Numeric, alreadyExist);
        if  (o)
        {
          *o << origFieldDesc          << "\t" 
             << y                      << "\t"
             << fileDesc->FieldName (x)
             << endl;
        }
      }
      break;

    case  FeWhatToDo::FeBinary:
      {
        for  (kkint32 z = 0;  z < cardinalityDest[x];  z++)
        {
          KKStr  nominalValue = fileDesc->GetNominalValue (srcFeatureNums[x], z);
          KKStr  encodedName  = fileDesc->FieldName (x) + "_" + nominalValue;
          newFileDesc->AddAAttribute (encodedName, AttributeType::Numeric, alreadyExist);
          if  (o)
          {
            *o << origFieldDesc << "\t" 
                << y             << "\t"
                << encodedName
                << endl;
          }

          y++;
        }
      }

      break;

    case  FeWhatToDo::FeScale:
      {
        newFileDesc->AddAAttribute (fileDesc->FieldName (x), AttributeType::Numeric, alreadyExist);
        if  (o)
        {
          *o << origFieldDesc << "\t" 
              << y             << "\t"
              << fileDesc->FieldName (x)
              << endl;
        }
      }
      break;
    }
  }

  newFileDesc = FileDesc::GetExistingFileDesc (newFileDesc);

  return  newFileDesc;
}  /* CreateEncodedFileDesc */





FeatureVectorPtr  FeatureEncoder2::EncodeAExample (FeatureVectorPtr  src)  const
{
  FeatureVectorPtr  encodedImage = new FeatureVector (codedNumOfFeatures);
  encodedImage->MLClass     (src->MLClass     ());
  encodedImage->PredictedClass (src->PredictedClass ());
  encodedImage->TrainWeight    (src->TrainWeight    ());

  const float*  featureData = src->FeatureData ();
  kkint32  x;

  for  (x = 0;  x < numOfFeatures; x++)
  {
    float  featureVal = featureData [srcFeatureNums[x]];
    kkint32  y = destFeatureNums[x];

    switch (destWhatToDo[x])
    {
    case  FeWhatToDo::FeAsIs:
      {
        encodedImage->AddFeatureData (y, featureVal);
      }
      break;

    case  FeWhatToDo::FeBinary:
      {
        for  (kkint32 z = 0; z < cardinalityDest[x]; z++)
        {
          float  bVal = ((kkint32)featureVal == z);
          encodedImage->AddFeatureData (y, bVal);
          y++;
        }
      }

      break;

    case  FeWhatToDo::FeScale:
      {
        encodedImage->AddFeatureData (y, (featureVal / (float)cardinalityDest[x]));
      }
      break;
    }
  }

  return  encodedImage;
}  /* EncodeAExample */





FeatureVectorListPtr  FeatureEncoder2::EncodeAllExamples (const FeatureVectorListPtr  srcData)
{
  FeatureVectorListPtr  encodedExamples = new FeatureVectorList (encodedFileDesc, 
                                                                  true                  // Will own the contents 
                                                                );

  FeatureVectorList::const_iterator  idx;

  for  (idx = srcData->begin ();  idx !=  srcData->end ();   idx++)
  {
    const FeatureVectorPtr srcExample = *idx;
    FeatureVectorPtr  encodedExample = EncodeAExample (srcExample);
    encodedExamples->PushOnBack (encodedExample);
  }

  return  encodedExamples;
}  /* EncodeAllImages */




FeatureVectorListPtr  FeatureEncoder2::EncodedFeatureVectorList (const FeatureVectorList&  srcData)  const
{
  if  (srcData.AllFieldsAreNumeric ())
    return  srcData.DuplicateListAndContents ();

  FeatureVectorListPtr  encodedFeatureVectorList = new FeatureVectorList (encodedFileDesc, true);

  FeatureVectorList::const_iterator  idx;
  for  (idx = srcData.begin ();   idx != srcData.end ();  idx++)
  {
    FeatureVectorPtr  srcExample = *idx;
    FeatureVectorPtr  encodedFeatureVector = EncodeAExample (srcExample);
    encodedFeatureVector->MLClass (srcExample->MLClass ());
    encodedFeatureVectorList->PushOnBack (encodedFeatureVector);
  }

  return  encodedFeatureVectorList;
}  /* EncodedFeatureVectorList */



struct  FeatureEncoder2::FeatureVar2
{
  FeatureVar2 (kkint32        _featureNum,  
                AttributeType  _attributeType,
                kkint32        _idx,  
                double         _var
              ):
          attributeType (_attributeType),
          featureNum    (_featureNum), 
          idx           (_idx),
          var           (_var)
        {}

    KKMLL::AttributeType  attributeType;
    kkint32  featureNum;
    kkint32  idx;
    double   var;
};


//typedef  FeatureEncoder2::FeatureVar2* FeatureVar2Ptr;

class  FeatureEncoder2::FeatureVar2List: public KKQueue<FeatureEncoder2::FeatureVar2>
{
public:
  FeatureVar2List (bool _owner):
      KKQueue<FeatureVar2> (_owner)
      {}

  ~FeatureVar2List () 
  {}
};
