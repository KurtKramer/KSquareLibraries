/* XmlStream.cpp -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <string>
#include <iostream>
#include <ostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "BitString.h"
#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "XmlTokenizer.h"
#include "XmlStream.h"
using namespace KKB;




XmlStream::XmlStream (XmlTokenizerPtr _tokenStream):
    endOfElementTagNames (),
    endOfElemenReached   (false),
    fileName             (),
    nameOfLastEndTag     (),
    tokenStream          (_tokenStream),
    weOwnTokenStream     (false)
{
}



XmlStream::XmlStream (const KKStr&  _fileName,
                      RunLog&       _log
                     ):
    endOfElementTagNames (),
    endOfElemenReached   (false),
    fileName             (_fileName),
    nameOfLastEndTag     (),
    tokenStream          (NULL),
    weOwnTokenStream     (false)
{
  tokenStream = new XmlTokenizer (fileName);
  weOwnTokenStream = true;
}




XmlStream::~XmlStream ()
{
}



XmlTokenPtr  XmlStream::GetNextToken (RunLog&  log)
{
  if  (endOfElemenReached)
    return NULL;

  XmlTokenPtr  token = NULL;

  KKStrPtr  t = tokenStream->GetNextToken ();
  if  (t == NULL)
    return NULL;

  if  (t->FirstChar () == '<')
  {
    XmlTagPtr  tag = new XmlTag (t);
    delete  t;
    t = NULL;
    if  (tag->TagType () == XmlTag::TagTypes::tagStart)
    {
      XmlFactoryPtr  factory = XmlFactory::FactoryLookUp (tag->Name ());
      if  (!factory)
        factory = XmlElementKKStr::FactoryInstance ();

      endOfElementTagNames.push_back (tag->Name ());
      token = factory->ManufatureXmlElement (tag, *this, log);
      KKStr  endTagName = endOfElementTagNames.back ();
      endOfElementTagNames.pop_back ();
      endOfElemenReached = false;
    }

    else if  (tag->TagType () == XmlTag::TagTypes::tagEmpty)
    {
      XmlFactoryPtr  factory = XmlFactory::FactoryLookUp (tag->Name ());
      if  (!factory)
        factory = XmlElementKKStr::FactoryInstance ();
      endOfElemenReached = true;
      token = factory->ManufatureXmlElement (tag, *this, log);
      endOfElemenReached = false;
    }
    else if  (tag->TagType () == XmlTag::TagTypes::tagEnd)
    {
      if  (endOfElementTagNames.size () < 1)
      {
        // end tag with no matching start tag.
        log.Level (-1) << endl
            << "XmlStream::GetNextToken   ***ERROR***   Encountered end-tag </" << tag->Name () << ">  with no matching start-tag." << endl
            << endl;
      }
      else
      {
        endOfElemenReached = true;
        nameOfLastEndTag = tag->Name ();
        if  (!endOfElementTagNames.back ().EqualIgnoreCase (nameOfLastEndTag))
        {
          log.Level (-1) << endl
            << "XmlStream::GetNextToken   ***ERROR***   Encountered end-tag </" << nameOfLastEndTag << ">  does not match StartTag <" << endOfElementTagNames.back () << ">." << endl
            << endl;
          // </End-Tag>  does not match <Start-Tag>  we will put back on token stream assuming that we are missing a </End-Tag>
          // We will end the current element here.
          tokenStream->PushTokenOnFront (new KKStr (">"));
          tokenStream->PushTokenOnFront (new KKStr (nameOfLastEndTag));
          tokenStream->PushTokenOnFront (new KKStr ("/"));
          tokenStream->PushTokenOnFront (new KKStr ("<"));
        }
      }
    }
  }
  else
  {
    token = new XmlContent (t);
  }
  return  token;
}  /* GetNextToken */

 


 XmlContentPtr  XmlStream::GetNextContent (RunLog& log)
{
  if  (!tokenStream)
    return NULL;

  KKStrListPtr  tokens = tokenStream->GetNextTokens ("<");
  if  (!tokens)
    return NULL;
  if  (tokens->QueueSize () < 1)
  {
    delete  tokens;
    return NULL;
  }

  KKStrPtr  result = new KKStr (100);

  KKStrList::const_iterator  idx;
  for  (idx = tokens->begin ();  idx != tokens->end ();  ++idx)
    result->Append (*(*idx));

  return  new XmlContent (result);
}



