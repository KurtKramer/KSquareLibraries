#ifndef  _FEATURENUMLIST_
#define  _FEATURENUMLIST_

//************************************************************************
//*                           FeatureNumList                             *
//************************************************************************

/**
 *@class KKMachineLearning::FeatureNumList
 *@brief Keeps track of selected features.  
 *@details Used by SVMModel and the newer 'Model', 'ModelParam' based classes.
 * Each instance of this class will have an associated 'FileDesc' instance. It
 * is meant to keep track of selected features from that instance('FileDesc').
 * <br />
 * This class is meant to work with both the FeaureVector, Model, and ModelParm
 * based classes.  This will allow us to select specific features
 * from the FeatureVector instances that are to be used.
 * <br />
 * At no time are feature numbers that are out of range of the associated 
 * FileDesc instance to be selected.
 * <br />
 * Two of the constructors allow you to specify a list of features in a string.
 * The list can consist of single features and/or ranges of features. Ranges of
 * features are specified by using the dash('-') charter between two numbers.
 * The comma(',') character will be used as a separator.  "All" specifies all are
 * to be selected except those that are flagged as  'IgnoreAttribute' in the 
 * Associated FileDesc instance. The list should be in ascending order.
 *
 *@code
 *  Example Strings:
 *    "1,2,3,10,20"    Selects [1,2,3,10, 20].
 *    "1,4-7,9-12,13"  Selects [1,4,,5,6,7,9,10,11,12,13]
 *    "All"            Selects all features that '_fileDesc' includes accept
 *                     those that are flagged as 'IgnoreAttribute' in the 
 *                     associated FileDesc instance.
 *@endcode
 */

#include  "Attribute.h"
#include  "KKBaseTypes.h"
#include  "BitString.h"
#include  "RunLog.h"
#include  "KKStr.h"


namespace KKMachineLearning 
{
  #ifndef  _FILEDESC_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif


  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;

  class  FeatureNumList
  {
  public:
    typedef  FeatureNumList*  FeatureNumListPtr;
    typedef enum {IncludeFeatureNums, ExcludeFeatureNums}  FeatureSelectionType;

    /** @brief  Copy constructor.  */
    FeatureNumList (const FeatureNumList&  featureNumList);


    /** @brief  Constructs a new 'FeatureNumList' instance with NO features selected. */
    FeatureNumList (FileDescPtr  _fileDesc);


    /**
     *@brief Constructs a 'FeatureNumList' instance using the set bits in 'bitString' to indicate which features are selected.
     *@details For each bit position in 'bitString' that is set to '1' the corresponding feature will be selected.  So the bit string '0110111' with consists of
     * bits 0, 1, 2, 5, and 6  set to one will cause the features elected to be set to (0, 1, 2, 4, 6).<br />
     * This is a useful constructor when dealing with dataset's that have a large number of features such as DNA based dataset's.
     *@param[in]  _fileDesc Description of the feature data.
     *@param[in]  bitString A bit string that indicates which features are selected.  Bits that are set to 1 indicate that
     *            the corresponding feature is selected.
     */
    FeatureNumList (FileDescPtr       _fileDesc,
                    const BitString&  bitString
                   );


    /**
     * @brief Constructs a 'FeatureNumList' instance from a string that contains a list of selected features.
     * @details The list can consist of single features and/or ranges of features. Ranges of features
     *  are specified by using the dash('-') charter between two numbers.  The comma(',')
     *  character will be used as a separator.  "All" specifies all ate to be selected except
     *  those that are flagged as  'IgnoreAttribute' in the associated FileDesc instance.
     *  The list should be in ascending order.
     * @code
     *  ex's:
     *    "1,2,3,10,20"    Selects [1,2,3,10, 20].
     *    "1,4-7,9-12,13"  Selects [1,4,,5,6,7,9,10,11,12,13]
     *    "All"            Selects all features that '_fileDesc' includes accept those that are
     *                     flagged as 'IgnoreAttribute' in the associated FileDesc instance.
     * @endcode
     * @see ExtractFeatureNumsFromStr
     * @param[in]  _fileDesc Description of the feature data.
     * @param[in]  _featureListStr  Comma separated string that contains list of selected features; a range of
     *             features can be specified using the dash('-') character.  ex:  The string "1,3,5-7,9" indicates
     *             that features 1,3,5,6,7,9 are selected.
     * @param[out] _valid returns false if '_featureListStr' is not a valid format.
     */
    FeatureNumList (FileDescPtr   _fileDesc,
                    const KKStr&  _featureListStr,
                    bool&         _valid
                   );


