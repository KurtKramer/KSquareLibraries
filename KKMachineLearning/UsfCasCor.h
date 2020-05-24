#if  !defined(__USFCASCOR__H_)
#define  __USFCASCOR__H_

/**
 *@brief   Adapted by Kurt Kramer be a 'class' definition so as to make it more usable in th ePices software world.
 *@details  The original author is _____________.
   See notes below for original author, etc.

   Modified by Kurt Kramer 2012-09-10:
      Originally written by    R. Scott Crowder, III   Se below for his comments.

      I ported this implementation as found at USF into a c++ Class and integrated into the Pices
      application.
        1) Restructured code as a c++ class.
        2) A trained classifier is written to disk and can be reread at a later time.
        3) Integrated into the Pices application.
        4) Primary use will to be used in a Dual classifier setup along with a Support Vector Machine(SVM)(libSVM)
           where both classifiers agree on the prediction will the label be returned otherwise the label
           "UnKnown" will be returned.  User will have option to have the common part of the prediction in the 
           class hierarchy returned instead.
        5) Added predicted confidence values for each class.


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
     -R      use training set (re-substitution)

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
//#include "utils.h"

#if defined(_MSC_VER)
#pragma warning(disable:4996)
#endif

#ifndef INT_MAX
#define INT_MAX 32767
#endif


typedef  bool  Boolean;

#define LINELEN   1024


/* switches used in the interface routines */
#define INT         0
#define FLOAT       1
#define ENUM        2      /* integer values that use #defines */
#define BOOLE       3
#define GETTRAINING 4
#define GETTEST     5
#define GO          6
#define INT_NO      7      /* parameters only good in net-file */
#define FLOAT_NO    8      /* most are used in memory allocation  */
#define ENUM_NO     9      /* and cannot be changed mid-simulation */
#define BOOLE_NO    10
#define VALUE       11
#define GETTRAININGFILE 12
#define GETTESTFILE     13
#define SAVE        14
#define INITFILE    15



#define NEXTLINE    0
#define FAILURE    -1

/* switch constants */
#define SIGMOID        0
#define GAUSSIAN       1
#define LINEAR         2
#define ASYMSIGMOID    3
#define VARSIGMOID     4

#define WIN            20
#define STAGNANT       21
#define TIMEOUT        22
#define LOSE           23

#define BITS           30
#define INDEX          31

#define FATAL           0
#define WARN            1

#define BOMB -99
/* Feature types. There are only three types supported (this is
   a neural network). The logic is ignore,binary otherwise real. */
#define REAL   1
#define uccIGNORE 2
#define BINARY 3

#if  !defined(min)
  #define min(X, Y) ((X < Y)?X:Y)                    
#endif

#if  !defined(max)
  #define max(X, Y) ((X > Y)?X:Y)
#endif

#include "KKBaseTypes.h"
#include "RunLog.h"
#include "XmlStream.h"

#include "MLClass.h"
#include "FeatureVector.h"


namespace KKMLL
{
  #if  !defined(_CLASSPROB_)
  class  ClassProb;
  typedef  ClassProb*  ClassProbPtr;
  class  ClassProbList;
  typedef  ClassProbList*  ClassProbListPtr;
  #endif


  /**
   *@brief Implementation of Cascade-Correlation network originally written by 
   * R. Scott Crowder, III or CMS  restructured as a c++ class.
   *@details 
   *@code
   *  Important Methods:
   *
   *  TrainNewClassifier        Trains a new classier from training examples.
   *  WriteXML                  Saves a trained classifier to a output stream in the form of a XML object.
   *  PredictClass              Returns back predicted class for a given Feature vector.
   *  PredictClassConfidences   Prediction that returns back confidence values for each class rather than a single class prediction.
   */

  class  UsfCasCor
  {
  public:
    UsfCasCor ();

    ~UsfCasCor ();

    kkMemSize  MemoryConsumedEstimated ()  const;


    void  TrainNewClassifier (kkint32                 _in_limit,
                              kkint32                 _out_limit,
                              kkint32                 _number_of_rounds,
                              kkint32                 _number_of_trials,
                              kkint64                 _the_random_seed,
                              bool                    _useCache,
                              FeatureVectorListPtr    _trainData,
                              FeatureNumListConstPtr  _selectedFeatures,
                              VolConstBool&           _cancelFlag,
                              RunLog&                 _log
                             );

