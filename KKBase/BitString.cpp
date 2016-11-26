/* BitString.cpp -- Bit String management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;

#include "BitString.h"
#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "XmlStream.h"
using namespace KKB;


// Bit                                  0      1      2      3       4       5       6        7
uchar  BitString::bitMasks[8]    = {    1,     2,     4,     8,     16,     32,     64,     128};

uchar  BitString::bitMasksRev[8] = {255-1, 255-2, 255-4, 255-8, 255-16, 255-32, 255-64, 255-128};

uchar* BitString::bitCounts = NULL;


/**
 *@brief  Initializes static array 'bitCounts' which maintains the number of bits in the binary representation of 0 - 255.
 *@details  The purpose of 'bitCounts' is to make the computation of total number of bits in a bit string stored in 'str' as fast as possible.
 */
void  BitString::BuildBitCounts ()
{
  if  (bitCounts)
    return;

  bitCounts = new uchar[256];

  kkint32  byte = 0;
  for  (byte = 0;  byte < 256;  byte++)
  {
    bitCounts[byte] = 0;
    kkint32  x = byte;
    while  (x > 0)
    {
      if  ((x % 2) == 1)
        bitCounts[byte]++;
      x = x / 2;
    }
  }
}  /* BuildBitCounts */



BitString::BitString ():
  bitLen  (0),
  byteLen (0)
{
  byteLen = ((bitLen - 1) / 8) + 1;
  str = NULL;
}



BitString::BitString (kkuint32  _bitLen):
  bitLen  (_bitLen),
  byteLen (0)
{
  byteLen = ((bitLen - 1) / 8) + 1;
  str = new uchar[byteLen];
  memset (str, 0, (size_t)byteLen);
}


BitString::BitString (const BitString&  bs):
  bitLen  (bs.bitLen),
  byteLen (bs.byteLen),
  str     (NULL)
{
  str = new uchar[byteLen];
  memcpy (str, bs.str, (size_t)byteLen);
}


BitString::BitString (kkuint32   _bitLen,
                      kkuint16*  bitNums,
                      kkuint32   bitNumsLen
                     ):
    bitLen  (_bitLen),
    byteLen (0),
    str     (NULL)
{
  byteLen = ((bitLen - 1) / 8) + 1;
  str = new uchar[byteLen];
  memset (str, 0, (size_t)byteLen);
  kkuint32  x;
  for  (x = 0;  x < bitNumsLen;  x++)
  {
    if  (bitNums[x] >= bitLen)
    {
      KKStr  msg (128);
      msg << "BitString   Constructing from list of numbers:  bitNums[" << x << "] = [" << bitNums[x] << "] which is >= bitLen[" << bitLen << "].";
      cerr << std::endl << std::endl << "BitString   ***ERROR***  " << msg << std::endl << std::endl;
      throw KKException (msg);
    }
    Set (bitNums[x]);
  }
}



BitString::~BitString ()
{
  delete  str;
  str = NULL;
}



BitString*  BitString::Duplicate ()  const
{
  return new BitString (*this);
}



void  BitString::CalcByteAndBitOffsets (kkuint32  bitNum,
                                        kkint32&  byteOffset,
                                        uchar&    bitOffset
                                       )  const
{
  byteOffset = bitNum / 8;
  bitOffset  = bitNum % 8;
} /* CalcByteAndBitOffsets */



kkuint32  BitString::Count  ()  const
{
  BuildBitCounts ();

  kkuint32  count = 0;
  kkuint32  byteOffset = 0;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
    count += bitCounts[str[byteOffset]];

  return  count;
}  /* Count */




bool  BitString::Test (kkuint32  bitNum) const
{
  kkint32 byteOffset;
  uchar   bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  bool  bit = ((str[byteOffset] & bitMasks[bitOffset]) != 0);

  return bit;
}  /* IsSet */




void  BitString::Set ()
{
  memset (str, 255, (size_t)byteLen);
}  /* Set */




void BitString::Set (kkuint32  bitNum)
{
  if  (bitNum >= bitLen)
  {
    // Index violation.
    cerr << std::endl
         << "BitString::Set    Invalid Index[" << bitNum << "]  BitString::bitLen[" << bitLen << "]." << std::endl
         << std::endl;
    exit (-1);
  }

  kkint32 byteOffset;
  uchar   bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  uchar&  br = str[byteOffset];
    
  br = (br | bitMasks[bitOffset]);
}  /* Set */





void  BitString::ReSet ()
{
  memset (str, 0, (size_t)byteLen);
}




