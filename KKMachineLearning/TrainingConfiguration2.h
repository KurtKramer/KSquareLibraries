#if  !defined(_TRAININGCONFIGURATION2_)
#define _TRAININGCONFIGURATION2_

#include "Configuration.h"
#include "DateTime.h"
#include "GoalKeeper.h"
#include "KKStr.h"
#include "XmlStream.h"

#include "FileDesc.h"
#include "FactoryFVProducer.h"
#include "FeatureVectorProducer.h"
#include "FeatureVector.h"
#include "Model.h"
#include "ModelParam.h"
#include "ModelParamOldSVM.h"
#include "SVMparam.h"
#include "TrainingClass.h"



namespace KKMLL 
{
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


  #if  !defined(_NormalizationParms_Defined_)
  class  NormalizationParms;
  typedef  NormalizationParms*  NormalizationParmsPtr;
  #endif


  #if !defined(_FactoryFVProducer_Defined_)
  class  FactoryFVProducer;
  typedef  FactoryFVProducer*  FactoryFVProducerPtr;
  #endif


  class  TrainingConfiguration2List;
  typedef  TrainingConfiguration2List*  TrainingConfiguration2ListPtr;


  class  TrainingConfiguration2:  public  Configuration
  {
  public:
    typedef  Model::ModelTypes   ModelTypes;
      

    typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
    typedef  TrainingConfiguration2  const  TrainingConfiguration2Const;
    typedef  TrainingConfiguration2Const*  TrainingConfiguration2ConstPtr;

    TrainingConfiguration2 ();

    TrainingConfiguration2 (const KKStr&  _configFileName,
                            bool          _validateDirectories,
                            RunLog&       _log
                           );


    TrainingConfiguration2 (const TrainingConfiguration2&  tc);


    /**
     *@brief Use this one if you want to create a default Configuration object.
     *@param[in]  _mlClasses  Will make copy of list of MLClasses and NOT take ownership.
     *@param[in]  _fvFactoryProducer  If NULL will default to "GrayScaleImagesFVProducerFactory".
     *@param[in]  _parameterStr Sting with Machine Learning Parameters.
     *@param[in]  _log  Where to send logging messages to.
     */
    TrainingConfiguration2 (MLClassListPtr        _mlClasses,
                            FactoryFVProducerPtr  _fvFactoryProducer,
                            const KKStr&          _parameterStr,
                            RunLog&               _log             
                           );


    /**
     *@brief Use this one if you want to create a default Configuration object.
     *@details We know what the underlying feature data is from _fileDesc but we do not know how they 
     * are computed; that is why we do not provide a "FactoryFVProducer" instance.
     *@param[in]  _mlClasses  Will make copy of list of MLClasses and NOT take ownership.
     *@param[in]  _fvFactoryProducer  If NULL will default to "GrayScaleImagesFVProducerFactory".
     *@param[in]  _parameterStr Sting with Machine Learning Parameters.
     *@param[in]  _log  Where to send logging messages to.
     */
    TrainingConfiguration2 (MLClassListPtr  _mlClasses,
                            FileDescPtr     _fileDesc,
                            const KKStr&    _parameterStr,
                            RunLog&         _log             
                           );


    /**
     *@brief  Creates a configuration file using the parameters specified in '_modelParameters';  does not read 
     * from a configuration file.  
     *@details  For each class specified in '_mlClasses' a 'TrainingClass' instance will be created. 
     * All feature-numbers will be assumed to be selected and unlimited examples per class will be allowed.
     *@param[in] _mlClasses
     *@param[in]  _fvFactoryProducer  If NULL will default to "GrayScaleImagesFVProducerFactory".
     *@param[in] _modelParameters  Will take ownership of this instance.
     *@param[in] _log
     */
    TrainingConfiguration2 (MLClassListPtr  _mlClasses,
                            FileDescPtr     _fileDesc,
                            ModelParamPtr   _modelParameters,
                            RunLog&         _log
                           );