    MLClassPtr  PredictClass (FeatureVectorPtr  example);

    void  PredictConfidences (FeatureVectorPtr    example,
                              MLClassPtr          knownClass,
                              MLClassPtr&         predClass1,
                              float&              predClass1Prob,
                              MLClassPtr&         predClass2,
                              float&              predClass2Prob,
                              float&              knownClassProb,
                              const MLClassList&  classOrder,      /**< Dictates the order in which 'probabilities' will be populated. */
                              VectorFloat&        probabilities
                             );


    ClassProbListPtr  PredictClassConfidences (FeatureVectorPtr  example);


    void  WriteXML (const KKStr&  varName,
                    ostream&      o
                   )  const;


    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );


  private:
    void  CleanUpMemory ();

    template<typename T>
    void  Delete2DArray (T**     &A,    
                         kkuint32  numRows
                        );


    /*****************************************************************************/
    /* Parameter table contains all user settable parameters.  It is used by the */
    /* input routines to change the values of the named parameters.  The list of */
    /* parameters must be sorted alphabetically so that the search routine will  */
    /* work.  Parameter names are used as keywords in the search.  Keywords are  */
    /* lower case to speed the comparison process.                               */
    /*****************************************************************************/
    struct  parmentry 
    {
      parmentry (): keyword (NULL), vartype (0), varptr (NULL)  {}

      parmentry (const char* _keyword, int _vartype, void* _varptr):
         keyword (_keyword), vartype (_vartype), varptr (_varptr)
      {}

      const char*   keyword;    /**< variable name in lower case */
      int           vartype;    /**< can be INT, FLOAT, or ENUM */
      void*         varptr;     /**< cast to correct type before use */
    };

    typedef parmentry PARMS;


    FeatureVectorListPtr  FilterOutExtremeExamples (FeatureVectorListPtr  trainExamples);


    /* Global variables */
    static  const char*  version;
    static  const char*  release_date;
    static  const char*  progname;
    
    char const *  boolean_to_string (bool var)  const;

    Boolean  string_to_boolean (const char* s);

    const KKStr&  type_to_string (int var)  const;

    int    string_to_type (const KKStr&  s);

    /* Utilities */
    
    void   INITIALIZE_GLOBALS ();



    //***************************************************************
    //*                            usfcascor                        *
    //***************************************************************
    int  in_limit;
    int  out_limit;
    int  number_of_trials;
    int  number_of_rounds;
    int  normalization_method;
    int  my_mpi_rank;

    void  setup_network (FeatureVectorListPtr  trainExamples,
                         RunLog&               log
                        );

    void  train_network    (VolConstBool&  cancelFlag,
                            RunLog&        log
                           );

    void  allocate_network (RunLog&  log);


    //****************************************************************
    //*                            'parms.c'                         *
    //****************************************************************
    static  const KKStr  type_strings[];
    static  const char*  boolean_strings[2];

    char parm_string[LINELEN];

    void     strdncase (char *s);

    int      _type_convert (char *input);

    Boolean  _boolean_convert (char *input);




    //**********************************************************************************
    //*                                 util.c                                         *
    //**********************************************************************************
    float  random_weight ();


    //******************************************************************************************
    //*                                   load_namesfile.c                                     *
    //******************************************************************************************
    void  load_namesfile (FeatureVectorListPtr trainExamples);
    

    //******************************************************************************************
    //*                                      learn.c                                           *
    //******************************************************************************************
    int ErrorMisclassifications;

    float  ACTIVATION (float sum);

    float  ACTIVATION_PRIME (float value, 
                             float sum
                            );

    float  OUTPUT_FUNCTION (float sum);

    float  OUTPUT_PRIME (float output);

    void  QUICKPROP_UPDATE (int i, 
                            float weights[], 
                            float deltas[], 
                            float slopes[], 
		                        float prevs[], 
                            float epsilon,
                            float decay,
                            float mu,
                            float shrink_factor
                           );
    
    void  SETUP_INPUTS (float inputs[]);

