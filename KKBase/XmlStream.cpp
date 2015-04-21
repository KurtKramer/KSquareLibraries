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
#include "KKStrParser.h"
#include "Tokenizer.h"

#include "XmlStream.h"
using namespace KKB;





XmlStream::XmlStream (TokenizerPtr _tokenStream):
  tokenStream (_tokenStream)
{
}



XmlStream::~XmlStream ()
{
}



XmlTokenPtr  XmlStream::GetNextToken (RunLog&  log)
{
  XmlTokenPtr  token = NULL;

  if  (!tokenStream)
    return NULL;

  KKStrPtr  t = tokenStream->GetNextToken ();
  if  (t == NULL)
    return NULL;

  if  (*t != "<")
  {
    XmlTagPtr  tag = new XmlTag (tokenStream);
    if  ((tag->TagType () == XmlTag::tagStart)  ||  (tag->TagType () == XmlTag::tagEmpty))
    {
      XmlFactoryPtr  factory = XmlFactory::FactoryLookUp (tag->Name ());
      if  (!factory)
        factory = XmlElementKKStr::FactoryInstance ();
      token = factory->ManufatureXmlElement (tag, *this, log);
    }
    else
    {
      token = new 

    }
  }
}  /* GetNextToken */

 

XmlElementPtr  XmlStream::GetNextElement (RunLog&  log)
{
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
  KKStrPtr  c = this->content}






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

  if  (tag->TagType () != XmlTag::tagEmpty)
  {
    XmlTokenPtr t = s.GetNextToken (log);
    while  (t != NULL)
    {
      if  (t->TokenType () == XmlToken::tokContent)
      {
        XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
        value = c->Content ().ToInt32 ();
      }

      delete  t;
      t = NULL;
    }
  }
}



class  XmlInt32Factory: public XmlFactory
{
  XmlInt32Factory (): XmlFactory ("Int32") {}

  virtual
  XmlElementInt32Ptr  ManufatureXmlElement (XmlTagPtr   tag,
                                            XmlStream&  s,
                                            RunLog&     log
                                           )
  {
    return new XmlElementInt32 (tag, s, log);
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
      XmlFactory::RegisterFactory (factoryInstance);
    }

    GlobalGoalKeeper::EndBlock ();
  }
  return  factoryInstance;
}

XmlInt32Factory*  XmlInt32Factory::factoryInstance =  XmlInt32Factory::FactoryInstance ();




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




class  XmlVectorInt32Factory: public XmlFactory
{
  XmlVectorInt32Factory (): XmlFactory ("VectorInt32") {}

  virtual
  XmlElementVectorInt32Ptr  ManufatureXmlElement (XmlTagPtr   tag,
                                                  XmlStream&  s,
                                                  RunLog&     log
                                                )
  {
    return new XmlElementVectorInt32 (tag, s, log);
  }

  static  XmlVectorInt32Factory*  FactoryInstance ();
  static  XmlVectorInt32Factory*  factoryInstance;
};



XmlVectorInt32Factory*  XmlVectorInt32Factory::FactoryInstance ()
{
  if  (factoryInstance == NULL)
  {
    GlobalGoalKeeper::StartBlock ();
    if  (!factoryInstance)
    {
      factoryInstance = new XmlVectorInt32Factory ();
      XmlFactory::RegisterFactory (factoryInstance);
    }

    GlobalGoalKeeper::EndBlock ();
  }
  return  factoryInstance;
}

XmlVectorInt32Factory*  XmlVectorInt32Factory::factoryInstance = XmlVectorInt32Factory::FactoryInstance ();









class  XmlKKStrFactory: public XmlFactory
{
public:
  XmlKKStrFactory (): XmlFactory ("KKStr") {}

  virtual
  XmlElementKKStrPtr  ManufatureXmlElement (XmlTagPtr   tag,
                                            XmlStream&  s,
                                            RunLog&     log
                                           )
  {
    return new XmlElementKKStr (tag, s, log);
  }

  static  XmlKKStrFactory*  FactoryInstance ();
  static  XmlKKStrFactory*  factoryInstance;
};



XmlKKStrFactory*  XmlKKStrFactory::FactoryInstance ()
{
  if  (factoryInstance == NULL)
  {
    GlobalGoalKeeper::StartBlock ();
    if  (!factoryInstance)
    {
      factoryInstance = new XmlKKStrFactory ();
      XmlFactory::RegisterFactory (factoryInstance);
    }

    GlobalGoalKeeper::EndBlock ();
  }
  return  factoryInstance;
}

XmlKKStrFactory*  XmlKKStrFactory::factoryInstance = XmlKKStrFactory::FactoryInstance ();






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


XmlFactoryPtr  XmlElementKKStr::FactoryInstance ()
{
  return  XmlKKStrFactory::FactoryInstance ();
}