    /**
     * @brief Constructs a 'FeatureNumList' instance from a string where '_selectionType' indicates if the features
     *  should be included or excluded.
     *
     * @details The list can consist of single features and/or ranges of features. Ranges of features
     *  are specified by using the ('-') charter between two numbers.  The comma(',')
     *  Character will be used as a separator.  The list should be in ascending order.
     *  The '_selectionType' parameter specifies weather we are going to select these
     *  features or exclude them form the list of all features(complement).
     * @code
     *   ex's:
     *     "1,2,3,10,20"    Selects [1,2,3,10, 20].
     *     "1,4-7,9-12,13"  Selects [1,4,,5,6,7,9,10,11,12,13]
     *     "All"            Selects all features that '_fileDesc' includes accept those that are
     *                      flagged as 'IgnoreAttribute' in the associated FileDesc instance.
     * @endcode
     * @param[in]  _fileDesc Description of the feature data.
     * @param[in]  _selectionType  Specifies whether the features listed in '_featureListStr' should be
     *              included (IncludeFeatureNums) or excluded(ExcludeFeatureNums).
     * @param[in]  _featureListStr  Comma separated string that contains list of features to be included or excluded;
     *             a range of features can be specified using the dash('-') character.  ex:  The string "1,3,5-7,9"
     *             indicates that features 1,3,5,6,7,9 are selected.
     * @param[out] _valid returns false if '_featureListStr' is not a valid format.
     */
    FeatureNumList (FileDescPtr           _fileDesc,
                    FeatureSelectionType  _selectionType,
                    const KKStr&          _featureListStr,
                    bool&                 _valid
                   );


    ~FeatureNumList ();   



    // Access Methods.
    const kkuint16*  FeatureNums    () const  {return featureNums;}
    FileDescPtr      FileDesc       () const  {return fileDesc;}
    kkint32          NumOfFeatures  () const  {return numOfFeatures;}
    kkint32          NumSelFeatures () const  {return numOfFeatures;}





    /** @brief Adds 'featureNum' to the list of selected features.  If it is already selected nothing happens. */
    void    AddFeature (kkuint16  featureNum);


    /** @brief Returns true if all features are selected. */
    bool    AllFeaturesSelected ()  const;


    KKMachineLearning::AttributeType  FeatureAttributeType (kkint32 idx)  const;


    /** @brief Create a FeatureNumList object where all features are selected, except ones that are flagged as IgnoreAttribute in '__fileDesc'. */
    static  FeatureNumList   AllFeatures (FileDescPtr  fileDesc);


    /** @brief Compare with another featureNumList returning -1, 0, and 1 indicating less_than, equal, or greater_than. */
    kkint32 Compare (const FeatureNumList&  _features)  const;


    /** @brief Perform a comkplement of selected features.  That is if a feature is selected turn it off and if it is not selected then turn it on. */
    FeatureNumList  Complement ()  const;


    /**
     * @brief Allocates a array of kkint32's that is a copy  of FeatureNums; The caller will own the array and is responsible for deleting it.
     * @return A dynamically allocated array if short its that will consists of a list of selected features.
     */
    kkuint16*  CreateFeatureNumArray ()  const;



    /**
     * @brief   Will select the features specified in "featureListStr".
     * @details The format is a comma delimited string, where each number represents a feature, ranges can be specified with
     * a dash("-").  "All" will select all features that are not flagged as a 'IgnoreAttribute'  in the associated FileDesc instance.
     * @code
     *  ex's:   String          Selected Features
     *         "1,2,3,10,20"    [1,2,3,10, 20].
     *         "1,4-7,9-12,13"  [1,4,,5,6,7,9,10,11,12,13]
     *         "All"            Selects all features that '_fileDesc' includes accept those that are
     *                          flagged as 'IgnoreAttribute' in the associated FileDesc instance.
     * @endcode
     */
    void  ExtractFeatureNumsFromStr (KKStr  featureListStr,
                                     bool&   valid
                                    );

    bool   IsSubSet (const FeatureNumList&  z);


    /**@brief returns true if _featureNum in  featureNums, meaning it was selected. */
    bool   InList (kkuint16 featureNum)  const;


    void   Load (const KKStr&  _fileName,
                 bool&         _successful,
                 RunLog&       _log
                );

    kkint32  MemoryConsumedEstimated ()  const;


