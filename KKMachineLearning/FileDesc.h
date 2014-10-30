#ifndef  _FILEDESC_
#define  _FILEDESC_

/**
 @file  FileDesc.h
 @author  Kurt Kramer
 @details
 @code
 **************************************************************************************
 **                                                                                  *
 **  Kurt Kramer                                                                     *
 **                                                                                  *
 **  Describes the different fields of a dataset. There will be one instance of this *
 **  class for each tyope of Dataset that you have in an application.  From this     *
 **  object you can get information such as number of attributes, Attribute types,   *
 **  weather they are nomainal, continuous.  If nomainal what are the accepted       *
 **  values.                                                                         *
 **                                                                                  *
 **                                                                                  *
 **  Never delete an instance of a FileDesc object.                                  *
 **                                                                                  *
 **  Only one FileDesc object can exist for any Dataset.  Example the Forest Cover   *
 **  dataset.  You can split the data into many files and manage them seperately but *
 **  you will only have one instance of a FileDesc object tha they will all refer    *
 **  to.  See "GetExistingFileDesc" method below.  You would initially create an     *
 **  instance of FileDesc abd then use "GetExistingFileDesc" to make sure that it is *
 **  unique. This typically happens in the FeatureFileIO derived classes.            *
 **                                                                                  *
 *************************************************************************************
 @endcode
 */

#ifdef  WIN32
#include  "..\\KKBase\\GoalKeeper.h"
#include  "..\\KKBase\\RunLog.h"
#include  "..\\KKBase\\KKStr.h"
#else
#include  "../KKBase/GoalKeeper.h"
#include  "../KKBase/RunLog.h"
#include  "../KKBase/KKStr.h"
#endif

#include  "Attribute.h"
#include  "MLClass.h"




namespace KKMachineLearning 
{
  class  FileDescList;
  typedef  FileDescList*  FileDescListPtr;

  #if  !defined(_FeatureFileIO_Defined_)
  class  FeatureFileIO;
  typedef  FeatureFileIO* FeatureFileIOPtr;
  #endif


  /**
    *@class  FileDesc
    *@brief  Provides a detailed description of the attributes of a dataset.
    *@author Kurt Kramer
    *@details
    *@code
    * ***********************************************************************************
    * * Describes the different fields of a dataset. There will be one instance of this *
    * * class for each type of Dataset that you have in an application.  From this      *
    * * object you can get information such as number of attributes, Attribute types,   *
    * * weather they are nominal, continuous.  If nominal what are the accepted         *
    * * values.                                                                         *
    * *                                                                                 *
    * *  Never delete an instance of a FileDesc object.                                 *
    * *                                                                                 *
    * * Only one FileDesc object can exist for any Dataset.  Example the Forest Cover   *
    * * dataset.  You can split the data into many files and manage them separately but *
    * * you will only have one instance of a FileDesc object that they will all refer   *
    * * to.  See "GetExistingFileDesc" method below.  You would initially create an     *
    * * instance of FileDesc and then use "GetExistingFileDesc" to make sure that it is *
    * * unique. This typically happens in the FeatureFileIO derived classes.            *
    *************************************************************************************
    *@endcode
    *@see GetExistingFileDesc
    *@see FeatureVectorList
    *@see FeatureFileIo
    */
  class  FileDesc
  {
  public:
    typedef  FileDesc*  FileDescPtr;

    typedef  FileDesc*  const   FileDescConstPtr;

    /**
     @brief  Clean up function,  call just vefore exiting the application.
     @details
     @code
     * *******************************************************************************
     * * Before you terminate your application you should call this method.  It will *
     * * clean up the FileDesc objects that were created during the runtime of your  *
     * * application.                                                                *
     *********************************************************************************
     @endcode
     */
    static  void  FinalCleanUp ();

    static  bool  FinaleCleanUpRanAlready ()  {return finaleCleanUpRanAlready;}

