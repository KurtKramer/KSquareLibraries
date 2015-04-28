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


#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
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



SVMModel::SVMModel (const KKStr&   _rootFileName,   // Create from existing Model on Disk.
                    bool&          _successful,
                    FileDescPtr    _fileDesc,
                    RunLog&        _log,
                    VolConstBool&  _cancelFlag
                   )
:  
  assignments              (_log),
  binaryFeatureEncoders    (NULL),
  binaryParameters         (NULL),
  cardinality_table        (),
  cancelFlag               (_cancelFlag),
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
  rootFileName             (_rootFileName),
  selectedFeatures         (NULL),
  svmParam                 (),
  trainingTime             (0.0),
  type_table               (),
  validModel               (true),
  votes                    (NULL),
  xSpaces                  (NULL),
  xSpacesTotalAllocated    (0)

{
  _log.Level (20) << "SVMModel::SVMModel - Construction from existing Model File[" 
                  << _rootFileName  << "]."
                  << endl;

  _successful = true;
  validModel = true;

  selectedFeatures = new FeatureNumList (fileDesc);

  type_table        = fileDesc->CreateAttributeTypeTable ( );
  cardinality_table = fileDesc->CreateCardinalityTable ( );

  ifstream  in (_rootFileName.Str ());
  if  (!in.is_open ())
  {
    validModel = false;
    _log.Level (-1) << endl << endl << "SVMModel::SVMModel   ***ERROR*** Could not open file[" << _rootFileName << "]" << endl << endl;
  }
  else
  {
    Read (in, _log);
  }

  if  ((!validModel)  ||   cancelFlag)
  {
    _successful = false;
  }

  numOfClasses = (kkint32)assignments.size ();

  BuildClassIdxTable ();
  BuildCrossClassProbTable ();
}


SVMModel::SVMModel (istream&       _in,   // Create from existing Model on Disk.
                    bool&          _successful,
                    FileDescPtr    _fileDesc,
                    RunLog&        _log,
                    VolConstBool&  _cancelFlag
                   )
:  
  assignments              (_log),
  binaryFeatureEncoders    (NULL),
  binaryParameters         (NULL),
  cancelFlag               (_cancelFlag),
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
  svmParam                 (),
  trainingTime             (0.0),
  type_table               (),
  validModel               (true),
  votes                    (NULL),
  xSpaces                  (NULL),
  xSpacesTotalAllocated    (0)
{
  _log.Level (20) << "SVMModel::SVMModel - Construction from File Stream." << endl;

  _successful = true;

  selectedFeatures = new FeatureNumList (fileDesc);


  type_table        = fileDesc->CreateAttributeTypeTable ( );
  cardinality_table = fileDesc->CreateCardinalityTable ( );

  Read (_in, _log);

  if  ((!validModel)  ||   cancelFlag)
  {
    _successful = false;
  }
  else
  {
    numOfClasses = (kkint32)assignments.size ();
    BuildClassIdxTable ();
    BuildCrossClassProbTable ();
  }
}


SVMModel::SVMModel (SVMparam&           _svmParam,      // Create new model from
                    FeatureVectorList&  _examples,      // Training data.
                    ClassAssignments&   _assignmnets,
                    FileDescPtr         _fileDesc,
                    RunLog&             _log,
                    VolConstBool&       _cancelFlag
                   )
:
  assignments              (_assignmnets),
  binaryFeatureEncoders    (NULL),
  binaryParameters         (NULL),
  cancelFlag               (_cancelFlag),
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
  svmParam                 (_svmParam),
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

  selectedFeatures = new FeatureNumList (fileDesc);

  type_table        = fileDesc->CreateAttributeTypeTable ( );
  cardinality_table = fileDesc->CreateCardinalityTable ( );
  
  struct svm_problem  prob;
  //  memset(&prob, 0, sizeof(svm_problem));

  numOfClasses = (kkint32)assignments.size ();

  _log.Level (20) << "SVMModel::SVMModel - Constructing From Training Data." << endl;

  // figure out how many features the svm_problem struct will have
  SetSelectedFeatures (svmParam.SelectedFeatures (), _log);

  try
  {
    if  (svmParam.MachineType () == SVM_MachineType::OneVsOne)
    {
      ConstructOneVsOneModel (&_examples, prob, _log);
    }
    else if  (svmParam.MachineType () == SVM_MachineType::OneVsAll)
    {
      ConstructOneVsAllModel (&_examples, prob, _log);
    }
    else if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
    {
      ConstructBinaryCombosModel (&_examples, _log);
    }
  }
  catch  (const exception& e)
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
  kkint32  x;

  if  (models)
  {
    for  (x = 0;  x < numOfModels;  x++)
    {
      if  (models[x] != NULL)
      {
        SvmDestroyModel (models[x]);
   	    delete  [] models[x];
        models[x] = NULL;
      }
    }

    delete[]  models;
    models = NULL;
  }

  if  (xSpaces)
  {
    for  (kkint32 x = 0;  x < numOfModels;  x++)
    {
      if  (xSpaces[x] != NULL)
      {
        free (xSpaces[x]);
        xSpaces[x] = NULL;
      }
    }

    delete[]  xSpaces;
    xSpaces = NULL;
    xSpacesTotalAllocated = 0;
  }


  if  (oneVsAllClassAssignments)
  {
    for  (kkint32 x = 0;  x < numOfModels;  x++)
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
    for  (x = 0;  x < numOfModels;  x++)
    {
      delete  binaryFeatureEncoders[x];
      binaryFeatureEncoders[x] = NULL;
    }
    delete[]  binaryFeatureEncoders;
    binaryFeatureEncoders = NULL;
  }

  if  (crossClassProbTable)
  {
    for  (x = 0;  x < crossClassProbTableSize;  x++)
    {
      delete  [] crossClassProbTable[x];
      crossClassProbTable[x] = NULL;
    }
    delete[]  crossClassProbTable;
    crossClassProbTable = NULL;
  }

  delete  selectedFeatures;  selectedFeatures = NULL;

  delete  [] classIdxTable;         classIdxTable        = NULL;
  delete  [] predictXSpace;         predictXSpace        = NULL;
  delete  [] probabilities;         probabilities        = NULL;
  delete  [] votes;                 votes                = NULL;
}



