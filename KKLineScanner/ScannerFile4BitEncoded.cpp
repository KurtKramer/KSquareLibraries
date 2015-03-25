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

#include  "ScannerFile4BitEncoded.h"

using namespace  KKLSC;

#pragma pack(push,1)
struct  ScannerFile4BitEncoded::OpRecEndOfScanLine
{
  uchar opCode:  4;  /** 0 = End of Scan Line. */
  uchar filler:  4;
};



struct  ScannerFile4BitEncoded::OpRecTextBlock1
{
  /** Will handle text blocks in one or more sections with 2^11 characters in each *
   *  The last section will have 'endOfText' == '1'                                 */

  uchar opCode:      4;  /**< 1 = Text Block.                                      */
  uchar endOfText:   1;  /**< 0 = More Text blocks to follow, 1 = last text block. */
  uchar lenHighBits: 3;  /**< Len = 16 * lenHighBits + lenLowBits                  */
};  /* OpRecTextBlock1 */
  


struct  ScannerFile4BitEncoded::OpRecTextBlock2
{
  uchar lenLowBits: 8;     /**< Add this value to the prev rec 'lenHighBits' field to get the length of the text block. */
};



struct  ScannerFile4BitEncoded::OpRecInstrumentDataWord1
{
  uchar  opCode: 4;  /**< 2 = InstrumentDataWord  32 Bit Instrument Data */
  uchar  idNum:  4;
};



struct  ScannerFile4BitEncoded::OpRecInstrumentDataWord2
{
  OpRecInstrumentDataWord2 (): scanLineNum (0) {}
  kkuint32  scanLineNum;
};  /**<  OpCode 2  */



struct  ScannerFile4BitEncoded::OpRecInstrumentDataWord3
{
  OpRecInstrumentDataWord3 (): word () {}
  WordFormat32Bits  word;
};  /**<  OpCode 2  */




struct  ScannerFile4BitEncoded::OpRecRunLen
{
  /** For one or two raw pixels */
  uchar  opCode     :4;      /**< 4 =   2 Pixels
                              *   5 =   3 Pixels
                              *   6 =   4 Pixels
                              *   7 =   5 Pixels
                              *   8 =   6 Pixels
                              *   9 =   7 Pixels
                              */
  uchar  pixelValue :4;
};  /* OpRecRunLen */



struct  ScannerFile4BitEncoded::OpRecRun256Len1
{
  /** For one or two raw pixels */
  uchar  opCode     :4;      /**< 10 = up to 256 run-length  */
  uchar  pixelValue :4;
};  /* OpRecRunLen */



struct  ScannerFile4BitEncoded::OpRecRun256Len2
{
  uchar  runLen     :8;      /**< 0= 1-Pixel, 1=2-Pixels, ... 255=256-Pixels */
};  /* OpRecRunLen */



struct  ScannerFile4BitEncoded::OpRecRaw1Pixel
{
  uchar  opCode     :4;      /**< 11 = One raw pixel      */
  uchar  pixelValue :4;
};  /* OpRecRaw1Pixel */



struct  ScannerFile4BitEncoded::OpRecRaw32Pixels
{
  uchar  opCode  :4;      /**< 12 = Even run length 2 thru 32 pixels      */
  uchar  len     :4;      /**< Num-Raw-Pixels = (len + 1) * 2   */
  /** To be followed by 'len' RawPixel recs */
};  /* OpRecRunLen1Bit */



struct  ScannerFile4BitEncoded::OpRecRaw513Pixels1
{
  uchar  opCode  :4;      /**< 13 = Odd number of pixels, up to 513 pixels.          */
  uchar  lenHigh :4;      /**< Num-Raw-Pixels = 1 + 2 * (1 + lenHigh * 16 + lenLow)  */
  /** To be followed by length RawPixelRec's */
};  /* OpRecRaw513Pixels1 */



struct  ScannerFile4BitEncoded::OpRecRaw513Pixels2
{
  uchar  lenLow :4;      /**< 13 = Odd number of pixes, up to 513 pixels.    */
  uchar  pix0   :4;      /**< 1st Raw pixel  */

