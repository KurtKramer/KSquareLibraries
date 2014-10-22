#include "FirstIncludes.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;

#if  defined(WIN32)
#include <LIMITS.H>
#include <FLOAT.H>
#include <windows.h>
#define _SCL_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#endif



#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "XmlStream.h"
using namespace  KKB;




#include "MLClass.h"
#include "FeatureVector.h"
#include "ClassProb.h"

#include "UsfCasCor.h"
using namespace  KKMachineLearning;



/*
   Cascade Correlation Neural Network

   See notes below for original author, etc.

   Changes made by Steven Eschrich <eschrich@csee.usf.edu>
   Fall, 2001

 - The code was heavily modified in the I/O portion to follow
   C4.5 standard data formats (although currently only real-valued
   attributes are supported). It parses the .names file for classes
   (1 of N representation) and feature values (ignore or real). The
   logic simply treats all non-ignore fields as real so integer labels
   are treated as continuous.

 - All network parameters can be passed via command line arguments,
   try cascor -h for details. Briefly,
     -i #    input epochs
     -o #    output epochs
     -r #    number of new units to add
     -P      make predictions. generates a filestem.pred file with class
       names as predictions (from a .test file).
     -N      normalize features to 0-1
     -f filestem   Filestem for data
     -R      use training set (resubstitution)

   All other parameters that could be specified in the .net file are
   specified as
     -O option=value    (e.g. -O UseCache=True)

 - Defaults are used for parameters, in case someone starts it without
   any...

 - Support for index files (-I option). That is, the following files
   are supposed to exist
     filestem.data
     filestem.idata
     filestem.itest (-P only)
     filestem.names

   The .idata and .itest are text files with the index number of the record
   in .data desired. The .data file **must** be in fixed-width format, each
   line padded to the same length with spaces.
*/
/****************************************************************************/
/* C implementation of the Cascade-Correlation learning algorithm.          */
/*                                                                          */
/*   Written by:          R. Scott Crowder, III                             */
/*                        School of Computer Science                        */
/*                        Carnegie Mellon University                        */
/*                        Pittsburgh, PA 15213-3890                         */
/*                                                                          */
/*                        Phone: (412) 268-8139                             */
/*        Internet:  rsc@cs.cmu.edu                                         */
/*                                                                          */
/*                                                                          */
/*  This code has been placed in the public domain by the author.  As a     */
/*  matter of simple courtesy, anyone using or adapting this code is        */
/*  expected to acknowledge the source.  The author would like to hear      */
/*  about any attempts to use this system, successful or not.               */
/*                                                                          */
/*  This code is a port to C from the original Common Lisp implementation   */
/*  written by Scott E. Fahlman.  (Version dated June 1 1990.)              */
/*                                                                          */
/*  For an explanation of this algorithm and some results, see "The         */
/*  Cascade-Correlation Learning Architecture" by Scott E. Fahlman and      */
/*  Christian Lebiere in D. S. Touretzky (ed.), "Advances in Neural         */
/*  Information Processing Systems 2", Morgan Kaufmann, 1990.  A somewhat   */
/*  longer version is available as CMU Computer Science Tech Report         */
/*  CMU-CS-90-100.  Instructions for Ftping this report are given at the    */
/*  end of this file.                                                       */
/*                                                                          */
/*  An example of the network set up file is provided at the bottom of      */
/*  this file.                                                              */
/*                                                                          */
/*  This code has been successfully compiled on the following machines.     */
/*                                                                          */
/*  DEC Station 3100 using the MIPS compiler version 1.31                   */
/*  Sun 4 using the gcc compiler version 1.23                               */
/*  IBM PC-RT  using the cc compiler                                        */
/*  IBM RS6000 (Model 520) using the xlc compiler                           */
/*  386 machine using the Turbo C 2.0 compiler                              */
/*  The implementation compiles with the ANSI standard.  Some machine       */
/*  specific preprocessor commands are required.  It is assumed that your   */
/*  system will provide the required preprocessor arguments.                */
/*                                                                          */
/****************************************************************************/
/* Change Log                                                               */
/****************************************************************************/
/*                                                                          */
/* Changes from Release 1 dated Jun-12-90 to Version 1.14 Jul-18-90         */
/*                                                                          */
/*  bug fix in TYPE_CONVERT  Thanks to Michael Witbrock for the 1st report  */
/*  bug fix in BUILD_NET     Thanks to Michael Witbrock for the 1st report  */
/*  bug fix in GET_ARRAY_ELEMENT       Thanks to Ken Lang                   */
/*  bug fix in COMPUTE_CORRELATIONS    Thanks to Eric Melz                  */
/*  bug fix in ADJUST_CORRELATIONS     Thanks to Chris Lebiere              */
/*  bug fix in COMPUTE_SLOPES          Thanks to Chris Lebiere              */
/*  removed 2nd call to INIT_GLOBALS   Thanks to Dimitris Michailidis       */
/*  Added UnitType ASYMSIGMOID for users who like sigmoids to go from 0-1   */
/*     all learning utility functions changed with this addition.           */
/*  Added command line argument option type 'cascor1 help' for usage info.  */
/*  Added .net file and on-the-fly parameter adjustment code.  See new      */
/*   samples files at the end of this listing for examples.  Functions      */
/*   main and GET_NETWORK_CONFIGURATION have changed completely.            */
/*  GET_USER_INPUT replaced by Y_OR_N_P                                     */
//*  <signal.h> included to support on-the-fly parameter updating            */
/****************************************************************************/
/*                                                                          */
/* Changes from Version 1.15 Jul-18-90 to  1.16  Oct-24-90                  */
/*                                                                          */
/* bug fix in BUILD_NETWORK, INSTALL_NEW_UNIT, and TRAIN to allow           */
/* NTestPatterns > NTrainingPatterns.  Thanks to William Stevenson          */
/****************************************************************************/
/*                                                                          */
/* Changes from Version 1.16  Oct-24-90  to 1.17  Nov-12-90                 */
/****************************************************************************/
/* bug fix in TRAIN line 1662 change NtrainingPatterns to NTrainingPatterns */
/*  Thanks to Merrill Flood for pointing out the problem.                   */
/****************************************************************************/
/*                                                                          */
/* Changes from Version 1.17  Nov-12-90 to 1.30  Jan-23-91                  */
/****************************************************************************/
/* Added code to allow user to save the weights into and load weights       */
/*   from external files.                                                   */
/* Added code to allow saving of .net files to save any changes to          */
/*   parameters made during interactive learning trials.                    */
/* Added an alternative main routine that can be used to calculate          */
/*   predictions using a previously saved set of weights.  To activate      */
/*   this feature compile the code with the symbol PREDICT_ONLY defined.    */
/* Added code to allow '# comment' lines in the training or test sets.      */
/* Added optional code to calculate the number of multiply-accumulates      */
/*   used during training.  This is useful for comparing                    */
/*   Cascade-correlation to other learning algorithms in a machine          */
/*   independent manner.  To activate this feature define the symbol        */
/*   CONNX at compile time.                                                 */
/* Added code to calculate the Lapedes and Faber Index.  Useful for    */
/*   problems with real-valued outputs.                                     */
/* Added UnitType VARSIGMOID which can have arbitrary output range          */
/*   defined by SigmoidMin and SigmoidMax.  Thanks to Dimitris              */
/*   Michailidis.                                                           */
/* Added code to allow the training and test data to be read from           */
/*   separate files.  Thanks to Carlos Puchol.                              */
/* Added code to save SumError for each output instead of combining it      */
/*   together for all outputs.  This change helps for multiple output       */
/*   problems.  Thanks to Scott Fahlman.                                    */
/* Code added to allow specification of a NonRandomSeed for the random      */
/*   number generator.  Thanks to Dimitris Michailidis.                     */
/* Removed useless setting of Ninputs and Noutputs from BUILD_NET.  Thanks  */
/*   to Dimitris Michailidis.                                               */
/****************************************************************************/
/*                                                                          */
/* Changes from Version 1.30  Jan-23-91 to 1.31 Jan-25-91                   */
/* fixed typo.  include <string.h> not <sting.h> thanks to Peter Hancock.   */
/*                                                                          */
/* Changes from Version 1.31 Jan-25-91 to 1.32 Mar-21-91                    */
/* BUG FIX in INIT_NET.  Thanks to Boris Gokhman                            */
/* BUG FIX in TEST_EPOCH.  Thanks to Boris Gokhman                          */
/*                                                                          */
/* Changes from Version 1.32 Mar-21-91 to 1.33 Apr-16-92                    */
/* Prototype correction for strtok.  Thanks to Brian Ripley                 */
/****************************************************************************/



// Forward Declarations.
kkint32      GetProcessId ();
kkint64      GetLocalDateTime ();
char*        GetDsiplayableTime ();
char* index (char* str, int c);

#if  defined(WIN32)
int  strncasecmp (const char*  s1, 
                  const char*  s2, 
                  int          count
                 );
#endif



const char*  _(const char* str)
{
  return str;
}




UsfCasCor::UsfCasCor (FileDescPtr    _fileDesc,
                      VolConstBool&  _cancelFlag,
                      RunLog&        _log
                     ):

  //***************************************************************
  //*                            usfcascor                        *
  //***************************************************************
  in_limit             (500),
  out_limit            (500),
  number_of_trials     (1),
  number_of_rounds     (-1),
  normalization_method (0),
  my_mpi_rank          (0),

  //*********************************************************************
  //                             globals.c                              *
  //*********************************************************************
  number_of_classes       (-1),
  feature_type            (NULL),

  the_random_seed         (0),

  load_weights            (False),

  UnitType                (SIGMOID),
  OutputType              (SIGMOID),
  SigmoidMax              (0.5f),
  SigmoidMin              (-0.5f),
  WeightRange             (1.0f),
  SigmoidPrimeOffset      (0.1f),
  WeightMultiplier        (1.0f),
  OutputMu                (2.0f),
  OutputShrinkFactor      (0.0f),
  OutputEpsilon           (0.35f), 
  OutputDecay             (0.0001f),
  OutputPatience          (8),    
  OutputChangeThreshold   (0.01f),
  InputMu                 (2.0f),
  InputShrinkFactor       (0.0f),
  InputEpsilon            (1.0f),
  InputDecay              (0.0f),
  InputPatience           (8),
  InputChangeThreshold    (0.03f),
                                  

  /*  Variables related to error and correlation.   */
  TrueError               (0.0f),
  ScoreThreshold          (0.35f),
  ErrorBits               (0),
  SumErrors               (NULL),
  DummySumErrors          (NULL),
  SumSqError              (0.0f),
  BestCandidateScore      (0.0f),
  BestCandidate           (0),


  /* These variables and switches control the simulation and display.    */
  UseCache                (False),
  Epoch                   (0),        
  Graphics                (False),
  NonRandomSeed           (False),
  Test                    (True),
  SinglePass              (False),
  SingleEpoch             (False),
  Step                    (False),
  Trial                   (0),

  /* The sets of training inputs and outputs. */
  NTrainingPatterns       (0),
  NTestPatterns           (0),
  TrainingInputs          (NULL),
  TrainingOutputs         (NULL),
  Goal                    (NULL),

  example_weight          (NULL),

  /*  For some benchmarks there is a separate set of values used for testing */
  /*  the network's ability to generalize.  These values are not used during */
  /*  training.                                                              */
  TestInputs              (NULL),
  TestOutputs             (NULL),

  MaxUnits                (2000),
                  
  Nunits                  (0),
  Ninputs                 (0),
  Noutputs                (0),
  Ncandidates             (8),
  MaxCases                (0),
  Ncases                  (0),
  FirstCase               (0),
                    
  /***************************************************************************/
  /* The following vectors hold values related to hidden units in the active */
  /* net and their input weights.                                            */
  /***************************************************************************/
  Values                  (NULL),
  ValuesCache             (NULL),
  ExtraValues             (NULL),
  Nconnections            (NULL),
  Connections             (NULL),
  Weights                 (NULL),

  /***************************************************************************/
  /* The following arrays of arrays hold values for the outputs of the active*/
  /*  network and the output-side weights.                                   */
  /***************************************************************************/
  Outputs                 (NULL),
  Errors                  (NULL),
  ErrorsCache             (NULL),
  ExtraErrors             (NULL),
  OutputWeights           (NULL),
  OutputDeltas            (NULL),
  OutputSlopes            (NULL),
  OutputPrevSlopes        (NULL),

  /***************************************************************************/
  /* The following arrays have one entry for each candidate unit in the      */
  /* pool of trainees.                                                       */
  /***************************************************************************/
  CandValues              (NULL),
  CandSumValues           (NULL),
  CandCor                 (NULL),
  CandPrevCor             (NULL),
  CandWeights             (NULL),
  CandDeltas              (NULL),
  CandSlopes              (NULL),
  CandPrevSlopes          (NULL),

  /***************************************************************************/
  /* This saves memory if each candidate unit receives a connection from     */
  /* each existing unit and input.  That's always true at present, but may   */
  /* not be in future.                                                       */
  /***************************************************************************/
  AllConnections          (NULL),

  /***************************************************************************/
  /* ErrorIndex specific globals.  Not in release Lisp version               */
  /***************************************************************************/
  NtrainingOutputValues   (0),
  NtestOutputValues       (0),
  TrainingStdDev          (1.0f),
  TestStdDev              (1.0f),
  ErrorIndex              (0.0f),
  ErrorIndexThreshold     (0.2f),
  ErrorMeasure            (BITS),


  /***************************************************************************/
  /* Save and plot file related varibles                                     */
  /***************************************************************************/
  WeightFile              (NULL),
  InterruptPending        (False),
  Nparameters             (0),
  classes                 (NULL),
  fileDesc                (_fileDesc),
  selectedFeatures        (NULL),
  cancelFlag              (_cancelFlag),
  log                     (_log)

{
  ConstructParmTable ();
}





UsfCasCor::~UsfCasCor ()
{
  delete  classes;           classes          = NULL;
  delete  selectedFeatures;  selectedFeatures = NULL;
  CleanUpMemory ();
}


template<typename T>
void  UsfCasCor::Delete2DArray (T**     &A,    
                                kkuint32  numRows
                               )
{
  if  (!A)
    return;

  for  (kkuint32 x = 0;  x < numRows;  ++x)
  {
    delete  A[x];
    A[x] = NULL;
  }

  delete  A;
  A = NULL;
}  /* Delete2DArray */


void  UsfCasCor::CleanUpMemory ()
{
  delete  ExtraValues;    ExtraValues  = NULL;
  delete  Nconnections;   Nconnections = NULL;
  delete  Connections;    Connections  = NULL;

  Delete2DArray (Weights, MaxUnits);

  delete  ExtraErrors;     ExtraErrors     = NULL;
  delete  SumErrors;       SumErrors       = NULL;
  delete  DummySumErrors;  DummySumErrors  = NULL;

  delete  Outputs;         Outputs = NULL;

  Delete2DArray (OutputWeights,    Noutputs);
  Delete2DArray (OutputDeltas,     Noutputs);
  Delete2DArray (OutputSlopes,     Noutputs);
  Delete2DArray (OutputPrevSlopes, Noutputs);

  delete  CandValues;     CandValues    = NULL;
  delete  CandSumValues;  CandSumValues = NULL;

  Delete2DArray (CandCor,         Ncandidates);
  Delete2DArray (CandPrevCor,     Ncandidates);
  Delete2DArray (CandWeights,     Ncandidates);
  Delete2DArray (CandDeltas,      Ncandidates);
  Delete2DArray (CandSlopes,      Ncandidates);
  Delete2DArray (CandPrevSlopes,  Ncandidates);
  Delete2DArray (TrainingInputs,  NTrainingPatterns);
  Delete2DArray (TrainingOutputs, NTrainingPatterns);
  Delete2DArray (TestInputs,      NTestPatterns);
  Delete2DArray (TestOutputs,     NTestPatterns);
  Delete2DArray (ValuesCache,     MaxCases);
  Delete2DArray (ErrorsCache,     MaxCases);

  delete  feature_type;    feature_type   = NULL;
  delete  AllConnections;  AllConnections = NULL;
}  /* CleanUpMemory */