kkint32  SVMModel::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (SVMModel)
       + assignments.MemoryConsumedEstimated ()
       + sizeof (kkint16) * oneVsAllAssignment.size ()
       + sizeof (kkint32) * cardinality_table.size ()
       + rootFileName.MemoryConsumedEstimated ()
       + svmParam.MemoryConsumedEstimated ()
       + sizeof (AttributeType) * type_table.size ();

  if  (binaryFeatureEncoders)
  {
    memoryConsumedEstimated += sizeof (FeatureEncoderPtr) * numOfModels;
    for  (kkint32 x = 0;  x < numOfModels;  x++)
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
    for  (kkint32 x = 0;  x < numOfModels;  ++x)
    {
      if  (models[x])
      if  (models[x][0])
       memoryConsumedEstimated += models[x][0]->MemoryConsumedEstimated ();
    }
  }

  if  (oneVsAllClassAssignments)
  {
    memoryConsumedEstimated += numOfModels * sizeof (ClassAssignmentsPtr);
    for  (kkint32 x = 0;  x < numOfModels;  ++x)
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






void  SVMModel::BuildClassIdxTable ()
{
  delete[]  classIdxTable;

  classIdxTable = new MLClassPtr[numOfClasses];
  for  (kkint32 classIdx = 0;  classIdx < numOfClasses;  classIdx++)
    classIdxTable[classIdx] = assignments.GetMLClassByIndex (classIdx);

  probabilities = new double[numOfClasses + 2]; // I am add 2 as a desperate move to deal with some kind of memory corruption  kak
  votes         = new kkint32[numOfClasses + 2];    // 
  
  for  (kkint32 x = 0;  x < numOfClasses;  x++)
  {
    probabilities[x] = 0.0;
    votes[x]         = 0;
  }

}  /* BuildClassIdxTable */



void  SVMModel::BuildProblemOneVsAll (FeatureVectorList&    examples,
                                      struct svm_problem&   prob,
                                      XSpacePtr&            xSpace,
                                      const MLClassList&    classesThisAssignment,
                                      FeatureEncoderPtr     featureEncoder,
                                      MLClassList&          allClasses,
                                      ClassAssignmentsPtr&  classAssignments,
                                      RunLog&               log
                                     )
{ 
  log.Level (20) << "SVMModel::BuildProblemOneVsAll" << endl;

  if  (!selectedFeatures)
    selectedFeatures = new FeatureNumList (examples.FileDesc ());

  kkint32  numOfFeaturesSelected = selectedFeatures->NumOfFeatures ();

  MLClassPtr     mlClass = NULL;
  delete  classAssignments;
  classAssignments = new ClassAssignments (log);

  MLClassList::const_iterator  idx;

  for  (idx = allClasses.begin ();  idx != allClasses.end ();  idx++)
  {
    mlClass = *idx;
    if  (classesThisAssignment.PtrToIdx (mlClass) >= 0)
    {
      // We are looking at a class that is to be treated logically as the 'one' class in 'One-vs-All'
      classAssignments->AddMLClass (mlClass, 0);
    }
    else
    {
      // We are looking at a class that is to be treated logically as one of the 'all' classes in 'One-vs-All'
      classAssignments->AddMLClass (mlClass, 1);
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

  if  (svmParam.Param ().gamma == 0)
    svmParam.Gamma_Param (1.0 / numOfFeaturesSelected);

  return;
}  /* BuildProblemOneVsAll */



void  SVMModel::BuildProblemBinaryCombos (FeatureVectorListPtr  class1Examples, 
                                          FeatureVectorListPtr  class2Examples, 
                                          SVMparam&             _svmParam,
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

  ClassAssignments  binaryAssignments (log);
  binaryAssignments.AddMLClass (class1, 0);
  binaryAssignments.AddMLClass (class2, 1);

  _twoClassParms = _svmParam.GetParamtersToUseFor2ClassCombo (class1, class2);

  FeatureVectorListPtr  twoClassExamples 
          = new FeatureVectorList (fileDesc, 
                                   false
                                  );
  twoClassExamples->AddQueue (*class1Examples);
  twoClassExamples->AddQueue (*class2Examples);

  _encoder = new FeatureEncoder (_svmParam, 
                                 fileDesc,
                                 type_table,
                                 cardinality_table,
                                 class1,
                                 class2
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
  kkint32  x, y;

  if  (crossClassProbTable)
  {
    for  (x = 0;  x < crossClassProbTableSize;  x++)
      delete  crossClassProbTable[x];
    delete[]  crossClassProbTable;
    crossClassProbTable = NULL;
  }

  crossClassProbTable = NULL;

  if  (numOfClasses > 0)
  {
    crossClassProbTableSize = numOfClasses;
    crossClassProbTable = new double*[crossClassProbTableSize + 2];   // I am adding 2 to deal with a memory corruption problem.   kak
    for  (x = 0;  x < crossClassProbTableSize;  x++)
    {
      crossClassProbTable[x] = new double[crossClassProbTableSize + 2];
      for  (y = 0;  y < crossClassProbTableSize;  y++)
        crossClassProbTable[x][y] = 0.0;
    }
  }
}  /* BuildCrossClassProbTable */




FeatureNumListConstPtr  SVMModel::GetFeatureNums ()  const
{
  return  svmParam.GetFeatureNums ();
}



FeatureNumListConstPtr  SVMModel::GetFeatureNums (FileDescPtr  fileDesc)  const
{
  return  svmParam.GetFeatureNums (fileDesc);
}



FeatureNumListConstPtr  SVMModel::GetFeatureNums (FileDescPtr  fileDesc,
                                                  MLClassPtr   class1,
                                                  MLClassPtr   class2
                                                 )  const
{
  return  svmParam.GetFeatureNums (fileDesc, class1, class2);
}  /* GetFeatureNums */



void  SVMModel::Save (const KKStr&  _rootFileName,
                      bool&         _successful,
                      RunLog&       _log
                     )
{
  rootFileName = _rootFileName;

  KKStr  svmModelFileName = rootFileName + ".SVMModel";

  ofstream  o (svmModelFileName.Str ());

  Write (o, svmModelFileName, _successful, _log);
  o.close ();
}  /* Save */




void  SVMModel::Write (ostream&      o,
                       const KKStr&  rootFileName,
                       bool&         _successful,
                       RunLog&       log
                      )
{
  o << "<SVMModel>" << endl;

  o << "<Header>" << endl;

  o << "RootFileName"       << "\t" << rootFileName                                     << endl;
  o << "Time"               << "\t" << osGetLocalDateTime ()                            << endl;
  o << "NumOfModels"        << "\t" << numOfModels                                      << endl;
  if  (selectedFeatures)
    o << "SelectedFeatures"   << "\t" << selectedFeatures->ToString ()                     << endl;
  o << "TrainingTime"       << "\t" << trainingTime                                     << endl;
  o << "MachineType"        << "\t" << MachineTypeToStr (svmParam.MachineType ())       << endl;
  o << "ClassAssignments"   << "\t" << assignments.ToString ().QuotedStr ()             << endl;

  svmParam.WriteXML (o);

  o << "</Header>" << endl;

  if  (svmParam.MachineType () == SVM_MachineType::OneVsOne)
  {
    o << "<OneVsOne>" << endl;

    KKStr  modelFileName = rootFileName + ".model";

    o << "ModelFileName" << "\t" << modelFileName.QuotedStr () << endl;

    SvmSaveModel (o, models[0]);

    o << "</OneVsOne>" << endl;
  }

  else if  (svmParam.MachineType () == SVM_MachineType::OneVsAll)
  {
    o << "<OneVsAll>" << endl;

    // In the case of One to All there is a one-to-one relationship between 
    // assignments[] and models[].  That is for each possible assignment there is
    // going to be a Support vector Machine.  assignments[i] is a pointer to the 
    // ClassAssignments structure that is used for the SVM that models[i] points 
    // to.
    for  (kkuint32  assignmentIDX = 0;  assignmentIDX < oneVsAllAssignment.size ();  assignmentIDX++)
    {
      o << "<OneVsAllEntry>" << endl;

      kkint32  assignmentNum = oneVsAllAssignment[assignmentIDX];   // The assignment as specified in the config file.

      // There can be more than one class with the same assignment, but we will use the 
      // one that 'GetMLClassByIndex' returns.
      MLClassPtr  classForOneSide = assignments.GetMLClassByIndex (assignmentNum);

      KKStr  modelFileName = rootFileName + "_" + classForOneSide->Name ();

      ClassAssignmentsPtr  oneVsAllAssignment = oneVsAllClassAssignments[assignmentIDX];

      o << "AssignmentNum"      << "\t" << assignmentNum                                << endl;
      o << "OneSideClassName"   << "\t" << classForOneSide->Name ().QuotedStr ()        << endl;  
      o << "ModelFileName"      << "\t" << modelFileName.QuotedStr ()                   << endl;
      o << "ClassAssignments"   << "\t" << oneVsAllAssignment->ToString ().QuotedStr () << endl;

      SvmSaveModel (models[assignmentIDX], modelFileName.Str (), _successful);
      if  (!_successful)
      {
        log.Level (-1) << "SVMModel::Save  Error saving Model into File["
                       << modelFileName << "]."
                       << endl;
        _successful = false;
        return;
      }

      o << "</OneVsAllEntry>" << endl;
    }

    o << "</OneVsAll>" << endl;
  }

  else if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
  {
    o << "<BinaryCombos>" << endl;

    kkint32  modelsIDX = 0;
    // With BinaryCombos, n = numberOfAssignmnets = assignments.size (), the number of binary combos is equal to 'n(n-1)/2'.

    for  (modelsIDX = 0;  modelsIDX < numOfModels;  modelsIDX++)
    {
      BinaryClassParmsPtr  binClassParms = binaryParameters[modelsIDX];
      o << "Classes" << "\t" << binClassParms->Class1Name () << "\t"  << binClassParms->Class2Name () << endl;
      SvmSaveModel (o, models[modelsIDX]);
    }

    o << "</BinaryCombos>" << endl;
  }

  o << "</SVMModel>" << endl;

  _successful = true;

} /* Write */





void  SVMModel::Read (istream&  i,
                      RunLog&   log
                     )
{
  validModel = true;

  ReadHeader (i, log);

  if  (svmParam.MachineType () == SVM_MachineType::OneVsOne)
  {
    ReadOneVsOne (i, log);
  }
  else if  (svmParam.MachineType () == SVM_MachineType::OneVsAll)
  {
    ReadOneVsAll (i, log);
  }
  else if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
  {
    ReadBinaryCombos (i, log);
  }

  if  (!cancelFlag)
  {
    bool  sectionFound;
    ReadSkipToSection (i, "</SVMModel>", sectionFound, log);
  }
}  /* Read */



void  SVMModel::ReadSkipToSection (istream&  i, 
                                   KKStr     sectName,
                                   bool&     sectionFound,
                                   RunLog&   log
                                  )
{
  sectionFound = false;
  char  buff[20480];
  sectName.Upper ();

  KKStr  field;

  // Skip to start of OneVsOne section
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", 
                                 true      // true = decode escape characters
                                );
    field.Upper ();
    if  (field == sectName)
    {
      sectionFound = true;
      break;
    }
  }

  if  (!sectionFound)
  {
    log.Level (-1) << endl << endl
                   << "SVMModel::ReadSkipToSection    *** Could not find section[" << sectName << "]" <<endl
                   << endl;
  }
}  /* ReadSkipToSection */




void  SVMModel::ReadHeader (istream&  i,
                            RunLog&   log
                           )
{
  bool  sectionFound = false;
  ReadSkipToSection (i, "<Header>", sectionFound, log);
  if  (!sectionFound)
  {
    validModel = false;
    return;
  }

  char  buff[20480];

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters

    field.Upper ();

    if  (field.EqualIgnoreCase ("</HEADER>"))
    {
      break;
    }

    if  (field == "TIME")
    {
      cout << "SVMModel::ReadHeader    Time[" << ln << "]" << endl;
    }

    else if  (field.EqualIgnoreCase ("NUMOFMODELS"))
      numOfModels = ln.ExtractTokenInt ("\n\t\r");

    else if  (field == "SELECTEDFEATURES")
    {
      KKStr  selectedFeaturesStr = ln.ExtractQuotedStr ("\n\r\t", true);
      FeatureNumListConstPtr  tempFeatures = FeatureNumList::ExtractFeatureNumsFromStr (selectedFeaturesStr);
      SetSelectedFeatures (tempFeatures, log);
      delete  tempFeatures;
      tempFeatures = NULL;
    }

    else if  (field == "TRAININGTIME")
      trainingTime = ln.ExtractTokenDouble ("\n\r\t");

    else if  (field == "CLASSASSIGNMENTS")
      assignments.ParseToString (ln.ExtractQuotedStr ("\n\r\t", true));

    else if  (field == "<SVMPARAM>")
      svmParam.ReadXML (i, fileDesc, log);
  }
}  /* ReadHeader */




void  SVMModel::ReadOneVsOne (istream&  i,
                              RunLog&   log
                             )
{
  bool  sectionFound = false;
  ReadSkipToSection (i, "<OneVsOne>", sectionFound, log);
  if  (!sectionFound)
  {
    validModel = false;
    return;
  }

  delete  featureEncoder;
  featureEncoder = new FeatureEncoder (svmParam, 
                                       fileDesc,
                                       type_table,
                                       cardinality_table,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL               // class2,     ""          ""            ""            ""
                                      );

  char  buff[20480];
  KKStr  field;

  KKStr  modelFileName;

  while  ((i.getline (buff, sizeof (buff)))  &&  validModel)
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</ONEVSONE>")
    {
      break;
    }

    else if  (field == "MODELFILENAME")
    {
      modelFileName = ln.ExtractQuotedStr ("\n\r\t", true);
      numOfModels = 1;
      delete  models;
      models = new ModelPtr[numOfModels];
      models[0] = SvmLoadModel (i, log);
      if  (!models[0]) 
      {
        log.Level (-1) << endl << endl 
                       << "SVMModel::ReadOnevsOne - Error Loading Model from File["
                       << modelFileName << "]." 
                       << endl;
        validModel = false;
      }
    }
  }
 
  if  (validModel  &&  modelFileName.Empty ())
  {
     log.Level (-1) << endl << endl 
                    << "SVMModel::ReadOnevsOne - Model File Name was not specified." << endl
                    << modelFileName << "]." 
                    << endl;
     validModel = false;
  }
}  /* ReadOneVsOne */




