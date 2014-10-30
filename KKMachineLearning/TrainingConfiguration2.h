#ifndef _TRAININGCONFIGURATION2_
#define _TRAININGCONFIGURATION2_

#include "Configuration.h"
#include "DateTime.h"
#include "KKStr.h"

#include "FileDesc.h"
#include "FactoryFVProducer.h"
#include "FeatureVectorProducer.h"
#include "FeatureVector.h"
#include "Model.h"
#include "ModelParam.h"
#include "ModelParamOldSVM.h"
#include "SVMparam.h"
#include "TrainingClass.h"


namespace KKMachineLearning 
{
  #if  !defined(_NormalizationParmsDefined_)
  class  NormalizationParms;
  typedef  NormalizationParms*  NormalizationParmsPtr;
  #endif

  #if  !defined(_FactoryFVProducer_Defined_)
  class  FactoryFVProducer;
  typedef  FactoryFVProducer*  FactoryFVProducerPtr;
  #endif




  class  TrainingConfiguration2:  public  Configuration
  {
  public:
    typedef  Model::ModelTypes   ModelTypes;
      

    typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;


    TrainingConfiguration2 (const KKStr&          _configFileName,
                            FactoryFVProducerPtr  _fvFactoryProducer,           /**< We will take ownership of this instance and delete it in destructor. */
                            bool                  _validateDirectories,
                            RunLog&               _log
                           );


    TrainingConfiguration2 (const TrainingConfiguration2&  tc);


    /**
     *@brief Use this one if you want to create a default Configuration object.
     *@param[in]  _fileDesc  Description of features.
     *@param[in]  _mlClasses  Will make copy of list of MLClasses and NOT take ownership.
     *@param[in]  _parameterStr Sting with Machine Learninig Parameters.
     *@param[in]  _fvFactoryProducer  Factory for creating FeatureVectorProducer instance.
     *@param[in]  _log  Where to send logging messages to.
     */
    TrainingConfiguration2 (MLClassListPtr        _mlClasses,       
                            KKStr                 _parameterStr,
                            FactoryFVProducerPtr  _fvFactoryProducer,      /**< We will take ownership of this instance and delete it in destructor. */
                            RunLog&               _log             
                           );

    ~TrainingConfiguration2 ();


    /**
     *@brief  Will create a instance using a sub-directory tree to drive the TraningClassList.
     *@details for each unique sub-directory entry below it that contains 'bmp' files a TrainingClass' 
     * instance will be created using the root-name as the class name.  If an existing Config file already
     * exists then parameters will be taken from it otherwise a default cnfiguration will be created.
     *@param[in]  _fileDesc  
     *@param[in]  _existingConfigFileName  Caller can specify an existing Configuration file to extract configuration 
     *                                     parameters from; if left blank then a configuration file with the same name 
     *                                     of the last directory in the path specified by _subDir will be assumed in the 
     *                                     default TraningModel directory.
     *@param[in]  _subDir   The root directory entry to the Sub-Directory structure that is to be used to construct 
     *                      training classs list from.
     *@param[in]  _fvFactoryProducerr
     *@param[in]  _log
     *@param[out] _successful
     *@param[out] _errorMessage Will contain description of errors encountered. 
     */
    static
    TrainingConfiguration2*  CreateFromDirectoryStructure 
                                            (const KKStr&          _existingConfigFileName,
                                             const KKStr&          _subDir,
                                             FactoryFVProducerPtr  _fvFactoryProducer,
                                             RunLog&               _log,
                                             bool&                 _successful,
                                             KKStr&                _errorMessage
                                            );


    static
    TrainingConfiguration2*  CreateFromFeatureVectorList
                                            (FeatureVectorList&    _examples,
                                             MLClassListPtr        _mlClasses,
                                             FactoryFVProducerPtr  _fvFactoryProducer,  /**< Will take ownership and delete in destructor.  */
                                             RunLog&               _log
                                            );

    
    static KKStr       ModelTypeToStr   (ModelTypes    _modelType)    {return  Model::ModelTypeToStr   (_modelType);}
    static ModelTypes  ModelTypeFromStr (const KKStr&  _modelTypeStr) {return  Model::ModelTypeFromStr (_modelTypeStr);}


    //  Access Methods.

    float                  A_Param () const;
    void                   A_Param (float  _aParam);

    float                  AvgNumOfFeatures ()  const;
    float                  AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const;

    const
    BinaryClassParmsPtr    BinaryClassParms (MLClassPtr  class1,
                                             MLClassPtr  class2
                                            )  const;

    double                 C_Param () const;

    void                   C_Param (double _CCC);

