#include "FirstIncludes.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include  "MemoryDebug.h"

using namespace std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "ScannerFile.h"
#include "Variables.h"
#include "ScannerFile3BitEncoded.h"
using namespace  KKLSC;


struct  ScannerFile3BitEncoded::OpRecTextBlock  // OpCode = 0
{
  ushort  opCode          : 3;
  ushort  endOfTextBlock  : 1;   /**< Last text block in message.  If a message requires more than   *
                                  *   4096 bytes it will be written in multiple text blocks with     *
                                  *   the last one having this flag set to '1'.                      */
  ushort  length          : 12;  /**< Text Block length.  */
};  /* OpRecTextBlock */


struct  ScannerFile3BitEncoded::OpRec4RawPixels   // OpCode = 1
{
  ushort  opCode :3;
  ushort  eol    :1;
  ushort  pix0   :3;
  ushort  pix1   :3;
  ushort  pix2   :3;
  ushort  pix3   :3;
};  /* OpRec4RawPixels */


struct  ScannerFile3BitEncoded::OpRecSpaces     // OpCode = 2
{
  ushort  opCode     :3;
  ushort  eol        :1;
  ushort  num4Spaces :12;
};  /* OpRecSpaces */



struct  ScannerFile3BitEncoded::OpRecBlackOuts  // OpCode = 3
{
  ushort  opCode        :3;
  ushort  eol           :1;
  ushort  num4BlackOuts :12;
};  /* OpRecBlackOuts */



struct  ScannerFile3BitEncoded::OpRecRunLen     // OpCode = 4
{
  ushort  opCode  :3;
  ushort  eol     :1;
  ushort  runLen  :9;
  ushort  pix     :3;
};  /* OpRecBlackOuts */





union  ScannerFile3BitEncoded::OpRec
{
  OpRecTextBlock  textBlock;  // OpCode = 0
  OpRec4RawPixels rawPixels;  // OpCode = 1
  OpRecSpaces     spaces;     // OpCode = 2
  OpRecBlackOuts  blackOuts;  // OpCode = 3
  OpRecRunLen     runLen;
};  /* OpRec */


//#pragma pack(show)


uchar*  ScannerFile3BitEncoded::convTable3BitTo8Bit = NULL;
uchar*  ScannerFile3BitEncoded::convTable8BitTo3Bit = NULL;
uchar*  ScannerFile3BitEncoded::compensationTable   = NULL;


ScannerFile3BitEncoded::ScannerFile3BitEncoded (const KKStr&  _fileName,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _log),
  outputBuff     (NULL),
  outputBuffLen  (0),
  workLine       (NULL),
  workLineLen    (0)

{
  BuildConversionTables ();
  AllocateWorkLineAndOutputBuf ();
  memset (fourSpaces,    0, sizeof (fourSpaces));
  memset (fourBlackOuts, 7, sizeof (fourBlackOuts));   // In 3bit world  '7' = 255
}



ScannerFile3BitEncoded::ScannerFile3BitEncoded (const KKStr&  _fileName,
                                                kkuint32      _pixelsPerScanLine,
                                                kkuint32      _frameHeight,
                                                RunLog&       _log
                                               ):
  ScannerFile (_fileName, _pixelsPerScanLine, _frameHeight, _log),
  outputBuff    (NULL),
  outputBuffLen (0),
  workLine      (NULL),
  workLineLen   (0)

{
  BuildConversionTables ();
  AllocateWorkLineAndOutputBuf ();
  memset (fourSpaces,    0, sizeof (fourSpaces));
  memset (fourBlackOuts, 7, sizeof (fourBlackOuts));
}



ScannerFile3BitEncoded::~ScannerFile3BitEncoded (void)
{
  if  (opened)
    Close ();
  delete  outputBuff;  outputBuff = NULL;
  delete  workLine;    workLine   = NULL;
}



