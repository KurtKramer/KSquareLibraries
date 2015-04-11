#ifndef  _TRAININGPROCESS2_
#define  _TRAININGPROCESS2_

#include <ostream>

#include "Application.h"
#include "MLClass.h"
#include "FeatureVector.h"
#include "ImageFeaturesDataIndexed.h"
#include "ImageFeaturesNameIndexed.h"
#include "NormalizationParms.h"
#include "Model.h"
#include "ModelOldSVM.h"
#include "SVMModel.h"
#include "TrainingConfiguration2.h"



namespace KKMLL 
{

class  TrainingProcess2
{
public:
   /**
    *@brief  Constructor that is driven by contents of configuration file.
    *@details  If no changes to config file or training data, will utilize an existing built model
    *          that was saved to disk earlier;  otherwise will train from data in training library
    *          and save resultant training classifier to disk.
    *
    *@param[in]  _configFileName  Configuration file name where classes and SVM parameters are
    *                             specified.  The directories where example images for each class
    *                             will be specified.
    *
    *@param[in]  _excludeList  List of Feature Vectors that are to be excluded from the TrainingData.
    *                          If will check by both ImageFileName and FeatureValues. If this parameter
    *                          is not equal to NULL then will not save the Training model.  Will
    *                          NOT take ownership of list or its contents.  Caller still owns it.
    *
    *@param[in]  _fileDesc Description of the Feature Data,  how many features, types, etc.
    *
    *@param[in]  _fvFactoryProducer  
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
    *                                ImageFileName or the save Feature Values.  If duplicates are in 
    *                                the same class then all but one will be removes.  If they are
    *                                in more then one class then they will both be removed.
    *
    *@param[in] _cancelFlag  Will be monitored by training process.  If this flag turns true will return
    *                        to caller as soon as convenient.
    *
    *@param[out] _statusMessage  Caller can monitor this field for messages that can be displayed to 
    *                            the user as a way of letting them know what is happening.
    */
   TrainingProcess2 (const KKStr&         _configFileName,
                     FeatureVectorListPtr _excludeList,
                     FactoryFVProducerPtr _fvFactoryProducer,
                     RunLog&              _log,
                     std::ostream*        _report,
                     bool                 _forceRebuild,
                     bool                 _checkForDuplicates,
                     VolConstBool&        _cancelFlag,
                     KKStr&               _statusMessage
                    );


    /**
    *@brief  Build a new model from scratch for the specified class level. Will also remove duplicates.
    *
    *@details  Using the parameter '_level' will construct a classifier that groups classes together
    *          by group.  Underscore characters in the class name will be used to differentiate group
    *          levels.  Ex:  Crustacean_Copepod_Calanoid has three levels of grouping where 'Crustacean'
    *          belongs to level 1.
    *
    *@param[in]  _configFileName  Configuration file name where classes and SVM parameters are
    *                             specified.  The directories where example images for each class
    *                             will be specified.
    *
    *@param[in]  _excludeList  List of Feature Vectors that are to be excluded from the TrainingData.
    *                          If will check by both ImageFileName and FeatureValues. If this parameter
    *                          is not equal to NULL then will not save the Training model.  Will
    *                          NOT take ownership of list or its contents.  Caller still owns it.
    *
    *@param[in]  _fileDesc Description of the Feature Data,  how many features, types, etc.
    *
    *@param[in]  _fvFactoryProducer  
    *
    *@param[in,out]  _log   Logging file.
    *
    *@param[in] _level  The grouping level to build a classifier for.  Ex: if _level = 2 is specified
    *                   and referring to the class name "Crustacean_Copepod_Calanoid" above all classes
    *                   that start with "Crustacean_Copepod_" will be combined as one logical class.
    *
    *@param[in] _cancelFlag  Will be monitored by training process.  If this flag turns true will return
    *                        to caller as soon as convenient.
    *
    *@param[out] _statusMessage  Caller can monitor this field for messages that can be displayed to 
    *                            the user as a way of letting them know what is happening.
    */
   TrainingProcess2 (const KKStr&         _configFileName,
                     FeatureVectorListPtr _excludeList,
                     FactoryFVProducerPtr _fvFactoryProducer,
                     RunLog&              _log,
                     kkuint32             _level,            /**< Class hierarchy level to train at. */
                     VolConstBool&        _cancelFlag, 
                     KKStr&               _statusMessage
                    );

   
   
   /**
    *@brief  Constructor Will use existing built model; will not check to see if it is up-to-date.
    *@details  If the previously saved training process is invalid or does not exist it will set the
    *    'Abort' flag to 'true' an return to caller.  It will NOT attempt to build a brand new
    *    TrainingProcess model.
    *
    *@param[in]  _configFileName  Configuration file name where classes and SVM parameters are
    *                             specified.  The directories where example images for each class
    *                             will be specified.
    *
    *@param[in]  _fileDesc Description of the Feature Data,  how many features, types, etc.
    *
    *@param[in]  _fvFactoryProducer  
    *
    *@param[in,out]  _log   Logging file.
    *
    *@param[in] _featuresAlreadyNormalized  If set to true will assume that all features in the
    *                                       training data are normalized.
    *
    *@param[in] _cancelFlag  Will be monitored by training process.  If this flag turns true will return
    *                        to caller as soon as convenient.
    *
    *@param[out] _statusMessage  Caller can monitor this field for messages that can be displayed to 
    *                            the user as a way of letting them know what is happening.
    */
   TrainingProcess2 (const KKStr&         _configFileName,
                     FactoryFVProducerPtr _fvFactoryProducer,
                     RunLog&              _log,
                     bool                 _featuresAlreadyNormalized,
                     VolConstBool&        _cancelFlag,
                     KKStr&               _statusMessage
                    );


