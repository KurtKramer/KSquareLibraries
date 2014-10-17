#ifndef  _MLCLASS_
#define  _MLCLASS_

/**
 *@class  KKMachineLearning::MLClass
 *@brief  Represents a "Class" in the Machine Learnining Sense.
 *@author  Kurt Kramer
 *@details
 *@code
 **********************************************************************
 **                          MLClass                               *
 **                                                                   *
 **  Each instance of this class represents a single Class as used in *
 **  Machine Learning sense.  Each instance of 'FeatureVector' class  *
 **  will point to an instance of this class.  There can only be one  *
 **  instance of each Class in memory.  Specificacly the 'name' field *
 **  will be unique.  This is enforced by making the constructor and  *
 **  destructor private.  The only way to create a new instance of a  *
 **  'MLClass' object is to call one of the the static methods of  *
 **  'CreateNewMLClass'.  These methods will look for a instance   *
 **  of 'MLClass' that already exists.  If one does not they will  *
 **  then create a new one.                                           *
 **                                                                   *
 **  Please refer to MLClassList at bottom of file.  That class is *
 **  the object you will most be unsing when dealing with Images.     *
 **  classes.                                                         *
 **                                                                   *
 **  UmiClass                                                         *
 **  There is a special relationship between this class  and a class  *
 **  called 'UmiClass' in the 'UnManagedInterface' library'.  UmiClass*
 **  is a managed c++ version of this class.  There is a one-to-one   *
 **  corespondance between the two classes.  When ever a instance     *
 **  of 'UmiClass' gets created it will automatically call the the    *
 **  the static method 'MLClass::CreateNewMLClass' to get the   *
 **  unmanaged version of the class.                                  *
 **********************************************************************
 *@endcode
 *@see KKMachineLearning::MLClassList, KKMachineLearning::FeatureVector, UnManagedInterface::UmiClass
 */


#include "GoalKeeper.h"
#include "RunLog.h"
#include "KKStr.h"
#include "KKQueue.h"


namespace KKMachineLearning
{

  class  MLClassListIndex;
  typedef  MLClassListIndex*  MLClassListIndexPtr;

  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;

  class  MLClass;
  typedef  MLClass*  MLClassPtr;


  class  MLClass 
  {
  private:
    static  MLClassListIndexPtr   existingMLClasses;


  public:
    typedef  KKB::uint32  uint32;

    static  MLClassListIndexPtr  GlobalClassList ();

    /**
     *@brief  Static method used to create a new instance of a MLClass object.
     *@details
     *@code
     ************************************************************************
     ** Used to get pointer to existing MLClass object that already      *
     ** exists in 'existingMLClasses'.  If one does not exist then a new *
     ** MLClass object with this name will be created.                   *
     **                                                                     *
     ** This is the only method that can actually create a MLClass       *
     ** instance.  The idea is that there is only one MLClass object     *
     ** ever created for each class.  All MLClassList container objects  *
     ** will point to instances that are in 'existingMLClasses.          *
     ************************************************************************
     *@endcode
     *@param[in]  _name   Name of class to be created.
     *@return  Pointer to an instance of 'MLClass' that will have the name '_name'.
     */
    static  MLClassPtr  CreateNewMLClass (const KKStr&  _name,
                                                int32           _classId = -1
                                               );

    static  MLClassPtr  GetUnKnownClassStatic ();

    static  MLClassPtr  GetByClassId (int32  _classId);

    /**
     *@brief Will change the name of an existing class verifying that a duplicate does not get created.
     *@details Will make sure that the new name is not in use by another class     *
     *@param[in]  mlClass  Pointer to existing MLClass instance that you wish to rename.
     *@param[in]  newName     New name that you with to give instance of 'MLClass'
     *@param[out] changeSuccessful Will return'True' if change was successful,  a reason it would not work is that the name is already used.
     */
    static  void  ChangeNameOfClass (MLClassPtr  mlClass, 
                                     const KKStr&   newName,
                                     bool&          changeSuccessful
                                    );


    /** @brief Call this as the last thing the application does, will delete all existing instances of 'MLClass'. */
    static  void  FinalCleanUp ();


  private:
    static void  CreateBlocker ();

    static GoalKeeperPtr  blocker;
    static bool  needToRunFinaleCleanUp;

    friend class KKQueue<MLClass>;
    friend class MLClassList;

    MLClass (const KKStr&  _name);

    MLClass (const MLClass&  mlClass);

    ~MLClass ();

    void  Name (const KKStr&  _name);

  public:
    static KKStr    GetClassNameFromDirName (const KKStr&  subDir);

    static MLClassListPtr  BuildListOfDecendents (MLClassPtr  parent);

   

    int32           ClassId ()  const  {return classId;}
    void            ClassId (int32 _classId)  {classId = _classId;}

    float           CountFactor () const  {return countFactor;}
    void            CountFactor (float _countFactor)  {countFactor = _countFactor;}

    void            Description (const KKStr&  _description)  {description = _description;}

    const KKStr&    Description ()  const {return description;}