kkint32  UsfCasCor::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (*this);

  if  (feature_type)     memoryConsumedEstimated += Ninputs  * sizeof (int);
  if  (SumErrors)        memoryConsumedEstimated += Noutputs * sizeof (float);
  if  (DummySumErrors)   memoryConsumedEstimated += Noutputs * sizeof (float);
  if  (TrainingInputs)   memoryConsumedEstimated += NTrainingPatterns * Ninputs  * sizeof (float);
  if  (TrainingOutputs)  memoryConsumedEstimated += NTrainingPatterns * Noutputs * sizeof (float);

  if  (AllConnections)   memoryConsumedEstimated += MaxUnits * sizeof (int);
  if  (Nconnections)     memoryConsumedEstimated += MaxUnits * sizeof (int);
  if  (Connections)      memoryConsumedEstimated += MaxUnits * sizeof (int*);

  if  (Nconnections)
  {
    memoryConsumedEstimated += MaxUnits * sizeof (float*);
    for  (int x = 0;  x < MaxUnits;  ++x)
      memoryConsumedEstimated += Nconnections[x] * sizeof (float);
  }
  
  if  (ExtraValues)       memoryConsumedEstimated += MaxUnits          * sizeof (float);
  if  (example_weight)    memoryConsumedEstimated += NTrainingPatterns * sizeof (float);
  if  (ValuesCache)       memoryConsumedEstimated += MaxCases * MaxUnits  * sizeof (float);
  if  (ErrorsCache)       memoryConsumedEstimated += MaxCases * Noutputs  * sizeof (float);
  if  (Outputs)           memoryConsumedEstimated += Noutputs * sizeof (float);


  if  (OutputWeights)     memoryConsumedEstimated += MaxCases * Noutputs  * sizeof (float);
  if  (OutputDeltas)      memoryConsumedEstimated += MaxCases * Noutputs  * sizeof (float);
  if  (OutputSlopes)      memoryConsumedEstimated += MaxCases * Noutputs  * sizeof (float);
  if  (OutputPrevSlopes)  memoryConsumedEstimated += MaxCases * Noutputs  * sizeof (float);

  if  (ExtraErrors)       memoryConsumedEstimated += Noutputs * sizeof (float);
  if  (CandValues)        memoryConsumedEstimated += Noutputs * sizeof (float);
  if  (CandSumValues)     memoryConsumedEstimated += Noutputs * sizeof (float);

  if  (CandCor)           memoryConsumedEstimated += Ncandidates * Noutputs  * sizeof (float);
  if  (CandPrevCor)       memoryConsumedEstimated += Ncandidates * Noutputs  * sizeof (float);
  if  (CandWeights)       memoryConsumedEstimated += Ncandidates * MaxUnits  * sizeof (float);
  if  (CandDeltas)        memoryConsumedEstimated += Ncandidates * MaxUnits  * sizeof (float);
  if  (CandSlopes)        memoryConsumedEstimated += Ncandidates * MaxUnits  * sizeof (float);
  if  (CandPrevSlopes)    memoryConsumedEstimated += Ncandidates * MaxUnits  * sizeof (float);

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */




/* Administrative variables */
const char*  UsfCasCor::version="5.0";
const char*  UsfCasCor::release_date="07-07-2012";
const char*  UsfCasCor::progname="UsfCasCor";



MLClassPtr  UsfCasCor::PredictClass (FeatureVectorPtr  example)
{
  MLClassPtr  predictedClass = NULL;
  _load_test_data (example);

  float  totalFeature = example->TotalOfFeatureData ();

  /* Globals must be saved from the last training phase. If they are not  */
  /* saved then the next unit will be training to correlate with the test */
  /* set error. */
  Boolean old_UC = UseCache;	/* temporarily turn off cache */
  float old_ST = ScoreThreshold; /* save global */
  float old_TE = TrueError;	/* save global */
  float *old_SE = SumErrors;	/* save global */
  float old_SSE = SumSqError;	/* save global */

  //ScoreThreshold = test_threshold;
  UseCache = False;

  Values = ExtraValues;
  Errors = ExtraErrors;

  int i = 0;
  {
    Goal = TestOutputs[i];
    FULL_FORWARD_PASS (TestInputs[i]);
    
    /* Find max. output (predicted class) */
    int max_pred = 0;
    for  (int j = 0;  j < Noutputs;  j++)
    {
      if  (Outputs[max_pred] < Outputs[j]) 
         max_pred = j;
    }

    if  (max_pred < classes->QueueSize ())
      predictedClass = classes->IdxToPtr (max_pred);
  } 

  /* restore globals */
  UseCache = old_UC;		
  ScoreThreshold = old_ST;
  TrueError = old_TE;	
  SumErrors = old_SE;	
  SumSqError = old_SSE;

  return  predictedClass;
}  /* PredictClass */




void  UsfCasCor::PredictConfidences (FeatureVectorPtr            example,
                                     MLClassPtr          knownClass,
                                     MLClassPtr&         predClass1,
                                     float&                      predClass1Prob,
                                     MLClassPtr&         predClass2,
                                     float&                      predClass2Prob,
                                     float&                      knownClassProb,
                                     const MLClassList&  classOrder,      /**< Dictates the order in which 'probabilities' will be populatd. */
                                     VectorFloat&                probabilities
                                    )
{
  float  totalFeature = example->TotalOfFeatureData ();
  _load_test_data (example);

  /* Globals must be saved from the last training phase. If they are not  */
  /* saved then the next unit will be training to correlate with the test */
  /* set error. */
  Boolean old_UC  = UseCache;	      /* temporarily turn off cache */
  float   old_ST  = ScoreThreshold; /* save global */
  float   old_TE  = TrueError;      /* save global */
  float*  old_SE  = SumErrors;      /* save global */
  float   old_SSE = SumSqError;     /* save global */

  //ScoreThreshold = test_threshold;
  UseCache = False;

  Values = ExtraValues;
  Errors = ExtraErrors;

  /* Zero some accumulators. */
  TrueError = 0.0;
  SumErrors = DummySumErrors;
  SumSqError = 0.0;
 
  float  totalDelta = 0.0f;
  predClass1     = NULL;
  predClass2     = NULL;
  predClass1Prob = -9999.99f;
  predClass2Prob = -9999.99f;

  int i = 0;
  {
    Goal = TestOutputs[i];
    FULL_FORWARD_PASS (TestInputs[i]);
    
    /* Find max. output (predicted class) */

    for  (int j = 0;  j < Noutputs;  j++)
      totalDelta += (Outputs[j] - SigmoidMin);
  } 

  probabilities.clear ();
  for  (int j = 0;  j < Noutputs;  j++)
  {
    MLClassPtr  ic = classes->IdxToPtr (j);

    float  prob = (Outputs[j] - SigmoidMin) / totalDelta;

    if  (ic == knownClass)
      knownClassProb = prob;

    if  (prob > predClass1Prob)
    {
      predClass2     = predClass1;
      predClass2Prob = predClass1Prob;
      predClass1     = ic;
      predClass1Prob = prob;
    }
    else if  (prob > predClass2Prob)
    {
      predClass2     = ic;
      predClass2Prob = prob;
    }
  }

  probabilities.clear ();
  MLClassList::const_iterator  idx;
  for  (idx = classOrder.begin ();  idx != classOrder.end ();  ++idx)
  {
    MLClassPtr  ic = *idx;
    int  j = classes->PtrToIdx (ic);
    if  (j >= 0)
    {
      float  prob = (Outputs[j] - SigmoidMin) / totalDelta;
      probabilities.push_back (prob);
    }
    else
    {
      probabilities.push_back (0.0f);
    }
  }
      
  /* restore globals */
  UseCache       = old_UC;		
  ScoreThreshold = old_ST;
  TrueError      = old_TE;	
  SumErrors      = old_SE;	
  SumSqError     = old_SSE;

  return;
}  /* PredictConfidences */






ClassProbListPtr  UsfCasCor::PredictClassConfidences (FeatureVectorPtr  example)
{
  float  totalFeature = example->TotalOfFeatureData ();
  _load_test_data (example);

  /* Globals must be saved from the last training phase. If they are not  */
  /* saved then the next unit will be training to correlate with the test */
  /* set error. */
  Boolean old_UC  = UseCache;	      /* temporarily turn off cache */
  float   old_ST  = ScoreThreshold; /* save global */
  float   old_TE  = TrueError;      /* save global */
  float*  old_SE  = SumErrors;      /* save global */
  float   old_SSE = SumSqError;     /* save global */

  //ScoreThreshold = test_threshold;
  UseCache = False;

  Values = ExtraValues;
  Errors = ExtraErrors;

  /* Zero some accumulators. */
  TrueError = 0.0;
  SumErrors = DummySumErrors;
  SumSqError = 0.0;
 
  float  totalDelta = 0.0f;
  int i = 0;
  {
    Goal = TestOutputs[i];
    FULL_FORWARD_PASS (TestInputs[i]);
    for  (int j = 0;  j < Noutputs;  j++)
      totalDelta += (Outputs[j] - SigmoidMin);
  } 


  ClassProbListPtr  results = new ClassProbList (true);

  for  (int j = 0;  j < Noutputs;  j++)
  {
    MLClassPtr  ic = classes->IdxToPtr (j);
    float  prob = (Outputs[j] - SigmoidMin) / totalDelta;
    results->PushOnBack (new ClassProb (ic, prob, 0.0f));
  }

  /* restore globals */
  UseCache       = old_UC;		
  ScoreThreshold = old_ST;
  TrueError      = old_TE;	
  SumErrors      = old_SE;	
  SumSqError     = old_SSE;

  return  results;
}  /* PredictClassConfidences */





void  UsfCasCor::TrainNewClassifier (kkint32                _in_limit,
                                     kkint32                _out_limit,
                                     kkint32                _number_of_rounds,
                                     kkint32                _number_of_trials,
                                     kkint64                _the_random_seed,
                                     bool                   _useCache,
                                     FeatureVectorListPtr   _trainData,
                                     const FeatureNumList&  _selectedFeatures
                                    )
{
  log.Level (10) << "Cascade Correlation:  Version[" << version << "]" << endl;

  selectedFeatures = new FeatureNumList (_selectedFeatures);

  FeatureVectorListPtr  filteredTrainData = FilterOutExtremeExamples (_trainData);

  delete  classes;
  classes = filteredTrainData->ExtractListOfClasses ();
  classes->SortByName ();

  INITIALIZE_GLOBALS ();

  in_limit         = _in_limit;
  out_limit        = _out_limit;
  number_of_rounds = _number_of_rounds;
  number_of_trials = _number_of_trials;
  the_random_seed  = _the_random_seed;
  if  (_useCache)
    UseCache = True;
  else
    UseCache = False;

  /* First, load the data and configuration */
  setup_network (filteredTrainData);

  train_network ();

  delete  filteredTrainData;
}  /* TrainNewClassifier */





void  UsfCasCor::LoadExistingClassifier (istream&  i,
                                         bool&     valid
                                        )
{
  log.Level (10) << "UsfCasCor::LoadExistingClassifier   ProgName[" << progname << "]  Version[" << version << "]" << endl;

  this->ReadXml (i, valid);
}  /* LoadExistingClassifier */



/**
 *@brief Will create a list that excludes wamples that have extreeem values that can trip up the Neural net.
 */
FeatureVectorListPtr  UsfCasCor::FilterOutExtremeExamples (FeatureVectorListPtr  trainExamples)
{
  // At this point the trainig data should be normalized.

  kkint16          numSelFeatures = selectedFeatures->NumOfFeatures ();
  const kkuint16*  selFeatures    = selectedFeatures->FeatureNums ();
 
  FeatureVectorListPtr result = new FeatureVectorList (trainExamples->FileDesc (), false, log);
  FeatureVectorList::iterator  idx;
  for  (idx = trainExamples->begin ();  idx != trainExamples->end ();  ++idx)
  {
    bool extremeValuesFound = false;
    FeatureVectorPtr fv = *idx;

    for  (kkint32 x = 0;  x < numSelFeatures;  ++x)
    {
      if  (fabs (fv->FeatureData (selFeatures[x])) > 1000.0)
      {
        extremeValuesFound = true;
        break;
      }
    }

    if  (!extremeValuesFound)
      result->PushOnBack (fv);
  }

  return  result;
}  /* FilterOutExtremeExamples */




/*
 *  Get and initialize a network. 
 */
void  UsfCasCor::setup_network (FeatureVectorListPtr  trainExamples)
{
  /* 
     There are some required variables, like NInputs,etc
     that can be taken from the training/testing files. 
  */
  load_namesfile (trainExamples, selectedFeatures);


  /* At this point, it looks like the MaxUnits parameter is
     simply the sum of NInputs+1 and the max. number of units
     to add. Set this manually, since it doesn't seem to be
     set elsewhere.
  */


  if  (number_of_rounds == -1)
     number_of_rounds=15;

  //(Ninputs+Noutputs)/2;
  MaxUnits = Ninputs + 1 + number_of_rounds;

  /* Once all arguments have been read and what parameters we
     have, we have -- then, build the network and load the data.
  */
  allocate_network ();
  load_data (trainExamples);

  /* Randomization. If not specified on command line and NonRandomSeed
     is not True then seed with time (truly random) */
  if  (NonRandomSeed) 
     the_random_seed = 1;

  if (the_random_seed <= 0)
     the_random_seed = time(NULL);

  the_random_seed += GetProcessId () + my_mpi_rank;

  SRand48 (the_random_seed);
  log.Level (10) << "Starting seed " << ((NonRandomSeed)?"fixed":"random") << " at " << the_random_seed << endl;

  /* Initialize the network variables */
  initialize_network ();

  return;
}  /* setup_network */




void UsfCasCor::train_network ()
{
  int nhidden;      /* number of hidden units used in run  */
  int vics, defs, i;
  long total_epochs, total_units, total_trials;
  long min_units, max_units, min_epochs, max_epochs;

  /* initialize testing parms */
  total_epochs = 0;
  total_units = 0;
  min_units = INT_MAX;
  min_epochs = INT_MAX;
  max_units = 0;
  max_epochs = 0;
  total_trials = 0;
  vics = 0;
  defs = 0;

  /* Start the main processing loop */
  log.Level (10) << "UsfCasCor::train_network   Starting run,  "
                 << "Ilim[" << in_limit << "] "
                 << "Olim [" << MaxUnits << "]  "
                 << "NumberOfRounds[" << number_of_rounds << "]  "
                 << "NumberOfTrials[" << number_of_trials << "]."
                 << endl;

  for  (i = 0; (i < number_of_trials)  &&  (!cancelFlag); i++)
  {
    Trial = i + 1;
    
    if  (number_of_trials > 1) 
      log.Level (10) << "train_network  Trial " << Trial << endl;

    switch (TRAIN (out_limit, in_limit, number_of_rounds))
    {
     case WIN:
          vics++;
          break;

     case LOSE:
          defs++;
           break;
    }

    /* how did we do? */
    if  (Test)
      TEST_EPOCH (ScoreThreshold);

#ifdef CONNX
    printf (" Connection Crossings: %d\n\n", conx);
#endif

    /* collect trail stats */
    nhidden = Nunits - Ninputs - 1;  /* don't count inputs or bias unit */
    total_epochs += Epoch;
    total_units += nhidden;
    total_trials++;
    min_epochs = (Epoch < min_epochs) ? Epoch : min_epochs;
    max_epochs = (Epoch > max_epochs) ? Epoch : max_epochs;
    min_units = (nhidden < min_units) ? nhidden : min_units;
    max_units = (nhidden > max_units) ? nhidden : max_units;
  } /* End trial loop */
  
  /* print out loop stats */
  log.Level (10) << endl << "train_network   TRAINING STATS" << endl;
  LIST_PARAMETERS ();
  log.Level (10) << "Victories: " << vics << ", Defeats: " << defs << endl;

  log.Level (10) << "Training Epochs - "
                 << "Min: " << min_epochs << "  "
                 << "Avg: " << (total_epochs / total_trials) << " "
                 << "Max: " << max_epochs
                 << endl;

  log.Level (10) << "Hidden Units -    "
                 << "Min: " << min_units << " "
                 << "Avg: " << ((float)total_units /total_trials) << " "
                 << "Max: " << max_units
                 << endl;

  return;
}  /* train_network */




/* Create the network data structures, given the number of input and output
 * units.  Get the MaxUnits value from a variable.
 */
