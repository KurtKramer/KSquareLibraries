#include "FirstIncludes.h"

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <sstream>
#include <string.h>
#include <string>
#include <iomanip>
#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace  KKB;


#include "FeatureEncoder.h"
#include "BinaryClassParms.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "SvmWrapper.h"
using namespace  KKMLL;


/**
 * @brief Constructs a Feature Encoder object.
 * @param[in] _svmParam 
 * @param[in] _fileDesc 
 * @param[in] _class1 
 * @param[in] _class2 
 * @param[in] _log A log-file stream. All important events will be output to this stream
 */
FeatureEncoder::FeatureEncoder (const SVMparam&       _svmParam,
                                FileDescPtr           _fileDesc,
                                AttributeTypeVector&  _attributeTypes,
                                VectorInt32&          _cardinalityTable,
                                MLClassPtr            _class1,
                                MLClassPtr            _class2,
                                RunLog&               _log
                               ):

    attributeTypes           (_attributeTypes),
    cardinalityDest          (NULL),
    cardinalityTable         (_cardinalityTable),
    class1                   (_class1),
    class2                   (_class2),
    codedNumOfFeatures       (0),
    destFeatureNums          (NULL),
    destFileDesc             (NULL),
    destWhatToDo             (NULL),
    encodingMethod           (NoEncoding),
    fileDesc                 (_fileDesc),
    log                      (_log),
    numEncodedFeatures       (0),
    numOfFeatures            (0),
    selectedFeatures         (_fileDesc),
    srcFeatureNums           (NULL),
    svmParam                 (_svmParam),
    xSpaceNeededPerExample   (0)
    
{
  log.Level (40) << "FeatureEncoder::FeatureEncoder" << endl;

  if  (class1  &&  class2)
    selectedFeatures = svmParam.GetFeatureNums (fileDesc, class1, class2);
  else
    selectedFeatures = svmParam.GetFeatureNums ();

  numOfFeatures = selectedFeatures.NumOfFeatures ();

  encodingMethod    = svmParam.EncodingMethod ();

  xSpaceNeededPerExample = 0;
  srcFeatureNums   = new kkint32[numOfFeatures];
  cardinalityDest  = new kkint32[numOfFeatures];
  destFeatureNums  = new kkint32[numOfFeatures];
  destWhatToDo     = new FeWhatToDo[numOfFeatures];

  VectorKKStr   destFieldNames;

  kkint32  x;

  for  (x = 0;  x < numOfFeatures;  x++)
  {
    kkint32  srcFeatureNum = selectedFeatures[x];
    srcFeatureNums  [x] = srcFeatureNum;
    destFeatureNums [x] = xSpaceNeededPerExample;
    cardinalityDest [x] = 1;
    destWhatToDo    [x] = FeAsIs;

    Attribute  srcAttribute = (fileDesc->Attributes ())[srcFeatureNum];

    switch (encodingMethod)
    {
      case BinaryEncoding:
        if  ((attributeTypes[srcFeatureNums[x]] == NominalAttribute)  || (attributeTypes[srcFeatureNums[x]] == SymbolicAttribute))
        {
          destWhatToDo    [x] = FeBinary;
          cardinalityDest [x] = cardinalityTable[srcFeatureNums [x]];
          xSpaceNeededPerExample += cardinalityDest[x];
          numEncodedFeatures   += cardinalityDest[x];
          for  (kkint32 zed = 0;  zed < cardinalityDest[x];  zed++)
          {
            KKStr  fieldName = srcAttribute.Name () + "_" + srcAttribute.GetNominalValue (zed);
            destFieldNames.push_back (fieldName);
          }
        }
        else
        {
          xSpaceNeededPerExample++;
          numEncodedFeatures++;
          destWhatToDo [x] = FeAsIs;
          destFieldNames.push_back (srcAttribute.Name ());
        }
        break;


      case  ScaledEncoding:
        xSpaceNeededPerExample++;
        numEncodedFeatures++;
        if  ((attributeTypes[srcFeatureNums[x]] == NominalAttribute)  ||  (attributeTypes[srcFeatureNums[x]] == SymbolicAttribute))
          destWhatToDo [x] = FeScale;
        else
          destWhatToDo [x] = FeAsIs;

        destFieldNames.push_back (srcAttribute.Name ());
        break;


      case  NoEncoding:
      default:
        xSpaceNeededPerExample++;
        numEncodedFeatures++;
        destWhatToDo [x] = FeAsIs;
        destFieldNames.push_back (srcAttribute.Name ());
        break;
    }
  }

  codedNumOfFeatures = xSpaceNeededPerExample;

  destFileDesc = FileDesc::NewContinuousDataOnly (log, destFieldNames);

  xSpaceNeededPerExample++;  // Add one more for the terminating (-1)
}







/**
 * @brief Frees any memory allocated by, and owned by the FeatureEncoder
 */
FeatureEncoder::~FeatureEncoder ()
{
  delete  srcFeatureNums;
  delete  destFeatureNums;
  delete  cardinalityDest;
  delete  destWhatToDo;
}



kkint32  FeatureEncoder::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (FeatureEncoder) 
    + attributeTypes.size () * sizeof (AttributeType)
    + selectedFeatures.MemoryConsumedEstimated ()
    + numOfFeatures * sizeof (kkint32);

  if  (cardinalityDest)
    memoryConsumedEstimated += 3 * sizeof (kkint32) * numOfFeatures;    // cardinalityDest + destFeatureNums + srcFeatureNums

  //  We do not own 'destFileDesc'  and  'fileDesc'
  if  (destWhatToDo)
    memoryConsumedEstimated += sizeof (FeWhatToDo) * numOfFeatures;

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */




FileDescPtr  FeatureEncoder::CreateEncodedFileDesc (ostream*  o)
{
  log.Level (40) << "FeatureEncoder::CreateEncodedFileDesc" << endl;
  FileDescPtr  newFileDesc = new FileDesc ();

  if  (o)
  {
    *o << endl 
       << "Orig"     << "\t" << "Orig"      << "\t" << "Field" << "\t" << "Encoded"  << "\t" << "Encoded"   << endl;
    *o << "FieldNum" << "\t" << "FieldName" << "\t" << "Type"  << "\t" << "FieldNum" << "\t" << "FieldName" << endl;
  }

  kkint32  x;

  bool  alreadyExist;
  
  for  (x = 0;  x < numOfFeatures; x++)
  {
    kkint32  srcFeatureNum = srcFeatureNums[x];
    kkint32  y = destFeatureNums[x];

    if  (y >= numEncodedFeatures)
    {
      cerr << endl
           << endl
           << "FeatureEncoder::CreateEncodedFileDesc     *** ERROR ***"           << endl
           << "             overriding number of encoded features.  This should"  << endl
           << "             never be able to happen. Something is wrong with"     << endl
           << "             object."                                              << endl
           << endl;
      osWaitForEnter ();
      exit (-1);
    }

    KKStr  origFieldDesc = StrFormatInt (srcFeatureNum, "zz0") + "\t" +
                            fileDesc->FieldName (srcFeatureNum) + "\t" +
                            fileDesc->TypeStr (srcFeatureNum);


    switch (destWhatToDo[x])
    {
    case  FeAsIs:
      {
        newFileDesc->AddAAttribute (fileDesc->FieldName (x), NumericAttribute, alreadyExist);
        if  (o)
        {
          *o << origFieldDesc          << "\t" 
             << y                      << "\t"
             << fileDesc->FieldName (x)
             << endl;
        }
      }
      break;

    case  FeBinary:
      {
        for  (kkint32 z = 0;  z < cardinalityDest[x];  z++)
        {
          KKStr  nominalValue = fileDesc->GetNominalValue (srcFeatureNums[x], z);
          KKStr  encodedName  = fileDesc->FieldName (x) + "_" + nominalValue;
          newFileDesc->AddAAttribute (encodedName, NumericAttribute, alreadyExist);
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

    case  FeScale:
      {
        newFileDesc->AddAAttribute (fileDesc->FieldName (x), NumericAttribute, alreadyExist);
        if  (o)
        {
          *o << origFieldDesc           << "\t" 
             << y                       << "\t"
             << fileDesc->FieldName (x)
             << endl;
        }
      }
      break;
    }
  }

  return  newFileDesc;
}  /* CreateEncodedFileDesc */






/**
 * @brief Converts a single example into the svm_problem format, using the method specified 
 * by the EncodingMethod() value returned by svmParam
 * @param[in] example That we're converting
 */
XSpacePtr  FeatureEncoder::EncodeAExample (FeatureVectorPtr  example)
{
  // XSpacePtr  xSpace  = (struct svm_node*)malloc (xSpaceNeededPerExample * sizeof (struct svm_node));
  XSpacePtr  xSpace  = new svm_node[xSpaceNeededPerExample];
  kkint32  xSpaceUsed = 0;
  EncodeAExample (example, xSpace, xSpaceUsed);
  return  xSpace;
}  /* EncodeAExample */




FeatureVectorPtr  FeatureEncoder::EncodeAExample (FileDescPtr       encodedFileDesc,
                                                  FeatureVectorPtr  src
                                                 )
{
  FeatureVectorPtr  encodedExample = new FeatureVector (numEncodedFeatures);
  encodedExample->MLClass     (src->MLClass     ());
  encodedExample->PredictedClass (src->PredictedClass ());
  //encodedExample->Version        (src->Version        ());
  encodedExample->TrainWeight    (src->TrainWeight    ());

  const float*  featureData = src->FeatureData ();
  kkint32  x;

  for  (x = 0;  x < numOfFeatures; x++)
  {
    float  featureVal = featureData [srcFeatureNums[x]];
    kkint32  y = destFeatureNums[x];

    switch (destWhatToDo[x])
    {
    case  FeAsIs:
      {
        encodedExample->AddFeatureData (y, featureVal);
      }
      break;

    case  FeBinary:
      {
        for  (kkint32 z = 0; z < cardinalityDest[x]; z++)
        {
          float  bVal = ((kkint32)featureVal == z);
          encodedExample->AddFeatureData (y, bVal);
          y++;
        }
      }

      break;

    case  FeScale:
      {
        encodedExample->AddFeatureData (y, (featureVal / (float)cardinalityDest[x]));
      }
      break;
    }
  }

  return  encodedExample;
}  /* EncodeAExample */





FeatureVectorListPtr  FeatureEncoder::EncodeAllExamples (const FeatureVectorListPtr  srcData)
{
  FileDescPtr  encodedFileDesc = CreateEncodedFileDesc (NULL);

  FeatureVectorListPtr  encodedExamples = new FeatureVectorList (encodedFileDesc, 
                                                                 true,                  // Will own the contents 
                                                                 log
                                                                );

  FeatureVectorList::const_iterator  idx;

  for  (idx = srcData->begin ();  idx !=  srcData->end ();   idx++)
  {
    const FeatureVectorPtr srcExample = *idx;
    FeatureVectorPtr  encodedExample = EncodeAExample (encodedFileDesc, srcExample);
    encodedExamples->PushOnBack (encodedExample);
  }

  return  encodedExamples;
}  /* EncodeAllExamples */




/**
 * @brief Converts a single example into the svm_problem format, using the method specified 
 * by the EncodingMethod() value returned by svmParam
 * @param[in] The example That we're converting
 * @param[in] The row kkint32 he svm_problem structure that the converted data will be stored
 */
void  FeatureEncoder::EncodeAExample (FeatureVectorPtr  example,
                                      svm_node*         xSpace,
                                      kkint32&          xSpaceUsed
                                     )
{
  const float*  featureData = example->FeatureData ();
  kkint32  x;

  xSpaceUsed = 0;

  for  (x = 0;  x < numOfFeatures; x++)
  {
    float  featureVal = featureData [srcFeatureNums[x]];
    kkint32  y = destFeatureNums[x];

    if  (y >= xSpaceNeededPerExample)
    {
      cerr << endl
           << endl
           << "FeatureEncoder::EncodeAExample     *** ERROR ***"  << endl
           << "             We are overwriting end of xSpace"      << endl
           << endl;
      osWaitForEnter ();
      exit (-1);
    }

    switch (destWhatToDo[x])
    {
    case  FeAsIs:
      {
        if  (featureVal != 0.0)
        {
          xSpace[xSpaceUsed].index = y;
          xSpace[xSpaceUsed].value = featureVal;
          xSpaceUsed++;
        }
      }
      break;

    case  FeBinary:
      {
        for  (kkint32 z = 0; z < cardinalityDest[x]; z++)
        {
          float  bVal = ((kkint32)featureVal == z);
          if  (bVal != 0.0)
          {
            xSpace[xSpaceUsed].index = y;
            xSpace[xSpaceUsed].value = bVal;
            xSpaceUsed++;
          }
          y++;
        }
      }

      break;

    case  FeScale:
      {
        if  (featureVal != (float)0.0)
        {
          xSpace[xSpaceUsed].index = y;
          xSpace[xSpaceUsed].value = featureVal / (float)cardinalityDest[x];
          xSpaceUsed++;
        }
      }
      break;
    }
  }

  xSpace[xSpaceUsed].index = -1;
  xSpace[xSpaceUsed].value = -1;
  xSpaceUsed++;
}  /* EncodeAExample */



kkint32  FeatureEncoder::DetermineNumberOfNeededXspaceNodes (FeatureVectorListPtr   src)  const
{
  kkint32  xSpaceNodesNeeded = 0;
  FeatureVectorList::const_iterator  idx;
  for  (idx = src->begin ();  idx != src->end ();  ++idx)
  {
    FeatureVectorPtr fv = *idx;
    const float*  featureData = fv->FeatureData ();

    for  (kkint32 x = 0;  x < numOfFeatures; x++)
    {
      float  featureVal = featureData [srcFeatureNums[x]];
      kkint32  y = destFeatureNums[x];
  
      switch (destWhatToDo[x])
      {
      case  FeAsIs:
        if  (featureVal != 0.0)
          xSpaceNodesNeeded++;
        break;

      case  FeBinary:
        for  (kkint32 z = 0; z < cardinalityDest[x]; z++)
        {
          float  bVal = ((kkint32)featureVal == z);
          if  (bVal != 0.0)
            xSpaceNodesNeeded++;
          y++;
         }
         break;

      case  FeScale:
         if  (featureVal != (float)0.0)
           xSpaceNodesNeeded++;
         break;
      }
    }
    xSpaceNodesNeeded++;
  }

  return xSpaceNodesNeeded;
}  /* DetermineNumberOfNeededXspaceNodes */



void  FeatureEncoder::EncodeIntoSparseMatrix
                               (FeatureVectorListPtr   src,
                                ClassAssignments&      assignments,
                                XSpacePtr&             xSpace,          
                                kkint32&               totalxSpaceUsed,
                                struct svm_problem&    prob
                               )

{
  FeatureVectorListPtr  compressedExamples    = NULL;
  FeatureVectorListPtr  examplesToUseFoXSpace = NULL;
  kkint32               xSpaceUsed            = 0;

  totalxSpaceUsed = 0;

  examplesToUseFoXSpace = src;

  kkint32  numOfExamples = examplesToUseFoXSpace->QueueSize ();
  //kkint32  elements      = numOfExamples * xSpaceNeededPerExample;

  prob.l     = numOfExamples;
  prob.y     = (double*)malloc  (prob.l * sizeof (double));
  prob.x     = (struct svm_node **) malloc (prob.l * sizeof (struct svm_node*));
  prob.index = new kkint32[prob.l];
  prob.exampleNames.clear ();

  kkint32  numNeededXspaceNodes = DetermineNumberOfNeededXspaceNodes (examplesToUseFoXSpace);

  kkint32  totalBytesForxSpaceNeeded = (numNeededXspaceNodes + 10) * sizeof (struct svm_node);  // I added '10' to elements because I am paranoid

  xSpace = (struct svm_node*) malloc (totalBytesForxSpaceNeeded);
  if  (xSpace == NULL)
  {
    log.Level (-1) << endl << endl << endl
                   << " FeatureEncoder::Compress   *** Failed to allocates space for 'xSpace' ****" << endl
                   << endl
                   << "     Space needed          [" << totalBytesForxSpaceNeeded << "]" << endl
                   << "     Num of Examples       [" << numOfExamples             << "]" << endl
                   << "     Num XSpaceNodesNeeded [" << numNeededXspaceNodes      << "]" << endl
                   << endl;
    // we sill have to allocate space for each individual training example separately.
    //throw "FeatureEncoder::Compress     Allocation of memory for xSpace Failed.";
  }

  prob.W = NULL;

  kkint32 i = 0;
 
  FeatureVectorPtr  example      = NULL;
  MLClassPtr        lastMlClass  = NULL;
  kkint16           lastClassNum = -1;

  kkint32  bytesOfxSpacePerExample = xSpaceNeededPerExample * sizeof (struct svm_node);

  for (i = 0;  i < prob.l;  i++)
  {
    if  (totalxSpaceUsed > numNeededXspaceNodes)
    {
      log.Level (-1) << endl << endl
        << "FeatureEncoder::Compress   ***ERROR***   We have exceeded the number of XSpace nodes allocated." << endl
        << endl;
    }

    example = examplesToUseFoXSpace->IdxToPtr (i);

    if  (example->MLClass () != lastMlClass)
    {
      lastMlClass  = example->MLClass ();
      lastClassNum = assignments.GetNumForClass (lastMlClass);
    }

    prob.y[i]     = lastClassNum;
    prob.index[i] = i;
    prob.exampleNames.push_back (osGetRootName (example->ExampleFileName ()));

    if  (prob.W)
    {
      prob.W[i] = example->TrainWeight () * svmParam.C_Param ();
      if  (example->TrainWeight () <= 0.0f)
      {
        log.Level (-1) << endl 
                       << "FeatureEncoder::EncodeIntoSparseMatrix    ***ERROR***   Example[" << example->ExampleFileName () << "]" << endl
                       << "      has a TrainWeight value of 0 or less defaulting to 1.0" << endl
                       << endl;
        prob.W[i] = 1.0 * svmParam.C_Param ();
      }
    }

    if  (xSpace == NULL)
    {
      struct svm_node*  xSpaceThisExample = (struct svm_node*) malloc (bytesOfxSpacePerExample);
      prob.x[i] = xSpaceThisExample;
      EncodeAExample (example, prob.x[i], xSpaceUsed);
      if  (xSpaceUsed < xSpaceNeededPerExample)
      {
        kkint32  bytesNeededForThisExample = xSpaceUsed * sizeof (struct svm_node);
        struct svm_node*  smallerXSpaceThisExample = (struct svm_node*) malloc (bytesNeededForThisExample);
        memcpy (smallerXSpaceThisExample, xSpaceThisExample, bytesNeededForThisExample);
        free  (xSpaceThisExample);
        prob.x[i] = smallerXSpaceThisExample;
      }
    }
    else
    {
      prob.x[i] = &xSpace[totalxSpaceUsed];
      EncodeAExample (example, prob.x[i], xSpaceUsed);
    }
    totalxSpaceUsed += xSpaceUsed;
  }

  delete  compressedExamples;
  return;
}  /* Compress */





/**
 * @brief  Left over from BitReduction days; removed all code except that which processed the NO bit reduction option.
 * @param[in] examples_list The list of examples you want to attempt to reduce
 * @param[out] compressed_examples_list The reduced list of examples
 */
void  FeatureEncoder::CompressExamples (FeatureVectorListPtr    srcExamples,
                                        FeatureVectorListPtr    compressedExamples,
                                        ClassAssignments&       assignments
                                       )
{
  double time_before, time_after;
  time_before = osGetSystemTimeUsed ();
  compressedExamples->AddQueue (*srcExamples);
  time_after = osGetSystemTimeUsed ();
  compressedExamples->Owner (false);
  return;
}  /* CompressExamples */




FeatureVectorListPtr  FeatureEncoder::CreateEncodedFeatureVector (FeatureVectorList&  srcData)
{
  if  (srcData.AllFieldsAreNumeric ())
    return  srcData.DuplicateListAndContents ();

  FeatureVectorListPtr  encodedFeatureVectorList = new FeatureVectorList (destFileDesc, true, log);

  FeatureVectorList::iterator  idx;
  for  (idx = srcData.begin ();   idx != srcData.end ();  idx++)
  {
    FeatureVectorPtr  srcExample = *idx;
    XSpacePtr  encodedData = EncodeAExample (srcExample);

    kkint32  zed = 0;
    FeatureVectorPtr  encodedFeatureVector = new FeatureVector (codedNumOfFeatures);
    while  (encodedData[zed].index != -1)
    {
      encodedFeatureVector->AddFeatureData (encodedData[zed].index, (float)encodedData[zed].value);
      zed++;
    }

    encodedFeatureVector->MLClass (srcExample->MLClass ());
    encodedFeatureVectorList->PushOnBack (encodedFeatureVector);

    delete  encodedData;
    encodedData = NULL;
  }

  return  encodedFeatureVectorList;
}  /* CreateEncodedFeatureVector */
