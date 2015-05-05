/* XmlStream.h -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _XMLSTREAM_
#define  _XMLSTREAM_
#include  <map>

#include "DateTime.h"
#include "KKStr.h"
#include "RunLog.h"
#include "XmlTokenizer.h"
using namespace KKB;

namespace  KKB
{
  /**
   *@class  XmlStream  XmlStream.h
   *@brief  I created these Xml classes 2010-02-22  to someday simplify the reading and writing
   *        of objects to disk.  Right now it is more of an Idea Generator than anything usable.
   */

  class  XmlToken;
  typedef  XmlToken*  XmlTokenPtr;

  class  XmlElement;
  typedef  XmlElement*  XmlElementPtr;

  class  XmlContent;
  typedef  XmlContent*  XmlContentPtr;

  class  XmlFactory;
  typedef  XmlFactory*  XmlFactoryPtr;

  class  XmlFactoryManager;
  typedef  XmlFactoryManager*  XmlFactoryManagerPtr;



  class  XmlStream
  {
  public:
    typedef  XmlStream*  XmlStreamPtr;

    XmlStream (XmlTokenizerPtr  _tokenStream);

    XmlStream (const KKStr&  _fileName,
               RunLog&       _log
              );

    virtual  ~XmlStream ();

    virtual  XmlTokenPtr  GetNextToken (RunLog&  log);     /**< Will return either a XmlElement or a XmlContent which ever is next. 
                                                            * If we are at the end of the element then NULL will be returned,
                                                            */


    virtual  XmlContentPtr  GetNextContent (RunLog& log);  /**< Will return any content that may exist before the next tag; if 
                                                            * there is no content before the next tag will return NULL
                                                            */

    void  RegisterFactory (XmlFactoryPtr  factory);        /** Registers factory with the highest level FactoryManager in 'factoryManagers'. */


  private:
    /**
     *@brief  returns the index of the latest instance of 'name' pushed onto the stack; if no entry with the same name returns -1.
     */
    kkint32  FindLastInstanceOnElementNameStack (const KKStr&  name);


    void  PushXmlElementLevel (const KKStr&  sectionName);

    void  PopXmlElementLevel ();

    XmlFactoryPtr  TrackDownFactory (const KKStr&  sectionName);     /** Will search for the factory starting at the highest level of 'factoryManagers'
                                                                      * the working down the stack; if not found there will then look at the 
                                                                      'XmlFactory::globalXmlFactoryManager'
                                                                      */


    /**
     * When we start a new element the name of the Section is pushed on the back  of 'endOfElementTagNames'.  This
     * is how we keep track of the End tag that we are looking for to close out the current element.  The member
     * 'factoryManagers' works in synchronization with endOfElementTagNames.  That is when we start a new Element
     * we push an empty 'XmlFactoryManager' instance onto 'factoryManagers'. This will allow XmlElement derived 
     * classes to specify 'XmlFactory' derived instances that are specific to their universe.  An example use would
     * be the 'TrainingProcess2' class requiring to add a XmlElement factory for 'Model' derived classes. These 
     * Factory derived classes would contain a cancelFlag reference that they pass in the constructor to all 
     */
    VectorKKStr                    endOfElementTagNames;
    vector<XmlFactoryManagerPtr>   factoryManagers;


    bool             endOfElemenReached;
    KKStr            fileName;
    KKStr            nameOfLastEndTag;
    XmlTokenizerPtr  tokenStream;
    bool             weOwnTokenStream;
  };  /* XmlStream */


  typedef  XmlStream::XmlStreamPtr  XmlStreamPtr;

