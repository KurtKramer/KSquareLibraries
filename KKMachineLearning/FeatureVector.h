#if  !defined(_FEATUREVECTOR_)
#define  _FEATUREVECTOR_


/**
 *@class  KKMLL::FeatureVector
 *@brief  Represents a Feature Vector of a single example, labeled or unlabeled
 *@author  Kurt Kramer
 *@details Used for the representation of a Single example.  You create an instance of 
 *        this object for each single feature vector. You can subclass from this Class
 *        to make a specialized FeatureVector as in the PostLarvaeFV class.  Besides 
 *        keeping track of feature data this class will also track other fields such as 
 *        ExampleFileName which should indicate where the FeatureVector was derived from,
 *        probability, breakTie, and others.
 *@see FeatureVectorList
 *@see PostLarvaeFV
 *@see FeatureFileIO
 */


#include "KKStr.h"
#include "KKQueue.h"
#include "RunLog.h"

#include "Attribute.h"
#include "ClassStatistic.h"
#include "FeatureFileIO.h"
#include "FeatureNumList.h"
#include "FileDesc.h"
#include "MLClass.h"


namespace KKMLL 
{


  #ifndef  _FeatureFileIO_Defined_
    class  FeatureFileIO;
    typedef  FeatureFileIO*  FeatureFileIOPtr;
  #endif

#if  !defined(_ClassProbList_Defined_)
   class  ClassProbList;
    typedef  ClassProbList*  ClassProbListPtr;
#endif


  /**
   *@class FeatureVector
   *@brief  Represents a Feature Vector of a single example, labeled or unlabeled
   *@details Used for the representation of a Single example. You create an instance of this object for each single feature
   * vector. You can subclass from this Class to make a specialized FeatureVector as in the PostLarvaeFV class.  Besides
   * keeping track of feature data this class will also track other fields such as ExampleFileName which should indicate
   *where the FeatureVector was derived from, probability, breakTie, and others.
   *@see FeatureVectorList
   *@see PostLarvaeFV
   *@see FeatureFileIO
   */
  class  FeatureVector 
  {
  public:
    typedef  float  FVFloat;
    typedef  FeatureVector*  FeatureVectorPtr;

    FeatureVector (kkint32  _numOfFeatures);

    FeatureVector (const FeatureVector&  _example);

    virtual  ~FeatureVector ();

    virtual  FeatureVectorPtr  Duplicate ()  const;

    void  BreakTie         (float         _breakTie)        {breakTie         = _breakTie;}         /**< Update the BreakTie value. */
    void  MLClass          (MLClassPtr    _mlClass)         {mlClass          = _mlClass;}          /**< Assign a class to this example. */
    void  ExampleFileName  (const KKStr&  _exampleFileName) {exampleFileName  = _exampleFileName;}  /**< Name of source of feature vector, ex: file name of image that the feature vector was computed from. */
    void  MissingData      (bool          _missingData)     {missingData      = _missingData;}      /**< True indicates that not all the feature data was present when this example was loaded from a data file. */
    void  OrigSize         (float         _origSize)        {origSize         = _origSize;}         /**< The value of Feature[0] before normalization. */
    void  PredictedClass   (MLClassPtr    _predictedClass)  {predictedClass   = _predictedClass;}
    void  Probability      (float         _probability)     {probability      = _probability;}      /**< Assign a prediction probability to this example.  */
    void  Version          (kkint16       _version)         {version          = _version;}

    /**
     *@brief Assign a value to a specific feature number for the feature vector.
     *@details This method will validate that '_featureNum' is not out of range (0 - 'numOfFeatures').
     * This will prevent the caller from corrupting memory.
     *@param[in] _featureNum Feature Number to assign '_featureValue' to.
     *@param[in] _featureValue Value to assign to feature '_featureNum'.
     */
    void  FeatureData (kkint32 _featureNum,
                       float   _featureValue
                      );


    /** @brief  Returns the total of all Feature Attributes for this feature vector. */
    float  TotalOfFeatureData ()  const;

    virtual
    kkint32 MemoryConsumedEstimated ()  const;

