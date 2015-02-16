/* BitString.cpp -- Bit String management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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
#include "BasicTypes.h"
#include "KKException.h"
using namespace KKU;


// Bit                                  0      1      2      3       4       5       6        7
uchar  BitString::bitMasks[8]    = {    1,     2,     4,     8,     16,     32,     64,     128};

uchar  BitString::bitMasksRev[8] = {255-1, 255-2, 255-4, 255-8, 255-16, 255-32, 255-64, 255-128};

uchar* BitString::bitCounts = NULL;

/*
bool   bitPatterns[256][8] = 
{
  {false, false, false, false, false, false, false, false},  //   0
  {false, false, false, false, false, false, false, true},   //   1
  {false, false, false, false, false, false, true,  false},  //   2
  {false, false, false, false, false, false, true,  true},   //   3
  {false, false, false, false, false, true,  false, false},  //   4
  {false, false, false, false, false, true,  false, true},   //   5
  {false, false, false, false, false, true,  true,  false},  //   6
  {false, false, false, false, false, true,  true,  true},   //   7

  {false, false, false, false, true,  false, false, false},  //   8
  {false, false, false, false, true,  false, false, true},   //   9
  {false, false, false, false, true,  false, true,  false},  //  10
  {false, false, false, false, true,  false, true,  true},   //  11
  {false, false, false, false, true,  true,  false, false},  //  12
  {false, false, false, false, true,  true,  false, true},   //  13
  {false, false, false, false, true,  true,  true,  false},  //  14
  {false, false, false, false, true,  true,  true,  true},   //  15

  {false, false, false, true, false, false, false, false},  //   16
  {false, false, false, true, false, false, false, true},   //   17
  {false, false, false, true, false, false, true,  false},  //   18
  {false, false, false, true, false, false, true,  true},   //   19
  {false, false, false, true, false, true,  false, false},  //   20
  {false, false, false, true, false, true,  false, true},   //   21
  {false, false, false, true, false, true,  true,  false},  //   22
  {false, false, false, true, false, true,  true,  true},   //   23

  {false, false, false, true,  true,  false, false, false},  //  24
  {false, false, false, true,  true,  false, false, true},   //  25
  {false, false, false, true,  true,  false, true,  false},  //  26
  {false, false, false, true,  true,  false, true,  true},   //  27
  {false, false, false, true,  true,  true,  false, false},  //  28
  {false, false, false, true,  true,  true,  false, true},   //  29
  {false, false, false, true,  true,  true,  true,  false},  //  30
  {false, false, false, true,  true,  true,  true,  true},   //  31

  {false, false, true,  false, false, false, false, false},  //  32
  {false, false, true,  false, false, false, false, true},   //  33
  {false, false, true,  false, false, false, true,  false},  //  34
  {false, false, true,  false, false, false, true,  true},   //  35
  {false, false, true,  false, false, true,  false, false},  //  36
  {false, false, true,  false, false, true,  false, true},   //  37
  {false, false, true,  false, false, true,  true,  false},  //  38
  {false, false, true,  false, false, true,  true,  true},   //  39

  {false, false, true,  false, true,  false, false, false},  //  40
  {false, false, true,  false, true,  false, false, true},   //  41
  {false, false, true,  false, true,  false, true,  false},  //  42
  {false, false, true,  false, true,  false, true,  true},   //  43
  {false, false, true,  false, true,  true,  false, false},  //  44
  {false, false, true,  false, true,  true,  false, true},   //  45
  {false, false, true,  false, true,  true,  true,  false},  //  46
  {false, false, true,  false, true,  true,  true,  true},   //  47

  {false, false, true,  true, false, false, false, false},  //   48
  {false, false, true,  true, false, false, false, true},   //   49
  {false, false, true,  true, false, false, true,  false},  //   50
  {false, false, true,  true, false, false, true,  true},   //   51
  {false, false, true,  true, false, true,  false, false},  //   52
  {false, false, true,  true, false, true,  false, true},   //   53
  {false, false, true,  true, false, true,  true,  false},  //   54
  {false, false, true,  true, false, true,  true,  true},   //   55
 
  {false, false, true,  true,  true,  false, false, false},  //  56
  {false, false, true,  true,  true,  false, false, true},   //  57
  {false, false, true,  true,  true,  false, true,  false},  //  58
  {false, false, true,  true,  true,  false, true,  true},   //  59 
  {false, false, true,  true,  true,  true,  false, false},  //  60
  {false, false, true,  true,  true,  true,  false, true},   //  61
  {false, false, true,  true,  true,  true,  true,  false},  //  62
  {false, false, true,  true,  true,  true,  true,  true},   //  63




  {false, true,  false, false, false, false, false, false},  //  64
  {false, true,  false, false, false, false, false, true},   //  65
  {false, true,  false, false, false, false, true,  false},  //  66
  {false, true,  false, false, false, false, true,  true},   //  67
  {false, true,  false, false, false, true,  false, false},  //  68
  {false, true,  false, false, false, true,  false, true},   //  69
  {false, true,  false, false, false, true,  true,  false},  //  70
  {false, true,  false, false, false, true,  true,  true},   //  71

  {false, true,  false, false, true,  false, false, false},  //  72
  {false, true,  false, false, true,  false, false, true},   //  73
  {false, true,  false, false, true,  false, true,  false},  //  74
  {false, true,  false, false, true,  false, true,  true},   //  75
  {false, true,  false, false, true,  true,  false, false},  //  76
  {false, true,  false, false, true,  true,  false, true},   //  77
  {false, true,  false, false, true,  true,  true,  false},  //  78
  {false, true,  false, false, true,  true,  true,  true},   //  79

  {false, true,  false, true, false, false, false, false},  //   80
  {false, true,  false, true, false, false, false, true},   //   81
  {false, true,  false, true, false, false, true,  false},  //   82
  {false, true,  false, true, false, false, true,  true},   //   83
  {false, true,  false, true, false, true,  false, false},  //   84
  {false, true,  false, true, false, true,  false, true},   //   85
  {false, true,  false, true, false, true,  true,  false},  //   86
  {false, true,  false, true, false, true,  true,  true},   //   87

  {false, true,  false, true,  true,  false, false, false},  //  88
  {false, true,  false, true,  true,  false, false, true},   //  89
  {false, true,  false, true,  true,  false, true,  false},  //  90
  {false, true,  false, true,  true,  false, true,  true},   //  91
  {false, true,  false, true,  true,  true,  false, false},  //  92
  {false, true,  false, true,  true,  true,  false, true},   //  93
  {false, true,  false, true,  true,  true,  true,  false},  //  94
  {false, true,  false, true,  true,  true,  true,  true},   //  95

  {false, true,  true,  false, false, false, false, false},  //  96
  {false, true,  true,  false, false, false, false, true},   //  97
  {false, true,  true,  false, false, false, true,  false},  //  98
  {false, true,  true,  false, false, false, true,  true},   //  99
  {false, true,  true,  false, false, true,  false, false},  // 100
  {false, true,  true,  false, false, true,  false, true},   // 101
  {false, true,  true,  false, false, true,  true,  false},  // 102
  {false, true,  true,  false, false, true,  true,  true},   // 103

  {false, true,  true,  false, true,  false, false, false},  // 104
  {false, true,  true,  false, true,  false, false, true},   // 105
  {false, true,  true,  false, true,  false, true,  false},  // 106
  {false, true,  true,  false, true,  false, true,  true},   // 107
  {false, true,  true,  false, true,  true,  false, false},  // 108
  {false, true,  true,  false, true,  true,  false, true},   // 109
  {false, true,  true,  false, true,  true,  true,  false},  // 110
  {false, true,  true,  false, true,  true,  true,  true},   // 111

  {false, true,  true,  true, false, false, false, false},  //  112
  {false, true,  true,  true, false, false, false, true},   //  113
  {false, true,  true,  true, false, false, true,  false},  //  114
  {false, true,  true,  true, false, false, true,  true},   //  115
  {false, true,  true,  true, false, true,  false, false},  //  116
  {false, true,  true,  true, false, true,  false, true},   //  117
  {false, true,  true,  true, false, true,  true,  false},  //  118
  {false, true,  true,  true, false, true,  true,  true},   //  119
 
  {false, true,  true,  true,  true,  false, false, false},  // 120
  {false, true,  true,  true,  true,  false, false, true},   // 121
  {false, true,  true,  true,  true,  false, true,  false},  // 122
  {false, true,  true,  true,  true,  false, true,  true},   // 123 
  {false, true,  true,  true,  true,  true,  false, false},  // 124
  {false, true,  true,  true,  true,  true,  false, true},   // 125
  {false, true,  true,  true,  true,  true,  true,  false},  // 126
  {false, true,  true,  true,  true,  true,  true,  true},   // 127


  //128-255
  {true,  false, false, false, false, false, false, false},  // 128
  {true,  false, false, false, false, false, false, true},   // 129
  {true,  false, false, false, false, false, true,  false},  // 130
  {true,  false, false, false, false, false, true,  true},   // 131
  {true,  false, false, false, false, true,  false, false},  // 132
  {true,  false, false, false, false, true,  false, true},   // 133
  {true,  false, false, false, false, true,  true,  false},  // 134
  {true,  false, false, false, false, true,  true,  true},   // 135

  {true,  false, false, false, true,  false, false, false},  // 136
  {true,  false, false, false, true,  false, false, true},   // 137
  {true,  false, false, false, true,  false, true,  false},  // 138
  {true,  false, false, false, true,  false, true,  true},   // 139
  {true,  false, false, false, true,  true,  false, false},  // 140
  {true,  false, false, false, true,  true,  false, true},   // 141
  {true,  false, false, false, true,  true,  true,  false},  // 142
  {true,  false, false, false, true,  true,  true,  true},   // 143

  {true,  false, false, true, false, false, false, false},  //  144
  {true,  false, false, true, false, false, false, true},   //  145
  {true,  false, false, true, false, false, true,  false},  //  146
  {true,  false, false, true, false, false, true,  true},   //  147
  {true,  false, false, true, false, true,  false, false},  //  148
  {true,  false, false, true, false, true,  false, true},   //  149
  {true,  false, false, true, false, true,  true,  false},  //  150
  {true,  false, false, true, false, true,  true,  true},   //  151

  {true,  false, false, true,  true,  false, false, false},  // 152
  {true,  false, false, true,  true,  false, false, true},   // 153
  {true,  false, false, true,  true,  false, true,  false},  // 154
  {true,  false, false, true,  true,  false, true,  true},   // 155
  {true,  false, false, true,  true,  true,  false, false},  // 156
  {true,  false, false, true,  true,  true,  false, true},   // 157
  {true,  false, false, true,  true,  true,  true,  false},  // 158
  {true,  false, false, true,  true,  true,  true,  true},   // 159

  {true,  false, true,  false, false, false, false, false},  // 160
  {true,  false, true,  false, false, false, false, true},   // 161
  {true,  false, true,  false, false, false, true,  false},  // 162
  {true,  false, true,  false, false, false, true,  true},   // 163
  {true,  false, true,  false, false, true,  false, false},  // 164
  {true,  false, true,  false, false, true,  false, true},   // 165
  {true,  false, true,  false, false, true,  true,  false},  // 166
  {true,  false, true,  false, false, true,  true,  true},   // 167

  {true,  false, true,  false, true,  false, false, false},  // 168
  {true,  false, true,  false, true,  false, false, true},   // 169
  {true,  false, true,  false, true,  false, true,  false},  // 170
  {true,  false, true,  false, true,  false, true,  true},   // 171
  {true,  false, true,  false, true,  true,  false, false},  // 172
  {true,  false, true,  false, true,  true,  false, true},   // 173
  {true,  false, true,  false, true,  true,  true,  false},  // 174
  {true,  false, true,  false, true,  true,  true,  true},   // 175

  {true,  false, true,  true, false, false, false, false},  //  176
  {true,  false, true,  true, false, false, false, true},   //  177
  {true,  false, true,  true, false, false, true,  false},  //  178
  {true,  false, true,  true, false, false, true,  true},   //  179
  {true,  false, true,  true, false, true,  false, false},  //  180
  {true,  false, true,  true, false, true,  false, true},   //  181
  {true,  false, true,  true, false, true,  true,  false},  //  182
  {true,  false, true,  true, false, true,  true,  true},   //  183

  {true,  false, true,  true,  true,  false, false, false},  // 184
  {true,  false, true,  true,  true,  false, false, true},   // 185
  {true,  false, true,  true,  true,  false, true,  false},  // 186
  {true,  false, true,  true,  true,  false, true,  true},   // 187 
  {true,  false, true,  true,  true,  true,  false, false},  // 188
  {true,  false, true,  true,  true,  true,  false, true},   // 189
  {true,  false, true,  true,  true,  true,  true,  false},  // 190
  {true,  false, true,  true,  true,  true,  true,  true},   // 191

  {true,  true,  false, false, false, false, false, false},  // 192
  {true,  true,  false, false, false, false, false, true},   // 193
  {true,  true,  false, false, false, false, true,  false},  // 194
  {true,  true,  false, false, false, false, true,  true},   // 195
  {true,  true,  false, false, false, true,  false, false},  // 196
  {true,  true,  false, false, false, true,  false, true},   // 197
  {true,  true,  false, false, false, true,  true,  false},  // 198
  {true,  true,  false, false, false, true,  true,  true},   // 199

  {true,  true,  false, false, true,  false, false, false},  // 200
  {true,  true,  false, false, true,  false, false, true},   // 201
  {true,  true,  false, false, true,  false, true,  false},  // 202
  {true,  true,  false, false, true,  false, true,  true},   // 203
  {true,  true,  false, false, true,  true,  false, false},  // 204
  {true,  true,  false, false, true,  true,  false, true},   // 205
  {true,  true,  false, false, true,  true,  true,  false},  // 206
  {true,  true,  false, false, true,  true,  true,  true},   // 207

  {true,  true,  false, true, false, false, false, false},  //  208
  {true,  true,  false, true, false, false, false, true},   //  209
  {true,  true,  false, true, false, false, true,  false},  //  210
  {true,  true,  false, true, false, false, true,  true},   //  211
  {true,  true,  false, true, false, true,  false, false},  //  212
  {true,  true,  false, true, false, true,  false, true},   //  213 
  {true,  true,  false, true, false, true,  true,  false},  //  214
  {true,  true,  false, true, false, true,  true,  true},   //  215

  {true,  true,  false, true,  true,  false, false, false},  // 216
  {true,  true,  false, true,  true,  false, false, true},   // 217
  {true,  true,  false, true,  true,  false, true,  false},  // 218
  {true,  true,  false, true,  true,  false, true,  true},   // 219
  {true,  true,  false, true,  true,  true,  false, false},  // 220
  {true,  true,  false, true,  true,  true,  false, true},   // 221
  {true,  true,  false, true,  true,  true,  true,  false},  // 222
  {true,  true,  false, true,  true,  true,  true,  true},   // 223

  {true,  true,  true,  false, false, false, false, false},  // 224
  {true,  true,  true,  false, false, false, false, true},   // 225
  {true,  true,  true,  false, false, false, true,  false},  // 226
  {true,  true,  true,  false, false, false, true,  true},   // 227
  {true,  true,  true,  false, false, true,  false, false},  // 228
  {true,  true,  true,  false, false, true,  false, true},   // 229
  {true,  true,  true,  false, false, true,  true,  false},  // 230
  {true,  true,  true,  false, false, true,  true,  true},   // 231

  {true,  true,  true,  false, true,  false, false, false},  // 232
  {true,  true,  true,  false, true,  false, false, true},   // 233
  {true,  true,  true,  false, true,  false, true,  false},  // 234
  {true,  true,  true,  false, true,  false, true,  true},   // 235
  {true,  true,  true,  false, true,  true,  false, false},  // 236
  {true,  true,  true,  false, true,  true,  false, true},   // 237
  {true,  true,  true,  false, true,  true,  true,  false},  // 238
  {true,  true,  true,  false, true,  true,  true,  true},   // 239

  {true,  true,  true,  true, false, false, false, false},  //  240
  {true,  true,  true,  true, false, false, false, true},   //  241
  {true,  true,  true,  true, false, false, true,  false},  //  242
  {true,  true,  true,  true, false, false, true,  true},   //  243
  {true,  true,  true,  true, false, true,  false, false},  //  244
  {true,  true,  true,  true, false, true,  false, true},   //  245
  {true,  true,  true,  true, false, true,  true,  false},  //  246
  {true,  true,  true,  true, false, true,  true,  true},   //  247
 
  {true,  true,  true,  true,  true,  false, false, false},  // 248
  {true,  true,  true,  true,  true,  false, false, true},   // 249
  {true,  true,  true,  true,  true,  false, true,  false},  // 250
  {true,  true,  true,  true,  true,  false, true,  true},   // 251 
  {true,  true,  true,  true,  true,  true,  false, false},  // 252
  {true,  true,  true,  true,  true,  true,  false, true},   // 253
  {true,  true,  true,  true,  true,  true,  true,  false},  // 254
  {true,  true,  true,  true,  true,  true,  true,  true}    // 255
};
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



KKU::uint32  BitString::Count  ()  const
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