void  ScannerFile3BitEncoded::BuildConversionTables ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (!convTable3BitTo8Bit)
  {
    kkint32 x = 0;
    kkint32 y = 0;
    float inc = 255.0f / 8.0f;
    convTable3BitTo8Bit = new uchar[8];
    convTable8BitTo3Bit = new uchar[256];
    for  (x = 0;  x < 8;  ++x)
    {
      kkint32  this8Bit = (kkint32)(0.5f + (float)x * inc);
      kkint32  next8Bit = (kkint32)(0.5f + (float)(x + 1) * inc);

      kkint32  threeBitTo8BitNum = (kkint32)(this8Bit + (x / 7.0f)  * (next8Bit - this8Bit));

      convTable3BitTo8Bit[x] = (uchar)threeBitTo8BitNum;
      for  (y = this8Bit; y <= next8Bit;  ++y)
        convTable8BitTo3Bit[y] = x;
    }

    compensationTable = new uchar[256];
    for  (kkint16 pv = 0;  pv < 256;  ++pv)
    {
      uchar encodedValue = convTable8BitTo3Bit[pv];
      compensationTable[pv] = convTable3BitTo8Bit[encodedValue];
    }

    atexit (ScannerFile3BitEncoded::ExitCleanUp);
  }

  GlobalGoalKeeper::EndBlock ();
}  /* BuildConversionTables */



const uchar*  ScannerFile3BitEncoded::CompensationTable ()
{
  GlobalGoalKeeper::StartBlock ();
  if  (!compensationTable)
    BuildConversionTables ();
  GlobalGoalKeeper::EndBlock ();

  return compensationTable;
}



void  ScannerFile3BitEncoded::ExitCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();
  delete  convTable3BitTo8Bit;  convTable3BitTo8Bit = NULL;
  delete  convTable8BitTo3Bit;  convTable8BitTo3Bit = NULL;
  delete  compensationTable;    compensationTable   = NULL;
  GlobalGoalKeeper::EndBlock ();
}




void  ScannerFile3BitEncoded::AllocateWorkLineAndOutputBuf ()
{
  delete  outputBuff;  outputBuff  = NULL;
  delete  workLine;    workLine    = NULL;

  workLineLen = Max (1024U, pixelsPerScanLine);
  workLine = new uchar[workLineLen];

  outputBuffLen = workLineLen / 4 + 10;
  outputBuff = new OpRec[outputBuffLen];
}  /* AllocateWorkLineAndOutputBuf */




void  ScannerFile3BitEncoded::ScanRate (float  _scanRate)
{
  ScannerFile::ScanRate (_scanRate);
}




kkuint32  ScannerFile3BitEncoded::ReadBufferFrame ()
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

  bool  endOfFileFound = (feof (file) != 0);
  if  (endOfFileFound)
    cerr << "end of file was found." << endl;

  frameBufferFileOffsetNext = osFTELL (file);
  frameBufferNextLine = 0;
  return  numScanLinesReadThisFrameBuffer;
}  /* ReadBufferFrame */
 


kkint64  ScannerFile3BitEncoded::SkipToNextFrame ()
{
  uchar*  scanLine = new uchar[pixelsPerScanLine];

  kkuint32  numScanLinesReadThisFrameBuffer = 0;
  while  ((feof (file) == 0)  &&  (numScanLinesReadThisFrameBuffer < frameHeight))
  {
    GetNextScanLine (scanLine, pixelsPerScanLine);
    ++numScanLinesReadThisFrameBuffer;
  }

  delete[]  scanLine;
  scanLine = NULL;

  if  (feof (file) != 0)
    return -1;
  else
    return osFTELL (file);
}  /* SkipToNextFrame */