void  UsfCasCor::allocate_network (void)
{
  int i;
/***************/

  if  (NTrainingPatterns > NTestPatterns)
    MaxCases = NTrainingPatterns;
  else
    MaxCases = NTestPatterns;

  Ncases    = NTrainingPatterns;
  FirstCase = 0;
  Nunits    = 1 + Ninputs;


  /* setup for ErrorIndex */
  NtrainingOutputValues = Noutputs * NTrainingPatterns; 
  NtestOutputValues = Noutputs * NTestPatterns;

  if  (Nunits > MaxUnits)
  {
    log.Level (-1) << endl 
      << "UsfCasCor::allocate_network  ***ERROR***   MaxUnits[" << MaxUnits << "] must be greater than Ninputs[" << Ninputs << "]." << endl
      << "                        Adjusting MaxUnits to [" << (Nunits + 1) << "]." << endl
      << endl;

    MaxUnits = Nunits + 1;
  }


  /* allocate memory for outer arrays */
  ExtraValues = new float[MaxUnits];
  Values = ExtraValues;

  Nconnections   = new int[MaxUnits];
  Connections    = new int*[MaxUnits];
  Weights        = new float*[MaxUnits];

  ExtraErrors    = new float[Noutputs];
  SumErrors      = new float[Noutputs];
  DummySumErrors = new float[Noutputs];
  Errors         = ExtraErrors;
  
  Outputs          = new float[Noutputs];
  OutputWeights    = new float*[Noutputs];
  OutputDeltas     = new float*[Noutputs];
  OutputSlopes     = new float*[Noutputs];
  OutputPrevSlopes = new float*[Noutputs];

  CandValues     = new float[Ncandidates];
  CandSumValues  = new float[Ncandidates];
  CandCor        = new float*[Ncandidates];
  CandPrevCor    = new float*[Ncandidates];
  CandWeights    = new float*[Ncandidates];
  CandDeltas     = new float*[Ncandidates];
  CandSlopes     = new float*[Ncandidates];
  CandPrevSlopes = new float*[Ncandidates];
 
  TrainingInputs  = new float*[NTrainingPatterns];
  TrainingOutputs = new float*[NTrainingPatterns];

  if  (NTestPatterns)
  {
    TestInputs  = new float*[NTestPatterns];
    TestOutputs = new float*[NTestPatterns];
  }

  else
  {       /* no test patterns so just point at training set */
    TestInputs  = TrainingInputs;
    TestOutputs = TrainingOutputs;
  }

  /* Only create the caches if UseCache is on -- may not always have room. */
  if  (UseCache)
  {
    ValuesCache = new float*[MaxCases];
    ErrorsCache = new float*[MaxCases];
    for  (i = 0;  i < MaxCases;  i++)
    {
      ValuesCache[i] = new float[MaxUnits];
      ErrorsCache[i] = new float[Noutputs];
    }
  }

  /* Allocate per unit data arrays */
  for  (i = 0;  i < Noutputs;  i++)
  {
    OutputWeights   [i] = new float[MaxUnits];
    OutputDeltas    [i] = new float[MaxUnits];
    OutputSlopes    [i] = new float[MaxUnits];
    OutputPrevSlopes[i] = new float[MaxUnits];
  }

  for  (i = 0; i < Ncandidates; i++)
  {
    CandCor[i]        = new float[Noutputs];
    CandPrevCor[i]    = new float[Noutputs];

    CandWeights[i]    = new float[MaxUnits];
    CandDeltas[i]     = new float[MaxUnits];
    CandSlopes[i]     = new float[MaxUnits];
    CandPrevSlopes[i] = new float[MaxUnits];
  }

  /* Allocate per case data arrays */
  for  (i = 0; i < NTrainingPatterns; i++)
  {
    TrainingInputs[i]  = new float[Ninputs]; 
    TrainingOutputs[i] = new float[Noutputs];
  }

  for  (i = 0; i < NTestPatterns; i++)
  {
    TestInputs[i]  = new float[Ninputs];
    TestOutputs[i] = new float[Noutputs];
  }

  /* Allocate generic connection vector */
  AllConnections = new int[MaxUnits];

  return;
}  /* allocate_network */

  





/********************************************************************/
/*   interrupt handling routines                                    */
/*   Thanks to Dimitris Michailidis for this code.                  */
/********************************************************************/

/* allow user to change parameters if they have hit Control-C 
 */
void CHECK_INTERRUPT (void)
{
  /*   if (InterruptPending){
    printf("  Simulation interrupted at epoch %d\n", Epoch);

  }
  */
}



/* Record an interrupt whenever the user presses Control-C 
 */
void TRAP_CONTROL_C(int sig)
{
  /* 
  InterruptPending = True;
  signal(SIGINT, TRAP_CONTROL_C);
  */
}




//****************************************************************
//*                            'parms.c'                         *   
//****************************************************************


#define EOL '\0'


//char parm_string[LINELEN];

/* Save the settable parameters to a file for later use. */
char*  UsfCasCor::parm_to_string (int k)
{
  switch  (ParmTable[k].vartype)
  { 
  case INT: case INT_NO:
    sprintf (parm_string,"%s=%d\n", 
             ParmTable[k].keyword,
             *(int *)ParmTable[k].varptr
            );
    break;

  case FLOAT: case FLOAT_NO:
    sprintf (parm_string, "%s=%f\n", 
             ParmTable[k].keyword, 
             *(float *)ParmTable[k].varptr
            );
    break;

  case ENUM: case ENUM_NO:
    sprintf (parm_string, "%s=%s\n", 
                          ParmTable[k].keyword, 
                          type_to_string (*(int *)ParmTable[k].varptr)
            );
    break;

  case BOOLE: case BOOLE_NO:
    sprintf(parm_string, "%s=%s\n", ParmTable[k].keyword, 
      boolean_to_string(*(Boolean *)ParmTable[k].varptr));
    break;

  default:
    parm_string[0]=0;
    break;      /* skip anything else */
  }

  return parm_string;
}


const char *  UsfCasCor::type_strings[]={"SIGMOID","GAUSSIAN", "LINEAR","ASYMSIGMOID","VARSIGMOID","WIN","STAGNANT","TIMEOUT","LOSE","BITS","INDEX","Bad Type"};


/* Input of the type variables and return a string showing its value.  This
 * is only used as a output routine for the user's convenience. 
 */
const char*  UsfCasCor::type_to_string (int var)
{
  switch (var) 
  {
    case SIGMOID:     return(type_strings[0]);
    case GAUSSIAN:    return(type_strings[1]);
    case LINEAR:      return(type_strings[2]);
    case ASYMSIGMOID: return(type_strings[3]);
    case VARSIGMOID:  return(type_strings[4]);
    case WIN:         return(type_strings[5]);
    case STAGNANT:    return(type_strings[6]);
    case TIMEOUT:     return(type_strings[7]);
    case LOSE:        return(type_strings[8]);
    case BITS:        return(type_strings[9]);
    case INDEX:       return(type_strings[10]);

    default:          return(type_strings[11]);
  }
}  /* type_to_string */



int  UsfCasCor::string_to_type (const char* s)
{
  if  (STRICMP (s, type_strings[ 0]) == 0)  return SIGMOID;
  if  (STRICMP (s, type_strings[ 1]) == 0)  return GAUSSIAN;
  if  (STRICMP (s, type_strings[ 2]) == 0)  return LINEAR;
  if  (STRICMP (s, type_strings[ 3]) == 0)  return ASYMSIGMOID;
  if  (STRICMP (s, type_strings[ 4]) == 0)  return VARSIGMOID;
  if  (STRICMP (s, type_strings[ 5]) == 0)  return WIN;
  if  (STRICMP (s, type_strings[ 6]) == 0)  return STAGNANT;
  if  (STRICMP (s, type_strings[ 7]) == 0)  return TIMEOUT;
  if  (STRICMP (s, type_strings[ 8]) == 0)  return LOSE;
  if  (STRICMP (s, type_strings[ 9]) == 0)  return BITS;
  if  (STRICMP (s, type_strings[10]) == 0)  return INDEX;

  return -1;
} /* string_to_type */



const char *  UsfCasCor::boolean_strings[2] = {"False", "True"};



char const *  UsfCasCor::boolean_to_string (Boolean var)
{
  switch (var) 
  {
    case False:  return(boolean_strings[0]);
    case True:   return(boolean_strings[1]);
    default: return  boolean_strings[0];
 }
}  /* boolean_to_string */




Boolean  UsfCasCor::string_to_boolean (const char* s)
{
  if  ((STRICMP (s, "True") == 0)  ||
       (STRICMP (s, "T")    == 0)  ||
       (STRICMP (s, "yes")  == 0)  ||
       (STRICMP (s, "on")   == 0)
       )
    return True;
  else
    return False;
}





/* Convert '\0' terminated sting to all lower case characters.  This routine
 * is destructive.
 */
void  UsfCasCor::strdncase (char *s)
{
  int i;
  for  (i = 0;  s[i] != EOL;  i++)
  {
    if  (isupper(s[i]))
      s[i] = tolower(s[i]);  /* tolower only guaranteed on upper case */
    else
      s[i] = s[i];
  }
}



/* Given a keyword string, return the index into the keyword table.
 * Assumes that the keys are in alphabetacal order.  Keyword comparison
 * is all lower case.  Return FAILURE when not found.
 */
int  UsfCasCor::find_key (char *searchkey)
{
  int lower = 0;
  int upper = Nparameters - 1; 
  int m, dif;
  /************/

  strdncase (searchkey);    /* convert case for comparison */

  while(lower <= upper)
  {
    m = (upper + lower) / 2;
    dif = strcmp (searchkey, ParmTable[m].keyword);
    if  (dif < 0)
      upper = m - 1;    /* look in lower half */

    else if  (dif == 0)
      return  (m);    /* found it */

    else if  (dif > 0)
      lower = m + 1;    /* look in upper half */
  }
  
  /* search failed */
  return(FAILURE);
}  /* find_key */



/* Parse a line of input into keyword value pairs and reset the given 
 * parameters to given values.  Comment lines start with the character
 * '#' and are ignored.  If a bad keyword is given a message is printed,
 * but processing continues.  The routine returns a value telling the 
 * calling routine whether to grap another line, read in  the training, 
 * or read in testing data.  The special keywords "Training", and 
 * "Testing" signal the changes in status.
 */
int   UsfCasCor::process_line (char *line)
{
  int k = 0;      /* location in ParmTable */
  char *keytok;      /* token pointer */
  char *valtok;      /* token pointer */
  static const char *seperators = "= \t\v\f\r\n,"; /* white space plus comma  */
  /*************/

  /* check for comment character */
  if(line[0] == '#' || line[0] == '\n')
    return(NEXTLINE);    /* skip comment and blank lines */
  else
  {
    keytok = strtok(line, seperators); /* get first token */
    while(keytok != NULL)
    {
      k = find_key(keytok);

      if  (k != FAILURE)
      {
        /* get value token for this parameter */
        valtok = strtok(NULL, seperators);

        /* read value in correct format */
        int  vartype = ParmTable[k].vartype;
        switch  (vartype)
        { 
          case INT: 
          case INT_NO:
            *(int *)ParmTable[k].varptr = atoi(valtok);
            break;

          case FLOAT: 
          case FLOAT_NO:
            *(float *)ParmTable[k].varptr = (float)atof(valtok);
            break;

          case ENUM: 
          case ENUM_NO: 
            *(int *)ParmTable[k].varptr = _type_convert(valtok);
            break;

          case BOOLE: 
          case BOOLE_NO:
            *(Boolean *)ParmTable[k].varptr= _boolean_convert(valtok);
            break;

          case INITFILE:
            log.Level (-1) << endl << "***ERROR***   initfile option no longer accepted." << endl;
            break;

          case GO: 
          case VALUE: 
          case SAVE:
            log.Level (-1) << "Keyword[" << keytok << "] only legal in interactive mode." << endl;
            break;

          default:
            log.Level (-1) << "***ERROR***  Bad vartype[" << vartype << "] for parameter[" << keytok << "].  No update performed." << endl;
            break;
          }
      }
      else      /* bad key */
      {
        log.Level (-1) << endl 
          << "UsfCasCor::process_line  ***ERROR***   \"" << keytok << "\" is to in parameter table." << endl
          << endl;
      }

      /* get next keyword token */
      keytok = strtok(NULL, seperators);  
    }        /* end while still keytok */

    return (NEXTLINE);
  }        /* end if comment */
}  /* process_line*/




/**********************************************************
 Functions needed only in this file. 
***********************************************************/

int  UsfCasCor::_type_convert (char *input)
{
  strdncase(input);
  
  if  (!strcmp (input,"true"))
    return (1);

  else if  (!strcmp (input, "1")) /* allow backward compatiple input */
    return (1);

  else if  (!strcmp (input, "false"))
    return (0);

  else if  (!strcmp(input, "0"))  /* allow backward compatiple input */
    return (0);

  else if  (!strcmp (input, "sigmoid"))
    return (SIGMOID);

  else if  (!strcmp (input, "gaussian"))
    return  (GAUSSIAN);

  else if  (!strcmp (input, "linear"))
    return  (LINEAR);

  else if  (!strcmp (input, "asymsigmoid"))
    return  (ASYMSIGMOID);

  else if  (!strcmp (input, "varsigmoid"))
    return  (VARSIGMOID);

  else if  (!strcmp (input, "bits"))
    return  (BITS);

  else if  (!strcmp (input, "index"))
    return  (INDEX);

  else 
    return -1;
}  /* _type_convert */



Boolean  UsfCasCor::_boolean_convert (char *input)
{
  strdncase (input);
  if  (!strcmp (input, "true") || 
       !strcmp (input, "1")    ||
       !strcmp (input, "t")
      )
    return True;

  if  (!strcmp (input, "false") || !strcmp(input,"0"))
    return False;

  return False;
}  /* _boolean_convert */
 

//**********************************************************************************
//*                                 util.c                                         *
//**********************************************************************************



float  UsfCasCor::random_weight ()
{
  return ( (float) (WeightRange * (LRand48 () % 1000 / 500.0)) - WeightRange);
}



//**********************************************************************************
//*                                 netio.c                                        *
//**********************************************************************************

#define BEGIN_PARAMETER_STRING "# Parameters\n"
#define END_PARAMETER_STRING   "# End Parameters\n"
#define BEGIN_CONNECTION_STRING "# Connections\n"
#define END_CONNECTION_STRING   "# End Connections\n"
#define BEGIN_INPUT_WEIGHTS_STRING "# Input Weights\n"
#define END_INPUT_WEIGHTS_STRING "# End Input Weights\n"
#define BEGIN_OUTPUT_WEIGHTS_STRING "# Output Weights\n"
#define END_OUTPUT_WEIGHTS_STRING "# End Output Weights\n"


template<typename T>
ostream&  ArrayCommaDelimited (ostream& o, T* a, int N)
{
  for  (int x = 0;  x < N;  ++x)
  {
    if  (x > 0)
      o << ",";
    o << a[x];
  }
  return o;
}  /* ArrayCommaDelimited */



void  WriteXmlArrayFloat (const char*  name,
                          ostream&     o,
                          int          len,
                          float*       A
                         )
{
  o << "<ArrayFloat Name=" << name  << " Len=" << len << ">";
  for  (int x = 0;  x < len;  ++x)
  {
    if  (x > 0)
      o << ",";
    o << A[x];
  }
  o << "</ArrayFloat>";
  o << endl;
}  /* WriteXmlArrayFloat */



void  WriteXmlArrayFloat2D (const char*  name,
                            ostream&     o,
                            int          height,
                            int          width,
                            float**      A
                           )
{
  o << "<ArrayFloat2D Name=" << name  << " Height=" << height << " Width=" << width << ">" << endl;

  for  (int row = 0;  row < height;  ++row)
  {
    float*  rowData = A[row];
    o << "<Row idx = " << row << ">";
    for  (int col = 0;  col < width;  ++col)
    {
      if  (col > 0)
        o << ",";
      o << rowData[col];
    }
    o << "</Row>" << endl;
  }
  o << "</ArrayFloat2D>" << endl;
}  /* WriteXmlArrayFloat2D */




void  WriteXmlArrayFloat2DVarying (const char*  name,
                                   ostream&     o,
                                   int          height,
                                   int*         widths,
                                   float**      A
                                  )
{
  o << "<ArrayFloat2DVarying Name=" << name  << " Height=" << height << ">" << endl;

  for  (int row = 0;  row < height;  ++row)
  {
    float*  rowData = A[row];
    o << "<Row idx = " << row << " Width=" << widths[row] << ">";
    for  (int col = 0;  col < widths[row];  ++col)
    {
      if  (col > 0)
        o << ",";
      o << rowData[col];
    }
    o << "</Row>" << endl;
  }
  o  << "</ArrayFloat2DVarying>" << endl;
}  /* WriteXmlArrayFloat2DVarying */



void  WriteXmlArrayInt (const char*  name,
                        ostream&     o,
                        int          len,
                        int*         A
                       )
{
  o << "<ArrayInt Name=" << name  << " Len=" << len << ">";
  for  (int x = 0;  x < len;  ++x)
  {
    if  (x > 0)
      o << ",";
    o << A[x];
  }
  o << "</ArrayInt>";
  o << endl;
}  /* WriteXmlArrayInt */



