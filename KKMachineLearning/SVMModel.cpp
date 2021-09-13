#include  "FirstIncludes.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <sstream>
#include <iomanip>
#include <set>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStr.h"
#include "OSservices.h"
#include "RunLog.h"
#include "XmlStream.h"
using namespace  KKB;

#include "SVMModel.h"
#include "KKMLLTypes.h"
#include "BinaryClassParms.h"
#include "FeatureEncoder.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "SvmWrapper.h"
using namespace KKMLL;


template<class T>
void  GetMaxIndex (T*       vote, 
                   kkint32  voteLength,
                   kkint32& maxIndex1,
                   kkint32& maxIndex2
                  )
{
  T max1     = vote[0];
  maxIndex1  = 0;

  T  max2    = 0;
  maxIndex2  = -1;

  for  (kkint32 i = 1;  i < voteLength;  i++)
  {
    if  (vote[i] > max1)
    {
      max2      = max1;
      maxIndex2 = maxIndex1;
      max1      = vote[i];
      maxIndex1 = i;
    }

    else if  (maxIndex2 < 0)
    {
      max2      = vote[i];
      maxIndex2 = i;
    }

    else if  (vote[i] > max2)
    {
      max2 = vote[i];
      maxIndex2 = i;
    }
  }

  return;
}  /* GetMaxIndex */





bool  SVMModel::GreaterThan (kkint32 leftVotes,
                             double  leftProb,
                             kkint32 rightVotes,
                             double  rightProb
                            )
{
  if  (leftVotes < rightVotes)
    return false;

  else if  (leftVotes > rightVotes)
    return true;

  else if  (leftProb < rightProb)
    return false;
  
  else if  (leftProb > rightProb)
    return  true;
  
  else
    return  false;
}  /* GreaterThan */



double  AdjProb (double  prob)
{
  if  (prob < 0.001)
    prob = 0.001;
  else if  (prob > 0.999)
    prob = 0.999;
  return  prob;
}


void  SVMModel::GreaterVotes (bool     useProbability,
                              kkint32  numClasses,
                              kkint32* votes,
                              kkint32& numOfWinners,
                              double*  probabilities,
                              kkint32& pred1Idx,
                              kkint32& pred2Idx
                             )
{
  if  (useProbability)
  {
    GetMaxIndex (probabilities, numClasses, pred1Idx , pred2Idx);
    numOfWinners = 1;
    return;
  }

  kkint32 max1Votes = votes[0];
  double  max1Prob  = probabilities[0];
  pred1Idx = 0;
  numOfWinners = 1;

  kkint32 max2Votes = -1;
  double  max2Prob  = -1.0f;
  pred2Idx = -1;

  for  (kkint32 x = 1;  x < numClasses;  x++)
  {
    if  (votes[x] > max1Votes)
      numOfWinners = 1;

    else if  (votes[x] == max1Votes)
      numOfWinners++;

    if  (GreaterThan (votes[x], probabilities[x], max1Votes, max1Prob))
    {
      max2Votes = max1Votes;
      max2Prob  = max1Prob;
      pred2Idx  = pred1Idx;
      max1Votes = votes[x];
      max1Prob  = probabilities[x];
      pred1Idx = x;
    }
    else if  ((pred2Idx < 0)  ||  GreaterThan (votes[x], probabilities[x], max2Votes, max2Prob))
    {
      max2Votes = votes[x];
      max2Prob  = probabilities[x];
      pred2Idx = x;
    }
  }
} /* GreaterVotes*/


     
SVMModel::SVMModel ():
  assignments              (),
  binaryFeatureEncoders    (NULL),
  binaryParameters         (NULL),
  cancelFlag               (false),
  cardinality_table        (),
  classIdxTable            (NULL),
  crossClassProbTable      (NULL),
  crossClassProbTableSize  (0),
  featureEncoder           (NULL),
  fileDesc                 (NULL),
  models                   (NULL),
  numOfClasses             (0),
  numOfModels              (0),
  oneVsAllAssignment       (),
  oneVsAllClassAssignments (NULL),
  predictXSpace            (NULL),
  predictXSpaceWorstCase   (0),
  probabilities            (NULL),
  rootFileName             (),
  selectedFeatures         (NULL),
  svmParam                 (NULL),
  trainingTime             (0.0),
  type_table               (),
  validModel               (true),
  votes                    (NULL),
  xSpaces                  (NULL),
  xSpacesTotalAllocated    (0)
{
  svmParam = new SVMparam ();
}



SVMModel::SVMModel (const SVMparam&     _svmParam,      // Create new model from
                    FeatureVectorList&  _examples,      // Training data.
                    ClassAssignments&   _assignmnets,
                    FileDescConstPtr    _fileDesc,
                    RunLog&             _log
                   )
:
  assignments              (_assignmnets),
  binaryFeatureEncoders    (NULL),
  binaryParameters         (NULL),
  cancelFlag               (false),
  cardinality_table        (),
  classIdxTable            (NULL),
  crossClassProbTable      (NULL),
  crossClassProbTableSize  (0),
  featureEncoder           (NULL),
  fileDesc                 (_fileDesc),
  models                   (NULL),
  numOfClasses             (0),
  numOfModels              (0),
  oneVsAllAssignment       (),
  oneVsAllClassAssignments (NULL),
  predictXSpace            (NULL),
  predictXSpaceWorstCase   (0),
  probabilities            (NULL),
  rootFileName             (),
  selectedFeatures         (NULL),
  svmParam                 (new SVMparam (_svmParam)),
  trainingTime             (0.0),
  type_table               (),
  validModel               (true),
  votes                    (NULL),
  xSpaces                  (NULL),
  xSpacesTotalAllocated    (0)

{
  if  (_examples.QueueSize () < 2)
  {
    _log.Level (-1) << endl
                    << "SVMModel  **** ERROR ****      NO EXAMPLES TO TRAIN WITH." << endl
                    << endl;
    validModel = false;
    return;
  }

  SetSelectedFeatures (_svmParam.SelectedFeatures (), _log);

  type_table        = fileDesc->CreateAttributeTypeTable ( );
  cardinality_table = fileDesc->CreateCardinalityTable ( );
  
  struct svm_problem  prob;

  numOfClasses = (kkuint32)assignments.size ();

  _log.Level (20) << "SVMModel::SVMModel - Constructing From Training Data." << endl;


  try
  {
    if  (svmParam->MachineType () == SVM_MachineType::OneVsOne)
    {
      ConstructOneVsOneModel (&_examples, prob, _log);
    }
    else if  (svmParam->MachineType () == SVM_MachineType::OneVsAll)
    {
      ConstructOneVsAllModel (&_examples, prob, _log);
    }
    else if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
    {
      ConstructBinaryCombosModel (&_examples, _log);
    }
  }
  catch  (const std::exception& e)
  {
    _log.Level (-1) << endl
      << "SVMModel  ***ERROR***   Exception occurred constructing model." << endl
      << e.what () << endl
      << endl;
    validModel = false;
  }
  catch  (...)
  {
    _log.Level (-1) << endl
      << "SVMModel  ***ERROR***   Exception occurred constructing model." << endl
      << endl;
    validModel = false;
  }

  if  (cancelFlag)
    validModel = false;

  if  (!validModel)
    return;

  BuildClassIdxTable ();
  BuildCrossClassProbTable ();
}



SVMModel::~SVMModel ()
{
  DeleteModels ();
  DeleteXSpaces ();

  if  (oneVsAllClassAssignments)
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      if  (oneVsAllClassAssignments[x])
      {
        delete  oneVsAllClassAssignments[x];
        oneVsAllClassAssignments[x] = NULL;
      }
    }
    delete  oneVsAllClassAssignments;
    oneVsAllClassAssignments = NULL;
  }

  delete  featureEncoder;  featureEncoder = NULL;

  if  (binaryParameters)
  {
    // We don't own the binaryParametres them self, only the 
    // array pointing to them,  they are owned by svmParam.
    // so we don't want to delete the contents.
    delete[]  binaryParameters;
    binaryParameters = NULL;
  }

  if  (binaryFeatureEncoders)
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      delete  binaryFeatureEncoders[x];
      binaryFeatureEncoders[x] = NULL;
    }
    delete[]  binaryFeatureEncoders;
    binaryFeatureEncoders = NULL;
  }

  if  (crossClassProbTable)
  {
    for  (kkuint32 x = 0;  x < crossClassProbTableSize;  x++)
    {
      delete  [] crossClassProbTable[x];
      crossClassProbTable[x] = NULL;
    }
    delete[]  crossClassProbTable;
    crossClassProbTable = NULL;
  }

  delete  [] classIdxTable;   classIdxTable   = NULL;
  delete  [] predictXSpace;   predictXSpace   = NULL;
  delete  [] probabilities;   probabilities   = NULL;
  delete  [] votes;           votes           = NULL;

  delete  selectedFeatures;  selectedFeatures = NULL;
  delete  svmParam;          svmParam         = NULL;
}



void  SVMModel::DeleteModels ()
{
  if  (models)
  {
    for  (kkuint32  x = 0;  x < numOfModels;  x++)
    {
      if  (models[x] != NULL)
      {
        SvmDestroyModel (models[x]);
   	    delete[] models[x];
        models[x] = NULL;
      }
    }

    delete[]  models;
    models = NULL;
  }
}


void  SVMModel::DeleteXSpaces ()
{
  if  (xSpaces)
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      if  (xSpaces[x] != NULL)
      {
        delete (xSpaces[x]);
        xSpaces[x] = NULL;
      }
    }

    delete[]  xSpaces;
    xSpaces = NULL;
    xSpacesTotalAllocated = 0;
  }
}






void  SVMModel::AllocateModels ()
{
  models = new ModelPtr  [numOfModels];
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      models[x]  = new SvmModel233*[1];
      models[x][0] = NULL;
    }
  }
}



void  SVMModel::AllocateXSpaces ()
{
  xSpaces = new XSpacePtr [numOfModels];
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
      xSpaces[x] = NULL;
  }
}