#define  _XmlStream_Defined_


  class  XmlAttribute
  {
  public:
    XmlAttribute (const KKStr&  _name,
                  const KKStr&  _value
                 ):
        name  (_name),
        value (_value)
    {}

    const KKStr&  Name  ()  const  {return name;}
    const KKStr&  Value ()  const  {return value;}

  private:
    KKStr  name;
    KKStr  value;
  };

  typedef  XmlAttribute*  XmlAttributePtr;
  typedef  XmlAttribute const  XmlAttributeConst;
  typedef  XmlAttributeConst*  XmlAttributeConstPtr;



  class  XmlAttributeList:  public KKQueue<XmlAttribute> 
  {
  public:
    XmlAttributeList (bool  _owner);
    XmlAttributeList (const XmlAttributeList&  attributes);

    void  AddAttribute (const KKStr&  name,
                        const KKStr&  value
                       );

    virtual  void  PushOnBack  (XmlAttributePtr a);
    virtual  void  PushOnFront (XmlAttributePtr a);

    virtual  XmlAttributePtr  PopFromBack ();
    virtual  XmlAttributePtr  PopFromFromt ();

    XmlAttributePtr  LookUpByName (const KKStr&  name) const;

    KKStrConstPtr  AttributeValueByName   (const KKStr&  name)   const;
    KKStrConstPtr  AttributeValueByIndex  (kkuint32      index)  const;
    KKStrConstPtr  AttributeNameByIndex   (kkuint32      index)  const;

    const KKStr&   AttributeValueKKStr    (const KKStr&  name)   const;
    kkint32        AttributeValueInt32    (const KKStr&  name)   const;
    DateTime       AttributeValueDateTime (const KKStr&  name)   const;

  private:
    void  DeleteFromNameIndex (XmlAttributePtr a);

    typedef  multimap<KKStr,XmlAttributePtr>   NameIndex;
    typedef  pair<KKStr,XmlAttributePtr>       NameIndexPair;
    NameIndex  nameIndex;
  };  /* XmlAttributeList */

  typedef  XmlAttributeList*  XmlAttributeListPtr;



  class  XmlTag
  {
  public:
    typedef  XmlTag*  XmlTagPtr;
    enum  class  TagTypes  {tagNULL, tagStart, tagEnd, tagEmpty};

    XmlTag (const KKStrConstPtr  tagStr);

    XmlTag (const KKStr&  _name,
           TagTypes       _tagType
           );

    /**
     *@brief Will construct a generic XML tag from the following characters in the stream.
     *@details It sis assumed that the next character read from the input stream "i" will be '<'; if not then it is assumed that the next 
     * character is the one immediately following the '<'.
     */
    XmlTag (istream&  i);

    const KKStr&  Name ()           const  {return  name;}
    TagTypes      TagType ()        const  {return  tagType;}

    kkint32        AttributeCount () const  {return  (kkint32)attributes.size ();}

    KKStrConstPtr  AttributeValueByName  (const KKStr&  name)   const;
    KKStrConstPtr  AttributeValueByIndex (kkuint32      index)  const;
    KKStrConstPtr  AttributeNameByIndex  (kkuint32      index)  const;

    const KKStr&   AttributeValueKKStr    (const KKStr&  name)   const;
    kkint32        AttributeValueInt32    (const KKStr&  attributeName)  const;
    DateTime       AttributeValueDateTime (const KKStr&  attributeName)  const;


    void  AddAtribute (const KKStr&  attributeName,
                       const KKStr&  attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       bool          attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       kkint32       attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       kkint64       attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       double        attributeValue
                      );

    void  AddAtribute (const KKStr&     attributeName,
                       const DateTime&  attributeValue
                      );

    KKStr  ToString ()  const;

    void  WriteXML (ostream& o);

  private:
    KKStr             name;
    XmlAttributeList  attributes;
    TagTypes          tagType;
  };  /* XmlTag */

  typedef  XmlTag::XmlTagPtr  XmlTagPtr;
  typedef  XmlTag const  XmlTagConst;
  typedef  XmlTagConst*  XmlTagConstPtr;