     virtual    
     ~TrainingConfiguration2 ();


     virtual
     FactoryFVProducerPtr   DefaultFeatureVectorProducer (RunLog&  runLog);


    /**
     *@brief  Will create a instance using a sub-directory tree to drive the TraningClassList.
     *@details for each unique sub-directory entry below it that contains 'bmp' files a TrainingClass' 
     * instance will be created using the root-name as the class name.  If an existing Config file already
     * exists then parameters will be taken from it otherwise a default configuration will be created.
     *@param[in]  _existingConfigFileName  Caller can specify an existing Configuration file to extract configuration 
     *                                     parameters from; if left blank then a configuration file with the same name 
     *                                     of the last name in the directory path specified by _subDir will be assumed 
     *                                     in the default TraningModel directory. If still no configuration file
     *                                     found then one will be created using default parameters. 
     *
     *@param[in]  _subDir   The root directory entry to the Sub-Directory structure that is to be used to construct 
     *                      training class list from.
     *
     *@param[in]  _fvFactoryProducer The FeatureVector computation routines you want to use;  if NULL will default.
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
                                            (FeatureVectorList&  _examples,
                                             FileDescPtr         _fileDesc,
                                             const KKStr&        _parameterStr, 
                                             RunLog&             _log
                                            );

    
    static KKStr       ModelTypeToStr   (ModelTypes    _modelType)    {return  Model::ModelTypeToStr   (_modelType);}
    static ModelTypes  ModelTypeFromStr (const KKStr&  _modelTypeStr) {return  Model::ModelTypeFromStr (_modelTypeStr);}


    //  Access Methods.

    float                  A_Param                 () const;
    float                  AvgNumOfFeatures        () const;
    double                 C_Param                 () const;
    const KKStr&           ConfigFileNameSpecified () const  {return configFileNameSpecified;}
    const KKStr&           ConfigRootName          () const  {return configRootName;}
    SVM_EncodingMethod     EncodingMethod          () const;
    kkint32                ExamplesPerClass        () const;
    FileDescPtr            FileDesc                () const  {return  fileDesc;}
    FactoryFVProducerPtr   FvFactoryProducer       () const  {return  fvFactoryProducer;}
    double                 Gamma                   () const;
    kkint32                ImagesPerClass          () const  {return  ExamplesPerClass ();};
    SVM_KernalType         KernalType              () const;
    SVM_MachineType        MachineType             () const;
    MLClassListPtr         MlClasses               () const  {return  mlClasses;}
    Model::ModelTypes      ModelingMethod          () const  {return  modelingMethod;}
    Model::ModelTypes      ModelType               () const  {return  modelingMethod;}
    KKStr                  ModelTypeStr            () const  {return  Model::ModelTypeToStr (modelingMethod);}
    MLClassPtr             NoiseMLClass            () const  {return  noiseMLClass;}
    const TrainingClassPtr NoiseTrainingClass      () const  {return  noiseTrainingClass;}

    kkint32                NumOfRounds             () const  {return  Number_of_rounds ();}
    kkint32                Number_of_rounds        () const;
    MLClassPtr             OtherClass              () const  {return  otherClass;}
    const KKStr&           RootDir                 () const  {return  rootDir;}
    KKStr                  RootDirExpanded         () const;

    TrainingConfiguration2ListPtr  SubClassifiers    () const {return  subClassifiers;}
    const TrainingClassList&       TrainingClasses   () const {return  trainingClasses;}
    ModelParamPtr          ModelParameters         () const {return  modelParameters;}
    KKStr                  ModelParameterCmdLine   () const;
    kkuint32               NumHierarchialLevels    () const;  /**< returns back the number of hierarchical levels thereare in the trainingClass that has the most. */
    SVM_SelectionMethod    SelectionMethod         () const;
    const SVMparam&        SVMparamREF             (RunLog&  log) const;