size_t SVMModel::MemoryConsumedEstimated ()  const
{
  size_t  memoryConsumedEstimated = sizeof (SVMModel)
       + assignments.MemoryConsumedEstimated ()
       + sizeof (kkint32) * oneVsAllAssignment.size ()
       + sizeof (kkint32) * cardinality_table.size ()
       + rootFileName.MemoryConsumedEstimated ()
       + svmParam->MemoryConsumedEstimated ()
       + sizeof (AttributeType) * type_table.size ();

  if  (binaryFeatureEncoders)
  {
    memoryConsumedEstimated += sizeof (FeatureEncoderPtr) * numOfModels;
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      if  (binaryFeatureEncoders[x])
        memoryConsumedEstimated += binaryFeatureEncoders[x]->MemoryConsumedEstimated ();
    }
  }

  if  (binaryParameters)
    memoryConsumedEstimated += numOfModels * sizeof (BinaryClassParmsPtr);

  if  (classIdxTable)
    memoryConsumedEstimated += numOfClasses * sizeof (MLClassPtr);

  if  (crossClassProbTable)
    memoryConsumedEstimated += crossClassProbTableSize * crossClassProbTableSize * sizeof (double);

  if  (featureEncoder)
    memoryConsumedEstimated += featureEncoder->MemoryConsumedEstimated ();

  // fileDesc  We do not own 'filedesc'.

  if  (models)
  {
    memoryConsumedEstimated +=  numOfModels * sizeof (ModelPtr);
    for  (kkuint32 x = 0;  x < numOfModels;  ++x)
    {
      if  (models[x])
      if  (models[x][0])
       memoryConsumedEstimated += models[x][0]->MemoryConsumedEstimated ();
    }
  }

  if  (oneVsAllClassAssignments)
  {
    memoryConsumedEstimated += numOfModels * sizeof (ClassAssignmentsPtr);
    for  (kkuint32 x = 0;  x < numOfModels;  ++x)
    {
      if  (oneVsAllClassAssignments[x])
        memoryConsumedEstimated += oneVsAllClassAssignments[x]->MemoryConsumedEstimated ();
    }
  }

  if  (predictXSpace)
     memoryConsumedEstimated += sizeof (svm_node) * predictXSpaceWorstCase;

  if  (probabilities)
     memoryConsumedEstimated += sizeof (double) * numOfClasses;

  if  (votes)
     memoryConsumedEstimated += sizeof (kkint32) * numOfClasses;

  if  (xSpaces)
    memoryConsumedEstimated += xSpacesTotalAllocated * sizeof (svm_node) + numOfModels * sizeof (XSpacePtr);

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */



void   SVMModel::CancelFlag (bool  _cancelFlag)
{
  cancelFlag = _cancelFlag;
}



void  SVMModel::BuildClassIdxTable ()
{
  delete[]  classIdxTable;

  classIdxTable = new MLClassPtr[numOfClasses];
  for  (kkuint32 classIdx = 0;  classIdx < numOfClasses;  classIdx++)
    classIdxTable[classIdx] = assignments.GetMLClassByIndex (classIdx);

  probabilities = new double[numOfClasses + 2]; // I am add 2 as a desperate move to deal with some kind of memory corruption  kak
  votes         = new kkint32[numOfClasses + 2];    // 
  
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    probabilities[x] = 0.0;
    votes[x]         = 0;
  }

}  /* BuildClassIdxTable */



void  SVMModel::BuildProblemOneVsAll (FeatureVectorList&    examples,
                                      struct svm_problem&   prob,
                                      XSpacePtr&            xSpace,
                                      const MLClassList&    classesThisAssignment,
                                      MLClassList&          allClasses,
                                      ClassAssignmentsPtr&  classAssignments,
                                      RunLog&               log
                                     )
{ 
  log.Level (20) << "SVMModel::BuildProblemOneVsAll" << endl;

  if  (!selectedFeatures)
  {
    FeatureNumListPtr tempFeatures = new FeatureNumList (examples.FileDesc ());
    SetSelectedFeatures (tempFeatures, log);
    delete  tempFeatures;
    tempFeatures = NULL;
  }

  kkint32  numOfFeaturesSelected = selectedFeatures->NumOfFeatures ();

  delete  classAssignments;
  classAssignments = new ClassAssignments ();

  MLClassList::const_iterator  idx;

  for  (auto mlClass: allClasses)
  {
    if  (classesThisAssignment.PtrToIdx (mlClass))
    {
      // We are looking at a class that is to be treated logically as the 'one' class in 'One-vs-All'
      classAssignments->AddMLClass (mlClass, 0, log);
    }
    else
    {
      // We are looking at a class that is to be treated logically as one of the 'all' classes in 'One-vs-All'
      classAssignments->AddMLClass (mlClass, 1, log);
    }
  }
  
  kkint32  totalxSpaceUsed;

  featureEncoder->EncodeIntoSparseMatrix (&examples,
                                          *classAssignments,
                                          xSpace, 
                                          totalxSpaceUsed,
                                          prob,
                                          log
                                         );

  xSpacesTotalAllocated += totalxSpaceUsed;

  if  (svmParam->Param ().gamma == 0)
    svmParam->Gamma_Param (1.0 / numOfFeaturesSelected);

  return;
}  /* BuildProblemOneVsAll */



void  SVMModel::BuildProblemBinaryCombos (FeatureVectorListPtr  class1Examples, 
                                          FeatureVectorListPtr  class2Examples, 
                                          BinaryClassParmsPtr&  _twoClassParms,
                                          FeatureEncoderPtr&    _encoder,
                                          struct svm_problem&   prob, 
                                          XSpacePtr&            xSpace, 
                                          MLClassPtr            class1, 
                                          MLClassPtr            class2,
                                          RunLog&               log
                                         )
{ 
  log.Level (10) << "SVMModel::BuildProblemBinaryCombos   Class1[" << class1->Name () << "]  Class2[" << class2->Name () << "]" << endl;

  kkint32  totalxSpaceUsed = 0;

  ClassAssignments  binaryAssignments;
  binaryAssignments.AddMLClass (class1, 0, log);
  binaryAssignments.AddMLClass (class2, 1, log);

  _twoClassParms = svmParam->GetParamtersToUseFor2ClassCombo (class1, class2);

  FeatureVectorListPtr  twoClassExamples 
          = new FeatureVectorList (fileDesc, 
                                   false
                                  );
  twoClassExamples->AddQueue (*class1Examples);
  twoClassExamples->AddQueue (*class2Examples);

  FeatureNumListConstPtr selFeatures = svmParam->GetFeatureNums (fileDesc, class1, class2);

  _encoder = new FeatureEncoder (fileDesc,
                                 class1, 
                                 class2,
                                 *selFeatures, 
                                 svmParam->EncodingMethod (),
                                 svmParam->C_Param ()
                                );
  
  _encoder->EncodeIntoSparseMatrix (twoClassExamples,
                                    binaryAssignments,
                                    xSpace,
                                    totalxSpaceUsed,
                                    prob,
                                    log
                                   );

  xSpacesTotalAllocated += totalxSpaceUsed;

  if  (_twoClassParms->Param ().gamma == 0)
  {
    _twoClassParms->Gamma (1.0f / (float)_encoder->CodedNumOfFeatures ());
    cout << endl << endl
         << "Gamma was set to ZERO" << endl
         << endl;
  }

  delete  twoClassExamples;
  twoClassExamples = NULL;
}  /* BuildProblemBinaryCombos */



void  SVMModel::BuildCrossClassProbTable ()
{
  if  (crossClassProbTable)
  {
    for  (kkuint32 x = 0;  x < crossClassProbTableSize;  x++)
      delete  crossClassProbTable[x];
    delete[]  crossClassProbTable;
    crossClassProbTable = NULL;
  }

  crossClassProbTable = NULL;

  if  (numOfClasses > 0)
  {
    crossClassProbTableSize = numOfClasses;
    crossClassProbTable = new double*[crossClassProbTableSize + 2];   // I am adding 2 to deal with a memory corruption problem.   kak
    for  (kkuint32 x = 0;  x < crossClassProbTableSize;  ++x)
    {
      crossClassProbTable[x] = new double[crossClassProbTableSize + 2];
      for  (kkuint32 y = 0;  y < crossClassProbTableSize;  ++y)
        crossClassProbTable[x][y] = 0.0;
    }
  }
}  /* BuildCrossClassProbTable */



FeatureNumListConstPtr  SVMModel::GetFeatureNums ()  const
{
  return  svmParam->GetFeatureNums ();
}



FeatureNumListConstPtr  SVMModel::GetFeatureNums (FileDescPtr  fd)  const
{
  return  svmParam->GetFeatureNums (fd);
}



FeatureNumListConstPtr  SVMModel::GetFeatureNums (FileDescPtr  fd,
                                                  MLClassPtr   class1,
                                                  MLClassPtr   class2
                                                 )  const
{
  return  svmParam->GetFeatureNums (fd, class1, class2);
}  /* GetFeatureNums */



double   SVMModel::DistanceFromDecisionBoundary (FeatureVectorPtr  example,
                                                 MLClassPtr        class1,
                                                 MLClassPtr        class2
                                                )
{
  if  (svmParam->MachineType () != SVM_MachineType::BinaryCombos)
  {
    cerr << endl << "SVMModel::DistanceFromDecisionBoundary   ***ERROR***    This method only works with BinaryCombos." << endl << endl;
    return  0.0;
  }

  kkuint32  modelIDX = 0;
  bool  revClassOrder = false;
  FeatureEncoderPtr  encoder  = NULL;

  while  (modelIDX < numOfModels)
  {
    encoder = binaryFeatureEncoders[modelIDX];
    if  ((encoder->Class1 () == class1)  &&  (encoder->Class2 () == class2))
    {
      revClassOrder = false;
      break;
    }
    else if  ((encoder->Class1 () == class2)  &&  (encoder->Class2 () == class1))
    {
      revClassOrder = true;
      break;
    }

    encoder = NULL;
    modelIDX++;
  }

  if  (encoder == NULL)
  {
    return 0.0;
  }

  kkint32  xSpaceUsed;
  encoder->EncodeAExample (example, predictXSpace, xSpaceUsed);

  double  distance = 0.0;

  svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, -1);

  if  (revClassOrder)
    distance = 0.0 - distance;

  return  distance;
}  /* DistanceFromDecisionBoundary */