    /**  
     * @brief  Assign a specific example a higher weight for training purposes.
     * @details The SVM will multiply the cost parameter by this amount when training the classifier
     *  for this specific example.
     */
    void  TrainWeight      (float  _trainWeight)   {trainWeight  = _trainWeight;}  


    /** @brief  Indicated whether an expert has validated the class assignment. */
    void  Validated        (bool   _validated)     {validated    = _validated;}


    float          BreakTie           () const  {return breakTie;}         /**< The difference in probability between the two most likely classes. */
    MLClassPtr     MLClass            () const  {return mlClass;}          /**< Class that is example is assigned to.                              */
    const KKStr&   MLClassName        () const;                            /**< Name of class that this example is assigned to.                    */
    const KKStr&   MLClassNameUpper   () const;                            /**< Name of class that this example is assigned to.                    */
    const KKStr&   ExampleFileName    () const  {return exampleFileName;}  /**< Name of file that this FeatureVector was computed from.            */
    KKStr          ExampleRootName    () const;                            /**< Root name of file that this FeatureVector was computed from.       */
    bool           MissingData        () const  {return missingData;}      /**< True indicates that one or more features were missing.             */        
    kkint32        NumOfFeatures      () const  {return numOfFeatures;}    /**< Number of features in this FeatureVector.                          */
    float          OrigSize           () const  {return origSize;}         /**< The value of Feature[0] before normalization.                      */
    MLClassPtr     PredictedClass     () const  {return predictedClass;}
    const KKStr&   PredictedClassName () const;
    float          Probability        () const  {return probability;}      /**< The probability assigned by classifier to the predicted class.     */
    float          TrainWeight        () const  {return trainWeight;}
    bool           Validated          () const  {return validated;}
    kkint16        Version            () const  {return version;}

    float          FeatureData        (kkint32 featureNum)  const;         /**< returns The value of 'featureNum'                             */
    const float*   FeatureData        () const  {return featureData;}      /**< Returns as a pointer to the feature data itself.        */
    float*         FeatureDataAlter   ()        {return featureData;}      /**< Same as 'FeatureData() except you can modify the data.  */
                                                                      
    const float*   FeatureDataConst   () const  {return featureData;}
    bool           FeatureDataValid   ();

    void    ResetNumOfFeatures (kkint32  newNumOfFeatures);  /**< Used to reallocate memory for feature data. */

    void    AddFeatureData (kkint32  _featureNum,   /**< Indicates which feature number to update.  */
                            float    _featureData   /**< New value to assign to '_featureNum'.      */
                           );

    bool  operator== (FeatureVector &other_example)  const;


    /** @brief Used by container classes such as 'FeatureVectorList'.  This way they can determine real underlying class. */
    //virtual  const char*  UnderlyingClass ()  const  {return  "FeatureVector";}


  protected:
    void  AllocateFeatureDataArray ();

    float*         featureData;
    kkint32        numOfFeatures;


  private:
    float          breakTie;         /**< @brief The difference in probability between the two most likeliest
                                      * classes as per the classifier. 
                                      */
    MLClassPtr     mlClass;
    KKStr          exampleFileName;
    bool           missingData;      /**< @brief Indicates that some features were flagged as missing in 
                                      * data file. 
                                      */
    float          origSize;
    MLClassPtr     predictedClass;   /**< @brief Represents the class that the Classifier assigned to this 
                                       * image; added to aid in the grading function.2 
                                       */

    float          probability;      /**< @brief Probability assigned by classifier to predicted Class. */

    float          trainWeight;      /**< @brief Weight to assign to this training image during Training.
                                      *@details  Will default to 1.0. during the SVM training process the 
                                      * Cost parameter will be multiplied by this amount.  
                                      */

    bool           validated;        /**< @brief  If true then the 'mlClass' entry has been validated by 
                                      * an expert; was introduced when the DataBase was implemented.
                                      */

    kkint16        version;          /**< This is the same versionNumber as in FeatureVectorList
                                      * It is related to the Feature calculation routine.  This
                                      * will assist in us changing the feature calculations in the 
                                      * future and  objects and methods having a means of 
                                      * knowing if the features are similar.
                                      */

  };  /* FeatureVector */