    /**
     * @brief Generates a new FeatureNumList object that will select at random 'numToKeep' features from this instance.
     * @param[in] numToKeep Number of features to select randomly from existing instance.
     * @return Dynamically allocated instance of a ImageFeaturesList with randomly selected features.
     */
    FeatureNumListPtr  RandomlySelectFeatures (kkint32  numToKeep)  const;

    void  Save (const KKStr&  _fileName,
                bool&         _successful,
                RunLog&       _log
               );
   

    void  Save (std::ostream&  o);


    /** @brief  Selects all featues except those that are flagged as 'IgnoreAttribute' in the associated FileDesc instance. */
    void   SetAllFeatures ();

    bool   Test (kkuint16 _featureNum)  const;  /**< Same as 'InList' */


    void   ToBitString (BitString&  bitStr)  const;


    KKStr  ToHexString ()  const;


    /** @brief Returns comma delimited list of all features selected.  Will make use of range specification. */
    KKStr       ToString ()  const;


    KKStr       ToCommaDelStr ()  const {return  ToString ();}


    /** @brief   Turns off all features so that no feature is selected.      */
    void        UnSet ();


    /** @brief Turns off specified feature 'featureNum'; if 'featureNum' is not turned on then nothing happens; same as using 'operator-='.  */
    void        UnSet (kkuint16  featureNum);


    /**
     * @brief Returns back the selected feature.
     * @details  A FeatureNumList instance consists of an list of selected features.  It is logically like an array of selected
     *  features that is the same length as the number of selected features.
     * @code
     *  Example code that scans the FeatureNumList object  'goodFeatures'
     *
     *  void  PrintSelectedFeatures (const FeatureNumList&  goodFeatures)
     *  {
     *    cout << "Selected Features: ";
     *    for  (kkint32 x = 0;  x < goodFeatures.NumOfFeatures ();
     *    {
     *      if  (x > 0)  cout << ",";
     *      cout << goodFeatures[x];
     *    }
     *    cout << endl;
     *  }
     * @endcode
     * @param[in]  _idx  The position in this instance that you want to return.
     * @return  Selected feature at position '_idx'.
     */
    kkuint16      operator[] (kkint32  idx)  const;



    FeatureNumList&  operator=  (const FeatureNumList&    _features);
    FeatureNumList&  operator=  (const FeatureNumListPtr  _features);
    FeatureNumList   operator+  (const FeatureNumList&    rightSide)  const;
    FeatureNumList   operator+  (kkuint16                 rightSide)  const;
    FeatureNumList&  operator+= (const FeatureNumList&    rightSide);
    FeatureNumList&  operator+= (kkuint16                 featureNum);
    FeatureNumList   operator-  (const FeatureNumList&    rightSide)  const;
    FeatureNumList   operator-  (kkuint16                 rightSide)  const;
    FeatureNumList&  operator-= (kkuint16                 rightSide);

    FeatureNumList   operator*  (const FeatureNumList&    rightSide)  const;
    bool             operator== (const FeatureNumList&    _features)  const;
    bool             operator>  (const FeatureNumList&    _features)  const;
    bool             operator<  (const FeatureNumList&    _features)  const;

  private:
    /*
     * @brief  Constructs an instance with no features selected and no associated 'FileDesc' instance.
     *  This is a private constructor and is used for internal use of 'FeatureNumList only.
     */
    FeatureNumList ();


    void   AllocateArraySize (kkint32 size);   /**< @brief  Make sure that FeatureNums is allocated to at least this size. */

    kkuint16*    featureNums;              /**< @brief The feature nums in this array are always kept in ascending order.  
                                            * @details There will be 'numOfFeatures' in this array.  'featureNumsAllocatedSize' 
                                            * indicates the size allocated, if more space is needed you need to call 
                                            * 'AllocateArraySize' to increase it.
                                            */
    kkint32      featureNumsAllocatedSize;
    FileDescPtr  fileDesc;
    kkint32      numOfFeatures;
  };  /* FeatureNumList */


  typedef  FeatureNumList*  FeatureNumListPtr;

  #define  _FeatureNumListDefined_


  std::ostream& operator<< (      std::ostream&     os, 
                            const FeatureNumList&   features
                           );


  std::ostream& operator<< (      std::ostream&       os, 
                            const FeatureNumListPtr&  features
                           );

  extern
  const  char*  FeatureDecriptions[];
} /* namespace KKMachineLearning */

#endif

