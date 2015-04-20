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
#include <vector>

#include "MemoryDebug.h"

using namespace std;


#include "BitString.h"
#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "Tokenizer.h"
using namespace KKB;


#include "XmlStream.h"



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

 


XmlStream::XmlStream (TokenizerPtr _tokenStream):
  tokenStream (_tokenStream)
{
}



XmlStream::~XmlStream ()
{
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




KKStrConstPtr  XmlTag::AttributeValue (const KKStr& attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */




KKStrConstPtr  XmlTag::AttributeValue (const char* attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */




XmlToken::XmlToken (TokenTypes  _tokenType):
  tokenType (_tokenType)
{
}



XmlElement::XmlElement (XmlTagPtr  _nameTag,
                        void*      _body
                      ):
  XmlToken (tokElement),
  nameTag (_nameTag),
  body    (_body)
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








map<KKStr, XmlStream::Factory*>*  factories = NULL;



XmlStream::Factory*  XmlStream::FactoryLookUp (const KKStr&  className)
{
  GlobalGoalKeeper::StartBlock ();
  if  (factories == NULL)
  {
    factories = new map<KKStr, Factory*> ();
    atexit (FinalCleanUp);
  }

  Factory*  factory = NULL;

  map<KKStr, Factory*>::const_iterator  idx;
  idx = factories->find (className);
  if  (idx != factories->end ())
    factory = idx->second;

  GlobalGoalKeeper::EndBlock ();

  return  factory;
}  /* FactoryLookUp */



void   XmlStream::RegisterFactory  (Factory*  factory)
{
  if  (!factory)
  {
    KKStr  errMsg = "XmlStream::RegisterFactory   ***ERROR***   (factory == NULL).";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  GlobalGoalKeeper::StartBlock ();
  Factory*  existingFactory = FactoryLookUp (factory->ClassName ());
  if  (existingFactory)
  {
    cerr << endl
         << "XmlStream::RegisterFactory   ***ERROR***   Factory[" << factory->ClassName () << "] already exists." << endl
         << endl;
  }
  else
  {
    factories->insert (pair<KKStr, Factory*> (factory->ClassName (), factory));
  }
  GlobalGoalKeeper::EndBlock ();
}



void  XmlStream::FinalCleanUp ()
{
  if  (factories)
  {
    map<KKStr, Factory*>::iterator  idx;
    for  (idx = factories->begin ();  idx != factories->end ();  ++idx)
      delete  idx->second;

    delete  factories;
    factories = NULL;
  }
}



XmlStream::Factory::Factory (const KKStr&  _clasName):
   className (_clasName)
{
}



class  XmlInt32Factory: public XmlStream::Factory
{
  XmlInt32Factory (): Factory ("Int32") {}

  void*  ManufatureInstance (const XmlTag&  tag,
                             XmlStream&     s,
                             RunLog&        log
                            )
  {
    kkint32 v = 0;
    KKStrConstPtr  valueStr = tag.AttributeValue ("Value");
    if  (valueStr)
      v = valueStr->ToInt32 ();

    if  (tag.TagType () != XmlTag::tagEmpty)
    {

      XmlTokenPtr t = s.GetNextToken (log);
      while  (t != NULL)
      {
        if  (t->TokenType () == XmlToken::tokContent)
        {
          XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
          v = c->Content ().ToInt32 ();
        }
      }
    }

    return new kkint32 (v);
  }

  static  XmlInt32Factory*  FactoryInstance ();
  static  XmlInt32Factory*  factoryInstance;
};



XmlInt32Factory*  XmlInt32Factory::FactoryInstance ()
{
  if  (factoryInstance == NULL)
  {
    GlobalGoalKeeper::StartBlock ();
    if  (!factoryInstance)
    {
      factoryInstance = new XmlInt32Factory ();
      XmlStream::RegisterFactory (factoryInstance);
    }

    GlobalGoalKeeper::EndBlock ();
  }
  return  factoryInstance;
}


XmlInt32Factory*  XmlInt32Factory::factoryInstance = NULL;




























