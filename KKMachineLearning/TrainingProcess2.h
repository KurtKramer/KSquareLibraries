#ifndef  _TRAININGPROCESS2_
#define  _TRAININGPROCESS2_

#include <ostream>

#include "XmlStream.h"

#include "Model.h"
#include "SVMModel.h"
#include "TrainingConfiguration2.h"



namespace KKMLL 
{
  #if  !defined(_FEATUREVECTOR_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined(FileDesc_Defined_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif


  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*  MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif


  #if  !defined(_MODELOLDSVM_)
  class  ModelOldSVM;
  typedef  ModelOldSVM*  ModelOldSVMPtr;
  #endif

  #if  !defined(_TrainingConfiguration2_Defined_)
  class  TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  typedef  TrainingConfiguration2  const  TrainingConfiguration2Const;
  typedef  TrainingConfiguration2Const*  TrainingConfiguration2ConstPtr;
  #endif

  class  TrainingProcess2List;
  typedef  TrainingProcess2List*  TrainingProcess2ListPtr;


  class  TrainingProcess2
  {
  public:
    typedef  TrainingProcess2*  TrainingProcess2Ptr;

    enum class  WhenToRebuild
    {
      AlwaysRebuild,
      NotUpToDate,
      NotValid,
      NeverRebuild
    };


    /**
     *@brief  Creates a TrainingPorcess based off a specified configuration; depending on the '_whenToRebuild' parameter
     * and the current status of the corresponding "save" file will either load existing trained classifier or build a 
     * new one from scratch.
     *@param[in]  _config  A previously loaded configuration file that specifies directories where 
     *                     example images for each class can be found.  Caller will still own 'config'
     *                     and be responsible for deleting it.
     *
     *@param[in] _checkForDuplicates  If set to true will look for duplicates in the training data. Two
     *                                FeatureVectors will be considered duplicate if they have the Same
     *                                ExampleFileName or the save Feature Values.  If duplicates are in 
     *                                the same class then all but one will be removes.  If they are
     *                                in more then one class then they will both be removed.
     *@param[in]  _whenToRebuild
     *
     *@param[in]  _saveTrainedModel   Specifies whether to the TrainingPorcess if it needs to be trained.
     *
     *@param[in]  _cancelFlag
     *
     *@param[in]  _log   Logging file.
     */
    static
    TrainingProcess2Ptr  CreateTrainingProcess (TrainingConfiguration2Const*  config,
                                                bool                          checkForDuplicates,
                                                WhenToRebuild                 whenToRebuild,
                                                bool                          saveTrainedModel,
                                                VolConstBool&                 cancelFlag,
                                                RunLog&                       log
                                               );
    


    /**
     *@brief  Build a new model from scratch for the specified class level. Will also remove duplicates.
     *
     *@details  Using the parameter '_level' will construct a classifier that groups classes together
     *          by group.  Underscore characters in the class name will be used to differentiate group
     *          levels.  Ex:  Crustacean_Copepod_Calanoid has three levels of grouping where 'Crustacean'
     *          belongs to level 1.
     *
     *@param[in]  _config  Configuration that will provide parameters such as classes and their related 
     *                     directories where training examples are found.
     *
     *@param[in]  _level  The grouping level to build a classifier for.  Ex: if _level = 2 is specified
     *                   and referring to the class name "Crustacean_Copepod_Calanoid" above all classes
     *                   that start with "Crustacean_Copepod_" will be combined as one logical class.
     *
     *@param[in]  _log  Logging file.
     *
     */
    static
    TrainingProcess2Ptr  CreateTrainingProcessForLevel (TrainingConfiguration2Const*  config,
                                                        kkuint32                      level,
                                                        VolConstBool&                 cancelFlag,
                                                        RunLog&                       log
                                                       );

    static
    TrainingProcess2Ptr  CreateTrainingProcessForLevel (const KKStr&   configFileName,
                                                        kkuint32       level,
                                                        VolConstBool&  cancelFlag,
                                                        RunLog&        log
                                                       );


    /**
     *@brief  Constructor that gets its training data from a list of examples provided in one of the parameters.
     *@param[in]  _config  A configuration that is already loaded in memory.
     *@param[in]  _trainingExamples  Training data to train classifier with.
     *@param[in]  _featuresAlreadyNormalized  If set to true will assume that all features in the
     *                                       training data are normalized.
     *@param[in]  _log   Logging file.
     */
    static
    TrainingProcess2Ptr  CreateTrainingProcessFromTrainingExamples (TrainingConfiguration2Const* config, 
                                                                    FeatureVectorListPtr         trainingExamples,
                                                                    bool                         takeOwnershipOfTrainingExamples,
                                                                    bool                         featuresAlreadyNormalized,
                                                                    VolConstBool&                cancelFlag,
                                                                    RunLog&                      log
                                                                   );