  typedef  FeatureVector* FeatureVectorPtr;

  #define  _FeatureVector_Defined_


  class  FeatureVectorComparison;



  /**
   *@class FeatureVectorList
   *@brief   Container class for FeatureVector derived objects. 
   *@details Supports various functions with respect to maintaining a list of FeatureVector's.  These
   *         include randomizing there order creating a stratified list by class, extracting a list 
   *         of classes,  sorting by various criteria.
   */
  class  FeatureVectorList:  public KKQueue<FeatureVector>
  {
  public:
    typedef  float  FVFloat;

    typedef  FeatureVectorList*  FeatureVectorListPtr;

    /**
     * When you use this constructor you need to define immediately provide the appropriate FileDesc instance via
     * calling "ResetFileDesc" method.
     */
    FeatureVectorList ();


    /**
     *@brief Will create a new empty list of FeatureVector's.
     *@param[in] _fileDesc Describes the feature data such as number of features and their attributes.
     *@param[in] _owner True indicates that this list will own its contents and when this list is deleted it
     *           will call the destructor for all its contents.
     *@param[out] _log Log file to send messages to.
     */
    FeatureVectorList (FileDescConstPtr  _fileDesc,
                       bool              _owner
                      );

  private:
    /**
     *@brief  Will create a duplicate List of examples, in the same order.
     *@details If the source 'examples' owns its entries, then new duplicate entries will be created, and the
     *  new 'FeatureVectorList' will own them, otherwise will only get pointers to existing instances in 'examples'.
     */
    FeatureVectorList (FeatureVectorList&  examples); 


  public:
    /** 
     *@brief  Create a duplicate list, depending on the '_owner' parameter may also duplicate the contents.
     *@details If '_owner' = true will create new instances of contents and own them.  If 'owner' = false, will
     *         copy over pointers to existing instances.
     *@param[in]  examples  Existing list of examples that we are going to copy.
     *@param[in]  _owner  If set to true will make a duplicate of the FeatureVectors in 'examples' and own then 
     *                    otherwise will just point to the same existing examples and not own them.
     */
    FeatureVectorList (const FeatureVectorList&  examples,
                       bool                      _owner
                      );


    /**
     *@brief  Will create a list of consisting of the subset of examples in '_examples' which are members of ImagesClasses.
     *@details  Will not own the contents;  it will just point to the existing examples that were in '_examples'.
     *@param[in] _mlClasses  List of classes that we want to include.
     *@param[in] _examples      Source of feature Vectors to extract from.
     *@param[out] _log          Log file to send messages to.
    */ 
    FeatureVectorList (MLClassList&        _mlClasses,
                       FeatureVectorList&  _examples
                      );

    
    /** 
     *@enum IFL_SortOrder
     *@brief  Represents the different orders that a list of FeatureVector instances in a FeatureVectorList object can be in.  
     */
    enum  class  IFL_SortOrder
             {IFL_UnSorted, 
              IFL_ByName, 
              IFL_ByProbability, 
              IFL_ByBreakTie, 
              IFL_ByRootName, 
              IFL_ByClassName
             };


    virtual  ~FeatureVectorList ();

  
    // Access methods.
    IFL_SortOrder      CurSortOrder    () const  {return curSortOrder;}
    kkint32            FeatureCount    () const  {return numOfFeatures;}
    FileDescConstPtr   FileDesc        () const  {return fileDesc;}
    const  KKStr&      FileName        () const  {return fileName;}
    kkint32            NumOfFeatures   () const  {return numOfFeatures;}
    kkint16            Version         () const  {return version;}
    //virtual  const char*      UnderlyingClass () const  {return "FeatureVectorList";}


    void   FileName (const KKStr& _fileName)  {fileName = _fileName;}
    void   Version  (kkint16      _version)   {version  = _version;}

    void   AddSingleExample (FeatureVectorPtr  _imageFeatures);  /**< @brief Same as PushOnBack */

    void   AddQueue (const FeatureVectorList&  examplesToAdd);   /**< @brief Add the contents of 'examplesToAdd' to the end of this list. */

    FeatureNumList  AllFeatures ();                              /**< @brief Will return a FeatureNumList instance with all features selected. */