void  ScannerFile3BitEncoded::GetNextScanLine (uchar* lineBuff,
                                               kkuint32 lineBuffSize
                                              )
{
  bool   eol = false;
  uchar  opCode = 0;
  OpRec  rec;
  kkuint32 recsRead = 0;

  kkuint32  lineSize = 0;

  char*   textMsg     = NULL;            
  kkuint32  textMsgLen  = 0;

  do
  {
    recsRead = fread (&rec, sizeof (rec), 1, file);
    if  (recsRead == 0)
    {
      break;
    }

    opCode = rec.textBlock.opCode;

    switch  (opCode)
    {
    case 0:
      {
        // textBlock
        kkuint32  numTextBytes = rec.textBlock.length;
        bool  endOfTextBlock = rec.textBlock.endOfTextBlock;


        if  (numTextBytes < 1)
        {
          // Invalid Text Block ?????
          /**@todo   ScannerFile3BitEncoded::GetNextScanLine    How do we get invalid text blocks */
          break;
        }

        char* textMsgPtr = NULL;

        if  (!textMsg)
        {
          textMsg = new char[numTextBytes];
          textMsgLen = numTextBytes;
          textMsgPtr = textMsg;
        }
        else
        {
          kkint32 newTextMsgLen = textMsgLen + numTextBytes;
          char*   newTextMsg = new char[newTextMsgLen];
          memcpy (newTextMsg, textMsg, textMsgLen);
          textMsgPtr = newTextMsg + textMsgLen;
          delete textMsg;
          textMsg = newTextMsg;
          newTextMsg = NULL;
          textMsgLen = newTextMsgLen;
        }

        recsRead = fread (textMsgPtr, 1, numTextBytes, file);
        if  (recsRead < numTextBytes)
        {
          eol = true;
        }
        if  (endOfTextBlock)
        {
          ReportTextMsg (textMsg, textMsgLen);
          delete  textMsg;  textMsg = NULL;
          textMsgLen = 0;
        }
      }
      break;


    case 1:
      {
        eol = (rec.rawPixels.eol == 1);
        if  (lineSize >= (lineBuffSize - 4))
        {
          // We are going to exceed the length of the provided buffer.
          lineSize = lineBuffSize;
        }
        else
        {
          uchar pix0 = convTable3BitTo8Bit[rec.rawPixels.pix0];
          uchar pix1 = convTable3BitTo8Bit[rec.rawPixels.pix1];
          uchar pix2 = convTable3BitTo8Bit[rec.rawPixels.pix2];
          uchar pix3 = convTable3BitTo8Bit[rec.rawPixels.pix3];

          lineBuff[lineSize] = pix0;
          lineSize++;

          lineBuff[lineSize] = pix1;
          lineSize++;

          lineBuff[lineSize] = pix2;
          lineSize++;

          lineBuff[lineSize] = pix3;
          lineSize++;
        }
      }
      break;

    case 2:
      {
        eol = (rec.spaces.eol == 1);
        kkuint32  numSpaces = 4 * rec.spaces.num4Spaces;
        kkuint32  newLineSize = lineSize + numSpaces;
        if  (newLineSize >= lineBuffSize)
        {
          numSpaces = lineBuffSize - lineSize;
          newLineSize = lineBuffSize;
        }

        memset (&(lineBuff[lineSize]), 0,  numSpaces);
        lineSize = newLineSize;
      }
      break;

      
    case 3:
      {
        eol = (rec.blackOuts.eol == 1);
        kkuint32  numBlackOuts = 4 * (kkuint32)(rec.blackOuts.num4BlackOuts);
        kkuint32  newLineSize = lineSize + numBlackOuts;
        if  (newLineSize >= lineBuffSize)
        {
          if  (lineSize > lineBuffSize)
            cout << endl << "How Can This Happen !!!!" << endl << endl;

          numBlackOuts = lineBuffSize - lineSize;
          newLineSize = lineBuffSize;
        }

        memset (&(lineBuff[lineSize]), 255,  numBlackOuts);
        lineSize = newLineSize;
      }
      break;

    case 4:
      {
        eol = (rec.runLen.eol == 1);
        kkuint32  runLen = rec.runLen.runLen;
        uchar   pixelValue = convTable3BitTo8Bit[rec.runLen.pix];

        kkuint32  newLineSize = lineSize + runLen;
        if  (newLineSize >= lineBuffSize)
        {
          runLen = lineBuffSize - lineSize;
          newLineSize = lineBuffSize;
        }

        memset (&(lineBuff[lineSize]), pixelValue,  runLen);
        lineSize = newLineSize;
      }
      break;
    }
  }  while  (!eol);


  if  (textMsg)
  {
    ReportTextMsg (textMsg, textMsgLen);
    delete[]  textMsg;  textMsg = NULL;
    textMsgLen = 0;
  }
}  /* GetNextScanLine */




