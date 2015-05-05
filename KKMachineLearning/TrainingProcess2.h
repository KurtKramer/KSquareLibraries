#ifndef  _TRAININGPROCESS2_
#define  _TRAININGPROCESS2_

#include <ostream>

#include "XmlStream.h"

#include "Model.h"
#include "SVMModel.h"
#include "TrainingConfiguration2.h"



namespace KKMLL 
{
  #if  !defined(_DataBase_Defined_)
  class  DataBase;
  typedef  DataBase*  DataBasePtr;
  #endif


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
  typedef  MLClass const  MLClassConst;
  typedef  MLClassConst*  MLClassConstPtr;
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



    /**
     *@brief  The default constructor; What will be used when creating an instance whil ereading in
     * from a XML Stream file.  All members will be set to default values.  The XMLRead methid 
     */
    TrainingProcess2 ();



    /**
     *@brief  Constructor that is driven by contents of configuration file.
     *@details  If no changes to config file or training data, will utilize an existing built model
     *          that was saved to disk earlier; otherwise will train from data in training library
     *          and save resultant training classifier to disk.
     *
     *@todo  I need to make a structure that would contain variables required to perform prediction 
     *       The idea is that we should only need to create one instance of "TrainingProcess2" for 
     *       use by multiple threads.  Since different classifiers will require different structures
     *       "TrainingProcess2" will have a factory method for creating instances of these structures.
     *       These structures would then have their own predict methods.
     *
     *@param[in]  _config  A previously loaded configuration file that specifies directories where 
     *                     example images for each class can be found.  Caller will still own 'config'
     *                     and be responsible for deleting it.
     *
     *@param[in]  _excludeList  List of Feature Vectors that are to be excluded from the TrainingData.
     *                          If will check by both ExampleFileName and FeatureValues. If this parameter
     *                          is not equal to NULL then will not save the Training model.  Will
     *                          NOT take ownership of list or its contents.  Caller still owns it.
     *
     *@param[in,out]  _log   Logging file.
     *
     *@param[in] _report  if not set to NULL will write statistics of the training process to stream.
     *
     *@param[in] _forceRebuild  If set to true will ignore existing training model and rebuild and
     *                          save new one.
     * 
     *@param[in] _checkForDuplicates  If set to true will look for duplicates in the training data. Two
     *                                FeatureVectors will be considered duplicate if they have the Same
     *                                ExampleFileName or the save Feature Values.  If duplicates are in 
     *                                the same class then all but one will be removes.  If they are
     *                                in more then one class then they will both be removed.
     *
     */
    TrainingProcess2 (TrainingConfiguration2Const*  _config,
                      FeatureVectorListPtr          _excludeList,
                      RunLog&                       _log,
                      ostream*                      _report,
                      bool                          _forceRebuild,
                      bool                          _checkForDuplicates
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
     *@param[in]  _excludeList  List of Feature Vectors that are to be excluded from the TrainingData.
     *                          If will check by both ExampleFileName and FeatureValues. If this parameter
     *                          is not equal to NULL then will not save the Training model.  Will
     *                          NOT take ownership of list or its contents.  Caller still owns it.
     *
     *@param[in,out]  _log   Logging file.
     *
     *@param[in] _level  The grouping level to build a classifier for.  Ex: if _level = 2 is specified
     *                   and referring to the class name "Crustacean_Copepod_Calanoid" above all classes
     *                   that start with "Crustacean_Copepod_" will be combined as one logical class.
     */
    TrainingProcess2 (TrainingConfiguration2 const *  _config,
                      FeatureVectorListPtr            _excludeList,
                      RunLog&                         _log,
                      kkuint32                        _level
                     );

   
   
    /**
     *@brief  Constructor Will use existing built model; will not check to see if it is up-to-date.
     *@details  If no changes to config file or training data, will utilize an existing built model
     *          that was saved to disk earlier;  otherwise will train from data in training library
     *          and save resultant training classifier to disk.
     *
     *@param[in]  _configFileName  Configuration file name where classes and SVM parameters are
     *                             specified.  The directories where example images for each class
     *                             will be specified.
     *
     *@param[in,out]  _log   Logging file.
     *
     *@param[in] _featuresAlreadyNormalized  If set to true will assume that all features in the
     *                                       training data are normalized.
     */
    TrainingProcess2 (const KKStr&  _configFileName,
                      RunLog&       _log,
                      bool          _featuresAlreadyNormalized
                     );


    /**
     *@brief  Constructor Will read existing built model from provided input stream.
     *
     *@param[in]  _in  Input stream that contains trainingProcess instance;  will process until 
     *                 first line containing "</TrainingProcess2>" is encountered.
     *
     *@param[in]  _fvFactoryProducer  
     *
     *@param[in,out]  _log   Logging file.
     *
     *@param[in] _featuresAlreadyNormalized  If set to true will assume that all features in the
     *                                       training data are normalized.
     */
    TrainingProcess2 (istream&  _in,
                      RunLog&   _log,
                      bool      _featuresAlreadyNormalized
                     );



    /**
     *@brief  Constructor that gets its training data from a list of examples provided in one of the parameters.
     *@param[in]  _config  A configuration that is already loaded in memory.
     *@param[in]  _trainingExamples  Training data to train classifier with.
     *@param[in]  _mlClasses  Class list.
     *@param[in]  _reportFile  if not set to NULL will write statistics of the training process to stream.
     *@param[in]  _fvFactoryProducer  
     *@param[in,out]  _log   Logging file.
     *@param[in] _featuresAlreadyNormalized  If set to true will assume that all features in the
     *                                       training data are normalized.
     */
    TrainingProcess2 (TrainingConfiguration2 const *  _config, 
                      FeatureVectorListPtr            _trainingExamples,
                      MLClassListPtr                  _mlClasses,
                      std::ostream*                   _reportFile,
                      RunLog&                         _log,
                      bool                            _featuresAlreadyNormalized
                     );

    virtual
    ~TrainingProcess2 ();

    kkint32  MemoryConsumedEstimated ()  const;


    /**
     * Sets the cancelFlag and then lets any objects that it owns that the cancelFlagstatus has changed 
     * by calling the their version of "CancelFlag"
     */
    void  CancelFlag (bool  _cancelFlag);



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



    void  CreateModelsFromTrainingData (RunLog&  log);

    /**@brief Extracts the list of classes including ones from Sub-Classifiers */
    MLClassListPtr  ExtractFullHierachyOfClasses ()  const;  

    void  ExtractTrainingClassFeatures (KKB::DateTime&  latestImageTimeStamp,
                                        bool&           changesMadeToTrainingLibraries,
                                        RunLog&         log
                                       );

    void  LoadPrevTrainedOtherwiseRebuild (bool  _forceRebuild,
                                           bool  _checkForDuplicates
                                          );

    void  Read (istream&  in,
                bool&     successful,
                RunLog&   log
               );

    void  ReportTraningClassStatistics (std::ostream&  report);

    void  SaveResults (RunLog&  log);

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

    void  WriteXml (ostream&  o,
                    RunLog&   log
                   );


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

    void    LoadSubClassifiers (bool     forceRebuild,
                                bool     checkForDuplicates,
                                RunLog&  log
                               );

    void    RemoveExcludeListFromTrainingData ();



    //************************************************************
    //             Routines for Extracting Features              *
    //************************************************************
    void  ExtractFeatures (const TrainingClassPtr  trainingClass,
                           KKB::DateTime&          latestTimeStamp,
                           bool&                   changesMade,
                           RunLog&                 log
                          );


    //************************************************************
    //       Routines for validating Configuration File.         *
    //************************************************************
    FeatureNumListPtr  DeriveFeaturesSelected (kkint32  sectionNum);

    TrainingClassPtr   ValidateClassConfig (kkint32  sectionNum);

    void               ValidateTrainingClassConfig (kkint32  sectionNum);

    void               ValidateModel3Section (kkint32 sectionNum);
 



    //************************************************************
    // Variables that are Global to TrainingProcess2 application. *
    //************************************************************

    bool                         abort;       /**< If problem building a model or loading will be set to True. */

    KKB::DateTime                buildDateTime;

    volatile bool                cancelFlag;  /**< A calling application can set this to true Training Process will monitor this Flag, if true will terminate. */


    TrainingConfiguration2Const* config;
    TrainingConfiguration2*      configOurs;  /**< If we own the instance of 'config' we assign to this member as well as 'config'; the 
                                               * destructor will delete  'configOurs'  
                                               */

    KKStr                        configFileName;           /**< The directory path where this file is actually located will be added to this name. */

    KKStr                        configFileNameSpecified;  /**< This will be the ConfigFileName specified by caller before the directory
                                                          * that is added for actual location of config file. */

    kkint32                      duplicateCount;
    kkint32                      duplicateDataCount;

    FeatureVectorListPtr         excludeList;  /**< If != NULL then list of trainingExamples that need to be eliminated from training data.
                                                * This would be used when classifying trainingExamples that might already contain training
                                                * data. If you wish to grade the results it would only be fare to delete these trainingExamples.
                                                */

    bool                         featuresAlreadyNormalized;

    FileDescPtr                  fileDesc;

    FactoryFVProducerPtr         fvFactoryProducer;

    MLClassListPtr               mlClasses; /**< List of all classes that are to be processed. There will be one entry for each MLClass,
                                             * Including one for noise trainingExamples(unknown trainingExamples).
                                             */

    ModelPtr                     model;

    ClassProbListPtr             priorProbability;  /**< Based on Training example distribution.  */

    std::ostream*                report;

    KKStr                        savedModelName;

    FeatureVectorListPtr         trainingExamples;  /**< All Images Loaded. Own's all trainingExamples. All other ImageList's will only point to
                                                     * these trainingExamples.
                                                     */

    TrainingProcess2ListPtr      subTrainingProcesses;
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