#define  _XmlTag_Defined_



  /**
   *@brief The parent Class to the two type of tokens,  "XmlElement"  and  "XmlContent"
   */
  class  XmlToken
  {
  public:
    typedef  XmlToken*  XmlTokenPtr;
    enum  class  TokenTypes  {tokNULL, tokElement, tokContent};

    XmlToken ();
    virtual  ~XmlToken ();

    virtual  TokenTypes  TokenType () = 0;


    /** If derived class is from the XmlElement family will be the name from the StartTag(XmlTag::Name ()) otherwise KKStr::EmptyStr()  */
    virtual  const KKStr&  SectionName ()  const  {return  KKStr::EmptyStr ();}


    /**  If the derived class is form the 'XmlElement' line will return the 'VarName' from that derived class otherwise it will return back a empty string. */
    virtual  const KKStr&  VarName ()  const  {return KKStr::EmptyStr ();}

  private:
  };  /* XmlToken */

  typedef  XmlToken::XmlTokenPtr  XmlTokenPtr;



  /**
   * Parent class for all XmlElements;  when XmlStream encounters the start of a element it looks up the appropriate ElementFactory that 
   * will create an instance of a XmlElement derived class. The constructor of that class will be provided the XmlTag that starts the element 
   * plus a pointer to the XmlStream instance that is reading the XML file. The XmlElement derived classes constructor will the be responsible
   * creating an instance of the class that the XmnlElement wraps.
   */
  class  XmlElement: public  XmlToken
  {
  public:
    typedef  XmlElement*  XmlElementPtr;

    XmlElement (const KKStr&      sectionName,
                XmlTag::TagTypes  tagType
               );

    XmlElement (XmlTagPtr   _nameTag,
                XmlStream&  s,
                RunLog&     log
               );
                
    virtual  ~XmlElement ();

    virtual  const KKStr&  SectionName ()  const;

    virtual  TokenTypes  TokenType () {return  TokenTypes::tokElement;}

    virtual  const KKStr&   VarName ()  const;

    XmlTagConstPtr  NameTag () const   {return nameTag;}

    KKStrConstPtr  AttributeValue (const char*   attributeName);
    KKStrConstPtr  AttributeValue (const KKStr&  attributeName);

  private:
    XmlTagPtr  nameTag;
  };  /* XmlElement */

  typedef  XmlElement::XmlElementPtr  XmlElementPtr;




  class  XmlContent: public  XmlToken
  {
  public:
    typedef  XmlContent* XmlContentPtr;

    XmlContent (KKStrPtr  _content);
    virtual ~XmlContent ();

    virtual  TokenTypes  TokenType () {return  TokenTypes::tokContent;}

    KKStrPtr const  Content () const  {return  content;}
    KKStrPtr        TakeOwnership ();

    static
    void  WriteXml (const KKStr&  s,
                    ostream&      o
                   );

  private:
    KKStrPtr  content;
  };  /* XmlContent */

  typedef  XmlContent*  XmlContentPtr;


  



  class  XmlFactoryManager
  {
  public:
    XmlFactoryManager (const KKStr&  _name);

    ~XmlFactoryManager ();
 

    /**
     *@brief  Give the FactoryManager instance ownership of this factory;  the name of the factory must be 
     * unique.
     */
    void   RegisterFactory  (XmlFactory*  factory);
      
    XmlFactory*  FactoryLookUp (const KKStr&  className)  const;

  private:
    map<KKStr, XmlFactory*>  factories;
    KKStr                    name;
  };  /* XmlFactoryManager */

  typedef  XmlFactoryManager*  XmlFactoryManagerPtr;






  class  XmlFactory
  {
  public:
    XmlFactory (const KKStr&  _clasName);

    virtual  const KKStr&  ClassName ()  const  {return className;}

    virtual  XmlElementPtr  ManufatureXmlElement (XmlTagPtr   tag,
                                                  XmlStream&  s,
                                                  RunLog&     log
                                                 ) = 0;
      
    static  XmlFactory*  FactoryLookUp (const KKStr&  className);

    /**
     *@brief  register a instance of a Derives Factory class for the Global XmlFactoryManager.
     *@param[in]  factory  The instance that is being registered; factories will take ownership and the method 
     *  'XmlStream' will be responsible for deleting upon application shutdown.
     */
    static  void   RegisterFactory  (XmlFactory*  factory);

    static  XmlFactoryManagerPtr  globalXmlFactoryManager;

    static  void  FinalCleanUp ();
    private:
      KKStr  className;  /**< Class that this factory produces instances of. */

  };  /* XmlFactory */

  typedef  XmlFactory*  XmlFactoryPtr;
  typedef  XmlFactory const *  XmlFactoryConstPtr;




  
  /**
   *@brief  To be used for classes that implement default constructor, ReadXML, and WriteXML.
   */
  template<class  T>
  class  XmlElementTemplate:  public  XmlElement
  {
  public:
    XmlElementTemplate (XmlTagPtr   tag,
                        XmlStream&  s,
                        RunLog&     log
                       ):
     XmlElement (tag, s, log),
     value (NULL)
    {
      value = new T();
      value->ReadXML (s, tag, log);
    }

                
    ~XmlElementTemplate ()
    {
      delete value;
      value = NULL;
    }

    T*  const  Value ()  const  {return value;}

    T*  TakeOwnership ()
    {
      T* v = value;
      value = NULL;
      return  v;
    }

    static
    void  WriteXML (const T&      t,
                    const KKStr&  varName,
                    ostream&      o
                   )
    {
      t.WriteXML (varName, o);
    }
  private:
    T*  value;
  };  /* XmlElementTemplate */





  /****************************************************************************/
  class  XmlElementBool:  public  XmlElement
  {
  public:
    XmlElementBool (XmlTagPtr   tag,
                    XmlStream&  s,
                    RunLog&     log
                   );
                
    virtual  ~XmlElementBool ();

    bool  Value ()  const;

    static
    void  WriteXML (const bool    b,
                    const KKStr&  varName,
                    ostream&      o
                   );

  private:
    bool  value;
  };
  typedef  XmlElementBool*  XmlElementBoolPtr;






  /****************************************************************************/
  class  XmlElementDateTime:  public  XmlElement
  {
  public:
    XmlElementDateTime (XmlTagPtr   tag,
                        XmlStream&  s,
                        RunLog&     log
                       );
                
    virtual  ~XmlElementDateTime ();

    DateTime  Value ()  const   {return  value;}

    static
    void  WriteXML (const DateTime&  d,
                    const KKStr&     varName,
                    ostream&         o
                   );

  private:
    DateTime  value;
  };
  typedef  XmlElementDateTime*  XmlElementDateTimePtr;











  /****************************************************************************/
  class  XmlElementKeyValuePairs:  public  XmlElement
  {
  public:

    /**
     *@brief  Used to construct an instance that will be written out to a XML file.
     */
    XmlElementKeyValuePairs ();
    

    /**
     * Used while from XmlStream  while reading file;  everytime it comes across a new Section(Start-Tag) 
     * a new instance of this class will be instantiated.
     */
    XmlElementKeyValuePairs (XmlTagPtr   tag,
                             XmlStream&  s,
                             RunLog&     log
                            );
                
    virtual  ~XmlElementKeyValuePairs ();

    vector<pair<KKStr,KKStr> >*  Value ()  const  {return value;}

    vector<pair<KKStr,KKStr> >*  TakeOwnership ();

    void  Add (const KKStr&  key,
               const KKStr&  v
              );

    void  Add (const KKStr&  key,
               kkint32       v
              );

    void  Add (const KKStr&  key,
               float         v
              );

    void  Add (const KKStr&  key,
               double        v
              );

    void  Add (const KKStr&  key,
               bool          v
              );

    void  Add (const KKStr&          key,
               const KKB::DateTime&  v
              );

    void  WriteXML (const KKStr&  varName,
                    ostream&      o
                   );

  private:
    vector<pair<KKStr,KKStr> >*  value;
  };  /* XmlElementKeyValuePairs */

  typedef  XmlElementKeyValuePairs*  XmlElementKeyValuePairsPtr;





  class  XmlElementArrayFloat2DVarying: public XmlElement
  {
  public:
    XmlElementArrayFloat2DVarying (XmlTagPtr   tag,
                                   XmlStream&  s,           
                                   RunLog&     log          
                                  );

    virtual  ~XmlElementArrayFloat2DVarying ();

    kkuint32   Height ()  const  {return height;}     
     
    float**    Value  ()  const  {return value;}

    kkuint32*  Widths ()  const  {return widths;}

    float**   TakeOwnership ();

    kkuint32*   TakeOwnershipWidths ();

    static                                         
    void  WriteXML (kkuint32        height,
                    const kkint32*  widths,      /**< Each entry in array defines the length of the corresponding row in 'mat'.  */ 
                    float** const   mat,
                    const KKStr&    varName,
                    ostream&        o 
                   );


    private:
      kkuint32   height;
      float**    value;
      kkuint32*  widths;
  };

  typedef  XmlElementArrayFloat2DVarying*   XmlElementArrayFloat2DVaryingPtr;







  typedef   XmlElementTemplate<KKStr>  XmlElementKKStr;
  typedef  XmlElementKKStr*  XmlElementKKStrPtr;
  XmlFactoryPtr  XmlElementKKStrFactoryInstance ();


  typedef  XmlElementTemplate<VectorKKStr>  XmlElementVectorKKStr;
  typedef  XmlElementVectorKKStr*  XmlElementVectorKKStrPtr;


  typedef  XmlElementTemplate<KKStrList>  XmlElementKKStrList;
  typedef  XmlElementKKStrList*  XmlElementKKStrListPtr;
  

  typedef  XmlElementTemplate<KKStrListIndexed>  XmlElementKKStrListIndexed;
  typedef  XmlElementKKStrListIndexed*  XmlElementKKStrListIndexedPtr;
  



