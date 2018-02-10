#ifndef  _SVMMODEL_
#define  _SVMMODEL_
//***********************************************************************
//*                           SVMModel                                  *
//*                                                                     *
//*  Represents a training model for svmlib. Uses Example Feature data  *
//*  MLClasses, libsvm parameters, and included features to build       *
//*  a svm training model.  You can save this model to disk for later   *
//*  use without having to train again. An application can create       *
//*  several instances of this model for different phases of a          *
//*  classification architecture.                                       *
//***********************************************************************


#include "KKStr.h"
#include "ClassAssignments.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "FeatureVector.h"
#include "svm.h"
#include "SVMparam.h"



namespace KKMLL
{

#ifndef  _FEATURENUMLIST_
  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;
  typedef  FeatureNumList const FeatureNumListConst;
  typedef  FeatureNumListConst*  FeatureNumListConstPtr;
#endif


#ifndef  _FEATUREENCODER_
  class  FeatureEncoder;
  typedef  FeatureEncoder*  FeatureEncoderPtr;
#endif



#ifndef  _RUNLOG_
  class  RunLog;
  typedef  RunLog*  RunLogPtr;
#endif


#ifndef _FILEDESC_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
#endif



  typedef  struct svm_node*     XSpacePtr;


  class  ProbNamePair
  {
  public:
    ProbNamePair (KKStr   _name,
                  double  _probability
                  ):
        name (_name),  probability (_probability)
        {}

    KKStr   name;
    double  probability;
  };



  class  SVMModel
  {
  public:
    /**
     *@brief Default constructor used by XmlElementSVMModel to create and load a new instance from a XML Stream.
     */
    SVMModel ();
  

    /**
     *@brief  Constructor that will create a svmlib training model using the 
     *        features and classes for training purposes.
     *@param[in]  _svmParam  Specifies the parameters to be used for training.  These
     *            are the same parameters that you would specify in the command line 
     *            to svm_train. Plus the feature numbers to be  used.
     *@param[in]  _examples  Training data for the classifier.
     *@param[in]  _assignments List of classes and there associated number ti be 
     *            used by the SVM.  You can merge 1 or more classes by assigning them 
     *            the same number. This number will be used by the SVM.  When 
     *            predictions are done by SVM it return a number, _assignments will 
     *            then be used to map back-to the correct class.
     *@param[in] _fileDesc  File-Description that describes the training data.
     *@param[out] _log Log file to log messages to.
     */
    SVMModel (const SVMparam&     _svmParam,
              FeatureVectorList&  _examples,
              ClassAssignments&   _assignments,
              FileDescConstPtr    _fileDesc,
              RunLog&             _log
             );


    /**
     *@brief Frees any memory allocated by, and owned by the SVMModel
     */
    virtual ~SVMModel ();

    virtual  void   CancelFlag (bool  _cancelFlag);

    FeatureNumListConstPtr   GetFeatureNums ()  const;

    FeatureNumListConstPtr   GetFeatureNums (FileDescPtr  fd)  const;

    FeatureNumListConstPtr   GetFeatureNums (FileDescPtr  fd,
                                             MLClassPtr   class1,
                                             MLClassPtr   class2
                                            )  const;

    //MLClassList&  MLClasses ()  {return mlClasses;}  
    const ClassAssignments&  Assignments () const  {return assignments;}

    //kkint32              DuplicateDataCount () const {return duplicateCount;}

    kkMemSize          MemoryConsumedEstimated ()  const;

    virtual
    bool               NormalizeNominalAttributes ();  /**< Return true, if nominal fields need to be normalized.  */

    kkint32            NumOfClasses ()  const  {return numOfClasses;}

    kkint32            NumOfSupportVectors () const;

    void               SupportVectorStatistics (kkint32& numSVs,
                                                kkint32& totalNumSVs
                                               );

    SVMparam const *   SVMParameters           () const {return svmParam;}

    const KKStr&       RootFileName            () const {return rootFileName;}


    SVM_SelectionMethod   SelectionMethod      () const {return svmParam->SelectionMethod ();}

    double             TrainingTime            () const {return trainingTime;}

    bool               ValidModel              () const {return validModel;}


    double   DistanceFromDecisionBoundary (FeatureVectorPtr  example,
                                           MLClassPtr        class1,
                                           MLClassPtr        class2
                                          );


    MLClassPtr  Predict (FeatureVectorPtr  example);


