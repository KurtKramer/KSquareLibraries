#ifndef  _MODEL_
#define  _MODEL_
/**
 @class  KKMLL::Model
 @brief Base class to all Learning Algorithms.
 @author  Kurt Kramer
 @details
   Base class to be used by all Learning Algorithm Models. The idea is that all learning algorithms 
   all follow the same basic flow.  The two obvious functions that they all support are Training from 
   supplied labeled examples(List of FeatureVector objects),  Prediction of an unlabeled example.
 */

#include "DateTime.h"
#include "KKBaseTypes.h"
#include "KKStr.h"
#include "RunLog.h"


#include "ModelParam.h"


namespace KKMLL
{
  #if  !defined(_CLASSPROB_)
  class  ClassProb;
  typedef  ClassProb*  ClassProbPtr;
  class  ClassProbList;
  typedef  ClassProbList*  ClassProbListPtr;
  #endif

  #if  !defined(_FEATUREENCODER2_)
  class  FeatureEncoder2;
  typedef  FeatureEncoder2*  FeatureEncoder2Ptr;
  #endif


  #ifndef  _FeatureNumListDefined_
  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;
  typedef  FeatureNumList const  FeatureNumListConst;
  typedef  FeatureNumListConst*  FeatureNumListConstPtr;
  #endif


  #if  !defined(_FEATUREVECTOR_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined(_FileDesc_Defined_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif


  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  typedef  const MLClass  MLClassConst;
  typedef  MLClassConst*  MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  class  MLClassIndexList;
  typedef  MLClassIndexList*  MLClassIndexListPtr;
  #endif


  #if  !defined(_NORMALIZATIONPARMS_)
  class  NormalizationParms;
  typedef  NormalizationParms*  NormalizationParmsPtr;
  #endif



  class  Model
  {
  public:
    typedef  Model*  ModelPtr;

    enum  ModelTypes {mtNULL = 0, mtOldSVM = 1, mtSvmBase = 2 , mtKNN= 3, mtUsfCasCor = 4, mtDual = 5};
    static KKStr       ModelTypeToStr   (ModelTypes    _modelingType);
    static ModelTypes  ModelTypeFromStr (const KKStr&  _modelingTypeStr);


    /**
     *@brief  Use this when you are planning on creating a empty model without parameters.
     */
    Model (FileDescPtr    _fileDesc,
           VolConstBool&  _cancelFlag,
           RunLog&        _log
          );


    /**
     *@brief  Construct a instance of 'Model' using the parameters specified in '_param'.
     *@param[in]  _name Name of training model.
     *@param[in]  _param  Parameters for learning algorithm; we will create a duplicate copy.
     *@param[in]  _fileDesc Description of the dataset that will be used to train the classifier and examples that will be classified.
     *@param[in]  _cancelFlag  Will monitor; if at any point it turns true this instance is to terminate and return to caller.
     *@param[in,out]  _log  Logging file.
     */
    Model (const KKStr&       _name,
           const ModelParam&  _param,         // Create new model from
           FileDescPtr        _fileDesc,
           VolConstBool&      _cancelFlag,
           RunLog&            _log
          );

  
    /**
     *@brief Copy Constructor.
     */
    Model (const Model&   _madel);

    virtual  ~Model ();


    /**
     *@brief  A factory method that will instantiate the appropriate class of training model based off '_modelType'.
     *@details  This method is used to construct a model that is going to be built from training data.
     *@param[in] _modelType  Type of model to be created; ex: mtOldSVM, mtSvmBase, or  mtKNN.
     *@param[in] _name
     *@param[in] _param  Parameters used to drive the creating of the model.
     *@param[in] _fileDesc Description of the dataset that will be used to train the classifier and examples that will be classified.
     *@param[in] _cancelFlag  Will monitor; if at any point it turns true this instance is to terminate and return to caller.
     *@param[in,out]  _log  Logging file.
     */
    static 
      ModelPtr  CreateAModel (ModelTypes         _modelType,
                              const KKStr&       _name,
                              const ModelParam&  _param,      /**< Will make a duplicate copy of */
                              FileDescPtr        _fileDesc,
                              VolConstBool&      _cancelFlag,
                              RunLog&            _log
                             );
  