    static
    TrainingProcess2Ptr  LoadExistingTrainingProcess (const KKStr&   configRootName,
                                                      VolConstBool&  cancelFlag,
                                                      RunLog&        log
                                                     );



    /**
     *@brief  The default constructor; What will be used when creating an instance while reading in
     * from a XML Stream file.  All members will be set to default values.  The XMLRead method 
     */
    TrainingProcess2 ();


    virtual
    ~TrainingProcess2 ();

    kkint32  MemoryConsumedEstimated ()  const;



    /**
     *@brief Call this method just after you construct a new instance of "TrainingProcess2"
     *@param[in]  _config
     *@param[in]  _whenToRebuild     Used for any sub classifiers that this instance of TrainingProcess2 might need to build.
     *@param[in]  _trainingExamples  
     *@param[in]  _takeOwnerShipOfTrainingExamples  If true this instance of 'TrainingProcess2' will take ownership of '_trainingExamples' and delete it when done with them.
     *@param[in]  _checkForDuplicates  If true will remove duplicate examples from '_trainingExamples'.
     *@param[in]  _cancelFlag   
     *@param[in]  _log
     */
    void  BuildTrainingProcess (TrainingConfiguration2Const*  _config,
                                WhenToRebuild                 _whenToRebuild,
                                FeatureVectorListPtr          _trainingExamples,
                                bool                          _takeOwnerShipOfTrainingExamples,
                                bool                          _checkForDuplicates,
                                VolConstBool&                 _cancelFlag,
                                RunLog&                       _log
                               );


    // Access Members
    void  Abort (bool _abort)  {abort = _abort;}

    bool                          Abort                     () const  {return abort;}
    const KKB::DateTime&          BuildDateTime             () const  {return buildDateTime;}
    TrainingConfiguration2Const*  Config                    ()        {return config;}    
    const KKStr&                  ConfigFileName            () const  {return configFileName;}
    VectorKKStr                   ConfigFileFormatErrors    () const;
    kkint32                       DuplicateCount            () const  {return duplicateCount;}
    kkint32                       DuplicateDataCount        () const  {return duplicateDataCount;}
    bool                          FeaturesAlreadyNormalized () const  {return featuresAlreadyNormalized;}
    FactoryFVProducerPtr          FvFactoryProducer         () const  {return fvFactoryProducer;}
    FeatureVectorListPtr          Images                    ()        {return trainingExamples;}
    MLClassListPtr                MLClasses                 () const  {return mlClasses;}
    Model::ModelTypes             ModelType                 () const;
    KKStr                         ModelTypeStr              () const;
    KKStr                         ModelDescription          () const;
    SVMModelPtr                   Model3                    ();
    kkint32                       NumOfSupportVectors       () const;
    ModelOldSVMPtr                OldSVMModel               () const;
    ClassProbList const *         PriorProbability          () const  {return  priorProbability;}
    ModelParamPtr                 Parameters                () const;
    TrainingProcess2ListPtr       SubTrainingProcesses      () const  {return subTrainingProcesses;}
    ModelPtr                      TrainedModel              () const  {return model;}
    double                        TrainingTime              () const;



    void  FeaturesAlreadyNormalized (bool _featuresAlreadyNormalized)  {featuresAlreadyNormalized = _featuresAlreadyNormalized;}


    void  CreateModelsFromTrainingData (WhenToRebuild   whenToRebuild,
                                        VolConstBool&   cancelFlag,
                                        RunLog&         log
                                       );


    /**@brief Extracts the list of classes including ones from Sub-Classifiers */
    MLClassListPtr  ExtractFullHierachyOfClasses ()  const;  

    static
    FeatureVectorListPtr  ExtractTrainingClassFeatures (TrainingConfiguration2ConstPtr  config,
                                                        KKB::DateTime&                  latestImageTimeStamp,
                                                        bool&                           changesMadeToTrainingLibraries,
                                                        VolConstBool&                   cancelFlag,
                                                        RunLog&                         log
                                                       );

