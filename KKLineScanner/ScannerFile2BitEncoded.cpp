#include "FirstIncludes.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include  "ScannerFile.h"

#include  "Variables.h"

#include  "ScannerFile2BitEncoded.h"

using namespace  KKLSC;

#pragma pack(push,1)
//#pragma pack(show)
struct  ScannerFile2BitEncoded::OpRecEndOfScanLine
{
  ushort opCode:  4;  /** 0 = End of Scan Line. */
  ushort filler:  4;
};



struct  ScannerFile2BitEncoded::OpRecTextBlock
{
  /** Will handle text blocks in one or more sections with 2^11 characters in each *
   *  The last section will have 'endOfText' == '1'                                 *
   */

  uchar opCode:      4;  /**< 1 = Text Block.                                      */
  uchar endOfText:   1;  /**< 0 = More Text blocks to follow, 1 = last text block. */
  uchar lenHigh:     3;  /**< Length = 256 * lenHigh + lenLow                      */
};  /* OpRecTextBlock */
  


struct  ScannerFile2BitEncoded::OpRecTextBlock_2
{
  uchar lenLow :8;     /**< Add this value to the prev rec 'lenHighBits' field to get the length of the text block. */
};



struct  ScannerFile2BitEncoded::OpRecRunLenPVx
{
  uchar  opCode    :4;      /**< Op Codes (4 thru 7)  4=PV0, 5=PV1, 6=PV2, 7=PV3              */
  uchar  len       :4;      /**<  (len + 3), 0=3, 1=4, ... 15=18,  White(0) pixels in a row.  */
};  /* OpRecRunLenPVx */



struct  ScannerFile2BitEncoded::OpRecRunLen10Bit
{
  uchar  opCode    :4;      /**< 8  Variable Run Length range (0 - 1023).  */
  uchar  pix       :2;      /**< Pixel                                     */
  uchar  lenHigh   :2;      /**< length = lenHigh * 256 + lenLow           */
};  /* OpRecRunLen10Bit */



struct  ScannerFile2BitEncoded::OpRecRunLen10Bit_2
{
  uchar   lenLow   :8;        /**< High bits of run-length    */
};  /* OpRecRunLen10Bit_2 */



struct  ScannerFile2BitEncoded::OpRecRawPixelOne
{
  uchar  opCode   :4;      /**< 10 = One raw pixel      */
  uchar  pix1     :2;
  uchar  filler   :2;
};  /* OpRecRawPixelOne */



struct  ScannerFile2BitEncoded::OpRecRawPixelsTwo
{
  uchar  opCode :4;         /**< 11 = Two raw pixel      */
  uchar  pix1   :2;
  uchar  pix2   :2;
};  /* OpRecRawPixelsTwo */



struct  ScannerFile2BitEncoded::OpRecRawPixelsVarLen4Bit
{
  uchar  opCode :4;         /**< 12 = Raw Pixels, Length 3 thru 18 */
  uchar  len    :4;         /**<  0 = 3, 1 = 4,,,, 15 = 18         */
};  /* OpRecRawPixelsVarLen4Bit */



struct  ScannerFile2BitEncoded::OpRecRawPixelsVarLen12Bit
{
  uchar  opCode  :4;         /**< 13 = Raw Pixels, Length 0 thru 4095 */
  uchar  lenHigh :4;         /**<  RawStrLen = lenHigh * 256 + lenLow */ 
};  /* OpRecRawPixelsVarLen12Bit */



struct  ScannerFile2BitEncoded::OpRecRawPixelsVarLen12Bit_2
{
  uchar  lenLow  :8;         /**< RawStrLen = lenHigh * 256 + lenLow */ 
};  /* OpRecRawPixelsVarLen12Bit_2 */




struct  ScannerFile2BitEncoded::RawPixelRec
{
  uchar  pix0   :2;
  uchar  pix1   :2;
  uchar  pix2   :2;
  uchar  pix3   :2;
};  /* RawPixelRec */