    void  AppendToFile (KKStr                  _fileName,
                        FeatureFileIOPtr       _driver,
                        const FeatureNumList&  _selFeatures
                       );

    /**
     *@brief Will search for the example with the same name as '_imageFileName'.
     *@details  If the list is  already sorted in name order will use a Binary Search otherwise a linear search.
     *  The method 'SortByImageFileName' will set a flag 'curSortOrder' indicating if the examples are sorted.
     *  The idea is that if you know that will be doing many searches then for performance reasons you
     *  should call 'SortByImageFileName' first. The methods 'PushOnBack', 'PushOnFront', and 'AddSingleExample'
     *  will reset 'curSortOrder' to unsorted.
     */
    FeatureVectorPtr  BinarySearchByName (const KKStr&  _imageFileName)  const;


    void  CalcStatsForFeatureNum (kkint32   _featureNum,
                                  kkint32&  _count,
                                  float&    _total,
                                  float&    _mean,
                                  float&    _var,
                                  float&    _stdDev
                                 );
   

    KKStr  ClassStatisticsStr ()  const;

    KKMLL::AttributeTypeVector  CreateAttributeTypeTable ()  const;

    vector<kkint32>  CreateCardinalityTable ()  const;


    /**
     *@brief Will create a list of FeatureVectors where the class assignment will reflect the specified Hierarchy level specified by 'level'.
     *@details The hierarchy of a given class will be indicated by underscore characters in the class name.
     *@code
     *    ex:   Level 1:  gelatinous
     *          Level 2:  gelatinous_hydromedusae
     *          Level 3:  gelatinous_hydromedusae_solmundella
     *    If the 'level' parameter is set to 1 then all FeatureVectors who's class name starts with 'gelatinous' will be grouped
     *    together under the class name 'gelatinous_hydromedusae'.
     *@endcode
     */
    FeatureVectorListPtr  CreateListForAGivenLevel (kkint32  level);


    /**
     *@brief  Creates a duplicate of list using the same container.
     */
    virtual
    FeatureVectorListPtr  Duplicate (bool _owner)  const;


    /**
     *@brief  Creates a duplicate of list and also duplicates it contents.
     *@return Duplicated list with hard-copy of its contents.
     */
    virtual
    FeatureVectorListPtr  DuplicateListAndContents ()  const;

    KKStrListPtr   ExtractDuplicatesByExampleFileName ();


    /**
     *@brief  Returns: a list of 'FeatureVector' objects that have duplicate root file names.
     *@details The returned list will not own these items.  All instances of the duplicate objects will be returned.
     *  Ex:  if three instances have the same ExampleFileName all three will be returned.
     */
    FeatureVectorListPtr  ExtractDuplicatesByRootImageFileName ();


    FeatureVectorListPtr  ExtractExamplesForAGivenClass (MLClassPtr  _mlClass,
                                                         kkint32     _maxToExtract = -1,
                                                         float       _minSize      = -1.0f
                                                        )  const;

    FeatureVectorListPtr  ExtractExamplesForClassList (MLClassListPtr  classes);

    VectorDouble   ExtractMeanFeatureValues ();


    /**
     *@brief  Will return a random sampling by class of our FeatureVector's; with a minimum per class of 'minClassCount'.
     *@param[in] percentage  The percentage between 0.0 and 100.0 of each class to randomly sample.
     *@param[in] minClassCount The minimum per class to keep.
     */
    FeatureVectorListPtr  ExtractRandomSampling (float    percentage,    /**<  A percentage between 0.0 and 100.0 */
                                                 kkint32  minClassCount
                                                );

    /**
     *@brief Will create a list of FeatureVectors where the class assignment will reflect the specified Hierarchy level specified by 'level'.
     *@details The hierarchy of a given class will be indicated by underscore characters in the class name.
     *@code
     *     ex:   Level 1:  gelatinous
     *           Level 2:  gelatinous_hydromedusae
     *           Level 3:  gelatinous_hydromedusae_solmundella
     *     If the 'level' parameter is set to 1 then all FeatureVectors who's class name starts with 'gelatinous' will be 
     *     grouped together under the class name 'gelatinous_hydromedusae'.
     *@endcode
     *@bug  This method appears to be a duplicate of 'CreateListForAGivenLevel'; We should verify this and get rid of one of them.
     */
    FeatureVectorListPtr      ExtractExamplesForHierarchyLevel (kkuint32 level);