void  UsfCasCor::WriteXmlConnections (ostream&  o)
{
  o << "<Connections len=" << MaxUnits << ">";
  for  (int x = 0;  x < MaxUnits;  ++x)
  {
    if  (x > 0)  o << ",";

    if  (Connections[x] == NULL)
      o << "NULL";
    else if  (Connections[x] == AllConnections)
      o << "AC";
    else
      o << "Other";
  }

  o << "</Connections>" << endl;
}  /* WriteXmlConnections */



void  UsfCasCor::WriteXML (ostream&  o)
{
  o << "<UsfCasCor>" << endl;

    /* Global variables */

  o << "progname"          << "\t" << progname << endl;

  o << "number_of_classes" << "\t" << number_of_classes               << endl;
  o << "classes"           << "\t" << classes->ToCommaDelimitedStr () << endl;
  o << "selectedFeatures"  << "\t" << selectedFeatures->ToString ()   << endl;

  o << "MaxUnits"                << "\t" << MaxUnits                << endl
    << "Nunits"                  << "\t" << Nunits                  << endl
    << "Ninputs"                 << "\t" << Ninputs                 << endl
    << "Noutputs"                << "\t" << Noutputs                << endl
    << "Ncandidates"             << "\t" << Ncandidates             << endl
    << "MaxCases"                << "\t" << MaxCases                << endl
    << "Ncases"                  << "\t" << Ncases                  << endl
    << "FirstCase"               << "\t" << FirstCase               << endl
    << "line_length"             << "\t" << line_length             << endl
    << "the_random_seed"         << "\t" << the_random_seed         << endl
    << "in_limit"                << "\t" << in_limit                << endl
    << "out_limit"               << "\t" << out_limit               << endl
    << "number_of_trials"        << "\t" << number_of_trials        << endl
    << "number_of_rounds"        << "\t" << number_of_rounds        << endl
    << "normalization_method"    << "\t" << normalization_method    << endl
    << "my_mpi_rank"             << "\t" << my_mpi_rank             << endl
    << "NTrainingPatterns"       << "\t" << NTrainingPatterns       << endl
    << "NTestPatterns"           << "\t" << NTestPatterns           << endl
    << "ErrorMisclassifications" << "\t" << ErrorMisclassifications << endl
    << "OutputPatience"          << "\t" << OutputPatience          << endl
    << "InputPatience"           << "\t" << InputPatience           << endl
    << "ErrorBits"               << "\t" << ErrorBits               << endl
    << "BestCandidate"           << "\t" << BestCandidate           << endl
    << "Epoch"                   << "\t" << Epoch                   << endl
    << "Trial"                   << "\t" << Trial                   << endl
    << "NtrainingOutputValues"   << "\t" << NtrainingOutputValues   << endl
    << "NtestOutputValues"       << "\t" << NtestOutputValues       << endl
    << "ErrorMeasure"            << "\t" << ErrorMeasure            << endl;


  o << "SigmoidMax"            << "\t" << SigmoidMax             << endl
    << "WeightRange"           << "\t" << WeightRange            << endl
    << "SigmoidPrimeOffset"    << "\t" << SigmoidPrimeOffset     << endl
    << "WeightMultiplier"      << "\t" << WeightMultiplier       << endl
    << "OutputMu"              << "\t" << OutputMu               << endl
    << "OutputShrinkFactor"    << "\t" << OutputShrinkFactor     << endl
    << "OutputEpsilon"         << "\t" << OutputEpsilon          << endl
    << "OutputDecay"           << "\t" << OutputDecay            << endl
    << "OutputChangeThreshold" << "\t" << OutputChangeThreshold  << endl
    << "InputMu"               << "\t" << InputMu                << endl
    << "InputShrinkFactor"     << "\t" << InputShrinkFactor      << endl
    << "InputEpsilon"          << "\t" << InputEpsilon           << endl
    << "InputDecay"            << "\t" << InputDecay             << endl
    << "InputChangeThreshold"  << "\t" << InputChangeThreshold   << endl
    << "TrueError"             << "\t" << TrueError              << endl
    << "ScoreThreshold"        << "\t" << ScoreThreshold         << endl
    << "SumSqError"            << "\t" << SumSqError             << endl
    << "BestCandidateScore"    << "\t" << BestCandidateScore     << endl
    << "TrainingStdDev"        << "\t" << TrainingStdDev         << endl
    << "TestStdDev"            << "\t" << TestStdDev             << endl
    << "ErrorIndex"            << "\t" << ErrorIndex             << endl
    << "ErrorIndexThreshold"   << "\t" << ErrorIndexThreshold    << endl;


  o << "UnitType"    << "\t" << type_to_string (UnitType)   << endl
    << "OutputType"  << "\t" << type_to_string (OutputType) << endl;

  o << "load_weights"            << "\t" << boolean_to_string (load_weights)            << endl
    << "UseCache"                << "\t" << boolean_to_string (UseCache)                << endl
    << "Graphics"                << "\t" << boolean_to_string (Graphics)                << endl
    << "NonRandomSeed"           << "\t" << boolean_to_string (NonRandomSeed)           << endl
    << "Test"                    << "\t" << boolean_to_string (Test)                    << endl
    << "SinglePass"              << "\t" << boolean_to_string (SinglePass)              << endl
    << "SingleEpoch"             << "\t" << boolean_to_string (SingleEpoch)             << endl
    << "Step"                    << "\t" << boolean_to_string (Step)                    << endl
    << "InterruptPending"        << "\t" << boolean_to_string (InterruptPending)        << endl;
  
  WriteXmlArrayInt ("feature_type", o, Ninputs, feature_type);

  WriteXmlArrayFloat ("SumErrors",      o, Noutputs, SumErrors);
  WriteXmlArrayFloat ("DummySumErrors", o, Noutputs, DummySumErrors);

  //>WriteXmlArrayFloat2D ("TrainingInputs",  o, NTrainingPatterns, Ninputs,  TrainingInputs);
  //>WriteXmlArrayFloat2D ("TrainingOutputs", o, NTrainingPatterns, Noutputs, TrainingOutputs);

  // Goal    is only a pointer to an existing Vector.
  // Values  is only a pointer to an existing Vector.

  // We will only need to alloate these variables not save or load them.
  //        float**  TestInputs;
  //        float**  TestOutputs;

  // 2012-07-20  Lines that start with "//>"  were determined not to be needed for prediction.

  if  (AllConnections)   WriteXmlArrayInt ("AllConnections", o, MaxUnits, AllConnections);
  if  (Nconnections)     WriteXmlArrayInt ("Nconnections", o, MaxUnits, Nconnections);
  WriteXmlConnections (o);
  WriteXmlArrayFloat2DVarying ("Weights", o, MaxUnits, Nconnections, Weights);

  if  (ExtraValues)       WriteXmlArrayFloat ("ExtraValues", o, MaxUnits, ExtraValues);

  //>if  (example_weight)    WriteXmlArrayFloat ("example_weight", o, NTrainingPatterns, example_weight);

  //>if  (ValuesCache)       WriteXmlArrayFloat2D ("ValuesCache", o, MaxCases, MaxUnits, ValuesCache);
  //>if  (ErrorsCache)       WriteXmlArrayFloat2D ("ErrorsCache", o, MaxCases, Noutputs, ErrorsCache);

  if  (Outputs)           WriteXmlArrayFloat ("Outputs", o, Noutputs, Outputs);

  if  (OutputWeights)     WriteXmlArrayFloat2D ("OutputWeights",    o, Noutputs, MaxUnits, OutputWeights);
  //>if  (OutputDeltas)      WriteXmlArrayFloat2D ("OutputDeltas",     o, Noutputs, MaxUnits, OutputDeltas);
  //>if  (OutputSlopes)      WriteXmlArrayFloat2D ("OutputSlopes",     o, Noutputs, MaxUnits, OutputSlopes);
  //>if  (OutputPrevSlopes)  WriteXmlArrayFloat2D ("OutputPrevSlopes", o, Noutputs, MaxUnits, OutputPrevSlopes);


  // errors needs to be declared butr not allocated;  it is used to point to vector of interest when needed.

  if  (ExtraErrors)    WriteXmlArrayFloat ("ExtraErrors",   o, Noutputs, ExtraErrors);
  //>if  (CandValues)     WriteXmlArrayFloat ("CandValues",    o, Noutputs, CandValues);
  //>if  (CandSumValues)  WriteXmlArrayFloat ("CandSumValues", o, Noutputs, CandSumValues);

  //>if  (CandCor)         WriteXmlArrayFloat2D ("CandCor",        o, Ncandidates, Noutputs, CandCor);
  //>if  (CandPrevCor)     WriteXmlArrayFloat2D ("CandPrevCor",    o, Ncandidates, Noutputs, CandPrevCor);
  //>if  (CandWeights)     WriteXmlArrayFloat2D ("CandWeights",    o, Ncandidates, MaxUnits, CandWeights);
  //>if  (CandDeltas)      WriteXmlArrayFloat2D ("CandDeltas",     o, Ncandidates, MaxUnits, CandDeltas);
  //>if  (CandSlopes)      WriteXmlArrayFloat2D ("CandSlopes",     o, Ncandidates, MaxUnits, CandSlopes);
  //>if  (CandPrevSlopes)  WriteXmlArrayFloat2D ("CandPrevSlopes", o, Ncandidates, MaxUnits, CandPrevSlopes);

  o << "</UsfCasCor>" << endl;
}  /* WriteXML */


void  SkipToStartOfTag (istream& i)
{
  char  ch = i.peek ();
  while  ((ch != '<')  &&  (!i.eof ()))
  {
    i.get ();
    ch = i.peek ();
  }
}  /* SkipToStartOfTag */


void  SkipWhiteSpace (istream& i)
{
  while ((!i.eof ())  &&  (strchr (" \n\t\r", i.peek ()) != NULL))
    i.get ();
}  /* SkipWhiteSpace */


/**
 *@details  Will return token less leading and trailing white space.  The chracter in 
 * 'del' that ended the token field will be consumed.  If the start of a tag ('<') is 
 * detected will treat as end of line.
 *@returns  The delimiter chracter that terminated the token; or 0 of end of file or
 * start of new tag detected.
 */
char  ReadNextToken (istream&    i,
                     KKStr&      token,
                     const char* del
                    )
{
  token = "";
  SkipWhiteSpace (i);

  char  nextCh = i.peek ();
  while  ((!i.eof ())  &&  (nextCh != '<')  &&  (strchr (del, nextCh) == NULL))
  {
    token.Append (nextCh);
    i.get ();
    nextCh = i.peek ();
  }

  if  (i.eof ())
    nextCh = 0;
  else if  (nextCh != '<')
    i.get ();

  token.TrimRight ();
  return  nextCh;
}  /* ReadNextToken */





XmlTagPtr  ReadFloatArrayUntilEndTag (istream&  i,
                                      float*    A,
                                      int       len
                                     )
{
  bool  eof = false;
  bool  eol = false;
  int  nextIdx = 0;

  XmlTagPtr  tag = NULL;

  KKStr  field (20);

  SkipWhiteSpace (i);
  while  (!i.eof ()  &&  (i.peek () != '<'))
  {
    ReadNextToken (i, field, ",");
    if  (nextIdx < len)
    {
      A[nextIdx] = field.ToFloat ();
      ++nextIdx;
    }
    SkipWhiteSpace (i);
  }

  while  (nextIdx < len)
  {
    A[nextIdx] = 0.0f;
    ++nextIdx;
  }

  if  (!i.eof ())
    tag = new XmlTag (i);

  return tag;
}  /* ReadFloatArrayUntilEndTag */



XmlTagPtr  ReadIntArrayUntilEndTag (istream&  i,
                                    int*      A,
                                    int       len
                                   )
{
  int  nextIdx = 0;

  XmlTagPtr  tag = NULL;

  KKStr  field (20);

  SkipWhiteSpace (i);
  while  (!i.eof ()  &&  (i.peek () != '<'))
  {
    ReadNextToken (i, field, ",");
    if  (nextIdx < len)
    {
      A[nextIdx] = field.ToInt ();
      ++nextIdx;
    }
    SkipWhiteSpace (i);
  }

  while  (nextIdx < len)
  {
    A[nextIdx] = 0;
    ++nextIdx;
  }

  if  (!i.eof ())
    tag = new XmlTag (i);

  return tag;
}  /* ReadIntArrayUntilEndTag */