    const
    BinaryClassParmsPtr    GetParamtersToUseFor2ClassCombo (MLClassPtr       class1,
                                                            MLClassPtr       class2
                                                           )  const;


    /** if BinaryClass parms exist for the two specified classes will return otherwise NULL. */
    BinaryClassParmsPtr      GetBinaryClassParms (MLClassPtr       class1,
                                                  MLClassPtr       class2
                                                 );

    double                 C_Param (MLClassPtr  class1,
                                    MLClassPtr  class2
                                   )  const;


    void  A_Param (float  _aParam);

    void  C_Param (double _CCC);

    void  C_Param (MLClassPtr  class1,
                   MLClassPtr  class2,
                   double      cParam
                  );

    void  EncodingMethod     (SVM_EncodingMethod     _encodingMethod);
    void  ExamplesPerClass   (kkint32                _examplesPerClass);
    void  Gamma              (double                 _gamma);
    void  ImagesPerClass     (kkint32                _imagesPerClass)  {ExamplesPerClass (_imagesPerClass);}
    void  KernalType         (SVM_KernalType         _kernalType);
    void  MachineType        (SVM_MachineType        _machineType);
    void  ModelParameters    (ModelParamPtr          _modelParameters);
    void  Number_of_rounds   (kkint32                _number_of_rounds);
    void  NumOfRounds        (kkint32                _numOfRounds)     {Number_of_rounds  (_numOfRounds);}
    void  RootDir            (const KKStr&           _rootDir);
    void SelectionMethod     (SVM_SelectionMethod    _selectionMethod);

    /**
     *@brief Adds specified Training Class to list taking ownership of it.
     */
    void                   AddATrainingClass (TrainingClassPtr  _trainClass);

    void                   AddATrainingClass (MLClassPtr  _newClass);  /**< Will assume that images for this class will 
                                                                        * be saved off the RootDirectory using its own 
                                                                        * name for the subdirectory name.
                                                                        */

    float                  AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const;


    KKStr                  DirectoryPathForClass (MLClassPtr  mlClass)  const;

    /**
     *@brief  Removes all training classes from the configuration; example use would be to remove all classes and then add 
     * the two needed by a Binary-Class-Pair classifier.
     */
    void                   EmptyTrainingClasses ();

    MLClassListPtr         ExtractClassList ()  const;          /**< Constructs new list of classes that caller will own. */

    MLClassListPtr         ExtractFullHierachyOfClasses ()  const;  /**< Extracts the list of classes including ones from Sub-Classifiers */

    MLClassListPtr         ExtractListOfClassesForAGivenHierarchialLevel (kkuint32 level)  const;

    TrainingConfiguration2Ptr  GenerateAConfiguraionForAHierarchialLevel (kkuint32  level,
                                                                          RunLog&   log
                                                                         )  const;

    FeatureNumList         GetFeatureNums ()  const;

    /**@brief  Returns features selected for the specified class-pairIf none were specified for that pair will return global feature nums. */
    FeatureNumList         GetFeatureNums (MLClassPtr  class1,
                                           MLClassPtr  class2
                                          );

    FeatureVectorListPtr   LoadFeatureDataFromTrainingLibraries (KKB::DateTime&  latestImageTimeStamp,
                                                                 bool&           changesMadeToTrainingLibraries,
                                                                 bool&           cancelFlag,
                                                                 RunLog&         log
                                                                );

    TrainingClassPtr       LocateByMLClassName (const KKStr&  className);

    bool                   NormalizeNominalFeatures ();

    kkint32                NumOfFeaturesAfterEncoding (RunLog&  log)  const;

    virtual void           ReadXML (XmlStream&      s,
                                    XmlTagConstPtr  tag,
                                    RunLog&         log
                                   );

    virtual void           Save (const KKStr& fileName)  const;

    virtual void           Save (ostream&  o)  const;

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


    static  bool           ConfigFileExists (const KKStr& _configFileName);

    static  KKStr          GetEffectiveConfigFileName (const  KKStr&  configFileName);

