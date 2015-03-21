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
#include "KKBaseTypes.h"
#include "KKException.h"
#include "Tokenizer.h"
using namespace KKB;


#include "XmlStream.h"


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
      attributes.push_back (XmlAttribute (attributeName, attributeValue));
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

  // We will assume that we have a start tag unless proven otherwise.
  tagType = tagStart;

  if  ((tokens->QueueSize () > 0)  &&  (*(tokens->BackOfQueue ()) == ">"))
  {
    t = tokens->PopFromBack ();
    delete t;
    t = NULL;

    if  ((tokens->QueueSize () > 0)  &&  (*(tokens->BackOfQueue ()) == "/"))
    {
      // We have a Empty tag.
      tagType = tagEmpty;
    }
  }

  if  ((tokens->QueueSize () > 0)  &&  ((*tokens)[0] == "/"))
  {
    // We have a Ending Tag
    tagType = tagEnd;
    t = tokens->PopFromBack ();
    delete t;
    t = NULL;

    if  (tokens->QueueSize () > 0)
      name = (*tokens)[0];

    delete  tokens;
    tokens = NULL;
    return;
  }


  if  (tokens->QueueSize () < 1)
  {
    delete  tokens;
    return;
  }


  // At this point the only thing in tokens should be Attribute Pairs.
  kkint32  idx = 0;
  name = (*tokens)[idx];
  idx++;

  KKStr  attrName = "";
  KKStr  attrValue = "";

  while  (idx < tokens->QueueSize ())
  {
    attrName = "";
    attrValue = "";

    attrName = (*tokens)[idx];  idx++;

    if  (idx < tokens->QueueSize ())
    {
      if  ((*tokens)[idx] == "=")
      {
        idx++;
        if  (idx < tokens->QueueSize ())
        {
          attrValue = (*tokens)[idx];
          idx++;
        }
      }

      attributes.push_back (XmlAttribute (attrName, attrValue));
    }

    if  ((idx < tokens->QueueSize ())  &&  ((*tokens)[idx] == ","))
      idx++;
  }

  delete  tokens;  tokens = NULL;
}  /* XmlTag::XmlTag (TokenizerPtr  tokenStream) */




const KKStr&  XmlTag::AttributeValue (const KKStr& attributeName)  const
{
  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin ();  idx != attributes.end ();  ++idx)
  {
    if  (idx->Name ().EqualIgnoreCase (attributeName))
      return idx->Value ();
  }

  return KKStr::EmptyStr ();
} /* AttributeValue */




const KKStr&  XmlTag::AttributeValue (const char* attributeName)  const
{
  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin ();  idx != attributes.end ();  ++idx)
  {
    if  (idx->Name ().EqualIgnoreCase (attributeName))
      return idx->Value ();
  }

  return KKStr::EmptyStr ();
} /* AttributeValue */




/**  
 *@brief  Will return either a XmlElement or a XmlContent
 */
XmlTokenPtr  XmlStream::GetNextToken ()  /*!< Will return either a XmlElement or a XmlContent */
{
  if  (tokenStream->EndOfFile ())
    return NULL;

  KKStrConstPtr  tokenStr = tokenStream->Peek (0);
  if  (tokenStr == NULL)
    return NULL;

  if  ((*tokenStr) == "<")
    return  ProcessElement ();
  else
    return new XmlContent (tokenStream);
}  /* GetNextToken */




XmlElementPtr  XmlStream::ProcessElement ()
{
  // We are assuming that we are at the very beginning of a new element.  In this case
  // the very next thing we get should be a tag field.

  XmlTagPtr tag = new XmlTag (tokenStream);
  if  (!tag)
    return  NULL;

  XmlElementCreator creator = LookUpXmlElementCreator (tag->Name ());
  if  (creator)
    return  creator (tag, *this);
  else
    return NULL;
}  /* ProcessElement */






XmlToken::XmlToken (TokenTypes  _tokenType):
  tokenType (_tokenType)
{
}





XmlElement::XmlElement ():
   XmlToken (tokElement)
{
}


XmlElement::XmlElement (TokenizerPtr  _tokenStream):
   XmlToken (tokElement)
{
}



XmlContent::XmlContent (TokenizerPtr  _tokenStream):
   XmlToken (tokContent)
{
}






map<KKStr, XmlStream::XmlElementCreator>  XmlStream::xmlElementCreators;



/**
 @brief Register a 'XmlElementCreator' function with its associated name.
 @details  if you try to register the same function with the same name will generate a warnming to 
           cerr.  If you try and register two different functions with the same name will throw 
           an exception.
 */
void   XmlStream::RegisterXmlElementCreator  (const KKStr&       elementName,
                                              XmlElementCreator  creator
                                             )
{
  map<KKStr, XmlElementCreator>::iterator  idx;
  idx = xmlElementCreators.find (elementName);
  if  (idx != xmlElementCreators.end ())
  {
    // A 'XmlElementCreator'  creator already exists with name 'elementName'.
    if  (idx->second == creator)
    {
      // Trying to regisetr the same function,  No harm done.
      cerr << std::endl
           << "XmlStream::RegisterXmlElementCreator   ***WARNING***   trying to register[" << elementName << "] Creator more than once." << std::endl
           << std::endl;
      return;
    }
    else
    {
      // Trying to register two different 'XmlElementCreator' functions.  This is VERY VERY bad.
      KKStr  errMsg = "XmlStream::RegisterXmlElementCreator   ***WARNING***   Trying to register[" + elementName + "] as two different Creator functions.";
      cerr << std::endl << errMsg << std::endl;
      throw KKException (errMsg);
    }
  }

  xmlElementCreators.insert (pair<KKStr,XmlElementCreator>(elementName, creator));
}  /* RegisterXmlElementCreator */



XmlStream::XmlElementCreator  XmlStream::LookUpXmlElementCreator (const KKStr&  elementName)
{
  map<KKStr, XmlElementCreator>::iterator  idx;
  idx = xmlElementCreators.find (elementName);
  if  (idx == xmlElementCreators.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUpXmlElementCreator */
