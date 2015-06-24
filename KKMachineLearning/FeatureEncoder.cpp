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


FeatureEncoder::FeatureEncoder ():
    cardinalityDest          (NULL),
    class1                   (NULL),
    class2                   (NULL),
    codedNumOfFeatures       (0),
    c_Param                  (1.0),
    destFeatureNums          (NULL),
    destFileDesc             (NULL),
    destWhatToDo             (NULL),
    encodingMethod           (SVM_EncodingMethod::NoEncoding),
    fileDesc                 (NULL),
    numEncodedFeatures       (0),
    numOfFeatures            (0),
    selectedFeatures         (),
    srcFeatureNums           (NULL),
    xSpaceNeededPerExample   (0)
{
}



/**
 * @brief Constructs a Feature Encoder object.
 * @param[in] _fileDesc 
 * @param[in] _class1 
 * @param[in] _class2 
 * @param[in] _log A log-file stream. All important events will be output to this stream
 */
FeatureEncoder::FeatureEncoder (FileDescPtr            _fileDesc,
                                MLClassPtr             _class1,
                                MLClassPtr             _class2,
                                const FeatureNumList&  _selectedFeatures,
                                SVM_EncodingMethod     _encodingMethod,
                                double                 _c_Param
                               ):

    cardinalityDest          (NULL),
    class1                   (_class1),
    class2                   (_class2),
    codedNumOfFeatures       (0),
    c_Param                  (_c_Param),
    destFeatureNums          (NULL),
    destFileDesc             (NULL),
    destWhatToDo             (NULL),
    encodingMethod           (_encodingMethod),
    fileDesc                 (_fileDesc),
    numEncodedFeatures       (0),
    numOfFeatures            (0),
    selectedFeatures         (_selectedFeatures),
    srcFeatureNums           (NULL),
    xSpaceNeededPerExample   (0)
{
  numOfFeatures   = selectedFeatures.NumOfFeatures ();

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

    const Attribute&  attribute = fileDesc->GetAAttribute (srcFeatureNum);
    AttributeType  attributeType = attribute.Type ();
    kkint32        cardinality   = attribute.Cardinality ();

    switch (encodingMethod)
    {
      case SVM_EncodingMethod::BinaryEncoding:
        if  ((attributeType == AttributeType::Nominal)  ||  (attributeType == AttributeType::Symbolic))
        {
          destWhatToDo    [x] = FeBinary;
          cardinalityDest [x] = cardinality;
          xSpaceNeededPerExample += cardinalityDest[x];
          numEncodedFeatures   += cardinalityDest[x];
          for  (kkint32 zed = 0;  zed < cardinalityDest[x];  zed++)
          {
            KKStr  fieldName = attribute.Name () + "_" + attribute.GetNominalValue (zed);
            destFieldNames.push_back (fieldName);
          }
        }
        else
        {
          xSpaceNeededPerExample++;
          numEncodedFeatures++;
          destWhatToDo [x] = FeAsIs;
          destFieldNames.push_back (attribute.Name ());
        }
        break;


      case  SVM_EncodingMethod::ScaledEncoding:
        xSpaceNeededPerExample++;
        numEncodedFeatures++;
        if  ((attributeType == AttributeType::Nominal)  ||
             (attributeType == AttributeType::Symbolic)
            )
          destWhatToDo [x] = FeScale;
        else
          destWhatToDo [x] = FeAsIs;

        destFieldNames.push_back (attribute.Name ());
        break;


      case  SVM_EncodingMethod::NoEncoding:
      default:
        xSpaceNeededPerExample++;
        numEncodedFeatures++;
        destWhatToDo [x] = FeAsIs;
        destFieldNames.push_back (attribute.Name ());
        break;
    }
  }

  codedNumOfFeatures = xSpaceNeededPerExample;

  destFileDesc = FileDesc::NewContinuousDataOnly (destFieldNames);

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
      KKStr  errMsg (128);
      errMsg << "FeatureEncoder::CreateEncodedFileDesc  numEncodedFeatures [" << numEncodedFeatures << "]  exceeded.";
      cerr << endl
           << "FeatureEncoder::CreateEncodedFileDesc     *** ERROR ***"           << endl
           << "             " << errMsg << endl
           << endl;
      throw KKException (errMsg);
      exit (-1);
    }

    KKStr  origFieldDesc = StrFormatInt (srcFeatureNum, "zz0") + "\t" +
                            fileDesc->FieldName (srcFeatureNum) + "\t" +
                            fileDesc->TypeStr (srcFeatureNum);


    switch (destWhatToDo[x])
    {
    case  FeAsIs:
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

    case  FeBinary:
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

    case  FeScale:
      {
        newFileDesc->AddAAttribute (fileDesc->FieldName (x), AttributeType::Numeric, alreadyExist);
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
 * @brief Converts a single example into the svm_problem format
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

  FeatureVectorListPtr  encodedExamples = new FeatureVectorList (encodedFileDesc, true);

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
 * @brief Converts a single example into the svm_problem format.
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
      KKStr  errMsg (128);
      errMsg << "FeatureEncoder::EncodeAExample  ***ERROR***   xSpaceNeededPerExample[" << xSpaceNeededPerExample << "].";
      cerr << endl
           << "FeatureEncoder::EncodeAExample     *** ERROR ***"  << endl
           << "             " << errMsg                           << endl
           << endl;
      throw KKException (errMsg);
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
                                struct svm_problem&    prob,
                                RunLog&                log
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
      prob.W[i] = example->TrainWeight () * c_Param;
      if  (example->TrainWeight () <= 0.0f)
      {
        log.Level (-1) << endl 
                       << "FeatureEncoder::EncodeIntoSparseMatrix    ***ERROR***   Example[" << example->ExampleFileName () << "]" << endl
                       << "      has a TrainWeight value of 0 or less defaulting to 1.0" << endl
                       << endl;
        prob.W[i] = 1.0 * c_Param;
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

  FeatureVectorListPtr  encodedFeatureVectorList = new FeatureVectorList (destFileDesc, true);

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




void  FeatureEncoder::WriteXML (const KKStr&  varName,
                                ostream&      o
                               )  const
{
  XmlTag  tagStart ("TrainingClassList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    tagStart.AddAtribute ("VarName", varName);

  tagStart.WriteXML (o);
  o << endl;

  XmlElementInt32::WriteXML  (codedNumOfFeatures,     "CodedNumOfFeatures",     o);
  XmlElementDouble::WriteXML (c_Param,                "c_Param",                o);
  XmlElementInt32::WriteXML  (numEncodedFeatures,     "NumEncodedFeatures",     o);
  XmlElementInt32::WriteXML  (numOfFeatures,          "NumOfFeatures",          o);
  XmlElementInt32::WriteXML  (xSpaceNeededPerExample, "xSpaceNeededPerExample", o);

  if  (cardinalityDest)
    XmlElementArrayInt32::WriteXML (numOfFeatures, cardinalityDest, "CardinalityDest", o);

  if  (class1)  class1->Name ().WriteXML ("Class1", o);
  if  (class2)  class2->Name ().WriteXML ("Class2", o);
  if  (destFeatureNums)
    XmlElementArrayInt32::WriteXML (numOfFeatures, destFeatureNums, "DestFeatureNums", o);

  if  (fileDesc)      fileDesc->WriteXML     ("FileDesc",     o);
  if  (destFileDesc)  destFileDesc->WriteXML ("DestFileDesc", o);

  if  (destWhatToDo)
  {
    VectorInt32  v;
    for  (kkint32 x = 0;  x < numOfFeatures;  ++x)
      v.push_back ((kkint32)(destWhatToDo[x]));
    XmlElementVectorInt32::WriteXML (v, "DestWhatToDo", o);
  }

  EncodingMethodToStr (encodingMethod).WriteXML ("EncodingMethod", o);

  selectedFeatures.WriteXML ("selectedFeatures", o);

  if  (srcFeatureNums)
    XmlElementArrayInt32::WriteXML (numOfFeatures, srcFeatureNums, "SrcFeatureNums", o);
 
  XmlTag  tagEnd ("TrainingClassList", XmlTag::TagTypes::tagEnd);
  tagEnd.WriteXML (o);
  o << endl;
}



void  FeatureEncoder::ReadXML (XmlStream&      s,
                               XmlTagConstPtr  tag,
                               RunLog&         log
                              )
{
  XmlTokenPtr t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr e = dynamic_cast<XmlElementPtr> (t);
      if  (e)
      {
        KKStr varName = e->VarName ();

        if  (varName.EqualIgnoreCase ("CodedNumOfFeatures"))
          codedNumOfFeatures= e->ToInt32 ();

        else if  (varName.EqualIgnoreCase ("C_Param"))
          c_Param = e->ToDouble ();

        else if  (varName.EqualIgnoreCase ("NumEncodedFeatures"))
          numEncodedFeatures = e->ToInt32 ();

        else if  (varName.EqualIgnoreCase ("NumOfFeatures"))
          numOfFeatures = e->ToInt32 ();

        else if  (varName.EqualIgnoreCase ("xSpaceNeededPerExample"))
          xSpaceNeededPerExample = e->ToInt32 ();

        else if  (typeid (*e) == typeid (XmlElementArrayInt32))
        {
          XmlElementArrayInt32Ptr xmlArray = dynamic_cast<XmlElementArrayInt32Ptr> (e);
          kkuint32 count = xmlArray->Count ();
          if  (count != numOfFeatures)
          {
            log.Level (-1) << endl
              << "FeatureEncoder::ReadXML   ***ERROR***  Variable[" << varName << "]  Invalid Length[" << count << "]  Expected[" << numOfFeatures << "]" << endl
              << endl;
          }
          else
          {
            if  (varName.EqualIgnoreCase ("CardinalityDest"))
            {
              delete  cardinalityDest;
              cardinalityDest = xmlArray->TakeOwnership ();
            }

            else if  (varName.EqualIgnoreCase ("DestFeatureNums"))
            {
              delete  destFeatureNums;
              destFeatureNums = xmlArray->TakeOwnership ();
            }

            else if  (varName.EqualIgnoreCase ("SrcFeatureNums"))
            {
              delete  srcFeatureNums;
              srcFeatureNums = xmlArray->TakeOwnership ();
            }
          }
        }

        else if  (varName.EqualIgnoreCase ("Class1"))
          class1 = MLClass::CreateNewMLClass (e->ToKKStr (), -1);

        else if  (varName.EqualIgnoreCase ("Class2"))
          class2 = MLClass::CreateNewMLClass (e->ToKKStr (), -1);

        else if  (varName.EqualIgnoreCase ("FileDesc")  &&  (typeid (*e) == typeid (XmlElementFileDesc)))
          fileDesc = dynamic_cast<XmlElementFileDescPtr> (e)->Value ();

        else if  (varName.EqualIgnoreCase ("DestFileDesc")  &&  (typeid (*e) == typeid (XmlElementFileDesc)))
          destFileDesc = dynamic_cast<XmlElementFileDescPtr> (e)->Value ();

        else if  (varName.EqualIgnoreCase ("DestWhatToDo")  &&  (typeid (*e) == typeid (XmlElementVectorInt32)))
        {
          XmlElementVectorInt32Ptr  xmlVect = dynamic_cast<XmlElementVectorInt32Ptr> (e);
          if  (xmlVect  &&  xmlVect->Value ())
          {
            const VectorInt32&  v = *(xmlVect->Value ());
            if  (v.size () != numOfFeatures)
            {
              log.Level (-1) << endl
                << "FeatureEncoder::ReadXML   ***ERROR***   Variable[" << varName << "]  Invalid Size[" << v.size () << "]  Expected[" << numOfFeatures << "]." << endl
                << endl;
            }
            else
            {
              delete  destWhatToDo;
              destWhatToDo = new FeWhatToDo[v.size ()];
              for  (kkuint32 x = 0;  x < v.size ();  ++x)
                destWhatToDo[x] = (FeWhatToDo)v[x];
            }
          }
        }

        else if  (varName.EqualIgnoreCase ("EncodingMethod"))
          encodingMethod = EncodingMethodFromStr (e->ToKKStr ());

        else
        {
          log.Level (-1) << "XmlElementTrainingClassList   ***ERROR***   Un-expected Section Element[" << e->SectionName () << "]" << endl;
        }
      }
    }

    delete  t;
    t = s.GetNextToken (log);
  }
}  /* ReadXML */