    MLClassPtr      MLClassForGivenHierarchialLevel (uint32 level)  const;

    bool            IsAnAncestor (MLClassPtr  c);   // will return true if 'c' is an ancestor

    uint32          NumHierarchialLevels ()  const;

    const  KKStr&   Name ()      const {return  name;}
    const  KKStr&   UpperName () const {return  upperName;} // Will return name capitalized..

    MLClassPtr      Parent () const {return parent;}
    void            Parent (MLClassPtr  _parent)  {parent = _parent;}
    const KKStr&    ParentName ()  const;

    void            ProcessRawData (KKStr&  data);  /**< Will parse data and populate this instance This string would typically be read in from a file. */

    bool            StoredOnDataBase () const  {return  storedOnDataBase;}

    void            StoredOnDataBase (bool _storedOnDataBase)  {storedOnDataBase = _storedOnDataBase;}

    KKStr           ToString ()  const;  /**< Create a KKStr representing this instance.  This string will later be written to a file. */
                    

    bool            UnDefined ()  const  {return  unDefined;}

    void            UnDefined (bool _unDefined)  {unDefined = _unDefined;}

    void            WriteXML (std::ostream& o)  const;
    

  private:
    int32          classId;      /**< From MySQL table  Classes, '-1' indicates that not loaded from table.                        */
    float          countFactor;  /**< Specifies number to increment count when this class picked;  ex:  Shrinmp_02 would have 2.0. */
    KKStr          name;         /**< Name of Class.                                                                               */
    KKStr          upperName;    /**< Upper case version of name;  Used by LookUpByName to assist in performance.                  */

    bool           unDefined;  /**< When  true  MLClass  represents a generic catch-all for all images that do not have a class
                                * defined for them. Only one MLClass in a list should have this set to true. For this one
                                * MLClass the Name field will be empty or "UNKNOWN".
                                */

    MLClassPtr     parent;     /**< Supports the concept of Parent/Child classes as part of a hierarchy.  Adding this field to help
                                * support the UnManagedInterface version of this class.
                                */

    bool           storedOnDataBase;  /**< Because we have no control over classes that users unilaterally creates it would be
                                       * useful to know which ones are stored in database table.
                                       */

    KKStr          description;
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
    typedef  KKB::uint32  uint32;

    MLClassList ();


    /**  @brief  Copy constructor;  will copy list but not own the contents. */
    MLClassList (const MLClassList&  _mlClasses);
    


    /** @brief Construct a MLClassList object from the contents of a file. */
    MLClassList (KKStr   fileName,
                 bool&   successfull
                );


    virtual
    ~MLClassList ();


    virtual
    void   AddMLClass (MLClassPtr  _mlClass);


    static
    MLClassListPtr  BuildListFromDelimtedStr (const KKStr&  s,
                                              char          delimiter
                                             );


    /** @brief  Using the class names create two title lines where we split names by "_" characters between the two lines.  */
    void  ExtractTwoTitleLines (KKStr&  titleLine1,
                                KKStr&  titleLine2 
                               ) const;


    /**  @brief Using the class names create three title lines where we split names by "_" characters between the three lines. */
    void  ExtractThreeTitleLines (KKStr&  titleLine1,
                                  KKStr&  titleLine2, 
                                  KKStr&  titleLine3 
                                 ) const;

    void  ExtractThreeTitleLines (KKStr&  titleLine1,
                                  KKStr&  titleLine2, 
                                  KKStr&  titleLine3,
                                  int32   fieldWidth
                                 ) const;

    /**
     *@brief Will generate a HTML formated string that can be used in a HTML table.
     *@details Using the class names create one header line for a HTML table. The underscore character ("_") will be used to separate levels
     */
    KKStr   ExtractHTMLTableHeader () const;


    MLClassListPtr  ExtractListOfClassesForAGivenHierarchialLevel (int32 level);


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
     *@brief  Returns a pointer of MLClass object with name (_name);  if none in list will then return NULL.
     *@param[in]  _name  Name of MLClass to search for.
     *@return  Pointer to MLClass or NULL  if not Found.
     */
    virtual
    MLClassPtr     LookUpByName (const KKStr&  _name)  const;


    virtual
    MLClassPtr     LookUpByClassId (int32  _classId);


    void           Load (KKStr  _fileName,
                         bool&  _successfull
                        );

    int32          MemoryConsumedEstimated () const;
      
    static
      MLClassListPtr  MergeClassList (const MLClassList&  list1,
                                      const MLClassList&  list2
                                     );


    uint32          NumHierarchialLevels ()  const;


    void            Save (KKStr   _fileName,
                          bool&   _successfull
                         );

    void            SortByName ();

    KKStr           ToString ()  const; 

    KKStr           ToCommaDelimitedStr ()  const;

    KKStr           ToTabDelimitedStr ()  const;

    void            WriteXML (std::ostream&  o)  const;

    bool            operator== (const MLClassList&  right)  const;

    bool            operator!= (const MLClassList&  right)  const;

                                                                       
    MLClassList& operator=  (const MLClassList&  right);