    MLClassListPtr            ExtractListOfClasses ()  const;

    bool                      AllFieldsAreNumeric ()  const;                  /**< @brief  Returns true if all fields are numeric, no nominal fields.              */
    KKMLL::AttributeType      FeatureType        (kkint32 featureNum) const;  /**< @brief  Returns the type of attribute for specified 'featureNum'. @see FileDesc */
    KKStr                     FeatureTypeStr     (kkint32 featureNum) const;
    kkint32                   FeatureCardinality (kkint32 featureNum) const;  /**< @brief Returns the number of values defined for a Nominal Field. @see FileDesc::Cardinality */
    const KKStr&              FieldName          (kkint32 featureNum) const;  /**< @brief Returns name of Attribute Field.                                        */

    ClassStatisticListPtr     GetClassStatistics ()  const;                   /**< @brief Returns the number of FeatureVectors per class @see ClassStatisticList */
    ClassProbListPtr          GetClassDistribution () const;

    kkint32                   GetClassCount (MLClassPtr  c)  const;           /**< @brief Returns number of examples for a specific Class (MLClass).   */

    //RunLog&                   Log () {return  log;}


    /**
     * @brief  Returns a pointer to the FeatureVector which has '_imageFileName' 
     *@details If the list is currently sorted by ExampleFileName  (curSortOrder == IFL_ByName)  then a Binary Search is performed
     *           otherwise a sequential search is performed.
     */
    FeatureVectorPtr          LookUpByImageFileName (const KKStr&  _imageFileName)  const;


    /** 
     *@brief   Returns a pointer to the FeatureVector who's ExampleFileName rootname = _rootName *\
     *@details If the list is currently sorted by ExampleFileName  (curSortOrder == IFL_ByRootName)  then a Binary Search is performed
     *         otherwise a sequential search is performed.   The parameter _rootName is assumed to be just the root name of the file.
     *         that is you used osGetRootName  to et the root part.
     */
    FeatureVectorPtr          LookUpByRootName (const KKStr&  _rootName);

    // void                  Sort (FeatureVectorComparison   comparison);

    float                     MajorityClassFraction () const; /**< Return's the fraction that the majority class makes up in this list. */

    /**
     *@brief Creates an instance of a Empty FeatureVectorList
     */
    virtual
    FeatureVectorListPtr      ManufactureEmptyList (bool _owner)  const;

    virtual
    kkint32                   MemoryConsumedEstimated ()  const;

    bool                      MissingData () const;  /**< Returns true if 1 or more entries have missing data. */

    kkint32                   NumEntriesOfAGivenClass (MLClassPtr  mlClass) const  {return GetClassCount (mlClass);}

    /**
     *@brief  Using list of ImageFileNames in a file('fileName') create a new FeatureVectorList instance with examples in order based 
     *       off contents of file. If error occurs will return NULL.
     *@param[in]  fileName  Name of file that contains a list of ExampleFileName's  with one entry per line.
     *@returns  A new list of FeatureVector instances in the order dictated by 'fileName'.
     */
    FeatureVectorListPtr      OrderUsingNamesFromAFile (const KKStr&  fileName,
                                                        RunLog&       log
                                                       );

    void                      RemoveDuplicateEntries (bool    allowDupsInSameClass,
                                                      RunLog& runLog
                                                     );

    /**
     *@brief  Will save into a file the current ordering of FeatureVector instances in list.  
     *@details This file can then be used at a later time to reproduce the exact same ordering of FeatureVector objects from a file.
     *@see OrderUsingNamesFromAFile
     *@param[in]  fileName  Name of file where ImagFileNames will be written to.
     *@param[out] successful Indicates if list is successfully written.
    */
    void  SaveOrderingOfImages (const KKStr&  fileName,
                                bool&         successful
                               );

    void  SynchronizeSymbolicData (FeatureVectorList&  otherData,
                                   RunLog&             log
                                  );