    /**
     *@brief  A factory method that will instantiate the appropriate class of training model based off the contents of the istream "i".
     *@details  This method is used to construct a model that has already been built and saved to disk.
     *@param[in] i  Input stream where previously built model has been saved.
     *@param[in] _param  Parameters used to drive the creating of the model.
     *@param[in] _fileDesc Description of the dataset that will be used to train the classifier and examples that will be classified.
     *@param[in]  _cancelFlag  Will monitor; if at any point it turns true this instance is to terminate and return to caller.
     *@param[in,out]  _log  Logging file.
     */
    static
      ModelPtr  CreateFromStream (istream&       i,
                                  FileDescPtr    _fileDesc,
                                  VolConstBool&  _cancelFlag,
                                  RunLog&        _log
                                 );
    virtual
    ModelPtr                 Duplicate () const = 0;


    // Access Methods
    bool                              AlreadyNormalized          () const {return alreadyNormalized;}

    virtual
    KKStr                    Description ()  const;  /**< Return short user readable description of model. */

    const FeatureEncoder2&            Encoder                    () const;

    virtual FeatureNumListConstPtr    GetFeatureNums             () const;

    virtual kkint32                   MemoryConsumedEstimated    () const;

    virtual ModelTypes                ModelType                  () const = 0;

    virtual KKStr                     ModelTypeStr               () const  {return ModelTypeToStr (ModelType ());}

    const KKStr&                      Name                       () const  {return name;}
    void                              Name (const KKStr&  _name)  {name = _name;}

    virtual bool                      NormalizeNominalAttributes () const; /**< Return true, if nominal fields need to be normalized. */

    ModelParamPtr                     Param                      () const  {return  param;}

    virtual FeatureNumListConstPtr    SelectedFeatures           () const;

    const KKStr&                      RootFileName               () const {return rootFileName;}

    const KKB::DateTime&              TimeSaved                  () const {return timeSaved;}

    double                            TrainingTime               () const {return trainingTime;}

    double                            TrianingPrepTime           () const {return trianingPrepTime;}  //*< Time ins secs spent preparing training data in Model::TrainModel */

    bool                              ValidModel                 () const {return validModel;}


    // Access Update Methods
    void  RootFileName (const KKStr&  _rootFileName)  {rootFileName = _rootFileName;}
  


    /**
     @brief  Derived classes call this method to start the clock for 'trainingTime'.
     */
    void  TrainingTimeStart ();

    /**
     @brief  Derived classes call this method to stop the clock for 'trainingTime'.
     */
    void  TrainingTimeEnd ();



    void  Load (const KKStr& _rootFileName,
                bool&        _successful
               );


    /**
     *@brief  Every prediction  method in every class that is inherited from this class should call
     *        this method before performing there prediction.  Such things as Normalization and
     *        Feature Encoding.
     *@param[in]  fv  Feature vector of example that needs to be prepared.
     *@param[out]  newExampleCreated  Indicates if either Feature Encoding and/or Normalization needed
     *             to be done.  If neither then the original instance is returned.  If Yes then 
     *             a new instance which the caller will have to delete will be returned.
     */
    virtual
    FeatureVectorPtr         PrepExampleForPrediction (FeatureVectorPtr  fv,
                                                       bool&             newExampleCreated
                                                      );


    /**
     * @brief  Expects to read in the entire contents of a previously trained model into
     *  this instance. One of the first lines to be read will contain the specific
     *  type of model to be read.  To update the fields that are particular to
     *  the specialized class the method 'ReadSpecificImplementationXML' will be
     *  called.
     */
    virtual  
    void  ReadXML (istream&  i,
                   bool&     _successful
                  ); 

    virtual  
    void  WriteXML (ostream&  o);


    void  Save (const KKStr& _rootFileName,
                bool&        _successful
               );
  

    virtual  
    void  WriteSpecificImplementationXML (ostream&  o) = 0;


    virtual  void  PredictRaw (FeatureVectorPtr  example,
                               MLClassPtr     &  predClass,
                               double&           dist
                              )
    {
      predClass = NULL;
      dist = 0.0;
    }

    //*********************************************************************
    //*     Routines that should be implemented by descendant classes.    *
    //*********************************************************************

    virtual
    MLClassPtr  Predict (FeatureVectorPtr  image) = 0;
  
    virtual
    void        Predict (FeatureVectorPtr  example,
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
                        ) = 0;


   virtual 
   ClassProbListPtr  ProbabilitiesByClass (FeatureVectorPtr  example) = 0;


