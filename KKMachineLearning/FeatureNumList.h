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
 * to be selected except those that are flagged as  'IgnoreAttribute' in the 
 * associated FileDesc instance. The list should be in ascending order.
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
#endif


  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;

  class  FeatureNumList
  {
  public:
    typedef  FeatureNumList*  FeatureNumListPtr;
    typedef  FeatureNumList  const  FeatureNumListConst;
    typedef  FeatureNumListConst*  FeatureNumListConstPtr;

    /** @brief  Copy constructor.  */
    FeatureNumList (FeatureNumListConst&  featureNumList);

    /*
     *@brief  Constructs an instance with no features selected and expecting a maximum of 'maxFeatureNum'.
     */
    FeatureNumList (kkuint32  _maxFeatureNum);


    /*
     *@brief  Constructs an instance with no features selected and derives the maximum number of features from _fileDesc.
     */
    FeatureNumList (FileDescPtr  _fileDesc);

    
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
     * @see ExtractFeatureNumsFromStr
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
    const kkuint16*  FeatureNums    () const  {return featureNums;}
    kkint32          MaxFeatureNum  () const  {return maxFeatureNum;}
    kkint32          NumOfFeatures  () const  {return numOfFeatures;}
    kkint32          NumSelFeatures () const  {return numOfFeatures;}


    /** 
     *@brief Adds 'featureNum' to the list of selected features.
     *@details  If it is already selected nothing happens.  If 'featureNum' exceeds the maxFeatureNum then 
     * 'maxfeatureNum'  will be set to this value.
     */
    void    AddFeature (kkuint16  featureNum);

    /** @brief Returns true if all features are selected. */
    bool    AllFeaturesSelected (FileDescPtr  fileDesc)  const;

    /** @brief Create a FeatureNumList object where all features are selected, except ones that are flagged as IgnoreAttribute in '__fileDesc'. */
    static  FeatureNumList   AllFeatures (FileDescPtr  fileDesc);

    /** @brief Compare with another featureNumList returning -1, 0, and 1 indicating less_than, equal, or greater_than. */
    kkint32 Compare (const FeatureNumList&  _features)  const;

    /** @brief Perform a complement of selected features.  That is if a feature is selected turn it off and if it is not selected then turn it on. */
    FeatureNumList  Complement ()  const;

    /**
     *@brief Allocates a array of kkint32's that is a copy  of FeatureNums.  The caller will own the array and is responsible for deleting it.
     *@returns A dynamically allocated array that will consist of a list of selected features.
     */
    kkuint16*  CreateFeatureNumArray ()  const;


    /**
     * @brief   Will select the features specified in "featureListStr".
     * @details The format is a comma delimited string, where each number represents a feature, ranges can be specified with
     * a dash("-").  "All" will select all features that are not flagged as a 'IgnoreAttribute'  in the associated FileDesc instance.
     * @code
     *  ex's:   String          Selected Features
     *         "1,2,3,10,20"    [1,2,3,10, 20].
     *         "1,4-7,9-12,23"  [1,4,5,6,7,9,10,11,12,23]
     *         "All"            Selects all features that '_fileDesc' includes accept those that are
     *                          flagged as 'IgnoreAttribute' in the associated FileDesc instance.
     * @endcode
     */
    static
    FeatureNumListPtr  ExtractFeatureNumsFromStr (const KKStr&  featureListStr);

    bool  IsSubSet (const FeatureNumList&  z);    /**< @brief  Returns true if 'z' is a subset of this instance. */
    
    bool  InList (kkuint16 featureNum)  const;    /**< @brief returns true if '_featureNum' is one of the selected features. */

    void  Load (const KKStr&  _fileName,
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

    void  Save (const KKStr&  fileName);
   
    void  SaveXML (std::ostream&  o);

    void  SetAllFeatures (FileDescPtr  fileDesc);   /**< @brief  Selects all features except those flagged as 'IgnoreAttribute' in the associated FileDesc. */

    bool  Test (kkuint16 _featureNum)  const;       /**< @brief Indicates whether feature '_featureNum' is selected. */

    void  ToBitString (BitString&  bitStr)  const;

    KKStr  ToHexString ()  const;                        /**< @brief Uses 'maxFeatureNum to determine length of hex string.  */

    KKStr  ToHexString (FileDescPtr  fileDesc)  const;   /**< @brief Uses 'fileDesc' to  determine length of hex string.  */


    /** @brief Returns comma delimited list of all features selected; will make use of range specification. */
    KKStr  ToString ()  const;


    KKStr  ToCommaDelStr ()  const {return  ToString ();}


    /** @brief   Turns off all features so that no feature is selected.      */
    void   UnSet ();

    /** @brief Turns off specified feature 'featureNum'; if 'featureNum' is not turned on then nothing happens; same as using 'operator-='.  */
    void   UnSet (kkuint16  featureNum);

    /**
     *@brief Returns back the selected feature.
     *@details  A FeatureNumList instance consists of a list of selected features. It is logically like an 
     * array of selected features that is the same length as the number of selected features.
     *@code
     * Example code that scans the FeatureNumList object  'goodFeatures'
     *
     * void  PrintSelectedFeatures (const FeatureNumList&  goodFeatures)
     * {
     *   cout << "Selected Features: ";
     *   for  (kkint32 x = 0;  x < goodFeatures.NumOfFeatures ();
     *   {
     *     if  (x > 0)  cout << ",";
     *     cout << goodFeatures[x];
     *   }
     *   cout << endl;
     * }
     *@endcode
     *@param[in]  _idx  The position in this instance that you want to return.
     *@return  Selected feature at position '_idx'.
     */
    kkuint16  operator[] (kkint32  idx)  const;

    FeatureNumList&  operator=  (const FeatureNumList&   _features);
    FeatureNumList&  operator=  (const FeatureNumListPtr _features);
    FeatureNumList   operator+  (const FeatureNumList&   rightSide)   const;  /**< @brief Returns new FeatureNumList that is a union of this instance and 'rightSide'.  */
    FeatureNumList   operator+  (kkuint16                rightSide)   const;  /**< @brief Returns new FeatureNumList that is a union of this instance and 'rightSide'.  */
    FeatureNumList&  operator+= (const FeatureNumList&   rightSide);          /**< @brief Returns this FeatureNumList that is a union of this instance and 'rightSide'. */
    FeatureNumList&  operator+= (kkuint16                featureNum);         /**< @brief Returns this FeatureNumList that is a union of this instance and 'rightSide'. */
    FeatureNumList   operator-  (const FeatureNumList&   rightSide)   const;  /**< Removes features that are selected in 'rightSide' from this instance and returns the result. */
    FeatureNumList   operator-  (kkuint16                rightSide)   const;  /**< Returns this instance with the feature specified by 'rightSide'  removed.                    */
    FeatureNumList&  operator-= (kkuint16                rightSide);          /**< Remove the feature specified by 'rightSide' from this instance.                              */
    FeatureNumList   operator*  (const FeatureNumList&   rightSide)   const;  /**<*@brief  Returns new instance that is the intersection of features.                      */
    bool             operator== (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the two FeatureNumLiost instances have the same features selected. */
    bool             operator>  (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the Left FeatureNumList instances is greater than the right one.   */
    bool             operator<  (const FeatureNumList&   _features)   const;  /**< @brief  Indicates if the Left FeatureNumList instances is less than the right one.      */

  private:
    /*
     * @brief  Constructs an instance with no features selected and no associated 'FileDesc' instance.
     *  This is a private constructor and is used for internal use of 'FeatureNumList only.
     */
    FeatureNumList ();

    void   AllocateArraySize (kkuint16 size);   /**< @brief  Make sure that FeatureNums is allocated to at least this size. */

    static  VectorUint16*  StrToUInt16Vetor (const KKStr&  s);

    kkuint16*  featureNums;              /**< @brief The feature numbers in this array are always kept in ascending order.  
                                          * @details There will be 'numOfFeatures' in this array.  'featureNumsAllocatedSize' 
                                          * indicates the size allocated, if more space is needed you need to call 
                                          * 'AllocateArraySize' to increase it.
                                          */
    kkint32   featureNumsAllocatedSize;
    kkint32   maxFeatureNum;
    kkint32   numOfFeatures;
  };  /* FeatureNumList */


  typedef  FeatureNumList*  FeatureNumListPtr;
  typedef  FeatureNumList  const  FeatureNumListConst;
  typedef  FeatureNumListConst*  FeatureNumListConstPtr;

  #define  _FeatureNumList_Defined_


  std::ostream& operator<< (      std::ostream&     os, 
                            const FeatureNumList&   features
                           );


  std::ostream& operator<< (      std::ostream&       os, 
                            const FeatureNumListPtr&  features
                           );

  //extern
  //const  char*  FeatureDecriptions[];


  
class  XmlElementFeatureNumList:  public  XmlElement
  {
  public:
    XmlElementFeatureNumList (XmlTagPtr   tag,
                        XmlStream&  s,
                        RunLog&     log
                       );
                
    virtual  ~XmlElementFeatureNumList ();

    FeatureNumListPtr  Value ()  const;

    FeatureNumListPtr  TakeOwnership ();

    static
    void  WriteXML (const FeatureNumList&  fnl,
                    const KKStr&           varName,
                    ostream&               o
                   );
  private:
    FeatureNumListPtr  value;
  };
  typedef  XmlElementFeatureNumList*  XmlElementFeatureNumListPtr;











} /* namespace KKMLL */

#endif