union  ScannerFile2BitEncoded::OpRec
{
  OpRecEndOfScanLine             endOfScanLine;             //  0
  OpRecTextBlock                 textBlock;                 //  1
  OpRecTextBlock_2               textBlock_2;               //  1 (Part 2)
  OpRecRunLenPVx                 runLenPVx;                 //  4 thru 7
  OpRecRunLen10Bit               runLen10Bit;               //  8
  OpRecRunLen10Bit_2             runLen10Bit_2;             //  8 (Part 2)
  OpRecRawPixelOne               rawPixelOne;               // 10
  OpRecRawPixelsTwo              rawPixelsTwo;              // 11
  OpRecRawPixelsVarLen4Bit       rawPixelsVarLen4Bit;       // 12
  OpRecRawPixelsVarLen12Bit      rawPixelsVarLen12Bit;      // 13
  OpRecRawPixelsVarLen12Bit_2    rawPixelsVarLen12Bit_2;    // 13 (Part 2)
  RawPixelRec                    rawPixelRec;
  uchar                          textChar;
};  /* OpRec */


#pragma pack(pop)
//#pragma pack(show)


uchar*  ScannerFile2BitEncoded::convTable2BitTo8Bit = NULL;
uchar*  ScannerFile2BitEncoded::convTable8BitTo2Bit = NULL;
uchar*  ScannerFile2BitEncoded::compensationTable   = NULL;


ScannerFile2BitEncoded::ScannerFile2BitEncoded (const KKStr&  _fileName,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _log),
  rawPixelRecBuffer     (NULL),
  rawPixelRecBufferLen  (0),
  rawPixelRecBufferSize (0),
  encodedBuff           (NULL),
  encodedBuffLen        (0),
  encodedBuffNext       (NULL),
  encodedBuffSize       (0),
  rawStr                (NULL),
  rawStrLen             (0),
  rawStrSize            (0),
  runLen                (0),
  runLenChar            (0),
  curCompStatus         (csNull)
{
  BuildConversionTables ();
}





ScannerFile2BitEncoded::ScannerFile2BitEncoded (const KKStr&  _fileName,
                                                kkuint32      _channelCount,
                                                kkuint32      _pixelsPerScanLine,
                                                kkuint32      _frameHeight,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _channelCount, _pixelsPerScanLine, _frameHeight, _log),
  rawPixelRecBuffer     (NULL),
  rawPixelRecBufferLen  (0),
  rawPixelRecBufferSize (0),
  encodedBuff           (NULL),
  encodedBuffLen        (0),
  encodedBuffNext       (NULL),
  encodedBuffSize       (0),
  rawStr                (NULL),
  rawStrLen             (0),
  rawStrSize            (0),
  runLen                (0),
  runLenChar            (0),
  curCompStatus         (csNull)
{
  BuildConversionTables ();
  AllocateEncodedBuff ();
  AllocateRawStr ((kkuint16)(_pixelsPerScanLine + 100));
}



ScannerFile2BitEncoded::~ScannerFile2BitEncoded (void)
{
  if  (opened)
    Close ();

  delete  encodedBuff;          encodedBuff         = NULL;
  delete  rawPixelRecBuffer;    rawPixelRecBuffer   = NULL;
  delete  rawStr;               rawStr              = NULL;
}


void  ScannerFile2BitEncoded::AllocateRawPixelRecBuffer (kkuint32 size)
{
  delete  rawPixelRecBuffer;
  rawPixelRecBuffer     = new RawPixelRec[size];
  rawPixelRecBufferSize = size;
  rawPixelRecBufferLen  = 0;
}  /* AllocateRawPixelRecBuffer */



void  ScannerFile2BitEncoded::AllocateRawStr (kkuint16  size)
{
  if  (rawStr)
  {
    uchar*    newRawStr = new uchar[size];
    kkuint16  bytesToCopy = Min (rawStrLen, size);
    memcpy (newRawStr, rawStr, bytesToCopy);
    delete rawStr;
    rawStr = newRawStr;
    rawStrLen = bytesToCopy;
    rawStrSize = size;
  }
  else
  {
    rawStrSize = size;
    rawStrLen = 0;
    rawStr = new uchar[rawStrSize];
  }
}  /* AllocateRawStr */




void  ScannerFile2BitEncoded::AllocateEncodedBuff ()
{
  delete  encodedBuff;  encodedBuff  = NULL;

  kkint32  frameWidth = Max ((kkuint32)2048, 2 * PixelsPerScanLine ());
  
  encodedBuffSize = (int)(1.2f * (float)frameWidth);

  encodedBuff = new OpRec[encodedBuffSize];
  encodedBuffNext = encodedBuff;
  encodedBuffLen  = 0;
}  /* AllocateEncodedBuff */