    double                 C_Param (MLClassPtr  class1,
                                    MLClassPtr  class2
                                   )  const;

    void                   C_Param (MLClassPtr  class1,
                                    MLClassPtr  class2,
                                    double      cParam
                                   );


    const KKStr&           ConfigFileNameSpecified () const  {return configFileNameSpecified;}

    SVM_EncodingMethod     EncodingMethod ()  const;
    void                   EncodingMethod (SVM_EncodingMethod _encodingMethod);

    kkint32                ExamplesPerClass ()  const;
    void                   ExamplesPerClass (kkint32 _examplesPerClass);
    
    MLClassListPtr         ExtractListOfClassesForAGivenHierarchialLevel (kkuint32 level)   const;

    FileDescPtr            FileDesc ()            const {return  fileDesc;}

    FactoryFVProducerPtr   FvFactoryProducer ()   const {return fvFactoryProducer;}

    double                 Gamma () const;
    void                   Gamma (double  _gamma);

    kkint32                ImagesPerClass ()  const               {return  ExamplesPerClass ();};
    void                   ImagesPerClass (kkint32  _imagesPerClass)  {ExamplesPerClass (_imagesPerClass);}
    
    SVM_KernalType         KernalType ()  const;
    void                   KernalType (SVM_KernalType _kernalType);

    SVM_MachineType        MachineType ();
    void                   MachineType (SVM_MachineType _machineType);

    Model::ModelTypes      ModelingMethod ()      const {return  modelingMethod;}

    MLClassPtr             NoiseImageClass ()     const {return  noiseImageClass;}

    const 
    TrainingClassPtr       NoiseTrainingClass ()  const {return  noiseTrainingClass;}

    kkint32                NoiseGuaranteedSize () const {return  noiseGuaranteedSize;}

    KKStr                  RootDirExpanded () const;
    void                   RootDir (const KKStr& _rootDir);

    const 
    TrainingClassList&     TrainingClasses ()     const {return  trainingClasses;}

   

    ModelParamPtr          ModelParameters ()     const {return  modelParameters;}

    KKStr                  ModelParameterCmdLine ()    const;

    void                   ModelParameters (ModelParamPtr  _modelParameters);

    kkuint32               NumHierarchialLevels ()  const;  // returns back the number of hierarchail levels there
                                                            // are in the trainingClass that has the most.

    SVM_SelectionMethod    SelectionMethod ()  const;
    void                   SelectionMethod (SVM_SelectionMethod  _selectionMethod);

    const SVMparam&        SVMparamREF ()  const;

    /**
     *@brief Adds specified Trainig Class to list taking ownership of it.
     */
    void                   AddATrainingClass (TrainingClassPtr  _trainClass);

    void                   AddATrainingClass (MLClassPtr  _newClass);  /**< Will assume that images for this class will 
                                                                        * be saved off the RootDirectory using its own 
                                                                        * name for the subdirectory name.
                                                                        */


    void                   BuildTrainingClassListFromDirectoryStructure (const KKStr&  _subDir,
                                                                         bool&         _successful,
                                                                         KKStr&        _errorMessage
                                                                        );


    KKStr                  DirectoryPathForClass (MLClassPtr  mlClass)  const;

    void                   EmptyTrainingClasses ();

    MLClassListPtr         ExtractClassList ()  const;

    TrainingConfiguration2Ptr  GenerateAConfiguraionForAHierarchialLevel (kkuint32 level);

    FeatureNumList         GetFeatureNums ()  const;

    /**@brief  Retuns features selected for teh specified class-pair. */
    FeatureNumList         GetFeatureNums (MLClassPtr  class1,   /**< First of two classes of pair. */
                                           MLClassPtr  class2    /**< Second of two classes of pair. */
                                          );


    FeatureVectorListPtr   LoadFeatureDataFromTrainingLibraries (KKB::DateTime&  latestImageTimeStamp,
                                                                 bool&           changesMadeToTrainingLibraries,
                                                                 bool&           cancelFlag
                                                                );

    TrainingClassPtr       LocateByImageClassName (const KKStr&  className);

    bool                   NormalizeNominalFeatures ();

    kkint32                NumOfFeaturesAfterEncoding ()  const;

    virtual void           Save (const KKStr& fileName);

    virtual void           Save (ostream&  o);

    void                   SetFeatureNums (const  FeatureNumList&  features);

    void                   SetFeatureNums (MLClassPtr             class1,
                                           MLClassPtr             class2,
                                           const FeatureNumList&  _features,
                                           float                  _weight = -1   //  -1 Indicates - use existing value
                                          );

