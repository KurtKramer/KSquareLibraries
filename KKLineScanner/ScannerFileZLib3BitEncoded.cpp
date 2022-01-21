#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <errno.h>
#include  <string.h>
#include  <ctype.h>

#include  <iostream>
#include  <fstream>
#include  <map>
#include  <vector>

#include  "MemoryDebug.h"

using namespace std;

#include  "Compressor.h"
#include  "KKBaseTypes.h"
#include  "OSservices.h"
#include  "RunLog.h"
#include  "KKStr.h"
using namespace KKB;


#include  "ScannerFile.h"

#include  "Variables.h"

#include  "ScannerFileZLib3BitEncoded.h"

using namespace  KKLSC;

#pragma pack(push)
#pragma pack(1)


struct  ScannerFileZLib3BitEncoded::TwoByteRec
{
  uint8  intHi;
  uint8  intLo;
};


struct  ScannerFileZLib3BitEncoded::ThreeByteRec
{
  uint8  intByte0;   // Highest Order
  uint8  intByte1;
  uint8  intByte2;   // Lowest Order 
};


struct  ScannerFileZLib3BitEncoded::FourByteRec
{
  uint8  intByte0;   // Highest Order
  uint8  intByte1;
  uint8  intByte2;
  uint8  intByte3;   // Lowest Order 
};


struct  ScannerFileZLib3BitEncoded::OpCodeRec1
{
  uint8  opCode;
  uint8  textBlockLen;
};



struct  ScannerFileZLib3BitEncoded::OpCodeRec2
{
  uint8  opCode;
  uint8  textBlockLenHi;
  uint8  textBlockLenLo;
};


struct  ScannerFileZLib3BitEncoded::OpCodeRec5
{
  uint8   opCode;
  uint8   compBuffLenHO;
  uint8   compBuffLenLO;
};  /* OpCodeRec5 */



struct  ScannerFileZLib3BitEncoded::OpCodeRec6
{
  uint8   opCode;
  uint8   compBuffLenByte0;
  uint8   compBuffLenByte1;
  uint8   compBuffLenByte2;
};  /* OpCodeRec6 */



struct  ScannerFileZLib3BitEncoded::OpCodeRec7
{
  uint8   opCode;
  uint8   compBuffLenByte0;
  uint8   compBuffLenByte1;
  uint8   compBuffLenByte2;
  uint8   compBuffLenByte3;
};  /* OpCodeRec7 */

#pragma pack(pop)


ScannerFileZLib3BitEncoded::ScannerFileZLib3BitEncoded (const KKStr&  _fileName,
                                                        RunLog&       _log
                                                       ):
  ScannerFile (_fileName, _log),
  compBuffer     (NULL),
  compBufferLen  (0),
  compBufferSize (0)
{
  AllocateBuffers ();
}



ScannerFileZLib3BitEncoded::ScannerFileZLib3BitEncoded (const KKStr&  _fileName,
                                                        kkuint32      _channelCount,
                                                        kkuint32      _pixelsPerScanLine,
                                                        kkuint32      _frameHeight,
                                                        RunLog&       _log
                                                       ):
  ScannerFile (_fileName, _channelCount, _pixelsPerScanLine, _frameHeight, _log),
  compBuffer      (NULL),
  compBufferLen   (0),
  compBufferSize  (0)
{
  AllocateBuffers ();
}



  
ScannerFileZLib3BitEncoded::~ScannerFileZLib3BitEncoded (void)
{
  if  (opened)
    Close ();
  delete  compBuffer;
  compBuffer  = NULL;
}



void  ScannerFileZLib3BitEncoded::AllocateBuffers ()
{
  delete  compBuffer;   compBuffer  = NULL;

  compBufferSize = (kkuint32)(frameBufferSize * 1.2);
  compBufferLen = 0;
  compBuffer = new uchar[compBufferSize];
}  /* AllocateBuffers */



void  ScannerFileZLib3BitEncoded::ScanRate (float  _scanRate)
{
  ScannerFile::ScanRate (_scanRate);
}



void  ScannerFileZLib3BitEncoded::WriteTextBlock (const uchar*  txtBlock,
                                                  kkuint32      txtBlockLen
                                                 )
{
  kkuint32  charsLeft = txtBlockLen;
  const uchar*  txtBlockPtr = txtBlock;
  while  (charsLeft > 0)
  {
    if  (charsLeft < 256)
    {
      OpCodeRec1  rec;
      rec.opCode = 1;
      rec.textBlockLen = (uint8)charsLeft;
      fwrite (&rec, sizeof (rec), 1, file);
      fwrite (txtBlockPtr, charsLeft, 1, file);
      txtBlockPtr += charsLeft;
      charsLeft = 0;
    }

    else
    {
      kkuint32  bytesToWrite = Min ((kkuint32)65535, charsLeft);
      OpCodeRec2  rec;
      rec.opCode = 2;
      rec.textBlockLenHi = (uint8)(bytesToWrite / 256);
      rec.textBlockLenLo = (uint8)(bytesToWrite % 256);
      fwrite (&rec, sizeof (rec), 1, file);
      fwrite (txtBlockPtr, bytesToWrite, 1, file);
      txtBlockPtr += bytesToWrite;
      charsLeft -= bytesToWrite;
    }
  }

  fileSizeInBytes = osFTELL (file);
}  /* WriteTextBlock */