void  ScannerFile2BitEncoded::ReSizeEncodedBuff (kkuint32  newSize)
{
  OpRecPtr  newEncodedBuff = new OpRec[newSize];

  kkuint32  recsToMove = Min (newSize, encodedBuffLen);

  memcpy (newEncodedBuff, encodedBuff, recsToMove * sizeof (OpRec));
  encodedBuffNext = newEncodedBuff + (encodedBuffNext - encodedBuff);

  delete  encodedBuff;
  encodedBuff = newEncodedBuff;
  encodedBuffSize = newSize;
}  /* ReSizeEncodedBuff */



void  ScannerFile2BitEncoded::BuildConversionTables ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (!convTable2BitTo8Bit)
  {
    kkint32  x = 0;

    convTable2BitTo8Bit = new uchar[4];
    convTable2BitTo8Bit[0] =   0;
    convTable2BitTo8Bit[1] =  85;
    convTable2BitTo8Bit[2] = 170;
    convTable2BitTo8Bit[3] = 255;

    convTable8BitTo2Bit = new uchar[256];
    for  (x = 0;  x <= 31;  ++x)
      convTable8BitTo2Bit[x] = 0;

    for  (x = 32;  x <= 105;  ++x)
      convTable8BitTo2Bit[x] = 1;

    for  (x = 106;  x <= 179;  ++x)
      convTable8BitTo2Bit[x] = 2;

    for  (x = 180;  x <= 255;  ++x)
      convTable8BitTo2Bit[x] = 3;

    compensationTable = new uchar[256];
    for  (kkint32 pv = 0;  pv < 256;  ++pv)
    {
      uchar encodedValue = convTable8BitTo2Bit[pv];
      compensationTable[pv] = convTable2BitTo8Bit[encodedValue];
    }
    atexit (ScannerFile2BitEncoded::ExitCleanUp);
  }

  GlobalGoalKeeper::EndBlock ();
}  /* BuildConversionTables */



const uchar*  ScannerFile2BitEncoded::CompensationTable ()
{
  GlobalGoalKeeper::StartBlock ();
  if  (!compensationTable)
    BuildConversionTables ();
  GlobalGoalKeeper::EndBlock ();
  return  compensationTable;
}  /* CompensationTable */





void  ScannerFile2BitEncoded::ExitCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();
  delete  convTable2BitTo8Bit;  convTable2BitTo8Bit = NULL;
  delete  convTable8BitTo2Bit;  convTable8BitTo2Bit = NULL;
  delete  compensationTable;    compensationTable   = NULL;
  GlobalGoalKeeper::EndBlock ();
}




void  ScannerFile2BitEncoded::ScanRate (float  _scanRate)
{
  ScannerFile::ScanRate (_scanRate);
}



/*****************************************************************************/
/*        Following routines are used for READING Scanner Files.             */
/*****************************************************************************/


kkuint32  ScannerFile2BitEncoded::ReadBufferFrame ()
{
  frameBufferLen = 0;
  frameBufferNextLine = 0;
  if  (feof (file) != 0)
  {
    memset (frameBuffer, 0, frameBufferSize);
    return 0;
  }

  frameBufferFileOffsetLast = osFTELL (file);
  kkuint32  numScanLinesReadThisFrameBuffer = 0;
  uchar*  buffPtr = frameBuffer;
  while  ((feof (file) == 0)  &&  (numScanLinesReadThisFrameBuffer < frameHeight))
  {
    GetNextScanLine (buffPtr, pixelsPerScanLine);
    frameBufferLen += pixelsPerScanLine;
    buffPtr += pixelsPerScanLine;
    ++numScanLinesReadThisFrameBuffer;
  }

  frameBufferFileOffsetNext = osFTELL (file);
  frameBufferNextLine = 0;
  return  numScanLinesReadThisFrameBuffer;
}  /* ReadBufferFrame */
 


kkint64  ScannerFile2BitEncoded::SkipToNextFrame ()
{
  uchar*  scanLine = new uchar[pixelsPerScanLine];

  kkuint32  numScanLinesReadThisFrameBuffer = 0;
  while  ((feof (file) == 0)  &&  (numScanLinesReadThisFrameBuffer < frameHeight))
  {
    GetNextScanLine (scanLine, pixelsPerScanLine);
    ++numScanLinesReadThisFrameBuffer;
  }

  if  (feof (file) != 0)
    return -1;
  else
    return osFTELL (file);
}  /* SkipToNextFrame */



