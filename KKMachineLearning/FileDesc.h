#if  !defined(_FILEDESC_)
#define  _FILEDESC_

/**
 @file  FileDesc.h
 @author  Kurt Kramer
 @details
 @code
 *************************************************************************************
 **  Describes the different fields of a dataset. There will be one instance of this *
 **  class for each type of Dataset that you have in an application.  From this      *
 **  object you can get information such as number of attributes, Attribute types,   *
 **  weather they are nominal, continuous.  If nominal what are the accepted         *
 **  values.                                                                         *
 **                                                                                  *
 **  Never delete an instance of a FileDesc object.                                  *
 **                                                                                  *
 **  Only one FileDesc object can exist for any Dataset.  Example the Forest Cover   *
 **  dataset.  You can split the data into many files and manage them separately but *
 **  you will only have one instance of a FileDesc object that they will all refer   *
 **  to.  See "GetExistingFileDesc" method below.  You would initially create an     *
 **  instance of FileDesc and then use "GetExistingFileDesc" to make sure that it is *
 **  unique. This typically happens in the FeatureFileIO derived classes.            *
 **                                                                                  *
 *************************************************************************************
 @endcode
 */

#include "GoalKeeper.h"
#include "KKStr.h"
#include "RunLog.h"
#include "XmlStream.h"

#include "Attribute.h"
#include "MLClass.h"



