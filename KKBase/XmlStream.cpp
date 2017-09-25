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



XmlStream::XmlStream (const KKStr& _fileName,  RunLog& _log):
    endOfElementTagNames (),
    endOfElemenReached   (false),
    fileName             (_fileName),
    nameOfLastEndTag     (),
    tokenStream          (NULL),
    weOwnTokenStream     (false)
{
  _log.Level(30) << "XmlStream::XmlStream   _fileName[" << _fileName << "]" << endl;
  bool fileOpened = false;
  tokenStream = new XmlTokenizer (fileName, fileOpened);
  weOwnTokenStream = true;
}



XmlStream::~XmlStream ()
{
  delete tokenStream;
  tokenStream = NULL;
}



/** @brief  Registers a Factory at the current hierarchy that is being processed. */
void  XmlStream::RegisterFactory (XmlFactoryPtr  factory)
{
  if  (factoryManagers.size () > 0)
    factoryManagers.back ()->RegisterFactory (factory);
  else
    XmlFactory::RegisterFactory (factory);

}  /* RegisterFactory */



void  XmlStream::PushXmlElementLevel (const KKStr&  sectionName)
{
  endOfElementTagNames.push_back (sectionName);
  factoryManagers.push_back (new XmlFactoryManager (sectionName));
}  /* PushNewXmlElementLevel */



void  XmlStream::PopXmlElementLevel ()
{
  if  ((endOfElementTagNames.size () < 1)  ||  (factoryManagers.size () < 1))
    return;

   endOfElementTagNames.pop_back ();
   XmlFactoryManagerPtr  fm = factoryManagers.back ();
   factoryManagers.pop_back ();
   delete  fm;
   fm = NULL;
}


XmlFactoryPtr  XmlStream::TrackDownFactory (const KKStr&  sectionName)
{
  XmlFactory*  result = NULL;
  kkuint32  level = (kkuint32)factoryManagers.size ();
  while  ((level > 0)  &&  (result == NULL))
  {
    --level;
    result = factoryManagers[level]->FactoryLookUp (sectionName);
  }

  if  (result == NULL)
    result = XmlFactory::FactoryLookUp (sectionName);
  return  result;
}


bool XmlStream::EndOfStream ()
{
  return (!tokenStream)  ||  (tokenStream->EndOfFile ());
}


XmlTokenPtr  XmlStream::GetNextToken (VolConstBool&  cancelFlag,
                                      RunLog&        log
                                     )
{
  if  (endOfElemenReached  || cancelFlag)
    return NULL;

  XmlTokenPtr  token = NULL;

  KKStrPtr   t = tokenStream->GetNextToken ();
  if  (t == NULL)
    return NULL;

  if  (t->FirstChar () == '<')
  {
    XmlTagPtr  tag = new XmlTag (t);
    delete  t;
    t = NULL;
    if  (tag->TagType () == XmlTag::TagTypes::tagStart)
    {
      XmlFactoryPtr  factory = TrackDownFactory (tag->Name ());
      if  (!factory)
        factory = XmlElementUnKnownFactoryInstance  ();

      log.Level (50) << "XmlStream::GetNextToken   Factory Selected: " << factory->ClassName () << endl;

      PushXmlElementLevel (tag->Name ());
      token = factory->ManufatureXmlElement (tag, *this, cancelFlag, log);
      KKStr  endTagName = endOfElementTagNames.back ();
      PopXmlElementLevel ();
      endOfElemenReached = false;
    }

    else if  (tag->TagType () == XmlTag::TagTypes::tagEmpty)
    {
      XmlFactoryPtr  factory = XmlFactory::FactoryLookUp (tag->Name ());
      if  (!factory)
        factory = XmlElementUnKnownFactoryInstance  ();
      PushXmlElementLevel (tag->Name ());
      endOfElemenReached = true;
      token = factory->ManufatureXmlElement (tag, *this, cancelFlag, log);
      if  ((!endOfElemenReached)  &&  (tag->TagType () == XmlTag::TagTypes::tagStart))
      {
        //  The element that we just read did not finish consuming all its components.
        log.Level (-1) << "XmlStream::GetNextToken   ***WARNING***   The element just read[" << tag->Name () << "]  Did not consume all its elements." << endl;
        XmlTokenPtr  t = GetNextToken (cancelFlag, log);
        while  (t)
        {
          delete t;
          t = GetNextToken (cancelFlag, log);
        }
      }
      endOfElemenReached = false;
      PopXmlElementLevel ();
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
            << "XmlStream::GetNextToken   ***ERROR***   " 
            << "Encountered end-tag </" << nameOfLastEndTag << ">  " 
            << "does not match StartTag <" << endOfElementTagNames.back () << ">." 
            << endl << endl;
          // </End-Tag>  does not match <Start-Tag>  we will put back on token stream assuming that we are missing a </End-Tag>
          // We will end the current element here.
          tokenStream->PushTokenOnFront (new KKStr ("<" + tag->Name () + " />"));
        }
      }
      delete  tag;
      tag = NULL;
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
  if  (!tokenStream) {
    log.Level(50) << "XmlStream::GetNextContent  tokenStream undefined." << endl;
    return NULL;
  }

  KKStrConstPtr  peekNext = tokenStream->Peek (0);
  if  (!peekNext)
    return NULL;

  if  (peekNext->FirstChar () == '<')
    return NULL;

  else
  {
    KKStrPtr ts = tokenStream->GetNextToken ();
    if  (ts)
      return  new XmlContent (ts);
    else
      return  NULL;
  }
}