    void                   WriteXML (const KKStr&  varName,
                                     ostream&      o
                                    )  const;


  protected:
    void                   BuildTrainingClassListFromDirectoryStructure (const KKStr&  _subDir,
                                                                         bool&         _successful,
                                                                         KKStr&        _errorMessage,
                                                                         RunLog&       _log
                                                                        );

    XmlTokenPtr            ReadXMLBaseToken (XmlTokenPtr  t,
                                             RunLog&      log
                                            );

    void                   ReadXMLPost (RunLog&  log);

    /**
     *@brief  To be used by both Base Class and Derived classes to write fields that are 
     specific to 'TrainingConfiguration2'.
     */
    void                   WriteXMLFields (ostream& o)  const;


  private:
    void                   BuildTrainingClassListFromDirectoryEntry (const KKStr&  rootDir,
                                                                     const KKStr&  subDir,
                                                                     bool&         successful,
                                                                     KKStr&        errorMessage,
                                                                     RunLog&       log
                                                                    );


    void                   CreateModelParameters (const KKStr&           _parameterStr,
                                                  const FeatureNumList&  _selFeatures,
                                                  kkint32                _sectionLineNum,
                                                  kkint32                _parametersLineNum, 
                                                  kkint32                _featuresIncludedLineNum,
                                                  RunLog&                _log
                                                 );


    FeatureNumListPtr      DeriveFeaturesSelected (kkint32  sectionNum);

    void                   DetermineWhatTheRootDirectoryIs ();

    virtual
    FeatureVectorListPtr   ExtractFeatures (const TrainingClassPtr  trainingClass,
                                            KKB::DateTime&          latestTimeStamp,
                                            bool&                   changesMade,
                                            bool&                   cancelFlag,
                                            RunLog&                 log
                                           );

    SVMparamPtr            SVMparamToUse ()  const;

    void                   SyncronizeMLClassListWithTrainingClassList ();

    TrainingClassPtr       ValidateClassConfig    (kkint32  sectionNum,
                                                   RunLog&  log
                                                  );

    void                   ValidateConfiguration (RunLog&  log);
    
    void                   ValidateGlobalSection (kkint32  sectionNum,
                                                  RunLog&  log
                                                 );

    void                   ValidateOtherClass (MLClassPtr  otherClass,
                                               kkint32     otherClassLineNum,
                                               RunLog&     log
                                              );

    TrainingConfiguration2Ptr  
                           ValidateSubClassifier (const KKStr&  subClassifierName,
                                                  bool&         errorsFound,
                                                  RunLog&       log
                                                 );

    void                   ValidateTrainingClassConfig (kkint32  sectionNum,
                                                        RunLog&  log
                                                       );

    void                   ValidateTwoClassParameters (kkint32  sectionNum,
                                                       RunLog&  log
                                                      );

    ModelParamOldSVMPtr    OldSvmParameters ()  const;


    KKStr                  configFileNameSpecified;   /**< Config file name that was specified by caller before
                                                       * directory path was added by 'GetEffectiveConfigFileName'.
                                                       */

    KKStr                  configRootName;

    kkint32                examplesPerClass;
    FactoryFVProducerPtr   fvFactoryProducer;
    FileDescPtr            fileDesc;
    MLClassListPtr         mlClasses;
    bool                   mlClassesWeOwnIt;      /**< If we own it we will delete it in the destructor.  */
    ModelTypes             modelingMethod;
    ModelParamPtr          modelParameters;

    MLClassPtr             noiseMLClass;

    TrainingClassPtr       noiseTrainingClass;    /**< The specific Training Class that is to be used for noise images. */

    MLClassPtr             otherClass;            /**< class that is to be used for "Other" examples when performing
                                                   * adjustment calculations. This was done as part of the Dual Class
                                                   * classification program. When specified the actual training of 
                                                   * the classifier will not include this class. It will be used when 
                                                   * a class can not be determined.
                                                   */