    /**@brief  Only applied to ModelDual classifier. */
    virtual
    void  ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                    KKStr&             classifier1Desc,
                                    KKStr&             classifier2Desc,
                                    ClassProbListPtr&  classifier1Results,
                                    ClassProbListPtr&  classifier2Results
                                   );


    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    example,
                                const MLClassList&  _mlClasses,
                                kkint32*            _votes,
                                double*             _probabilities
                               ) = 0;

    /**
     *@brief Derives predicted probabilities by class.
     *@details Will get the probabilities assigned to each class by the classifier.  The 
     *        '_mlClasses' parameter dictates the order of the classes. That is the 
     *        probabilities for any given index in '_probabilities' will be for the class
     *        specified in the same index in '_mlClasses'.
     *@param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _mlClasses  List of image classes that caller is aware of. This should be the
     *            same list that was used when constructing this Model object.  The list must
     *            be the same but not necessarily in the same order as when Model was 1st
     *            constructed.  The ordering of this list will dictate the order that '_probabilities'
     *            will be populated.
     *@param[out] _probabilities An array that must be as big as the number of classes in
     *            '_mlClasses'.  The probability of class in '_mlClasses[x]' will be 
     *            returned in probabilities[x].
     */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    _example,
                                const MLClassList&  _mlClasses,
                                double*             _probabilities
                               ) = 0;
  

    virtual  
    void  ReadSpecificImplementationXML (istream&  i,
                                         bool&     _successful
                                        ) = 0; 


    virtual  
    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**      crossProbTable  /**< two dimension matrix that needs to be classes.QueueSize ()  squared. */
                                 );

    /**
     *@brief Performs operations such as FeatureEncoding, and  Normalization.  The actual training
     *  of models occurs in the specific derived implementation of 'Model'.
     *@param[in] _trainExamples  Training data that classifier will be built from.  If the examples need to be
     *                           normalized or encoded and we are not taking ownership then a duplicate list of
     *                           examples will be created that this method and class will be free to modify.
     *@param[in] _alreadyNormalized  Indicates if contents of '_trainExamples' are normalized already; if not
     *                               they will be normalized.
     *@param[in] _takeOwnership  This instance of Model will take ownership of '_examples' and is free to 
     *                           modify its contents.
     */
    virtual  
    void  TrainModel (FeatureVectorListPtr  _trainExamples,
                      bool                  _alreadyNormalized,
                      bool                  _takeOwnership  
                     );


  protected:
    void  AllocatePredictionVariables ();


    void  DeAllocateSpace ();


    void  NormalizeProbabilitiesWithAMinumum (kkint32  numClasses,
                                              double*  probabilities,
                                              double   minProbability
                                             );


    void  Read         (istream& i,
                        bool&    _successful
                       );

    void  ReadSkipToSection (istream& i, 
                             KKStr    sectName,
                             bool&    sectionFound
                            );

    void  ReduceTrainExamples ();



    bool                   alreadyNormalized;

    VolConstBool&          cancelFlag;

    MLClassListPtr         classes;

    MLClassIndexListPtr    classesIndex;

    double*                classProbs;
 
    double**               crossClassProbTable;   /*!< Probabilities  between Binary Classes From last Prediction */

    kkint32                crossClassProbTableSize;

    FeatureEncoder2Ptr     encoder;

    FileDescPtr            fileDesc;

    RunLog&                log;

    NormalizationParmsPtr  normParms;

    kkint32                numOfClasses;   /**< Number of Classes defined in crossClassProbTable. */

    ModelParamPtr          param;          /**< Will own this instance,                           */

    KKStr                  rootFileName;   /**< This is the root name to be used by all component objects; such as svm_model,
                                            * mlClasses, and svmParam(including selected features). Each one will have the
                                            * same rootName with a different suffix.
                                            *@code
                                            *      mlClasses  "<rootName>.image_classes"
                                            *      svmParam      "<rootName>.svm_parm"
                                            *      model         "<rootName>"
                                            *@endcode
                                            */

    FeatureVectorListPtr   trainExamples;

    bool                   validModel;

    kkint32*               votes;

    bool                   weOwnTrainExamples;  /**< Indicates if we own the 'trainExamples'. This does not mean that we own its
                                                 * contents. That is determined by 'trainExamples->Owner ()'.
                                                 */
   

  private:
    double                 trianingPrepTime;    /**<  Time that it takes to perform normalization, and encoding */
    double                 trainingTime;
    double                 trainingTimeStart;   /**<  Time that the clock for TraininTime was started. */
    KKStr                  name;
    KKB::DateTime          timeSaved;           /**<  Date and Time that this model was saved. */
  };
  
  typedef  Model::ModelPtr  ModelPtr;
  
}  /* namespace MML */

#endif