namespace KKMLL 
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
    * Describes the different fields of a dataset. There will be one instance of this
    * class for each type of Dataset that you have in an application.  From this
    * object you can get information such as number of attributes, Attribute types,
    * weather they are nominal, continuous.  If nominal what are the accepted values.                                                                         *
    *
    * Never delete an instance of a FileDesc object.
    *
    * Only one FileDesc object can exist for any Dataset. Example the Forest-Cover
    * dataset. You can split the data into many files and manage them separately but
    * you will only have one instance of a FileDesc object that they will all refer
    * to. See "GetExistingFileDesc" method below. You would initially create an
    * instance of FileDesc and then use "GetExistingFileDesc" to make sure that it is
    * unique. This typically happens in the FeatureFileIO derived classes.
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
     *@brief  Clean up function, call just before exiting the application.
     *@details
     * Before you terminate your application you should call this method.  It will
     * clean up the FileDesc objects that were created during the runtime of your
     * application.
     */
    static  void  FinalCleanUp ();

    static  bool  FinalCleanUpRanAlready ()  {return finalCleanUpRanAlready;}

    /**
     @brief  Creates a simple FileDesc that consists of continuous data only.
     @details
     * Creates a file description that will consist of continuous fields only.
     * The vector '_fieldNames' will provide the list of field names.
     @param[in]  _log   Logging file to use.
     @param[in]  _fieldNames Name of fields;  one entry for each field.
     */
    static
      FileDescPtr   NewContinuousDataOnly (VectorKKStr&  _fieldNames);

      FileDesc ();

  protected:
    ~FileDesc ();

  public:
    friend class KKQueue<FileDesc>;

    kkint32  MemoryConsumedEstimated ()  const;


  public:
    // Access Methods
    const KKMLL::AttributeList&  Attributes          ()  const  {return attributes;};
    const AttributeTypeVector&   AttributeVector     ()  const  {return attributeVector;};
    const VectorInt32&           CardinalityVector   ()  const  {return cardinalityVector;}
    const MLClassList&           Classes             ()  const  {return classes;}
    const KKStr&                 ClassNameAttribute  ()  const  {return classNameAttribute;}   /**< ClassNameAttribute added to support dstWeb  data files. */
    const KKStr&                 FileName            ()  const  {return fileName;}
    kkint32                      SparseMinFeatureNum ()  const  {return sparseMinFeatureNum;}
    kkint16                      Version             ()  const  {return version;}

    void   ClassNameAttribute  (const KKStr&  _classNameAttribute)   {classNameAttribute  = _classNameAttribute;}
    void   FileName            (const KKStr&  _fileName)             {fileName            = _fileName;}
    void   SparseMinFeatureNum (kkint32       _sparseMinFeatureNum)  {sparseMinFeatureNum = _sparseMinFeatureNum;}
    void   Version             (kkint16       _version)              {version             = _version;}


    void  AddAAttribute (const KKB::KKStr&     _name,
                         KKMLL::AttributeType  _type,
                         bool&                 alreadyExists
                        );
                            
    void  AddAAttribute (const KKMLL::Attribute&  attribute);

    void  AddClasses (const MLClassList&  classesToAdd);
                            
    bool  AllFieldsAreNumeric ()  const;

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

    void  AddAttributes (const KKMLL::AttributeList&  attributes);


    kkint32  Cardinality (kkint32  fieldNum)  const;

    const 
      KKMLL::AttributePtr*      CreateAAttributeTable ()  const;  /**< Caller will be responsible for deleting  */

    KKMLL::AttributeTypeVector  CreateAttributeTypeTable ()  const;
    
    VectorInt32                 CreateCardinalityTable ()  const;

    void                        DisplayAttributeMappings ();

    const KKStr&                FieldName (kkint32  fieldNum)  const;

    const KKMLL::Attribute&     GetAAttribute (kkint32 fieldNum) const;

    const  
    KKStr&                      GetNominalValue (kkint32  fieldNum, 
                                                 kkint32  code
                                                ) const;

    MLClassPtr                  GetMLClassPtr (const KKStr&  className);

    kkint32                     GetFieldNumFromAttributeName (const KKStr&  attributeName)  const;

    const
    KKMLL::AttributePtr         LookUpByName (const KKStr&  attributeName)  const;  

    kkint32                     LookUpNominalCode (kkint32       fieldNum, 
                                                   const KKStr&  nominalValue
                                                  )  const;

    MLClassPtr                  LookUpMLClassByName (const KKStr&  className);

    MLClassPtr                  LookUpUnKnownMLClass ();
    
    kkuint32                    NumOfFields () const  {return (kkuint32)attributes.size ();}

    void                        ReadXML (XmlStream&      s,
                                         XmlTagConstPtr  tag,
                                         RunLog&         log
                                        );


    bool                        SameExceptForSymbolicData (const FileDesc&  otherFd,
                                                           RunLog&          log
                                                          )  const;


    KKMLL::AttributeType        Type (kkint32 fieldNum)  const;

    KKStr                       TypeStr (kkint32 fieldNum)  const;


    /**
     *@brief Returns a pointer to an existing instance of 'fileDesc' if it exists, otherwise will use one being passed in.
     *@details
     * > First looks to see if a the same FileDesc is already in the existing list 
     *   in that case will return back the same pointer.
     *
     *@code
     * > Second Will look for a existing FileDesc that is the same as the 'fileDesc'
     *   being passed in.
     *   if  one is found   them
     *     fileDesc is deleted
     *     exiting one will be returned.
     *   else
     *     fileDesc will be added to existinList (exisitingDescriptions) and returned.
     *     and returned.
     *@endcode
     @param[in] fileDesc Pointer to a FileDesc object that you want to look and see if one that is identical already exists.
     @return pointer to the 'FileDesc' instance that the caller should be using.
     */
    static  FileDescPtr     GetExistingFileDesc (FileDescPtr  fileDesc);

    /**
     * @brief  Merges the Symbolic fields of two different 'FileDesc' instances producing a new instance of 'FileDesc'.
     * @details This method will only work if both instances have the same number of fields, their names must be
     *  the same(NOT case sensitive), and each field in both instances must be the same type.  If all these conditions
     *  are not 'true' will return NULL.  The fields that are of 'SymbolicAttribute' will have their values merged
     *  together.
     *@see KKMLL:Attribute
     */
    static FileDescPtr  MergeSymbolicFields (const FileDesc&  left,
                                             const FileDesc&  right,
                                             RunLog&          log
                                            );


    void  WriteXML (const KKStr&  varName,
                    ostream&      o
                   )  const;


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


  private:
    static void  CreateBlocker ();

    kkint32 NumOfAttributes ()  {return attributes.QueueSize ();}


    void  ValidateFieldNum (kkint32      fieldNum,
                            const char*  funcName
                           )  const;

    KKMLL::AttributeList    attributes;
    AttributeTypeVector     attributeVector;
    VectorInt32             cardinalityVector;
    MLClassList             classes;
    KKStr                   classNameAttribute;   /**< Added to support DstWeb files; the name of the attribute that specifies the className */
    KKMLL::AttributePtr     curAttribute;
    KKStr                   fileName;
    kkint32                 sparseMinFeatureNum;  /**< Used specifically for sparse files.  */
    kkint16                 version;

    static
      KKB::GoalKeeperPtr  blocker;

    static
      FileDescListPtr     exisitingDescriptions;  /**< Will keep a list of all FileDesc s instantiated. */

    static
      bool                finalCleanUpRanAlready;

  };  /* FileDesc */


  typedef  FileDesc::FileDescPtr        FileDescPtr;  
  typedef  FileDesc::FileDescConstPtr   FileDescConstPtr;

  #define  _FileDesc_Defined_




class  XmlElementFileDesc:  public  XmlElement
  {
  public:
    XmlElementFileDesc (XmlTagPtr   tag,
                        XmlStream&  s,
                        RunLog&     log
                       );
                
    virtual  ~XmlElementFileDesc ();

    FileDescPtr  Value ()  const;

    FileDescPtr  TakeOwnership ();

    static
    void  WriteXML (const FileDesc&  fileDesc,
                    const KKStr&     varName,
                    ostream&         o
                   );
  private:
    FileDescPtr  value;
  };
  typedef  XmlElementFileDesc*  XmlElementFileDescPtr;





}  /* namespace KKMLL  */


#endif