void  ScannerFile3BitEncoded::WriteBufferFrame ()
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
 
  #if  defined(OS_WINDOWS)
    frameBufferFileOffsetNext = osFTELL (file);
  #else
    /**
     *@todo  We need to implement a 64 bit ftell function for non windows; this will involve 
     * implementation of '#define _FILE_OFFSET_BITS 64' and ftell; probably thru macros
     * in the "FirstIncludes.h" file.
     */
  #endif

  fileSizeInBytes = frameBufferFileOffsetNext;
}  /* WriteBufferFrame*/



void  ScannerFile3BitEncoded::Write4Spaces (OpRecPtr&  outputBuffPtr,
                                            kkint32&   outputBuffUsed,
                                            kkint32&   num4SpacesInARow,
                                            ushort     eol
                                           )
{
  if  (num4SpacesInARow > 0)
  {
    outputBuffPtr->spaces.opCode     = 2;  // Spaces Code.
    outputBuffPtr->spaces.eol        = eol;
    outputBuffPtr->spaces.num4Spaces = num4SpacesInARow;
    ++outputBuffPtr;
    ++outputBuffUsed;
    num4SpacesInARow = 0;
  }
}  /* Write4Spaces */



void  ScannerFile3BitEncoded::Write4BlackOuts (OpRecPtr&  outputBuffPtr,
                                               kkint32&   outputBuffUsed,
                                               kkint32&   num4BlackOutsInARow,
                                               ushort     eol
                                              )
{
  if  (num4BlackOutsInARow > 0)
  {
    outputBuffPtr->blackOuts.opCode     = 3;  // Spaces Code.
    outputBuffPtr->blackOuts.eol        = eol;
    outputBuffPtr->blackOuts.num4BlackOuts = num4BlackOutsInARow;
    ++outputBuffPtr;
    ++outputBuffUsed;
    num4BlackOutsInARow = 0;
  }
}  /* Write4BlackOuts */



void  ScannerFile3BitEncoded::WriteNextScanLine (const uchar*  buffer,
                                                 kkuint32      bufferLen
                                                )
{
  kkuint32  x = 0;

  if  (bufferLen > workLineLen)
  {
    // the buffer line being passed in is larger than the current workLine so we need to expand workLine.
    delete  workLine;
    workLine = new uchar[bufferLen];
    workLineLen = bufferLen;
  }

  for  (x = 0;  x < bufferLen;  ++x)
    workLine[x] = convTable8BitTo3Bit[buffer[x]];

  kkint32  outputBuffUsed      = 0;
  kkint32  num4SpacesInARow    = 0;
  kkint32  num4BlackOutsInARow = 0;
  x = 0;
  uchar*  workLinePtr = workLine;
  OpRecPtr  outputBuffPtr = outputBuff; 

  while  (x < (bufferLen - 3))
  {
    if  (memcmp (workLinePtr, fourSpaces, sizeof (fourSpaces)) == 0)
    {
      Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 0);
      ++num4SpacesInARow;
      x += 4;
      workLinePtr += 4;
    }

    else if  (memcmp (workLinePtr, fourBlackOuts, sizeof (fourBlackOuts)) == 0)
    {
      Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 0);
      ++num4BlackOutsInARow;
      x += 4;
      workLinePtr += 4;
    }

    else
    {
      Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 0);
      Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 0);

      outputBuffPtr->rawPixels.opCode = 1;
      outputBuffPtr->rawPixels.eol    = 0;
      outputBuffPtr->rawPixels.pix0 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix1 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix2 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix3 = workLine[x];  ++x;
      ++outputBuffPtr;
      ++outputBuffUsed;
      workLinePtr += 4;
    }
  }

  if  (num4SpacesInARow > 0)
    Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 1);

  else if  (num4BlackOutsInARow > 0)
    Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 1);


  outputBuff[outputBuffUsed - 1].spaces.eol = 1;  // Set the EOL flag

  fwrite (outputBuff, sizeof (OpRec), outputBuffUsed, file);

  fileSizeInBytes = osFTELL (file);
}  /* WriteNextScanLine */