  /** To be followed by (1 + lenHigh * 16 + lenLow) 'RawPixelRecs' */
};  /* OpRecRaw513Pixels2 */



struct  ScannerFile4BitEncoded::RawPixelRec
{
  uchar  pix0   :4;
  uchar  pix1   :4;
};  /* OpRecord */



union  ScannerFile4BitEncoded::OpRec
{
  OpRecEndOfScanLine         endOfScanLine;        /**< OpCode 0         */
  
  OpRecTextBlock1            textBlock1;           /**< OpCode 1         */
  OpRecTextBlock2            textBlock2;
  
  OpRecInstrumentDataWord1   instrumentDataWord1;  /**<  OpCode 2        */
  OpRecRunLen                runLen;               /**< OpCodes 4 thru 9 */

  OpRecRun256Len1            run256Len1;           /**< OpCode 10        */
  OpRecRun256Len2            run256Len2;
  
  OpRecRaw1Pixel             raw1Pixel;            /**< OpCode 11        */
  
  OpRecRaw32Pixels           raw32Pixels;          /**< OpCode 12        */

  OpRecRaw513Pixels1         raw513Pixels1;        /**< OpCode 13        */
  OpRecRaw513Pixels2         raw513Pixels2;

  RawPixelRec                rawPixels;            /* Two 4 bit pixels   */

  uchar                      textChar;
};  /* OpRec */



#pragma pack(pop)


uchar*  ScannerFile4BitEncoded::convTable4BitTo8Bit = NULL;
uchar*  ScannerFile4BitEncoded::convTable8BitTo4Bit = NULL;
uchar*  ScannerFile4BitEncoded::compensationTable   = NULL;


ScannerFile4BitEncoded::ScannerFile4BitEncoded (const KKStr&  _fileName,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _log),
  rawPixelRecBuffer     (NULL),
  rawPixelRecBufferSize (0),
  rawPixelRecBufferLen  (0),

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
  PrintSizeInfo ();
}



ScannerFile4BitEncoded::ScannerFile4BitEncoded (const KKStr&  _fileName,
                                                kkuint32      _pixelsPerScanLine,
                                                kkuint32      _frameHeight,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _pixelsPerScanLine, _frameHeight, _log),
  rawPixelRecBuffer     (NULL),
  rawPixelRecBufferSize (0),
  rawPixelRecBufferLen  (0),
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
  AllocateRawStr (_pixelsPerScanLine + 100);
}



void  ScannerFile4BitEncoded::PrintSizeInfo ()
{
  /*
  #pragma pack(1)
  int x01 = sizeof (OpRecEndOfScanLine  ) ;
  int x02 = sizeof (OpRecTextBlock1     ) ;
  int x03 = sizeof (OpRecTextBlock2     ) ;
  int x04 = sizeof (OpRecRunLen         ) ;
  int x05 = sizeof (OpRecRun256Len1     ) ;
  int x06 = sizeof (OpRecRun256Len2     ) ;
  int x07 = sizeof (OpRecRaw1Pixel      ) ;
  int x08 = sizeof (OpRecRaw32Pixels    ) ;
  int x09 = sizeof (OpRecRaw513Pixels1  ) ;
  int x10 = sizeof (OpRecRaw513Pixels2  ) ;
  int x11 = sizeof (RawPixelRec         ) ;
  int x12 = sizeof (OpRec );


  cout << endl
       <<   "OpRecEndOfScanLine  " << "\t" << sizeof (OpRecEndOfScanLine  ) << endl
       <<   "OpRecTextBlock1     " << "\t" << sizeof (OpRecTextBlock1     ) << endl
       <<   "OpRecTextBlock2     " << "\t" << sizeof (OpRecTextBlock2     ) << endl
       <<   "OpRecRunLen         " << "\t" << sizeof (OpRecRunLen         ) << endl
       <<   "OpRecRun256Len1     " << "\t" << sizeof (OpRecRun256Len1     ) << endl
       <<   "OpRecRun256Len2     " << "\t" << sizeof (OpRecRun256Len2     ) << endl
       <<   "OpRecRaw1Pixel      " << "\t" << sizeof (OpRecRaw1Pixel      ) << endl
       <<   "OpRecRaw32Pixels    " << "\t" << sizeof (OpRecRaw32Pixels    ) << endl
       <<   "OpRecRaw513Pixels1  " << "\t" << sizeof (OpRecRaw513Pixels1  ) << endl
       <<   "OpRecRaw513Pixels2  " << "\t" << sizeof (OpRecRaw513Pixels2  ) << endl
       <<   "RawPixelRec         " << "\t" << sizeof (RawPixelRec         ) << endl
       << endl;
  */
}    



ScannerFile4BitEncoded::~ScannerFile4BitEncoded (void)
{
  if  (opened)
    Close ();

  delete encodedBuff;        encodedBuff       = NULL;
  delete rawPixelRecBuffer;  rawPixelRecBuffer = NULL;
  delete rawStr;             rawStr            = NULL;
}




void  ScannerFile4BitEncoded::AllocateRawPixelRecBuffer (kkuint32 size)
{
  delete  rawPixelRecBuffer;
  rawPixelRecBuffer     = new RawPixelRec[size];
  rawPixelRecBufferSize = size;
  rawPixelRecBufferLen  = 0;
}  /* AllocateRawPixelRecBuffer */



void  ScannerFile4BitEncoded::AllocateRawStr (kkuint16  size)
{
  if  (rawStr)
  {
    uchar*    newRawStr = new uchar[size];
    kkuint16  bytesToCopy = Min (rawStrLen, size);
    memcpy (newRawStr, rawStr, bytesToCopy);
    delete rawStr;
    rawStr = NULL;
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




void  ScannerFile4BitEncoded::AllocateEncodedBuff ()
{
  delete  encodedBuff;  encodedBuff  = NULL;

  kkint32  frameWidth = Max ((kkuint32)2048, 4 * PixelsPerScanLine ());
  
  encodedBuffSize = (int)(1.2f * (float)frameWidth);

  encodedBuff = new OpRec[encodedBuffSize];
  encodedBuffNext = encodedBuff;
  //encodedBuffSize = encodedBuffSize;
  encodedBuffLen  = 0;
}  /* AllocateEncodedBuff */



void  ScannerFile4BitEncoded::ReSizeEncodedBuff (kkuint32  newSize)
{
  OpRecPtr  newEncodedBuff = new OpRec[newSize];

  kkuint32  recsToMove = Min (newSize, encodedBuffLen);

  memcpy (newEncodedBuff, encodedBuff, recsToMove * sizeof (OpRec));
  encodedBuffNext = newEncodedBuff + (encodedBuffNext - encodedBuff);

  delete  encodedBuff;
  encodedBuff = newEncodedBuff;
  encodedBuffSize = newSize;
}  /* ReSizeEncodedBuff */



void  ScannerFile4BitEncoded::BuildConversionTables ()
{
  GlobalGoalKeeper::StartBlock ();
  if  (!convTable4BitTo8Bit)
  {
    kkint32 x = 0;
    kkint32 y = 0;
    kkint32 inc = 256 / 16;
    convTable4BitTo8Bit = new uchar[16];
    convTable8BitTo4Bit = new uchar[256];
    for  (x = 0;  x < 16;  ++x)
    {
      kkint32  this8Bit = x * inc;
      kkint32  next8Bit = (x + 1) * inc;

      kkint32  fourBitTo8BitNum = (kkint32)(this8Bit + (kkint32)(((float)x / 16.0f)  * (float)inc));

      convTable4BitTo8Bit[x] = (uchar)fourBitTo8BitNum;
      for  (y = this8Bit;  y < next8Bit;  ++y)
        convTable8BitTo4Bit[y] = x;
    }

    compensationTable = new uchar[256];
    for  (kkint16 pv = 0;  pv < 256;  ++pv)
    {
      uchar encodedValue = convTable8BitTo4Bit[pv];
      compensationTable[pv] = convTable4BitTo8Bit[encodedValue];
    }
    atexit (ScannerFile4BitEncoded::ExitCleanUp);
  }

  GlobalGoalKeeper::EndBlock ();
}  /* BuildConversionTables */



const uchar*  ScannerFile4BitEncoded::CompensationTable ()
{
  GlobalGoalKeeper::StartBlock ();
  if  (!compensationTable)
    BuildConversionTables ();
  GlobalGoalKeeper::EndBlock ();
  return  compensationTable;
}  /* CompensationTable */





void  ScannerFile4BitEncoded::ExitCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();
  delete  convTable4BitTo8Bit;  convTable4BitTo8Bit = NULL;
  delete  convTable8BitTo4Bit;  convTable8BitTo4Bit = NULL;
  delete  compensationTable;    compensationTable   = NULL;
  GlobalGoalKeeper::EndBlock ();
}




void  ScannerFile4BitEncoded::ScanRate (float  _scanRate)
{
  ScannerFile::ScanRate (_scanRate);
}



/*****************************************************************************/
/*        Following routines are used for READING Scanner Files.             */
/*****************************************************************************/


kkuint32  ScannerFile4BitEncoded::ReadBufferFrame ()
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
 


kkint64  ScannerFile4BitEncoded::SkipToNextFrame ()
{
  uchar*  scanLine = new uchar[pixelsPerScanLine];

  kkuint32  numScanLinesReadThisFrameBuffer = 0;
  while  ((feof (file) == 0)  &&  (numScanLinesReadThisFrameBuffer < frameHeight))
  {
    GetNextScanLine (scanLine, pixelsPerScanLine);
    ++numScanLinesReadThisFrameBuffer;
  }

  delete  scanLine;
  scanLine = NULL;

  if  (feof (file) != 0)
    return -1;
  else
    return osFTELL (file);
}  /* SkipToNextFrame */



void  ScannerFile4BitEncoded::ProcessTextBlock (const OpRec&  rec)
{
  OpRec  rec2;

  kkuint32  recsRead = fread (&rec2, sizeof (rec2), 1, file);
  if  (recsRead < 1)
  {
    eof = true;
    return;
  }

   kkuint32  numTextBytes = 256 * rec.textBlock1.lenHighBits + rec2.textBlock2.lenLowBits;
   char* textMsgPtr = new char[numTextBytes + 1];  // "+ 1" for terminating NULL Character.
   kkuint32 textMsgLen = numTextBytes;
   
   recsRead = fread (textMsgPtr, 1, numTextBytes, file);
   if  (recsRead < numTextBytes)
      eof = true;
   else
   {
     textMsgPtr[recsRead] = 0;
     ReportTextMsg (textMsgPtr, textMsgLen);
   }
   delete  textMsgPtr;  textMsgPtr = NULL;
   textMsgLen = 0;
}  /* ProcessTextBlock */



void  ScannerFile4BitEncoded::ProcessInstrumentDataWord (const OpRec&  rec)
{
  uchar  idNum = rec.instrumentDataWord1.idNum;

  OpRecInstrumentDataWord2  rec2;
  OpRecInstrumentDataWord3  rec3;
  kkuint32  recsRead = fread (&rec2, sizeof (rec2), 1, file);
  if  (recsRead < 1)
    eof = true;
  else
  {
    recsRead = fread (&rec3, sizeof (rec3), 1, file);
    if  (recsRead < 1)
      eof = true;
    else
      ReportInstrumentDataWord (idNum, rec2.scanLineNum, rec3.word);
  }
}  /* ProcessInstrumentDataWord */





void  ScannerFile4BitEncoded::ProcessRawPixelRecs (kkuint16  numRawPixelRecs,
                                                   uchar*    lineBuff,
                                                   kkuint32  lineBuffSize,
                                                   kkuint32& bufferLineLen
                                                  )
{
  if  (numRawPixelRecs > rawPixelRecBufferSize)
    AllocateRawPixelRecBuffer (numRawPixelRecs + 30);

  size_t  recsRead = fread (rawPixelRecBuffer, sizeof (RawPixelRec), numRawPixelRecs, file);
  if  (recsRead < numRawPixelRecs)
  {
    eof = true;
    return;
  }

  for  (kkuint32 x = 0;  x < numRawPixelRecs;  ++x)
  {
    if  (bufferLineLen < lineBuffSize)  {lineBuff[bufferLineLen] = convTable4BitTo8Bit[rawPixelRecBuffer[x].pix0];   ++bufferLineLen;}
    if  (bufferLineLen < lineBuffSize)  {lineBuff[bufferLineLen] = convTable4BitTo8Bit[rawPixelRecBuffer[x].pix1];   ++bufferLineLen;}
  }

  return;
}  /* ProcessRawPixelRecs */



void  ScannerFile4BitEncoded::GetNextScanLine (uchar* lineBuff,
                                               kkuint32 lineBuffSize
                                              )
{
  bool   eol = false;
  uchar  opCode = 0;
  OpRec  rec;
  OpRec  rec2;
  kkuint32 recsRead = 0;

  kkuint32  bufferLineLen = 0;

  do
  {
    recsRead = fread (&rec, sizeof (rec), 1, file);
    if  (recsRead == 0)
    {
      break;
    }

    opCode = rec.textBlock1.opCode;

    if  (opCode == 0)
      eol = true;

    else if  (bufferLineLen >= lineBuffSize)
    {
      // Something has gone wrong,  we should have encountered a eol opCode before this point;  that is the
      // length of the encoded line is exceeding the scan line length.
      eol = true;
      ungetc (rec.textChar, file);
      break;
    }


    else if  (opCode == 1)
      ProcessTextBlock (rec);

    else if  (opCode == 2)
      ProcessInstrumentDataWord (rec);

    else if  ((opCode >= 4)  &&  (opCode <= 9))
    {
      // OpCode determines RunLen  (4=2, 5=3, ... 9=7)
      kkuint32 runLen = opCode - 2;
      runLenChar = convTable4BitTo8Bit [rec.runLen.pixelValue];
      kkuint32  newLineSize = bufferLineLen + runLen;

      if  (newLineSize > lineBuffSize)
      {
        cerr << "ScannerFile4BitEncoded::GetNextScanLine   ***ERROR***  Exceeding 'bufferLineLen';  ScanLine[" << nextScanLine << "]." << endl;
        cerr << "                         newLineSize: " << newLineSize << endl;
      }
      else
      {
        memset (&(lineBuff[bufferLineLen]), runLenChar,  runLen);
        bufferLineLen = newLineSize;
      }
    }

    else if  (opCode == 10)  /* OpRecRun256Len1 */
    {
      recsRead = fread (&rec2, sizeof (rec2), 1, file);
      if  (recsRead < 1)
        eol = true;
      else
      {
        kkuint32  runLen = 1 + rec2.run256Len2.runLen;
        kkuint32  newLineSize = bufferLineLen + runLen;
        if  (newLineSize > lineBuffSize)
        {
          cerr << "ScannerFile4BitEncoded::GetNextScanLine   ***ERROR***  Exceeding 'bufferLineLen';  ScanLine[" << nextScanLine << "]." << endl;
        }
        else
        {
          uchar  runLenChar = convTable4BitTo8Bit [rec.run256Len1.pixelValue];
          memset (&(lineBuff[bufferLineLen]), runLenChar,  runLen);
          bufferLineLen = newLineSize;
        }
      }
    }

    else if  (opCode == 11)
    {
      if  (bufferLineLen < lineBuffSize)
      {
        lineBuff[bufferLineLen] = convTable4BitTo8Bit[rec.raw1Pixel.pixelValue];
        ++bufferLineLen;
      }
      else
      {
        cerr << "ScannerFile4BitEncoded::GetNextScanLine   ***ERROR***  Exceeding 'bufferLineLen';  ScanLine[" << nextScanLine << "]." << endl;
      }
    }

    else if  (opCode == 12)
    {
      kkuint16  numRawRecs = rec.raw32Pixels.len + 1;   // We add 1 to 'numRawRecs' because '1' was subtracted out when written to Scanner File.
      kkuint16  numRawPixels = numRawRecs * 2;
      kkuint16  newBufferLineLen = bufferLineLen + numRawPixels;
      if  (newBufferLineLen > lineBuffSize)
      {
        cerr << "ScannerFile4BitEncoded::GetNextScanLine   ***ERROR***  Exceeding 'bufferLineLen';  ScanLine[" << nextScanLine << "]." << endl;
      }
      ProcessRawPixelRecs (numRawRecs, lineBuff, lineBuffSize, bufferLineLen);
    }

    else if  (opCode == 13)
    {
      recsRead = fread (&rec2, sizeof (rec2), 1, file);
      if  (recsRead < 1)
      {
        eol = true;
        eof = true;
      }
      else
      {
        kkuint16  numRawRecs = 1 + 16 * (kkuint16)(rec.raw513Pixels1.lenHigh) + (kkuint16)(rec2.raw513Pixels2.lenLow);
        kkuint16  numRawPixels = 1 + 2 * numRawRecs;
        kkuint16  newBufferLineLen = bufferLineLen + numRawPixels;
        if  (newBufferLineLen > lineBuffSize)
        {
          cerr << "ScannerFile4BitEncoded::GetNextScanLine   ***ERROR***  Exceeding 'bufferLineLen';  ScanLine[" << nextScanLine << "]." << endl;
        }

        if  (bufferLineLen < lineBuffSize)  
        {
          lineBuff[bufferLineLen] = convTable4BitTo8Bit[rec2.raw513Pixels2.pix0];  
          ++bufferLineLen;
        }
      
        ProcessRawPixelRecs (numRawRecs, lineBuff, lineBuffSize, bufferLineLen);
      }
    }
  }  while  (!eol);

}  /* GetNextScanLine */



/*****************************************************************************/
/*   Following routines are used for writing Scanner Files.                  */
/*****************************************************************************/

void  ScannerFile4BitEncoded::WriteBufferFrame ()
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



void  ScannerFile4BitEncoded::AddCurRunLenToOutputBuffer ()
{
  OpRec  rec;

  while  (runLen > 0)
  {
    if  (runLen == 1)
    {
      // We will treat this as a 1 pixel long raw string.
      rec.raw1Pixel.opCode = 11;
      rec.raw1Pixel.pixelValue = runLenChar;
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else if  (runLen < 8)
    {
      rec.runLen.opCode     = runLen  + 2;
      rec.runLen.pixelValue = runLenChar;
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else if  (runLen < 257)
    {
      ushort encodedLen = runLen - 1;

      rec.run256Len1.opCode     = 10;
      rec.run256Len1.pixelValue = runLenChar;
      *encodedBuffNext = rec;
      ++encodedBuffNext;

      rec.run256Len2.runLen = encodedLen;
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen = 0;
    }

    else
    {
      rec.run256Len1.opCode = 10;
      rec.run256Len1.pixelValue = runLenChar;
      *encodedBuffNext = rec;
      ++encodedBuffNext;

      rec.run256Len2.runLen = 255;
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      runLen -= 256;
    }
  }
}  /* AddCurRunLenToOutputBuffer */



void  ScannerFile4BitEncoded::AddCurRawStrToOutputBuffer ()
{
  OpRec  rec;

  kkuint16  nextCp = 0;
  kkuint16  len = rawStrLen;

  while  (len > 0)
  {
    if  (len < 2)
    {
      rec.raw1Pixel.opCode = 11;
      rec.raw1Pixel.pixelValue = rawStr[nextCp];
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      len = 0;
      ++nextCp;
    }

    else if  (len < 33)
    {
      ushort  rawPixelRecsNeeded = (ushort)(len / 2) - 1;  // We subtract 1 when writing;  when reading back file will add back in 1.
      rec.raw32Pixels.opCode = 12;
      rec.raw32Pixels.len  = rawPixelRecsNeeded;
      *encodedBuffNext = rec;
      ++encodedBuffNext;

      while  (len > 1)
      {
        rec.rawPixels.pix0 = rawStr[nextCp];  ++nextCp;
        rec.rawPixels.pix1 = rawStr[nextCp];  ++nextCp;
        *encodedBuffNext = rec;
        ++encodedBuffNext;
        len -= 2;
      }
    }

    else
    {
      kkuint16  lenToProcess = Min ((kkuint16)513, len);
      if  ((lenToProcess % 2) > 0)
      {
        // Can only handle odd number of pixels.
        --lenToProcess;
      }

      kkuint16  numRawRecsNeeded = (lenToProcess - 1) / 2;
      kkuint16  numRawRecsNeededEnc = numRawRecsNeeded - 1;  // When reading back and decoding file will add back 1.
      kkuint16  lenHigh = numRawRecsNeededEnc / 16;
      kkuint16  lenLow  = numRawRecsNeededEnc % 16;

      rec.raw513Pixels1.opCode  = 13;
      rec.raw513Pixels1.lenHigh = lenHigh;
      *encodedBuffNext = rec;
      ++encodedBuffNext;

      rec.raw513Pixels2.lenLow  = lenLow;
      rec.raw513Pixels2.pix0    = rawStr[nextCp];
      *encodedBuffNext = rec;
      ++encodedBuffNext;
      ++nextCp;
      --len;
      --lenToProcess;

      while  (lenToProcess > 1)
      {
        rec.rawPixels.pix0 = rawStr[nextCp];  ++nextCp;
        rec.rawPixels.pix1 = rawStr[nextCp];  ++nextCp;
        *encodedBuffNext = rec;
        ++encodedBuffNext;
        len          -= 2;
        lenToProcess -= 2;
      }
    }
  }

  rawStrLen = 0;
}  /* AddCurRawStrToOutputBuffer */



void  ScannerFile4BitEncoded::WriteNextScanLine (const uchar*  buffer,
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
    uchar   nextCh = convTable8BitTo4Bit[buffer[x]];

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

        else if  (rawStr[rawStrLen - 1] != nextCh)
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
  rec.endOfScanLine.opCode = 0;
  rec.endOfScanLine.filler = 0;
  *encodedBuffNext = rec;
  ++encodedBuffNext;

  encodedBuffLen = encodedBuffNext - encodedBuff;
  fwrite (encodedBuff, sizeof (OpRec), encodedBuffLen, file);
  fileSizeInBytes = osFTELL (file);

  /**@todo Write out OutputBuffer;  */

}  /* WriteNextScanLine */





void  ScannerFile4BitEncoded::WriteTextBlock (const uchar*  txtBlock,
                                              kkuint32      txtBlockLen
                                             )
{
  if  (txtBlockLen > (encodedBuffSize - 10))
  {
    kkuint32  newEncodedBuffSise = txtBlockLen + 10;
    ReSizeEncodedBuff (newEncodedBuffSise);
  }

  encodedBuffNext = encodedBuff;
  encodedBuffLen  = 0;
  kkint32  charsLeft = txtBlockLen;
  const uchar*  txtBlockPtr = txtBlock;
  while  (charsLeft > 0)
  {
    kkint32  charsToWrite = Min (2048, charsLeft);
    OpRec  rec;

    rec.textBlock1.opCode = 1;
    rec.textBlock1.endOfText    = (charsToWrite < charsLeft) ? 0 : 1;
    rec.textBlock1.lenHighBits  = charsToWrite / 256;
    *encodedBuffNext = rec;
    ++encodedBuffNext;

    encodedBuffNext->textBlock2.lenLowBits     = charsToWrite % 256;
    ++encodedBuffNext;

    memcpy (encodedBuffNext, txtBlockPtr, charsToWrite);

    charsLeft       -= charsToWrite;
    txtBlockPtr     += charsToWrite;
    encodedBuffNext += charsToWrite;
  }

  fwrite (encodedBuff, sizeof (OpRec), encodedBuffLen, file);
  fileSizeInBytes = osFTELL (file);
}  /* WriteTextBlock */


/*

void   ScannerFile4BitEncoded::WriteInstrumentDataWord (uchar             idNum,
                                                        kkuint32          scanLineNum,
                                                        WordFormat32Bits  dataWord
                                                       )
{
  OpRecInstrumentDataWord1  r1;
  r1.opCode = 2;
  r1.idNum  = idNum;
  fwrite (&r1, sizeof (r1), 1, file);

  OpRecInstrumentDataWord2  r2;
  r2.scanLineNum = scanLineNum;
  fwrite (&r2, sizeof (r2), 1, file);

  OpRecInstrumentDataWord3  r3;
  r3.word = dataWord;
  fwrite (&r3, sizeof (r3), 1, file);

  fileSizeInBytes = osFTELL (file);
} 
*/