void  UsfCasCor::ReadXmlArrayFloat (XmlTagPtr  tag, 
                                    istream&   i
                                   )
{
  float*  A = NULL;

  const KKStr&  name = tag->AttributeValue ("Name");

  kkint32  len = tag->AttributeValue ("Len").ToInt32 ();
  if  ((len < 1)  ||  (len > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat  Invalid Array Len[" << len << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }

  A = new float[len];

  XmlTagPtr  endTag = ReadFloatArrayUntilEndTag (i, A, len);

  if       (name.EqualIgnoreCase ("SumErrors"))       {delete  SumErrors;       SumErrors      = A;}
  else if  (name.EqualIgnoreCase ("DummySumErrors"))  {delete  DummySumErrors;  DummySumErrors = A;}
  else if  (name.EqualIgnoreCase ("example_weight"))  {delete  example_weight;  example_weight = A;}
  else if  (name.EqualIgnoreCase ("Outputs"))         {delete  Outputs;         Outputs        = A;}
  else if  (name.EqualIgnoreCase ("CandValues"))      {delete  CandValues;      CandValues     = A;}
  else if  (name.EqualIgnoreCase ("CandSumValues"))   {delete  CandSumValues;   CandSumValues  = A;}
  else if  (name.EqualIgnoreCase ("ExtraValues"))     {delete  ExtraValues;     ExtraValues    = A;}
  else if  (name.EqualIgnoreCase ("ExtraErrors"))     {delete  ExtraErrors;     ExtraErrors    = A;}

  else
  {
    log.Level (-1) << endl 
      << "UsfCasCor::ReadXmlArrayFloat   ***ERROR***  Unrecognized variable[" << name << "]" << endl
      << endl;
  }

  delete  endTag;
  endTag = NULL;
}  /* ReadXmlArrayFloat */



void  UsfCasCor::ReadXmlArrayFloat2D (XmlTagPtr  tag, 
                                      istream&   i
                                     )
{
  const KKStr&  name = tag->AttributeValue ("Name");

  kkint32  height = tag->AttributeValue ("height").ToInt32 ();
  kkint32  width  = tag->AttributeValue ("width").ToInt32 ();
  if  ((height < 1)  ||  (height > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat2D  Invalid Height[" << height << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }

  if  ((width < 1)  ||  (width > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat2D  Invalid Width[" << width << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }

  log.Level (10) << "Reading [" << name << "]  height[" << height << "]  width[" << width << "]." << endl;

  float**  A = new float*[height];

  int  rowIdx = 0;
  while  (!i.eof ())
  {
    SkipToStartOfTag (i);
    if  (i.eof ())
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  Input data exhausted prematurly." << endl
        << "                                name[" << name << "]" << endl
        << endl;
      break;
    }

    XmlTagPtr  rowTag = new XmlTag (i);
    if  ((rowTag->Name ().EqualIgnoreCase ("ArrayFloat2D"))  &&  (rowTag->TagType () == XmlTag::tagEnd))
    {
      // We have reached the end of the 2D Matrix
      delete  rowTag;
      rowTag = NULL;
      break;
    }

    if  ((rowTag->TagType () != XmlTag::tagStart)  ||  (!(rowTag->Name ().EqualIgnoreCase ("Row"))))
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  2D Float array not formatted as expected." << endl
        << "                                name[" << name << "]" << endl
        << endl;
      delete  rowTag;
      rowTag = NULL;
      break;
    }

    if  (rowIdx >= height)
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  Rows[" << rowIdx << "] exceeds expected height[" << height << "] rows." << endl
        << "                                name[" << name << "]" << endl
        << endl;
    }
    else
    {
      A[rowIdx] = new float[width];
      XmlTagPtr  endRowTag = ReadFloatArrayUntilEndTag (i, A[rowIdx], width);
      delete  endRowTag;  
      endRowTag = NULL;
    }
     
    delete  rowTag;
    rowTag = NULL;
    ++rowIdx;
  }

  while  (rowIdx < height)
  {
    A[rowIdx] = new float[width];
    for  (int col = 0;  col < width;  ++col)
      A[rowIdx][col] = 0.0f;
    ++rowIdx;
  }

  if       (name.EqualIgnoreCase ("TrainingInputs"))     TrainingInputs    = A;
  else if  (name.EqualIgnoreCase ("TrainingOutputs"))    TrainingOutputs   = A;
  else if  (name.EqualIgnoreCase ("ValuesCache"))        ValuesCache       = A;
  else if  (name.EqualIgnoreCase ("ErrorsCache"))        ErrorsCache       = A;
  else if  (name.EqualIgnoreCase ("OutputWeights"))      OutputWeights     = A;
  else if  (name.EqualIgnoreCase ("OutputDeltas"))       OutputDeltas      = A;
  else if  (name.EqualIgnoreCase ("OutputSlopes"))       OutputSlopes      = A;
  else if  (name.EqualIgnoreCase ("OutputPrevSlopes"))   OutputPrevSlopes  = A;
  else if  (name.EqualIgnoreCase ("CandCor"))            CandCor           = A;
  else if  (name.EqualIgnoreCase ("CandPrevCor"))        CandPrevCor       = A;
  else if  (name.EqualIgnoreCase ("CandWeights"))        CandWeights       = A;
  else if  (name.EqualIgnoreCase ("CandDeltas"))         CandDeltas        = A;
  else if  (name.EqualIgnoreCase ("CandSlopes"))         CandSlopes        = A;
  else if  (name.EqualIgnoreCase ("CandPrevSlopes"))     CandPrevSlopes    = A;
  else
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat2D   ***ERROR***    Unrecognized Rariable Name[" << name << "]" << endl
      << endl;
    for  (int x = 0;  x < height;  ++x)
      delete  A[x];
    delete A;
    A = NULL;
  }
}  /* ReadXmlArrayFloat2D */




void  UsfCasCor::ReadXmlArrayFloat2DVarying (XmlTagPtr  tag, 
                                             istream&   i
                                            )
{
  const KKStr&  name = tag->AttributeValue ("Name");

  kkint32  height = tag->AttributeValue ("height").ToInt32 ();
  if  ((height < 1)  ||  (height > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat2D  Invalid Height[" << height << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }

  float**  A = new float*[height];

  int  rowIdx = 0;
  while  (!i.eof ())
  {
    SkipToStartOfTag (i);
    if  (i.eof ())
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  Input data exhausted prematurly." << endl
        << "                                name[" << name << "]" << endl
        << endl;
      break;
    }

    XmlTagPtr  rowTag = new XmlTag (i);
    if  ((rowTag->Name ().EqualIgnoreCase ("ArrayFloat2DVarying"))  &&  (rowTag->TagType () == XmlTag::tagEnd))
    {
      // We have reached the end of the 2D Matrix
      delete  rowTag;
      rowTag = NULL;
      break;
    }

    if  ((rowTag->TagType () != XmlTag::tagStart)  ||  (!(rowTag->Name ().EqualIgnoreCase ("Row"))))
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  2D Float array not formatted as expected." << endl
        << "                                name[" << name << "]" << endl
        << endl;
      delete  rowTag;
      rowTag = NULL;
      break;
    }

    if  (rowIdx >= height)
    {
      log.Level (-1) << endl 
        << "UsfCasCor::ReadXmlArrayFloat2D  ***ERROR***  Rows[" << rowIdx << "] exceeds expected height[" << height << "] rows." << endl
        << "                                name[" << name << "]" << endl
        << endl;
    }
    else
    {
      kkint32  width  = rowTag->AttributeValue ("width").ToInt32 ();
      if  ((width < 0)  ||  (width > 100000000))
      {
        log.Level (-1) << endl
           << "UsfCasCor::ReadXmlArrayFloat2D  Invalid Width[" << width << "]  for variable[" << name << "]" << endl
           << endl;
        width = 1;
      }

      if  (width > 0)
        A[rowIdx] = new float[width];
      else
        A[rowIdx] = NULL;

      XmlTagPtr  endRowTag = ReadFloatArrayUntilEndTag (i, A[rowIdx], width);
      delete  endRowTag;  
      endRowTag = NULL;
    }
     
    delete  rowTag;
    rowTag = NULL;
    ++rowIdx;
  }

  while  (rowIdx < height)
  {
    A[rowIdx] = NULL;
    ++rowIdx;
  }

  if  (name.EqualIgnoreCase ("Weights"))
    Weights = A;
  else
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayFloat2D   ***ERROR***    Unrecognized Rariable Name[" << name << "]" << endl
      << endl;
    for  (int x = 0;  x < height;  ++x)
      delete A[x];
    delete A;
    A = NULL;
  }
}  /* ReadXmlArrayFloat2DVarying */



void  UsfCasCor::ReadXmlArrayInt (XmlTagPtr  tag, 
                                  istream&   i
                                 )
{
  int*  A = NULL;

  const KKStr&  name = tag->AttributeValue ("Name");
  kkint32  len = tag->AttributeValue ("Len").ToInt32 ();
  if  ((len < 1)  ||  (len > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayInt  Invalid Array Len[" << len << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }

  A = new int[len];

  XmlTagPtr  endTag = ReadIntArrayUntilEndTag (i, A, len);

  if       (name.EqualIgnoreCase ("feature_type"))    {delete  feature_type;    feature_type   = A;}
  else if  (name.EqualIgnoreCase ("AllConnections"))  {delete  AllConnections;  AllConnections = A;}
  else if  (name.EqualIgnoreCase ("Nconnections"))    {delete  Nconnections;    Nconnections   = A;}
  else
  {
    log.Level (-1) << endl 
      << "UsfCasCor::ReadXmlArrayInt   ***ERROR***  Unrecognized variable[" << name << "]" << endl
      << endl;
    delete  A;
    A = NULL;
  }

  delete  endTag;
  endTag = NULL;
}  /* ReadXmlArrayInt */



void  UsfCasCor::ReadXmlConnections (XmlTagPtr  tag, 
                                     istream&   i
                                    )
{
  delete  Connections; Connections = NULL;

  const KKStr&  name = tag->AttributeValue ("Name");
  kkint32  len = tag->AttributeValue ("Len").ToInt32 ();
  if  ((len < 1)  ||  (len > 100000000))
  {
    log.Level (-1) << endl
      << "UsfCasCor::ReadXmlArrayInt  Invalid Array Len[" << len << "]  for variable[" << name << "]" << endl
      << endl;
    return;
  }
  
  Connections = new int*[len];

  int        nextIdx = 0;
  XmlTagPtr  endTag  = NULL;

  KKStr  field (20);

  SkipWhiteSpace (i);
  while  (!i.eof ()  &&  (i.peek () != '<'))
  {
    ReadNextToken (i, field, ",");
    if  (nextIdx < len)
    {
      if  (field.EqualIgnoreCase ("AC"))
        Connections[nextIdx] = AllConnections;
      else 
        Connections[nextIdx] = NULL;

      ++nextIdx;
    }
    SkipWhiteSpace (i);
  }

  while  (nextIdx < len)
  {
    Connections[nextIdx] = NULL;
    ++nextIdx;
  }

  if  (!i.eof ())
    endTag = new XmlTag (i);

  delete  endTag;
  endTag = NULL;
}  /* ReadXmlConnections */




void  UsfCasCor::ReadXmlNameValueLine (istream&  i)
{
  bool  eof = false;
  KKStr  line =   osReadRestOfLine (i, eof);
  if  (eof == true)
    return;

  line.TrimLeft ();
  KKStr  fieldName = line.ExtractToken2 ("\t");
  line.TrimLeft ();
  line.TrimRight ();

  if  (fieldName.EqualIgnoreCase ("progname"))
  {
    log.Level(50) << "progname: " << line << endl;
  }

  else if  (fieldName.EqualIgnoreCase ("number_of_classes"))
    number_of_classes = line.ToInt ();

  else if  (fieldName.EqualIgnoreCase ("classes"))
  {
    delete  classes;
    classes = MLClassList::BuildListFromDelimtedStr (line, ',');
  }

  else if  (fieldName.EqualIgnoreCase ("selectedFeatures"))
  {
    bool  valid = true;
    delete selectedFeatures;
    selectedFeatures = new FeatureNumList (fileDesc, line, valid);
  }

  // ints
  else if  (fieldName.EqualIgnoreCase ("MaxUnits"))                 MaxUnits                = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Nunits"))                   Nunits                  = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Ninputs"))                  Ninputs                 = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Noutputs"))                 Noutputs                = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Ncandidates"))              Ncandidates             = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("MaxCases"))                 MaxCases                = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Ncases"))                   Ncases                  = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("FirstCase"))                FirstCase               = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("line_length"))              line_length             = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("the_random_seed"))          the_random_seed         = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("in_limit"))                 in_limit                = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("out_limit"))                out_limit               = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("number_of_trials"))         number_of_trials        = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("number_of_rounds"))         number_of_rounds        = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("normalization_method"))     normalization_method    = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("my_mpi_rank"))              my_mpi_rank             = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("NTrainingPatterns"))        NTrainingPatterns       = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("NTestPatterns"))            NTestPatterns           = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("ErrorMisclassifications"))  ErrorMisclassifications = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("OutputPatience"))           OutputPatience          = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("InputPatience"))            InputPatience           = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("ErrorBits"))                ErrorBits               = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("BestCandidate"))            BestCandidate           = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Epoch"))                    Epoch                   = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("Trial"))                    Trial                   = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("NtrainingOutputValues"))    NtrainingOutputValues   = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("NtestOutputValues"))        NtestOutputValues       = line.ToInt ();
  else if  (fieldName.EqualIgnoreCase ("ErrorMeasure"))             ErrorMeasure            = line.ToInt ();

  // Floats
  else if  (fieldName.EqualIgnoreCase ("SigmoidMax"))            SigmoidMax            = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("WeightRange"))           WeightRange           = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("SigmoidPrimeOffset"))    SigmoidPrimeOffset    = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("WeightMultiplier"))      WeightMultiplier      = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("OutputMu"))              OutputMu              = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("OutputShrinkFactor"))    OutputShrinkFactor    = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("OutputEpsilon"))         OutputEpsilon         = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("OutputDecay"))           OutputDecay           = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("OutputChangeThreshold")) OutputChangeThreshold = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("InputMu"))               InputMu               = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("InputShrinkFactor"))     InputShrinkFactor     = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("InputEpsilon"))          InputEpsilon          = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("InputDecay"))            InputDecay            = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("InputChangeThreshold"))  InputChangeThreshold  = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("TrueError"))             TrueError             = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("ScoreThreshold"))        ScoreThreshold        = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("SumSqError"))            SumSqError            = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("BestCandidateScore"))    BestCandidateScore    = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("TrainingStdDev"))        TrainingStdDev        = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("TestStdDev"))            TestStdDev            = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("ErrorIndex"))            ErrorIndex            = line.ToFloat ();
  else if  (fieldName.EqualIgnoreCase ("ErrorIndexThreshold"))   ErrorIndexThreshold   = line.ToFloat ();

  else if  (fieldName.EqualIgnoreCase ("UnitType"))     UnitType   = string_to_type (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("OutputType"))   OutputType = string_to_type (line.Str ());

  // Boolean
  else if  (fieldName.EqualIgnoreCase ("load_weights"))             load_weights            = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("UseCache"))                 UseCache                = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("Graphics"))                 Graphics                = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("NonRandomSeed"))            NonRandomSeed           = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("Test"))                     Test                    = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("SinglePass"))               SinglePass              = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("SingleEpoch"))              SingleEpoch             = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("Step"))                     Step                    = string_to_boolean (line.Str ());
  else if  (fieldName.EqualIgnoreCase ("InterruptPending"))         InterruptPending        = string_to_boolean (line.Str ());
  else
  {
    log.Level (-1) << endl 
      << "UsfCasCor::ReadXmlNameValueLine   ***ERROR***   Unrecognized Variable[" << fieldName << "]." << endl
      << endl;
  }
}  /* ReadXmlNameValueLine */





void  UsfCasCor::ReadXml (istream&  i,
                          bool&     valid
                         )
{
  valid = true;
  SkipWhiteSpace (i);
  char  nextCh = i.peek ();
  while  (!i.eof ())
  {
    if  (nextCh == '<')
    {
      // We have a tag field.
      XmlTagPtr  tag = new XmlTag (i);
      if  (tag->Name ().EqualIgnoreCase ("ArrayFloat"))
        ReadXmlArrayFloat (tag, i);

      else if  (tag->Name ().EqualIgnoreCase ("ArrayFloat2D"))
        ReadXmlArrayFloat2D (tag, i);

      else if  (tag->Name ().EqualIgnoreCase ("ArrayFloat2DVarying"))
        ReadXmlArrayFloat2DVarying (tag, i);

      else if  (tag->Name ().EqualIgnoreCase ("ArrayInt"))
        ReadXmlArrayInt (tag, i);

      else if  (tag->Name ().EqualIgnoreCase ("Connections"))
        ReadXmlConnections (tag, i);

      else if  (tag->Name ().EqualIgnoreCase ("UsfCasCor"))
      {
        if  (tag->TagType () == XmlTag::tagEnd)
        {
          // We have reached the end of the UsfCasCor part of this stream.
          delete  tag;
          tag = NULL;
          break;
        }
        else
        {
          log.Level (-1) << endl << "UsfCasCor::ReadXml   ***ERROR***   Impropper formated file." << endl;
          valid = false;
        }
      }

      delete  tag;
      tag = NULL;
    }
    else
    {
      // We have a Name/Valure pair line.
      ReadXmlNameValueLine (i);
    }

    SkipWhiteSpace (i);
    nextCh = i.peek ();
  }

  if  (NTestPatterns > 0)
  {
    TestInputs  = new float*[NTestPatterns];
    TestOutputs = new float*[NTestPatterns];
    for  (int z = 0; z < NTestPatterns; ++z)
    {
      TestInputs[z]  = new float[Ninputs];
      TestOutputs[z] = new float[Noutputs];
    }
  }
  else
  {
    TestInputs  = TrainingInputs;
    TestOutputs = TrainingOutputs;
  }

  if  (!ExtraValues)
    ExtraValues = new float[MaxUnits];

  if  (!Values)
    Values = ExtraValues;


  if  (UseCache) 
  {
     /* Odd error check. If usecache was specified in file, but not on
      * command line, then Cache was not allocated. We look for NULL
      * value and allocate storage here. 
      */
     if  (ValuesCache == NULL )
     {
       ValuesCache = new float*[MaxCases];     //(float **)CALLOC(MaxCases, sizeof(float *));
       ErrorsCache = new float*[MaxCases];

       for  (kkint32 i = 0;  i < MaxCases;  i++)
       {
         ValuesCache[i] = new float[MaxUnits];
         ErrorsCache[i] = new float[Noutputs];
       }
     }


    for  (kkint32 i = 0;  i < NTrainingPatterns;  i++)
    {
      Values = ValuesCache[i];
      
      /* Unit values must be calculated in order because the activations */
      /* cascade down through the hidden layers */
      for  (kkint32 j = 1 + Ninputs;  j < Nunits;  j++) 
         COMPUTE_UNIT_VALUE(j);
    }
  }
}  /* ReadXml */





//******************************************************************************************
//*                                   load_namesfile.c                                     *
//******************************************************************************************
void  UsfCasCor::load_namesfile (FeatureVectorListPtr  trainExamples,
                                 FeatureNumListPtr     selectedFeatures
                                )
{
  /* First, ensure the necessary variables are reset */
  NTrainingPatterns = -1;
  NTestPatterns     = -1;

  if  (!classes)
    classes = trainExamples->ExtractListOfClasses ();
  classes->SortByName ();
  number_of_classes = classes->QueueSize ();

  NTrainingPatterns = trainExamples->QueueSize ();

  NTestPatterns = 1;  // We will be testing one example at a time.

  kkint32 feature_count = selectedFeatures->NumOfFeatures ();

  delete  feature_type;
  feature_type = new int[feature_count];
  for  (int i = 0;  i < feature_count;  ++i)
  {
    feature_type[i] = REAL;
  }

  Noutputs = number_of_classes;

  Ninputs = feature_count;

  return;
}  /* load_namesfile */




/***********************************************************************/
/*                             learn.c                                 */
/***********************************************************************/


/*
 * Given the sum of weighted inputs, compute the unit's activation value.
 * Defined unit types are SIGMOID, VARSIGMOID, and GAUSSIAN.
 */