void  ScannerFileZLib3BitEncoded::WriteBufferFrame ()
{
  kkuint32  frameBufferSpaceUsed = frameBufferNextLine * pixelsPerScanLine;
  kkuint32  compressedBuffLen = 0;

  for (kkuint32 x = 0; x < frameBufferSpaceUsed;  ++x)
  {
    frameBuffer[x] = frameBuffer[x] & 0xe0;
  }

  uchar*  compressedData = (uchar*)Compressor::CreateCompressedBuffer (frameBuffer, frameBufferSpaceUsed, compressedBuffLen);

  frameBufferFileOffsetLast = osFTELL (file);

  if  (compressedBuffLen <= 65535)
  {
    OpCodeRec5   rec5;
    rec5.opCode = 5;
    rec5.compBuffLenLO = (uint8)(compressedBuffLen  % 256);
    rec5.compBuffLenHO = (uint8)(compressedBuffLen  / 256);
    fwrite (&rec5, sizeof (rec5), 1, file);
  }

  else if  (compressedBuffLen <= 16777215)
  {
    kkuint32 buffLen = compressedBuffLen;
    OpCodeRec6   rec6;
    rec6.opCode = 6;
    rec6.compBuffLenByte2 = buffLen % 256;  buffLen = buffLen / 256;
    rec6.compBuffLenByte1 = buffLen % 256;  buffLen = buffLen / 256;
    rec6.compBuffLenByte0 = buffLen % 256;
    fwrite (&rec6, sizeof (rec6), 1, file);
  }

  else
  {
    kkuint32 buffLen = compressedBuffLen;
    OpCodeRec7   rec7;
    rec7.opCode = 7;
    rec7.compBuffLenByte3 = buffLen % 256;  buffLen = buffLen / 256;
    rec7.compBuffLenByte2 = buffLen % 256;  buffLen = buffLen / 256;
    rec7.compBuffLenByte1 = buffLen % 256;  buffLen = buffLen / 256;
    rec7.compBuffLenByte0 = (uchar)buffLen;
    fwrite (&rec7, sizeof (rec7), 1, file);
  }

  fwrite (compressedData, compressedBuffLen, 1, file);
  delete  compressedData;  compressedData = NULL;
  frameBufferNextLine = 0;

  frameBufferFileOffsetNext = osFTELL (file);
  fileSizeInBytes = frameBufferFileOffsetNext;
}  /* WriteBufferFrame */




void  ScannerFileZLib3BitEncoded::ExpandBuffer (uchar*&  buffer,
                                                kkuint32&  bufferSize,
                                                kkuint32 bufferNewSize
                                               )
{
  uchar*  newBuffer = new  uchar[bufferNewSize];
  if  (buffer != NULL)
  {
    memcpy (newBuffer, buffer, bufferSize);
    delete  buffer;
  }
  buffer = newBuffer;
  newBuffer = NULL;
  bufferSize = bufferNewSize;
  return;
}  /* ExpandBuffer */



void  ScannerFileZLib3BitEncoded::ExpandBufferNoCopy (uchar*&  buffer,
                                                      kkuint32&  bufferSize,
                                                      kkuint32 bufferNewSize
                                                     )
{
  uchar*  newBuffer = new uchar[bufferNewSize];
  if  (buffer != NULL)
    delete  buffer;

  buffer = newBuffer;
  newBuffer = NULL;
  bufferSize = bufferNewSize;
  return;
}  /* ExpandBufferNoCopy */



kkint64  ScannerFileZLib3BitEncoded::SkipToNextFrame ()
{
  bool  bufferFrameRead = false;

  while  ((feof (file) == 0)  &&  (!bufferFrameRead))
  {
    uint8  opCode;
    fread (&opCode, sizeof (opCode), 1, file);

    if  (feof (file) != 0)
      return -1;

    switch  (opCode)
    {
    case 1:
      {
        uint8  textBlockLen;
        fread (&textBlockLen, 1, 1, file);
        if  (feof (file) != 0)
          return -1;
        SkipBytesForward (textBlockLen);
        break;
      }

    case 2:
      {
        TwoByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) != 0)
          return -1;
        kkuint32  len = textBlockLen.intHi * 256 + textBlockLen.intLo;
        SkipBytesForward (len);
        break;
      }

    case  5:
      {
        TwoByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) != 0)
          return -1;
        kkuint32  compBufferLenCode5 = (kkuint32)(textBlockLen.intHi * 256 + textBlockLen.intLo);
        SkipBytesForward (compBufferLenCode5);
        bufferFrameRead = true;
        break;
      }

    case  6:
      {
        ThreeByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) != 0)
          return -1;
        kkuint32  compBufferLenCode6 = (kkuint32)(textBlockLen.intByte0 * 256 * 256 +
                                textBlockLen.intByte1 * 256       +
                                textBlockLen.intByte2);
        SkipBytesForward (compBufferLenCode6);
        bufferFrameRead = true;
        break;
      }

    case  7:
      {
        FourByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) != 0)
          return -1;

        kkuint32  compBufferLenCode7 = (kkuint32)(textBlockLen.intByte0 * 256 * 256 * 256 +
                                textBlockLen.intByte1 * 256 * 256       +
                                textBlockLen.intByte2 * 256             +
                                textBlockLen.intByte3);
        SkipBytesForward (compBufferLenCode7);
        bufferFrameRead = true;
        break;
      }
    }
  }

  if  (feof (file) != 0)
    return -1;
  else
    return osFTELL (file);

}  /* SkipToNextFrame */