XmlAttributeList::XmlAttributeList (bool  _owner):
  KKQueue<XmlAttribute> (_owner)
{
}


XmlAttributeList::XmlAttributeList (const XmlAttributeList&  attributes)
{
  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin ();  idx != attributes.end ();  ++idx)
    PushOnBack (*idx);
}



void  XmlAttributeList::PushOnBack  (XmlAttributePtr a)
{
  KKQueue<XmlAttribute>::PushOnBack (a);
  nameIndex.insert (NameIndexPair (a->Name (), a));
}


void  XmlAttributeList::PushOnFront (XmlAttributePtr a)
{
  KKQueue<XmlAttribute>::PushOnFront (a);
  nameIndex.insert (NameIndexPair (a->Name (), a));
}


XmlAttributePtr  XmlAttributeList::PopFromBack ()
{
  XmlAttributePtr  a = KKQueue<XmlAttribute>::PopFromBack ();
  if  (a)
    DeleteFromNameIndex (a);
  return  a;
}


XmlAttributePtr  XmlAttributeList::PopFromFromt ()
{
  XmlAttributePtr  a = KKQueue<XmlAttribute>::PopFromFront ();
  if  (a)
    DeleteFromNameIndex (a);
  return  a;
}



void  XmlAttributeList::DeleteFromNameIndex (XmlAttributePtr a)
{
  {
    NameIndex::iterator idx;
    idx = nameIndex.find (a->Name ());
    while  ((idx != nameIndex.end ()) &&  
            (idx->first == a->Name ())  &&
            (idx->second != a)
           )
       ++idx;

    if  ((idx != nameIndex.end ())  &&  (idx->second == a))
      nameIndex.erase (idx);
  }
}



KKStrConstPtr  XmlAttributeList::AttributeValueByName  (const KKStr&  name)   const
{
  XmlAttributePtr  a = LookUpByName (name);
  if  (a)
    return &(a->Name ());
  else
    return NULL;
}


KKStrConstPtr  XmlAttributeList::AttributeValueByIndex (kkuint32  index)  const
{
  if  (index >= size ())
    return NULL;
  else
    return &(at (index)->Value ());
}


KKStrConstPtr  XmlAttributeList::AttributeNameByIndex  (kkuint32  index)  const
{
  if  (index >= size ())
    return NULL;
  else
    return &(at (index)->Name ());
}



const KKStr&   XmlAttributeList::AttributeValueKKStr   (const KKStr&  name)   const
{
  KKStrConstPtr s = AttributeValueByName (name);
  if  (s)
    return  *s;
  else
    return KKStr::EmptyStr ();
}



kkint32  XmlAttributeList::AttributeValueInt32  (const KKStr&  name)   const
{
  KKStrConstPtr s = AttributeValueByName (name);
  if  (s)
    return  0;
  else
    return s->ToInt32 ();
}




void  XmlAttributeList::AddAttribute (const KKStr&  name,
                                      const KKStr&  value
                                     )
{
  PushOnBack (new XmlAttribute (name, value));
}



XmlAttributePtr  XmlAttributeList::LookUpByName (const KKStr&  name) const
{
  NameIndex::const_iterator  idx;
  idx = nameIndex.find (name);
  if  (idx == nameIndex.end ())
    return NULL;

  if  (idx->first != name)
    return NULL;

  return idx->second;
}