    /**
     *@brief  Will predict the two most likely classes of 'example'.
     *@param[in]  example The Example to predict on.
     *@param[in]  knownClass  Class that we already no the example to be;  can be pointing to NULL indicating that you do not know.
     *@param[out] predClass1  Most likely class; depending on classifier parameters this could be by number of votes or probability.
     *@param[out] predClass2  Second most likely class.
     *@param[out] predClass1Votes Number votes 'predClass1' received.
     *@param[out] predClass2Votes Number votes 'predClass2' received.
     *@param[out] probOfKnownClass Probability of 'knownClass' if specified.
     *@param[out] predClass1Prob Probability of 'predClass1' if specified.
     *@param[out] predClass2Prob Probability of 'predClass2' if specified.
     *@param[out] numOfWinners  Number of classes that had the same number of votes as the winning class.
     *@param[out] knownClassOneOfTheWinners  Will return true if the known class was one of the classes that had the highest number of votes.
     *@param[out] breakTie The difference in probability between the classes with the two highest probabilities.
     */
    void   Predict (FeatureVectorPtr  example,
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
                   );


    /**
     *@brief  Returns the distance from the decision border of the SVM.
     */
    void  PredictRaw (FeatureVectorPtr  example,
                      MLClassPtr     &  predClass,
                      double&           dist
                     );

    /**
     *@brief  Will get the probabilities assigned to each class.
     *@param[in]  example unknown example that we want to get predicted probabilities for. 
     *@param[in]  _mlClasses  List classes that caller is aware of.  This should be the same list that 
     *                           was used when constructing this SVMModel object.  The list must be the 
     *                           same but not necessarily in the same order as when SVMModel was 1st 
     *                          constructed.
     *@param[out] _votes  An array that must be as big as the number of classes in _mlClasses.
     *@param[out] _probabilities  An array that must be as big as the number of classes in _mlClasses.  
     *                            The probability of class in _mlClasses[x] will be returned in _probabilities[x].
     *@param[in]  _log Logging file.  
     */
    void  ProbabilitiesByClass (FeatureVectorPtr    example,
                                const MLClassList&  _mlClasses,
                                kkint32*            _votes,
                                double*             _probabilities,
                                RunLog&             _log
                               );



    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and re-compute the decision boundary and probability.  It will then
     *          return the S/V's that when removed improve the probability in 'c1's 
     *          the most.
     *@param[in]  example  The Example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    std::vector<ProbNamePair>  FindWorstSupportVectors (FeatureVectorPtr  example,
                                                        kkint32           numToFind,
                                                        MLClassPtr        c1,
                                                        MLClassPtr        c2
                                                       );

 
    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and retraining a new SVM and then comparing with the new prediction results.
     *@param[in]  example  The Example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    std::vector<ProbNamePair>  FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                         kkint32           numToFind,
                                                         MLClassPtr        c1,
                                                         MLClassPtr        c2
                                                        );

 
    std::vector<KKStr>  SupportVectorNames ()  const;


    std::vector<KKStr>  SupportVectorNames (MLClassPtr  c1,
                                            MLClassPtr  c2
                                           )  const;