void  SVMModel::ReadOneVsAll (istream&  i,
                              RunLog&   log
                             )
{
  bool  sectionFound = false;
  ReadSkipToSection (i, "<OneVsAll>", sectionFound, log);
  if  (!sectionFound)
  {
    validModel = false;
    return;
  }

  delete  featureEncoder;
  featureEncoder = new FeatureEncoder (svmParam, 
                                       fileDesc,
                                       type_table,
                                       cardinality_table,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL               // class2,     ""          ""            ""            ""
                                      );

  kkint32  modelIDX = 0;

  // allocate and initialize the model objects we'll need
  models = new ModelPtr[numOfModels];
  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
    models[modelIDX] = NULL;


  oneVsAllAssignment.clear ();
  while  (oneVsAllAssignment.size () < kkuint32 (numOfModels))
    oneVsAllAssignment.push_back (-1);

  {
    delete  oneVsAllClassAssignments;
    oneVsAllClassAssignments = new ClassAssignmentsPtr[numOfModels];
    for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
      oneVsAllClassAssignments[modelIDX] = NULL;
  }


  char  buff[10240];
  KKStr  field;

  while  ((i.getline (buff, sizeof (buff)))  &&  validModel)
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</ONEVSALL>")
      break;

    if  (field == "<ONEVSALLENTRY>")
    {
      ReadOneVsAllEntry (i, modelIDX, log);
    }
  }

}  /* ReadOneVsAll */