    void  OUTPUT_FORWARD_PASS ();

    void  COMPUTE_UNIT_VALUE (int j);

    void  FULL_FORWARD_PASS (float input[]);


    void  COMPUTE_ERRORS (float    goal[], 
                          Boolean  output_slopesp, 
                          Boolean  statsp, 
                          int      xw
                         );

    void  UPDATE_OUTPUT_WEIGHTS ();


    void  TRAIN_OUTPUTS_EPOCH ();

    int  TRAIN_OUTPUTS (int            max_epochs,
                        VolConstBool&  cancelFlag
                       );
   
    void  INIT_CANDIDATES ();

    void  INSTALL_NEW_UNIT (RunLog&  log);

    void  COMPUTE_CORRELATIONS ();

    void ADJUST_CORRELATIONS ();

    void  COMPUTE_SLOPES ();

    void  UPDATE_INPUT_WEIGHTS ();

    void  TRAIN_INPUTS_EPOCH ();

    void  CORRELATIONS_EPOCH ();

    int   TRAIN_INPUTS (int max_epochs);

    void  LIST_PARAMETERS ();

    int  TRAIN (int            outlimit, 
                int            inlimit, 
                int            rounds,
                VolConstBool&  cancelFlag,
                RunLog&        log
               );

    void  TEST_EPOCH (double   test_threshold,
                      RunLog&  log
                     );

    void  OUT_PASS_OUTPUT ();

    KKStr PRINT_SUMMARY (int n);

    void  initialize_network ();

    float  STANDARD_DEV (float** outputs, 
                         int     npatterns, 
                         int     nvalues
                        );


    float  ERROR_INDEX (double std_dev, 
                        int    num
                       );


    //******************************************************************
    //*                       load_data.c                              *
    //******************************************************************


    void  load_data (FeatureVectorListPtr  trainExamples,
                     RunLog&               log
                    );

    void  _load_training_data (FeatureVectorListPtr  trainExamples);

    void  _load_training_example (FeatureVectorPtr   example,
                                  int                i
                                 );


    void  _load_test_data (FeatureVectorPtr  example);



    //******************************************************************************
    //                                globals.c                                    *
    //******************************************************************************
    int      number_of_classes;
    char*    filestem;
    int      line_length;
    int*     feature_type;

    kkint64  the_random_seed;


    /* Flags */
    Boolean  load_weights;


    int UnitType;		               /**< hidden unit type can be SIGMOID or GAUSIAN*/
    int OutputType;	               /**< output unit type can be SIGMOID or LINEAR */
    
    float SigmoidMax;              /**< Maximum output value for sigmoid units. Used to alter sigmoid range without having to edit *
                                    * training values.  Use the symbols "min" and  "max" in the input file.  The input routines    * 
                                    * will translate to the appropriate float values.
                                    */
   
    float SigmoidMin;              /**< Minimum output value for sigmoid units.                                            */
    float WeightRange;             /**< Random-init weights in range [-WR,+WR]                                             */
    float SigmoidPrimeOffset;      /**< Add to sigmoid-prime to kill flat spots                                            */
    float WeightMultiplier;	       /**< Scale Candidate correlation to get init weight                                     */
    float OutputMu;		             /**< Mu used to quickprop train output weights.                                         */
    float OutputShrinkFactor;      /**< Used in computing whether the proposed step is too large.  Related to OutputMu.    */
    float OutputEpsilon;           /**< Controls the amount of linear gradient descent to use in updating output weights.  */

    float OutputDecay;             /**< This factor times the current weight is added to the slope at the start of each
                                    * output epoch Keeps weights from growing too big.
                                    */

    int OutputPatience;            /**< If we go for this many epochs with no real change, it's time to stop tuning.
                                    *   If 0, go on forever.
                                    */

    float OutputChangeThreshold;   /**< The error must change by at least this fraction of its old value to count as a significant change. */
    float InputMu;	               /**< Mu used to quick prop train input weights.  */
    float InputShrinkFactor;       /**< Used in computing whether the proposed step is too large.  Related to InputMu. */
    float InputEpsilon;	           /**< Controls the amount of linear gradient descent to use in updating Input weights. */
    float InputDecay;	             /* This factor times the current weight is added to the slope at the start of each Input epoch *
                                    * Keeps weights from growing too big. */