void  ScannerFile3BitEncoded::WriteNextScanLine2 (const uchar*  buffer,
                                                  kkuint32      bufferLen
                                                 )
{
  kkuint32  x = 0;

  if  (bufferLen > workLineLen)
  {
    // the buffer line being passed in is larger than the current workLine so we need to expand workLine.
    delete  workLine;
    workLine = new uchar[bufferLen];
    workLineLen = bufferLen;
  }

  for  (x = 0;  x < bufferLen;  ++x)
    workLine[x] = convTable8BitTo3Bit[buffer[x]];

  curCompStatus = csNull;

  kkint32  outputBuffUsed      = 0;
  kkint32  num4SpacesInARow    = 0;
  kkint32  num4BlackOutsInARow = 0;
  x = 0;
  uchar*  workLinePtr = workLine;
  OpRecPtr  outputBuffPtr = outputBuff; 

  while  (x < (bufferLen - 3))
  {
    if  (memcmp (workLinePtr, fourSpaces, sizeof (fourSpaces)) == 0)
    {
      Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 0);
      ++num4SpacesInARow;
      x += 4;
      workLinePtr += 4;
    }

    else if  (memcmp (workLinePtr, fourBlackOuts, sizeof (fourBlackOuts)) == 0)
    {
      Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 0);
      ++num4BlackOutsInARow;
      x += 4;
      workLinePtr += 4;
    }

    else
    {
      Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 0);
      Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 0);

      outputBuffPtr->rawPixels.opCode = 1;
      outputBuffPtr->rawPixels.eol    = 0;

      outputBuffPtr->rawPixels.pix0 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix1 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix2 = workLine[x];  ++x;
      outputBuffPtr->rawPixels.pix3 = workLine[x];  ++x;
      ++outputBuffPtr;
      ++outputBuffUsed;
      workLinePtr += 4;
    }
  }

  if  (num4SpacesInARow > 0)
    Write4Spaces (outputBuffPtr, outputBuffUsed, num4SpacesInARow, 1);

  else if  (num4BlackOutsInARow > 0)
    Write4BlackOuts (outputBuffPtr, outputBuffUsed, num4BlackOutsInARow, 1);


  outputBuff[outputBuffUsed - 1].spaces.eol = 1;  // Set the EOL flag

  fwrite (outputBuff, sizeof (OpRec), outputBuffUsed, file);

  fileSizeInBytes = osFTELL (file);
}  /* WriteNextScanLine2 */






void  ScannerFile3BitEncoded::WriteTextBlock (const uchar*  txtBlock,
                                              kkuint32      txtBlockLen
                                             )
{
  kkint32  charsLeft = txtBlockLen;
  const uchar*  txtBlockPtr = txtBlock;
  while  (charsLeft > 0)
  {
    kkint32  charsToWrite = Min (4096, charsLeft);
    OpRec  rec;
    rec.textBlock.opCode         = 0;
    rec.textBlock.endOfTextBlock = (charsToWrite < charsLeft) ? 0 : 1;
    rec.textBlock.length         = charsToWrite;
    fwrite (&rec, sizeof (rec), 1, file);
    fwrite (txtBlockPtr, 1, charsToWrite, file);
    charsLeft -= charsToWrite;
    txtBlockPtr += charsToWrite;
  }

  //nextScanLineOffset = osFTELL (file);
  fileSizeInBytes = osFTELL (file);
}  /* WriteTextBlock */