void  BitString::ReSet (kkuint32 bitNum)
{
  if  (bitNum >= bitLen)
  {
    // Index violation.
    cerr << std::endl
         << "BitString::Set    Invalid Index[" << bitNum << "]  BitString::bitLen[" << bitLen << "]." << std::endl
         << std::endl;
    exit (-1);
  }

  kkint32 byteOffset;
  uchar   bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  uchar&  br = str[byteOffset];
    
  br = (br & bitMasksRev[bitOffset]);
}  /* ReSet */




void  BitString::PopulateVectorBool (VectorBool&  boolVector)  const
{
  boolVector.erase (boolVector.begin (), boolVector.end ());

  kkuint32  byteOffset = 0;
  kkuint32  numOfBits  = 0;
  kkuint32  x;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
  {
    uchar  br = str[byteOffset];
    if  (byteOffset < (byteLen - 1))
      numOfBits = 8;
    else
      numOfBits = bitLen % 8;

    for  (x = 0;  x < numOfBits;  x++)
    {
      boolVector.push_back ((br % 2) == 1);
      br = br / 2;
    }
  }
}  /* PopulateVectorBool */




void  BitString::ListOfSetBits16 (VectorUint16&  setBits)  const
{
  if  (bitLen > 65535)
  {
    KKStr  msg (50);
    msg << "BitString::ListOfSetBits  BitLen[" << bitLen << "] of this instance of BitString exceeds capacity of 'VectorUint16'.";
    cerr << std::endl << "BitString::ListOfSetBits   ***ERROR***   " << msg << std::endl << std::endl;
    throw KKException (msg);
  }

  setBits.clear ();

  kkuint32  byteOffset = 0;
  kkuint32  numOfBits  = 0;
  kkuint32  bitNum     = 0;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
  {
    uchar  br = str[byteOffset];
    if  (br == 0)
    {
      bitNum = bitNum + 8;
      continue;
    }
    else
    {
      if  (byteOffset < (byteLen - 1))
        numOfBits = 8;
      else
      {
        numOfBits = bitLen % 8;
        if  (numOfBits == 0)
          numOfBits = 8;
      }

      kkuint32 x;
      for  (x = 0;  x < numOfBits;  x++)
      {
        if  ((br % 2) == 1)
        {
          setBits.push_back ((kkuint16)bitNum);
        }

        br = br / 2;
        bitNum++;
      }
    }
  }
}  /* ListOfSetBits16 */


void  BitString::ListOfSetBits32 (VectorUint32&  setBits)  const
{
  setBits.clear ();

  kkuint32  byteOffset = 0;
  kkuint32  numOfBits  = 0;
  kkuint32  bitNum     = 0;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
  {
    uchar  br = str[byteOffset];
    if  (br == 0)
    {
      bitNum = bitNum + 8;
      continue;
    }
    else
    {
      if  (byteOffset < (byteLen - 1))
        numOfBits = 8;
      else
      {
        numOfBits = bitLen % 8;
        if  (numOfBits == 0)
          numOfBits = 8;
      }

      kkuint32 x;
      for  (x = 0;  x < numOfBits;  x++)
      {
        if  ((br % 2) == 1)
        {
          setBits.push_back (bitNum);
        }

        br = br / 2;
        bitNum++;
      }
    }
  }
}  /* ListOfSetBits32 */



KKStr  BitString::HexStr ()  const
{
  KKStr  hexStr (byteLen * 2);  // There will be 2 hex characters for every byte in bit string

  static char  hexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7','8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  kkuint32  byteOffset = 0;
  kkint32   high4Bits;
  kkint32   low4Bits;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
  {
    high4Bits = str[byteOffset] / 16;
    low4Bits  = str[byteOffset] % 16;

    hexStr.Append (hexChars[low4Bits]);
    hexStr.Append (hexChars[high4Bits]);
  }

  return hexStr;
}  /* HexStr */



kkint32  BitString::HexCharToInt (uchar hexChar)
{
  if  ((hexChar >= '0')  &&  (hexChar <= '9'))
    return  (kkint32 (hexChar) - kkint32 ('0'));

  hexChar = (char)toupper (hexChar);
  if  ((hexChar < 'A')  ||  (hexChar > 'F'))
    return -1;

  return  (10 + (kkint32 (hexChar) - kkint32 ('A')));
}  /* HexCharToInt */



BitString  BitString::FromHexStr (const KKStr&  hexStr,
                                  bool&         validHexStr
                                 )
{
  BitString  bs (hexStr.Len () * 4);
  kkint32  byteNum = 0;
  kkint32  hexStrLen = hexStr.Len ();
  kkint32  high4Bits = 0;
  kkint32  low4Bits  = 0;
  kkint32  x = 0;

  validHexStr = true;

  while  (x < hexStrLen)
  {
    low4Bits = HexCharToInt (hexStr[x]);
    x++;
    if  (low4Bits < 0)
      validHexStr = false;

    if  (x < hexStrLen)
    {
      high4Bits = HexCharToInt (hexStr[x]);
      x++;
      if  (high4Bits < 0)
        validHexStr = false;
    }
    else
    {
      high4Bits = 0;
    }

    bs.str[byteNum] = (uchar)((uchar)low4Bits + (uchar)high4Bits * (uchar)16);
    byteNum++;
  }

  return  bs;
} /* FromHexStr */