XmlAttributeList::XmlAttributeList (bool  _owner):
  KKQueue<XmlAttribute> (_owner)
{
}


XmlAttributeList::XmlAttributeList (const XmlAttributeList&  attributes):
    KKQueue<XmlAttribute> (attributes)
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
    return &(a->Value ());
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
  if  (!s)
    return  0;
  else
    return s->ToInt32 ();
}



DateTime  XmlAttributeList::AttributeValueDateTime  (const KKStr&  name)   const
{
  KKStrConstPtr s = AttributeValueByName (name);
  if  (!s)
    return  DateTime ();
  
  DateTime  dt (*s);
  return  dt;
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
  KKStrParser parser(*tagStr);
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
   name       (_name),
   tagType    (_tagType)
{
}


XmlTag::~XmlTag ()
{
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           const KKStr&  attributeValue
                          )
{
  attributes.AddAttribute (attributeName, attributeValue);
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           bool          attributeValue
                          )
{
  attributes.AddAttribute (attributeName, (attributeValue ? "Yes" : "No"));
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           kkint32       attributeValue
                         )
{
  attributes.AddAttribute (attributeName, StrFromInt32 (attributeValue));
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           kkint64       attributeValue
                         )
{
  attributes.AddAttribute (attributeName, StrFromInt64 (attributeValue));
}



void  XmlTag::AddAtribute (const KKStr&  attributeName,
                           double        attributeValue
                          )
{
  KKStr  s (20);
  s << attributeValue;
  attributes.AddAttribute (attributeName, s);
}



void  XmlTag::AddAtribute (const KKStr&     attributeName,
                           const DateTime&  attributeValue
                          )
{
  attributes.AddAttribute (attributeName,  attributeValue.YYYY_MM_DD_HH_MM_SS ());
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


DateTime  XmlTag::AttributeValueDateTime (const KKStr&  attributeName)   const
{
 return  this->attributes.AttributeValueDateTime  (attributeName);
}


const KKStr&  XmlTag::AttributeValueKKStr (const KKStr& attributeName)  const
{
 return  attributes.AttributeValueKKStr  (attributeName);
}


KKStr  XmlTag::ToString ()  const
{
  KKStr  s (name.Len () + attributes.size () * 30);
  s << "<";
  
  if  (tagType == TagTypes::tagEnd)
    s << "/";

  s << name;

  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin();  idx != attributes.end ();  ++idx)
  {
    XmlAttributePtr  a = *idx;
    s << " " <<  a->Name () << "=" << a->Value ().QuotedStr ();
  }

  if  (tagType == TagTypes::tagEmpty)
    s << "/";

   s << ">";

   return  s;
}


kkint32  xmlLevel = 0;


void  XmlTag::WriteXML (std::ostream& o)
{
  if  (TagType () == TagTypes::tagEnd)
    --xmlLevel;

  for  (kkint32 x = 0;  x < xmlLevel;  ++x)
    o << "  ";
  o << ToString ();

  if  (this->TagType () == TagTypes::tagStart)
    ++xmlLevel;
}  /* WriteXML */





XmlToken::XmlToken ()
{
}


XmlToken::~XmlToken ()
{
}


XmlElement::XmlElement (const KKStr&      sectionName,
                        XmlTag::TagTypes  tagType
                       ):
  nameTag (new XmlTag (sectionName, tagType))
{}
 


XmlElement::XmlElement (XmlTagPtr   _nameTag,
                        XmlStream&  s,
                        RunLog&     log
                       ):
  nameTag (_nameTag)
{
  if  (s.EndOfStream ())
    log.Level(-1) << ("XmlElement::XmlElement   Reached EndOfStream.");
}
 


XmlElement ::~XmlElement ()
{
  delete  nameTag;
  nameTag = NULL;
}



KKStr  XmlElement::NameTagStr ()  const
{
  if  (!nameTag)
    return KKStr::EmptyStr ();
  else
    return nameTag->ToString ();
}



const KKStr&  XmlElement::SectionName ()  const
{
  if  (nameTag)
    return nameTag->Name ();
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


void  XmlContent::WriteXml (const KKStr&   s,
                            std::ostream&  o
                           )
{
  const char*  str = s.Str ();
  kkuint32 len = s.Len ();

  for  (kkuint32 x = 0;  x < len;  ++x)
  {
    char ch = str[x];
    switch  (ch)
    {
    case  '&':  o << "&amp;";  break;
    case  '<':  o << "&lt;";   break;
    case  '>':  o << "&gt;";   break;
    default:    o << ch;       break;
    }
  }
}  /* WriteXML */



XmlFactoryManagerPtr   XmlFactory::globalXmlFactoryManager = NULL;



XmlFactory*  XmlFactory::FactoryLookUp (const KKStr&  className)
{
  GlobalGoalKeeper::StartBlock ();
  if  (globalXmlFactoryManager == NULL)
  {
    globalXmlFactoryManager = new XmlFactoryManager ("globalXmlFactoryManager");
    atexit (FinalCleanUp);
  }

  XmlFactory*  factory = globalXmlFactoryManager->FactoryLookUp (className);

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

  if  (!globalXmlFactoryManager)
  {
    globalXmlFactoryManager = new XmlFactoryManager ("globalXmlFactoryManager");
    atexit (FinalCleanUp);
  }

  globalXmlFactoryManager->RegisterFactory (factory);

  GlobalGoalKeeper::EndBlock ();
}



void  XmlFactory::FinalCleanUp ()
{
  if  (globalXmlFactoryManager)
    delete  globalXmlFactoryManager;
  globalXmlFactoryManager = NULL;
}




XmlFactory::XmlFactory (const KKStr&  _clasName):
   className (_clasName)
{
}




XmlFactoryManager::XmlFactoryManager (const KKStr&  _name):
  factories (),
  name      (_name)
{
}


XmlFactoryManager::~XmlFactoryManager ()
{
  map<KKStr, XmlFactory*>::iterator  idx;
  for  (idx = factories.begin ();  idx != factories.end ();  ++idx)
  {
    XmlFactory*  f = idx->second;
    delete  f;
  }
  factories.clear ();
}



void   XmlFactoryManager::RegisterFactory  (XmlFactory*  factory)
{
  GlobalGoalKeeper::StartBlock ();
  XmlFactory*  existingFactory = FactoryLookUp (factory->ClassName ());
  if  (existingFactory)
  {
    GlobalGoalKeeper::EndBlock ();
    KKStr  errMsg (200);
    errMsg << "XmlFactoryManager::RegisterFactory  FactoryManager[" << name << "]  Factory[" << factory->ClassName () << "] already exists." ;
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  else
  {
    factories.insert (pair<KKStr, XmlFactory*> (factory->ClassName (), factory));
  }
  GlobalGoalKeeper::EndBlock ();
}
      

XmlFactory*  XmlFactoryManager::FactoryLookUp (const KKStr&  className)  const
{
  GlobalGoalKeeper::StartBlock ();
  XmlFactory*  result = NULL;

  map<KKStr, XmlFactory*>::const_iterator  idx;
  idx = factories.find (className);
  if  (idx == factories.end ())
    result =  NULL;
  else
    result = idx->second;

  GlobalGoalKeeper::EndBlock ();
  return  result;
}  /* FactoryLookUp */




XmlElementBool::XmlElementBool (XmlTagPtr      tag,
                                XmlStream&     s,
                                VolConstBool&  cancelFlag,
                                RunLog&        log
                               ):
    XmlElement (tag, s, log),
    value (false)
{
  KKStrConstPtr  valueStr = tag->AttributeValueByName ("Value");
  if  (valueStr)
    value = valueStr->ToBool ();
  XmlTokenPtr t = s.GetNextToken (cancelFlag, log);
  while  (t != NULL)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      value = c->Content ()->ToBool ();
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
}


XmlElementBool::~XmlElementBool ()
{
}


bool  XmlElementBool::Value ()  const
{
  return  value;
}



void  XmlElementBool::WriteXML (const bool     b,
                                const KKStr&   varName,
                                std::ostream&  o
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


XmlElementUnKnown::XmlElementUnKnown (XmlTagPtr      tag,
                                      XmlStream&     s,
                                      VolConstBool&  cancelFlag,
                                      RunLog&        log
                                     ):
    XmlElement (tag, s, log),
    value (new deque<XmlTokenPtr> ())
{
  XmlTokenPtr t = s.GetNextToken (cancelFlag, log);
  while  (t != NULL)
  {
    value->push_back (t);
    t = s.GetNextToken (cancelFlag, log);
  }
}
                

XmlElementUnKnown::~XmlElementUnKnown ()
{
  if  (value)
  {
    for  (auto idx: *value)
      delete  idx;
    delete  value;
    value = NULL;
  }
}


deque<XmlTokenPtr>*  XmlElementUnKnown::TakeOwnership ()
{
  deque<XmlTokenPtr>* v = value;
  value = NULL;
  return v;
}



XmlFactoryMacro(UnKnown)

XmlFactoryPtr  KKB::XmlElementUnKnownFactoryInstance ()
{
  return  XmlFactoryUnKnown::FactoryInstance ();
}



XmlElementDateTime::XmlElementDateTime (XmlTagPtr      tag,
                                        XmlStream&     s,
                                        VolConstBool&  cancelFlag,
                                        RunLog&        log
                                       ):
    XmlElement (tag, s, log),
    value ()
{
  value = tag->AttributeValueDateTime ("Value");
  if  (cancelFlag)
    log.Level(10) << "XmlElementDateTime::XmlElementDateTime   ***CANCELED***" << endl;
}


               
XmlElementDateTime::~XmlElementDateTime ()  
{
}



void  XmlElementDateTime::WriteXML (const DateTime&  d,
                                    const KKStr&     varName,
                                    std::ostream&    o
                                   )
{
  XmlTag startTag ("DateTime", XmlTag::TagTypes::tagEmpty);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Value", d.YYYY_MM_DD_HH_MM_SS ());
  startTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(DateTime)






XmlElementKeyValuePairs::XmlElementKeyValuePairs ():
    XmlElement ("KeyValuePairs", XmlTag::TagTypes::tagStart),
    value (new vector<pair<KKStr,KKStr> > ())
{
}


XmlElementKeyValuePairs::XmlElementKeyValuePairs (XmlTagPtr      tag,
                                                  XmlStream&     s,
                                                  VolConstBool&  cancelFlag,
                                                  RunLog&        log
                                                 ):
    XmlElement (tag, s, log),
    value (NULL)
{
  value = new vector<pair<KKStr,KKStr> > ();

  XmlTokenPtr t = s.GetNextToken (cancelFlag, log);
  while  (t != NULL)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      if  (c)
      {
        KKStrParser p (*(c->Content ()));
        p.TrimWhiteSpace (" ");
        
        while  (p.MoreTokens ())
        {
          KKStr  key = p.GetNextToken ("\t\n");
          if  (p.LastDelimiter () == '\n')
            value->push_back (pair<KKStr,KKStr> (key, ""));
          else
            value->push_back (pair<KKStr,KKStr> (key, p.GetRestOfLine ()));
        }
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
}


XmlElementKeyValuePairs::~XmlElementKeyValuePairs ()
{
  delete  value;
  value = NULL;
}


vector<pair<KKStr,KKStr> >*  XmlElementKeyValuePairs::TakeOwnership ()
{
  vector<pair<KKStr,KKStr> >* v = value;
  value = NULL;
  return  v;
}


void  XmlElementKeyValuePairs::Add (const KKStr&  key,
                                    const KKStr&  v
                                   )
{
  value->push_back (pair<KKStr,KKStr> (key, v));
}




void  XmlElementKeyValuePairs::Add (const KKStr&  key,
                                    kkint32       v
                                   )
{
  Add (key, KKB::StrFromInt32 (v));
}



void  XmlElementKeyValuePairs::Add (const KKStr&  key,
                                    float         v
                                   )
{
  Add (key, KKB::StrFromFloat (v));
}




void  XmlElementKeyValuePairs::Add (const KKStr&  key,
                                    double        v
                                   )
{
  Add (key, KKB::StrFromDouble (v));
}




void  XmlElementKeyValuePairs::Add (const KKStr&  key,
                                    bool          v
                                   )
{
  KKStr  vStr = "False";
  if  (v)  vStr = "True";
  Add (key, vStr);
}



void  XmlElementKeyValuePairs::Add (const KKStr&          key,
                                    const KKB::DateTime&  v
                                   )
{
  Add (key, v.YYYY_MM_DD_HH_MM_SS ());
}



void  XmlElementKeyValuePairs::WriteXML (const KKStr&  varName,
                                         ostream&      o
                                        )
{
  XmlTag  startTag ("KeyValuePairs",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  vector<pair<KKStr,KKStr> >::const_iterator  idx;
  for  (idx = value->begin ();  idx != value->end ();  ++idx)
  {
    XmlContent::WriteXml (idx->first,  o);
    o << "\t";
    XmlContent::WriteXml (idx->second, o);
    o << endl;
  }

  XmlTag  endTag ("KeyValuePairs", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(KeyValuePairs)





XmlElementArrayFloat2DVarying::XmlElementArrayFloat2DVarying (XmlTagPtr      tag,
                                                              XmlStream&     s,
                                                              VolConstBool&  cancelFlag,
                                                              RunLog&        log          
                                                            ):
    XmlElement (tag, s, log),
    height (0),
    value  (NULL),
    widths (NULL)
{
  height = tag->AttributeValueInt32 ("Height");
  if  (height < 1)
  {
    log.Level (-1) << endl 
      << "XmlElementArrayFloat2DVarying   ***ERROR***   Height[" << height << "] must be greater than 0." << endl 
      << endl;
    height = 0;
    return;
  }
  value  = new float*[height];
  widths = new kkuint32[height];

  XmlTokenPtr  tok = s.GetNextToken (cancelFlag, log);
  kkuint32  rowCount = 0;
  while  (tok)
  {
    if  (typeid(*tok) == typeid(XmlElementArrayFloat))
    {
      if  (rowCount >= height)
      {
        log.Level (-1) << endl
          << "XmlElementArrayFloat2DVarying   ***ERROR***   Number of defined rows exceeds defined Height[" << height << "]" << endl
          << endl;
        widths[rowCount] = 0;
        value[rowCount]  = NULL;
      }
      else
      {
        XmlElementArrayFloatPtr f = dynamic_cast<XmlElementArrayFloatPtr>(tok);
        widths[rowCount] = f->Count ();
        value[rowCount] = f->TakeOwnership ();
      }
      ++rowCount;
    }
    delete  tok;
    tok = s.GetNextToken (cancelFlag, log);
  }
  while  (rowCount < height)
  {
    value[rowCount] = NULL;
    ++rowCount;
  }
}



XmlElementArrayFloat2DVarying::~XmlElementArrayFloat2DVarying ()
{
  if  (value)
  {
    for  (kkuint32 x = 0;  x < height;  ++x)
    {
      delete value[x];
      value[x] = NULL;
    }
  }
  delete  value;   value  = NULL;
  delete  widths;  widths = NULL;
}




float**   XmlElementArrayFloat2DVarying::TakeOwnership ()
{
  float** v = value;
  value = NULL;
  return v;
}


kkuint32*   XmlElementArrayFloat2DVarying::TakeOwnershipWidths ()
{
  kkuint32* w = widths;
  widths = NULL;
  return w;
}


                                      
void  XmlElementArrayFloat2DVarying::WriteXML 
               (kkuint32        height,
                const kkint32*  widths,      /**< Each entry in array defines the length of the corresponding row in 'mat'.  */ 
                float** const   mat,
                const KKStr&    varName,
                ostream&        o 
                )
{
  XmlTag startTag ("ArrayFloat2DVarying", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Height", (kkint32)height);
  startTag.WriteXML (o);
  o << endl;

  for  (kkuint32 r = 0;  r < height;  ++r)
  {
    KKStr rowName = "Row_" + StrFormatInt (r, "000");
    XmlElementArrayFloat::WriteXML (widths[r], mat[r], rowName, o);
  }

  XmlTag  endTag ("ArrayFloat2DVarying", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(ArrayFloat2DVarying)












XmlFactoryMacro(KKStr)

XmlFactoryPtr  KKB::XmlElementKKStrFactoryInstance ()
{
  return  XmlFactoryKKStr::FactoryInstance ();
}




XmlFactoryMacro(VectorKKStr)
XmlFactoryMacro(KKStrList)
XmlFactoryMacro(KKStrListIndexed)









/**
 * Works with matching Marco (XmlElementBuiltInTypeHeader) defined in XmlStream.h  
 * XmlElementBuiltInTypeBody(kkint32,Int32,ToInt32)  // XmlElementInt32
 */
#define  XmlElementBuiltInTypeBody(T,TypeName,ParseMethod)                \
 XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr      tag,          \
                                             XmlStream&     s,            \
                                             VolConstBool&  cancelFlag,   \
                                             RunLog&        log           \
                                            ):                            \
  XmlElement (tag, s, log),                                               \
  value ((T)0)                                                            \
{                                                                         \
  KKStrConstPtr  valueStr = tag->AttributeValueByName ("Value");          \
  if  (valueStr)                                                          \
    value = (T)(valueStr->ParseMethod ());                                \
  XmlTokenPtr tok = s.GetNextToken (cancelFlag, log);                     \
  while  (tok != NULL)                                                    \
  {                                                                       \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)           \
    {                                                                     \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);                \
      value = (T)c->Content ()->ParseMethod ();                           \
    }                                                                     \
    delete  tok;                                                          \
    tok = s.GetNextToken (cancelFlag, log);                               \
  }                                                                       \
}                                                                         \
                                                                          \
                                                                          \
XmlElement##TypeName::~ XmlElement##TypeName ()                           \
{                                                                         \
}                                                                         \
                                                                          \
                                                                          \
void   XmlElement##TypeName::WriteXML (T              d,                  \
                                       const KKStr&   varName,            \
                                       std::ostream&  o                   \
                                      )                                   \
{                                                                         \
  XmlTag startTag (#TypeName, XmlTag::TagTypes::tagEmpty);                \
  if  (!varName.Empty ())                                                 \
    startTag.AddAtribute ("VarName", varName);                            \
  startTag.AddAtribute ("Value", d);                                      \
  startTag.WriteXML (o);                                                  \
  o << endl;                                                              \
}                                                                         \
                                                                          \
                                                                          \
bool     XmlElement##TypeName::ToBool   () const {return value != 0;}     \
KKStr    XmlElement##TypeName::ToKKStr  () const {return (KKStr)value;}   \
double   XmlElement##TypeName::ToDouble () const {return (double)value;}  \
float    XmlElement##TypeName::ToFloat  () const {return (float)value;}   \
kkint32  XmlElement##TypeName::ToInt32  () const {return (kkint32)value;} \
                                                                          \
XmlFactoryMacro(TypeName)






#define  XmlElementArrayBody(T,TypeName,ParserNextTokenMethod)                \
XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr      tag,               \
                                            XmlStream&     s,                 \
                                            VolConstBool&  cancelFlag,        \
                                            RunLog&        log                \
                                          ):                                  \
      XmlElement (tag, s, log),                                               \
      count (0),                                                              \
      value (NULL)                                                            \
{                                                                             \
  count = tag->AttributeValueInt32 ("Count");                                 \
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
  XmlTokenPtr  tok = s.GetNextToken (cancelFlag, log);                        \
  while  (tok)                                                                \
  {                                                                           \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)               \
    {                                                                         \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);                    \
                                                                              \
      KKStrParser p (*(c->Content ()));                                       \
                                                                              \
      while  (p.MoreTokens ())                                                \
      {                                                                       \
        double  zed = p.ParserNextTokenMethod ("\t,");                        \
        if  (fieldsExtracted < count)                                         \
          value[fieldsExtracted] = (T)zed;                                    \
        ++fieldsExtracted;                                                    \
      }                                                                       \
    }                                                                         \
    delete  tok;                                                              \
    tok = s.GetNextToken (cancelFlag, log);                                   \
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
                                      std::ostream&  o                        \
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









/**
 *@details  Works with corresponding macro (XmlElementArray2DHeader) defined in header file "XmlStream.h"
 *@param  T  The Built-In type,  examples: float, kkint32, double, ...
 *@param  TypeName  The name we give the type  ex:  "Int32", "Float", ....
 *@param  XmlElementToUse  the XmlElement derived class that will be used to process each row in the matrix.
 */
#define  XmlElementArray2DBody(T,TypeName,XmlElementToUse)                     \
XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr      tag,                \
                                            XmlStream&     s,                  \
                                            VolConstBool&  cancelFlag,         \
                                            RunLog&        log                 \
                                          ):                                   \
      XmlElement (tag, s, log),                                                \
      height (0),                                                              \
      value (NULL),                                                            \
      width (0)                                                                \
{                                                                              \
  height = tag->AttributeValueInt32 ("Height");                                \
  if  (height <= 0)                                                            \
  {                                                                            \
    log.Level (-1) << endl                                                     \
      << "XmlElement" << #TypeName << "   ***ERROR***   Attribute Height["     \
      << height << "] must be a positive value; will set array to NULL."       \
      << endl << endl;                                                         \
      height = 0;                                                              \
  }                                                                            \
                                                                               \
  width = tag->AttributeValueInt32 ("Width");                                  \
  if  (width <= 0)                                                             \
  {                                                                            \
    log.Level (-1) << endl                                                     \
      << "XmlElement" << #TypeName << "   ***ERROR***   Attribute Width["      \
      << width << "] must be a positive value; will set array to NULL."        \
      << endl << endl;                                                         \
    width = 0;                                                                 \
  }                                                                            \
                                                                               \
  if  ((height <= 0)  || (width <= 0))                                         \
  {                                                                            \
    value = NULL;                                                              \
    height = 0;                                                                \
    width = 0;                                                                 \
  }                                                                            \
  else                                                                         \
  {                                                                            \
    value = new T*[width];                                                     \
  }                                                                            \
                                                                               \
  kkuint32  rowCount = 0;                                                      \
  XmlTokenPtr  tok = s.GetNextToken (cancelFlag, log);                         \
  while  (tok)                                                                 \
  {                                                                            \
    if  (typeid(*tok) != typeid(XmlElementToUse))                              \
    {                                                                          \
       log.Level (-1) << endl                                                  \
         << "XmlElement" << #TypeName << "   ***ERROR***   Element: "          \
         <<  tok->SectionName ()  << "  unexpected."                           \
         << endl << endl;                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      XmlElementToUse*  row = dynamic_cast<XmlElementToUse*> (tok);            \
      if  (row->Count () != width)                                             \
      {                                                                        \
        log.Level (-1) << endl                                                 \
          << "XmlElement" << #TypeName << "   ***ERROR***   Row Count[ "       \
          <<  row->Count () << " not equal to Width[" << width << "]."         \
          << endl << endl;                                                     \
      }                                                                        \
      else                                                                     \
      {                                                                        \
        if  (rowCount < height)                                                \
          value[rowCount] = row->TakeOwnership ();                             \
        ++rowCount;                                                            \
      }                                                                        \
    }                                                                          \
    delete  tok;                                                               \
    tok = s.GetNextToken (cancelFlag, log);                                    \
  }                                                                            \
                                                                               \
  if  (rowCount != height)                                                     \
  {                                                                            \
    log.Level (-1) << endl                                                     \
      << "XmlElement" << #TypeName << "   ***ERROR***   RowCount[ "            \
      << rowCount << "]  differs from specified height[" << height << "]."     \
      << endl << endl;                                                         \
  }                                                                            \
  while  (rowCount < height)                                                   \
  {                                                                            \
    value[rowCount] = new T[width];                                            \
    for (kkuint32 x = 0; x < width;  ++x)                                      \
        value[rowCount][x] = (T)0;                                             \
    ++rowCount;                                                                \
  }                                                                            \
}                                                                              \
                                                                               \
                                                                               \
XmlElement##TypeName::~XmlElement##TypeName ()                                 \
{                                                                              \
  if  (value)                                                                  \
  {                                                                            \
    for  (kkuint32 x = 0;  x < height;  ++x)                                   \
      delete value[x];                                                         \
  }                                                                            \
  delete  value;                                                               \
  value = NULL;                                                                \
}                                                                              \
                                                                               \
                                                                               \
T**   XmlElement##TypeName::TakeOwnership ()                                   \
{                                                                              \
  T** v = value;                                                               \
  value = NULL;                                                                \
  return v;                                                                    \
}                                                                              \
                                                                               \
                                                                               \
void  XmlElement##TypeName::WriteXML (kkuint32      height,                    \
                                      kkuint32      width,                     \
                                      T** const     mat,                       \
                                      const KKStr&  varName,                   \
                                      std::ostream&  o                         \
                                     )                                         \
{                                                                              \
  XmlTag startTag (#TypeName, XmlTag::TagTypes::tagStart);                     \
  if  (!varName.Empty ())                                                      \
    startTag.AddAtribute ("VarName", varName);                                 \
  startTag.AddAtribute ("Height", (kkint32)height);                            \
  startTag.AddAtribute ("Width",  (kkint32)width);                             \
  startTag.WriteXML (o);                                                       \
                                                                               \
  for  (kkuint32 r = 0;  r < height;  ++r)                                     \
  {                                                                            \
    KKStr vn = "Row_" + StrFormatInt (r, "0000");                              \
    XmlElementToUse::WriteXML (width, mat[r], vn, o);                          \
  }                                                                            \
  XmlTag  endTag (#TypeName, XmlTag::TagTypes::tagEnd);                        \
  endTag.WriteXML (o);                                                         \
  o << endl;                                                                   \
}                                                                              \
                                                                               \
XmlFactoryMacro(TypeName)









#define  XmlElementVectorBody(T,TypeName,ParserNextTokenMethod)            \
XmlElement##TypeName::XmlElement##TypeName (XmlTagPtr      tag,            \
                                            XmlStream&     s,              \
                                            VolConstBool&  cancelFlag,     \
                                            RunLog&        log             \
                                           ):                              \
  XmlElement (tag, s, log)                                                 \
{                                                                          \
  kkuint32  count = 0;                                               \
  KKStrConstPtr  countStr = tag->AttributeValueByName ("Count");           \
  if  (countStr)                                                           \
    count = countStr->ToUint32 ();                                          \
                                                                           \
  value = new vector<T> ();                                                \
                                                                           \
  XmlTokenPtr  tok = s.GetNextToken (cancelFlag, log);                     \
  while  (tok)                                                             \
  {                                                                        \
    if  (tok->TokenType () == XmlToken::TokenTypes::tokContent)            \
    {                                                                      \
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (tok);                 \
                                                                           \
      KKStrParser p (*(c->Content ()));                                    \
                                                                           \
      while  (p.MoreTokens ())                                             \
      {                                                                    \
        T  zed = p.ParserNextTokenMethod ("\t,");                          \
        value->push_back ((T)zed);                                         \
      }                                                                    \
    }                                                                      \
    delete  tok;                                                           \
    tok = s.GetNextToken (cancelFlag, log);                                \
  }                                                                        \
                                                                           \
  if  (count != value->size ())                                            \
  {                                                                        \
    log.Level(-1) <<"XmlElement##TypeName::XmlElement##TypeName  "         \
      << "***WARNING***   count[" << count << "] not equal number "        \
      << "elements value->size()[" << value->size () << "] actually "      \
      << "found.";                                                         \
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
                                      std::ostream&     o                  \
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
XmlElementBuiltInTypeBody(kkint32,Int32,ToInt32)  // XmlElementInt32

XmlElementBuiltInTypeBody(kkint64,Int64,ToInt64)  // XmlElementInt64

XmlElementBuiltInTypeBody(float,Float,ToDouble)   // XmlElementFloat

XmlElementBuiltInTypeBody(double,Double,ToDouble) // XmlElementDouble




// Arrays

XmlElementArrayBody(kkuint16, ArrayUint16,   GetNextTokenUint)     // XmlElementArrayUint16

XmlElementArrayBody(kkint32,  ArrayInt32,    GetNextTokenInt)      // XmlElementArrayInt32

XmlElementArrayBody(double,   ArrayDouble,   GetNextTokenDouble)   

XmlElementArrayBody(float,    ArrayFloat,    GetNextTokenDouble)


XmlElementArray2DBody(float, ArrayFloat2D, XmlElementArrayFloat)   // XmlElementArrayFloat2D



// Vectors

XmlElementVectorBody(kkint32,  VectorInt32,  GetNextTokenInt)
XmlElementVectorBody(float,    VectorFloat,  GetNextTokenFloat)
