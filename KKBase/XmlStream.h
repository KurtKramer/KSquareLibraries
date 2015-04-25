/* XmlStream.h -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _XMLSTREAM_
#define  _XMLSTREAM_
#include  <map>

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

  class XmlToken;
  typedef  XmlToken*  XmlTokenPtr;

  class XmlElement;
  typedef  XmlElement*  XmlElementPtr;

  class  XmlContent;
  typedef  XmlContent*  XmlContentPtr;


  class  XmlStream
  {
  public:
    typedef  XmlStream*  XmlStreamPtr;

    XmlStream (XmlTokenizerPtr  _tokenStream);

    XmlStream (const KKStr&  _fileName,
               RunLog&       _log
              );

    virtual  ~XmlStream ();

    virtual  XmlTokenPtr  GetNextToken (RunLog&  log);  /**< Will return either a XmlElement or a XmlContent which ever is next. 
                                                         * If we are at the end of the element then NULL will be returned,
                                                         */


    virtual  XmlContentPtr  GetNextContent (RunLog& log);  /**< Will return any content that may exist before the next tag; if 
                                                            * there is no content before the next tag will return NULL
                                                            */

  private:
    /**
     *@brief  returns the index of the latest instance of 'name' pushed onto the stack; if no entry with the same name returns -1.
     */
    kkint32  FindLastInstanceOnElementNameStack (const KKStr&  name);

    VectorKKStr      endOfElementTagNames;
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

    KKStrConstPtr  AttributeValueByName  (const KKStr&  name)   const;
    KKStrConstPtr  AttributeValueByIndex (kkuint32      index)  const;
    KKStrConstPtr  AttributeNameByIndex  (kkuint32      index)  const;

    const KKStr&   AttributeValueKKStr   (const KKStr&  name)   const;
    kkint32        AttributeValueInt32   (const KKStr&  name)   const;


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

    const KKStr&   AttributeValueKKStr   (const KKStr&  name)   const;
    kkint32        AttributeValueInt32   (const KKStr&  attributeName)  const;

    void  AddAtribute (const KKStr&  attributeName,
                       const KKStr&  attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       double        attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       kkint32       attributeValue
                      );

    void  AddAtribute (const KKStr&  attributeName,
                       bool          attributeValue
                      );

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




  class  XmlToken
  {
  public:
    typedef  XmlToken*  XmlTokenPtr;
    enum  class  TokenTypes  {tokNULL, tokElement, tokContent};

    XmlToken ();
    virtual  ~XmlToken ();

    virtual  TokenTypes  TokenType () = 0;

  private:
  };  /* XmlToken */

  typedef  XmlToken::XmlTokenPtr  XmlTokenPtr;



  class  XmlElement: public  XmlToken
  {
  public:
    typedef  XmlElement*  XmlElementPtr;

    XmlElement (XmlTagPtr   _nameTag,
                XmlStream&  s,
                RunLog&     log
               );
                
    virtual  ~XmlElement ();

    virtual  TokenTypes  TokenType () {return  TokenTypes::tokElement;}

    const KKStr&   Name () const;   /**< Comes from 'name' field of XmlTag 'nameTab'. */

    const KKStr&   VarName ()  const;

    XmlTagConstPtr  NameTag () const;

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
     *@brief  register a instance of a Derives Factory class that is meant to create instances for a  specific class.
     *@param[in]  factory  The instance that is being registered; factories will take ownership and the method 
     *  'XmlStream' will be responsible for deleting upon application shutdown.
     */
    static  void   RegisterFactory  (XmlFactory*  factory);

    static  map<KKStr, XmlFactory*>*  factories;

    static  void  FinalCleanUp ();
    private:
      KKStr  className;  /**< Class that this factory produces instances of. */

  };  /* XmlFactory */

  typedef  XmlFactory*  XmlFactoryPtr;
  typedef  XmlFactory const *  XmlFactoryConstPtr;



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





  class  XmlElementKKStr:  public  XmlElement
  {
  public:
    XmlElementKKStr (XmlTagPtr   tag,
                     XmlStream&  s,
                     RunLog&     log
                    );
                
    virtual  ~XmlElementKKStr ();

    KKStrPtr  const  Value ()  const  {return value;}

    KKStrPtr  TakeOwnership ();

    static  XmlFactoryPtr  FactoryInstance ();

    static
    void  WriteXML (const KKStr&  s,
                    const KKStr&  varName,
                    ostream&      o
                   );

  private:
    KKStrPtr  value;
  };
  typedef  XmlElementKKStr*  XmlElementKKStrPtr;




  class  XmlElementVectorKKStr:  public  XmlElement
  {
  public:
    XmlElementVectorKKStr (XmlTagPtr   tag,
                           XmlStream&  s,
                           RunLog&     log
                         );
                
    virtual  ~XmlElementVectorKKStr ();

    VectorKKStr*  const  Value ()  const  {return value;}

    VectorKKStr*  TakeOwnership ();

    static  XmlFactoryPtr  FactoryInstance ();

    static
    void  WriteXML (const VectorKKStr& v,
                    const KKStr&       varName,
                    ostream&           o
                   );

  private:
    VectorKKStr*  value;
  };
  typedef  XmlElementVectorKKStr*  XmlElementVectorKKStrPtr;




  
  class  XmlElementKKStrList:  public  XmlElement
  {
  public:
    XmlElementKKStrList (XmlTagPtr   tag,
                         XmlStream&  s,
                         RunLog&     log
                       );
                
    virtual  ~XmlElementKKStrList();

    KKStrListPtr  const  Value ()  const  {return value;}

    KKStrListPtr  TakeOwnership ();

    static  XmlFactoryPtr  FactoryInstance ();

    static
    void  WriteXML (const KKStrList& v,
                    const KKStr&     varName,
                    ostream&         o
                   );

  private:
    KKStrListPtr  value;
  };
  typedef  XmlElementKKStrList*  XmlElementKKStrListPtr;




  
  class  XmlElementKKStrListIndexed:  public  XmlElement
  {
  public:
    XmlElementKKStrListIndexed (XmlTagPtr   tag,
                                XmlStream&  s,
                                RunLog&     log
                              );
                
    virtual  ~XmlElementKKStrListIndexed ();

    KKStrListIndexed*  const  Value ()  const;

    KKStrListIndexed*  TakeOwnership ();

    static
    void  WriteXML (const KKStrListIndexed& v,
                    const KKStr&            varName,
                    ostream&                o
                   );

  private:
    KKStrListIndexed*  value;
  };
  typedef  XmlElementKKStrListIndexed*  XmlElementKKStrListIndexedPtr;