void  ScannerFile2BitEncoded::ProcessTextBlock (const OpRec&  rec)
{
  OpRec  rec2;

  size_t  recsRead = fread (&rec2, sizeof (rec2), 1, file);
  if  (recsRead < 1)
  {
    eof = true;
    return;
  }

  kkuint32  numTextBytes = 256 * rec.textBlock.lenHigh + rec2.textBlock_2.lenLow;
  char* textMsgPtr = new char[numTextBytes + 1];
  kkuint32 textMsgLen = numTextBytes;
  
  recsRead = fread (textMsgPtr, 1, numTextBytes, file);
  if  (recsRead < numTextBytes)
     eof = true;
  textMsgPtr[recsRead] = 0;
  ReportTextMsg (textMsgPtr, textMsgLen);
  delete[]  textMsgPtr;  textMsgPtr = NULL;
  textMsgLen = 0;
}  /* ProcessTextBlock */



void  ScannerFile2BitEncoded::ProcessRawPixelRecs (kkuint16  numRawPixels,
                                                   uchar*    lineBuff,
                                                   kkuint32  lineBuffSize,
                                                   kkuint32& bufferLineLen
                                                  )
{
  kkuint32  numRawPixelRecsToRead  = numRawPixels / 4;
  kkuint32  numFullOccupiedRawRecs = numRawPixelRecsToRead;
  kkuint32  numOverFlowPixels      = numRawPixels % 4;
  if  (numOverFlowPixels > 0)
    ++numRawPixelRecsToRead;

  if  (numRawPixelRecsToRead > rawPixelRecBufferSize)
    AllocateRawPixelRecBuffer (numRawPixelRecsToRead + 30);

  size_t  recsRead = fread (rawPixelRecBuffer, sizeof (RawPixelRec), numRawPixelRecsToRead, file);
  if  (recsRead < numRawPixelRecsToRead)
  {
    eof = true;
    return;
  }

  kkuint32  newBufferLineLen = bufferLineLen + numRawPixels;
  if  (newBufferLineLen > lineBuffSize)
  {
    cerr << "ScannerFile2BitEncoded::ProcessRawPixelRecs    *** Line Length Exceeded ***" << endl;
    return;
  }

  kkuint32 x = 0;
  for  (x = 0;  x < numFullOccupiedRawRecs;  ++x)
  {
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix0];   ++bufferLineLen;
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix1];   ++bufferLineLen;
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix2];   ++bufferLineLen;
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix3];   ++bufferLineLen;
  }

  if  (numOverFlowPixels > 0)
  {
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix0];   ++bufferLineLen;
  }

  if  (numOverFlowPixels > 1)
  {
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix0];   ++bufferLineLen;
  }

  if  (numOverFlowPixels > 2)
  {
    lineBuff[bufferLineLen] = convTable2BitTo8Bit[rawPixelRecBuffer[x].pix0];   ++bufferLineLen;
  }

  return;
}  /* ProcessRawPixelRecs */