void  SVMModel::ReadOneVsAllEntry (istream&  i,
                                   kkint32   modelIDX,
                                   RunLog&   log
                                  )
{
  // If we are here the header to this sub-section has already been read.

  char  buff[20480];
  KKStr  field;

  kkint32              assignmentNum   = -1;
  MLClassPtr           classForOneSide = NULL;
  KKStr                modelFileName;
  ClassAssignmentsPtr  assignment      = NULL;

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    KKStr  value = ln.ExtractQuotedStr ("\n\r\t", true);
  
    if  (field == "</ONEVSALLENTRY>")
      break;

    if  (field == "ASSIGNMENTNUM")
    {
      assignmentNum = atoi (value.Str ());
      oneVsAllAssignment[modelIDX] = assignmentNum;
    }

    else if  (field == "ONESIDECLASSNAME")
      classForOneSide = MLClass::CreateNewMLClass (value);

    else if  (field == "ModelFileName")
      modelFileName = value;

    else if  (field == "CLASSASSIGNMENTS")
    {
      assignment = new ClassAssignments (log);
      assignment->ParseToString(value);
      oneVsAllClassAssignments[modelIDX] = assignment;
    }
  }

  if  (modelFileName.Empty ())
  {
    validModel = false;
    return;
  }

  if  (!assignment)
  {
    validModel = false;
    return;
  }

  if  (!classForOneSide)
  {
    validModel = false;
    return;
  }

  models[modelIDX] = SvmLoadModel (modelFileName.Str ());
  if  (!models[modelIDX])
  {
    validModel = false;
    return;
  }
}  /* ReadOneVsAllEntry */