float  UsfCasCor::ACTIVATION (float sum)
{
  float temp;

  switch (UnitType)
  {
  case SIGMOID: 
    /* Sigmoid function in range -0.5 to 0.5. */
    if  (sum < -15.0)  
      return(-0.5f);
    
    else if  (sum >  15.0)
      return(0.5f);

    else
      return (1.0f / (1.0f + exp (-sum)) - 0.5f);

  case GAUSSIAN:
    /* Gaussian activation function in range 0.0 to 1.0. */
    temp = -0.5f * sum * sum;
    if  (temp < -75.0f)
      return  (0.0f);
    else 
      return (exp (temp));

  case ASYMSIGMOID: 
    /* asymmetrical sigmoid function in range 0.0 to 1.0. */
    if  (sum < -15.0f) 
      return (0.0f);
    else if  (sum > 15.0f) 
      return (1.0f);
    else 
      return (1.0f  / (1.0f + exp(-sum)));


  case VARSIGMOID: 
    /* Sigmoid function in range SigmoidMin to SigmoidMax. */
    if  (sum < -15.0) 
      return (SigmoidMin);

    else if   (sum > 15.0f) 
      return (SigmoidMax);
    
    else 
      return ((SigmoidMax - SigmoidMin)/ (1.0f + exp(-sum)) + SigmoidMin);
  }
  return -1;
}  /* ACTIVATION */




/*
 * Given the unit's activation value and sum of weighted inputs, compute
 * the derivative of the activation with respect to the sum.  Defined unit
 * types are SIGMOID, VARSIGMOID, and GAUSSIAN.
 *
 * Note: do not use sigmoid prime offset here, as it confuses the
 * correlation machinery.  But do use it in output-prime.
 * 
 */
float  UsfCasCor::ACTIVATION_PRIME (float value, 
                                    float sum
                                   )
{
  switch(UnitType)
  {
  case SIGMOID: 
    /* Symmetrical sigmoid function. */
    return (0.25f -  value * value);

  case GAUSSIAN:
    /* Gaussian activation function. */
    return (sum * (- value));

  case ASYMSIGMOID: 
    /* asymmetrical sigmoid function in range 0.0 to 1.0. */
    return (value * (1.0f - value));

  case VARSIGMOID: 
    /* Sigmoid function with range SigmoidMin to SigmoidMax. */
    return ((value - SigmoidMin) * (1.0f - (value - SigmoidMin) / 
				    (SigmoidMax - SigmoidMin)));

  }
  return -1.0f;
}  /* ACTIVATION_PRIME */




/* Compute the value of an output, given the weighted sum of incoming values.
 * Defined output types are SIGMOID, ASYMSIGMOID, and LINEAR.
 */
float  UsfCasCor::OUTPUT_FUNCTION (float sum)
{
  switch (OutputType)
  {
  case SIGMOID: 
    /* Symmetrical sigmoid function, used for binary functions. */
    if  (sum < -15.0) 
      return (-0.5f);

    else if  (sum > 15.0f) 
      return (0.5f);
    
    else 
      return (1.0f / (1.0f + exp (-sum)) - 0.5f);

  case LINEAR:
    /* Linear output function, used for continuous functions. */
    return (sum);

  case ASYMSIGMOID: 
    /* asymmetrical sigmoid function in range 0.0 to 1.0. */
    if  (sum < -15.0f) 
      return (0.0f);

    else if  (sum > 15.0f) 
      return (1.0f);

    else 
      return (1.0f / (1.0f + exp (-sum)));

  case VARSIGMOID:
    /* Sigmoid function in range SigmoidMin to SigmoidMax. */
    if  (sum < -15.0f) 
      return (SigmoidMin);

    else if  (sum > 15.0f) 
      return(SigmoidMax);

    else 
      return ((SigmoidMax - SigmoidMin) / (1.0f + exp (-sum)) + SigmoidMin);
  }
  return -1.0f;
}  /* OUTPUT_FUNCTION */




/* Compute the value of an output, given the weighted sum of incoming values.
 * Defined output types are SIGMOID, ASYMSIGMOID, and LINEAR.
 *
 * Sigmoid_Prime_Offset used to keep the back-prop error value from going to 
 * zero.
 */
float  UsfCasCor::OUTPUT_PRIME (float output)
{
  switch(OutputType)
  {
  case SIGMOID: 
    /* Symmetrical sigmoid function, used for binary functions. */
    return (SigmoidPrimeOffset + 0.25f -  output * output);

  case LINEAR:
    /* Linear output function, used for continuous functions. */
    return (1.0);

  case ASYMSIGMOID: 
    /* asymmetrical sigmoid function in range 0.0 to 1.0. */
    return (SigmoidPrimeOffset + output * (1.0f - output));

  case VARSIGMOID: 
    /* Sigmoid function with range SigmoidMin to SigmoidMax. */
    return (SigmoidPrimeOffset + 
	    (output - SigmoidMin) * (1.0f - (output - SigmoidMin) / 
				     (SigmoidMax - SigmoidMin)));
  }

  return -1.0f;
}  /* OUTPUT_PRIME */




/* The basic routine for doing quickprop-style update of weights, given a
 * pair of slopes and a delta.
 *
 * Given arrays holding weights, deltas, slopes, and previous slopes,
 * and an index i, update weight[i] and delta[i] appropriately.  Move
 * slope[i] to prev[i] and zero out slope[i].  Add weight decay term to
 * each slope before doing the update.
 */
void  UsfCasCor::QUICKPROP_UPDATE (int i, 
                                   float weights[], 
                                   float deltas[], 
                                   float slopes[], 
		                               float prevs[], 
                                   float epsilon,
                                   float decay,
                                   float mu,
                                   float shrink_factor
                                  )
{
  float w,d,s,p, next_step;
  /********/

  w = weights[i];
  d = deltas[i];
  s = slopes[i] +  decay * w;
  p = prevs[i];
  next_step = 0.0f;

  /* The step must always be in direction opposite to the slope. */

  if(d < 0.0f){			
    /* If last step was negative...  */  
    if(s > 0.0f)	  
      /*  Add in linear term if current slope is still positive.*/
      next_step -= epsilon * s;
    /*If current slope is close to or larger than prev slope...  */
    if(s >= (shrink_factor*p)) 
      next_step += mu * d;	/* Take maximum size negative step. */
    else
      next_step += d * s / (p - s); /* Else, use quadratic estimate. */
  }
  else if(d > 0.0f){
    /* If last step was positive...  */
    if(s < 0.0f)	  
      /*  Add in linear term if current slope is still negative.*/
      next_step -= epsilon * s;
    /* If current slope is close to or more neg than prev slope... */
    if(s <= (shrink_factor*p)) 
      next_step += mu * d;	/* Take maximum size negative step. */
    else
      next_step += d * s / (p - s); /* Else, use quadratic estimate. */
  }
  else
    /* Last step was zero, so use only linear term. */
    next_step -= epsilon * s;
  
  /* update global data arrays */
  deltas[i] = next_step;
  weights[i] = w + next_step;
  prevs[i] = s;
  slopes[i] = 0.0;
}  /* QUICKPROP_UPDATE */




/* Set up all the inputs from the INPUT vector as the first few entries in
   in the values vector.
*/
void  UsfCasCor::SETUP_INPUTS (float inputs[])
{
  int i;
  /*********/

  Values[0] = 1.0;		/* bias unit */
  for(i=0;  i < Ninputs;  i++)
    Values[i+1] = inputs[i];
}




/* Assume the values vector has been set up.  Just compute the output
   values.
*/
void  UsfCasCor::OUTPUT_FORWARD_PASS ()
{
  int i,j;
  float sum;
  float *ow;
/********/

  for  (j = 0;  j < Noutputs;  j++)
  {
    sum = 0.0;
    ow  = OutputWeights[j];

    for(i=0; i<Nunits; i++)
      sum += Values[i] * ow[i];

#ifdef CONNX
      conx += Nunits;
#endif

    Outputs[j] = OUTPUT_FUNCTION(sum);
  }
}  /* OUTPUT_FORWARD_PASS */




/* Assume that values vector has been set up for units with index less
   than J.  Compute and record the value for unit J.
*/
void  UsfCasCor::COMPUTE_UNIT_VALUE (int j)
{
  int   i;
  int   *c;		/* pointer to unit's connections array */
  float *w,		/* pointer to unit's weights array*/
        sum = 0.0;
/********/

  c = Connections[j];
  w = Weights[j];

  for  (i = 0;  i < Nconnections[j];  i++)
    sum += Values[c[i]] * w[i];

#ifdef CONNX
    conx += Nconnections[j];
#endif

  Values[j] = ACTIVATION (sum);
}  /* COMPUTE_UNIT_VALUE */




/* Set up the inputs from the INPUT vector, then propagate activation values
   forward through all hidden units and output units.
*/
void  UsfCasCor::FULL_FORWARD_PASS (float input[])
{
  int j;
  /********/

  SETUP_INPUTS (input);

  /* Unit values must be calculated in order because the activations */
  /* cascade down through the hidden layers */

  for  (j = 1 + Ninputs;  j < Nunits;  j++) /* For each hidden unit J, compute the */
    COMPUTE_UNIT_VALUE (j);	                /* activation value.                   */

  OUTPUT_FORWARD_PASS ();	/* Now compute outputs. */
}  /* FULL_FORWARD_PASS */




/*  Goal is a vector of desired values for the output units.  Compute and
 *  record the output errors for the current training case.  Record error
 *  values and related statistics.  If output_slopesp is TRUE, then use errors
 *  to compute slopes for output weights.  If statsp is TRUE, accumulate error
 *  statistics. 
 */
void  UsfCasCor::COMPUTE_ERRORS (float    goal[], 
                                 Boolean  output_slopesp, 
                                 Boolean  statsp, 
                                 int      xw
                                )
{
  int     i;
  int     j;
  float   out       = 0.0f;
  float   dif       = 0.0f;
  float   err_prime = 0.0f;
  float*  os        = NULL;  /* pointer to unit's output slopes array */

  int     goal_winner;
  int     output_winner;

  goal_winner   = 0;
  output_winner = 0;

  for  (i = 1;  i < Noutputs;  i++)
  {
    if ( Outputs[output_winner] < Outputs[i]) 
      output_winner=i;

    if ( goal[goal_winner] < goal[i] )
      goal_winner=i;
  }

  if  (goal_winner != output_winner) 
    ErrorMisclassifications++;

  for  (j = 0;  j < Noutputs;  j++)
  {
    out = Outputs[j];
    dif = out - goal[j];
    if  (load_weights  &&  xw >= 0  &&  example_weight[xw] != 1.0 ) 
      dif *= example_weight[xw];

    err_prime = dif * OUTPUT_PRIME(out);
    os = OutputSlopes[j];

    Errors[j] = err_prime;

    if  (statsp)
    {
      if  (fabs(dif) > ScoreThreshold)
        ErrorBits++;
      TrueError += dif * dif;
      SumErrors[j] += err_prime;
      SumSqError += err_prime * err_prime;
    }

    if  (output_slopesp)
    {
      for  (i = 0;  i < Nunits;  i++)
        os[i] += err_prime * Values[i];
    }
  }				/* end for unit j */

}  /* COMPUTE_ERRORS */




/* Update the output weights, using the pre-computed slopes, prev-slopes,
 * and delta values.
 */
void  UsfCasCor::UPDATE_OUTPUT_WEIGHTS ()
{
  int i,j;
  float eps;			/* epsilon scaled by fan-in */
/********/

  eps = OutputEpsilon / Ncases;

  for  (j = 0;  j < Noutputs;  j++)
    for  (i = 0;  i < Nunits;  i++)
      QUICKPROP_UPDATE (i, 
                        OutputWeights[j], 
                        OutputDeltas[j],
                        OutputSlopes[j], 
                        OutputPrevSlopes[j], 
                        eps,
                        OutputDecay, 
                        OutputMu, 
                        OutputShrinkFactor
                       );

}




/***********************************************************************/
/*                                                                     */
/* The outer loops for training output weights.                        */
/*                                                                     */
/***********************************************************************/


/* Perform forward propagation once for each set of weights in the
 * training vectors, computing errors and slopes.  Then update the output
 * weights.
 */
void  UsfCasCor::TRAIN_OUTPUTS_EPOCH ()
{
  int i;
/********/

  /* zero error accumulators */
  ErrorBits = 0;
  TrueError = 0.0;
  ErrorMisclassifications = 0;
  for  (i = 0;  i < Noutputs;  i++)
  {
    SumErrors[i] = 0.0;
  }
  SumSqError = 0.0;

  /* User may have changed mu between epochs, so fix shrink-factor. */
  OutputShrinkFactor = OutputMu / (1.0f + OutputMu);

  for  (i= FirstCase; i < (FirstCase+Ncases); i++)
  {
    Goal = TrainingOutputs[i];

    if  (UseCache)
    {
      Values = ValuesCache[i];
      Errors = ErrorsCache[i];
      OUTPUT_FORWARD_PASS();
    }
    else
    {
      Values = ExtraValues;
      Errors = ExtraErrors;
      FULL_FORWARD_PASS(TrainingInputs[i]);
    }
    COMPUTE_ERRORS (Goal, True, True, i);
  }
 
  switch (ErrorMeasure)
  {
  case BITS:
    /* Do not change weights or count epoch if this run was a winner. */
    if  (ErrorBits > 0)
    {
      UPDATE_OUTPUT_WEIGHTS();
      Epoch++;
    }
    break;

  case INDEX:
    /* Compute index and don't change weights if we have a winner. */
    ErrorIndex = ERROR_INDEX(TrainingStdDev, NtrainingOutputValues);
    if  (ErrorIndex > ErrorIndexThreshold)
    {
      UPDATE_OUTPUT_WEIGHTS();
      Epoch++;
    }
    break;
  }
   
}  /* TRAIN_OUTPUTS_EPOCH */





/* Train the output weights.  If we exhaust max_epochs, stop with value
 * TIMEOUT.  If there are zero error bits, stop with value WIN.  Else,
 * keep going until the true error has changed by a significant amount,
 * and then until it does not change significantly for Patience epochs.
 * Then return STAGNANT.  If Patience is zero, we do not stop until victory
 * or until max_epochs is used up.
 */

int  UsfCasCor::TRAIN_OUTPUTS (int max_epochs)
{
  int i;
  int retval = TIMEOUT;	  /* will be reset within loop for other conditions */
  float last_error = 0.0;
  int quit_epoch = Epoch + OutputPatience;
  Boolean first_time = True;
/********/

  for(i = 0;  (i < max_epochs)  &&  (!cancelFlag);  ++i)
  {
    TRAIN_OUTPUTS_EPOCH ();

    if  ((ErrorMeasure == BITS) &&  (ErrorBits == 0))
    {
      retval = WIN;
      break;
    }
    
    else if  ((ErrorMeasure == INDEX) && (ErrorIndex <= ErrorIndexThreshold))
    {
      retval = WIN;
      break;
    }
    
    else if  (OutputPatience == 0)
      continue;			/* continue training until victory */
    
    else if  (first_time)
    {
      first_time = False;
      last_error = TrueError;
    }

    else if  (fabs(TrueError - last_error) > (last_error * OutputChangeThreshold))
    {
       /* still getting better */
      last_error = TrueError;
      quit_epoch = Epoch + OutputPatience;
    }

    else if  (Epoch >= quit_epoch)
    { 
      /* haven't gotten better for a while */
      retval = STAGNANT;
      break;
    }
  }

  /* tell user about the output weights of new unit */
  /*for(o=0; o<Noutputs; o++){
    fprintf(stderr,"  Output %d Weights: ", o);
    for(i=0; i<Nunits; i++)
      fprintf(stderr,"%6f ", OutputWeights[o][i]);
    fprintf(stderr,"\n");
  }
  */

  /* return result,  will be TIMEOUT unless reset in loop */
  return(retval);
}  /* TRAIN_OUTPUTS */






/***********************************************************************/
/*                                                                     */
/*  Machinery for Training and selecting candidate units.              */
/*                                                                     */
/***********************************************************************/

/* Give new random weights to all of the candidate units.  Zero the other
 * candidate-unit statistics.
 */
void  UsfCasCor::INIT_CANDIDATES ()
{
  int i,j,o;
/********/

  for  (i = 0;  i < Ncandidates;  i++)
  {
    CandValues[i] = 0.0;
    CandSumValues[i] = 0.0;

    for  (j = 0;  j < Nunits;  j++)
    {
      CandWeights[i][j] = random_weight();
      CandDeltas[i][j] = 0.0;
      CandSlopes[i][j] = 0.0;
      CandPrevSlopes[i][j] = 0.0;
    }
    
    for(o=0; o<Noutputs; o++)
    {
      CandCor[i][o] = 0.0;
      CandPrevCor[i][o] = 0.0;
    }
  }
}  /* INIT_CANDIDATES */




/* Add the candidate-unit with the best correlation score to the active
 * network.  Then reinitialize the candidate pool.
 */