/** @brief  Read in the next frame into the Buffer from the file. */
kkuint32  ScannerFileZLib3BitEncoded::ReadBufferFrame ()
{
  frameBufferLen = 0;
  frameBufferNextLine = 0;
  if  (feof (file) != 0)
  {
    memset (frameBuffer, 0, frameBufferSize);
    return 0;
  }

  bool  bufferFrameRead = false;

  frameBufferFileOffsetLast = osFTELL (file);

  while  ((!feof (file))  &&  (!bufferFrameRead))
  {
    uint8  opCode;

    fread (&opCode, sizeof (opCode), 1, file);

    if  (feof (file) != 0)
    {
      memset (frameBuffer, 0, frameBufferSize);
      return  0;
    }

    switch  (opCode)
    {
    case 1:
      {
        uint8  textBlockLen;
        fread (&textBlockLen, 1, 1, file);
        char  buff[257];
        fread (&buff, textBlockLen, 1, file);
        if  (!feof (file))
        {
          buff[textBlockLen] = 0;
          ReportTextMsg (buff, textBlockLen);
        }
        break;
      }

    case 2:
      {
        TwoByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) == 0)
        {
          kkuint32  len = textBlockLen.intHi * 256 + textBlockLen.intLo;
          fread (compBuffer, len, 1, file);
          if  (!feof (file))
            ReportTextMsg ((char*)compBuffer, len);
        }
        break;
      }

    case  5:
      {
        TwoByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (feof (file) == 0)
        {
          kkuint32  compBufferLenCode5 = (kkuint32)(textBlockLen.intHi * 256 + textBlockLen.intLo);
          if  (compBufferLenCode5 > compBufferSize)
            ExpandBufferNoCopy (compBuffer, compBufferSize, compBufferLenCode5);

          fread (compBuffer, compBufferLenCode5, 1, file);
          if  (feof (file) == 0)
          {
            Compressor::Decompress (compBuffer, compBufferLenCode5,
                                    frameBuffer, frameBufferSize, frameBufferLen
                                   );
          }
        }
        bufferFrameRead = true;
        break;
      }

    case  6:
      {
        ThreeByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (!feof (file))
        {
          kkuint32  compBufferLenCode6 = (kkuint32)(textBlockLen.intByte0 * 256 * 256 +
                                  textBlockLen.intByte1 * 256       +
                                  textBlockLen.intByte2);
          if  (compBufferLenCode6 > compBufferSize)
            ExpandBufferNoCopy (compBuffer, compBufferSize, compBufferLenCode6);

          fread (compBuffer, compBufferLenCode6, 1, file);
          if  (feof (file) == 0)
          {
            Compressor::Decompress (compBuffer, compBufferLenCode6,
                                    frameBuffer, frameBufferSize, frameBufferLen
                                   );
          }
        }
        bufferFrameRead = true;
        break;
      }

    case  7:
      {
        FourByteRec  textBlockLen;
        fread (&textBlockLen, sizeof (textBlockLen), 1, file);
        if  (!feof (file))
        {
          kkuint32  compBufferLenCode7 = (kkuint32)(textBlockLen.intByte0 * 256 * 256 * 256 +
                                  textBlockLen.intByte1 * 256 * 256       +
                                  textBlockLen.intByte2 * 256             +
                                  textBlockLen.intByte3);
          if  (compBufferLenCode7 > compBufferSize)
            ExpandBufferNoCopy (compBuffer, compBufferSize, compBufferLenCode7);

          fread (compBuffer, compBufferLenCode7, 1, file);
          if  (feof (file) == 0)
          {
            Compressor::Decompress (compBuffer, compBufferLenCode7,
                                    frameBuffer, frameBufferSize, frameBufferLen
                                   );
          }
        }
        bufferFrameRead = true;
        break;
      }
    }

    frameBufferNextLine = 0;
  }

  frameBufferFileOffsetNext = osFTELL (file);

  return  (frameBufferLen / pixelsPerScanLine);
}  /* ReadBufferFrame */