void  SVMModel::ReadBinaryCombos (istream&  i,
                                  RunLog&   log
                                 )
{
  log.Level (20) << "SVMModel::ReadBinaryCombos" << endl;

  char  buff[40960];
  kkint32 modelsIDX = 0;

  delete  binaryParameters;
  binaryParameters       = new BinaryClassParmsPtr [numOfModels];
  models                 = new ModelPtr            [numOfModels];
  binaryFeatureEncoders  = new FeatureEncoderPtr   [numOfModels];

  for  (kkint32 x = 0;  x < numOfModels;  x++)
  {
    models                 [x] = NULL;
    binaryParameters       [x] = NULL;
    binaryFeatureEncoders  [x] = NULL;
  }

  KKStr  class1Name; 
  KKStr  class2Name;

  MLClassPtr  class1 = NULL;
  MLClassPtr  class2 = NULL;

  bool  endOfBinaryCombosRead = false;

  while  ((i.getline (buff, sizeof (buff)))  &&  (!cancelFlag)  &&  validModel)
  {
    KKStr  line (buff);
    line.TrimLeft ();
    line.TrimRight ();

    if  (line == "<BinaryCombos>")
      continue;

    if  (line == "</BinaryCombos>")
    {
      endOfBinaryCombosRead = true;
      break;
    }
    
    KKStr  lineName = line.ExtractToken2 ("\t");
    lineName.Upper ();

    if  (lineName == "CLASSES")  
    {
      if  (modelsIDX >= numOfModels)
      {
        log.Level (-1) << endl << endl << endl 
                       << "SVMModel::ReadBinaryCombos   ***ERROR***     Number of models defined exceeds what was expected." << endl
                       << endl;
        validModel = false;
        return;
      }

      class1Name = line.ExtractToken2 ("\t");
      class2Name = line.ExtractToken2 ("\t");

      class1 = MLClass::CreateNewMLClass (class1Name);
      class2 = MLClass::CreateNewMLClass (class2Name);

      log.Level (10) << "SVMModel::ReadBinaryCombos    Class1[" << class1Name << "]  Class2[" << class2Name << "]" << endl;

      BinaryClassParmsPtr  binClassParms = svmParam.GetParamtersToUseFor2ClassCombo (class1, class2);
      if  (binClassParms == NULL)
      {
        validModel = false;
        return;
      }

      binaryParameters[modelsIDX] = binClassParms;

      models[modelsIDX] = SvmLoadModel (i, log);   
      if  (models[modelsIDX] == NULL)
      {
        log.Level (-1) << endl << endl << endl 
                       << "SVMModel::ReadBinaryCombos     ***ERROR***     Unable to load model." << endl
                       << endl
                       << "Class1[" << binClassParms->Class1Name () << "]  Class2[" << binClassParms->Class2Name () << "]" << endl
                       << endl;
        validModel = false;
      }
      else
      {
        binaryFeatureEncoders[modelsIDX] 
              = new FeatureEncoder (svmParam, 
                                    fileDesc,
                                    type_table,
                                    cardinality_table,
                                    binClassParms->Class1 (),
                                    binClassParms->Class2 ()
                                   );
      }

      modelsIDX++;
    }
  }

  if  (!endOfBinaryCombosRead)
  {
    // We need to ski[p to end of Binary Section.
    while  ((i.getline (buff, sizeof (buff)))  &&  (!cancelFlag)  &&  validModel)
    {
      KKStr  line (buff);
      line.TrimRight ();
      line.TrimLeft ();
      if  (line.EqualIgnoreCase ("</BinaryCombos>"))
        break;
    }
  }


  if  ((modelsIDX < numOfModels)  &&  (!cancelFlag))
  {
    log.Level (-1) << endl << endl << endl 
                   << "SVMModel::ReadBinaryCombos     **** ERROR ****     Less BinaryComboParms were specified than expected." << endl
                   << endl;
    validModel = false;
  }
}  /* ReadBinaryCombos */