void  ReadWholeTag (istream&  i,
                    KKStr&    tagStr
                   )
{
  tagStr = "";
  while  (!i.eof ())
  {
    char nextCh = i.get ();
    if  (nextCh != 0)
      tagStr.Append (nextCh);
    if  (nextCh == '>')
      break;
  }

  return;
}  /* ReadWholeTag */






void  ExtractAttribute (KKStr&  tagStr, 
                        KKStr&  attributeName,
                        KKStr&  attributeValue
                       )
{
  kkint32  startIdx = 0;
  kkint32  len = tagStr.Len ();
  attributeName  = "";
  attributeValue = "";

  // Skip over leading spaces
  while  (startIdx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[startIdx]) == NULL)
      break;
    startIdx++;
  }

  if  (startIdx >= len)
  {
    tagStr = "";
    return;
  }

  kkint32 idx = startIdx;

  // Skip until we find the '=' character.
  while  (idx < len)
  {
    if  (tagStr[idx] == '=')
      break;
    ++idx;
  }

  if  (idx >= len)
  {
    tagStr = "";
    return;
  }

  attributeName = tagStr.SubStrPart (startIdx, idx - 1);

  ++idx;  // Skip past '=' character.
  
  // Skip over leading spaces
  while  (idx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[idx]) == NULL)
      break;
    ++idx;
  }

  if  (idx >= len)
  {
    tagStr = "";
    return;
  }

  int  valueStartIdx = idx;

  while  (idx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[idx]) != NULL)
      break;
    ++idx;
  }

  attributeValue = tagStr.SubStrPart (valueStartIdx, idx - 1);

  tagStr = tagStr.SubStrPart (idx + 1);

  return;
}  /* ExtractAttribute */



XmlTag::XmlTag (istream&  i):
   attributes (true),
   name       (),
   tagType    (TagTypes::tagNULL)
{
  tagType = TagTypes::tagNULL;

  if  (i.peek () == '<')
    i.get ();

  KKStr tagStr (100);
  ReadWholeTag (i, tagStr);

  if  (tagStr.FirstChar () == '/')
  {
    tagStr.ChopFirstChar ();
    tagType = TagTypes::tagEnd;
  }

  if  (tagStr.EndsWith ("/>"))
  {
    tagType = TagTypes::tagEmpty;
    tagStr.ChopLastChar ();
    tagStr.ChopLastChar ();
  }

  else if  (tagStr.LastChar () != '>')
  {
    tagType = TagTypes::tagStart;
  }

  else
  {
    if  (tagType == TagTypes::tagNULL)
      tagType = TagTypes::tagStart;
    tagStr.ChopLastChar ();
  }

  name.TrimLeft ();
  name.TrimRight ();

  name = tagStr.ExtractToken2 (" \n\r\t");
  KKStr attributeName (20);
  KKStr attributeValue (20);

  while  (!tagStr.Empty ())
  {
    ExtractAttribute (tagStr, attributeName, attributeValue);
    if  (!attributeName.Empty ())
      attributes.AddAttribute (attributeName, attributeValue);
  }
}



XmlTag::XmlTag (const KKStrConstPtr  tagStr):
   attributes (true),
   name       (),
   tagType    (TagTypes::tagNULL)
{
  KKStrParser parser(tagStr);
  parser.TrimWhiteSpace (" ");

  if  (parser.PeekNextChar () == '<')
    parser.GetNextChar ();

  if  (parser.PeekLastChar () == '>')
    parser.GetLastChar ();

  if  (parser.PeekNextChar () == '/')
  {
    parser.GetNextChar ();
    tagType = XmlTag::TagTypes::tagEnd;
  }
  else if  (parser.PeekLastChar () == '/')
  {
    parser.GetLastChar ();
    tagType = XmlTag::TagTypes::tagEmpty;
  }
  else
  {
    tagType = XmlTag::TagTypes::tagStart;
  }

  name = parser.GetNextToken ();
  parser.SkipWhiteSpace ();

  while  (parser.MoreTokens ())
  {
    KKStr attributeName  = parser.GetNextToken ("=");
    KKStr attributeValue = parser.GetNextToken (" \t\n\r");
    attributes.AddAttribute (attributeName, attributeValue);
  }
}  /* XmlTag::XmlTag (const KKStrConstPtr  tagStr) */




