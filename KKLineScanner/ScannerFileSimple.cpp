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

#include  "GlobalGoalKeeper.h"
#include  "KKBaseTypes.h"
#include  "OSservices.h"
#include  "RunLog.h"
#include  "KKStr.h"
using namespace KKB;


#include  "ScannerFile.h"
#include  "Variables.h"

#include "ScannerFileSimple.h"


using namespace  KKLSC;


ScannerFileSimple::ScannerFileSimple (const KKStr&  _fileName,
                                      RunLog&       _log
                                     ):
             ScannerFile (_fileName, _log)
{
}



ScannerFileSimple::ScannerFileSimple (const KKStr&  _fileName,
                                      kkuint32      _pixelsPerScanLine,
                                      kkuint32      _frameHeight,
                                      RunLog&       _log
                                     ):
             ScannerFile (_fileName, _pixelsPerScanLine, _frameHeight, _log)
{
}




ScannerFileSimple::~ScannerFileSimple ()
{
  if  (opened)
    Close ();
}




kkuint32  ScannerFileSimple::ReadBufferFrame ()
{
  frameBufferLen = 0;
  frameBufferNextLine = 0;
  if  (feof (file) != 0)
  {
    memset (frameBuffer, 0, frameBufferSize);
    return 0;
  }

  frameBufferFileOffsetLast = osFTELL (file);
  frameBufferLen = fread (frameBuffer, 1, frameBufferSize, file);
  frameBufferFileOffsetNext = osFTELL (file);
  frameBufferNextLine = 0;
  return  (frameBufferLen / pixelsPerScanLine);
}  /* ReadBufferFrame */



kkint64  ScannerFileSimple::SkipToNextFrame ()
{
  //int64  byteOffset = osFTELL (file);
  kkint64  byteOffset = osFTELL (file);

  kkint64  nextFrameByteOffset = byteOffset + frameBufferSize;
  kkint32  returnCd = osFSEEK (file, nextFrameByteOffset - 1, SEEK_SET);

  char buff[10];
  returnCd = fread (buff, 1, 1, file);
  if  (returnCd < 1)
    return -1;
  else
    return nextFrameByteOffset;
}  /* SkipToNextFrame */



void  ScannerFileSimple::WriteBufferFrame ()
{
  frameBufferFileOffsetLast = osFTELL (file);

  kkuint32  frameBufferSpaceUsed = frameBufferNextLine * pixelsPerScanLine;
  fwrite (frameBuffer, 1, frameBufferSpaceUsed, file);

  kkuint32  paddingNeeded = frameBufferSize - frameBufferSpaceUsed;
  if  (paddingNeeded > 0)
  {
    uchar* buff = new uchar[paddingNeeded];
    memset (buff, 0, paddingNeeded);
    fwrite (buff, 1, paddingNeeded, file);
    delete[]  buff;
  }

  frameBufferNextLine = 0;

  frameBufferFileOffsetNext = osFTELL (file);
  fileSizeInBytes = frameBufferFileOffsetNext;
}  /* WriteBufferFrame */




void   ScannerFileSimple::WriteTextBlock (const uchar*  txtBlock,
                                          kkuint32      txtBlockLen
                                         )
{
  log.Level (-1) << endl
    << "ScannerFileSimple::WriteTextBlock   ***ERROR***   The SimpleFormat does not support TextBlocks" << endl
    << endl;
}  /* WriteTextBlock */




const uchar*  ScannerFileSimple::CompensationTable ()
{
  GlobalGoalKeeper::StartBlock ();

  uchar*  cs = new uchar[256];
  for  (kkuint32 x = 0;  x < 256;  ++x)
    cs[x] = x;

  GlobalGoalKeeper::EndBlock ();

  return cs;
}