void  SVMModel::InializeProbClassPairs ()
{
  if  (svmParam->ProbClassPairs ().size () < 1)
  {
    svmParam->ProbClassPairsInitialize (assignments);
  }
}  /* InializeProbClassPairs */



void   SVMModel::Predict (FeatureVectorPtr  example,
                          MLClassPtr        knownClass,
                          MLClassPtr&       predClass1,
                          MLClassPtr&       predClass2,
                          kkint32&          predClass1Votes,
                          kkint32&          predClass2Votes,
                          double&           probOfKnownClass,
                          double&           predClass1Prob,
                          double&           predClass2Prob,
                          kkint32&          numOfWinners,
                          bool&             knownClassOneOfTheWinners,
                          double&           breakTie
                         )
{
  InializeProbClassPairs ();

  breakTie = 0.0f;   // experiments.

  knownClassOneOfTheWinners = false;

  predClass1 = NULL;
  predClass2 = NULL;

  predClass1Votes = -1;
  predClass2Votes = -1;

  probOfKnownClass = 0.0;
  predClass1Prob   = 0.0;
  predClass2Prob   = -1.0f;

  numOfWinners = 0;

  if  (svmParam->MachineType () == SVM_MachineType::OneVsAll)
  {
    EncodeExample (example, predictXSpace);
    PredictOneVsAll (predictXSpace,   //  used to be xSpace,  
                     knownClass,
                     predClass1,
                     predClass2,
                     probOfKnownClass,
                     predClass1Prob,
                     predClass2Prob,
                     numOfWinners,
                     knownClassOneOfTheWinners,
                     breakTie
                    );
    predClass1Votes = 1;
    predClass2Votes = 1;
    //  free (xSpace);  // We are now allocating only once.
  }

  else if  (svmParam->MachineType () == SVM_MachineType::OneVsOne)
  {  
    EncodeExample (example, predictXSpace);

    kkint32  knownClassNum  = -1;
    kkint32  prediction     = -1;
    kkint32  prediction2    = -1;

    if  (knownClass)
    {
      knownClassNum = (kkint32)assignments.GetNumForClass (knownClass).value_or (-1);
    }

    vector<kkint32>  winners;

    SvmPredictClass (*svmParam,
                     models[0],
                     predictXSpace, 
                     votes,
                     probabilities, 
                     knownClassNum,
                     prediction,
                     prediction2,
                     predClass1Votes,
                     predClass2Votes,
                     predClass1Prob,
                     predClass2Prob,
                     probOfKnownClass,
                     winners,
                     crossClassProbTable,
                     breakTie
                    );

    numOfWinners = (kkint32)winners.size ();

    for  (kkint32 idx = 0;  idx < (kkint32)winners.size ();  idx++)
    {
      if  (winners[idx] == knownClassNum)
      {
        knownClassOneOfTheWinners = true;
        break;
      }
    }

    predClass1 = assignments.GetMLClass ((kkint16)prediction);
    predClass2 = assignments.GetMLClass ((kkint16)prediction2);
    //free (xSpace);
  }

  
  else if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
  {
    PredictByBinaryCombos (example, 
                           knownClass,
                           predClass1,
                           predClass2,
                           predClass1Votes,
                           predClass2Votes,
                           probOfKnownClass,
                           predClass1Prob,
                           predClass2Prob,
                           breakTie,
                           numOfWinners,
                           knownClassOneOfTheWinners
                          );
  }
    
  else
  {
    KKStr  errMsg = "***ERROR*** SVMModel::Predict   Invalid Machine Type Specified.";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  return;
}  /* Predict */



void  SVMModel::PredictRaw (FeatureVectorPtr example,
                            MLClassPtr&      predClass,
                            double&          dist
                           )
{
  InializeProbClassPairs ();

  dist = 0.0;
  double  label = 0.0;
  EncodeExample (example, predictXSpace);
  SvmPredictRaw (models[0], predictXSpace, label, dist);
  predClass  = assignments.GetMLClass ((kkint16)label);
  return;
}  /* PredictRaw */



void   SVMModel::PredictOneVsAll (XSpacePtr    xSpace,
                                  MLClassPtr   knownClass,
                                  MLClassPtr&  predClass1, 
                                  MLClassPtr&  predClass2,  
                                  double&      probOfKnownClass,
                                  double&      predClass1Prob,
                                  double&      predClass2Prob,
                                  kkint32&     numOfWinners,
                                  bool&        knownClassOneOfTheWinners,
                                  double&      breakTie
                                 )
{
  InializeProbClassPairs ();

  predClass1 = NULL;
  predClass2 = NULL;
  knownClassOneOfTheWinners = false;

  probOfKnownClass = 0.0;
  predClass1Prob   = 0.0;

  vector<kkint32>   winningClasses;

  double*  predProbs = new double [numOfModels + 2];  // I am adding 2 as a desperate measure to deal with a memory corruption problem   kak

  double  largestLosingProbability    = FLT_MIN;
  kkint32 largestLosingProbabilityIDX = -1;

  double  secondLargestLosingProbability    = FLT_MIN;
  kkint32 secondLargestLosingProbabilityIDX = -1;

  double  largestWinningProbability    = FLT_MIN;
  kkint32 largestWinningProbabilityIDX = -1;

  double  secondLargestWinningProbability = FLT_MIN;

  kkint32 knownAssignmentIDX = -1;

  kkuint32  assignmentIDX;

  for  (assignmentIDX = 0;  assignmentIDX < oneVsAllAssignment.size ();  ++assignmentIDX)
  {
    auto  assignmentNum = oneVsAllAssignment[assignmentIDX];

    kkint32  knownClassNum = -1;
    kkint32  predClassNum1 = -1;
    kkint32  predClassNum2 = -1;

    MLClassPtr  classWeAreLookingAt = assignments.GetMLClassByIndex (assignmentNum);

    if  (knownClass)
    {
      if  (knownClass == classWeAreLookingAt)
      {
        knownAssignmentIDX = assignmentIDX;
        knownClassNum = 0;
      }
      else
      {
        knownClassNum = 1;
      }
    }

    double  predictedClassProbability  = 0.0f;
    double  predictedClassProbability2 = 0.0f;
    double  knownClassProbabilioty     = 0.0f;

    vector<kkint32>  winners;

    double*  tempProbabilities = new double[numOfClasses + 2];  //  I am adding 2 as a desperate measure to deal with a memory corruption problem   kak
    kkint32* tempVotes         = new kkint32[numOfClasses + 2];

    kkint32  predClass1Votes = -1;
    kkint32  predClass2Votes = -1;

    SvmPredictClass (*svmParam,
                     models[assignmentIDX],
                     xSpace, 
                     tempVotes,
                     tempProbabilities,
                     predClass1Votes,
                     predClass2Votes,
                     knownClassNum,    
                     predClassNum1,
                     predClassNum2,
                     predictedClassProbability,
                     predictedClassProbability2,
                     knownClassProbabilioty,
                     winners,
                     crossClassProbTable,
                     breakTie
                    );

    delete[]  tempVotes;
    tempVotes = NULL;
    delete[]  tempProbabilities;
    tempProbabilities = NULL;

    if  (predClassNum1 == 0)
    {
      winningClasses.push_back (assignmentIDX);
      predProbs[assignmentIDX] = predictedClassProbability;
      if  (predictedClassProbability > largestWinningProbability)
      {
        secondLargestWinningProbability    = largestWinningProbability;

        largestWinningProbability = predictedClassProbability;
        largestWinningProbabilityIDX = assignmentIDX;
      }
      else if  (predictedClassProbability > secondLargestWinningProbability)
      {
        secondLargestWinningProbability    = predictedClassProbability;
      }
    }
    else
    {
      predProbs[assignmentIDX] = 1.0 - predictedClassProbability;
      if  (predProbs[assignmentIDX] > largestLosingProbability)
      {
        secondLargestLosingProbabilityIDX = largestLosingProbabilityIDX;
        secondLargestLosingProbability    = largestLosingProbability;

        largestLosingProbabilityIDX = assignmentIDX;
        largestLosingProbability    = predProbs[assignmentIDX];
      }
      else if  (predProbs[assignmentIDX] > secondLargestLosingProbability)
      {
        secondLargestLosingProbabilityIDX = assignmentIDX;
        secondLargestLosingProbability    = predProbs[assignmentIDX];
      }
    }
  }

  numOfWinners = (kkint32)winningClasses.size ();

  kkint32 assignmentIDXthatWon = -1;
  kkint32 assignmentIDXsecond  = -1;

  if  (winningClasses.size () <= 0)
  {
    // There were no winners,  lets just use the 1 that had the highest probability.

    assignmentIDXthatWon = largestLosingProbabilityIDX;
    assignmentIDXsecond  = secondLargestLosingProbabilityIDX;

    predClass1 = assignments.GetMLClassByIndex (oneVsAllAssignment[assignmentIDXthatWon]);

    knownClassOneOfTheWinners = false;
  }

  else if  (winningClasses.size () == 1)
  {
    assignmentIDXthatWon      = winningClasses[0];
    knownClassOneOfTheWinners = (assignmentIDXthatWon == knownAssignmentIDX);
    assignmentIDXsecond       = largestLosingProbabilityIDX;
  }

  else
  {
    // We had more than one Winner
    assignmentIDXthatWon = largestWinningProbabilityIDX;
    assignmentIDXsecond  = secondLargestLosingProbabilityIDX;

    for  (kkint32  idx = 0;  idx < (kkint32)winningClasses.size ();  idx++)
    {
      if  (winningClasses[idx] == knownAssignmentIDX)
      {
        knownClassOneOfTheWinners = true;
        break;
      }
    }
  }

  predClass1 = assignments.GetMLClassByIndex (oneVsAllAssignment[assignmentIDXthatWon]);
  predClass2 = assignments.GetMLClassByIndex (oneVsAllAssignment[assignmentIDXsecond]);

  predClass1Prob   = (predProbs[oneVsAllAssignment[assignmentIDXthatWon]]);
  predClass2Prob   = (predProbs[oneVsAllAssignment[assignmentIDXsecond]]);
  probOfKnownClass = (predProbs[oneVsAllAssignment[knownAssignmentIDX]]);

  delete[]  predProbs;
  predProbs = NULL;

  return;
}  /* PredictOneVsAll */



MLClassPtr  SVMModel::Predict (FeatureVectorPtr  example)
{
  double      breakTie         = -1.0f;
  bool        knownClassOneOfTheWinners = false;
  kkint32     numOfWinners     = -1;
  MLClassPtr  pred1            = NULL;
  MLClassPtr  pred2            = NULL;
  double      predClass1Prob   = -1.0;
  double      predClass2Prob   = -1.0;
  double      probOfKnownClass = -1.0;

  kkint32     predClass1Votes = -1;
  kkint32     predClass2Votes = -1;
  
  InializeProbClassPairs ();

  Predict (example, 
           NULL,
           pred1,
           pred2,
           predClass1Votes,
           predClass2Votes,
           probOfKnownClass,
           predClass1Prob,
           predClass2Prob,
           numOfWinners,
           knownClassOneOfTheWinners,
           breakTie
          );

  return  pred1;
}  /* Predict */



void  SVMModel::PredictByBinaryCombos (FeatureVectorPtr  example,
                                       MLClassPtr        knownClass,
                                       MLClassPtr&       predClass1,
                                       MLClassPtr&       predClass2,
                                       kkint32&          predClass1Votes,
                                       kkint32&          predClass2Votes,
                                       double&           probOfKnownClass,
                                       double&           predClass1Prob,
                                       double&           predClass2Prob,
                                       double&           breakTie,
                                       kkint32&          numOfWinners,
                                       bool&             knownClassOneOfTheWinners
                                      )
{
  predClass1        = NULL;
  predClass2        = NULL;
  probOfKnownClass  = -1.0f;
  predClass1Prob  = -1.0f;
  predClass2Prob  = -1.0f;

  predClass1Prob = 0.0;
  predClass2Prob = 0.0;

  knownClassOneOfTheWinners = false;

  double  probability  = -1.0;

  kkint32 knownClassIDX = numOfClasses - 1;
  kkint32 predClass1IDX  = -1;
  kkint32 predClass2IDX  = -1;

  kkint32 modelIDX = 0;

  for  (kkuint32 x = 0; x < numOfClasses;  x++)
  {
    votes[x] = 0;
    probabilities[x] = 1.0f;
  }

  for  (kkuint32  class1IDX = 0;  class1IDX < (numOfClasses - 1);  class1IDX++)
  {
    MLClassPtr  class1  = classIdxTable [class1IDX];

    if  (class1 == knownClass)
      knownClassIDX = class1IDX;
     
    for  (kkuint32  class2IDX = (class1IDX + 1);  class2IDX < numOfClasses;  class2IDX++)
    {
      BinaryClassParmsPtr  thisComboPrameters = binaryParameters[modelIDX];

      kkint32  xSpaceUsed;

      if  (binaryFeatureEncoders[modelIDX] == NULL)
      {
        KKStr  errMsg;
        errMsg << "SVMModel::PredictByBinaryCombos   ***ERROR***   No feature encoder for model[" << modelIDX << "]";
        cerr << endl << errMsg << endl << endl;
        throw KKException (errMsg);
      }

      binaryFeatureEncoders[modelIDX]->EncodeAExample (example, predictXSpace, xSpaceUsed);

      double  distance = 0.0;

      svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, -1);
      probability = (1.0 / (1.0 + exp (-1.0 * (thisComboPrameters->Param ().A) * distance)));
      probability = AdjProb (probability);  // KAK 2011-06-10

      if  (probability > 0.5)
      {
        votes[class1IDX]++;
        probabilities[class1IDX] *= probability;
        probabilities[class2IDX] *= (1.0f - probability);
      }
      else
      {
        votes[class2IDX]++;
        probabilities[class2IDX] *= (1.0 - probability);
        probabilities[class1IDX] *= probability;
      }

      modelIDX++;
    }
  }

  double  totProbability = 0.0;
  for  (kkuint32 classIDX = 0;  classIDX < numOfClasses;  classIDX++)
    totProbability += probabilities[classIDX];

  if  (totProbability <= 0.0)
    totProbability = 1.0;

  for  (kkuint32 classIDX = 0;  classIDX < numOfClasses;  classIDX++)
    probabilities[classIDX] = probabilities[classIDX] / totProbability;

  GreaterVotes (svmParam->SelectionMethod () == SVM_SelectionMethod::Probability,
                numOfClasses,
                votes,
                numOfWinners,
                probabilities,
                predClass1IDX,
                predClass2IDX
               );

  if  (predClass1IDX >= 0)
  {
    predClass1      = classIdxTable [predClass1IDX];
    predClass1Votes = votes         [predClass1IDX];
    predClass1Prob  = probabilities [predClass1IDX];
  }

  if  (predClass2IDX >= 0)
  {
    predClass2      = classIdxTable [predClass2IDX];
    predClass2Votes = votes         [predClass2IDX];
    predClass2Prob  = probabilities [predClass2IDX];
  }

  if  (knownClassIDX >= 0)
    probOfKnownClass = probabilities[knownClassIDX];
  
  breakTie = fabs (predClass1Prob - predClass2Prob);
  knownClassOneOfTheWinners = (predClass1IDX == knownClassIDX);

  return;
}  /* PredictByBinaryCombos */