    /**
     *@brief  Will return the probabilities for all pairs of the classes listed in 'classes'.
     *@param[in]  classes  The Classes that you wish to get class pair probabilities for;  the order will 
     *            dictate how the two dimensional matrix 'crossProbTable' will be populated.
     *@param[out] crossProbTable  Will contain the probabilities of all the class pairs that can be formed from
     *            the list of classes in 'classes'.  'crossProbTable' will be a two dimension square matrix 
     *            size will be dictated by the number of classes in 'classes'.  Ex:  Entry[3][2] will have the 
     *            contain the probability between classes[3] and classes[2].
     *@param[in]  log Logging file.  
     */
    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**      crossProbTable,  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                  RunLog&       log
                                 );


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelReadXML,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&   varName,
                             std::ostream&  o
                            )  const;


  private:
    typedef  struct SvmModel233**   ModelPtr;

    FeatureVectorListPtr*   BreakDownExamplesByClass (FeatureVectorListPtr  examples);


    void  DeleteModels ();
    void  DeleteXSpaces ();

    void  AllocateModels ();
    void  AllocateXSpaces ();

    void  BuildClassIdxTable ();


    void  BuildCrossClassProbTable ();


    /**
     *@brief Constructs svm_problem structure from the examples passed to it. 
     *@details This is called once for each logical class that is going to be built in 
     *         the One-vs-All. For each class there will be a binary SVM where one class
     *         is pitted against all other classes.  Using the 'classesThisAssignment'
     *         parameter several classes can be grouped together as one logical class.
     *@param[in]  examples The examples to build the svm_problem(s) with
     *@param[out] prob The svm_problem structure that will be constructed
     *@param[in]  xSpace A list of pointers to the memory allocated for all of the 
     *            svm_node structures used in building the svm_problem.
     *@param[in]  classesThisAssignment The list of classes that are to be treated 
     *            as class '0' all other classes are treated as class '1'.
     *@param[in]  allClasses List of all classes;  The ones that are contained in
     *            'classesThisAssignment' will be the '0' class or the "One" in "One-Vs-All"
     *            while classes that are not in 'classesThisAssignment' will be coded as
     *            '1' or the "All" in "One-Vs-All".
     */ 
    void  BuildProblemOneVsAll (FeatureVectorList&    examples,
                                struct svm_problem&   prob,
                                XSpacePtr&            xSpace,
                                const MLClassList&    classesThisAssignment,
                                MLClassList&          allClasses,
                                ClassAssignmentsPtr&  classAssignments,
                                RunLog&               log
                               );


    /**
     *@brief Constructs svm_problem For two classes SVM. 
     *@param[in] examples The examples to build the svm_problem(s) with
     *@param[in]  class1Examples  Examples for 1st class.  
     *@param[in]  class2Examples  Examples for 2nd class.  
     *@param[in]  _svmParam  SVM Parameters used for building overall classifier.
     *@param[in]  _twoClassParms Parameters for the specific two classes in question.
     *@param[out] _encoder Based off parameters a FeatureEncoder will be built and 
     *            returned to caller.  Caller will get ownership and be responsible
     *            for deleting it.
     *@param[out] prob The Resultant two class classifier that will be built; caller
     *            will get ownership.
     *@param[out] xSpace A list of pointers to the S/V's that the built classifier 'prob'
     *            will be referring to.
     *@param[in]  class1 The first class that 'class1Examples' represent.
     *@param[in]  class2 The second class that 'class1Examples' represent.
     */ 
    void  BuildProblemBinaryCombos (FeatureVectorListPtr  class1Examples, 
                                    FeatureVectorListPtr  class2Examples, 
                                    BinaryClassParmsPtr&  _twoClassParms,
                                    FeatureEncoderPtr&    _encoder,
                                    struct svm_problem&   prob, 
                                    XSpacePtr&            xSpace, 
                                    MLClassPtr            class1, 
                                    MLClassPtr            class2,
                                    RunLog&               log
                                   );


    void  PredictProbabilitiesByBinaryCombos (FeatureVectorPtr    example,  
                                              const MLClassList&  _mlClasses,
                                              kkint32*            _votes,
                                              double*             _probabilities,
                                              RunLog&             _log
                                             );



    /**
     *@brief  calculates the number of features that will be present after encoding, and allocates
     *        predictedXSpace to accommodate the size.
     */
    void  CalculatePredictXSpaceNeeded (RunLog&  log);



    /**
     *@brief Builds a BinaryCombo svm model
     *@param[in] examples The examples to use when training the new model
     */
    void ConstructBinaryCombosModel (FeatureVectorListPtr  examples,
                                     RunLog&               log
                                    );


    /**
     *@brief Builds a OneVsAll svm model to be used by SvnLib
     *@param[in] examples The examples to use when training the new model.
     *@param[out] prob  Data structure used by SvnLib.
     */
    void ConstructOneVsAllModel (FeatureVectorListPtr examples,
                                 svm_problem&         prob,
                                 RunLog&              log
                                );


    /**
     *@brief Builds a OneVsOne svm model
     *@param[in] examples The examples to use when training the new model
     *@param[out] prob  Data structure used by SvnLib.
     */
    void ConstructOneVsOneModel (FeatureVectorListPtr  examples,
                                 svm_problem&          prob,
                                 RunLog&               log
                                );


    /**
     *@brief Converts a single example into the svm_problem format, using the method specified 
     *  by the EncodingMethod() value returned by svmParam
     *@param[in] example That we're converting
     *@param[in] row      The svm_problem structure that the converted data will be stored
     */
    kkint32  EncodeExample (FeatureVectorPtr  example,
                            svm_node*         row
                           );


    static
    bool  GreaterThan (kkint32 leftVotes,
                       double  leftProb,
                       kkint32 rightVotes,
                       double  rightProb
                      );


    static
    void  GreaterVotes (bool     useProbability,
                        kkint32  numClasses,
                        kkint32* votes,
                        kkint32& numOfWinners,
                        double*  probabilities,
                        kkint32& pred1Idx,
                        kkint32& pred2Idx
                       );


    void  InializeProbClassPairs ();


    void  SetSelectedFeatures (FeatureNumListConst&  _selectedFeatures,
                               RunLog&               _log
                              );

    void  SetSelectedFeatures (FeatureNumListConstPtr  _selectedFeatures,
                               RunLog&                 _log
                              );


    void  PredictOneVsAll (XSpacePtr    xSpace,
                           MLClassPtr   knownClass,
                           MLClassPtr   &predClass1,
                           MLClassPtr&  predClass2,
                           double&      probOfKnownClass,
                           double&      predClass1Prob,
                           double&      predClass2Prob,
                           kkint32&     numOfWinners,
                           bool&        knownClassOneOfTheWinners,
                           double&      breakTie
                          );



    void  PredictByBinaryCombos (FeatureVectorPtr  example,
                                 MLClassPtr        knownClass,
                                 MLClassPtr       &predClass1,
                                 MLClassPtr&       predClass2,
                                 kkint32&          predClass1Votes,
                                 kkint32&          predClass2Votes,
                                 double&           probOfKnownClass,
                                 double&           predClass1Prob,
                                 double&           predClass2Prob,
                                 double&           breakTie,
                                 kkint32&          numOfWinners,
                                 bool&             knownClassOneOfTheWinners
                                );


    ClassAssignments       assignments;

    FeatureEncoderPtr*     binaryFeatureEncoders;

    BinaryClassParmsPtr*   binaryParameters;      /**< only used when doing Classification with diff Feature 
                                                   * Selection by 2 class combo's
                                                   */
    volatile bool          cancelFlag;

    VectorInt32            cardinality_table;

    MLClassPtr*            classIdxTable;         /**< Supports reverse class lookUp,  indexed by ClassAssignments number,
                                                   * works with assignments.
                                                   */

    double**               crossClassProbTable;   /**< Probabilities  between Binary Classes From last Prediction */

    kkint32                crossClassProbTableSize;  /**< Dimension of each side of 'crossClassProbTable'    */

    FeatureEncoderPtr      featureEncoder;        /**< used when doing OneVsOne or OnevsAll processing
                                                   * When doing binary feature selection will use 
                                                   * binaryFeatureEncoders.
                                                   */
    FileDescConstPtr       fileDesc;

    ModelPtr*              models;

    kkint32                numOfClasses;          /**< Number of Classes defined in crossClassProbTable.  */
    kkint32                numOfModels;

    VectorInt32            oneVsAllAssignment;
    ClassAssignmentsPtr*   oneVsAllClassAssignments;

    XSpacePtr              predictXSpace;         /**< Used by Predict OneVsOne, to avoid deleting and reallocating every call. */

    kkuint32               predictXSpaceWorstCase;

    double*                probabilities;

    KKStr                  rootFileName;          /**< This is the root name to be used by all component 
                                                    * objects; such as SvmModel233, mlClasses, and
                                                    * svmParam(including selected features).  Each one
                                                    * will have the same rootName with a different Suffix
                                                    *     mlClasses  "<rootName>.example_classes"
                                                    *     svmParam   "<rootName>.svm_parm"
                                                    *     model      "<rootName>"
                                                    */

    FeatureNumListPtr      selectedFeatures;

    SVMparamPtr            svmParam;

    double                 trainingTime;

    KKMLL::AttributeTypeVector  type_table;

    bool                   validModel;

    kkint32*               votes;

    XSpacePtr*             xSpaces;    /**< There will be one xSpace structure for each libSVM classifier that has 
                                        *   to be built; for a total of 'numOfModels'. This will be the input to 
                                        *   the trainer for each one.
                                        */

    kkint32                xSpacesTotalAllocated;
  };

  typedef  SVMModel*  SVMModelPtr;



  typedef  XmlElementTemplate<SVMModel>  XmlElementSVMModel;
  typedef  XmlElementSVMModel*  XmlElementSVMModelPtr;



} /* namespace KKMLL */



#endif