    void  LoadPrevTrainedOtherwiseRebuild (bool  _forceRebuild,
                                           bool  _checkForDuplicates
                                          );

    void  ReportTraningClassStatistics (std::ostream&  report);

    /**
     * @brief  Saves the built training model into the Save file in Xml Format.
     */
    void  SaveTrainingProcess (RunLog&  log);

    void  SupportVectorStatistics (kkint32&  numSVs,
                                   kkint32&  totalNumSVs
                                  );

    /**
      *@brief Returns back pointer to 1st classifier of Dual Classifier.
      *@details If not a Dual classifier will return back NULL. Keep in mind that you will
      *  not own this classifier and that it can be deleted at any time.
      */
    TrainingProcess2Ptr   TrainingProcessLeft ();

    /**
      *@brief Returns back pointer to 2nd classifier of Dual Classifier.
      *@details If not a Dual classifier will return back NULL.
      */
    TrainingProcess2Ptr   TrainingProcessRight ();

    void  ValidateConfiguration ();

    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;


  private:
    void    AddImagesToTrainingLibray (FeatureVectorList&  trainingExamples,
                                       FeatureVectorList&  examplesToAdd,
                                       RunLog&             log
                                      );

    void    BuildModel3 ();

    void    CheckForDuplicates (bool     allowDupsInSameClass,
                                RunLog&  log
                               );

    void    LoadSubClassifiers (WhenToRebuild  whenToRebuild,
                                bool           checkForDuplicates,
                                VolConstBool&  cancelFlag,
                                RunLog&        log
                               );


    static
    FeatureVectorListPtr  ExtractFeatures (TrainingConfiguration2ConstPtr  config,
                                           MLClassList&                    mlClasses,
                                           const TrainingClassPtr          trainingClass,
                                           KKB::DateTime&                  latestTimeStamp,
                                           bool&                           changesMade,
                                           VolConstBool&                   cancelFlag,
                                           RunLog&                         log
                                          );




    //************************************************************
    // Variables that are Global to TrainingProcess2 application. *
    //************************************************************

    bool                         abort;       /**< If problem building a model or loading will be set to True. */

    KKB::DateTime                buildDateTime;

    TrainingConfiguration2Const* config;
    TrainingConfiguration2*      configOurs;  /**< If we own the instance of 'config' we assign to this member as well as 'config'; the 
                                               * destructor will delete  'configOurs'  
                                               */

    KKStr                        configFileName;           /**< The directory path where this file is actually located will be added to this name. */

    KKStr                        configFileNameSpecified;  /**< This will be the ConfigFileName specified by caller before the directory
                                                            * that is added for actual location of config file.
                                                            */

    kkint32                      duplicateCount;
    kkint32                      duplicateDataCount;

    bool                         featuresAlreadyNormalized;

    FileDescPtr                  fileDesc;

    FactoryFVProducerPtr         fvFactoryProducer;

    MLClassListPtr               mlClasses;    /**< List of all classes that are to be processed. There will be one entry for each MLClass,
                                                * Including one for noise trainingExamples(unknown trainingExamples).
                                                */

    ModelPtr                     model;

    ClassProbListPtr             priorProbability;  /**< Based on Training example distribution.  */

    std::ostream*                report;

    KKStr                        savedModelName;

    TrainingProcess2ListPtr      subTrainingProcesses;

    FeatureVectorListPtr         trainingExamples;  /**< All Images Loaded. Own's all trainingExamples. All other ImageList's will only point to
                                                     * these trainingExamples.
                                                     */

    bool                         weOwnMLClasses;
    bool                         weOwnTrainingExamples;
  };  /* TrainingProcess2 */

  typedef  TrainingProcess2::TrainingProcess2Ptr  TrainingProcess2Ptr;

#define  _TrainingProcess2_Defined_

  class  TrainingProcess2List:  public  KKQueue<TrainingProcess2>
  {
  public:
    TrainingProcess2List (bool  _owner);
    virtual  ~TrainingProcess2List ();

    kkint32 MemoryConsumedEstimated ()  const;

  };

  typedef  TrainingProcess2List*  TrainingProcess2ListPtr;

#define  _TrainingProcess2List_Defined_


  // XlmStream  instances.

  typedef  XmlElementTemplate<TrainingProcess2>  XmlElementTrainingProcess2;
  typedef  XmlElementTrainingProcess2*  XmlElementTrainingProcess2Ptr;

} /* namespace KKMLL */

#endif
