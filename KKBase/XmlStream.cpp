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
#include "Tokenizer.h"
#include "XmlStream.h"
using namespace KKB;




XmlStream::XmlStream (TokenizerPtr _tokenStream):
    endOfElementTagNames (),
    endOfElemenReached   (false),
    fileName             (),
    nameOfLastEndTag     (),
    tokenStream          (_tokenStream),
    weOwnTokenStream     (false)
{
  tokenStream->DefineOperatorChars ("<>/");
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
  tokenStream = new Tokenizer (fileName);
  weOwnTokenStream = true;
  tokenStream->DefineOperatorChars ("<>/");
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

  if  (*t == "<")
  {
    delete  t;
    t = NULL;
    XmlTagPtr  tag = new XmlTag (tokenStream);
    if  (tag->TagType () == XmlTag::tagStart)
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

    else if  (tag->TagType () == XmlTag::tagEmpty)
    {
      XmlFactoryPtr  factory = XmlFactory::FactoryLookUp (tag->Name ());
      if  (!factory)
        factory = XmlElementKKStr::FactoryInstance ();
      endOfElemenReached = true;
      token = factory->ManufatureXmlElement (tag, *this, log);
      endOfElemenReached = false;
    }
    else if  (tag->TagType () == XmlTag::tagEnd)
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



XmlAttributeList::XmlAttributeList ()
{
}


XmlAttributeList::XmlAttributeList (const XmlAttributeList&  attributes)
{
  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin ();  idx != attributes.end ();  ++idx)
    insert (*idx);
}



KKStrConstPtr  XmlAttributeList::LookUp (const KKStr&  name)  const
{
  XmlAttributeList::const_iterator  idx;
  idx = find (name);
  if  (idx == end ())
    return NULL;
  else
    return &(idx->second);
}

 


void  XmlAttributeList::AddAttribute (const KKStr&  name,
                                      const KKStr&  value
                                     )
{
  XmlAttributeList::iterator  idx;
  idx = find (name);
  if  (idx == end ())
    insert (pair<KKStr,KKStr> (name, value));
  else
    idx->second = value;
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

  // Skip over lading spaces
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



XmlTag::XmlTag (istream&  i)
{
  tagType = tagNULL;

  if  (i.peek () == '<')
    i.get ();

  KKStr tagStr (100);
  ReadWholeTag (i, tagStr);

  if  (tagStr.FirstChar () == '/')
  {
    tagStr.ChopFirstChar ();
    tagType = tagEnd;
  }

  if  (tagStr.EndsWith ("/>"))
  {
    tagType = tagEmpty;
    tagStr.ChopLastChar ();
    tagStr.ChopLastChar ();
  }

  else if  (tagStr.LastChar () != '>')
  {
    tagType = tagStart;
  }

  else
  {
    if  (tagType == tagNULL)
      tagType = tagStart;
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
      attributes.insert (pair<KKStr, KKStr> (attributeName, attributeValue));
  }
}



XmlTag::XmlTag (TokenizerPtr  tokenStream):
     tagType (tagNULL)
{
  // We are assumed to be at the beginning of a new tag.
  KKStrListPtr  tokens = tokenStream->GetNextTokens (">");
  if  (!tokens)
    return;

  KKStrPtr t = NULL;


  if  (tokens->QueueSize () < 1)
  {
    delete  tokens;
    return;
  }

  if  ((*tokens)[0] == "<")
  {
    t = tokens->PopFromFront ();
    delete  t;
    t = NULL;
  }

  if  (tokens->QueueSize () < 1)
  {
    // We have a empty tag with no name.
    delete  tokens;
    tokens = NULL;
    return;
  }


  if  (*(tokens->BackOfQueue ()) == ">")
  {
    t = tokens->PopFromBack ();
    delete  t;
    t = NULL;
  }

  if  (tokens->QueueSize () < 1)
  {
    // We have a empty tag with no name.
    delete  tokens;
    tokens = NULL;
    return;
  }

  if  (tokens->QueueSize () > 1)
  {
    if  (*(tokens->BackOfQueue ()) == "/")
    {
      tagType = tagEmpty;
      t = tokens->PopFromBack ();
      delete  t;
      t = NULL;
    }
  }

  t = tokens->PopFromFront ();
  if  (*t == "/")
  {
    tagType = tagEnd;
    delete  t;
    t = tokens->PopFromFront ();
  }

  if  (t)
  {
    // At this point "t" should be the name of the tag.
    name = *t;
    delete  t;
    t = NULL;

    KKStrPtr  t1 = tokens->PopFromFront ();
    KKStrPtr  t2 = tokens->PopFromFront ();
    KKStrPtr  t3 = tokens->PopFromFront ();

    // Everything else should be attribute pairs  (Name = value).
    while  (t3 != NULL)
    {
      if  (*t2 != "=")
      {
        delete  t1;
        t1 = t2;
        t2 = t3;
        t3 = tokens->PopFromFront ();
      }
      else
      {
        if  (attributes.LookUp (*t) == NULL)
          attributes.insert (pair<KKStr, KKStr>(*t1, *t3));
        delete  t1;
        delete  t2;
        delete  t3;
        t1 = tokens->PopFromFront ();
        t2 = tokens->PopFromFront ();
        t3 = tokens->PopFromFront ();
      }
    }
    delete  t;   t  = NULL;
    delete  t2;  t2 = NULL;
    delete  t3;  t3 = NULL; 
  }

  delete  tokens;  tokens = NULL;
}  /* XmlTag::XmlTag (TokenizerPtr  tokenStream) */




XmlTag::XmlTag (const KKStr&  _name,
                TagTypes      _tagType
               ):
   name    (_name),
   tagType (_tagType)
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



KKStrConstPtr  XmlTag::AttributeValue (const KKStr& attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */




KKStrConstPtr  XmlTag::AttributeValue (const char* attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */




void  XmlTag::WriteXML (ostream& o)
{
  o << "<";
  
  if  (tagType == tagEnd)
    o << "/";

  o << name;

  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin();  idx != attributes.end ();  ++idx)
    o << " " << idx->first << "=" << idx->second.QuotedStr ();

  if  (tagType == tagEmpty)
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


XmlElementInt32::XmlElementInt32 (XmlTagPtr   tag,
                                  XmlStream&  s,
                                  RunLog&     log
                                 ):
    XmlElement (tag, s, log)
{
  KKStrConstPtr  valueStr = tag->AttributeValue ("Value");
  if  (valueStr)
  value = valueStr->ToInt32 ();
  XmlTokenPtr t = s.GetNextToken (log);
  while  (t != NULL)
  {
    if  (t->TokenType () == XmlToken::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      value = c->Content ()->ToInt32 ();
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}




//XmlFactoryMacro(XmlInt32Factory,XmlElementInt32,"Int32")
XmlFactoryMacro(Int32)





XmlElementVectorInt32::XmlElementVectorInt32 (XmlTagPtr   tag,
                                              XmlStream&  s,
                                              RunLog&     log
                                             ):
  XmlElement (tag, s, log)
{
  kkint32  count = 0;
  KKStrConstPtr  countStr = tag->AttributeValue ("Count");
  if  (countStr)
    count = countStr->ToInt32 ();

  value = new VectorInt32 ();

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);

      KKStrParser p (c->Content ());

      while  (p.MoreTokens ())
      {
        kkint32  zed = p.GetNextTokenInt ("\t\n\r,");
        value->push_back (zed);
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}
                



XmlElementVectorInt32::~XmlElementVectorInt32 ()
{
  delete  value;
  value = NULL;
}



VectorInt32*  const  XmlElementVectorInt32::Value ()  const
{
  return value;
}


VectorInt32*  XmlElementVectorInt32::TakeOwnership ()
{
  VectorInt32*  v = value;
  value = NULL;
  return v;
}


XmlFactoryMacro(VectorInt32)




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
    if  (t->TokenType () == XmlToken::tokContent)
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
    if  (t->TokenType () == XmlToken::tokContent)
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
  XmlTag t ("VectorKKStr", XmlTag::tagStart);
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

  XmlTag  endTag ("VectorKKStr", XmlTag::tagEnd);
  endTag.WriteXML (o);
}  /* WriteXml */


XmlFactoryMacro(VectorKKStr)






XmlElementKKStrListIndexed::XmlElementKKStrListIndexed (XmlTagPtr   tag,
                                                        XmlStream&  s,
                                                        RunLog&     log
                                                       ):
  XmlElement (tag, s, log),
  value (NULL)
{
  bool  caseSensative = tag->AttributeValue ("CaseSensative")->ToBool ();
  value = new KKStrListIndexed (true, caseSensative);
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*c->Content ());

      while  (p.MoreTokens ())
      {
        KKStr  s = p.GetNextToken (",\t\n\r");
        value->Add (s);
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
  XmlTag  startTag ("KKStrListIndexed", XmlTag::tagStart);

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

  XmlTag endTag ("KKStrListIndexed", XmlTag::tagEnd);
  endTag.WriteXML (o);
}


XmlFactoryMacro(KKStrListIndexed)