    MLClassList& operator-= (const MLClassList&  right);         /**< remove all classes that in the 'right' parameter */
    
    MLClassList  operator-  (const MLClassList&  right)  const;  /**< remove all classes that in the 'right' parameter */

    MLClassList& operator+= (const MLClassList&  right);         /**< add all classes that are in the 'right' parameter */


  private:
    friend class MLClass;

    /**
     *@brief  Set the owner flag.
     *@details Forcing Owner to be private to make sure that no list can own any MLClass objects, to prevent accidental deletion of a 'MLClass'
     * object.  Only 'MLClass::existingMLClasses' may own the contents of its list.
     */
    void      Owner (bool _owner)  {KKQueue<MLClass>::Owner (_owner);}

    bool      undefinedLoaded;  /**< Indicates if the class that represents Images that have not been classified yet has been loaded. */

    class  MLClassNameComparison;
  };  /* MLClassList */

  #define  _MLClass_List_Defined_



  std::ostream&  operator<< (      std::ostream&  os, 
                             const MLClassList&   classList
                            );


  KKStr&  operator<< (      KKStr&        str, 
                      const MLClassList&  classList
                     );


  typedef  MLClassList*  MLClassListPtr;



  /**
   *@class  MLClassListIndex
   *@brief  List of MLClass instances similar to MLClassList with the addition of indexes by name and classId.
   */
  class  MLClassListIndex:  public  MLClassList
  {
  public:
    typedef  MLClassListIndex*  MLClassListIndexPtr;
     
    MLClassListIndex ();

    /** @brief  Copy constructor; will make a duplicate list of mlClass pointers but will not be owner of them. */
    MLClassListIndex (const MLClassListIndex&  _mlClasses);
    
    /** @brief  Conversion constructor;  will convert a MLClassList instance to a 'MLClassListIndex' instance. */
    MLClassListIndex (const MLClassList&  _mlClasses);
    


    ~MLClassListIndex ();

    virtual
    void   AddMLClass (MLClassPtr  _mlClass);



    /**
     *@brief  Returns a pointer of MLClass object with name (_name);  if none in list will then return NULL.
     *@param[in]  _name  Name of MLClass to search for.
     *@return  Pointer to MLClass or NULL  if not Found.
     */
    virtual
    MLClassPtr  LookUpByName (const KKStr&  _name)  const;


    /**  @brief  return pointer to instance with '_name';  if none exists, create one and add to list.  */
    virtual
    MLClassPtr  GetMLClassPtr (const KKStr&  _name);


  private:
    friend class MLClass;


    /** @brief  Informs the Index that the class name has changed but does not change it in the MLClass instance. */
    void  ChangeNameOfClass (MLClassPtr    mlClass, 
                             const KKStr&  oldName,
                             const KKStr&  newName,
                             bool&         successful
                            );

    typedef  map<KKStr,MLClassPtr>     NameIndex;
    NameIndex     nameIndex;
  };  /* MLClassListIndex */

  #define  _MLClassListIndex_Defined_


  //typedef  MLClassListIndex:MLClassListIndexPtr  MLClassListIndexPtr;



  /**
   *@brief  Maintains a list of classes and their associated integer index.  
   *@details 
   *  Multiple Classes can have the same index but a class may only appear once in the list.  The primary purpose
   *  of this class is to allow the quick access to classes by numerical indexes.  This comes in useful when
   *  communicating with another library that does not recognize alpha numeric strings for class names such
   *  libSVM which only uses integers class names.
   *@see KKMachineLearning::Model
   *@see KKMachineLearning::FeatureEncoder2::compress
   */
  class  ClassIndexList: public  map<MLClassPtr, short>
  {
  public:
    typedef  ClassIndexList*  ClassIndexListPtr;

    ClassIndexList ();
    ClassIndexList (const ClassIndexList&  _list);
    ClassIndexList (const MLClassList&  _classes);

    void  AddClass (MLClassPtr  _ic,
                    bool&       _dupEntry
                   );


    void  AddClassIndexAssignment (MLClassPtr _ic,
                                   short      _classIndex,
                                   bool&      _dupEntry
                                  );

    /**
     @brief  Returns the corresponding index to the class 'c';  if not in list will return -1.
     */
    short  GetClassIndex (MLClassPtr  c);


    /**
     *@brief  Locates the MLClass that was assigned classIndex.
     *@details If not found then returns NULL.  If more than one class has the same classIndex will return the first one added.
     */
    MLClassPtr  GetMLClass (short classIndex);

    int32  MemoryConsumedEstimated ()  const;

    void  ParseClassIndexList (const KKStr&  s);

    KKStr  ToCommaDelString ();


  private:
    map<short, MLClassPtr>  shortIdx;
    short                   largestIndex;   /**< largest index used so far. */
  };  /* ClassIndexList */

  typedef  ClassIndexList::ClassIndexListPtr  ClassIndexListPtr;

  #define  _ClassIndexList_Defined_


  extern  MLClassList  globalClassList;

}  /* namespace KKMachineLearning */


#endif