void  ScannerFile2BitEncoded::GetNextScanLine (uchar*   lineBuff,
                                               kkuint32 lineBuffSize
                                              )
{
  bool   eol = false;
  uchar  opCode = 0;
  OpRec  rec;
  OpRec  rec2;
  size_t recsRead = 0;

  kkuint32  bufferLineLen = 0;

  do
  {
    recsRead = fread (&rec, sizeof (rec), 1, file);
    if  (recsRead == 0)
    {
      break;
    }

    opCode = rec.textBlock.opCode;

    if  (opCode == 0)
      eol = true;

    else if  (bufferLineLen >= lineBuffSize)
    {
      // Something has gone wrong,  we should have encountered a eol op code before this point;  that is the 
      // length of the encoded line is exceeding the scan line length.
      eol = true;
      ungetc (rec.textChar, file);
      break;
    }


    else if  (opCode == 1)
      ProcessTextBlock (rec);

    else if  ((opCode >= 4)  &&  (opCode <= 7))
    {
      // OpCode determines PixelValue OpCode(4)=PV(0), OpCode(5)=PV(1), OpCode(6)=PV(2), OpCode(7)=PV(3),
      kkuint32  encodedChar = opCode - 4;
      runLenChar = convTable2BitTo8Bit [encodedChar];
      runLen     = rec.runLenPVx.len + 3;
      kkuint32  newLineSize = bufferLineLen + runLen;

      if  (newLineSize > lineBuffSize)
      {
         cerr << "ScannerFile2BitEncoded::GetNextScanLine    *** Line Length Exceeded ****" << endl;
      }
      else
      {
        memset (&(lineBuff[bufferLineLen]), runLenChar,  runLen);
        bufferLineLen = newLineSize;
      }
    }

    else if  (opCode == 8)
    {
      // Variable Run Length 0 thru 1023
      recsRead = fread (&rec2, sizeof (rec2), 1, file);
      if  (recsRead < 1)
      {
        eol = true;
      }
      else
      {
        kkuint32  runLenOpCode8 = 256 * (kkuint32)rec.runLen10Bit.lenHigh + (kkuint32)rec2.runLen10Bit_2.lenLow;
        kkuint32  newLineSize = bufferLineLen + runLenOpCode8;
        if  (newLineSize > lineBuffSize)
        {
          cerr << "ScannerFile2BitEncoded::GetNextScanLine    *** Line Length Exceeded ****" << endl;
          runLenOpCode8 = lineBuffSize - bufferLineLen;
          newLineSize = bufferLineLen;
        }

        uchar  runLenChar2To8 = convTable2BitTo8Bit [rec.runLen10Bit.pix];
        memset (&(lineBuff[bufferLineLen]), runLenChar2To8, runLenOpCode8);
        bufferLineLen = newLineSize;
      }
    }


    else if  (opCode == 10)
    {
      // One Raw Pixel
      lineBuff[bufferLineLen] = convTable2BitTo8Bit [rec.rawPixelOne.pix1];
      ++bufferLineLen;
    }


    else if  (opCode == 11)
    {
      // Two Raw Pixels.
      lineBuff[bufferLineLen] = convTable2BitTo8Bit [rec.rawPixelsTwo.pix1];  ++bufferLineLen;
      lineBuff[bufferLineLen] = convTable2BitTo8Bit [rec.rawPixelsTwo.pix2];  ++bufferLineLen;
    }


    else if  (opCode == 12)
    {
      // Variable Length Raw Pixels where string length = (3 thru 18)
      kkuint32  numRawPixels = rec.rawPixelsVarLen4Bit.len + 3;
      ProcessRawPixelRecs ((kkuint16)numRawPixels, lineBuff, lineBuffSize, bufferLineLen);
    }


    else if  (opCode == 13)
    {
      // Variable Length Raw Pixels where string length = (0 thru 4095)

      // Variable Run Length 0 thru 1023
      recsRead = fread (&rec2, sizeof (rec2), 1, file);
      if  (recsRead < 1)
      {
        eol = true;
      }
      else
      {
        kkuint32  numRawPixels = 256 * rec.rawPixelsVarLen12Bit.lenHigh + rec2.rawPixelsVarLen12Bit_2.lenLow;
        ProcessRawPixelRecs ((kkuint16)numRawPixels, lineBuff, lineBuffSize, bufferLineLen);
      }
    }

    else
    {
      cerr << "ScannerFile2BitEncoded::GetNextScanLine    *** Invalid OpCode[" << opCode<< "] Encountered." << endl; 
    }

  }  while  (!eol);

}  /* GetNextScanLine */



/*****************************************************************************/
/*   Following routines are used for writing Scanner Files.                  */
/*****************************************************************************/

void  ScannerFile2BitEncoded::WriteBufferFrame ()
{
  frameBufferFileOffsetLast = osFTELL (file);
  //nextScanLineOffset = osFTELL (file);

  kkuint32  frameRow = 0;
  uchar*  framePtr = frameBuffer;
  while  (frameRow < frameBufferNextLine)
  {
    WriteNextScanLine (framePtr, pixelsPerScanLine);
    framePtr += pixelsPerScanLine;
    ++frameRow;
  }

  frameBufferFileOffsetNext = osFTELL (file);
  fileSizeInBytes = frameBufferFileOffsetNext;
}  /* WriteBufferFrame*/



