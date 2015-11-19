#if  !defined(_MLCLASS_)
#define  _MLCLASS_

/**
 *@class  KKMLL::MLClass
 *@brief  Represents a "Class" in the Machine Learning Sense.
 *@author  Kurt Kramer
 *@details
 *  Each instance of this class represents a single Class as used in
 *  Machine Learning sense.  Each instance of 'FeatureVector' class
 *  will point to an instance of this class.  There can only be one
 *  instance of each Class in memory.  Specifically the 'name' field
 *  will be unique.  This is enforced by making the constructor and
 *  destructor private.  The only way to create a new instance of a
 *  'MLClass' object is to call one of the static methods of
 *  'CreateNewMLClass'.  These methods will look for a instance of
 *  'MLClass' that already exists.  If one does not they will then
 *  create a new one.
 *
 *  Please refer to MLClassList at bottom of file. That class is
 *  the object you will most be using when dealing with Images.
 *  classes.
 *
 *  UmiClass  and PicesClass
 *  There is a special relationship between this class  and a class
 *  called 'UmiClass' and 'PicesClass in the 'UnManagedInterface'
 *  and PicesInterface libraries, ".net' Managed libraries'. These
 *  managed c++ versions of this class.  There is a one-to-one
 *  correspondence between the two classes.  When ever a instance
 *  of 'UmiClass' or 'PicesClass' get created they  will automatic-
 *  ally call the static method 'MLClass::CreateNewMLClass' to get
 *  the unmanaged version of the class.
 *@see KKMLL::MLClassList, KKMLL::FeatureVector, UnManagedInterface::UmiClass, MLL::PicesClass
 */


#include "GoalKeeper.h"
#include "KKStr.h"
#include "KKQueue.h"
#include "RunLog.h"
#include "XmlStream.h"


namespace KKMLL
{
  class  MLClass;
  typedef  MLClass*  MLClassPtr;

  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;

  class  MLClass 
  {
  private:
    static  MLClassListPtr                           existingMLClasses;
    static  std::map<MLClassListPtr,MLClassListPtr>  existingClassLists;

    static  void  AddImageClassList    (MLClassListPtr  list);
    static  void  DeleteImageClassList (MLClassListPtr  list);


  public:
    static  MLClassListPtr  GlobalClassList ();

    /**
     *@brief  Static method used to create a new instance of a MLClass object.
     *@details
     * Used to get pointer to existing MLClass object that already exists
     * in 'existingMLClasses'. If one does not exist then a new MLClass
     * object with this name will be created.
     *
     * This is the only method that can actually create a MLClass instance.
     * The idea is that there is only one MLClass object ever created for
     * each class.  All MLClassList container objects will point to
     *@param[in]  _name   Name of class to be created.
     *@return  Pointer to an instance of 'MLClass' that will have the name '_name'.
     */
    static  MLClassPtr  CreateNewMLClass (const KKStr&  _name,
                                          kkint32       _classId = -1
                                         );

    static  MLClassPtr  GetUnKnownClassStatic ();

    static  MLClassPtr  GetByClassId (kkint32  _classId);

    /**
     *@brief Changes the name of an existing class verifying that a duplicate does not get created.
     *@details Since the class name can not be duplicated and there is a nameIndex structure maintained by each
     * 'MLClassList' instances we need to make sure the name is not already in use by another instance of 
     * 'mlClass'.  This is done by first trying to update the 'nameIndex' in 'existingMLClasses' (list
     * of all 'inageClass' instances in existence); if this is successful will then change the name in 'mlClass'
     * and update all existing 'MLClassList' instances 'nameIndex' structures.
     *
     *@param[in,out] mlClass Class having its name changed; upon entry should contain its original name; if
     *  no other class already has its name it will be updated to the value in 'newName'.
     *@param[in] newName  The new name that 'mlClass' is to receive.
     *@param[out] changeSuccessful Returns 'true' if name was changed; if set to false then the 'name' field in 'mlClass'
     *  will not be changed.
     */
    static  void  ChangeNameOfClass (MLClassPtr    mlClass,
                                     const KKStr&  newName,
                                     bool&         changeSuccessful
                                    );

    static  void  ResetAllParentsToAllClasses ();


    /** @brief Call this as the last thing the application does, will delete all existing instances of 'MLClass'. */
    static  void  FinalCleanUp ();


  private:
    static void  CreateBlocker ();

    static GoalKeeperPtr  blocker;
    static bool  needToRunFinalCleanUp;

    friend class KKQueue<MLClass>;
    friend class MLClassList;

    MLClass (const KKStr&  _name);

    MLClass (const MLClass&  mlClass);

    ~MLClass ();

    void  Name (const KKStr&  _name);


  public:
    static KKStr    GetClassNameFromDirName (const KKStr&  subDir);