kkint32  SVMModel::NumOfSupportVectors ()  const
{
  kkint32  numOfSupportVectors = 0;

  if  (models == NULL)
    return 0;

  if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
  {
    vector<KKStr> svNames = SupportVectorNames ();
    numOfSupportVectors = (kkint32)svNames.size ();
  }
  else
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
      numOfSupportVectors += models[x][0]->l;
  }

  return  numOfSupportVectors;
}  /* NumOfSupportVectors */



void  SVMModel::SupportVectorStatistics (kkint32&  numSVs,
                                         kkint32&  totalNumSVs
                                        )
{
  numSVs = 0;
  totalNumSVs = 0;
  if  (models == NULL)
    return;

  if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
  {
    numSVs = NumOfSupportVectors ();
    for  (kkuint32 modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
    {
      totalNumSVs += models[modelIDX][0]->l;
    }
  }
  else
  {
    svm_GetSupportVectorStatistics (models[0][0], numSVs, totalNumSVs);
  }

}  /* SupportVectorStatistics */



void  SVMModel::ProbabilitiesByClass (FeatureVectorPtr    example,
                                      const MLClassList&  _mlClasses,
                                      kkint32*            _votes,
                                      double*             _probabilities,
                                      RunLog&             _log
                                     )
{
  InializeProbClassPairs ();

  if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
  {
    PredictProbabilitiesByBinaryCombos (example, _mlClasses, _votes, _probabilities, _log);
    return;
  }

  kkint32  predClass1Votes   = -1;
  kkint32  predClass2Votes   = -1;
  double   predClass1Prob    = 0.0;
  double   predClass2Prob    = 0.0;
  double   probOfKnownClass  = 0.0;

  double   breakTie                = 0.0f;

  MLClassPtr  predictedClass  = NULL;
  XSpacePtr   xSpace          = NULL;

  xSpace = featureEncoder->EncodeAExample (example);

  kkint32  knownClassNum = 0;
  kkint32  prediction    = 0;
  kkint32  prediction2   = 0;

  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
  {
    probabilities [x] = 0.0;
    _probabilities[x] = 0.0;
    _votes        [x] = 0;
  }

  vector<kkint32>  winners;

  SvmPredictClass (*svmParam,
                   models[0],
                   xSpace, 
                   votes,
                   probabilities, 
                   knownClassNum, 
                   prediction,
                   prediction2,
                   predClass1Votes,
                   predClass2Votes,
                   predClass1Prob,
                   predClass2Prob,
                   probOfKnownClass,
                   winners,
                   crossClassProbTable,
                   breakTie
                  );

  // We have  to do it this way, so that we pass back the probabilities in the 
  // same order that the user indicated with mlClasses being passed in.
  for  (kkuint32  x = 0;  x < numOfClasses;  x++)
  {
    predictedClass = assignments.GetMLClass ((kkint16)x);
    if  (predictedClass)
    {
      auto y = _mlClasses.PtrToIdx (predictedClass);
      if  (!y  ||  (y >= numOfClasses))
      {
        _log.Level (-1) << endl
                << "SVMModel::ProbabilitiesByClass   ***ERROR***"                                                     << endl
                << "                                 Invalid classIdx[" << y                       << "]  Specified." << endl
                << "                                 ClassName       [" << predictedClass->Name () << "]"             << endl
                << endl;
      }
      else
      {
        _votes        [y.value ()] = votes        [x];
        _probabilities[y.value ()] = probabilities[x];
      }
    }
  }

  delete  xSpace;  
  xSpace = NULL;

  return;
}  /* ProbabilitiesByClass */



void  SVMModel::PredictProbabilitiesByBinaryCombos (FeatureVectorPtr    example,  
                                                    const MLClassList&  _mlClasses,
                                                    kkint32*            _votes,
                                                    double*             _probabilities,
                                                    RunLog&             _log
                                                   )
{
  InializeProbClassPairs ();

  double  probability = -1.0;

  kkuint32 modelIDX = 0;

  for  (kkuint32 x = 0; x < numOfClasses;  x++)
  {
    probabilities[x] = 1.0;
    votes[x] = 0;
  }


  if  (numOfClasses != crossClassProbTableSize)
  {
    _log.Level (-1) << endl << endl 
                   << "SVMModel::PredictProbabilitiesByBinaryCombos                     ***ERROR***" << endl
                   << "                      numfClasses != crossClassProbTableSize"                 << endl       
                   << endl;
    return;
  }


  for  (kkuint32  class1IDX = 0;  class1IDX < (numOfClasses - 1);  class1IDX++)
  {

    for  (kkuint32  class2IDX = (class1IDX + 1);  class2IDX < numOfClasses;  class2IDX++)
    {
      BinaryClassParmsPtr  thisComboPrameters = binaryParameters[modelIDX];

      if  (binaryFeatureEncoders[modelIDX] == NULL)
      {
        KKStr  errMsg;
        errMsg << "SVMModel::PredictProbabilitiesByBinaryCombos   ***ERROR***   No feature encoder for model[" << modelIDX << "]";
        _log.Level (-1) << endl << endl << errMsg << endl << endl;
        throw KKException (errMsg);
      }

      kkint32  xSpaceUsed;
      binaryFeatureEncoders[modelIDX]->EncodeAExample (example, predictXSpace, xSpaceUsed);

      double  distance = 0.0;

      svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, -1);
      probability = (1.0 / (1.0 + exp (-1.0 * (thisComboPrameters->Param ().A) * distance)));
      probability = AdjProb (probability);  // KAK 2011-06-10

      crossClassProbTable[class1IDX][class2IDX] = probability;
      crossClassProbTable[class2IDX][class1IDX] = 1.0 - probability;

      if  (probability > 0.5f)
      {
        probabilities[class1IDX] *= probability;
        probabilities[class2IDX] *= (1.0f - probability);
        votes[class1IDX]++;
      }
      else
      {
        probabilities[class2IDX] *= (1.0f - probability);
        probabilities[class1IDX] *= probability;
        votes[class2IDX]++;
      }

      modelIDX++;
    }
  }

  double  totProbability = 0.0;

  for  (kkuint32 classIDX = 0;  classIDX < numOfClasses;  classIDX++)
  {
    totProbability += probabilities[classIDX];
  }

  if  (totProbability == 0.0f)
    totProbability = 1.0f;

  {
    kkint32  callersIdx = 0;
    MLClassList::const_iterator  idx;
    for  (idx = _mlClasses.begin ();  idx != _mlClasses.end ();  idx++)
    {
      auto  ourIdx = assignments.GetNumForClass (*idx);
      if  ((!ourIdx)  ||  (ourIdx >= numOfClasses))
      {
        // For what ever reason the MLClass instance in '_mlClasses' provided by caller
        // is not one of the classes that this model was built for.
        _log.Level (-1) << endl 
          << "SVMModel::PredictProbabilitiesByBinaryCombos   ***WARNING***    MLClass[" << (*idx)->Name () << "] is not one of the classes in SVMModel."  << endl
          << endl;
        _votes        [callersIdx] = 0;
        _probabilities[callersIdx] = 0.0;
      }
      else
      {
        _votes        [callersIdx] = votes        [ourIdx.value ()];
        _probabilities[callersIdx] = probabilities[ourIdx.value ()] / totProbability;
      }
      callersIdx++;
    }
  }

  return;
}  /* PredictByBinaryCombos */