    kkint32                otherClassLineNum;     /**< Line where OtherClass in configuration was defined. */

    KKStr                  rootDir;               /**< Common directory that all images for this training
                                                   * library come from. This is determined by iterating 
                                                   * through all the 'trainingClasses' entries and 
                                                   * looking for the common string that they all start by.
                                                   */

    KKStr                  rootDirExpanded;       /**< Same as 'rootDir' except environment variables will be expanded. */

    TrainingConfiguration2ListPtr
                           subClassifiers;        /**< Used when implementing a hierarchical classifier. This list is 
                                                   * a consolidated list from the 'TrainingClass'  objects that are 
                                                   * specified in the configuration file. Each 'TrainingClass' section 
                                                   * can specify a Sub-Classifier that is used to further break down that 
                                                   * class. Multiple 'TrainingClass' sections can specify the same 
                                                   * subClasifer; in that case we will want top only maintain one 
                                                   * instance of that classifier.
                                                   */

    VectorKKStr*           subClassifierNameList; /**< Used to communicate between by ReadXML, ReadXMLBaseToken, and ReadXMLPost */

    TrainingClassList      trainingClasses;       /**< List of  'Training_Class' objects; one for each 'Training_Classe'
                                                   * section defined in configuration file. Plus one for the 'Noise_Images'
                                                   * section.
                                                   */

    bool                   validateDirectories;

  public:
    virtual  KKStr  FactoryName () const  {return "TrainingConfiguration2::Factory";}   /**<  The name of the Factory class that produces an instance of this class. */

    static  TrainingConfiguration2Ptr  Manufacture 
                             (const KKStr&  _className,
                              const KKStr&  _configFileName,
                              bool          _validateDirectories,
                              RunLog&       _log
                             );

    /**
     * Will be one TrainingConfiguration2::Factory derived class for each derived class of "TrainingConfiguration2"; they will be responsible for 
     * creating an instance of the correct "TrainingConfiguration2" derived instance.  The static member "TrainingConfiguration2::registeredFactories"
     * will keep track of all instances of the Factory derived class.  
     */
    class  Factory
    {
    public:
      Factory (): className ("TrainingConfiguration2::Factory")  {}
      Factory (const KKStr&  _className): className (_className) {}

      virtual
      const KKStr&   ClassName ()  {return className;}

      virtual
      TrainingConfiguration2Ptr  Manufacture (const KKStr&  _configFileName,
                                              bool          _validateDirectories,
                                              RunLog&       _log
                                            );
    private:
      KKStr  className;
    };  /* Factory */

    static  Factory*   factoryInstace;

    static  Factory*   FactoryInstace ();


    static void  RegisterFatory (const KKStr&  className,
                                 Factory*      factory
                                );
  
  private:
    static map<KKStr,Factory*>*  registeredFactories;
    
    static  void  FinalCleanUp ();


  };  /* TrainingConfiguration2 */

  #define  _TrainingConfiguration2_Defined_

  typedef  TrainingConfiguration2::TrainingConfiguration2Ptr       TrainingConfiguration2Ptr;
  typedef  TrainingConfiguration2::TrainingConfiguration2Const     TrainingConfiguration2Const;
  typedef  TrainingConfiguration2::TrainingConfiguration2ConstPtr  TrainingConfiguration2ConstPtr;


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

    /** @brief  Returns the instance that has the same root name as 'rootName'  if none found returns NULL. */
    TrainingConfiguration2Ptr  LookUp (const KKStr&  rootName)  const;
  };

  typedef  TrainingConfiguration2List*  TrainingConfiguration2ListPtr;

  #define  _TrainingConfiguration2List_Defined_


  typedef  XmlElementTemplate<TrainingConfiguration2>  XmlElementTrainingConfiguration2;
  typedef  XmlElementTrainingConfiguration2*  XmlElementTrainingConfiguration2Ptr;
}  /* namespace KKMLL */




#endif