    static MLClassListPtr  BuildListOfDecendents (MLClassPtr  parent);

   
    kkint32         ClassId ()  const  {return classId;}  /**< From MySQL table  Classes, '-1' indicates that not loaded from mysql table. */
    void            ClassId (kkint32 _classId)  {classId = _classId;}

    float           CountFactor () const  {return countFactor;}
    void            CountFactor (float _countFactor)  {countFactor = _countFactor;}

    const KKStr&    Description ()  const {return description;}
    void            Description (const KKStr&  _description)  {description = _description;}

    bool            IsAnAncestor (MLClassPtr       c)  const;    /**< Returns true if 'c' is an ancestor */

    MLClassPtr      MLClassForGivenHierarchialLevel (KKB::kkuint16 level)  const;

    bool            Mandatory () const {return mandatory;}
    void            Mandatory (bool _mandatory)  {mandatory = _mandatory;}

    kkuint16        NumHierarchialLevels ()  const;

    const  KKStr&   Name ()      const {return  name;}
    const  KKStr&   UpperName () const {return  upperName;}  /**< Returns name capitalized. */

    MLClassPtr      Parent () const {return parent;}
    void            Parent (MLClassPtr  _parent)  {parent = _parent;}
    const KKStr&    ParentName ()  const;

//    void                ProcessRawData (KKStr&  data);  /**< Parses 'data' and populates this instance.
//                                                         *   @details Extracts name of class from first field in 'data' using whitespace
//                                                         *   ',', ' ', '\n', '\r', or '\t' as delimiter.  Data will have this name removed from
//                                                         *   the beginning of it.
//                                                         */

    bool            StoredOnDataBase () const  {return  storedOnDataBase;}

    void            StoredOnDataBase (bool _storedOnDataBase)  {storedOnDataBase = _storedOnDataBase;}

    bool            Summarize () const {return summarize;}   /**< Indicates that Classification report should produce a summary 
                                                              * column for the family of classes decedent from this class. 
                                                              * Example classes that would set this field true are 'Protist',
                                                              * 'Phyto',  'Crustacean', etc....
                                                              */

    void            Summarize (bool _summarize)  {summarize = _summarize;}

    KKStr           ToString ()  const;  /**< Returns a KKStr representing this instance.
                                          *   @details This string will later be written to a file.
                                          */

    bool            UnDefined ()  const  {return  unDefined;}
    void            UnDefined (bool _unDefined)  {unDefined = _unDefined;}

    void            WriteXML (const KKStr&   varName,
                              std::ostream&  o
                             )  const;
  private:
    kkint32         classId;      /**< From MySQL table  Classes, '-1' indicates that not loaded from table.                        */
    float           countFactor;  /**< Specifies number to increment count when this class picked;  ex:  Shrinmp_02 would have 2.0. */
    KKStr           description;

    bool            mandatory;    /**< Class needs to be included in Classification Status even if none occurred. */

    KKStr           name;         /**< Name of Class.                                                                               */


    MLClassPtr      parent;       /**< Supports the concept of Parent/Child classes as part of a hierarchy.
                                   * Adding this field to help support the PicesInterface version of this class.
                                   */

    bool            storedOnDataBase; /**< Because we have no control over classes that users unilaterally create
                                       * it would be useful to know which ones are stored in the PICES database
                                       * table "Classes".
                                       */

    bool            summarize;    /**< Indicates that Classification report should produce a summary column 
                                   * for the family of classes decedent from this class. Example classes that
                                   * would set this field true are 'Protist',  'Phyto',  'Crustacean', etc....
                                   */

    KKStr           upperName;    /**< Upper case version of name;  Used by LookUpByName to assist in performance. */

    bool            unDefined;    /**< A class who's name is "", "UnKnown", "UnDefined", or starts with "Noise_" */

  };  /* MLClass */

  #define  _MLClass_Defined_


  typedef  MLClass*  MLClassPtr;




  /**
   *@class MLClassList
   *@brief  Maintains a list of MLClass instances.
   *@details  There will be only one instance of each MLClass by name and these instances will 
   *          be owned by MLClass::existingMLClasses.
   */
  class  MLClassList:  public KKQueue<MLClass>
  {
  public:
    MLClassList ();


    /**  @brief  Copy constructor;  will copy list but not own the contents. */
    MLClassList (const MLClassList&  _mlClasses);
    


    /** @brief Construct a MLClassList object from the contents of a file. */
    MLClassList (const  KKStr&  fileName,
                 bool&  successfull
                );


    virtual
    ~MLClassList ();


    virtual
    void   AddMLClass (MLClassPtr  _mlClass);


    static
    MLClassListPtr  BuildListFromDelimtedStr (const KKStr&  s,
                                              char          delimiter
                                             );


    /** @brief  Clears the contents of this list and updates nameIndex structure. */
    virtual
      void  Clear ();
      