namespace  KKMLL
{
  bool  PairCompareOperator(ProbNamePair l, ProbNamePair  r) 
  { 
    return l.probability > r.probability;
  }
}



vector<KKStr>  SVMModel::SupportVectorNames (MLClassPtr     c1,
                                             MLClassPtr     c2
                                            )  const
{
  vector<KKStr>  results;
  if  (svmParam->MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  kkuint32  modelIDX = 0;
  BinaryClassParmsPtr  parms = NULL;
  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
  {
    parms = binaryParameters[modelIDX];
    if  ((parms->Class1 () == c1)  &&  (parms->Class2 () == c2))
      break;

    else if  ((parms->Class2 () == c1)  &&  (parms->Class1 () == c2))
      break;
  }

  if  (modelIDX >= numOfModels)
  {
    // Binary Combo not found.
    cerr << endl
         << "SVMModel::SupportVectorNames   ***ERROR***  Class1[" << c1->Name () << "]  Class2[" << c2->Name () << "]  not part of model." << endl
         << endl;
    return results;
  }

  kkint32  numSVs = models[modelIDX][0]->l;
  kkint32  svIDX = 0;
  for  (svIDX = 0;  svIDX < numSVs;  svIDX++)
  {
    KKStr  svName = models[modelIDX][0]->SupportVectorName (svIDX);
    results.push_back (svName);
  }
  return  results;
}  /* SupportVectorNames */



vector<KKStr>  SVMModel::SupportVectorNames () const
{
  vector<KKStr>  results;
  if  (svmParam->MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  map<KKStr,KKStr>  names;
  map<KKStr,KKStr>::iterator  svnIDX;

  // Locate the binary parms in question.
  kkuint32  modelIDX = 0;
  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
  {
    kkint32  numSVs = models[modelIDX][0]->l;
    kkint32  svIDX = 0;
    for  (svIDX = 0;  svIDX < numSVs;  svIDX++)
    {
      KKStr  svName = models[modelIDX][0]->SupportVectorName (svIDX);
      svnIDX = names.find (svName);
      if  (svnIDX == names.end ())
      {
        names.insert (pair<KKStr,KKStr>(svName, svName));
        results.push_back (svName);
      }
    }
  }

  return  results;
}  /* SupportVectorNames */



vector<ProbNamePair>  SVMModel::FindWorstSupportVectors (FeatureVectorPtr  example,
                                                         kkint32           numToFind,
                                                         MLClassPtr        c1,
                                                         MLClassPtr        c2
                                                        )
{
  vector<ProbNamePair>  results;
  if  (svmParam->MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  bool  c1RevFlag = false;
  kkuint32  modelIDX = 0;

  BinaryClassParmsPtr  parms = NULL;

  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
  {
    parms = binaryParameters[modelIDX];
    if  ((parms->Class1 () == c1)  &&  (parms->Class2 () == c2))
    {
      c1RevFlag = false;
      break;
    }
    else if  ((parms->Class2 () == c1)  &&  (parms->Class1 () == c2))
    {
      c1RevFlag = true;
      break;
    }
  }

  if  (modelIDX >= numOfModels)
  {
    // Binary Combo not found.
    cerr << endl
         << "SVMModel::FindWorstSupportVectors   ***ERROR***  Class1[" << c1->Name () << "]  Class2[" << c2->Name () << "]  not part of model." << endl
         << endl;
    return results;
  }

  if  (binaryFeatureEncoders[modelIDX] == NULL)
  {
    KKStr  errMsg;
    errMsg << "SVMModel::FindWorstSupportVectors   ***ERROR***   No feature encoder for model[" << modelIDX << "]";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  kkint32  xSpaceUsed;
  binaryFeatureEncoders[modelIDX]->EncodeAExample (example, predictXSpace, xSpaceUsed);

  kkint32  svIDX = 0;
  //kkint32  numSVs = models[modelIDX][0]->l;
  kkint32  numSVs = models[modelIDX][0]->l;

  double  origProbabilityC1 = 0.0;
  double  probabilityC1     = 0.0;
  double  distance          = 0.0;

  svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, -1);
  origProbabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
  origProbabilityC1 = AdjProb (origProbabilityC1);  // KAK 2011-06-10

  if  (c1RevFlag)
    origProbabilityC1 = 1.0f - origProbabilityC1;

  vector<ProbNamePair>  candidates;

  for  (svIDX = 0;  svIDX < numSVs;  svIDX++)
  {
    svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, svIDX);
    probabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
    probabilityC1 = AdjProb (probabilityC1);  // KAK 2011-06-10
    if  (c1RevFlag)
      probabilityC1 = 1.0f - probabilityC1;

    double  deltaProb = probabilityC1 - origProbabilityC1;
    KKStr   svName = models[modelIDX][0]->SupportVectorName (svIDX);
    candidates.push_back (ProbNamePair (svName, deltaProb));
  }

  sort (candidates.begin (), candidates.end (), PairCompareOperator);

  kkint32  zed = 0;
  for  (zed = 0;  (zed < (kkint32)candidates.size ())  &&  (zed < numToFind);  zed++)
    results.push_back (candidates[zed]);

  return  results;
}  /* FindWorstSupportVectors */



vector<ProbNamePair>  SVMModel::FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                          kkint32           numToFind,
                                                          MLClassPtr     c1,
                                                          MLClassPtr     c2
                                                         )
{
  vector<ProbNamePair>  results;
  if  (svmParam->MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  bool  c1RevFlag = false;
  kkuint32  modelIDX = 0;

  BinaryClassParmsPtr  parms = NULL;

  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
  {
    parms = binaryParameters[modelIDX];
    if  ((parms->Class1 () == c1)  &&  (parms->Class2 () == c2))
    {
      c1RevFlag = false;
      break;
    }
    else if  ((parms->Class2 () == c1)  &&  (parms->Class1 () == c2))
    {
      c1RevFlag = true;
      break;
    }
  }

  if  (modelIDX >= numOfModels)
  {
    // Binary Combo not found.
    cerr << endl
         << "SVMModel::FindWorstSupportVectors   ***ERROR***  Class1[" << c1->Name () << "]  Class2[" << c2->Name () << "]  not part of model." << endl
         << endl;
    return results;
  }

  if  (binaryFeatureEncoders[modelIDX] == NULL)
  {
    KKStr  errMsg;
    errMsg << "SVMModel::FindWorstSupportVectors2   ***ERROR***   No feature encoder for model[" << modelIDX << "]";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  kkint32  xSpaceUsed;
  binaryFeatureEncoders[modelIDX]->EncodeAExample (example, predictXSpace, xSpaceUsed);

  kkint32  svIDX = 0;
  //kkint32  numSVs = models[modelIDX][0]->l;
  kkint32  numSVs = models[modelIDX][0]->l;

  double  origProbabilityC1 = 0.0;
  double  probabilityC1     = 0.0;
  double  distance          = 0.0;

  svm_predictTwoClasses (models[modelIDX][0], predictXSpace, distance, -1);
  origProbabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
  origProbabilityC1 = AdjProb (origProbabilityC1);
  if  (c1RevFlag)
    origProbabilityC1 = 1.0 - origProbabilityC1;

  vector<ProbNamePair>  candidates;

  {
    svm_problem*  subSetProb = svm_BuildProbFromTwoClassModel  (models[modelIDX][0], -1);
    svm_parameter  parameters = models[modelIDX][0]->param;
    parameters.nr_class = 2;

    SvmModel233*  subSetModel = svm_train  (subSetProb, &parameters);

    svm_predictTwoClasses (subSetModel, predictXSpace, distance, -1);
    probabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
    probabilityC1 = AdjProb (probabilityC1);
    if  (c1RevFlag)
      probabilityC1 = 1.0f - probabilityC1;
  }

  for  (svIDX = 0;  svIDX < numSVs;  svIDX++)
  {
    svm_problem*  subSetProb = svm_BuildProbFromTwoClassModel  (models[modelIDX][0], svIDX);
    svm_parameter  parameters = models[modelIDX][0]->param;
    parameters.nr_class = 2;

    SvmModel233*  subSetModel = svm_train  (subSetProb, &parameters);

    svm_predictTwoClasses (subSetModel, predictXSpace, distance, -1);
    probabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
    probabilityC1 = AdjProb (probabilityC1);
    if  (c1RevFlag)
      probabilityC1 = 1.0f - probabilityC1;

    double  deltaProb = probabilityC1 - origProbabilityC1;

    KKStr  svName = models[modelIDX][0]->SupportVectorName (svIDX);
    candidates.push_back (ProbNamePair (svName, deltaProb));

    delete  subSetModel;  subSetModel = NULL;
    delete  subSetProb;   subSetProb  = NULL;
  }

  sort (candidates.begin (), candidates.end (), PairCompareOperator);

  kkint32  zed = 0;
  for  (zed = 0;  (zed < (kkint32)candidates.size ())  &&  (zed < numToFind);  zed++)
    results.push_back (candidates[zed]);

  return  results;
}  /* FindWorstSupportVectors2 */



void SVMModel::CalculatePredictXSpaceNeeded (RunLog&  log)
{
  if  (!selectedFeatures)
  {
    KKStr  errMsg = "SVMModel::CalculatePredictXSpaceNeeded   ***ERROR***    numOfFeaturesSelected  is NOT defined.";
    log.Level (-1) << endl << errMsg << endl
      << endl;
    throw KKException (errMsg);
  }

  kkint32 z;
  kkint32 numFeaturesAfterEncoding = 0;
  kkint32 numOfFeaturesSelected = selectedFeatures->NumOfFeatures ( );

  switch (svmParam->EncodingMethod())
  {
  case SVM_EncodingMethod::Binary:
    for  (z = 0;  z < numOfFeaturesSelected;  z++)
    {
      if  ((type_table[(*selectedFeatures)[z]] == AttributeType::Nominal)  ||
           (type_table[(*selectedFeatures)[z]] == AttributeType::Symbolic)
          )
         numFeaturesAfterEncoding += cardinality_table[(*selectedFeatures)[z]];
      else
         numFeaturesAfterEncoding ++;
    }
    break;

  case SVM_EncodingMethod::Scaled:
  case SVM_EncodingMethod::NoEncoding:
  default:
    //numFeaturesAfterEncoding = fileDesc->NumOfFields ( );
    numFeaturesAfterEncoding = selectedFeatures->NumOfFeatures ();
    break;
  }

  numFeaturesAfterEncoding++;  // extra node for -1 index 

  predictXSpaceWorstCase = numFeaturesAfterEncoding + 10;

  // We need to make sure that 'predictXSpace' is bigger than the worst possible case.
  // When doing the Binary Combo case we will assume that this can be all EncodedFeatures.
  // Since I am really only worried about the BinaryCombo case with Plankton data where there 
  // is not encoded fields the number of attributes in the FileDesc should surface.
  if  (predictXSpaceWorstCase < (fileDesc->NumOfFields () + 10))
     predictXSpaceWorstCase = fileDesc->NumOfFields () + 10;


  delete  predictXSpace;
  predictXSpace = new svm_node[predictXSpaceWorstCase];
}  /* CalculatePredictXSpaceNeeded */



kkint32  SVMModel::EncodeExample (FeatureVectorPtr  example,
                                  svm_node*         row
                                 )
{
  if  (!featureEncoder)
  {
    featureEncoder = new FeatureEncoder (fileDesc,
                                         NULL,               /**< class1, set to NULL because we are dealing with all classes */
                                         NULL,               /**< class2,     ""          ""            ""            ""      */
                                         *selectedFeatures,
                                         svmParam->EncodingMethod (),
                                         svmParam->C_Param ()
                                        );
  }

  kkint32  xSpaceNodesNeeded = 0;
  featureEncoder->EncodeAExample (example, row, xSpaceNodesNeeded);
  return  xSpaceNodesNeeded;
}  /* EncodeExample */



void SVMModel::ConstructOneVsOneModel (FeatureVectorListPtr  examples,
                                       svm_problem&          prob,
                                       RunLog&               log
                                      )
{
  //**** Start of new compression replacement code.
  kkint32  totalxSpaceUsed = 0;

  numOfModels = 1;
  models      = new ModelPtr  [numOfModels];
  xSpaces     = new XSpacePtr [numOfModels];

  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      models[x]  = NULL;
      xSpaces[x] = NULL;
    }
  }

  delete  featureEncoder;
  featureEncoder = new FeatureEncoder (fileDesc,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL,              // class2,     ""          ""            ""            ""
                                       *selectedFeatures,
                                       svmParam->EncodingMethod (),
                                       svmParam->C_Param ()
                                      );

  featureEncoder->EncodeIntoSparseMatrix (examples,
                                          assignments,
                                          xSpaces[0], 
                                          totalxSpaceUsed,
                                          prob,
                                          log
                                         );
  xSpacesTotalAllocated += totalxSpaceUsed;

  //**** End of new compression replacement code.
  // train the model using the svm_problem built above
  double  startTrainingTime = osGetSystemTimeUsed ();
  models[0] = SvmTrainModel (svmParam->Param (), prob);
  double  endTrainingTime = osGetSystemTimeUsed ();
  trainingTime = endTrainingTime - startTrainingTime;

  // free the memory for the svm_problem
  delete[] prob.index;  prob.index = NULL;
  free (prob.y);        prob.y = NULL;
  if  (xSpaces[0] != NULL)
  {
    // We can only free prob.x if xSpacs[0] was allocated otherwise we need
    // prob.x to located xSpace that will need to be deleted later.
    free (prob.x);  prob.x = NULL;
  }
  delete[]  prob.W;     prob.W = NULL;
}  /* ConstructOneVsOneModel */



void SVMModel::ConstructOneVsAllModel (FeatureVectorListPtr examples,
                                       svm_problem&         prob,
                                       RunLog&              log
                                      )
{
  MLClassListPtr  allClasses = examples->ExtractListOfClasses ();

  auto  assignmentNums = assignments.GetUniqueListOfAssignments ();
  numOfModels = (kkint32)assignmentNums.size ();

  models   = new ModelPtr  [numOfModels];
  xSpaces  = new XSpacePtr [numOfModels];
  {
    for  (kkuint32 x = 0;  x < numOfModels;  x++)
    {
      models[x]  = NULL;
      xSpaces[x] = NULL;
    }
  }

  featureEncoder = new FeatureEncoder (fileDesc,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL,              // class2,     ""          ""            ""            ""
                                       *selectedFeatures,
                                       svmParam->EncodingMethod (),
                                       svmParam->C_Param ()
                                      );

  kkuint32  modelIDX = 0;

  trainingTime = 0;
  oneVsAllAssignment.erase (oneVsAllAssignment.begin (), oneVsAllAssignment.end ());

  {
    delete  oneVsAllClassAssignments;
    oneVsAllClassAssignments = new ClassAssignmentsPtr[numOfModels];
    for  (kkuint32 oneVsAllIDX = 0;  oneVsAllIDX < numOfModels;  ++oneVsAllIDX)
      oneVsAllClassAssignments[oneVsAllIDX] = NULL;
  }

  // build the models
  for (kkuint32 assignmentIDX = 0;  assignmentIDX < numOfModels;  ++assignmentIDX)
  {
    auto  assignmentNum = assignmentNums[assignmentIDX];
    oneVsAllAssignment.push_back (assignmentNum);

    MLClassList  classesThisAssignment = assignments.GetMLClasses (assignmentNum);

    BuildProblemOneVsAll (*examples, 
                          prob, 
                          xSpaces[modelIDX], 
                          classesThisAssignment, 
                          *allClasses, 
                          oneVsAllClassAssignments [modelIDX],
                          log
                         );

    // train the model using the svm_problem built above

    double  trainTimeStart = osGetSystemTimeUsed ();
    models[modelIDX] = SvmTrainModel (svmParam->Param (), prob);
    double  trainTimeEnd   = osGetSystemTimeUsed ();
    trainingTime += (trainTimeEnd - trainTimeStart);

    // free the memory for the svm_problem
    delete  [] prob.index;  prob.index = NULL;
    free (prob.y);   prob.y = NULL;
    free (prob.x);   prob.x = NULL;
    delete[] prob.W;

    modelIDX++;
  }

  delete  allClasses;
}  /* ConstructOneVsAllModel */



void SVMModel::ConstructBinaryCombosModel (FeatureVectorListPtr  examples,
                                           RunLog&               log
                                          )
{
  log.Level (10) << "SVMModel::ConstructBinaryCombosModel" << endl;

  kkint32 maxXSpaceNeededPerExample = 0;
  
  numOfModels           = (numOfClasses * (numOfClasses - 1)) / 2;

  models                 = new ModelPtr            [numOfModels];
  xSpaces                = new XSpacePtr           [numOfModels];
  binaryParameters       = new BinaryClassParmsPtr [numOfModels];
  binaryFeatureEncoders  = new FeatureEncoderPtr   [numOfModels];

  kkuint32  modelIDX = 0;

  for  (kkuint32 x = 0;  x < numOfModels;  x++)
  {
    models                [x] = NULL;
    binaryParameters      [x] = NULL;
    binaryFeatureEncoders [x] = NULL;
    xSpaces               [x] = NULL;
  }

  FeatureVectorListPtr srcExamples      = examples;
  FeatureVectorListPtr compressedExamples = NULL;

  FeatureVectorListPtr*   examplesByClass = BreakDownExamplesByClass (srcExamples);


  // NOTE: compression is performed in the BuildProblemBinaryCombos() function since
  // we can do the compression on just the example for those two classes - KNS

  // build a model for each possible 2-class combination
  for (kkuint32 class1IDX = 0;  (class1IDX < (numOfClasses - 1))  &&  (!cancelFlag);  class1IDX++)
  {
    MLClassPtr  class1 = assignments.GetMLClassByIndex (class1IDX);

    for  (kkuint32 class2IDX = class1IDX + 1;   (class2IDX < numOfClasses)   &&  (!cancelFlag);   class2IDX++)
    {
      MLClassPtr  class2 = assignments.GetMLClassByIndex (class2IDX);

      log.Level (20) << "ConstructBinaryCombosModel  Class1[" << class1->Name () << "]  Class2[" << class2->Name () << "]" << endl;

      struct svm_problem  prob;
      //memset (&prob, 0, sizeof(svm_problem));  kak

      binaryParameters      [modelIDX] = NULL;
      binaryFeatureEncoders [modelIDX] = NULL;
      xSpaces               [modelIDX] = NULL;

      // build the svm_problem for the current combination
      BuildProblemBinaryCombos (examplesByClass[class1IDX],
                                examplesByClass[class2IDX],
                                binaryParameters      [modelIDX],
                                binaryFeatureEncoders [modelIDX],
                                prob, 
                                xSpaces               [modelIDX], 
                                class1, 
                                class2, 
                                log
                               );

      maxXSpaceNeededPerExample = Max (maxXSpaceNeededPerExample, binaryFeatureEncoders [modelIDX]->XSpaceNeededPerExample ());

      // train the model
      double  startTrainingTime = osGetSystemTimeUsed ();
      models[modelIDX] = SvmTrainModel (binaryParameters[modelIDX]->Param (), prob);
      double  endTrainingTime = osGetSystemTimeUsed ();
      trainingTime += (endTrainingTime - startTrainingTime);

      // free the memory for the svm_problem
      delete  [] prob.index;  prob.index = NULL;
      free (prob.y);      prob.y     = NULL;
      free (prob.x);      prob.x     = NULL;
      delete[] prob.W;

      modelIDX++;   
    }
  }

  {
    for  (kkint32 x = 0;  x < (kkint32)assignments.size ();  x++)
      delete  examplesByClass[x];
    delete[]  examplesByClass;
    examplesByClass = NULL;
  }

  predictXSpaceWorstCase = maxXSpaceNeededPerExample + 10;

  // We need to make sure that 'predictXSpace' is bigger than the worst possible case.
  // When doing the Binary Combo case we will assume that this can be all EncodedFeatures.
  // Since I am really only worried about the BinaryCombo case with Plankton data where there 
  // is not encoded fields the number of attributes in the FileDesc should suffice.
  if  (predictXSpaceWorstCase < (fileDesc->NumOfFields () + 10))
    predictXSpaceWorstCase = fileDesc->NumOfFields () + 10;

  delete  predictXSpace;  predictXSpace = NULL;
  predictXSpace = new svm_node[predictXSpaceWorstCase];

  delete  compressedExamples;

  log.Level (10) << "SVMModel::ConstructBinaryCombosModel  Done." << endl;
}  /* ConstructBinaryCombosModel */



FeatureVectorListPtr*   SVMModel::BreakDownExamplesByClass (FeatureVectorListPtr  examples)
{
  FeatureVectorListPtr* examplesByClass = new FeatureVectorListPtr[numOfClasses];
  for  (kkuint32 x = 0;  x < numOfClasses;  x++)
    examplesByClass[x] = new FeatureVectorList (fileDesc, false);

  MLClassPtr  lastMLClass = NULL;
  kkint32     classIdx     = 0;

  FeatureVectorList::iterator  idx;

  for  (auto example: *examples)
  {
    if  (lastMLClass != example->MLClass ())
    {
      lastMLClass = example->MLClass ();
      classIdx    = assignments.GetNumForClass (lastMLClass).value ();
    }

    examplesByClass[classIdx]->PushOnBack (example);
  }

  return  examplesByClass;
}  /* BreakDownExamplesByClass */



bool  SVMModel::NormalizeNominalAttributes ()
{
  if  (svmParam->EncodingMethod () == SVM_EncodingMethod::NoEncoding)
    return  true;
  else
    return  false;
}  /* NormalizeNominalAttributes */



void  SVMModel::SetSelectedFeatures (FeatureNumListConst& _selectedFeatures,
                                     RunLog&              _log
                                    )
{
  delete  selectedFeatures;
  selectedFeatures = new FeatureNumList (_selectedFeatures);
  CalculatePredictXSpaceNeeded (_log);
}  /* SetSelectedFeatures */



void  SVMModel::SetSelectedFeatures (FeatureNumListConstPtr  _selectedFeatures,
                                     RunLog&                 _log
                                    )
{
  delete  selectedFeatures;
  selectedFeatures = new FeatureNumList (*_selectedFeatures);
  CalculatePredictXSpaceNeeded (_log);
}  /* SetSelectedFeatures */



void  SVMModel::RetrieveCrossProbTable (MLClassList&   classes,
                                        double**       crossProbTable,  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                        RunLog&        log
                                       )
{
  if  (classes.QueueSize () != crossClassProbTableSize)
  {
    // There Class List does not have the same number of entries as our 'CrossProbTable'
    log.Level (-1) << endl
                   << "SVMModel::RetrieveCrossProbTable   ***ERROR***   classes.QueueSize (): " << classes.QueueSize () << " != crossClassProbTableSize: " << crossClassProbTableSize << endl
                   << endl;
    return;
  }

  OptionUInt32*  indexTable = new OptionUInt32[classes.QueueSize ()];

  for  (kkuint32 x = 0;  x < classes.QueueSize ();  x++)
  {
    for  (kkuint32 y = 0;  y < classes.QueueSize ();  y++)
       crossProbTable[x][y] = 0.0;

    indexTable[x] = assignments.GetNumForClass (classes.IdxToPtr (x));
    if  (!indexTable[x])
    {
      log.Level (-1) << endl
                     << "SVMModel::RetrieveCrossProbTable   ***WARNING***   Class Index: " << x << " Name: " << classes[x].Name ()
                     << "  will populate this index with zeros." 
                     << endl << endl;
    }
  }

  if  (classes.QueueSize () != crossClassProbTableSize)
  {
    log.Level (-1) << endl
                   << "SVMModel::RetrieveCrossProbTable   ***ERROR***   'classes.QueueSize () != crossClassProbTableSize'"  << endl
                   << endl;
    return;
  }
  
  // x,y         = 'Callers'   Class Indexes..
  // xIdx, yIdx  = 'SVMNodel'  Class Indexed.
  for  (kkuint32 x = 0;  x < classes.QueueSize ();  x++)
  {
    auto xIdx = indexTable[x];
    if  (xIdx)
    {
      for  (kkuint32 y = 0;  y < classes.QueueSize ();  y++)
      {
        auto  yIdx = indexTable[y];
        if  (yIdx)
          crossProbTable[x][y] = this->crossClassProbTable[xIdx.value ()][yIdx.value ()];
      }
    }
  }

  delete[]  indexTable;  indexTable = NULL;
  return;
}  /* RetrieveCrossProbTable */



void  SVMModel::WriteXML (const KKStr&  varName,
                          ostream&      o
                         )  const
{
  XmlTag  startTag ("SVMModel",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  if  (fileDesc)
    fileDesc->WriteXML ("FileDesc", o);

  {
    XmlElementKeyValuePairs*   headerFields = new XmlElementKeyValuePairs ();

    headerFields->Add ("RootFileName",      rootFileName);
    headerFields->Add ("Time",              osGetLocalDateTime ());
    headerFields->Add ("NumOfModels",       numOfModels);
    if  (selectedFeatures)
      headerFields->Add ("SelectedFeatures", selectedFeatures->ToString ());

    headerFields->Add ("TrainingTime",      trainingTime);
    headerFields->Add ("Assignments",       assignments.ToString ());
    headerFields->WriteXML ("HeaderFields", o);
    delete  headerFields;
    headerFields = NULL;
  }
  svmParam->WriteXML ("svmParam", o);
   
  if  (svmParam->MachineType () == SVM_MachineType::OneVsOne)
  {
    //  "<OneVsOne>"
    rootFileName.WriteXML ("RootFileName", o);
    if  (models  &&  (models[0])  &&  models[0][0])
      models[0][0]->WriteXML ("OneVsOneModel", o);
  }

  else if  (svmParam->MachineType () == SVM_MachineType::OneVsAll)
  {
    // Not Supported
  }

  else if  (svmParam->MachineType () == SVM_MachineType::BinaryCombos)
  {
    for  (kkuint32 modelsIDX = 0;  modelsIDX < numOfModels;  ++modelsIDX)
    {
      BinaryClassParmsPtr  binClassParms = binaryParameters[modelsIDX];
      KKStr  binaryClassNames (256U);
      binaryClassNames << modelsIDX << "\t" << binClassParms->Class1Name () << "\t"  << binClassParms->Class2Name ();
      binaryClassNames.WriteXML ("BinaryCombo", o);
      KKStr  binaryComboModelName = "BinaryComboModel_" + StrFormatInt (modelsIDX, "000");
      models[modelsIDX][0]->WriteXML (binaryComboModelName, o);
    }
  }

  XmlTag  endTag ("SVMModel", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  SVMModel::ReadXML (XmlStream&      s,
                         XmlTagConstPtr  tag,
                         VolConstBool&   cancelReadXML,
                         RunLog&         log
                        )
{
  log.Level (30) << "SVMModel::ReadXML   tag: " << tag->Name () << endl;

  kkuint32      numModeLoaded = 0;
  KKStr         lastBinaryClass1Name = "";
  KKStr         lastBinaryClass2Name = "";
  OptionUInt32  lastModelIdx = {};

  DeleteModels ();
  DeleteXSpaces ();

  delete  binaryParameters;       binaryParameters      = NULL;
  delete  models;                 models                = NULL;
  delete  binaryFeatureEncoders;  binaryFeatureEncoders = NULL;

  numOfClasses = 0;
  numOfModels  = 0;

  DateTime  timeSaved;

  bool  errorsFound = false;
  XmlTokenPtr  t = NULL;
  while  (!errorsFound)
  {
    delete  t;
    t = s.GetNextToken (cancelReadXML, log);
    if  ((!t)  || cancelReadXML)
      break;

    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr e = dynamic_cast<XmlElementPtr> (t);
      if  (!e)
        continue;

      const KKStr&  varName = e->VarName ();

      if  (varName.EqualIgnoreCase ("FileDesc")  &&  (typeid (*e) == typeid (XmlElementFileDesc)))
      {
        fileDesc = dynamic_cast<XmlElementFileDescPtr> (e)->Value ();
      }

      else if  (varName.EqualIgnoreCase ("HeaderFields")  &&  (typeid (*e) == typeid (XmlElementKeyValuePairs)))
      {
        XmlElementKeyValuePairsPtr hf = dynamic_cast<XmlElementKeyValuePairsPtr> (e);
        if  (!hf)
          continue;

        for  (auto  idx: *hf->Value ())
        {
          if  (idx.first.EqualIgnoreCase ("RootFileName"))
            rootFileName = idx.second;

          else if  (idx.first.EqualIgnoreCase ("Time"))
            timeSaved = DateTime (idx.second);

          else if  (idx.first.EqualIgnoreCase ("NumOfModels"))
            numOfModels = idx.second.ToInt32 ();

          else if  (idx.first.EqualIgnoreCase ("SelectedFeatures"))
          {
            delete  selectedFeatures;
            bool  validFeatures = false;
            selectedFeatures  = new FeatureNumList (idx.second, validFeatures);
          }

          else if  (idx.first.EqualIgnoreCase ("TrainingTime"))
            trainingTime = idx.second.ToDouble ();

          else if  (idx.first.EqualIgnoreCase ("Assignments")  ||  idx.first.EqualIgnoreCase ("ClassAssignments"))
            assignments.ParseToString (idx.second, log);

          else
          {
            log.Level (-1) << endl
              << "SVMModel::ReadXML   ***ERROR***   Unrecognized Header Field: " << idx.first << endl
              << endl;
            errorsFound = true;
          }
        }

        if  (numOfModels < 1)
        {
          log.Level (-1) << endl
            << "SVMModel::ReadXML   ***ERROR***   numOfModels: " << numOfModels << " Is invalid." << endl
            << endl;
          errorsFound = true;
        }
        else
        {
          DeleteModels ();
          AllocateModels ();
          binaryParameters       = new BinaryClassParmsPtr [numOfModels];
          binaryFeatureEncoders  = new FeatureEncoderPtr   [numOfModels];

          for  (kkuint32 x = 0;  x < numOfModels;  x++)
          {
            binaryParameters       [x] = NULL;
            binaryFeatureEncoders  [x] = NULL;
          }
        }
      }

      else if  (varName.EqualIgnoreCase ("SvmParam")  &&  (typeid (*e) == typeid (XmlElementSVMparam)))
      {
        XmlElementSVMparamPtr xmlSvmParam = dynamic_cast<XmlElementSVMparamPtr> (e);
        if  (xmlSvmParam)
        {
          delete  svmParam;
          svmParam = xmlSvmParam->TakeOwnership ();
        }
      }

      else if  (varName.EqualIgnoreCase ("RootFileName"))
      {
        rootFileName = e->ToKKStr ();
      }

      else if  (varName.EqualIgnoreCase ("OneVsOneModel")  &&  (typeid (*e) == typeid (XmlElementSvmModel233)))
      {
        XmlElementSvmModel233Ptr xmlElementModel = dynamic_cast<XmlElementSvmModel233Ptr> (e);
        SvmModel233 const * m = xmlElementModel->Value ();
        if  (m)
        {
          if  (!m->valid)
          {
            log.Level (-1) << endl
              << "SVMModel::ReadXML   ***ERROR***   'OneVsOneModel'  is invalid." << endl
              << endl;
          }
          else
          {
            if  (!models)
            {
              log.Level (-1) << endl
                << "SVMModel::ReadXML   ***ERROR***   'OneVsOneModel'   models was not defined/allocated." << endl
                << endl;
            }
            else
            {
              models[0][0] = xmlElementModel->TakeOwnership ();
              ++numModeLoaded;
            }
          }
        }
      }

      else if  (varName.EqualIgnoreCase ("BinaryCombo")  &&  (typeid (*e) == typeid (XmlElementKKStr)))
      {
        KKStr binaryComboStr = *(dynamic_cast<XmlElementKKStrPtr> (e)->Value ());
        lastModelIdx = binaryComboStr.ExtractTokenUint ("\t");
        lastBinaryClass1Name = binaryComboStr.ExtractToken2 ("\t");
        lastBinaryClass2Name = binaryComboStr.ExtractToken2 ("\t");
      }

      else if  (varName.StartsWith ("BinaryComboModel_")  &&  (typeid (*e) == typeid (XmlElementSvmModel233)))
      {
        XmlElementSvmModel233Ptr xmlElementModel = dynamic_cast<XmlElementSvmModel233Ptr> (e);
        SvmModel233 const * m = xmlElementModel->Value ();
        if  ((!m)  ||  (!m->valid))
        {
          log.Level (-1) << endl
            << "SVMModel::ReadXML   ***ERROR***  SvmModel233[" << varName << "] is invalid." << endl
            << endl;
          errorsFound = true;
        }

        else if  (numModeLoaded >= numOfModels)
        {
          log.Level (-1) << endl
            << "SVMModel::ReadXML   ***ERROR***   Number of models being loaded exceeds what was expected." << endl
            << endl;
          errorsFound = true;
        }

        else
        {
          MLClassPtr  class1 = MLClass::CreateNewMLClass (lastBinaryClass1Name);
          MLClassPtr  class2 = MLClass::CreateNewMLClass (lastBinaryClass2Name);

          log.Level (10) << "SVMModel::ReadXML     Class1[" << lastBinaryClass1Name << "]  Class2[" << lastBinaryClass2Name << "]" << endl;

          BinaryClassParmsPtr  binClassParms = svmParam->GetParamtersToUseFor2ClassCombo (class1, class2);
          if  (!binClassParms)
          {
            log.Level (-1) << endl
              << "SVMModel::ReadXM    ***ERROR***   Binary Class Parms are missing for classes " << lastBinaryClass1Name << " and " << lastBinaryClass2Name << endl
              << endl;
            errorsFound = true;
          }
          else
          {
            models[numModeLoaded][0] = dynamic_cast<XmlElementSvmModel233Ptr> (e)->TakeOwnership ();
            binaryParameters[numModeLoaded] = binClassParms;

            binaryFeatureEncoders[numModeLoaded] 
                = new FeatureEncoder (fileDesc,
                                      binClassParms->Class1 (),
                                      binClassParms->Class2 (),
                                      *(binClassParms->SelectedFeatures ()),
                                      svmParam->EncodingMethod (),
                                      binClassParms->C ()
                                     );

            if  (lastModelIdx != numModeLoaded)
            {
              log.Level(50) << "SVMModel::ReadXML    ***WARNING***   Index from xml file[" << lastModelIdx << "] "
            		          << "not matched numModeLoaded[" << numModeLoaded << "]."
							       << endl;
            }

            ++numModeLoaded;
          }
        }
      }
    }
  }
  delete  t;
  t = NULL;

  if  (cancelReadXML)
    errorsFound = true;

  if  (assignments.size () < 2)
  {
   log.Level (-1) << endl 
     << "SVMModel::ReadXML   ***ERROR***   'assignments is not properly defined." << endl
     << endl;
    errorsFound = true;
  }

  if  (!fileDesc)
  {
   log.Level (-1) << endl 
     << "SVMModel::ReadXML   ***ERROR***   'fileDesc' is not defined." << endl
     << endl;
    errorsFound = true;
  }

  if  (!svmParam)
  {
   log.Level (-1) << endl 
     << "SVMModel::ReadXML   ***ERROR***   'svmParam' is not defined." << endl
     << endl;
    errorsFound = true;
  }

  if  (!errorsFound)
  {
    type_table        = fileDesc->CreateAttributeTypeTable ( );
    cardinality_table = fileDesc->CreateCardinalityTable ( );

    numOfClasses = (kkint32)assignments.size ();
    BuildClassIdxTable ();
    BuildCrossClassProbTable ();

    CalculatePredictXSpaceNeeded (log);

    if  ((svmParam->MachineType () ==  SVM_MachineType::OneVsOne)  ||  (svmParam->MachineType () == SVM_MachineType::OneVsAll))
    {
      delete  featureEncoder;
      featureEncoder = new FeatureEncoder (fileDesc,
                                           NULL,              // class1, set to NULL because we are dealing with all classes
                                           NULL,              // class2,     ""          ""            ""            ""
                                           *selectedFeatures,
                                           svmParam->EncodingMethod (),
                                           svmParam->C_Param ()
                                          );
    }
  }

  validModel = !errorsFound;
}  /* ReadXML */


XmlFactoryMacro(SVMModel)