BitString&  BitString::operator=  (const BitString&  right)
{
  delete  str;
  
  bitLen  = right.bitLen;
  byteLen = right.byteLen;
  str = new uchar[byteLen];

  kkuint32  x;
  for  (x = 0;  x < byteLen;  x++)
    str[x] = right.str[x];

  return  *this;
}  /* operator= */




BitString&  BitString::operator|=  (const BitString&  right)
{
  kkuint32 shortestByteLen = Min (byteLen, right.byteLen);

  kkuint32  x;

  for  (x = 0;  x < shortestByteLen;  x++)
  {
    str[x] = str[x] | right.str[x];
  }

  return  *this;
} /* operator|= */



BitString&  BitString::operator+=  (const BitString&  right)
{
  operator|= (right);
  return  *this;
} /* operator+= */




BitString&  BitString::operator&=  (const BitString&  right)
{
  kkuint32 shortestByteLen = Min (byteLen, right.byteLen);

  kkuint32  x;

  for  (x = 0;  x < shortestByteLen;  x++)
  {
    str[x] = str[x] & right.str[x];
  }
  for  (x = shortestByteLen;  x < byteLen;  ++x)
  {
    str[x] = 0;
  }

  return  *this;
} /* operator&= */



BitString&  BitString::operator*=  (const BitString&  right)  
{
  operator&= (right);
  return  *this;
} /* operator*= */



kkint32  BitString::Compare (const BitString&  right)  const 
{
  kkuint32 shortestByteLen = Min (byteLen, right.byteLen);

  kkuint32  x = 0;

  while  (x < shortestByteLen)
  {
    if  (str[x] < right.str[x])
      return -1;

    else if  (str[x] > right.str[x])
      return  1;

    else
      x++;
  }

  if  (x >= byteLen)
  {
    if  (x >= right.byteLen)
      return 0;
    else
      return -1;
  }
  else
  {
    return  1;
  }
}  /* Compare */



bool  BitString::operator== (const BitString&  right)  const
{
  return (Compare (right) == 0);
}  /* operator== */



bool  BitString::operator!= (const BitString&  right)  const
{
  return (Compare (right) != 0);
}  /* operator!= */



bool  BitString::operator> (const BitString&  right)  const
{
  return (Compare (right) > 0);
}  /* operator> */



bool  BitString::operator>= (const BitString&  right)  const
{
  return (Compare (right) >= 0);
}  /* operator>= */



bool  BitString::operator< (const BitString&  right)  const
{
  return (Compare (right) < 0);
}  /* operator< */



BitString&   BitString::operator^= (const BitString&  right)
{
  kkuint32 shortestByteLen = Min (byteLen, right.byteLen);
  kkuint32 longestByteLen  = Max (byteLen, right.byteLen);

  kkuint32  x;

  for  (x = 0;  x < shortestByteLen;  x++)
    str[x] = str[x] ^ right.str[x];

  for  (x = shortestByteLen;  x < longestByteLen;  x++)
    str[x] = 0;

  return  *this;
}



BitString   BitString::operator^  (const BitString&  right)  /* bitwise exclusive-or */
{
  kkuint32 shortestByteLen = Min (byteLen, right.byteLen);
  kkuint32 longestByteLen  = Max (byteLen, right.byteLen);

  kkuint32  x;

  BitString result (Max (bitLen, right.bitLen));

  for  (x = 0;  x < shortestByteLen;  x++)
    result.str[x] = str[x] ^ right.str[x];

  for  (x = shortestByteLen;  x < longestByteLen;  x++)
    result.str[x] = 0;

  return  result;
}



void  BitString::ReadXML (XmlStream&      s,
                          XmlTagConstPtr  tag,
                          VolConstBool&   cancelFlag,
                          RunLog&         log
                         )
{
  log.Level(50) << "BitString::ReadXML    tag->name" << tag->Name() << endl;
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (typeid (*t) == typeid (XmlContent))
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      if  (c  &&  c->Content ())
      {
        KKStrConstPtr  text = c->Content ();
        /** @TODO decode text block into list of bits. */
        //ParseClassIndexList (*text, log);
        delete text;
        text = NULL;
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */




void  BitString::WriteXML (const KKStr&  varName,
                           ostream&      o
                          )  const
{
  XmlTag  startTag ("BitString", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  XmlContent::WriteXml (this->HexStr(), o);
  XmlTag  endTag ("BitString", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}


XmlFactoryMacro(BitString)