    int InputPatience;             /* If we go for this many epochs with no real change, it's time to stop tuning.  If 0, 
                                    * go on forever. */

    float InputChangeThreshold;    /* The error must change by at least this fraction of its old value to count as a significant change. */

    /*  Variables related to error and correlation.   */
    float    TrueError;             /**< Total output error for one epoch                */
    float    ScoreThreshold;        /**< This close to desired value => bit is correct   */
    int      ErrorBits;             /**< Total # bits in epoch that were wrong           */

    float*   SumErrors;             /**< Accumulate the sum of the error values used in the correlation phase. 
                                     * Sum is stored separately for each output.  Values are converted to 
                                     * average errors before use in ADJUST_CORRELATION 
                                     */

    float*   DummySumErrors;        /**< Replace SumErrors with this for test epochs. */
    float    SumSqError;            /**< Accumulate the sum of the square of the error values used in the correlation phase.*/
    float    BestCandidateScore;    /**< Best correlation score of all candidate units. */
    int      BestCandidate;         /**< Index of the candidate unit with best score. */
    Boolean  UseCache;	            /**< If TRUE, cache the forward-pass values instead of repeatedly computing them. */
    int      Epoch;                 /**< Current epoch number */
    Boolean  Graphics;	            /**< If TRUE, print progress after each epoch. */
    Boolean  NonRandomSeed;	        /**< TRUE => use 1 as the seed for the random number generator.  Useful when comparing
			                               * different parameter settings.  FALSE => use system clock to start random sequence. 
                                     */

    Boolean  Test;		               /**< If TRUE, run a test epoch and print the result after each round of output tuning. */

    Boolean  SinglePass;            /**< TRUE => Pause after forward/backward cycle      */
    Boolean  SingleEpoch;           /**< TRUE => Pause after each training epoch         */
    Boolean  Step;                  /**< Turned to TRUE after each pause, briefly        */
    int      Trial;                 /**< Current trial number, used in log outputs       */


    /* The sets of training inputs and outputs.  */
    int      NTrainingPatterns;    /**< !! Not in Lisp version.  Needed here. */
    int      NTestPatterns;        /**< !! Not in Lisp version.  Needed here. */
    float**  TrainingInputs;
    float**  TrainingOutputs;
    float*   Goal;                 /**< Goal vector for the current training or testing case.                           */

    float*   example_weight;

    /*  For some benchmarks there is a separate set of values used for testing */
    /*  the network's ability to generalize.  These values are not used during */
    /*  training.                                                              */
    float**  TestInputs;
    float**  TestOutputs;


    /***************************************************************************/
    /* Fundamental data structures.                                            */
    /*                                                                         */
    /* Unit outputs and weights are floats.                                    */
    /*                                                                         */
    /* Instead of representing each unit by a structure, we represent the      */
    /* unit by a int.  This is used to index into various arrays that hold     */
    /* per-unit information, such as the activation value of each unit.        */
    /*                                                                         */
    /* Per-connection information for each connection COMING INTO unit is      */
    /* stored in a array of arrays.  The outer array is indexed by the unit    */
    /* number, and the inner array is then indexed by connection number.       */
    /*                                                                         */
    /* Unit 0 is always at a maximum-on value.  Connections from this unit     */
    /* supply a bias.  Next come some input units, then some hidden units.     */
    /*                                                                         */
    /* Output units have their own separate set of data structures, as do      */
    /* candidate units whose inputs are currently being trained.               */
    /***************************************************************************/
    int      MaxUnits;            /**< Maximum number of input values and hidden in the network. */
    int      Nunits;              /**<  Total number of active units in net */
    int      Ninputs;             /**< Number of input units */
    int      Noutputs;            /**< Number of output units */
    int      Ncandidates;	        /**< Number of candidate units trained at once. */
    int      MaxCases;		        /**< Maxi mum number of training cases that can be accommodated by the current data structures.  */
    int      Ncases;		          /**< Number  of training cases currently in use. Assume a contiguous block beginning with   */
    int      FirstCase;	          /**< Address of the first training case in the currently active set.  Usually zero, but may
                                   *  differ if we are training on different chunks of the training set at different times.       
                                   */


