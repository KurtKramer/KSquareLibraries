#if  !defined(__USFCASCOR__H_)

/**
 *@brief   Adapted by Kurt Kramer be a 'class' definition so as to  ake it more usable in th ePices software world.
 *@details  The original author is _____________.
 */

//#include "utils.h"

#pragma warning(disable:4996)

#ifndef INT_MAX
#define INT_MAX 32767
#endif




#ifndef _BOOLEAN_TYPE_
#define _BOOLEAN_TYPE_
typedef enum {False, True} Boolean;
#endif


#define LINELEN   1024


/* switches used in the interface routines */
#define INT         0
#define FLOAT       1
#define ENUM        2      /* interger values that use #defines */
#define BOOLE       3
#define GETTRAINING 4
#define GETTEST     5
#define GO          6
#define INT_NO      7      /* parameters only good in netfile */
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


namespace KKMachineLearning
{
  #if  !defined(_CLASSPROB_)
  class  ClassProb;
  typedef  ClassProb*  ClassProbPtr;
  class  ClassProbList;
  typedef  ClassProbList*  ClassProbListPtr;
  #endif


  class  UsfCasCor
  {
  public:
    UsfCasCor (FileDescPtr           _fileDesc,
               volatile const bool&  _cancelFlag,
               RunLog&               _log
              );

    ~UsfCasCor ();

    void  CleanUpMemory ();

    template<typename T>
    void  Delete2DArray (T**     &A,    
                         uint32  numRows
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

    int32  MemoryConsumedEstimated ()  const;


    FeatureVectorListPtr  FilterOutExtremeExamples (FeatureVectorListPtr  trainExamples);

    void  TrainNewClassifier (int32                  _in_limit,
                              int32                  _out_limit,
                              int32                  _number_of_rounds,
                              int32                  _number_of_trials,
                              kkint64                _the_random_seed,
                              bool                   _useCache,
                              FeatureVectorListPtr   _trainData,
                              const FeatureNumList&  _selectedFeatures
                             );

    void  LoadExistingClassifier (istream&  i,
                                  bool&     valid
                                 );

    MLClassPtr  PredictClass (FeatureVectorPtr  example);

    void  PredictConfidences (FeatureVectorPtr       example,
                              MLClassPtr          knownClass,
                              MLClassPtr&         predClass1,
                              float&                 predClass1Prob,
                              MLClassPtr&         predClass2,
                              float&                 predClass2Prob,
                              float&                 knownClassProb,
                              const MLClassList&  classOrder,      /**< Dictates the order in which 'probabilities' will be populatd. */
                              VectorFloat&           probabilities
                             );


    ClassProbListPtr  PredictClassConfidences (FeatureVectorPtr  example);

    void  WriteXML (ostream&  o);

    void  WriteXmlConnections (ostream&  o);

    void  ReadXml (istream&  i,
                   bool&     valid
                  );


    void  ReadXmlArrayFloat (XmlTagPtr  tag, 
                             istream&   i
                            );

    void  ReadXmlArrayFloat2D (XmlTagPtr  tag, 
                               istream&   i
                              );

    void  ReadXmlArrayFloat2DVarying (XmlTagPtr  tag, 
                                      istream&   i
                                     );

    void  ReadXmlArrayInt (XmlTagPtr  tag, 
                           istream&   i
                          );

    void  ReadXmlConnections (XmlTagPtr  tag, 
                              istream&   i
                             );

    void  ReadXmlNameValueLine (istream& i);


    /* Global variables */
    static  const char*  version;
    static  const char*  release_date;
    static  const char*  progname;
    
    /* Functions */
    char*  parm_to_string (int k);

    char const *  boolean_to_string (Boolean var);

    Boolean  string_to_boolean (const char* s);

    const char*  type_to_string (int var);

    int    string_to_type (const char* s);

    int    process_line (char *line);

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

    void  setup_network (FeatureVectorListPtr  trainExamples);

    void  train_network    ();
    void  allocate_network ();


    //****************************************************************
    //*                            'parms.c'                         *
    //****************************************************************
    static  const char*  type_strings[];
    static  const char*  boolean_strings[2];

    char parm_string[LINELEN];

    void     strdncase (char *s);

    int      find_key (char *searchkey);

    int      _type_convert (char *input);

    Boolean  _boolean_convert (char *input);




    //**********************************************************************************
    //*                                 util.c                                         *
    //**********************************************************************************
    float  random_weight ();


    //******************************************************************************************
    //*                                   load_namesfile.c                                     *
    //******************************************************************************************
    void  load_namesfile (FeatureVectorListPtr  trainExamples,
                          FeatureNumListPtr     selectedFeatures
                         );




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

    int  TRAIN_OUTPUTS (int max_epochs);
   
    void  INIT_CANDIDATES ();

    void  INSTALL_NEW_UNIT ();

    void  COMPUTE_CORRELATIONS ();

    void ADJUST_CORRELATIONS ();

    void  COMPUTE_SLOPES ();

    void  UPDATE_INPUT_WEIGHTS ();

    void  TRAIN_INPUTS_EPOCH ();

    void  CORRELATIONS_EPOCH ();

    int   TRAIN_INPUTS (int max_epochs);

    void  LIST_PARAMETERS ();

    int  TRAIN (int  outlimit, 
                int  inlimit, 
                int  rounds
               );

    void  TEST_EPOCH (double test_threshold);

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


    void  load_data (FeatureVectorListPtr  trainExamples);

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
    
    float SigmoidMax;              /**< Maximum output vaule for sigmoid units. Used to alter sigmoid range without having to edit *
                                    * training values.  Use the symbols "min" and  "max" in the input file.  The input routines    * 
                                    * will translate to the appropriate float values.
                                    */
   
    float SigmoidMin;              /**< Minimum output vaule for sigmoid units.                                            */
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
    float InputMu;	               /**< Mu used to quickprop train input weights.  */
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
                                     * Sum is stored seperately for each output.  Values are converted to 
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
    int      Ncandidates;	        /**< Nu mber of candidate units trained at once. */
    int      MaxCases;		        /**< Maxi mum number of training cases that can be accommdated by the current data structures.  */
    int      Ncases;		          /**< Number  of training cases currently in use. Assume a contiguous block beginning with   */
    int      FirstCase;	          /**< Addres s of the first training case in the currently active set.  Usually zero, but may
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
    float    ErrorIndex;               /**< Normalized error function for continuos output training sets. */
    float    ErrorIndexThreshold;      /**< Stop training when ErrorIndex is < EIT. */
    int      ErrorMeasure;             /**< Set to BITS for using ErrorBits to stop of INDEX to use ErrorIndex to stop.  */

    /***************************************************************************/
    /* Save and plot file related varibles                                     */
    /***************************************************************************/
    FILE*    WeightFile;	             /**< Contains weights from the current net. */

    int      Nparameters;
    Boolean  InterruptPending;         /**< TRUE => user has pressed Control-C */
    PARMS    ParmTable[37];

    void   ConstructParmTable ();

    MLClassListPtr  classes;            /**<  Classes that the training data consisted of.  */

    FileDescPtr        fileDesc;

    FeatureNumListPtr  selectedFeatures;   /**< The selected features that are to be used from the source training data. */

    RunLog&  log;

    volatile const bool&  cancelFlag;

  };  /* UsfCasCor */

  typedef  UsfCasCor*  UsfCasCorPtr;

} /* MLL */

#endif
