/* XmlStream.h -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _XMLSTREAM_
#define  _XMLSTREAM_
#include  <map>

#include "KKStr.h"
#include "RunLog.h"
#include "Tokenizer.h"
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




  class  XmlStream
  {
  public:
    typedef  XmlStream*  XmlStreamPtr;

    XmlStream (TokenizerPtr  _tokenStream);

    XmlStream (const XmlStreamPtr  _parentXmlStream,
               const KKStr&        _endOfElementTagName
              );

    virtual  ~XmlStream ();

    void  BufferedXmlTag (XmlTagPtr  _bufferedXmlTag)  {bufferedXmlTag = _bufferedXmlTag;}

    virtual  XmlTokenPtr  GetNextToken (RunLog&  log);  /**< Will return either a XmlElement or a XmlContent */

  private:
    /**
     *@brief  returns the index of the latest instance of 'name' pushed onto the stack; if no entry with the same name returns -1.
     */
    kkint32  FindLastInstanceOnElementNameStack (const KKStr&  name);

    XmlTagPtr     bufferedXmlTag;
    KKStr         endOfElementTagName;
    bool          endOfElemenReached;
    XmlStreamPtr  paremtXmlStream;
    TokenizerPtr  tokenStream;

  };  /* XmlStream */


  typedef  XmlStream::XmlStreamPtr  XmlStreamPtr;




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


  class  XmlAttributeList:  public map<KKStr, KKStr>
  {
  public:
    XmlAttributeList ();
    XmlAttributeList (const XmlAttributeList&  attributes);

    KKStrConstPtr  LookUp (const KKStr&  name) const;
  };  /* XmlAttributeList */

  typedef  XmlAttributeList*  XmlAttributeListPtr;



  class  XmlTag
  {
  public:
    typedef  XmlTag*  XmlTagPtr;
    typedef  enum {tagNULL, tagStart, tagEnd, tagEmpty}  TagTypes;

    XmlTag (TokenizerPtr  _tokenStream);

    /**
     *@brief Will construct a generic XML tag from the following characters in the stream.
     *@details It sis assumed that the next character read from the input stream "i" will be '<'; if not then it is assumed that the next 
     * character is the one immediately following the '<'.
     */
    XmlTag (istream&  i);

    const KKStr&  Name ()           const  {return  name;}
    TagTypes      TagType ()        const  {return  tagType;}
    kkint32       AttributeCount () const  {return  (kkint32)attributes.size ();}
    KKStrConstPtr  AttributeName  (kkint32 _attributeNum)  const;
    KKStrConstPtr  AttributeValue (kkint32 _attributeNum)  const;
    KKStrConstPtr  AttributeValue (const KKStr& attributeName)  const;
    KKStrConstPtr  AttributeValue (const char*  attributeName)  const;

  private:
    KKStr             name;
    XmlAttributeList  attributes;
    TagTypes          tagType;
  };  /* XmlTag */

  typedef  XmlTag::XmlTagPtr  XmlTagPtr;




  class  XmlToken
  {
  public:
    typedef  XmlToken*  XmlTokenPtr;
    typedef  enum  {tokNULL, tokElement, tokContent}  TokenTypes;

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

    virtual  TokenTypes  TokenType () {return  XmlToken::tokElement;}

    const KKStr&   Name () const;

  private:
    XmlTagPtr  nameTag;
    void*      body;

  };  /* XmlElement */

  typedef  XmlElement::XmlElementPtr  XmlElementPtr;




  class  XmlContent: public  XmlToken
  {
  public:
    typedef  XmlContent* XmlContentPtr;

    XmlContent (KKStrPtr  _content);
    virtual ~XmlContent ();

    virtual  TokenTypes  TokenType () {return  XmlToken::tokContent;}

    KKStrPtr const  Content () const  {return  content;}
    KKStrPtr        TakeOwnership ();

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




  class  XmlElementInt32:  public  XmlElement
  {
  public:
    XmlElementInt32 (XmlTagPtr   tag,
                     XmlStream&  s,
                     RunLog&     log
                    );
                
    virtual  ~XmlElementInt32 ();

    kkint32  Value ()  const;

  private:
    kkint32  value;
  };
  typedef  XmlElementInt32*  XmlElementInt32Ptr;



  class  XmlElementVectorInt32:  public  XmlElement
  {
  public:
    XmlElementVectorInt32 (XmlTagPtr   tag,
                           XmlStream&  s,
                           RunLog&     log
                          );
                
    virtual  ~XmlElementVectorInt32 ();

    VectorInt32*  const  Value ()  const;

    VectorInt32*  TakeOwnership ();

  private:
    VectorInt32*  value;
  };
  typedef  XmlElementVectorInt32*  XmlElementVectorInt32Ptr;



  class  XmlElementKKStr:  public  XmlElement
  {
  public:
    XmlElementKKStr (XmlTagPtr   tag,
                     XmlStream&  s,
                     RunLog&     log
                    );
                
    virtual  ~XmlElementKKStr ();

    KKStrPtr  const  Value ()  const;

    KKStrPtr  TakeOwnership ();

    static  XmlFactoryPtr  FactoryInstance ();

  private:
    KKStrPtr  value;
  };
  typedef  XmlElementKKStr*  XmlElementKKStrPtr;




}  /* KKB */



#endif