double   SVMModel::DistanceFromDecisionBoundary (FeatureVectorPtr  example,
                                                 MLClassPtr        class1,
                                                 MLClassPtr        class2
                                                )
{
  if  (svmParam.MachineType () != SVM_MachineType::BinaryCombos)
  {
    cerr << endl << "SVMModel::DistanceFromDecisionBoundary   ***ERROR***    This method only works with BinaryCombos." << endl << endl;
    return  0.0;
  }

  kkint32  modelIDX = 0;
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
  if  (svmParam.ProbClassPairs ().size () < 1)
  {
    svmParam.ProbClassPairsInitialize (assignments);
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

  if  (svmParam.MachineType () == SVM_MachineType::OneVsAll)
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

  else if  (svmParam.MachineType () == SVM_MachineType::OneVsOne)
  {  
    EncodeExample (example, predictXSpace);

    kkint32  knownClassNum  = -1;
    kkint32  prediction     = -1;
    kkint32  prediction2    = -1;

    if  (knownClass)
      knownClassNum = assignments.GetNumForClass (knownClass);

    vector<kkint32>  winners;

    SvmPredictClass (svmParam,
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

    predClass1 = assignments.GetMLClass (prediction);
    predClass2 = assignments.GetMLClass (prediction2);
    //free (xSpace);
  }

  
  else if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
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

  double*  probabilities = new double [numOfModels + 2];  // I am adding 2 as a desperate measure to deal with a memory corruption problem   kak

  double  largestLosingProbability    = FLT_MIN;
  kkint32 largestLosingProbabilityIDX = -1;

  double  secondLargestLosingProbability    = FLT_MIN;
  kkint32 secondLargestLosingProbabilityIDX = -1;

  double  largestWinningProbability    = FLT_MIN;
  kkint32 largestWinningProbabilityIDX = -1;

  double  secondLargestWinningProbability    = FLT_MIN;
  kkint32 secondLargestWinningProbabilityIDX = -1;

  kkint32 knownAssignmentIDX = -1;

  kkuint32  assignmentIDX;

  for  (assignmentIDX = 0;  assignmentIDX < oneVsAllAssignment.size ();  ++assignmentIDX)
  {
    kkint16  assignmentNum = oneVsAllAssignment[assignmentIDX];

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

    SvmPredictClass (svmParam,
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
      probabilities[assignmentIDX] = predictedClassProbability;
      if  (predictedClassProbability > largestWinningProbability)
      {
        secondLargestWinningProbability    = largestWinningProbability;
        secondLargestWinningProbabilityIDX = largestWinningProbabilityIDX;

        largestWinningProbability = predictedClassProbability;
        largestWinningProbabilityIDX = assignmentIDX;
      }
      else if  (predictedClassProbability > secondLargestWinningProbability)
      {
        secondLargestWinningProbability    = predictedClassProbability;
        secondLargestWinningProbabilityIDX = assignmentIDX;
      }
    }
    else
    {
      probabilities[assignmentIDX] = 1.0 - predictedClassProbability;
      if  (probabilities[assignmentIDX] > largestLosingProbability)
      {
        secondLargestLosingProbabilityIDX = largestLosingProbabilityIDX;
        secondLargestLosingProbability    = largestLosingProbability;

        largestLosingProbabilityIDX = assignmentIDX;
        largestLosingProbability    = probabilities[assignmentIDX];
      }
      else if  (probabilities[assignmentIDX] > secondLargestLosingProbability)
      {
        secondLargestLosingProbabilityIDX = assignmentIDX;
        secondLargestLosingProbability    = probabilities[assignmentIDX];
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

  predClass1Prob   = (probabilities [oneVsAllAssignment[assignmentIDXthatWon]]);
  probOfKnownClass = (probabilities [oneVsAllAssignment[knownAssignmentIDX]]);

  delete[]  probabilities;      
  probabilities = NULL;

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
  kkint32   classIDX;

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

  for  (kkint32 x = 0; x < numOfClasses;  x++)
  {
    votes[x] = 0;
    probabilities[x] = 1.0f;
  }

  for  (kkint32  class1IDX = 0;  class1IDX < (numOfClasses - 1);  class1IDX++)
  {
    MLClassPtr  class1  = classIdxTable [class1IDX];

    if  (class1 == knownClass)
      knownClassIDX = class1IDX;
     
    for  (kkint32  class2IDX = (class1IDX + 1);  class2IDX < numOfClasses;  class2IDX++)
    {
      MLClassPtr           class2 = classIdxTable [class2IDX];
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
      double  margin   = 0.0;

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
  for  (classIDX = 0;  classIDX < numOfClasses;  classIDX++)
    totProbability += probabilities[classIDX];

  if  (totProbability <= 0.0)
    totProbability = 1.0;

  for  (classIDX = 0;  classIDX < numOfClasses;  classIDX++)
    probabilities[classIDX] = probabilities[classIDX] / totProbability;

  GreaterVotes (svmParam.SelectionMethod () == SVM_SelectionMethod::SelectByProbability,
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

  if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
  {
    vector<KKStr> svNames = SupportVectorNames ();
    numOfSupportVectors = (kkint32)svNames.size ();
  }
  else
  {
    for  (kkint32 x = 0;  x < numOfModels;  x++)
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

  if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
  {
    numSVs = NumOfSupportVectors ();
    for  (kkint32 modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
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

  if  (svmParam.MachineType () == SVM_MachineType::BinaryCombos)
  {
    PredictProbabilitiesByBinaryCombos (example, _mlClasses, _votes, _probabilities, _log);
    return;
  }

  kkint32  predClass1Votes         = -1;
  kkint32  predClass2Votes         = -1;
  double   predClass1Prob          = 0.0;
  double   predClass2Prob          = 0.0;
  double   probOfKnownClass        = 0.0;
  double   smallestDistOfPredClass = 0.0;

  double   breakTie                = 0.0f;

  MLClassPtr  predictedClass  = NULL;
  XSpacePtr   xSpace          = NULL;
  kkint32     x, y;

  xSpace = featureEncoder->EncodeAExample (example);

  kkint32  knownClassNum = 0;
  kkint32  prediction    = 0;
  kkint32  prediction2   = 0;

  for  (x = 0;  x < numOfClasses;  x++)
  {
    probabilities [x] = 0.0;
    _probabilities[x] = 0.0;
    _votes        [x] = 0;
  }

  vector<kkint32>  winners;

  SvmPredictClass (svmParam,
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
  for  (x = 0;  x < numOfClasses;  x++)
  {
    predictedClass = assignments.GetMLClass (x);
    if  (predictedClass)
    {
      y = _mlClasses.PtrToIdx (predictedClass);
      if  ((y < 0)   ||   (y >= numOfClasses))
      {
        _log.Level (-1) << endl
                << "SVMModel::ProbabilitiesByClass   ***ERROR***"                                                     << endl
                << "                                 Invalid classIdx[" << y                       << "]  Specified." << endl
                << "                                 ClassName       [" << predictedClass->Name () << "]"             << endl
                << endl;
      }
      else
      {
        _votes        [y] = votes        [x];
        _probabilities[y] = probabilities[x];
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

  kkint32   classIDX;

  double  probability = -1.0;

  kkint32 modelIDX = 0;

  for  (kkint32 x = 0; x < numOfClasses;  x++)
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


  for  (kkint32  class1IDX = 0;  class1IDX < (numOfClasses - 1);  class1IDX++)
  {

    for  (kkint32  class2IDX = (class1IDX + 1);  class2IDX < numOfClasses;  class2IDX++)
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

  for  (classIDX = 0;  classIDX < numOfClasses;  classIDX++)
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
      kkint32  ourIdx = assignments.GetNumForClass (*idx);
      if  ((ourIdx < 0)  ||  (ourIdx >= numOfClasses))
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
        _votes        [callersIdx] = votes        [ourIdx];
        _probabilities[callersIdx] = probabilities[ourIdx] / totProbability;
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
  if  (svmParam.MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  kkint32  modelIDX = 0;
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
  if  (svmParam.MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  map<KKStr,KKStr>  names;
  map<KKStr,KKStr>::iterator  svnIDX;

  // Locate the binary parms in question.
  kkint32  modelIDX = 0;
  BinaryClassParmsPtr  parms = NULL;
  for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
  {
    parms = binaryParameters[modelIDX];
  
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
  if  (svmParam.MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  bool  c1RevFlag = false;
  kkint32  modelIDX = 0;

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
  if  (svmParam.MachineType () != SVM_MachineType::BinaryCombos)
    return  results;

  // Locate the binary parms in question.
  bool  c1RevFlag = false;
  kkint32  modelIDX = 0;

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

    svm_model*  subSetModel = svm_train  (subSetProb, &parameters);

    svm_predictTwoClasses (subSetModel, predictXSpace, distance, -1);
    probabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
    probabilityC1 = AdjProb (probabilityC1);
    if  (c1RevFlag)
      probabilityC1 = 1.0f - probabilityC1;
    kkint32 zed = 100;
  }

  for  (svIDX = 0;  svIDX < numSVs;  svIDX++)
  {
    svm_problem*  subSetProb = svm_BuildProbFromTwoClassModel  (models[modelIDX][0], svIDX);
    svm_parameter  parameters = models[modelIDX][0]->param;
    parameters.nr_class = 2;

    svm_model*  subSetModel = svm_train  (subSetProb, &parameters);

    svm_predictTwoClasses (subSetModel, predictXSpace, distance, -1);
    probabilityC1 = ((1.0 / (1.0 + exp (-1.0 * (parms->Param ().A) * distance))));
    probabilityC1 = AdjProb (probabilityC1);
    if  (c1RevFlag)
      probabilityC1 = 1.0f - probabilityC1;

    double  deltaProb = probabilityC1 - origProbabilityC1;

    KKStr  svName = models[modelIDX][0]->SupportVectorName (svIDX);
    candidates.push_back (ProbNamePair (svName, deltaProb));

    subSetModel->Dispose ();

    delete  subSetModel;  subSetModel = NULL;
    delete  subSetProb;   subSetProb = NULL;
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

  switch (svmParam.EncodingMethod())
  {
  case SVM_EncodingMethod::BinaryEncoding:
    for  (z = 0;  z < numOfFeaturesSelected;  z++)
    {
      if  ((type_table[(*selectedFeatures)[z]] == AttributeType::NominalAttribute)  ||
           (type_table[(*selectedFeatures)[z]] == AttributeType::SymbolicAttribute)
          )
         numFeaturesAfterEncoding += cardinality_table[(*selectedFeatures)[z]];
      else
         numFeaturesAfterEncoding ++;
    }
    break;

  case SVM_EncodingMethod::ScaledEncoding:
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
    featureEncoder = new FeatureEncoder (svmParam, 
                                         fileDesc,
                                         type_table,
                                         cardinality_table,
                                         NULL,              /**< class1, set to NULL because we are dealing with all classes */
                                         NULL               /**< class2,     ""          ""            ""            ""      */
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
    kkint32 x;
    for  (x = 0;  x < numOfModels;  x++)
    {
      models[x]  = NULL;
      xSpaces[x] = NULL;
    }
  }

  delete  featureEncoder;
  featureEncoder = new FeatureEncoder (svmParam, 
                                       fileDesc,
                                       type_table,
                                       cardinality_table,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL               // class2,     ""          ""            ""            ""
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
  models[0] = SvmTrainModel (svmParam.Param (), prob);
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

  VectorShort  assignmentNums = assignments.GetUniqueListOfAssignments ();
  numOfModels = (kkint32)assignmentNums.size ();

  models   = new ModelPtr  [numOfModels];
  xSpaces  = new XSpacePtr [numOfModels];
  {
    kkint32 x;
    for  (x = 0;  x < numOfModels;  x++)
    {
      models[x]  = NULL;
      xSpaces[x] = NULL;
    }
  }

  featureEncoder = new FeatureEncoder (svmParam, 
                                       fileDesc,
                                       type_table,
                                       cardinality_table,
                                       NULL,              // class1, set to NULL because we are dealing with all classes
                                       NULL               // class2,     ""          ""            ""            ""
                                      );

  kkint32  modelIDX = 0;
  kkint32  assignmentIDX;

  trainingTime = 0;
  oneVsAllAssignment.erase (oneVsAllAssignment.begin (), oneVsAllAssignment.end ());

  {
    kkint32  modelIDX;
    delete  oneVsAllClassAssignments;
    oneVsAllClassAssignments = new ClassAssignmentsPtr[numOfModels];
    for  (modelIDX = 0;  modelIDX < numOfModels;  modelIDX++)
      oneVsAllClassAssignments[modelIDX] = NULL;
  }

  // build the models
  for (assignmentIDX = 0;  assignmentIDX < numOfModels;  assignmentIDX++)
  {
    kkint16  assignmentNum = assignmentNums[assignmentIDX];
    oneVsAllAssignment.push_back (assignmentNum);

    MLClassList  classesThisAssignment = assignments.GetMLClasses (assignmentNum);

    BuildProblemOneVsAll (*examples, 
                          prob, 
                          xSpaces[modelIDX], 
                          classesThisAssignment, 
                          featureEncoder, 
                          *allClasses, 
                          oneVsAllClassAssignments [modelIDX],
                          log
                         );

    // train the model using the svm_problem built above

    double  trainTimeStart = osGetSystemTimeUsed ();
    models[modelIDX] = SvmTrainModel (svmParam.Param (), prob);
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

  kkint32  modelIDX = 0;
  kkint32  class1IDX;
  kkint32  class2IDX;
  kkint32  x;

  for  (x = 0;  x < numOfModels;  x++)
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
  for (class1IDX = 0;  (class1IDX < (numOfClasses - 1))  &&  (!cancelFlag);  class1IDX++)
  {
    MLClassPtr  class1 = assignments.GetMLClassByIndex (class1IDX);

    for  (class2IDX = class1IDX + 1;   (class2IDX < numOfClasses)   &&  (!cancelFlag);   class2IDX++)
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
                                svmParam,
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


      /*
      {
        {
          ofstream xxx ("c:\\Temp\\testModel.txt");
          SvmSaveModel (xxx, models[modelIDX]);
          xxx.close ();
        }

        {
          FILE*  xxx = fopen ("c:\\Temp\\testModel.txt", "r");
          svm_model **  m = SvmLoadModel (xxx);
          fclose (xxx);
        }
      }
      */

      // log.Level (10) << "Support Vectors[" << models[modelIDX][0]->l << "]" << endl;

      //osWaitForEnter ();

      // free the memory for the svm_problem
      delete  [] prob.index;  prob.index = NULL;
      free (prob.y);      prob.y     = NULL;
      free (prob.x);      prob.x     = NULL;
      delete[] prob.W;

      modelIDX++;   
    }
  }

  {
    kkint32  x;
    for  (x = 0;  x < (kkint32)assignments.size ();  x++)
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
  kkint32  x;

  FeatureVectorListPtr* examplesByClass = new FeatureVectorListPtr[numOfClasses];
  for  (x = 0;  x < numOfClasses;  x++)
    examplesByClass[x] = new FeatureVectorList (fileDesc, false);

  MLClassPtr  lastMLClass = NULL;
  kkint32     classIdx     = 0;

  FeatureVectorList::iterator  idx;

  for  (idx = examples->begin ();  idx != examples->end ();  idx++)
  {
    FeatureVectorPtr example = *idx;
    
    if  (lastMLClass != example->MLClass ())
    {
      lastMLClass = example->MLClass ();
      classIdx    = assignments.GetNumForClass (lastMLClass);
    }

    examplesByClass[classIdx]->PushOnBack (example);
  }

  return  examplesByClass;
}  /* BreakDownExamplesByClass */



bool  SVMModel::NormalizeNominalAttributes ()
{
  if  (svmParam.EncodingMethod () == SVM_EncodingMethod::NoEncoding)
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
                   << "SVMModel::RetrieveCrossProbTable    ***ERROR***" << endl
                   << "            classes.QueueSize ()[" << classes.QueueSize () << "] != crossClassProbTableSize[" << crossClassProbTableSize << "]" << endl
                   << endl;
    return;
  }

  kkint32*  indexTable = new kkint32[classes.QueueSize ()];
  kkint32  x, y;
  for  (x = 0;  x < classes.QueueSize ();  x++)
  {
    for  (y = 0;  y < classes.QueueSize ();  y++)
       crossProbTable[x][y] = 0.0;

    indexTable[x] = assignments.GetNumForClass (classes.IdxToPtr (x));
    if  (indexTable[x] < 0)
    {
      log.Level (-1) << endl << endl
                     << "SVMModel::RetrieveCrossProbTable      ***WARNING***" << endl
                     << endl
                     << "      Class Index[" << x << "]  Name[" << classes[x].Name () << "]" << endl
                     << "      will populate this index with zeros."                         << endl
                     << endl;
    }
  }

  if  (classes.QueueSize () != crossClassProbTableSize)
  {
    log.Level (-1) << endl
                   << "SVMModel::RetrieveCrossProbTable      ***ERROR***"                                       << endl
                   << "                                     'classes.QueueSize () != crossClassProbTableSize'"  << endl
                   << endl;
    return;
  }


  // x,y         = 'Callers'   Class Indexes..
  // xIdx, yIdx  = 'SVMNodel'  Class Indexed.
  for  (x = 0;  x < classes.QueueSize ();  x++)
  {
    kkint32 xIdx = indexTable[x];
    if  (xIdx >= 0)
    {
      for  (y = 0;  y < classes.QueueSize ();  y++)
      {
        kkint32  yIdx = indexTable[y];
        if  (yIdx >= 0)
        {
          if  ((x != xIdx)  ||  (y != yIdx))
          {
            //kak  I just added this check to see when this situation actually occurs.
            kkint32 zed = 111;
          }
          crossProbTable[x][y] = this->crossClassProbTable[xIdx][yIdx];
        }
      }
    }
  }

  delete  indexTable;  indexTable = NULL;
  return;
}  /* RetrieveCrossProbTable */