#define  XmlFactoryMacro(NameOfClass)                                             \
    class  XmlFactory##NameOfClass: public XmlFactory                             \
    {                                                                             \
    public:                                                                       \
      XmlFactory##NameOfClass (): XmlFactory (#NameOfClass) {}                    \
                                                                                  \
      virtual  XmlElement##NameOfClass*  ManufatureXmlElement (XmlTagPtr   tag,   \
                                                               XmlStream&  s,     \
                                                               RunLog&     log    \
                                                              )                   \
      {                                                                           \
        return new XmlElement##NameOfClass(tag, s, log);                          \
      }                                                                           \
                                                                                  \
      static   XmlFactory##NameOfClass*   factoryInstance;                        \
                                                                                  \
      static   XmlFactory##NameOfClass*   FactoryInstance ()                      \
      {                                                                           \
        if  (factoryInstance == NULL)                                             \
        {                                                                         \
          GlobalGoalKeeper::StartBlock ();                                        \
          if  (!factoryInstance)                                                  \
          {                                                                       \
            factoryInstance = new XmlFactory##NameOfClass ();                     \
            XmlFactory::RegisterFactory (factoryInstance);                        \
          }                                                                       \
          GlobalGoalKeeper::EndBlock ();                                          \
         }                                                                        \
        return  factoryInstance;                                                  \
      }                                                                           \
    };                                                                            \
                                                                                  \
    XmlFactory##NameOfClass*   XmlFactory##NameOfClass::factoryInstance           \
                  = XmlFactory##NameOfClass::FactoryInstance ();