    /**
     @brief  Creates a simple FileDesc that consists of continuous data only.
     @details
     @code
     * ***************************************************************************
     * * Creates a file description that will consist of continuous fields only. *
     * * The vector '_fieldNames' will provide the list of field names.          *
     * ***************************************************************************
     @endcode
     @param[in]  _log   Logging file to use.
     @param[in]  _fieldNames Name of fields;  one entry for each field.
     */
    static
      FileDescPtr   NewContinuousDataOnly (RunLog&       _log,        
                                           VectorKKStr&  _fieldNames
                                          );

      FileDesc ();

  protected:
    ~FileDesc ();

  public:
    friend class KKQueue<FileDesc>;

  public:
    // Access Methods
    const KKMachineLearning::AttributeList&   Attributes          ()  const  {return attributes;};
    const AttributeTypeVector&                AttributeVector     ()  const  {return attributeVector;};
    const VectorInt32&                        CardinalityVector   ()  const  {return cardinalityVector;}
    const MLClassList&                        Classes             ()  const  {return classes;}
    const KKStr&                              ClassNameAttribute  ()  const  {return classNameAttribute;}   /**< ClassNameAttribute added to support dstWeb  data files. */
    const KKStr&                              FileName            ()  const  {return  fileName;}
    kkint32                                   SparseMinFeatureNum ()  const  {return sparseMinFeatureNum;}
    kkint16                                   Version             ()  const  {return version;}

    void   SparseMinFeatureNum (kkint32  _sparseMinFeatureNum)  {sparseMinFeatureNum = _sparseMinFeatureNum;}
    void   Version             (kkint16  _version)              {version             = _version;}



    void                      AddAAttribute (const KKB::KKStr&                 _name,
                                             KKMachineLearning::AttributeType  _type,
                                             bool&                             alreadyExists
                                            );
                            
    void                      AddAAttribute (const KKMachineLearning::Attribute&  attribute);

    void                      AddClasses (MLClassList&  classesToAdd);
                            
    bool                      AllFieldsAreNumeric ()  const;


    kkint32                   Cardinality (kkint32  fieldNum,
                                           RunLog&  log
                                          )  const;

    const 
      KKMachineLearning::AttributePtr*      CreateAAttributeTable ()  const;  /**< Caller will be responsable for deleteing  */

    KKMachineLearning::AttributeTypeVector  CreateAttributeTypeTable ()  const;
    
    VectorInt32               CreateCardinalityTable ()  const;

    const KKStr&              FieldName (kkint32  fieldNum)  const;

    const KKMachineLearning::Attribute&     GetAAttribute (kkint32 fieldNum) const;

    const  
    KKStr&                    GetNominalValue (kkint32  fieldNum, 
                                               kkint32  code
                                              ) const;

    MLClassPtr                GetMLClassPtr       (const KKStr&  className);

    kkint32                   GetFieldNumFromAttributeName (const KKStr&  attributeName)  const;

    //RunLog&                   Log () const {return log;}

    kkint32                   LookUpNominalCode (kkint32       fieldNum, 
                                                 const KKStr&  nominalValue
                                                )  const;

    MLClassPtr                LookUpImageClassByName (const KKStr&  className);

    MLClassPtr                LookUpUnKnownImageClass ();
    
    kkint32                   MemoryConsumedEstimated ()  const;

    kkuint32                  NumOfFields () const  {return attributes.size ();}

    bool                      SameExceptForSymbolicData (const FileDesc&  otherFd,
                                                         RunLog&          log
                                                        )  const;

    KKMachineLearning::AttributeType        
                              Type (kkint32 fieldNum)  const;

    KKStr                     TypeStr (kkint32 fieldNum)  const;