XmlTag::XmlTag (const KKStr&  _name,
                TagTypes      _tagType
               ):
   attributes (true),
   name       (),
   tagType    (TagTypes::tagNULL)
{
}




void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           const KKStr&  attributeValue
                          )
{
  attributes.AddAttribute (attributeName, attributeValue);
}




void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           double        attributeValue
                          )
{
  KKStr  s (12);
  s << attributeValue;
  attributes.AddAttribute (attributeName, s);
}




void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           kkint32       attributeValue
                         )
{
  attributes.AddAttribute (attributeName, StrFromInt32 (attributeValue));
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           bool          attributeValue
                          )
{
  attributes.AddAttribute (attributeName, (attributeValue ? "Yes" : "No"));
}




KKStrConstPtr  XmlTag::AttributeValueByName  (const KKStr&  name)   const
{
  return  attributes.AttributeValueByName (name);
}


KKStrConstPtr  XmlTag::AttributeValueByIndex (kkuint32  index)  const
{
  return  attributes.AttributeValueByIndex (index);
}


KKStrConstPtr  XmlTag::AttributeNameByIndex  (kkuint32  index)  const
{
  return  attributes.AttributeNameByIndex (index);
}


kkint32  XmlTag::AttributeValueInt32 (const KKStr& attributeName)  const
{
 return  this->attributes.AttributeValueInt32  (attributeName);
}


const KKStr&  XmlTag::AttributeValueKKStr (const KKStr& attributeName)  const
{
 return  attributes.AttributeValueKKStr  (attributeName);
}





void  XmlTag::WriteXML (ostream& o)
{
  o << "<";
  
  if  (tagType == TagTypes::tagEnd)
    o << "/";

  o << name;

  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin();  idx != attributes.end ();  ++idx)
  {
    XmlAttributePtr  a = *idx;
    o << " " <<  a->Name () << "=" << a->Value ().QuotedStr ();
  }

  if  (tagType == TagTypes::tagEmpty)
    o << "/";

   o << ">";
}  /* WriteXML */





XmlToken::XmlToken ()
{
}


XmlToken::~XmlToken ()
{
}




XmlElement::XmlElement (XmlTagPtr   _nameTag,
                        XmlStream&  s,
                        RunLog&     log
                       ):
  nameTag (_nameTag)
{}
 


XmlElement ::~XmlElement ()
{
}



const KKStr&   XmlElement::Name ()  const 
{
  if  (nameTag)
    return  nameTag->Name ();
  else
    return KKStr::EmptyStr ();
}


const KKStr&   XmlElement::VarName ()  const
{
  if  (!nameTag)
    return KKStr::EmptyStr ();

  KKStrConstPtr  nameStr = nameTag->AttributeValueByName ("VarName");
  if  (nameStr)
    return  *nameStr;
  else
    return  KKStr::EmptyStr ();
}




KKStrConstPtr  XmlElement::AttributeValue (const KKStr&  attributeName)
{
  if  (nameTag)
    return nameTag->AttributeValueByName (attributeName);
  else
    return NULL;
}


KKStrConstPtr  XmlElement::AttributeValue (const char* attributeName)
{
  if  (nameTag)
    return nameTag->AttributeValueByName (attributeName);
  else
    return NULL;
}




XmlContent::XmlContent (KKStrPtr  _content): 
      XmlToken (), 
      content (_content) 
{
}



XmlContent::~XmlContent ()
{
  delete  content;
  content = NULL;
}



KKStrPtr  XmlContent::TakeOwnership ()
{
  KKStrPtr  c = content;
  content = NULL;
  return c;
}




map<KKStr, XmlFactory*>*  XmlFactory::factories = NULL;