    void                   SetModelParameters (ModelParamPtr  _svmParanters,
                                               kkint32        _examplesPerClass
                                              );


    void                   SetBinaryClassFields (MLClassPtr                   class1,
                                                 MLClassPtr                   class2,
                                                 const SVM233::svm_parameter& _param,
                                                 const FeatureNumList&        _features,
                                                 float                        _weight
                                                );

    void                   SetTrainingClasses (TrainingClassListPtr  _trainingClasses);


    static  KKStr          GetEffectiveConfigFileName (const  KKStr&  configFileName);


  private:
    void                   BuildTrainingClassListFromDirectoryEntry (const KKStr&  rootDir,
                                                                     const KKStr&  subDir,
                                                                     bool&         successful,
                                                                     KKStr&        errorMessage
                                                                    );


    void                   CreateModelParameters (const KKStr&           _parameterStr,
                                                  const FeatureNumList&  _selFeatures,
                                                  kkint32                _sectionLineNum,
                                                  kkint32                _parametersLineNum, 
                                                  kkint32                _featuresIncludedLineNum
                                                 );


    FeatureNumListPtr      DeriveFeaturesSelected (kkint32  sectionNum);

    void                   DetermineWhatTheRootDirectoryIs ();

    virtual
    FeatureVectorListPtr   ExtractFeatures (const TrainingClassPtr  trainingClass,
                                            KKB::DateTime&          latestTimeStamp,
                                            bool&                   changesMade,
                                            bool&                   cancelFlag
                                           );

    SVMparamPtr            SVMparamToUse ()  const;

    void                   SyncronizeImageClassListWithTrainingClassList ();

    TrainingClassPtr       ValidateClassConfig    (kkint32  sectionNum);

    void                   ValidateConfiguration ();
    
    void                   ValidateGlobalSection (kkint32  sectionNum);

    void                   ValidateTrainingClassConfig (kkint32  sectionNum);

    void                   ValidateTwoClassParameters (kkint32  sectionNum);


    ModelParamOldSVMPtr    OldSvmParameters ()  const;


    KKStr                  configFileNameSpecified;   /**< Config file name that was specified by caller before
                                                       * directory path was added by 'GetEffectiveConfigFileName'.
                                                       */

    kkint32                   examplesPerClass;
    FactoryFVProducerPtr      fvFactoryProducer;
    FileDescPtr               fileDesc;
    MLClassListPtr            mlClasses;
    bool                      imageClassesWeOwnIt;  /**< If we own it we will delete it in the destructor.  */
    RunLog&                   log;
    ModelTypes                modelingMethod;
    ModelParamPtr             modelParameters;

    kkint32                   noiseGuaranteedSize;  /**< Images smaller than this size will be classified as noise 
                                                     * and will not be used for training purposes.
                                                     */

    MLClassPtr                noiseImageClass;

    TrainingClassPtr          noiseTrainingClass;   /**< The specific Training Class that is to be 
                                                     * used for noise images. 
                                                     */

    NormalizationParmsPtr     normalizationParms;

    KKStr                     rootDir;           /**< Common directory that all images for this training
                                                  * library come from.  This is determined by iterating 
                                                  * through all the 'trainingClasses' entries and 
                                                  * looking for the common string that they all start by.
                                                  */

    KKStr                     rootDirExpanded;   /**< Same as 'rootDir' except environment variables will be expanded. */

    TrainingClassList         trainingClasses;   /**< List of  'Training_Class' objects.  One for
                                                  * each 'Training_Classe' section defined in
                                                  * configuration file.  Plus one for the 
                                                  * 'Noise_Images' section.
                                                  */

    bool                      validateDirectories;
  };  /* TrainingConfiguration2 */

  #define  _TrainingConfiguration2_Defined_

  typedef  TrainingConfiguration2::TrainingConfiguration2Ptr  TrainingConfiguration2Ptr;




  KKStr&  operator<< (KKStr&                               left,
                      TrainingConfiguration2::ModelTypes   modelingMethod
                     );


  std::ostream& operator<< (std::ostream&                       os,
                            TrainingConfiguration2::ModelTypes  modelingMethod
                           );




  class  TrainingConfiguration2List: public KKQueue<TrainingConfiguration2>
  {
  public:
    TrainingConfiguration2List (bool _owner):
        KKQueue<TrainingConfiguration2> (_owner)
        {}

    ~TrainingConfiguration2List ()
        {}

  };
  typedef  TrainingConfiguration2List*  TrainingConfiguration2ListPtr;

  #define  _TrainingConfiguration2List_Defined_
}  /* namespace KKMachineLearning */

#endif
