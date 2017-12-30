#if  !defined(_FEATURENUMLIST2_)
#define  _FEATURENUMLIST2_

/**
 *@class KKMLL::FeatureNumList
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
 * features are specified by using the dash('-') character between two numbers.
 * The comma(',') character will be used as a separator.  "All" specifies all are
 * to be selected except those that are flagged as  'Ignore' in the 
 * associated FileDesc instance. The list should be in ascending order.
 *
 *@code
 *  Example Strings:
 *    "1,2,3,10,20"    Selects [1,2,3,10, 20].
 *    "1,4-7,9-12,13"  Selects [1,4,,5,6,7,9,10,11,12,13]
 *    "All"            Selects all features that '_fileDesc' includes accept
 *                     those that are flagged as 'Ignore' in the 
 *                     associated FileDesc instance.
 *@endcode
 */

#include "KKBaseTypes.h"
#include "BitString.h"
#include "RunLog.h"
#include "KKStr.h"

#include "Attribute.h"


namespace KKMLL 
{
#if  !defined(_FileDesc_Defined_)
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  typedef  FileDesc const * FileDescConstPtr;
#endif


  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;

  class  FeatureNumList
  {
  public:
    typedef  kkuint16 IntType;
    typedef  std::vector<IntType>  VectorIntType;
    typedef  FeatureNumList*  FeatureNumListPtr;
    typedef  FeatureNumList  const  FeatureNumListConst;
    typedef  FeatureNumListConst*  FeatureNumListConstPtr;
    static const kkuint32  maxIntType;

    FeatureNumList ();


    /** @brief  Copy constructor.  */
    FeatureNumList (FeatureNumListConst&  featureNumList);


    /** @brief  Move constructor.  */
    FeatureNumList (FeatureNumList  &&featureNumList);


    /**  @brief  Constructs an instance with no features selected and expecting a maximum of 'maxFeatureNum'.  */
    FeatureNumList (IntType  _maxFeatureNum);


    /*
     *@brief  Constructs an instance with no features selected and derives the maximum number of features from _fileDesc.
     */
    FeatureNumList (FileDescConstPtr  _fileDesc);

    
    /**
     *@brief Constructs a 'FeatureNumList' instance using the set bits in 'bitString' to indicate which features are selected.
     *@details For each bit position in 'bitString' that is set to '1' the corresponding feature will be selected.  So the bit string '0110111' with consists of
     * bits 0, 1, 2, 5, and 6  set to one will cause the features elected to be set to (0, 1, 2, 4, 6). The length of 'bitString' will indicate the maximum 
     * features.
     *@param[in]  bitString A bit string that indicates which features are selected.  Bits that are set to 1 indicate that
     *            the corresponding feature is selected.
     */
    FeatureNumList (const BitString&  bitString);


    /**
     * @brief Constructs a 'FeatureNumList' instance from a string that contains a list of selected features.
     * @details The list can consist of single features and/or ranges of features. Ranges of features
     *         are specified by using the dash('-') character between two numbers.  The comma(',')
     *  character will be used as a separator.
     * @code
     *  ex's:
     *    "1,2,3,10,20"    Selects [1,2,3,10, 20].
     *    "1,4-7,9-12,13"  Selects [1,4,,5,6,7,9,10,11,12,13]
     * @endcode
     * @see ParseToString
     * @param[in]  _featureListStr  Comma separated string that contains list of selected features; a range of
     *             features can be specified using the dash('-') character.  ex:  The string "1,3,5-7,9" indicates
     *             that features 1,3,5,6,7,9 are selected.
     * @param[out] _valid returns false if '_featureListStr' is not a valid format.
     */
    FeatureNumList (const KKStr&  _featureListStr,
                    bool&         _valid
                   );


    ~FeatureNumList ();   


    // Access Methods.
    const IntType*  FeatureNums    () const  {return featureNums;}
    IntType         MaxFeatureNum  () const  {return maxFeatureNum;}
    IntType         NumOfFeatures  () const  {return numOfFeatures;}
    IntType         NumSelFeatures () const  {return numOfFeatures;}