    MLClassListPtr   ExtractMandatoryClasses ()  const;

    MLClassListPtr   ExtractSummarizeClasses ()  const;

    /**
     *@brief  Using the class names create two title lines where we split
     *        names by "_" characters between the two lines.
     */
    void  ExtractTwoTitleLines (KKStr&  titleLine1,
                                KKStr&  titleLine2 
                               ) const;


    /** 
     *@brief Using the class names create three title lines where we split names 
     *       by "_" characters between the three lines.
     */
    void  ExtractThreeTitleLines (KKStr&  titleLine1,
                                  KKStr&  titleLine2, 
                                  KKStr&  titleLine3 
                                 ) const;

    void  ExtractThreeTitleLines (KKStr&  titleLine1,
                                  KKStr&  titleLine2, 
                                  KKStr&  titleLine3,
                                  kkint32 fieldWidth
                                 ) const;

    /**
     *@brief Will generate a HTML formatted string that can be used in a HTML table.
     *@details Using the class name's create one header line for a HTML table. The 
     *         underscore character ("_") will be used to separate levels
     */
    KKStr   ExtractHTMLTableHeader () const;


    MLClassListPtr  ExtractListOfClassesForAGivenHierarchialLevel (kkint32 level);


    /** @brief  return pointer to instance with '_name';  if none exists, create one and add to list. */
    virtual
    MLClassPtr  GetMLClassPtr (const KKStr& _name);

    MLClassPtr  GetNoiseClass ()  const;

    /**
     *@brief Return a pointer to the MLClass object that represents the unknown Class in the list. 
     *@details That is the one with the name 'UNKNOWN'.  If none is found then one will be created 
     * and added to the list.
     */
    MLClassPtr   GetUnKnownClass ();


    /**
     *@brief  Returns a pointer of MLClass object with name (_name);  if none 
     *        in list will then return NULL.
     *@param[in]  _name  Name of MLClass to search for.
     *@return  Pointer to MLClass or NULL  if not Found.
     */
    virtual
    MLClassPtr     LookUpByName (const KKStr&  _name)  const;


    virtual
    MLClassPtr     LookUpByClassId (kkint32 _classId)  const;


    void           Load (const KKStr&  _fileName,
                         bool&  _successfull
                        );

    kkint32        MemoryConsumedEstimated () const;
      
    static
      MLClassListPtr  MergeClassList (const MLClassList&  list1,
                                      const MLClassList&  list2
                                     );


    KKB::kkuint16  NumHierarchialLevels ()  const;


    virtual
    MLClassPtr  PopFromBack ();

    virtual
    MLClassPtr  PopFromFront ();

    virtual
    void        PushOnBack (MLClassPtr  mlClass);

    virtual
    void        PushOnFront (MLClassPtr  mlClass);


    void        Save (KKStr   _fileName,
                      bool&   _successfull
                     );

    void        SortByName ();

    KKStr       ToString ()  const;

    KKStr       ToCommaDelimitedStr ()  const;

    KKStr       ToTabDelimitedStr ()  const;

    KKStr       ToCommaDelimitedQuotedStr ()  const;

    void        WriteXML (const KKStr&   varName,
                          std::ostream&  o
                         )  const;
    
    bool        operator== (const MLClassList&  right)  const;

    bool        operator!= (const MLClassList&  right)  const;

                                                                       
    MLClassList& operator=  (const MLClassList&  right);

    MLClassList& operator-= (const MLClassList&  right);         /**< remove all classes that in the 'right' parameter */
    
    MLClassList  operator-  (const MLClassList&  right)  const;  /**< remove all classes that in the 'right' parameter */

    MLClassList& operator+= (const MLClassList&  right);         /**< add all classes that are in the 'right' parameter */


  private:
    friend class MLClass;

    void  AddMLClassToNameIndex (MLClassPtr  _mlClass);


    /** @brief  Should only be called from "MLClass::ChangeNameOfClass". */
    void  ChangeNameOfClass (MLClassPtr  mlClass, 
                             const KKStr&   oldName,
                             const KKStr&   newName,
                             bool&          successful
                            );

    typedef  std::map<KKStr,MLClassPtr>   NameIndex;
    NameIndex     nameIndex;

    /**
     *@brief  Set the owner flag.
     *@details Forcing Owner to be private to make sure that no list can own any MLClass objects, to
     * prevent accidental deletion of a 'MLClass' object.  Only 'MLClass::existingMLClasses' may own
     * the contents of its list.
     */
    void      Owner (bool _owner)  {KKQueue<MLClass>::Owner (_owner);}

    bool      undefinedLoaded;  /**< Indicates if the class that represents examples that have not been
                                 *   classified yet has been loaded.
                                 */

    class  MLClassNameComparison;
  };  /* MLClassList */

  #define  _MLClassList_Defined_