XmlFactory*  XmlFactory::FactoryLookUp (const KKStr&  className)
{
  GlobalGoalKeeper::StartBlock ();
  if  (factories == NULL)
  {
    factories = new map<KKStr, XmlFactory*> ();
    atexit (FinalCleanUp);
  }

  XmlFactory*  factory = NULL;

  map<KKStr, XmlFactory*>::const_iterator  idx;
  idx = factories->find (className);
  if  (idx != factories->end ())
    factory = idx->second;

  GlobalGoalKeeper::EndBlock ();

  return  factory;
}  /* FactoryLookUp */



void   XmlFactory::RegisterFactory  (XmlFactory*  factory)
{
  if  (!factory)
  {
    KKStr  errMsg = "XmlStream::RegisterFactory   ***ERROR***   (factory == NULL).";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  GlobalGoalKeeper::StartBlock ();
  XmlFactory*  existingFactory = FactoryLookUp (factory->ClassName ());
  if  (existingFactory)
  {
    cerr << endl
         << "XmlStream::RegisterFactory   ***ERROR***   Factory[" << factory->ClassName () << "] already exists." << endl
         << endl;
  }
  else
  {
    factories->insert (pair<KKStr, XmlFactory*> (factory->ClassName (), factory));
  }
  GlobalGoalKeeper::EndBlock ();
}



void  XmlFactory::FinalCleanUp ()
{
  if  (factories)
  {
    map<KKStr, XmlFactory*>::iterator  idx;
    for  (idx = factories->begin ();  idx != factories->end ();  ++idx)
      delete  idx->second;

    delete  factories;
    factories = NULL;
  }
}



XmlFactory::XmlFactory (const KKStr&  _clasName):
   className (_clasName)
{
}





XmlElementBool::XmlElementBool (XmlTagPtr   tag,
                                XmlStream&  s,
                                RunLog&     log
                               ):
    XmlElement (tag, s, log),
    value (false)
{
  KKStrConstPtr  valueStr = tag->AttributeValueByName ("Value");
  if  (valueStr)
    value = valueStr->ToBool ();
  XmlTokenPtr t = s.GetNextToken (log);
  while  (t != NULL)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      value = c->Content ()->ToBool ();
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}



XmlElementBool::~XmlElementBool ()
{
}



bool  XmlElementBool::Value ()  const
{
  return  value;
}



void  XmlElementBool::WriteXML (const bool    b,
                                const KKStr&  varName,
                                ostream&      o
                               )
{
  XmlTag startTag ("Bool", XmlTag::TagTypes::tagEmpty);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Value", b);
  startTag.WriteXML (o);
  o << endl;
}

XmlFactoryMacro(Bool)






XmlElementKKStr::XmlElementKKStr (XmlTagPtr   tag,
                                  XmlStream&  s,
                                  RunLog&     log
                                 ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new KKStr (10);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      value->Append (c->Content ());
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}
                


XmlElementKKStr::~XmlElementKKStr ()
{
  delete  value;
  value = NULL;
}



KKStrPtr const  XmlElementKKStr::Value ()  const
{
  return value;
}


KKStrPtr  XmlElementKKStr::TakeOwnership ()
{
  KKStrPtr  v = value;
  value = NULL;
  return  v;
}



void  XmlElementKKStr::WriteXML (const KKStr&  s,
                                 const KKStr&  varName,
                                 ostream&      o
                                )
{
  XmlTag startTag ("KKStr", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << s.ToXmlStr ();
  XmlTag  endTag ("KKStr", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(KKStr)




XmlFactoryPtr  XmlElementKKStr::FactoryInstance ()
{
  return  XmlFactoryKKStr::FactoryInstance ();
}





XmlElementVectorKKStr::XmlElementVectorKKStr (XmlTagPtr   tag,
                                              XmlStream&  s,
                                              RunLog&     log
                                             ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new VectorKKStr (10);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*c->Content ());

      while  (p.MoreTokens ())
      {
        KKStr  s = p.GetNextToken (",\t\n\r");
        value->push_back (s);
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}




XmlElementVectorKKStr::~XmlElementVectorKKStr ()
{
  delete  value;
  value = NULL;
}

VectorKKStr*  const  XmlElementVectorKKStr::Value ()  const
{
  return  value;
}


VectorKKStr*  XmlElementVectorKKStr::TakeOwnership ()
{
  VectorKKStr* v = value;
  value = NULL;
  return v;
}


void  XmlElementVectorKKStr::WriteXml (const VectorKKStr&  v,
                                       const KKStr&        varName,
                                       ostream&            o
                                      )
{
  XmlTag t ("VectorKKStr", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
  t.AddAtribute ("VarName", varName);
  t.WriteXML (o);
  VectorKKStr::const_iterator idx;
  kkint32  c = 0;
  for  (idx = v.begin (), c= 0;  idx != v.end ();  ++idx, ++c)
  {
    if  (c > 0)
      o << "\t";
    o << idx->QuotedStr ();
  }

  XmlTag  endTag ("VectorKKStr", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXml */


XmlFactoryMacro(VectorKKStr)






XmlElementKKStrListIndexed::XmlElementKKStrListIndexed (XmlTagPtr   tag,
                                                        XmlStream&  s,
                                                        RunLog&     log
                                                       ):
  XmlElement (tag, s, log),
  value (NULL)
{
  bool  caseSensative = tag->AttributeValueByName ("CaseSensative")->ToBool ();
  value = new KKStrListIndexed (true, caseSensative);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*c->Content ());

      while  (p.MoreTokens ())
      {
        KKStr  s = p.GetNextToken (",\t\n\r");
        value->Add (new KKStr (s));
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}



XmlElementKKStrListIndexed::~XmlElementKKStrListIndexed ()
{
  delete  value;
}


KKStrListIndexed*  const  XmlElementKKStrListIndexed::Value ()  const
{
  return value;
}


KKStrListIndexed*  XmlElementKKStrListIndexed::TakeOwnership ()
{
  KKStrListIndexed*  v = value;
  value = NULL;
  return v;
}


void  XmlElementKKStrListIndexed::WriteXml (const KKStrListIndexed& v,
                                            const KKStr&            varName,
                                            ostream&                o
                                           )
{
  XmlTag  startTag ("KKStrListIndexed", XmlTag::TagTypes::tagStart);

  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("CaseSensative", v.CaseSensative ());
  startTag.WriteXML (o);

  kkuint32 n = v.size ();
  for  (kkuint32 x = 0;  x < n;  ++x)
  {
    KKStrConstPtr s = v.LookUp ((kkint32)x);
    if  (x > 0)
      o << "\t";
    o << s->QuotedStr ();
  }

  XmlTag endTag ("KKStrListIndexed", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(KKStrListIndexed)










#define  XmlElementIntegralBody(T,TypeName,ParseMethod)           \
 XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr   tag,     \
                                             XmlStream&  s,       \
                                             RunLog&     log      \
                                            ):                    \
  XmlElement (tag, s, log),                                       \
  value ((T)0)                                                    \
{                                                                 \
  KKStrConstPtr  valueStr = tag->AttributeValueByName ("Value");  \
  if  (valueStr)                                                  \
    value = (T)valueStr->##ParseMethod ();                        \
  XmlTokenPtr tok = s.GetNextToken (log);                         \
  while  (tok != NULL)                                            \
  {                                                               \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)   \
    {                                                             \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);        \
      value = (T)c->Content ()->##ParseMethod ();                 \
    }                                                             \
    delete  tok;                                                  \
    tok = s.GetNextToken (log);                                   \
  }                                                               \
}                                                                 \
                                                                  \
                                                                  \
XmlElement##TypeName::~ XmlElement##TypeName ()                   \
{                                                                 \
}                                                                 \
                                                                  \
                                                                  \
void   XmlElement##TypeName::WriteXML (T             d,           \
                                       const KKStr&  varName,     \
                                       ostream&      o            \
                                      )                           \
{                                                                 \
  XmlTag startTag (#TypeName, XmlTag::TagTypes::tagEmpty);        \
  if  (!varName.Empty ())                                         \
    startTag.AddAtribute ("VarName", varName);                    \
  startTag.AddAtribute ("Value", d);                              \
  startTag.WriteXML (o);                                          \
  o << endl;                                                      \
}                                                                 \
                                                                  \
XmlFactoryMacro(TypeName)











#define  XmlElementArrayBody(T,TypeName,ParserNextTokenMethod)                \
XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr   tag,                  \
                                            XmlStream&  s,                    \
                                            RunLog&     log                   \
                                          ):                                  \
      XmlElement (tag, s, log),                                               \
      count (0),                                                              \
      value (NULL)                                                            \
{                                                                             \
  count = tag->AttributeValueInt32 ("Count");                                 \
  if  (count < 0)                                                             \
  {                                                                           \
    log.Level (-1) << endl                                                    \
      << "XmlElement##TypeName   ***ERROR***   Attribute Count[" << count     \
      << "] must be a positive value; will set array to NULL." << endl        \
      << endl;                                                                \
      count = 0;                                                              \
  }                                                                           \
                                                                              \
  if  (count <= 0)                                                            \
  {                                                                           \
    value = NULL;                                                             \
    count = 0;                                                                \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    value = new T[count];                                                     \
  }                                                                           \
                                                                              \
  kkuint32  fieldsExtracted = 0;                                              \
  XmlTokenPtr  tok = s.GetNextToken (log);                                    \
  while  (tok)                                                                \
  {                                                                           \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)               \
    {                                                                         \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);                    \
                                                                              \
      KKStrParser p (c->Content ());                                          \
                                                                              \
      while  (p.MoreTokens ())                                                \
      {                                                                       \
        double  zed = p.##ParserNextTokenMethod ("\t,");                      \
        if  (fieldsExtracted < count)                                         \
          value[fieldsExtracted] = (T)zed;                                    \
        ++fieldsExtracted;                                                    \
      }                                                                       \
    }                                                                         \
    delete  tok;                                                              \
    tok = s.GetNextToken (log);                                               \
  }                                                                           \
                                                                              \
  if  (fieldsExtracted != count)                                              \
  {                                                                           \
    log.Level (-1) << endl                                                    \
      << "XmlElement##TypeName<>   ***ERROR***   FieldsExtracted["            \
      << fieldsExtracted << "]  differs from specified Count["                \
      << count << "]" << endl                                                 \
      << endl;                                                                \
  }                                                                           \
}                                                                             \
                                                                              \
                                                                              \
XmlElement##TypeName::~XmlElement##TypeName ()                                \
{                                                                             \
  delete  value;                                                              \
  value = NULL;                                                               \
}                                                                             \
                                                                              \
                                                                              \
T*   XmlElement##TypeName::TakeOwnership ()                                   \
{                                                                             \
  T* v = value;                                                               \
  value = NULL;                                                               \
  return v;                                                                   \
}                                                                             \
                                                                              \
                                                                              \
void  XmlElement##TypeName::WriteXML (kkuint32       count,                   \
                                      const T*       d,                       \
                                      const KKStr&   varName,                 \
                                      ostream&       o                        \
                                      )                                       \
{                                                                             \
  XmlTag startTag (#TypeName, XmlTag::TagTypes::tagStart);                    \
  if  (!varName.Empty ())                                                     \
    startTag.AddAtribute ("VarName", varName);                                \
  startTag.AddAtribute ("Count", (kkint32)count);                             \
  startTag.WriteXML (o);                                                      \
                                                                              \
  for  (kkuint32 x = 0;  x < count;  ++x)                                     \
  {                                                                           \
    if  (x > 0)                                                               \
      o << "\t";                                                              \
    o << d[x];                                                                \
  }                                                                           \
  XmlTag  endTag (#TypeName, XmlTag::TagTypes::tagEnd);                       \
  endTag.WriteXML (o);                                                        \
  o << endl;                                                                  \
}                                                                             \
                                                                              \
XmlFactoryMacro(TypeName)







#define  XmlElementVectorBody(T,TypeName,ParserNextTokenMethod)            \
XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr   tag,               \
                                            XmlStream&  s,                 \
                                            RunLog&     log                \
                                           ):                              \
  XmlElement (tag, s, log)                                                 \
{                                                                          \
  kkint32  count = 0;                                                      \
  KKStrConstPtr  countStr = tag->AttributeValueByName ("Count");           \
  if  (countStr)                                                           \
    count = countStr->ToInt32 ();                                          \
                                                                           \
  value = new vector<T> ();                                                \
                                                                           \
  XmlTokenPtr  tok = s.GetNextToken (log);                                 \
  while  (tok)                                                             \
  {                                                                        \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)            \
    {                                                                      \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);                 \
                                                                           \
      KKStrParser p (c->Content ());                                       \
                                                                           \
      while  (p.MoreTokens ())                                             \
      {                                                                    \
        T  zed = p.##ParserNextTokenMethod ("\t,");                        \
        value->push_back ((T)zed);                                         \
      }                                                                    \
    }                                                                      \
    delete  tok;                                                           \
    tok = s.GetNextToken (log);                                            \
  }                                                                        \
}                                                                          \
                                                                           \
                                                                           \
                                                                           \
XmlElement##TypeName::~XmlElement##TypeName ()                             \
{                                                                          \
  delete  value;                                                           \
  value = NULL;                                                            \
}                                                                          \
                                                                           \
                                                                           \
                                                                           \
vector<T>*  XmlElement##TypeName::TakeOwnership ()                         \
{                                                                          \
  vector<T>*  v = value;                                                   \
  value = NULL;                                                            \
  return v;                                                                \
}                                                                          \
                                                                           \
                                                                           \
                                                                           \
void  XmlElement##TypeName::WriteXML (const vector<T>&  v,                 \
                                      const KKStr&      varName,           \
                                      ostream&          o                  \
                                     )                                     \
{                                                                          \
  XmlTag startTag (#TypeName, XmlTag::TagTypes::tagStart);                 \
  if  (!varName.Empty ())                                                  \
    startTag.AddAtribute ("VarName", varName);                             \
  startTag.WriteXML (o);                                                   \
                                                                           \
  for  (kkuint32 x = 0;  x < v.size ();  ++x)                              \
  {                                                                        \
    if  (x > 0)                                                            \
      o << "\t";                                                           \
    o << v[x];                                                             \
  }                                                                        \
  XmlTag  endTag (#TypeName, XmlTag::TagTypes::tagEnd);                    \
  endTag.WriteXML (o);                                                     \
  o << endl;                                                               \
}                                                                          \
                                                                           \
XmlFactoryMacro(TypeName)





// Integral Types
XmlElementIntegralBody(kkint32,Int32,ToInt32)  // XmlElementInt32

XmlElementIntegralBody(float,Float,ToDouble)   // XmlElementFloat

XmlElementIntegralBody(double,Double,ToDouble) // XmlElementDouble




// Arrays

XmlElementArrayBody(kkuint16, ArrayUint16,   GetNextTokenUint)     // XmlElementArrayUint16

XmlElementArrayBody(kkint32,  ArrayInt32,    GetNextTokenInt)      // XmlElementArrayInt32

XmlElementArrayBody(double,   ArrayDouble,   GetNextTokenDouble)   

XmlElementArrayBody(float,    ArrayFloat,    GetNextTokenDouble)


// Vectors

XmlElementVectorBody(kkint32,  VectorInt32,  GetNextTokenInt)
