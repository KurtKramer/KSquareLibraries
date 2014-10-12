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
#include "KKBaseTypes.h"
#include "KKException.h"
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

  int32  byte = 0;
  for  (byte = 0;  byte < 256;  byte++)
  {
    bitCounts[byte] = 0;
    int32  x = byte;
    while  (x > 0)
    {
      if  ((x % 2) == 1)
        bitCounts[byte]++;
      x = x / 2;
    }
  }
}  /* BuildBitCounts */





BitString::BitString (uint32  _bitLen):
  bitLen  (_bitLen),
  byteLen (0)

{
  byteLen = ((bitLen - 1) / 8) + 1;
  str = new uchar[byteLen];
  memset (str, 0, byteLen);
}




BitString::BitString (const BitString&  b):
    bitLen  (b.bitLen),
    byteLen (b.byteLen),
    str     (new uchar[b.byteLen])

{
  uint32  x;
  for  (x = 0;  x < byteLen;  x++)
  {
    str[x] = b.str[x];
  }
}


BitString::BitString (uint32   _bitLen,
                      uint16*  bitNums,
                      uint32   bitNumsLen
                     ):
    bitLen  (_bitLen),
    byteLen (0),
    str     (NULL)

{
  byteLen = ((bitLen - 1) / 8) + 1;
  str = new uchar[byteLen];
  memset (str, 0, byteLen);

  uint32  x;

  for  (x = 0;  x < bitNumsLen;  x++)
  {
    if  (bitNums[x] >= bitLen)
    {
      cerr << std::endl << std::endl
           << "BitString::BitString Constructing from list of numbers  *** ERROR ***" << std::endl
           << std::endl
           << "           bitNums[" << x << "] = [" << bitNums[x] << "] which is >= bitLen[" << bitLen << "]." << std::endl
           << std::endl;
      exit (-1);
    }

    Set (bitNums[x]);
  }
}




BitString::~BitString ()
{
  delete  str;
  str = NULL;
}


void  BitString::CalcByteAndBitOffsets (uint32  bitNum,
                                        int32&    byteOffset,
                                        uchar&  bitOffset
                                       )  const
{
  byteOffset = bitNum / 8;
  bitOffset  = bitNum % 8;
} /* CalcByteAndBitOffsets */



KKB::uint32  BitString::Count  ()  const
{
  BuildBitCounts ();

  uint32  count = 0;
  uint32  byteOffset = 0;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
    count += bitCounts[str[byteOffset]];

  return  count;
}  /* Count */




bool  BitString::Test (uint32  bitNum) const
{
  int32 byteOffset;
  uchar bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  bool  bit = ((str[byteOffset] & bitMasks[bitOffset]) != 0);

  return bit;
}  /* IsSet */




void  BitString::Set ()
{
  memset (str, 255, byteLen);
}  /* Set */





void BitString::Set (uint32  bitNum)
{
  if  (bitNum >= bitLen)
  {
    // Index violation.
    cerr << std::endl
         << "BitString::Set    Invalid Index[" << bitNum << "]  BitString::bitLen[" << bitLen << "]." << std::endl
         << std::endl;
    exit (-1);
  }

  int32 byteOffset;
  uchar bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  uchar&  br = str[byteOffset];
    
  br = (br | bitMasks[bitOffset]);
}  /* Set */





void  BitString::ReSet ()
{
  memset (str, 0, byteLen);
}




void  BitString::ReSet (uint32 bitNum)
{
  if  (bitNum >= bitLen)
  {
    // Index violation.
    cerr << std::endl
         << "BitString::Set    Invalid Index[" << bitNum << "]  BitString::bitLen[" << bitLen << "]." << std::endl
         << std::endl;
    exit (-1);
  }

  int32 byteOffset;
  uchar bitOffset;

  CalcByteAndBitOffsets (bitNum, byteOffset, bitOffset);

  uchar&  br = str[byteOffset];
    
  br = (br & bitMasksRev[bitOffset]);
}  /* ReSet */




void  BitString::PopulateVectorBool (VectorBool&  boolVector)  const
{
  boolVector.erase (boolVector.begin (), boolVector.end ());

  uint32  byteOffset = 0;
  uint32  numOfBits  = 0;
  uint32  x;

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

  uint32  byteOffset = 0;
  uint32  numOfBits  = 0;
  uint32  bitNum     = 0;

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

      uint32 x;
      for  (x = 0;  x < numOfBits;  x++)
      {
        if  ((br % 2) == 1)
        {
          setBits.push_back ((uint16)bitNum);
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

  uint32  byteOffset = 0;
  uint32  numOfBits  = 0;
  uint32  bitNum     = 0;

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

      uint32 x;
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

  uint32  byteOffset = 0;
  int32 high4Bits;
  int32 low4Bits;

  for  (byteOffset = 0;  byteOffset < byteLen;  byteOffset++)
  {
    high4Bits = str[byteOffset] / 16;
    low4Bits  = str[byteOffset] % 16;

    hexStr.Append (hexChars[low4Bits]);
    hexStr.Append (hexChars[high4Bits]);
  }

  return hexStr;
}  /* HexStr */



int32  BitString::HexCharToInt (uchar hexChar)
{
  if  ((hexChar >= '0')  &&  (hexChar <= '9'))
    return  (int32 (hexChar) - int32 ('0'));

  hexChar = (char)toupper (hexChar);
  if  ((hexChar < 'A')  ||  (hexChar > 'F'))
    return -1;

  return  (10 + (int32 (hexChar) - int32 ('A')));
}  /* HexCharToInt */



BitString  BitString::FromHexStr (const KKStr&  hexStr,
                                  bool&         validHexStr
                                 )
{
  BitString  bs (hexStr.Len () * 4);
  int32  byteNum = 0;
  int32  hexStrLen = hexStr.Len ();
  int32  high4Bits = 0;
  int32  low4Bits  = 0;
  int32  x = 0;

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

    bs.str[byteNum] = (uchar)low4Bits + (uchar)high4Bits * 16;
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

  uint32  x;
  for  (x = 0;  x < byteLen;  x++)
    str[x] = right.str[x];

  return  *this;
}  /* operator= */




BitString&  BitString::operator|=  (const BitString&  right)
{
  uint32 shortestByteLen = Min (byteLen, right.byteLen);

  uint32  x;

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
  uint32 shortestByteLen = Min (byteLen, right.byteLen);

  uint32  x;

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



int32  BitString::Compare (const BitString&  right)  const 
{
  uint32 shortestByteLen = Min (byteLen, right.byteLen);

  uint32  x = 0;

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
  uint32 shortestByteLen = Min (byteLen, right.byteLen);
  uint32 longestByteLen  = Max (byteLen, right.byteLen);

  uint32  x;

  for  (x = 0;  x < shortestByteLen;  x++)
    str[x] = str[x] ^ right.str[x];

  for  (x = shortestByteLen;  x < longestByteLen;  x++)
    str[x] = 0;

  return  *this;
}



BitString   BitString::operator^  (const BitString&  right)  /* bitwise exclusive-or */
{
  uint32 shortestByteLen = Min (byteLen, right.byteLen);
  uint32 longestByteLen  = Max (byteLen, right.byteLen);

  uint32  x;

  BitString result (Max (bitLen, right.bitLen));

  for  (x = 0;  x < shortestByteLen;  x++)
    result.str[x] = str[x] ^ right.str[x];

  for  (x = shortestByteLen;  x < longestByteLen;  x++)
    result.str[x] = 0;

  return  result;
}