void  UsfCasCor::INSTALL_NEW_UNIT ()
{
  int i,o;
  float wm;			/* temporary weight multiplier */
  float *w;			/* temporary weight array */
  float *cw;
/********/

  if  (Nunits >= MaxUnits)
  {
    log.Level (-1) << endl 
      << "UsfCasCor::INSTALL_NEW_UNIT   ***ERROR***   Can not add more units; limit of MaxUnits[" << MaxUnits << "] has been reached." << endl
      << endl;
    return;
  }

  Nconnections[Nunits] = Nunits;
  Connections[Nunits] = AllConnections;
  /* Set up the weight vector for the new unit. */
  w =  new float[Nunits];
  cw = CandWeights[BestCandidate];
  for  (i = 0;  i < Nunits;  i++)
    w[i] = cw[i];
  Weights[Nunits] = w;

  /* Tell user about the new unit. */
  //for(i=0; i<Nunits; i++)
  //  fprintf(stderr,"%6f ", Weights[Nunits][i]);
  //fprintf(stderr,"\n");

  /* Fix up output weights for candidate unit.  Use minus the           */
  /* correlation times the WeightMultiplier as an initial guess.        */

  if  (ErrorMeasure == BITS)
    wm = WeightMultiplier;
  else				/* ErrorMeasure == INDEX */
    wm = WeightMultiplier / (float)Nunits;

  for  (o = 0;  o < Noutputs;  o++)
    OutputWeights[o][Nunits] = -CandPrevCor[BestCandidate][o] * wm;

  /* If using cache, run an epoch to compute this unit's values.        */
  if  (UseCache)
    for  (i = 0;  i < NTrainingPatterns;  i++)
    {
      Values = ValuesCache[i];
      COMPUTE_UNIT_VALUE(Nunits);
    }

  /* Reinitialize candidate units with random weights.                  */
  Nunits++;
  INIT_CANDIDATES();    
}  /* INSTALL_NEW_UNIT*/







/* Note: Ideally, after each adjustment of the candidate weights, we would  */
/* run two epochs.  The first would just determine the correlations         */
/* between the candidate unit outputs and the residual error.  Then, in a   */
/* second pass, we would adjust each candidate's input weights so as to     */
/* maximize the absolute value of the correlation.  We need to know the     */
/* direction to tune the input weights.                                     */
/*                                                                          */
/* Since this ideal method doubles the number of epochs required for        */
/* training candidates, we cheat slightly and use the correlation values    */
/* computed BEFORE the most recent weight update.  This combines the two    */
/* epochs, saving us almost a factor of two.  To bootstrap the process, we  */
/* begin with a single epoch that computes only the correlation.            */
/*                                                                          */
/* Since we look only at the sign of the correlation after the first ideal  */
/* epoch and since that sign should change very infrequently, this probably */
/* is OK.  But keep a lookout for pathological situations in which this     */
/* might cause oscillation.                                                 */

/* For the current training pattern, compute the value of each candidate
 * unit and begin to compute the correlation between that unit's value and
 * the error at each output.  We have already done a forward-prop and
 * computed the error values for active units.
 */
void  UsfCasCor::COMPUTE_CORRELATIONS ()
{
  int i,o,u;
  float sum=0.0;
  float v=0.0;
/*********/

  for(u=0; u<Ncandidates; u++){
    sum = 0.0;
    v = 0.0;
    /* Determine activation value of each candidate unit. */
    for(i=0; i<Nunits; i++)
      sum += CandWeights[u][i] * Values[i];
#ifdef CONNX
    conx += Nunits;
#endif
    v = ACTIVATION(sum);
    CandValues[u] = v;
    CandSumValues[u] += v;
    /* Accumulate value of each unit times error at each output. */
    for(o=0; o<Noutputs; o++)
      CandCor[u][o] += v * Errors[o];
  }
}  /* COMPUTE_CORRELATIONS */





/* NORMALIZE each accumulated correlation value, and stuff the normalized
 * form into the CandPrevCor data structure.  Then zero CandCor to
 * prepare for the next round.  Note the unit with the best total
 * correlation score.
 */
void  UsfCasCor::ADJUST_CORRELATIONS ()
{
  int o,u;
  float cor, score;
  float *cc, *cpc;
  float avg_value;
/*********/

  BestCandidate = 0;
  BestCandidateScore = 0.0;
  for(u=0; u<Ncandidates; u++){
    avg_value = CandSumValues[u] / Ncases;
    cor = 0.0;
    score = 0.0;
    cc = CandCor[u];
    cpc = CandPrevCor[u];
    for(o=0; o<Noutputs; o++)
    {
      cor = (cc[o] - avg_value * SumErrors[o]) / SumSqError;
      cpc[o] = cor;
      cc[o] = 0.0;
      score += fabs(cor);
    }

    /* zero CandSumValues for next epoch */
    CandSumValues[u] = 0.0;
    /* Keep track of the candidate with the best overall correlation. */
    if(score > BestCandidateScore){
      BestCandidateScore = score;
      BestCandidate = u;
    }
  }
}  /* ADJUST_CORRELATIONS */





/* After the correlations have been computed, we do a second pass over
 * the training set and adjust the input weights of all candidate units.
 */
void  UsfCasCor::COMPUTE_SLOPES ()
{
  int i,o,u;
  float sum, value, actprime, direction, error, change;
/*********/

  for  (u=0; u<Ncandidates; u++)
  {
    sum = 0.0;
    value = 0.0;
    actprime = 0.0;
    direction = 0.0;
    change = 0.0;
    /* Forward pass through each candidate unit to compute activation-prime. */
    for(i=0; i<Nunits; i++)
      sum += CandWeights[u][i] * Values[i];
#ifdef CONNX
    conx += Nunits;
#endif
    value = ACTIVATION(sum);
    actprime = ACTIVATION_PRIME(value, sum);
    CandSumValues[u] += value;
    /* Now try to adjust the inputs so as to maximize the absolute value */
    /* of the correlation. */
    for(o=0; o<Noutputs; o++){
      error = Errors[o];
      direction = (CandPrevCor[u][o] < 0.0f) ? -1.0f : 1.0f;
      change -= direction * actprime *((error -SumErrors[o])/SumSqError);
      CandCor[u][o] += error * value;
    }
    for(i=0; i<Nunits; i++)
       CandSlopes[u][i] += change * Values[i];
  }
}  /* COMPUTE_SLOPES */




/* Update the input weights, using the pre-computed slopes, prev-slopes,
 * and delta values.
 */
void  UsfCasCor::UPDATE_INPUT_WEIGHTS ()
{
  int i,u;
  float eps;
  float *cw, *cd, *cs, *cp;
/*********/

  eps = InputEpsilon / (float)(Ncases * Nunits);
  for(u=0; u<Ncandidates; u++)
  {
    cw = CandWeights[u];
    cd = CandDeltas[u];
    cs = CandSlopes[u];
    cp = CandPrevSlopes[u];
    for(i=0; i<Nunits; i++)
      QUICKPROP_UPDATE(i, cw, cd, cs, cp, eps, InputDecay, InputMu, 
		       InputShrinkFactor);
  }
}  /* UPDATE_INPUT_WEIGHTS */




/* For each training pattern, perform a forward pass and compute correlations.
 * Then perform a second forward pass and compute input slopes for the 
 * candidate units.  Finally, use quickprop update to adjust the input weights.
 */

void  UsfCasCor::TRAIN_INPUTS_EPOCH ()
{
  int i;
/********/

  for(i=FirstCase; i<(Ncases+FirstCase); i++)
  {
    Goal = TrainingOutputs[i];
    if(UseCache){
      Values = ValuesCache[i];
      Errors = ErrorsCache[i];
    }
    else {
      Values = ExtraValues;
      Errors = ExtraErrors;
      FULL_FORWARD_PASS(TrainingInputs[i]);
      COMPUTE_ERRORS (Goal, False, False, i);
     }
    COMPUTE_SLOPES();
  }
  /*  User may have changed mu between epochs, so fix shrink-factor.*/
  InputShrinkFactor = InputMu / (1.0f + InputMu);

  /* Now tweak the candidate unit input weights. */
  UPDATE_INPUT_WEIGHTS();

  /*  Fix up the correlation values for the next epoch.*/
  ADJUST_CORRELATIONS();
  Epoch++;
}




/* Do an epoch through all active training patterns just to compute the
 * correlations.  After this one pass, we will update the correlations as we
 * train.
 */
void  UsfCasCor::CORRELATIONS_EPOCH ()
{
  int i;
/********/

  for  (i=FirstCase; i<(Ncases+FirstCase); i++)
  {
    Goal = TrainingOutputs[i];
    if  (UseCache)
    {
      Values = ValuesCache[i];
      Errors = ErrorsCache[i];
    }
    else 
    {
      Values = ExtraValues;
      Errors = ExtraErrors;
      FULL_FORWARD_PASS(TrainingInputs[i]);
      COMPUTE_ERRORS(Goal, False, False, i);
    }
    COMPUTE_CORRELATIONS();
  }
  /*  Fix up the correlation values for the next epoch. */
  ADJUST_CORRELATIONS();
  Epoch++;
}  /* CORRELATIONS_EPOCH */




/* Train the input weights of all candidates.  If we exhaust max_epochs,
 * stop with value TIMEOUT.  Else, keep going until the best candidate unit's
 * score has changed by a significant amount, and then
 * until it does not change significantly for Patience epochs.  Then return
 * STAGNANT.  If Patience is zero, we do not stop until victory or until
 * max_epochs is used up.
 */
int  UsfCasCor::TRAIN_INPUTS (int max_epochs)
{
  int i;
  float last_score = 0.0;
  int quit = max_epochs;
  Boolean first_time = True;
/**********/

  for(i=0; i<Noutputs; i++)	/* Convert to the average error for use in */
    SumErrors[i]  /=  Ncases;	/* calculation of the correlation. */

  CORRELATIONS_EPOCH();

  for(i=0; i<max_epochs; i++){
    TRAIN_INPUTS_EPOCH();

    if(InputPatience == 0)
      continue;			/* continue training until victory */
    else if(first_time){
      first_time = False;
      last_score = BestCandidateScore;
    }
    else if(fabs(BestCandidateScore - last_score) > /* still getting better */
	    (last_score * InputChangeThreshold)){
      last_score = BestCandidateScore;
      quit = i + InputPatience;
    }
    else if(i >= quit) /* haven't gotten better for a while */
      return(STAGNANT);
  }

  /* didn't return within the loop, so must have run out of time. */
  return(TIMEOUT);

}  /* TRAIN_INPUTS */





/**********************************************************************/
/*                                                                    */
/*  The outer loop routines                                           */
/*                                                                    */
/**********************************************************************/


void  UsfCasCor::LIST_PARAMETERS ()
{
#ifdef __STDC__			/* does is compiler conform to the standard? */
  //fprintf(stderr,"\nCascor.c Version: %5.2f %s   Compiled: %s  %s\n", 
	 //VERSION, REL_DATE, __DATE__, __TIME__);
#else
  //fprintf(stderr,"\nCascor.c Version: %5.2f  %s\n", VERSION, REL_DATE);
#endif
  /*fprintf(stderr,"Trial Number %d Parameters\n", Trial);
  fprintf(stderr,"SigOff %4.2f, WtRng %4.2f, WtMul %4.2f\n",
	 SigmoidPrimeOffset, WeightRange, WeightMultiplier);
  fprintf(stderr,"OMu %4.2f, OEps %4.2f, ODcy %7.5f, OPat %d, OChange %4.2f\n",
	  OutputMu, OutputEpsilon, OutputDecay, OutputPatience,
	  OutputChangeThreshold);
  fprintf(stderr,"IMu %4.2f, IEps %4.2f, IDcy %7.5f, IPat %d, IChange %4.2f\n",
	  InputMu, InputEpsilon, InputDecay, InputPatience,
	  InputChangeThreshold);
  fprintf(stderr,"Utype: %s, Otype: %s, Pool %d\n",
	  TYPE_STRING(UnitType), TYPE_STRING(OutputType), Ncandidates);
  fprintf(stderr,"ErrMeas: %s, ErrThres: %5.3f\n",
	 TYPE_STRING(ErrorMeasure), ErrorIndexThreshold);
*/
}



/* Train the output weights until stagnation or victory is reached.  Then
 * train the input weights to stagnation or victory.  Then install the best
 * candidate unit and repeat.  OUTLIMIT and INLIMIT are upper limits on the
 * number of cycles in the output and input phases.  ROUNDS is an upper
 * limit on the number of unit-installation cycles.
 */
int  UsfCasCor::TRAIN (int  outlimit, 
                       int  inlimit, 
                       int  rounds
                      )
{
  int i,r;

/***********/

  initialize_network();
  LIST_PARAMETERS();

  if  (UseCache)
    for(i=0; i<NTrainingPatterns; i++)
    {
      Values = ValuesCache[i];
      SETUP_INPUTS(TrainingInputs[i]);
    }

  for  (r = 0;  (r < rounds)  &&  (!cancelFlag);  r++)
  {
    log.Level (10) << "TRAIN  Round " << r << endl;
    switch(TRAIN_OUTPUTS(outlimit)){
    case WIN:
      LIST_PARAMETERS();
      log.Level (10) << "Victory at " 
                     << Epoch                   << "epochs, " 
                     << Nunits                  << " units, " 
                     << (Nunits - Ninputs - 1)  << " hidden, "
                     << TrueError               << " Error "
                     << ErrorIndex              << " EI"
                     << endl;
      return(WIN);

    case TIMEOUT:      
      log.Level (10) << "TRAIN  -Output: Epoch " << Epoch << " Timeout" << endl
                     << "   train:" <<  PRINT_SUMMARY (NTrainingPatterns) << endl;
      break;

    case STAGNANT:
      log.Level (10) << "TRAIN  +Output: Epoch " << Epoch << " Timeout" << endl
                     << "    train:" << PRINT_SUMMARY (NTrainingPatterns) << endl;
      break;

    default:
      log.Level (10) << "Bad return from TRAIN_OUTPUTS" << endl;
      break;
    }

    /* DumpWeightsFileforROundx */
    if  (Test)  TEST_EPOCH(0.49);  /* how are we doing? */

    switch  (TRAIN_INPUTS (inlimit))
    {
    case TIMEOUT:
      log.Level (10) << "TRAIN  -Input : Epoch " << Epoch << " Timeout  (Correlation " << BestCandidateScore << ")" << endl;
      break;

    case STAGNANT:
      log.Level (10) << "TRAIN  -Input : Epoch " << Epoch << " Timeout  (Correlation " << BestCandidateScore << ")" << endl;
      break;

    default:
      log.Level (10) << "TRAIN   Bad return from TRAIN_INPUTS" << endl;
      break;
    }

    INSTALL_NEW_UNIT(); 
    log.Level (10) << "ADDED UNIT: " << (r + 1) << endl;
  }

  LIST_PARAMETERS ();

  switch (TRAIN_OUTPUTS (outlimit))
  {
    case WIN:
      log.Level (10) << "TRAIN   Victory at " << Epoch  << " epochs, " << Nunits << " units, " << (Nunits - Ninputs - 1) << " hidden, Error " << TrueError << " EI " << ErrorIndex << endl;
      return(WIN);

    case TIMEOUT: 
    case STAGNANT:
      log.Level (10) << "TRAIN   Defeat at " << Nunits << " units  " << PRINT_SUMMARY (NTrainingPatterns) << endl;
      return(LOSE);

    default:
      log.Level (10) << "TRAIN   Bad return from TRAIN_OUTPUTS" << endl;
      break;
    }

  return -1;
}  /* TRAIN */






/* Perform forward propagation once for each set of weights in the
 * testing vectors, computing errors.  Do not change any weights.
 */
void  UsfCasCor::TEST_EPOCH (double test_threshold)
{
  int i;

  /* Globals must be saved from the last training phase. If they are not  */
  /* saved then the next unit will be training to correlate with the test */
  /* set error. */
  Boolean old_UC = UseCache;	/* temporarily turn off cache */
  float old_ST = ScoreThreshold; /* save global */
  float old_TE = TrueError;	/* save global */
  float *old_SE = SumErrors;	/* save global */
  float old_SSE = SumSqError;	/* save global */

  if  ((test_threshold > FLT_MAX)  || (test_threshold < FLT_MIN))
    cerr << endl << "UsfCasCor::TEST_EPOCH    test_threshold[" << test_threshold << "]  has exceeded capacity of a float variabnle." << endl << endl;

  ScoreThreshold = (float)test_threshold;
  UseCache = False;

  Values = ExtraValues;
  Errors = ExtraErrors;
  /* If no separate test inputs, use training inputs. */
  //if  (NTestPatterns == 0)
  //{
  //  TestInputs = TrainingInputs;    
  //  TestOutputs = TrainingOutputs;
  //  NTestPatterns = NTrainingPatterns;
  //}

  /* Zero some accumulators. */
  ErrorBits = 0;
  ErrorMisclassifications=0;
  TrueError = 0.0;
  SumErrors = DummySumErrors;
  SumSqError = 0.0;

  /* Now run all test patterns and report the results. */
  for  (i = 0; i < NTrainingPatterns;  ++i)
  {
    Goal = TrainingOutputs[i];
    FULL_FORWARD_PASS (TrainingInputs[i]);
    COMPUTE_ERRORS (Goal, False, True, -1);
  } 

  if  (ErrorMeasure == INDEX)
    ErrorIndex = ERROR_INDEX (TestStdDev, NtestOutputValues);

  log.Level (10) << "TEST_EPOCH   test :" << PRINT_SUMMARY (NTrainingPatterns) << endl;

  /* restore globals */
  UseCache = old_UC;		
  ScoreThreshold = old_ST;
  TrueError = old_TE;	
  SumErrors = old_SE;	
  SumSqError = old_SSE;
}  /* TEST_EPOCH */