    /** 
     *@brief Adds 'featureNum' to the list of selected features.
     *@details  If it is already selected nothing happens.  If 'featureNum' exceeds the maxFeatureNum then 
     * 'maxfeatureNum'  will be set to this value.
     */
    void    AddFeature (IntType  featureNum);

    /** @brief Returns true if all features are selected. */
    bool    AllFeaturesSelected (FileDescConstPtr  fileDesc)  const;

    /** @brief Create a FeatureNumList object where all features are selected, except ones that are flagged as Ignore in '__fileDesc'. */
    static  FeatureNumList   AllFeatures (FileDescConstPtr  fileDesc);

    /** @brief Compare with another featureNumList returning -1, 0, and 1 indicating less_than, equal, or greater_than. */
    kkint32 Compare (const FeatureNumList&  _features)  const;

    /** @brief Perform a complement of selected features.  That is if a feature is selected turn it off and if it is not selected then turn it on. */
    FeatureNumList  Complement ()  const;

    /**
     *@brief Allocates a array of kkint32's that is a copy  of FeatureNums.  The caller will own the array and is responsible for deleting it.
     *@returns A dynamically allocated array that will consist of a list of selected features.
     */
    IntType*  CreateFeatureNumArray ()  const;

    bool  IsSubSet (const FeatureNumList&  z);    /**< @brief  Returns true if 'z' is a subset of this instance. */
    
    bool  InList (IntType featureNum)  const;    /**< @brief returns true if '_featureNum' is one of the selected features. */

    kkMemSize  MemoryConsumedEstimated ()  const;


    /**
     * @brief   Will select the features specified in "featureListStr".
     * @details The format is a comma delimited string, where each number represents a feature, ranges can be specified with
     * a dash("-").  "All" will select all features that are not flagged as a 'Ignore'  in the associated FileDesc instance.
     * @code
     *  ex's:   String          Selected Features
     *         "1,2,3,10,20"    [1,2,3,10, 20].
     *         "1,4-7,9-12,23"  [1,4,5,6,7,9,10,11,12,23]
     *         "All"            Selects all features that '_fileDesc' includes accept those that are
     *                          flagged as 'Ignore' in the associated FileDesc instance.
     * @endcode
     */
    void  ParseToString (const KKStr&  _str,
                         bool&         _valid
                        );

    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );

    /**
     * @brief Generates a new FeatureNumList object that will select at random 'numToKeep' features from this instance.
     * @param numToKeep Number of features to select randomly from existing instance.
     * @param rng Random Number Generator used to select features.
     * @return Dynamically allocated instance of a ImageFeaturesList with randomly selected features.
     */
    FeatureNumListPtr  RandomlySelectFeatures (IntType  numToKeep, KKB::RandomNumGenerator& rng)  const;

    /**
    * @brief Generates a new FeatureNumList object that will select at random 'numToKeep' features from this instance.
    * @param numToKeep Number of features to select randomly from existing instance.
    * @return Dynamically allocated instance of a ImageFeaturesList with randomly selected features.
    */
    FeatureNumListPtr  RandomlySelectFeatures (IntType  numToKeep)  const;

    void  SetAllFeatures (FileDescConstPtr  fileDesc);   /**< @brief  Selects all features except those flagged as 'Ignore' in the associated FileDesc. */

    bool  Test (IntType _featureNum)  const;       /**< @brief Indicates whether feature '_featureNum' is selected. */

    void  ToBitString (BitString&  bitStr)  const;

    KKStr  ToHexString ()  const;                        /**< @brief Uses 'maxFeatureNum to determine length of hex string.  */

    KKStr  ToHexString (FileDescConstPtr  fileDesc)  const;   /**< @brief Uses 'fileDesc' to  determine length of hex string.  */


    /** @brief Returns comma delimited list of all features selected; will make use of range specification. */
    KKStr  ToString ()  const;


