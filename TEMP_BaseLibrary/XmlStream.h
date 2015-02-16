/* XmlStream.h -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#ifndef  _XMLSTREAM_
#define  _XMLSTREAM_
#include  <map>

#include "Str.h"
#include "Tokenizer.h"
using namespace KKU;

namespace  KKU
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

  typedef  vector<XmlAttribute>  XmlAttributeList;



  class  XmlTag
  {
  public:
    typedef  XmlTag*  XmlTagPtr;
    typedef  enum {tagNULL, tagStart, tagEnd, tagEmpty}  TagTypes;

    XmlTag (TokenizerPtr  _tokenStream);

    /**
     *@brief Will construct a generix XML tag from the following characters in the stream.
     *@details It sis assumed that the next chracter read from the input strem "i" will be '<'; if not then it is assumed that the next 
     * chracter is the one imediately following the '<'.
     */
    XmlTag (istream&  i);

    const KKStr&  Name ()           const  {return  name;}
    TagTypes      TagType ()        const  {return  tagType;}
    int32         AttributeCount () const  {return  (int32)attributes.size ();}
    const KKStr&  AttributeName  (int32 _attributeNum)  const;
    const KKStr&  AttributeValue (int32 _attributeNum)  const;
    const KKStr&  AttributeValue (const KKStr& attributeName)  const;
    const KKStr&  AttributeValue (const char*  attributeName)  const;

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

    XmlElement ();
    XmlElement (TokenizerPtr  _tokenStream);

    const KKStr&   Name ()  {return (tag != NULL) ? tag->Name () : KKStr::EmptyStr ();}

  private:
    XmlTagPtr  tag;
  };  /* XmlElement */

  typedef  XmlElement::XmlElementPtr  XmlElementPtr;



  class  XmlContent: public  XmlToken
  {
  public:
    typedef  XmlContent* XmlContentPtr;

    XmlContent (TokenizerPtr  _tokenStream);

  private:
    KKStr  content;
  };  /* XmlContent */




  class  XmlStream
  {
  public:
    XmlStream (TokenizerPtr  _tokenStream);

    typedef  XmlStream*  XmlStreamPtr;

    XmlStream ();
    ~XmlStream ();

    XmlTokenPtr  GetNextToken ();  /**< Will return either a XmlElement or a XmlContent */

    typedef  XmlElementPtr  (*XmlElementCreator) (XmlTagPtr  tag, XmlStream& i);

    static
    void   RegisterXmlElementCreator  (const KKStr&       elementName,
                                       XmlElementCreator  creator
                                      );

  private:
    XmlElementPtr  ProcessElement ();

    TokenizerPtr  tokenStream;

    static  map<KKStr, XmlElementCreator>  xmlElementCreators;
    static  XmlElementCreator  LookUpXmlElementCreator (const KKStr&  _name);
  };  /* XmlStream */
}






#endif