/* print out the things interesting after a pass.
 */
void  UsfCasCor::OUT_PASS_OUTPUT ()
{
  int i;

  fprintf (stderr," Outputs: ");
  for  (i = 0;  i < Noutputs;  i++)
    fprintf(stderr,"%6.4f ", Outputs[i]);

  fprintf (stderr,"\n Errors: ");
  for  (i = 0;  i < Noutputs;  i++)
    fprintf(stderr,"%6.4f ", Errors[i]);

  fprintf(stderr,"\n Values: ");
  for(i=0;i<Nunits;i++)
    fprintf(stderr,"%6.4f ", Values[i]);

  fprintf(stderr,"\n\n");
}  /* OUT_PASS_OUTPUT */



/* Print the summary statistics based on the value of ErrorMeasure.
 */
KKStr   UsfCasCor::PRINT_SUMMARY (int n)
{
  KKStr  result (20);
  switch  (ErrorMeasure)
  {
  case BITS:
    result << "errbits " << ErrorBits << "\t"  << "error " << TrueError;
    break;

  case INDEX:
    result << " ErrorIndex " << ErrorIndex << ", TrueError "  << TrueError;
    break;
  }

  double acc = 100.0 - (100.0 * ErrorMisclassifications) / n;
  result << " accuracy "
         << StrFormatDouble (acc, "##0.00") << "% "
         << ErrorMisclassifications << "/" << n;
  return result;
}  /* PRINT_SUMMARY */




/* Set up the network for a learning problem.  Clean up all the data
 * structures.  Initialize the output weights to random values controlled by
 * WeightRange.
 */
void  UsfCasCor::initialize_network ()
{
  int i,j;
/**********/

  /* Set up the AllConnections vector. */
  for  (i = 0;  i < MaxUnits;  i++)
    AllConnections[i] = i;

  /* Initialize the active unit data structures. */
  for  (i = 0;  i < MaxUnits;  i++)
  {
    ExtraValues  [i] = 0.0;
    Nconnections [i] = 0;
    Connections  [i] = NULL;
    Weights      [i] = NULL;
  }

  /* Initialize the per-output data structures. */
  for  (i = 0;  i < Noutputs;  i++)
  {
    Outputs[i] = 0.0;
    ExtraErrors[i] = 0.0;
    for(j=0; j<MaxUnits; j++)
    {
      OutputWeights    [i][j] = 0.0;
      OutputDeltas     [i][j] = 0.0;
      OutputSlopes     [i][j] = 0.0;
      OutputPrevSlopes [i][j] = 0.0;
    }
    /* Set up initial random weights for the input-to-output connections. */
    for  (j = 0;  j < (Ninputs + 1);  ++j)
      OutputWeights[i][j] = random_weight();
  }

  /* Initialize the caches if they are in use. */
  if  (UseCache)
  {
    for  (j = 0;  j < MaxCases;  ++j)
    {
      for  (i = 0;  i < MaxUnits;  ++i)
        ValuesCache[j][i] = 0.0;

      for  (i = 0;  i < Noutputs;  ++i)
        ErrorsCache[j][i] = 0.0;
    }
  }

  /* Candidate units get initialized in a separate routine. */
  INIT_CANDIDATES ();

  ExtraValues[0] = 1.0;		/* bias unit */
  Epoch = 0;
  Nunits = Ninputs + 1;  
  ErrorBits = 0;
  TrueError = 0.0;
  
  for  (i = 0;  i < Noutputs;  ++i)
  {
    SumErrors[i] = 0.0;
    DummySumErrors[i] = 0.0;
  }

  SumSqError = 0.0;
  BestCandidateScore = 0.0;
  BestCandidate = 0;
#ifdef CONNX
  conx = 0l;
#endif

  if(ErrorMeasure == INDEX){
    /* ErrorIndex initialization */
    ErrorIndex = 0.0;
    TrainingStdDev = STANDARD_DEV(TrainingOutputs, NTrainingPatterns, 
				  NtrainingOutputValues);
    if(NTestPatterns)
      TestStdDev = STANDARD_DEV(TestOutputs, NTestPatterns, 
				NtestOutputValues);
  }
}  /* initialize_network */




/* Calculate the standard deviation of an entire output set.
 */
float  UsfCasCor::STANDARD_DEV (float** outputs, 
                                int     npatterns, 
                                int     nvalues
                               )
{
  int i,j;
  float sum_o_sqs = 0.0;
  float sum = 0.0;
  float cur = 0.0;
  float fnum = (float)nvalues;
/**************/

  for(i=0;i<npatterns;i++)
    for(j=0;j<Noutputs;j++){
      cur = outputs[i][j];
      sum += cur;
      sum_o_sqs += cur * cur;
    }

  return (sqrt((fnum * sum_o_sqs - sum * sum)/
	       (fnum * (fnum - 1.0f))));
}  /* STANDARD_DEV */




/* ErrorIndex is the rms TrueError normalized by the standard deviation of the 
 * goal set.
 */
float  UsfCasCor::ERROR_INDEX (double std_dev, 
                               int    num
                              )
{
  return (sqrt( TrueError / (float)num) / (float)std_dev);
}






//******************************************************************************
//                            Utility Functions.                               *
//******************************************************************************


int  GetProcessId ()
{
  return  osGetProcessId ();
}



int daysYTDforMonth[] = 
   {0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
   //  Jan  Feb  Mar  Apr  may  jun  jul  aug  sep  oct  nov  dec



kkint64  GetLocalDateTime ()
{
#ifdef  WIN32
  SYSTEMTIME  sysTime;
  GetLocalTime(&sysTime);

  int  year   = (short)sysTime.wYear;
  int  month  = sysTime.wMonth;
  int  day    = sysTime.wDay;
  int  hour   = sysTime.wHour;
  int  minute = sysTime.wMinute;
  int  second = sysTime.wSecond;

  int  years = year - 1970;
  int  numLeapYears = 0;
  int  y =1972;
  while  (y < year)  
  {
    y += 4; 
    ++numLeapYears;
  }

  if  (((year % 4) == 0)  &&  (month > 2))
    ++numLeapYears;

  int  ytdDays = day;
  if  ((month > 0)  &&  (month < 13))
    ytdDays = daysYTDforMonth[month - 1];


  long long  result = years * 365 * 24 * 60 * 60  + 
                      numLeapYears * 24 * 60 * 60 +
                      ytdDays      * 24 * 60 * 60 +
                      hour         * 60 * 60      +
                      minute       * 60           +
                      second;
  return  result;
#else
  time_t   long_time;
  return  long_time;
#endif

}  /* GetCurrentDateTime */


char*  GetDsiplayableTime ()
{
#if  defined(WIN32)
   __time64_t ltime;
   _time64( &ltime );
   return  strdup (_ctime64 (&ltime));
#else
  struct tm  *curTime;
  time_t     long_time;
  time (&long_time);
  return  strdup (ctime (&long_time)
      );
#endif
}  /* GetDsiplayableTime */


char*   index (char* str, int c)
{
  return strchr (str, c);
}


#if  defined(WIN32)
int  strncasecmp (const char*  s1, 
                  const char*  s2, 
                  int          count
                 )
{
  if  (s1 == NULL)
  {
    if  (s2 == NULL)
      return 0;
    else
      return -1;
  }
  else if  (s2 == NULL)
  {
    return 1;
  }

  const char*  c1 = s1;   
  const char*  c2 = s2;
  int    c = 0;
  while  (c < count)
  {
    int  x = tolower (*c1) - tolower(*c2);
    if  (x < 0)
      return -1;

    else if  (x > 0)
      return 1;

    if  (*c1 == 0)
      break;

    ++c1;
    ++c2;
    ++c;
  }

  return 0;
}  /* srtncasecmp */
#endif




//******************************************************************************
//                                globals.c                                    *
//******************************************************************************

/*********************************************************************/
/* keyword table used for updating the simulation parameters without */
/* recompilation. They must be in alphabetical order by keyword.     */
/*********************************************************************/

void  UsfCasCor::ConstructParmTable ()
{
  //ParmTable = 
  //{
  ParmTable[ 0] = parmentry ("errorindexthreshold",   FLOAT,      (void *)&ErrorIndexThreshold);
  ParmTable[ 1] = parmentry ("errormeasure",          ENUM_NO,    (void *)&ErrorMeasure);
  ParmTable[ 2] = parmentry ("go",                    GO,         (void *)NULL); /* special keyword */
  ParmTable[ 3] = parmentry ("graphics",              BOOLE,      (void *)&Graphics);
  ParmTable[ 4] = parmentry ("inputchangethreshold",  FLOAT,      (void *)&InputChangeThreshold);
  ParmTable[ 5] = parmentry ("inputdecay",            FLOAT,      (void *)&InputDecay);
  ParmTable[ 6] = parmentry ("inputepsilon",          FLOAT,      (void *)&InputEpsilon);
  ParmTable[ 7] = parmentry ("inputmu",               FLOAT,      (void *)&InputMu);
  ParmTable[ 8] = parmentry ("inputpatience",         INT,        (void *)&InputPatience);
  ParmTable[ 9] = parmentry ("maxunits",              INT_NO,     (void *)&MaxUnits);
  ParmTable[10] = parmentry ("ncandidates",           INT_NO,     (void *)&Ncandidates);
  ParmTable[11] = parmentry ("ninputs",               INT_NO,     (void *)&Ninputs);
  ParmTable[12] = parmentry ("nonrandomseed",         BOOLE,      (void *)&NonRandomSeed);
  ParmTable[13] = parmentry ("noutputs",              INT_NO,     (void *)&Noutputs);
    //{"ntestpatterns",        INT_NO, (void *)&NTestPatterns},
    //{"ntrainingpatterns",    INT_NO, (void *)&NTrainingPatterns},
  ParmTable[14] = parmentry ("outputchangethreshold", FLOAT,      (void *)&OutputChangeThreshold);
  ParmTable[15] = parmentry ("outputdecay",           FLOAT,      (void *)&OutputDecay);
  ParmTable[16] = parmentry ("outputepsilon",         FLOAT,      (void *)&OutputEpsilon);
  ParmTable[17] = parmentry ("outputmu",              FLOAT,      (void *)&OutputMu);
  ParmTable[18] = parmentry ("outputpatience",        INT,        (void *)&OutputPatience);
  ParmTable[19] = parmentry ("outputtype",            ENUM,       (void *)&OutputType);
  ParmTable[20] = parmentry ("quit",                  BOMB,       (void *)NULL);    /* special keyword */
  ParmTable[21] = parmentry ("save",                  SAVE,       (void *)NULL);    /* special keyword */
  ParmTable[22] = parmentry ("scorethreshold",        FLOAT,      (void *)&ScoreThreshold);
  ParmTable[23] = parmentry ("sigmoidmax",            FLOAT_NO,   (void *)&SigmoidMax);
  ParmTable[24] = parmentry ("sigmoidmin",            FLOAT_NO,   (void *)&SigmoidMin);
  ParmTable[25] = parmentry ("sigmoidprimeoffset",    FLOAT,      (void *)&SigmoidPrimeOffset);
  ParmTable[26] = parmentry ("singleepoch",           BOOLE,      (void *)&SingleEpoch);
  ParmTable[27] = parmentry ("singlepass",            BOOLE,      (void *)&SinglePass);
  ParmTable[28] = parmentry ("test",                  BOOLE,      (void *)&Test);
  ParmTable[29] = parmentry ("testing",               GETTEST,    (void *)NULL);      /* special keyword */
  ParmTable[30] = parmentry ("training",              GETTRAINING,(void *)NULL);      /* special keyword */
  ParmTable[31] = parmentry ("unittype",              ENUM_NO,    (void *)&UnitType);
  ParmTable[32] = parmentry ("usecache",              BOOLE_NO,   (void *)&UseCache);
  ParmTable[33] = parmentry ("values",                VALUE,      (void *)NULL);      /* special keyword */
  ParmTable[34] = parmentry ("weightfile",            INITFILE,   (void *)NULL);      /* special keyword */
  ParmTable[35] = parmentry ("weightmultiplier",      FLOAT,      (void *)&WeightMultiplier);
  ParmTable[36] = parmentry ("weightrange",           FLOAT,      (void *)&WeightRange);
  //}
  // 4160 - 4122 + 1 - 2 = 37 entries
  Nparameters = 		/* Number of entries in ParmTable */
      sizeof(ParmTable)/sizeof(PARMS);
}  /* ConstructParmTable */


/* Initialize all globals that are not problem dependent.  Put this function 
 * in a seperate file to make changing parameters less painful.
 */
void UsfCasCor::INITIALIZE_GLOBALS ()
{
  OutputShrinkFactor=OutputMu / (1.0f + OutputMu); 
  InputShrinkFactor=InputMu / (1.0f + InputMu);
  //signal(SIGINT, TRAP_CONTROL_C); /* initialize interrupt handler */
  //InterruptPending = FALSE;
}






//******************************************************************
//*                       load_data.c                              *
//******************************************************************


void  UsfCasCor::load_data (FeatureVectorListPtr  trainExamples)
{
  _load_training_data (trainExamples);

  log.Level (10) << "UsfCasCor::load_data   Read in [" << NTrainingPatterns << "] training patterns of dimension[" << Ninputs << "]." << endl;
  
  return;
}  /* load_data */



/*****************************************************************
  private functions
******************************************************************/
void  UsfCasCor::_load_training_data (FeatureVectorListPtr  trainExamples)
{
  for  (kkint32 i = 0;  i < NTrainingPatterns;  ++i) 
    _load_training_example (trainExamples->IdxToPtr (i), i);
}  /* _load_training_data */



/* Build the next NTrainingPattern example from 'example'. */
void  UsfCasCor::_load_training_example (FeatureVectorPtr  example,
                                         int               i
                                        )
{
  if  (Ninputs != selectedFeatures->NumSelFeatures ())
  {
    log.Level (-1) << endl 
      << "UsfCasCor::_load_training_example  ***ERROR***  Ninputs[" << Ninputs 
      << "] != selectedFeatures->NumSelFeatures ()[" << selectedFeatures->NumSelFeatures () << "]" << endl 
      << endl;
  }

  kkint32  j = 0;

  const float*     featureData = example->FeatureData ();
  const kkuint16*  featureNums = selectedFeatures->FeatureNums ();

  // at this point   nInputs should equal selecedFeatures->NumSelFeatures ()
  for  (j = 0;  j < Ninputs;  ++j)
    TrainingInputs[i][j] = featureData[featureNums[j]];

  kkint32  k =  classes->PtrToIdx (example->MLClass ());

  for  (j = 0;  j < Noutputs;  j++)
  {
    if  (j == k) 
      TrainingOutputs[i][j] = SigmoidMax;
    else
      TrainingOutputs[i][j] = SigmoidMin; 
  }
}  /* _load_training_example */




/* Get the test data from a seperate file.  Open the file
 */
void  UsfCasCor::_load_test_data (FeatureVectorPtr  example)
{
  const float*     featureData = example->FeatureData ();
  const kkuint16*  featureNums = selectedFeatures->FeatureNums ();

  // at this point   nInputs should equal selecedFeatures->NumSelFeatures ()
  for  (int j = 0;  j < Ninputs;  ++j)
    TestInputs[0][j] = featureData[featureNums[j]];

  //int k = classes->PtrToIdx (example->MLClass ());
  int k = -1;

  for  (int j = 0;  j < Noutputs;  j++)
  {
    if  (j == k) 
      TestOutputs[0][j] = SigmoidMax;
    else
      TestOutputs[0][j] = SigmoidMin;
  }
  return;

}  /* _load_test_data */