  std::ostream&  operator<< (      std::ostream&  os, 
                             const MLClassList&   classList
                            );


  KKStr&  operator<< (      KKStr&        str, 
                      const MLClassList&  classList
                     );


  typedef  MLClassList*  MLClassListPtr;






  /**
   *@class  MLClassIndexList
   *@brief  Maintains a list of classes and their associated integer index.  
   *@details 
   *  Multiple Classes can have the same index but a class may only appear once in the list.  The primary purpose
   *  of this class is to allow the quick access to classes by numerical indexes.  This comes in useful when
   *  communicating with another library that does not recognize alpha numeric strings for class names such
   *  libSVM which only uses integers class names.
   *@see KKMLL::Model
   *@see KKMLL::FeatureEncoder2::compress
   */
  class  MLClassIndexList: public  std::map<MLClassPtr, kkint16>
  {
  public:
    typedef  MLClassIndexList*  MLClassIndexListPtr;

    MLClassIndexList ();
    MLClassIndexList (const MLClassIndexList&  _list);
    MLClassIndexList (const MLClassList&  _classes);

    virtual   ~MLClassIndexList ()  {}

    kkint32  MemoryConsumedEstimated ()  const;

    virtual
      void  Clear ();

    void  AddClass (MLClassPtr  _ic,
                    bool&       _dupEntry
                   );


    void  AddClassIndexAssignment (MLClassPtr _ic,
                                   kkint16    _classIndex,
                                   bool&      _dupEntry
                                  );

    /**
     @brief  Returns the corresponding index to the class 'c';  if not in list will return -1.
     */
    kkint16  GetClassIndex (MLClassPtr  c);


    /**
     *@brief  Locates the MLClass that was assigned classIndex.
     *@details If not found then returns NULL.  If more than one class has the same classIndex will return the first one added.
     */
    MLClassPtr  GetMLClass (kkint16 classIndex);

    
    void  ParseClassIndexList (const KKStr&  s,
                               RunLog&       log
                              );


    /** 
     *@brief Returns string consisting of all contained classes indicating ClassIndex assigned to each class.
     *@details Each class will be separated by a comma(",") delimiter character and each class will consist of 
     * "ClassName" and associated "ClassIndex" separated by colon(":") character.  The class names will be 
     * enclosed in quotes("). The method "ParseClassIndexList" will be able to decode this string to
     * repopulate an instance of "MLClassIndexList".
     *@see "KKStr::QuotedStr"
     * Example String:  "Copepod":1,"Doliolid":2,"Larvacean":3
     */
    KKStr  ToCommaDelString ()  const;


    virtual
    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );


    virtual
    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;


  private:
    std::map<kkint16, MLClassPtr>  shortIdx;
    kkint16                        largestIndex;   /**< largest index used so far. */
  };  /* MLClassIndexList */

  typedef  MLClassIndexList::MLClassIndexListPtr  MLClassIndexListPtr;

  #define  _MLClassIndexList_Defined_


  extern  MLClassList  globalClassList;




  /**
   *@details  
   * - Because a MLClass instance can only be created through the static method "MLClass::CreateNewMLClass" the "XmlElementTemplate"  
   * template can not be used.
   */
  class  XmlElementMLClass:  public  XmlElement
  {
  public:
    XmlElementMLClass (XmlTagPtr      tag,
                       XmlStream&     s,
                       VolConstBool&  cancelFlag,
                       RunLog&        log
                      );
                
    virtual  ~XmlElementMLClass ();

    MLClassPtr  Value ()  const;

    MLClassPtr  TakeOwnership ();

    static
    void  WriteXML (const MLClass&  mlClass,
                    const KKStr&    varName,
                    std::ostream&   o
                   );
  private:
    MLClassPtr  value;
  };
  typedef  XmlElementMLClass*  XmlElementMLClassPtr;




  /**
   *@brief  Will only write the ClassName rather than complete MLClass instances
   */
  class  XmlElementMLClassNameList:  public  XmlElement
  {
  public:
    XmlElementMLClassNameList (XmlTagPtr      tag,
                               XmlStream&     s,
                               VolConstBool&  cancelFlag,
                               RunLog&        log
                              );
                
    virtual  ~XmlElementMLClassNameList ();

    MLClassListPtr  Value ()  const;

    MLClassListPtr  TakeOwnership ();

    static
    void  WriteXML (const MLClassList&  mlClassList,
                    const KKStr&        varName,
                    std::ostream&       o
                   );
  private:
    MLClassListPtr  value;
  };
  typedef  XmlElementMLClassNameList*  XmlElementMLClassNameListPtr;




  typedef  XmlElementTemplate<MLClassIndexList>  XmlElementMLClassIndexList;
  typedef  XmlElementMLClassIndexList*  XmlElementMLClassIndexListPtr;



}  /* namespace KKMLL */


#endif

