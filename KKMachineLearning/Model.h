#ifndef  _MODEL_
#define  _MODEL_
/**
 @class  KKMachineLearning::Model
 @brief Base class to all Learning Algorithms.
 @author  Kurt Kramer
 @details
   Base class to be used by all Learning Algorithm Models. The idea is that all learning algorithms 
   all follow the same basic flow.  The two obvious functions that they all support are Training from 
   supplied labeled examples(List of FeatureVector objects),  Prediction of an unlabeled example.
 */

#include "RunLog.h"
#include "KKStr.h"

#include "ClassProb.h"
#include "FeatureEncoder2.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "ModelParam.h"
#include "NormalizationParms.h"



namespace KKMachineLearning
{
  #ifndef  _FeatureNumListDefined_
  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;
  #endif


  #ifndef _FileDesc_Defined_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif



  class  Model
  {
  public:
    typedef  Model*  ModelPtr;

    typedef  enum  {mtNULL, mtOldSVM, mtSvmBase, mtKNN, mtUsfCasCor}   ModelTypes;
    static KKStr       ModelTypeToStr   (ModelTypes    _modelingType);
    static ModelTypes  ModelTypeFromStr (const KKStr&  _modelingTypeStr);


/*
    class  ClassPairProb
    {
    public:
      ClassPairProb (MLClassPtr _classLabel,
                     double        _probability
                    );

      ClassPairProb (const ClassPairProb&  _pair);

      MLClassPtr classLabel;
      double        probability;
    };
    typedef  ClassPairProb*  ClassPairProbPtr;



    class ClassPairProbList:  public  KKQueue<ClassPairProb>
    {
    public:
      ClassPairProbList ();
      ClassPairProbList (bool owner);
      ClassPairProbList (const ClassPairProbList&  pairList);
      void  SortByClassName ();
      void  SortByProbability (bool highToLow = true);
    private:
      static bool  CompairByClassName (const ClassPairProbPtr left, const ClassPairProbPtr right);

      class  ProbabilityComparer;
    };
    typedef  ClassPairProbList*  ClassPairProbListPtr;

*/


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
     *@details  This method is used to construct a model that has already been build and saved to disk.
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
    const FeatureEncoder2&            Encoder                    () const;
    virtual const FeatureNumList&     GetFeatureNums             () const;
    virtual kkint32                   MemoryConsumedEstimated    () const;
    virtual ModelTypes                ModelType                  () const = 0;
    virtual KKStr                     ModelTypeStr               () const  {return ModelTypeToStr (ModelType ());}
    const KKStr&                      Name                       () const  {return name;}
    virtual bool                      NormalizeNominalAttributes () const; /**< Return true, if nominal fields need to be normalized. */
    ModelParamPtr                     Param                      () const  {return  param;}
    virtual const FeatureNumList&     SelectedFeatures           () const;
    const KKStr&                      RootFileName               () const {return rootFileName;}
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





    //*********************************************************************
    //*     Routines that should be implemented by descendant classes.    *
    //*********************************************************************

    virtual
    MLClassPtr      Predict (FeatureVectorPtr  image) = 0;
  
    virtual
    void               Predict (FeatureVectorPtr  example,
                                MLClassPtr     knownClass,
                                MLClassPtr&    predClass1,
                                MLClassPtr&    predClass2,
                                kkint32&          predClass1Votes,
                                kkint32&          predClass2Votes,
                                double&           probOfKnownClass,
                                double&           probOfPredClass1,
                                double&           probOfPredClass2,
                                kkint32&          numOfWinners,
                                bool&             knownClassOneOfTheWinners,
                                double&           breakTie
                               ) = 0;


   virtual 
   ClassProbListPtr  ProbabilitiesByClass (FeatureVectorPtr  example) = 0;


    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr       example,
                                const MLClassList&  _mlClasses,
                                kkint32*                   _votes,
                                double*                _probabilities
                               ) = 0;

    /**
     *@brief Derives predicted probabilities by class.
     *@details Will get the probabilities assigned to each class by the classifier.  The 
     *        '_mlClasses' parameter dictates the order of the classes. That is the 
     *        probabilities for any given index in '_probabilities' will be for the class
     *        specified in the same index in '_mlClasses'.
     *@param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _ImageClasses  List of image classes that caller is aware of.  This should be the
     *            same list that was used when constructing this Model object.  The list must
     *            be the same but not necessarily in the same order as when Model was 1st
     *            constructed.  The ordering of this list will dictate the order that '_probabilities'
     *            will be populated.
     *@param[out] _probabilities An array that must be as big as the number of classes in
     *            '_mlClasses'.  The probability of class in '_mlClasses[x]' will be 
     *            returned in probabilities[x].
     */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr       _example,
                                const MLClassList&  _mlClasses,
                                double*                _probabilities
                               ) = 0;
  

    virtual  
    void  ReadSpecificImplementationXML (istream&  i,
                                         bool&     _successful
                                        ) = 0; 


    virtual  
    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**         crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
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

    ClassIndexListPtr      classesIndex;

    double*                classProbs;
 
    double**               crossClassProbTable;   /*!< Probabilities  between Binary Classes From last Prediction */

    kkint32                crossClassProbTableSize;

    FeatureEncoder2Ptr     encoder;

    FileDescPtr            fileDesc;

    RunLog&                log;

    KKStr                  name;

    kkuint32               numOfClasses;   /**< Number of Classes defined in crossClassProbTable. */

    NormalizationParmsPtr  normParms;

    ModelParamPtr          param;          /**< Will own this instance                            */

    KKStr                  rootFileName;   /**< This is the root name to be used by all component objects; such as svm_model, mlClasses, and
                                            * svmParam(including selected features).  Each one will have the same rootName with a different Suffix
                                            *      mlClasses  "<rootName>.image_classes"
                                            *      svmParam      "<rootName>.svm_parm"
                                            *      model         "<rootName>"
                                            */

    FeatureVectorListPtr   trainExamples;

    bool                   validModel;

    kkint32*                 votes;

    bool                   weOwnTrainExamples;

  private:
    double                 trianingPrepTime;    /**<  Time that it takes to perform normalization, and encoding */
    double                 trainingTime;
    double                 trainingTimeStart;   /**<  Time that the clock for TraininTime was started. */
  };
  
  typedef  Model::ModelPtr  ModelPtr;
  
}  /* namespace MML */

#endif




