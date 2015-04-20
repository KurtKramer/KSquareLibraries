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

    XmlToken (TokenTypes  _tokenType);

    TokenTypes  TokenType ()  {return tokenType;}

  private:
    TokenTypes  tokenType;
  };  /* XmlToken */

  typedef  XmlToken::XmlTokenPtr  XmlTokenPtr;


  class  XmlElement: public  XmlToken
  {
  public:
    typedef  XmlElement*  XmlElementPtr;

    XmlElement (XmlTagPtr  _nameTag,
                void*      _body
               );
                
    virtual  ~XmlElement ();

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

    XmlContent (const KKStr&  _content): XmlToken (tokContent), content (_content) {}

    const KKStr  Content () const  {return  content;}

  private:
    KKStr  content;
  };  /* XmlContent */

  typedef  XmlContent*  XmlContentPtr;




  class  XmlStream
  {
  public:
    XmlStream (TokenizerPtr  _tokenStream);

    typedef  XmlStream*  XmlStreamPtr;

    XmlStream ();
    ~XmlStream ();

    XmlTokenPtr  GetNextToken (RunLog&  log);  /**< Will return either a XmlElement or a XmlContent */

    XmlElementPtr  GetNextElement ();


    class  Factory
    {
    public:
      Factory (const KKStr&  _clasName);
      virtual  const KKStr&  ClassName ()  const  {return className;}

      virtual  void*  ManufatureInstance (const XmlTag&  tag,
                                          XmlStream&     s,
                                          RunLog&        log
                                         ) = 0;
    private:
      KKStr  className;  /**< Class that this factory produces instances of. */
    };



    static  Factory*  FactoryLookUp (const KKStr&  className);

    /**
     *@brief  register a instance of a Derives Factory class that is meant to create instances for a  specific class.
     *@param[in]  factory  The instance that is being registered; factories will take ownership and the method 
     *  'XmlStream' will be responsible for deleting upon application shutdown.
     */
    static  void   RegisterFactory  (Factory*  factory);

    static  map<KKStr, Factory*>*  factories;

    static  void  FinalCleanUp ();

  private:
    TokenizerPtr  tokenStream;
  };  /* XmlStream */
}




#endif
