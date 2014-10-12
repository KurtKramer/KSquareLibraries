/* XmlStream.h -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _XMLSTREAM_
#define  _XMLSTREAM_
#include  <map>

#include  "KKStr.h"
#include  "Tokenizer.h"
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

    const KKStr&  Name ()           const  {return  name;}
    int32         AttributeCount () const  {return  (int32)attributes.size ();}
    const KKStr&  AttributeName  (int32 _attributeNum)  const;
    const KKStr&  AttributeValue (int32 _attributeNum)  const;

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


  class  XmlOperator
  {
  public:  
    XmlOperator (const KKStr&  _name);
    ~XmlOperator ();


  virtual void  Write (ostream& o,
                       void*    element
                      ) = 0;

  virtual void*  Read (Tokenizer&  i) = 0;

  /**
   * @brief  Will register itself plus any XmlOperator's that it needs to process a Read or Write request.
   */
  virtual void  Register ();

  private:
    KKStr   name;
  };  /* XmlOperator */



  class  XmlStream
  {
  public:
    XmlStream (TokenizerPtr  _tokenStream);

    typedef  XmlStream*  XmlStreamPtr;

    XmlStream ();
    ~XmlStream ();

    XmlTokenPtr  GetNextToken ();  /*!< Will return either a XmlElement or a XmlContent */

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


  class  XmlObject
  {
  public:
    const KKStr&  Name  () const  {return  name;}
    const KKStr&  Type  () const  {return  type;}
    void*         Value () const  {return  value;}

  private:
    KKStr  name;
    KKStr  type;
    void*  value;
  };

/*

  XmlObjectPtr   GetNextObject (XmlStream s)
  {
    // 1) element = GetNextElement ();
    // 2) XmlStream = NewStreamEndingWith (element-Name());
    // 3) return CreateXmlObject (element, XmlStream);
    return NULL;
  }



  XmlObjectPtr   XmlStream::CreateXmlObject (XmlElement  e, XmlStream s)
  {
    XmlOperator op = GetOperator (e);

    XmlObjectList  objs = new XmlObjectList ();

    XmlObject  nextObj = GetNextObject (s);

    while  (nextObj)
    {
      objs.Add (nextObj);
      nextObj = GetNextObject (s);
    }

    op->CreatFommXmlObjs (e, objs);
  }  //  / * CreateXmlObject * /
*/

}


#endif
