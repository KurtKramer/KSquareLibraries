#ifndef _TRAININGCONFIGURATION2_
#define _TRAININGCONFIGURATION2_

#include "Configuration.h"
#include "DateTime.h"
#include "FileDesc.h"
#include "FeatureVector.h"
#include "Model.h"
#include "ModelParam.h"
#include "ModelParamOldSVM.h"
#include "KKStr.h"
#include "SVMparam.h"
#include "TrainingClass.h"


namespace KKMachineLearning 
{
  #if  !defined(_NormalizationParmsDefined_)
  class  NormalizationParms;
  typedef  NormalizationParms*  NormalizationParmsPtr;
  #endif


  class  TrainingConfiguration2:  public  Configuration
  {
  public:
    typedef  Model::ModelTypes   ModelTypes;
      

    typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;


    TrainingConfiguration2 (FileDescPtr    _fileDesc,
                            const KKStr&   _configFileName, 
                            RunLog&        _log,
                            bool           validateDirectories = true
                           );


    TrainingConfiguration2 (const TrainingConfiguration2&  tc);


    /**
     *@brief Use this one if you want to create a default Configuration object.
     */
    TrainingConfiguration2 (FileDescPtr        _fileDesc,
                            MLClassListPtr  _mlClasses,
                            KKStr              _parameterStr,
                            RunLog&            _log             
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
     *@param[in]  _log
     *@param[out] _successful
     *@param[out] _errorMessage Will contain description of errors encountered. 
     */
    static
    TrainingConfiguration2*  CreateFromDirectoryStructure 
                                            (FileDescPtr   _fileDesc,
                                             const KKStr&  _existingConfigFileName,
                                             const KKStr&  _subDir,
                                             RunLog&       _log,
                                             bool&         _successful,
                                             KKStr&        _errorMessage
                                            );


    static
    TrainingConfiguration2*  CreateFromFeatureVectorList
                                            (FeatureVectorList&  _examples,
                                             MLClassListPtr   _mlClasses,
                                             RunLog&             _log
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
                                    double         cParam
                                   );


    const KKStr&           ConfigFileNameSpecified () const  {return configFileNameSpecified;}

    SVM_EncodingMethod     EncodingMethod ()  const;
    void                   EncodingMethod (SVM_EncodingMethod _encodingMethod);

    int32                  ExamplesPerClass ()  const;
    void                   ExamplesPerClass (int32 _examplesPerClass);
    
    MLClassListPtr      ExtractListOfClassesForAGivenHierarchialLevel (uint32 level)   const;

    FileDescPtr            FileDesc ()            const {return  fileDesc;}

    double                 Gamma () const;
    void                   Gamma (double  _gamma);

    int32                  ImagesPerClass ()  const               {return  ExamplesPerClass ();};
    void                   ImagesPerClass (int32  _imagesPerClass)  {ExamplesPerClass (_imagesPerClass);}
    
    SVM_KernalType         KernalType ()  const;
    void                   KernalType (SVM_KernalType _kernalType);

    SVM_MachineType        MachineType ();
    void                   MachineType (SVM_MachineType _machineType);

    Model::ModelTypes      ModelingMethod ()      const {return  modelingMethod;}

    MLClassPtr          NoiseImageClass ()     const {return  noiseImageClass;}

    const 
    TrainingClassPtr       NoiseTrainingClass ()  const {return  noiseTrainingClass;}

    int32                  NoiseGuaranteedSize () const {return  noiseGuaranteedSize;}

    KKStr                  RootDirExpanded () const;
    void                   RootDir (const KKStr& _rootDir);

    const 
    TrainingClassList&     TrainingClasses ()     const {return  trainingClasses;}

   

    ModelParamPtr          ModelParameters ()     const {return  modelParameters;}

    KKStr                  ModelParameterCmdLine ()    const;

    void                   ModelParameters (ModelParamPtr  _modelParameters);

    uint32                 NumHierarchialLevels ()  const;  // returns back the number of hierarchail levels there
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

    MLClassListPtr      ExtractClassList ()  const;

    TrainingConfiguration2Ptr  GenerateAConfiguraionForAHierarchialLevel (uint32 level);

    FeatureNumList         GetFeatureNums ()  const;

    FeatureNumList         GetFeatureNums (MLClassPtr  class1,   // Will get feature nums that will be
                                           MLClassPtr  class2    // used by the given class pair.
                                          );                        // If none were specified for that pair
                                                                    // will return the genral one specified 


    FeatureVectorListPtr   LoadFeatureDataFromTrainingLibraries (KKB::DateTime&  latestImageTimeStamp,
                                                                 bool&           changesMadeToTrainingLibraries,
                                                                 bool&           cancelFlag
                                                                );

    TrainingClassPtr       LocateByImageClassName (const KKStr&  className);

    bool                   NormalizeNominalFeatures ();

    int32                  NumOfFeaturesAfterEncoding ()  const;

    static
      TrainingConfiguration2Ptr
                           PromptForConfigurationFile (RunLog&  log);

    virtual void           Save (const KKStr& fileName);

    virtual void           Save (ostream&  o);

    void                   SetFeatureNums (const  FeatureNumList&  features);

    void                   SetFeatureNums (MLClassPtr          class1,
                                           MLClassPtr          class2,
                                           const FeatureNumList&  _features,
                                           float                  _weight = -1   //  -1 Indicates - use existing value
                                          );

    void                   SetModelParameters (ModelParamPtr  _svmParanters,
                                               int32          _examplesPerClass
                                              );


    void                   SetBinaryClassFields (MLClassPtr                class1,
                                                 MLClassPtr                class2,
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
                                                  int32                  _sectionLineNum,
                                                  int32                  _parametersLineNum, 
                                                  int32                  _featuresIncludedLineNum
                                                 );


    FeatureNumListPtr      DeriveFeaturesSelected (int32  sectionNum);

    void                   DetermineWhatTheRootDirectoryIs ();

    virtual
    FeatureVectorListPtr   ExtractFeatures (const TrainingClassPtr  trainingClass,
                                            KKB::DateTime&          latestTimeStamp,
                                            bool&                   changesMade,
                                            bool&                   cancelFlag
                                           );

    SVMparamPtr            SVMparamToUse ()  const;

    void                   SyncronizeImageClassListWithTrainingClassList ();

    TrainingClassPtr       ValidateClassConfig    (int32  sectionNum);

    void                   ValidateConfiguration ();
    
    void                   ValidateGlobalSection (int32  sectionNum);

    void                   ValidateTrainingClassConfig (int32  sectionNum);

    void                   ValidateTwoClassParameters (int32  sectionNum);


    ModelParamOldSVMPtr    OldSvmParameters ()  const;


    KKStr                  configFileNameSpecified;   /**< Config file name that was specified by caller before
                                                       * directory path was added by 'GetEffectiveConfigFileName'.
                                                       */

    int32                  examplesPerClass;
    FileDescPtr            fileDesc;
    MLClassListPtr      mlClasses;
    bool                   imageClassesWeOwnIt;       /**< If we own it we will delete it in the destructor.  */
    RunLog&                log;
    ModelTypes             modelingMethod;
    ModelParamPtr          modelParameters;

    int32                  noiseGuaranteedSize;  /**< Images smaller than this size will be classified as noise 
                                                  * and will not be used for training purposes.
                                                  */

    MLClassPtr          noiseImageClass;

    TrainingClassPtr       noiseTrainingClass;   /**< The specific Training Class that is to be 
                                                  * used for noise images. 
                                                  */

    NormalizationParmsPtr  normalizationParms;

    KKStr                  rootDir;           /**< Common directory that all images for this training
                                               * library come from.  This is determined by iterating 
                                               * through all the 'trainingClasses' entries and 
                                               * looking for the common string that they all start by.
                                               */

    KKStr                  rootDirExpanded;   /**< Same as 'rootDir' except environment variables will be expanded. */

    TrainingClassList      trainingClasses;   /**< List of  'Training_Class' objects.  One for
                                               * each 'Training_Classe' section defined in
                                               * configuration file.  Plus one for the 
                                               * 'Noise_Images' section.
                                               */

    bool                   validateDirectories;
  };  /* TrainingConfiguration2 */

  #define  _TrainingConfiguration2Defined_

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

}  /* namespace KKMachineLearning */

#endif