    KKStr  ToCommaDelStr ()  const {return  ToString ();}


    /** @brief   Turns off all features so that no feature is selected.      */
    void   UnSet ();

    /** @brief Turns off specified feature 'featureNum'; if 'featureNum' is not turned on then nothing happens; same as using 'operator-='.  */
    void   UnSet (IntType  featureNum);

    void   WriteXML (const KKStr& varName,  std::ostream&  o)  const;


    ///<summary>
    /// Returns back selected feature. A FeatureNumList instance consists of a list of selected features. It is logically like an array of selected 
    /// features that is the same length as the number of selected features.
    ///</summary>
    ///<param name="idx"> The position in this instance that you want to return. </param>
    ///<returns> Selected feature at position &quot;_idx&quot;.</returns>
    IntType  operator[] (kkint32  idx)  const;

    FeatureNumList&  operator=  (const FeatureNumList&   _features);
    FeatureNumList&  operator=  (FeatureNumList&&        _features);
    FeatureNumList&  operator=  (const FeatureNumListPtr _features);
    FeatureNumList   operator+  (const FeatureNumList&   rightSide)   const;  /**< @brief Returns new FeatureNumList that is a union of this instance and 'rightSide'.  */
    FeatureNumList   operator+  (IntType                 rightSide)   const;  /**< @brief Returns new FeatureNumList that is a union of this instance and 'rightSide'.  */
    FeatureNumList&  operator+= (const FeatureNumList&   rightSide);          /**< @brief Returns this FeatureNumList that is a union of this instance and 'rightSide'. */
    FeatureNumList&  operator+= (IntType                 featureNum);         /**< @brief Returns this FeatureNumList that is a union of this instance and 'rightSide'. */
    FeatureNumList   operator-  (const FeatureNumList&   rightSide)   const;  /**< Removes features that are selected in 'rightSide' from this instance and returns the result. */
    FeatureNumList   operator-  (IntType                 rightSide)   const;  /**< Returns this instance with the feature specified by 'rightSide'  removed.                    */
    FeatureNumList&  operator-= (IntType                 rightSide);          /**< Remove the feature specified by 'rightSide' from this instance.                              */
    FeatureNumList   operator*  (const FeatureNumList&   rightSide)   const;  /**<*@brief  Returns new instance that is the intersection of features.                      */
    bool             operator== (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the two FeatureNumLiost instances have the same features selected. */
    bool             operator>  (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the Left FeatureNumList instances is greater than the right one.   */
    bool             operator<  (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the Left FeatureNumList instances is less than the right one.      */

  private:
    void   AllocateArraySize (IntType size);   /**< @brief  Make sure that FeatureNums is allocated to at least this size. */

    static  VectorIntType*  StrToUInt16Vetor (const KKStr&  s);

    IntType*  featureNums;              /**< @brief The feature numbers in this array are always kept in ascending order.  
                                         * @details There will be 'numOfFeatures' in this array.  'featureNumsAllocatedSize' 
                                         * indicates the size allocated, if more space is needed you need to call 
                                         * 'AllocateArraySize' to increase it.
                                         */
    IntType  featureNumsAllocatedSize;
    IntType  maxFeatureNum;
    IntType  numOfFeatures;
  };  /* FeatureNumList */


  typedef  FeatureNumList*  FeatureNumListPtr;
  typedef  FeatureNumList  const  FeatureNumListConst;
  typedef  FeatureNumListConst*  FeatureNumListConstPtr;

  #define  _FeatureNumList_Defined_


  std::ostream& operator<< (std::ostream& os,  const FeatureNumList& features);

  std::ostream& operator<< (std::ostream& os,  const FeatureNumListPtr&  features);

  typedef  XmlElementTemplate<FeatureNumList>  XmlElementFeatureNumList;
  typedef  XmlElementFeatureNumList*  XmlElementFeatureNumListPtr;
} /* namespace KKMLL */

#endif