// To be used when the XmlElement derived class is defined as a member of the class in question.

#define  XmlFactoryMacro2(NameOfClass)                                            \
    class  XmlFactory##NameOfClass: public XmlFactory                             \
    {                                                                             \
    public:                                                                       \
      XmlFactory##NameOfClass (): XmlFactory (#NameOfClass) {}                    \
      virtual  XmlElement##NameOfClass*  ManufatureXmlElement (XmlTagPtr   tag,   \
                                                               XmlStream&  s,     \
                                                               RunLog&     log    \
                                                              )                   \
      {                                                                           \
        return new ##NameOfClass::Xml (tag, s, log);                              \
      }                                                                           \
                                                                                  \
      static   XmlFactory##NameOfClass*   factoryInstance;                        \
                                                                                  \
      static   XmlFactory##NameOfClass*   FactoryInstance ()                      \
      {                                                                           \
        if  (factoryInstance == NULL)                                             \
        {                                                                         \
          GlobalGoalKeeper::StartBlock ();                                        \
          if  (!factoryInstance)                                                  \
          {                                                                       \
            factoryInstance = new XmlFactory##NameOfClass ();                     \
            XmlFactory::RegisterFactory (factoryInstance);                        \
          }                                                                       \
          GlobalGoalKeeper::EndBlock ();                                          \
         }                                                                        \
        return  factoryInstance;                                                  \
      }                                                                           \
    };                                                                            \
                                                                                  \
    XmlFactory##NameOfClass*   XmlFactory##NameOfClass::factoryInstance           \
                  = XmlFactory##NameOfClass::FactoryInstance ();







#define  XmlElementBuiltInTypeHeader(T,TypeName)       \
                                                       \
  class  XmlElement##TypeName:  public  XmlElement     \
  {                                                    \
  public:                                              \
    XmlElement##TypeName (XmlTagPtr   tag,             \
                          XmlStream&  s,               \
                          RunLog&     log              \
                         );                            \
                                                       \
    virtual  ~XmlElement##TypeName ();                 \
                                                       \
    T  Value ()  const  {return value;}                \
                                                       \
    static                                             \
    void  WriteXML (T             d,                   \
                    const KKStr&  varName,             \
                    ostream&      o                    \
                   );                                  \
                                                       \
  private:                                             \
    T   value;                                         \
  };                                                   \
  typedef  XmlElement##TypeName*   XmlElement##TypeName##Ptr;