   /**
    *@brief  Constructor that gets its training data from a list of examples provided in one of the parameters.
    *@param[in]  _config  A configuration that is already loaded in memory.
    *@param[in]  _trainingExamples  Training data to train classifier with.
    *@param[in]  _mlClasses  Class list.
    *@param[in]  _reportFile  if not set to NULL will write statistics of the training process to stream.
    *@param[in]  _fileDesc Description of the Feature Data,  how many features, types, etc.
    *@param[in]  _fvFactoryProducer  
    *@param[in,out]  _log   Logging file.
    *@param[in] _featuresAlreadyNormalized  If set to true will assume that all features in the
    *                                       training data are normalized.
    *@param[in] _cancelFlag  Will be monitored by training process.  If this flag turns true will return
    *                        to caller as soon as convenient.
    *@param[out] _statusMessage  Caller can monitor this field for messages that can be displayed to 
    *                            the user as a way of letting them know what is happening.
    */
   TrainingProcess2 (TrainingConfiguration2Ptr  _config, 
                     FeatureVectorListPtr       _trainingExamples,
                     MLClassListPtr             _mlClasses,
                     std::ostream*              _reportFile,
                     FactoryFVProducerPtr       _fvFactoryProducer,
                     RunLog&                    _log,
                     bool                       _featuresAlreadyNormalized,
                     VolConstBool&              _cancelFlag,
                     KKStr&                     _statusMessage
                    );


   ~TrainingProcess2 ();

  void    CreateModelsFromTrainingData ();

  void    ExtractTrainingClassFeatures (KKB::DateTime&  latestImageTimeStamp,
                                        bool&           changesMadeToTrainingLibraries
                                       );

  void    ReportTraningClassStatistics (std::ostream&  report);

  void    SaveResults ();

  void    ValidateConfiguration ();


  // Access Members
  bool                     Abort              () const  {return abort;}

  const KKB::DateTime&     BuildDateTime      () const  {return  buildDateTime;}

  TrainingConfiguration2Ptr Config            ()        {return  config;}    

  const KKStr&             ConfigFileName     () const  {return configFileName;}

  VectorKKStr              ConfigFileFormatErrors ()  const;

  kkint32                  DuplicateDataCount () const;

  bool                     FeaturesAlreadyNormalized ()  
                                                 {return  featuresAlreadyNormalized;}


  FeatureVectorListPtr     Images             () {return  trainingExamples;}

  MLClassListPtr           ImageClasses       () {return  mlClasses;}

  RunLog&                  Log                () {return log;}

  kkint32                  MemoryConsumedEstimated ()  const;

  SVMModelPtr              Model3 ();
 
  ModelOldSVMPtr           OldSVMModel ()  const;

  void                     Read (istream&  in,
                                 bool&     successful
                                );

  ModelParamPtr            Parameters                () const;

  ModelPtr                 TrainedModel              () const  {return model;}

  double                   TrainingTime              () const;   // Comes from SVMModel

  kkint32                  NumOfSupportVectors       () const;

  void                     SupportVectorStatistics (kkint32&  numSVs,
                                                    kkint32&  totalNumSVs
                                                   );

  void                     Abort (bool _abort)  {abort = _abort;}


private:
  void    AddImagesToTrainingLibray (FeatureVectorList&  trainingImages,
                                     FeatureVectorList&  imagesToAdd
                                    );

  void    BuildModel3 ();

  void    CheckForDuplicates (bool  allowDupsInSameClass);

  void    RemoveExcludeListFromTrainingData ();



  //************************************************************
  //             Routines for Extracting Features              *
  //************************************************************
  void  ExtractFeatures (const TrainingClassPtr  trainingClass,
                         KKB::DateTime&          latestTimeStamp,
                         bool&                   changesMade
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

  VolConstBool&                cancelFlag;  /**< A calling application can set this to true Training Process will monitor this Flag, if true will terminate. */


  TrainingConfiguration2Ptr    config;

  KKStr                        configFileName;           /**< The directory path where this file is actually located will be added to this name. */

  KKStr                        configFileNameSpecified;  /** This will be the ConfigFileName specified by caller before the directory
                                                          * that is added for actual location of config file. directory */

  kkint32                      duplicateDataCount;

  FeatureVectorListPtr         excludeList;  /**< If != NULL then list of trainingExamples that need to be eliminated from training data.
                                              * This would be used when classifying trainingExamples that might already contain training
                                              * data.  If you wish to grade the results it would only be fare to delete these trainingExamples.
                                              */

  FactoryFVProducerPtr         fvFactoryProducer;

  bool                         featuresAlreadyNormalized;

  FileDescPtr                  fileDesc;

  MLClassListPtr               mlClasses; /**< List of all classes that are to be processed. There will be one entry for each MLClass,
                                              * Including one for noise trainingExamples(unknown trainingExamples).
                                              */

  RunLog&                      log;

  ModelPtr                     model;

  std::ostream*                report;

  KKStr                        savedModelName;

  KKStr&                       statusMessage; /**< A means of communicating back to calling function in a multi threaded environment. */

  FeatureVectorListPtr         trainingExamples;  /**< All Images Loaded.  Own's all trainingExamples.  All other ImageList's will only point to
                                                   * these trainingExamples.
                                                   */

  bool                         weOwnImageClassesConfig;
  bool                         weOwnTrainingExamples;
};


typedef  TrainingProcess2*  TrainingProcess2Ptr;


} /* namespace KKMLL */

#endif