    /**
     @brief Returns a pointer to an existing instance of 'fileDesc' if it exists, otherwise will use one being passed in.
     @details
     @code
     * ******************************* GetExistingFileDesc ******************************
     * *  Will look for a existing FileDesc that is the same as the 'fileDesc' being    *
     * *  passed in.                                                                    *
     * *  if  one is found   them                                                       *
     * *    fileDesc is deleted                                                         *
     * *    exiting one will be returned.                                               *
     * *  else                                                                          *
     * *    fileDesc will be added to existinList (exisitingDescriptions) and returned. *
     * *    and returned.                                                               *
     * **********************************************************************************
     @endcode
     @param[in] fileDesc Pointer to a FileDesc object that you want to look and see if one that is identical already exists.
     @return pointer to the 'FileDesc' instance that the caller should be using.
     */
    static
      FileDescPtr     GetExistingFileDesc (FileDescPtr  fileDesc);

    const
      KKMachineLearning::AttributePtr      LookUpByName (const KKStr&  attributeName)  const;  


    void  DisplayAttributeMappings ();

    /** 
     *@brief Returns true if file description on the right size is identical.
     *@details Both FileDesc instances must have the same number of fields, the fields
     *  must have the same names(NOT case sensitive), and the fields must have
     *  matching types(ex numerical, ordinal, etc).
     */
    bool  operator== (const FileDesc&  rightSize)  const;


    /** 
     *@brief Returns true if file description on the right size is NOT identical.
     *@details If both FileDesc instances have different number of fields, or any one
     *  of the fields has a different name(NOT case sensitive), or one of the fields
     *  is of a different type.
     */
    bool  operator!= (const FileDesc&  rightSize)  const;


    /**
     * @brief  Merges the Symbolic fields of two different 'FileDesc' instances producing a new instance of 'FileDesc'.
     * @details This method will only work if both instances have the same number of fields, their names must be
     *  the same(NOT case sensitive), and each field in both instances must be the same type.  If all these conditions
     *  are not 'true' will return NULL.  The fields that are of 'SymbolicAttribute' will have their values merged
     *  together.
     *@see MLL:Attribute
     */
    static FileDescPtr  MergeSymbolicFields (const FileDesc&  left,
                                             const FileDesc&  right,
                                             RunLog&          log
                                            );
  public:
    void  AddANominalValue (kkint32       fieldNum,
                            const KKStr&  nominalValue,
                            bool&         alreadyExist,
                            RunLog&       log
                           );


    void  AddANominalValue (const KKStr&  nominalValue,
                            bool&         alreadyExist,
                            RunLog&       log
                           );


    void  AddANominalValue (const KKStr&  attributeName,
                            const KKStr&  nominalValue,
                            bool&         alreadyExist,
                            RunLog&       log
                           );

  private:
    static void  CreateBlocker ();

    kkint32 NumOfAttributes ()  {return attributes.QueueSize ();}


    void  ValidateFieldNum (kkint32      fieldNum,
                            const char*  funcName
                           )  const;

    KKMachineLearning::AttributeList    attributes;
    AttributeTypeVector                 attributeVector;
    VectorInt32                         cardinalityVector;
    MLClassList                         classes;
    KKStr                               classNameAttribute;   /**< Added to support DstWeb files.  The name of the attribute that specifies the className */
    KKMachineLearning::AttributePtr     curAttribute;
    KKStr                               fileName;
    //RunLog&                           log;
    kkint32                             sparseMinFeatureNum;  /**< Used specifically for sparse files.  */
    kkint16                             version;

    static
      KKB::GoalKeeperPtr  blocker;

    static
      FileDescListPtr     exisitingDescriptions;  /**< Will keep a list of all FileDesc s instantiated. */

    static
      bool                finaleCleanUpRanAlready;

  };  /* FileDesc */


  typedef  FileDesc::FileDescPtr        FileDescPtr;  
  typedef  FileDesc::FileDescConstPtr   FileDescConstPtr;


  #define  _FileDesc_Defined_

}  /* namespace KKMachineLearning  */


#endif