#define  XmlFactoryMacro(NameOfClass)                                             \
    class  XmlFactory##NameOfClass: public XmlFactory                             \
    {                                                                             \
    public:                                                                       \
      XmlFactory##NameOfClass (): XmlFactory (#NameOfClass) {}                    \
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







#define  XmlElementIntegralHeader(T,TypeName)          \
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
    vector<##T>*  const  Value ()  const {return value;}           \
                                                                   \
    vector<##T>*  TakeOwnership ();                                \
                                                                   \
    static                                                         \
    void  WriteXML (const vector<##T>&  d,                         \
                    const KKStr&        varName,                   \
                    ostream&            o                          \
                   );                                              \
                                                                   \
  private:                                                         \
    VectorInt32*  value;                                           \
  };                                                               \
  typedef  XmlElement##TypeName*   XmlElement##TypeName##Ptr;




XmlElementIntegralHeader(kkint32,Int32)
//typedef  XmlElementInt32*   XmlElementInt32Ptr;



XmlElementIntegralHeader(float,Float)

XmlElementIntegralHeader(double,Double)








XmlElementArrayHeader(kkuint16, ArrayUint16,   GetNextTokenUint)     // XmlElementArrayUint16

XmlElementArrayHeader(kkint32,  ArrayInt32,    GetNextTokenInt)      // XmlElementArrayInt32

XmlElementArrayHeader(double,   ArrayDouble,   GetNextTokenDouble)   // XmlElementArrayDouble

XmlElementArrayHeader(float,    ArrayFloat,    GetNextTokenDouble)   // XmlElementArrayFloat


XmlElementVectorHeader(kkint32,  VectorInt32,  GetNextTokenInt)


}  /* KKB */



#endif