    void  PrintClassStatistics (std::ostream&  o)  const;

    void  PrintClassStatisticsHTML (std::ostream&  o)  const;

    void  PrintFeatureStatisticsByClass (std::ostream&  o)  const;

    void  PushOnBack (FeatureVectorPtr  image);   /**< @brief Overloading the PushOnBack function in KKQueue so we can monitor the Version and Sort Order. */

    void  PushOnFront (FeatureVectorPtr  image);  /**< @brief Overloading the PushOnFront function in KKQueue so we can monitor the Version and Sort Order. */
   
    void  ResetNumOfFeaturs (kkint32 newNumOfFeatures);

    void  ResetFileDesc (FileDescConstPtr  newFileDesc);  /**< You would use this if youRecalc all the data to a newer version of the file. */

    void  ReSyncSymbolicData (FileDescPtr  newFileDesc);

    void  RemoveEntriesWithMissingFeatures (RunLog&  log);  /**< Will delete entries from list that have missing data. */


    /**
     * @details
     *   Determines if the other FeatreVectorList has the same underlining layout;  that is each
     *   field is of the same type and meaning.  This way we can determine if one list contains
     *   Apples while the other contains Oranges.
     */
    bool  SameExceptForSymbolicData (const FeatureVectorList&  otherData,
                                     RunLog&                   log
                                    )  const;


    FeatureVectorListPtr  StratifyAmoungstClasses (kkint32  numOfFolds,
                                                   RunLog&  log
                                                  );


    FeatureVectorListPtr  StratifyAmoungstClasses (MLClassListPtr  mlClasses,
                                                   kkint32         maxImagesPerClass,
                                                   kkint32         numOfFolds,
                                                   RunLog&         log
                                                  );
   
    void  SortByClass         (bool  reversedOrder = false);

    void  SortByImageFileName (bool  reversedOrder = false);

    void  SortByProbability   (bool  reversedOrder = false);

    void  SortByBreakTie      (bool  reversedOrder = false);

    void  SortByRootName      (bool  reversedOrder = false);

  private:
    class  BreakTieComparison;
    class  BreakTieComparisonReversed;
    class  ClassNameComparrison;
    class  ClassNameComparrisonReversed;
    class  ImageFileNameComparison;
    class  ImageFileNameComparisonReversed;
    class  ProbabilityComparison;
    class  ProbabilityComparisonReversed;
    class  RootNameComparrison;
    class  RootNameComparrisonReversed;


    void  SortBy (bool reversedOrder,
                  bool (*comp)(FeatureVectorPtr l, FeatureVectorPtr r)
                 );

    void  ValidateFileDescAndFieldNum (kkint32      fieldNum, 
                                       const char*  funcDesc
                                      )  const;



    /** 
     * @brief  Keeps track of the current order of FeatureVector entries in the list.
     * @details This helps functions such as LookUpByImageFileName to work more efficiently.  If in ExampleFileName order
     *  it can then perform a binary search rather than a seq. scan. This field is updated by the different sort
     *  routines, and by the methods that allow you to add an entry.
     */
    IFL_SortOrder     curSortOrder;

    FileDescConstPtr  fileDesc;

    KKStr             fileName;

    kkint32           numOfFeatures;

    kkint16           version;  /**< Represents the version of the Feature data,  when ever I update the
                                 * way Features are calculated I increment the VersionNum in the respective
                                 * "FeatureVectorProducer" derived class. This way if we load a older
                                 * FeatureData file we can be aware of this.  Methods like FeatureDataReSink
                                 * will force the recalculation of Feature data if not up-to-date.  Also
                                 * works in coordination with the version field in the PostLarvaeFV object.
                                 * A value of 0 indicates that we do not know what Version the feature data
                                 * is. This can happen when not all the PostLarvaeFV objects in the list have
                                 * the same version number.
                                 */

  };  /* FeatureVectorList */


  #define  _FeatureVectorList_Defined_


  typedef  FeatureVectorList*  FeatureVectorListPtr;

  typedef  FeatureVectorList const *  FeatureVectorListConstPtr;

}  /* namespace KKMLL */

#endif