void  ScannerFile2BitEncoded::AddCurRunLenToOutputBuffer ()
{
  OpRec  rec;

  while  (runLen > 0)
  {
    if  (runLen == 1)
    {
      curCompStatus = csRaw;
      rawStr[rawStrLen] = runLenChar;
      ++rawStrLen;
      runLen = 0;
    }

    else if  (runLen < 2)
    {
#include "DisableConversionWarning.h"
      rec.rawPixelOne.opCode = 10;
      rec.rawPixelOne.pix1 = runLenChar;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else if  (runLen < 3)
    {
#include "DisableConversionWarning.h"
      rec.rawPixelsTwo.opCode = 11;
      rec.rawPixelsTwo.pix1 = runLenChar;
      rec.rawPixelsTwo.pix2 = runLenChar;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else if  (runLen < 19)
    {
#include "DisableConversionWarning.h"
      rec.runLenPVx.opCode = 4 + runLenChar;
      rec.runLenPVx.len = runLen - 3;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else
    {
      kkint32  runLenThisLoop = Min (toint32_t (1023), runLen);

#include "DisableConversionWarning.h"
      rec.runLen10Bit.opCode = 8;
      rec.runLen10Bit.pix = runLenChar;
      rec.runLen10Bit.lenHigh = runLenThisLoop / 256;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;

#include "DisableConversionWarning.h"
      rec.runLen10Bit_2.lenLow = runLenThisLoop % 256;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = runLen - runLenThisLoop;
    }
  }
}  /* AddCurRunLenToOutputBuffer */




void  ScannerFile2BitEncoded::AddRawStrPixelsToEncodedBuffer (kkuint16&  nextCp, 
                                                              kkuint16   len
                                                             )
{
  OpRec  rec;

  while  (len > 0)
  {
    if  (len > 3)
    {
#include "DisableConversionWarning.h"
      rec.rawPixelRec.pix0 = rawStr[nextCp];  ++nextCp;
      rec.rawPixelRec.pix1 = rawStr[nextCp];  ++nextCp;
      rec.rawPixelRec.pix2 = rawStr[nextCp];  ++nextCp;
      rec.rawPixelRec.pix3 = rawStr[nextCp];  ++nextCp;
#include "RestoreConversionWarning.h"
      len = (kkuint16)(len - 4);
    }
    else
    {
#include "DisableConversionWarning.h"
      rec.rawPixelRec.pix0 = rawStr[nextCp];  ++rawStr;
      if  (len > 1)  {rec.rawPixelRec.pix1 = rawStr[nextCp];  ++nextCp;}
      if  (len > 2)  {rec.rawPixelRec.pix2 = rawStr[nextCp];  ++nextCp;}
      if  (len > 3)  {rec.rawPixelRec.pix3 = rawStr[nextCp];  ++nextCp;}
#include "RestoreConversionWarning.h"
      len = 0;
    }

    // KAK 2014-May-06    Was missing the next two lines;  I need to verify if this is correct.
    *encodedBuffNext = rec;
    ++encodedBuffNext;
  }
}  /* AddRawStrPixelsToEncodedBuffer */



void  ScannerFile2BitEncoded::AddCurRawStrToOutputBuffer ()
{
  OpRec  rec;

  kkuint16  nextCp = 0;
  kkuint16  len = rawStrLen;

  while  (len > 0)
  {
    if  (len < 2)
    {
#include "DisableConversionWarning.h"
      rec.rawPixelOne.opCode = 10;
      rec.rawPixelOne.pix1 = rawStr[nextCp];
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      len = 0;
      ++nextCp;
    }

    else if  (len < 3)
    {
      rec.rawPixelsTwo.opCode = 11;
#include "DisableConversionWarning.h"
      rec.rawPixelsTwo.pix1 = rawStr[nextCp];  ++nextCp;
      rec.rawPixelsTwo.pix2 = rawStr[nextCp];  ++nextCp;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      len = 0;
    }


    else if  (len < 19)
    {
#include "DisableConversionWarning.h"
      rec.rawPixelsVarLen4Bit.opCode = 12;
      rec.rawPixelsVarLen4Bit.len = len - 3;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      AddRawStrPixelsToEncodedBuffer (nextCp, len);
      len = 0;
    }

    else
    {
      kkuint16  rawPixelsThisLoop = Min (len, (kkuint16)4095);
      kkuint16  lenHigh = rawPixelsThisLoop / 256;
      kkuint16  lenLow  = rawPixelsThisLoop % 256;

#include "DisableConversionWarning.h"
      rec.rawPixelsVarLen12Bit.opCode = 13;
      rec.rawPixelsVarLen12Bit.lenHigh = (uchar)lenHigh;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;

#include "DisableConversionWarning.h"
      rec.rawPixelsVarLen12Bit_2.lenLow = (uchar)lenLow;
#include "RestoreConversionWarning.h"
      *encodedBuffNext = rec;
      ++encodedBuffNext;

      AddRawStrPixelsToEncodedBuffer (nextCp, rawPixelsThisLoop);
      len = (kkuint16)(len - rawPixelsThisLoop);
    }
  }

  rawStrLen = 0;
}  /* AddCurRawStrToOutputBuffer */



void  ScannerFile2BitEncoded::WriteNextScanLine (const uchar*  buffer,
                                                 kkuint32      bufferLen
                                                )
{
  encodedBuffNext = encodedBuff;
  encodedBuffLen  = 0;
  kkuint32 encodedBuffLeft = encodedBuffSize  - encodedBuffLen;
  if  (bufferLen > (encodedBuffLeft - 10))
  {
    kkuint32  newEncodedBuffSise = bufferLen + 10;
    ReSizeEncodedBuff (newEncodedBuffSise);
  }

  curCompStatus = csNull;

  for  (kkuint16 x = 0;  x < bufferLen;  ++x)
  {
    uchar   nextCh = convTable8BitTo2Bit[buffer[x]];

    switch  (curCompStatus)
    {
    case  csNull:
      {
        runLen = 1;
        runLenChar = nextCh;
        curCompStatus = csRunLen;
      }
      break;

    case  csRunLen:
      {
        if  (nextCh == runLenChar)
        {
          ++runLen;
        }
        else if  (runLen == 1)
        {
          rawStr[0] = runLenChar;
          rawStr[1] = nextCh;
          rawStrLen = 2;
          runLen = 0;
          curCompStatus = csRaw;
        }
        else
        {
          AddCurRunLenToOutputBuffer ();
          runLenChar = nextCh;
          runLen = 1;
        }
      }
      break;

    case  csRaw:
      {
        if  (rawStrLen == 0)
        {
          rawStr[rawStrLen] = nextCh;
          ++rawStrLen;
        }

        if  (rawStr[rawStrLen - 1] != nextCh)
        {
          rawStr[rawStrLen] = nextCh;
          ++rawStrLen;
        }
        else
        {
          --rawStrLen;
          if  (rawStrLen > 0)
            AddCurRawStrToOutputBuffer ();

          runLen = 2;
          runLenChar = nextCh;
          curCompStatus = csRunLen;
        }
      }
      break;
    }
  }

  if  (curCompStatus == csRunLen)
    AddCurRunLenToOutputBuffer ();

  if  (curCompStatus == csRaw)
    AddCurRawStrToOutputBuffer ();

  OpRec  rec;
#include "DisableConversionWarning.h"
  rec.endOfScanLine.opCode = 0;
  rec.endOfScanLine.filler = 0;
#include "RestoreConversionWarning.h"
  *encodedBuffNext = rec;
  ++encodedBuffNext;

  encodedBuffLen = (kkint32)(encodedBuffNext - encodedBuff);
  fwrite (encodedBuff, sizeof (OpRec), encodedBuffLen, file);


  /**@todo Write out OutputBuffer;  */

}  /* WriteNextScanLine */



void  ScannerFile2BitEncoded::WriteTextBlock (const uchar*  txtBlock,
                                              kkuint32      txtBlockLen
                                             )
{
  kkint32  charsLeft = txtBlockLen;
  const uchar*  txtBlockPtr = txtBlock;
  while  (charsLeft > 0)
  {
    kkint32  charsToWrite = Min ((kkint32)2048, charsLeft);
    OpRec  rec;

#include "DisableConversionWarning.h"
    rec.textBlock.opCode     = 1;
    rec.textBlock.endOfText  = (charsToWrite < charsLeft) ? 0 : 1;
    rec.textBlock.lenHigh    = charsToWrite / 256;
#include "RestoreConversionWarning.h"
    *encodedBuffNext = rec;
    ++encodedBuffNext;

#include "DisableConversionWarning.h"
    encodedBuffNext->textBlock_2.lenLow = charsToWrite % 256;
#include "RestoreConversionWarning.h"

    ++encodedBuffNext;

    memcpy (encodedBuffNext, txtBlockPtr, charsToWrite);

    charsLeft       -= charsToWrite;
    txtBlockPtr     += charsToWrite;
    encodedBuffNext += charsToWrite;
  }

  //nextScanLineOffset = osFTELL (file);
  fileSizeInBytes = osFTELL (file);
}  /* WriteTextBlock */