    /***************************************************************************/
    /* The following vectors hold values related to hidden units in the active */
    /* net and their input weights.                                            */
    /***************************************************************************/
    float*   Values;   	          /**< Current activation value for each unit */
    float**  ValuesCache;         /**< Holds a distinct Values array for each of the MaxCases training cases. */
    float*   ExtraValues;	        /**< Extra Values vector to use when no cache. */
    int*     Nconnections;        /**< # of INCOMING connections per unit */
    int**    Connections;         /**< C[i][j] lists jth unit projecting to unit i */
    float**  Weights;             /**< W[i][j] holds weight of C[i][j] */

    /***************************************************************************/
    /* The following arrays of arrays hold values for the outputs of the active*/
    /*  network and the output-side weights.                                   */
    /***************************************************************************/
    float*   Outputs;             /**< Network output values */
    float*   Errors;              /**< Final error value for each unit */
    float**  ErrorsCache;         /**< Holds a distinct Errors array for each of the MaxCases training cases. */
    float*   ExtraErrors;         /**< Extra Errors vector to use when no cache. */
    float**  OutputWeights;       /**< OW[i][j] holds the weight from hidden unit i to output unit j */
    float**  OutputDeltas;        /**< Change between previous OW and current one */
    float**  OutputSlopes;        /**< Partial derivative of TotalError wrt OW[i][j] */
    float**  OutputPrevSlopes;    /**< Previous value of OutputSlopes[i][j] */

    /***************************************************************************/
    /* The following arrays have one entry for each candidate unit in the      */
    /* pool of trainees.                                                       */
    /***************************************************************************/
    float*   CandValues;         /**< Current output value of each candidate unit.   */
    float*   CandSumValues;      /**< Output value of each candidate unit, summed over an entire training set. */
    float**  CandCor;            /**< Correlation between unit & residual error at   each output, computed over a whole epoch.      */
    float**  CandPrevCor;        /**< Holds the CandCor values from last epoch.      */
    float**  CandWeights;        /**< Current input weights for each candidate unit. */
    float**  CandDeltas;         /**< Input weights deltas for each candidate unit.  */
    float**  CandSlopes;         /**< Input weights slopes for each candidate unit.  */
    float**  CandPrevSlopes;     /**< Holds the previous values of CandSlopes.       */

    /***************************************************************************/
    /* This saves memory if each candidate unit receives a connection from       */
    /* each existing unit and input.  That's always true at present, but may     */
    /* not be in future.                                                         */
    /***************************************************************************/
    int*     AllConnections;           /**< A standard connection that connects a unit to all previous units, in order, but not to the bias unit. */

    /***************************************************************************/
    /* ErrorIndex specific globals.  Not in release Lisp version               */
    /***************************************************************************/
    int      NtrainingOutputValues;    /**< Number of outputs in the training set.  */
    int      NtestOutputValues;	       /**< Number of outputs in the test set.      */
    float    TrainingStdDev;           /**< Std Dev of entire training set.  Used to normalize the ErrorIndex. */
    float    TestStdDev;
    float    ErrorIndex;               /**< Normalized error function for continuous output training sets. */
    float    ErrorIndexThreshold;      /**< Stop training when ErrorIndex is < EIT. */
    int      ErrorMeasure;             /**< Set to BITS for using ErrorBits to stop of INDEX to use ErrorIndex to stop.  */

    /***************************************************************************/
    /* Save and plot file related variables                                     */
    /***************************************************************************/
    FILE*    WeightFile;	             /**< Contains weights from the current net. */

    Boolean  InterruptPending;         /**< TRUE => user has pressed Control-C */

    MLClassListPtr  classes;            /**<  Classes that the training data consisted of.  */

    FeatureNumListPtr  selectedFeatures;   /**< The selected features that are to be used from the source training data. */
  };  /* UsfCasCor */

  typedef  UsfCasCor*  UsfCasCorPtr;

  
  typedef  XmlElementTemplate<UsfCasCor>  XmlElementUsfCasCor;
  typedef  XmlElementUsfCasCor*  XmlElementUsfCasCorPtr;




} /* KKMLL */

#endif