#define  XmlElementArrayHeader(T,TypeName,ParserNextTokenMethod)   \
  class  XmlElement##TypeName:  public  XmlElement                 \
  {                                                                \
  public:                                                          \
    XmlElement##TypeName (XmlTagPtr   tag,                         \
                          XmlStream&  s,                           \
                          RunLog&     log                          \
                         );                                        \
                                                                   \
    virtual  ~XmlElement##TypeName ();                             \
                                                                   \
    kkuint32  Count ()  const  {return count;}                     \
    T*        Value ()  const  {return value;}                     \
                                                                   \
    T*   TakeOwnership ();                                         \
                                                                   \
    static                                                         \
    void  WriteXML (kkuint32       count,                          \
                    const T*       d,                              \
                    const KKStr&   varName,                        \
                    ostream&       o                               \
                   );                                              \
                                                                   \
  private:                                                         \
    kkuint32  count;                                               \
    T*        value;                                               \
  };                                                               \
  typedef  XmlElement##TypeName*   XmlElement##TypeName##Ptr;



#define  XmlElementArray2DHeader(T,TypeName,XmlElementToUse)       \
  class  XmlElement##TypeName:  public  XmlElement                 \
  {                                                                \
  public:                                                          \
    XmlElement##TypeName (XmlTagPtr   tag,                         \
                          XmlStream&  s,                           \
                          RunLog&     log                          \
                         );                                        \
                                                                   \
    virtual  ~XmlElement##TypeName ();                             \
                                                                   \
    kkuint32   Height ()  const  {return height;}                  \
    T**        Value  ()  const  {return value;}                   \
    kkuint32   Width  ()  const  {return width;}                   \
                                                                   \
    T**   TakeOwnership ();                                        \
                                                                   \
    static                                                         \
    void  WriteXML (kkuint32      height,                          \
                    kkuint32      width,                           \
                    T** const     mat,                             \
                    const KKStr&  varName,                         \
                    ostream&      o                                \
                   );                                              \
                                                                   \
  private:                                                         \
    kkuint32  height;                                              \
    T**       value;                                               \
    kkuint32  width;                                               \
  };                                                               \
  typedef  XmlElement##TypeName*   XmlElement##TypeName##Ptr;








#define  XmlElementVectorHeader(T,TypeName,ParserNextTokenMethod)  \
  class  XmlElement##TypeName:  public  XmlElement                 \
  {                                                                \
  public:                                                          \
    XmlElement##TypeName (XmlTagPtr   tag,                         \
                          XmlStream&  s,                           \
                          RunLog&     log                          \
                         );                                        \
                                                                   \
    virtual  ~XmlElement##TypeName ();                             \
                                                                   \
    vector<T>*  const  Value ()  const {return value;}             \
                                                                   \
    vector<T>*  TakeOwnership ();                                  \
                                                                   \
    static                                                         \
    void  WriteXML (const vector<##T>&  d,                         \
                    const KKStr&        varName,                   \
                    ostream&            o                          \
                   );                                              \
                                                                   \
  private:                                                         \
    vector<T>*  value;                                             \
  };                                                               \
  typedef  XmlElement##TypeName*   XmlElement##TypeName##Ptr;




XmlElementBuiltInTypeHeader(kkint32, Int32)
XmlElementBuiltInTypeHeader(kkint64, Int64)
XmlElementBuiltInTypeHeader(float,   Float)
XmlElementBuiltInTypeHeader(double,  Double)



XmlElementArrayHeader(kkuint16, ArrayUint16,   GetNextTokenUint)     // XmlElementArrayUint16

XmlElementArrayHeader(kkint32,  ArrayInt32,    GetNextTokenInt)      // XmlElementArrayInt32

XmlElementArrayHeader(double,   ArrayDouble,   GetNextTokenDouble)   // XmlElementArrayDouble

XmlElementArrayHeader(float,    ArrayFloat,    GetNextTokenDouble)   // XmlElementArrayFloat


XmlElementArray2DHeader(float, ArrayFloat2D, XmlElementArrayFloat)   // XmlElementArrayFloat2D



XmlElementVectorHeader(kkint32,  VectorInt32,  GetNextTokenInt)
XmlElementVectorHeader(float,    VectorFloat,  GetNextTokenFloat)



}  /* KKB */



#endif
