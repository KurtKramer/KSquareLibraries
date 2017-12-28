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


#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "ScannerFile.h"
#include "ScannerHeaderFields.h"
#include "ScannerFileSimple.h"
#include "ScannerFile2BitEncoded.h"
#include "ScannerFile3BitEncoded.h"
#include "ScannerFile4BitEncoded.h"
#include "ScannerFileZLib3BitEncoded.h"
#include "ScannerFileEntry.h"
#include "StartStopPoint.h"
#include "Variables.h"
using namespace  KKLSC;



/**  Constructor for opening file for reading */
ScannerFile::ScannerFile (const KKStr&  _fileName,
                          RunLog&       _log
                          ):
  byteOffsetScanLineZero    (0),
  eof                       (false),
  file                      (NULL),
  fileName                  (_fileName),
  fileSizeInBytes           (0),
  flatFieldEnabled          (false),
  frameHeight               (0),
  headerDataWritten         (false),
  headerFields              (NULL),
  ioMode                    (ioRead),
  largestKnownScanLine      (-1),
  lastScanLine              (0),
  log                       (_log),
  nextScanLine              (0),
  opened                    (false),
  pixelsPerScanLine         (4096),
  scanRate                  (20000),

  flowMeterCounter          (0),
  flowMeterCounterScanLine  (0),
  frameBuffer               (NULL),
  frameBufferFileOffsetLast (0),
  frameBufferFileOffsetNext (0),
  frameBufferLastReadEof    (false),
  frameBufferLen            (0),
  frameBufferNextLine       (0),
  frameBufferNumScanLines   (0),
  frameBufferSize           (0),
  frameNumCurLoaded         (0),
  frameOffsets              (),
  frameOffsetsBuildRunning  (false),
  frameOffsetsLoaded        (false),
  goalie                    (NULL),
  indexFile                 (NULL),
  indexFileName             (),
  scannerFileEntry          (NULL),
  startStopPoints           ()
{
  CreateGoalie ();

  scannerFileEntry = ScannerFileEntry::GetOrCreateScannerFileEntry (osGetRootName (fileName));
  scannerFileEntry->FullName (fileName);

  Open (fileName); 
  if  (opened)
  {
    ReadHeader ();
    byteOffsetScanLineZero = osFTELL (file);
    if  (frameHeight == 0)
      frameHeight = pixelsPerScanLine;
    AllocateFrameBuffer ();
    UpdateFrameOffset (0, 0, byteOffsetScanLineZero);
    scannerFileEntry->PixelsPerScanLine (pixelsPerScanLine);
    scannerFileEntry->ScanRate (scanRate);
  }
}

 
  
  
/**  Constructor for opening file for Writing */
ScannerFile::ScannerFile (const KKStr&  _fileName,
                          kkuint32      _pixelsPerScanLine,
                          kkuint32      _frameHeight,
                          RunLog&       _log
                          ):

  byteOffsetScanLineZero    (0),
  eof                       (false),
  file                      (NULL),
  fileName                  (_fileName),
  fileSizeInBytes           (0),
  flatFieldEnabled          (false),
  frameHeight               (_frameHeight),
  headerDataWritten         (false),
  headerFields              (NULL),
  ioMode                    (ioWrite),
  largestKnownScanLine      (-1),
  lastScanLine              (0),
  log                       (_log),
  nextScanLine              (0),
  opened                    (false),
  pixelsPerScanLine         (_pixelsPerScanLine),
  scanRate                  (20000),

  flowMeterCounter          (0),
  flowMeterCounterScanLine  (0),
  frameBuffer               (NULL),
  frameBufferFileOffsetLast (0),
  frameBufferFileOffsetNext (0),
  frameBufferLastReadEof    (false),
  frameBufferLen            (0),
  frameBufferNextLine       (0),
  frameBufferNumScanLines   (0),
  frameBufferSize           (0),
  frameNumCurLoaded         (0),
  frameOffsets              (),
  frameOffsetsBuildRunning  (false),
  frameOffsetsLoaded        (false),
  goalie                    (NULL),
  indexFile                 (NULL),
  indexFileName             (),
  scannerFileEntry          (NULL),
  startStopPoints           ()
{
  scannerFileEntry = ScannerFileEntry::GetOrCreateScannerFileEntry (osGetRootName (fileName));
  scannerFileEntry->FullName (fileName);
  scannerFileEntry->PixelsPerScanLine (pixelsPerScanLine);

  Open (fileName);
  if  (opened)
  {
    byteOffsetScanLineZero = osFTELL (file);
    AllocateFrameBuffer ();
    //UpdateFrameOffset (0, 0, byteOffsetScanLineZero);
  }
}



ScannerFile::~ScannerFile ()
{
  if  (opened)
    Close ();

  delete  headerFields;  headerFields = NULL;
  delete  frameBuffer;   frameBuffer  = NULL;
  
  GoalKeeper::Destroy (goalie);
  goalie = NULL;
}



kkMemSize  ScannerFile::MemoryConsumedEstimated ()  const
{
  kkMemSize  mem = sizeof (*this) +
               fileName.MemoryConsumedEstimated ()       +
               frameOffsets.size () * sizeof(kkint64)    +
               indexFileName.MemoryConsumedEstimated ()  +
               startStopPoints.MemoryConsumedEstimated ();

  if  (file)              mem += 2000;
  if  (headerFields)      mem += headerFields->MemoryConsumedEstimated ();
  if  (frameBuffer)       mem += frameBufferSize;
  if  (indexFile)         mem += 2000;
  if  (scannerFileEntry)  mem += scannerFileEntry->MemoryConsumedEstimated ();
  return  mem;
}



void  ScannerFile::AllocateFrameBuffer ()
{
  delete  frameBuffer;   frameBuffer = NULL;
  frameBufferSize = frameHeight * pixelsPerScanLine;
  frameBufferLen  = 0;
  frameBuffer = new uchar[frameBufferSize];
}



void  ScannerFile::Close ()
{
  if  (opened  &&  (ioMode == ScannerFile::ioWrite))
  {
    if  (frameBufferNextLine > 0)
      WriteBufferFrame ();
    fclose (file);
    file = NULL;

    if  (indexFile)
    {
      indexFile->close ();
      delete  indexFile;
      indexFile = NULL;
    }

    opened = false;
  }
}



void  ScannerFile::Flush ()
{
  if  (file)
  {
    fflush (file);
  }
}




void  ScannerFile::ScanRate (float  _scanRate)
{
  scanRate = _scanRate;
}




void  ScannerFile::Reset ()
{
  if  (!opened)
    return;

  eof = false;  
  rewind (file);
  ReadHeader ();

  frameBufferLastReadEof = false;
  lastScanLine = 0;
  nextScanLine = 0;
  frameBufferFileOffsetLast = -1;
  frameBufferFileOffsetNext = osFTELL (file);
}  /* Reset */




void  ScannerFile::Open (const KKStr&  _fileName)
{
  eof = false;
  fileName = _fileName;
  fileSizeInBytes = 0;

  if  (ioMode == ioRead)
  {
    file = osFOPEN (fileName.Str (), "rb");
    if  (file)
      fileSizeInBytes = osGetFileSize (fileName);
  }
  else
  {
    file = osFOPEN (fileName.Str (), "wb");
    indexFileName = osRemoveExtension (_fileName) + ".idx";
    indexFile = new ofstream (indexFileName.Str ());
    *indexFile << "IndexFile"                                      << endl
               << "ScannerFile" << "\t" << _fileName               << endl
               << "DateTime"    << "\t" << osGetLocalDateTime ()   << endl;

    *indexFile << "IndexEntryFields" << "\t" << "FrameNum"
                                     << "\t" << "ScanLineNum"
                                     << "\t" << "ByteOffset"
                                     << endl;
  }

  if  (file)
  {
    opened = true;
  }
  else
  {
    opened = false;
    log.Level (-1) << endl << endl << "ScannerFile::Open     ***ERROR***   Opening File[" << fileName << "]" << endl << endl;
  }
}  /* Open */


void  ScannerFile::CreateGoalie ()
{
  if  (!goalie)
  {
    GoalKeeper::Create ("ScannerFile-" + osGetRootName (fileName), goalie);
  }
}



void  ScannerFile::AddStartStopEntryToIndexFile (kkint32                        scanLineNum,
                                                 StartStopPoint::StartStopType  type,
                                                 bool                           deleteEntry
                                                )
{
  if  (indexFileName.Empty ())
    return;

  CreateGoalie ();

  bool  clearToUpdate = false;
  while  (!clearToUpdate)
  {
    goalie->StartBlock ();
    if  (indexFile != NULL)
    {
      goalie->EndBlock ();
      osSleepMiliSecs (10);
    }
    else
    {
      clearToUpdate = true;
    }
  }

  indexFile = new ofstream (indexFileName.Str (), ios_base::app);

  if  (deleteEntry)
    *indexFile << "DeleteStartStopPoint" << "\t" << scanLineNum << endl;
  else
    *indexFile << "StartStopPoint" << "\t" << scanLineNum << "\t" << StartStopPoint::StartStopTypeToStr (type) << endl;

  indexFile->close ();
  delete  indexFile;
  indexFile = NULL;
  goalie->EndBlock ();
}




void  ScannerFile::AddStartPoint (kkint32  _scanLineNum)
{
  startStopPoints.AddEntry (_scanLineNum, StartStopPoint::StartStopType::StartPoint);
  AddStartStopEntryToIndexFile (_scanLineNum, StartStopPoint::StartStopType::StartPoint, false);
}



void  ScannerFile::AddStopPoint (kkint32  _scanLineNum)
{
  startStopPoints.AddEntry (_scanLineNum, StartStopPoint::StartStopType::StopPoint);
  AddStartStopEntryToIndexFile (_scanLineNum, StartStopPoint::StartStopType::StopPoint, false);
}



void  ScannerFile::StartStopPointDelete (kkint32 _scanLineNum)
{
  startStopPoints.DeleteEntry (_scanLineNum);
  AddStartStopEntryToIndexFile (_scanLineNum, StartStopPoint::StartStopType::StopPoint, true);
}




StartStopPointPtr  ScannerFile::StartStopPointNearestEntry (kkint32 _scanLineNum)
{
  return  startStopPoints.NearestEntry (_scanLineNum);
}



StartStopPointPtr  ScannerFile::StartStopPointPrevEntry (kkint32 _scanLineNum)
{
  return  startStopPoints.PrevEntry (_scanLineNum);
}



StartStopPointPtr  ScannerFile::StartStopPointSuccEntry (kkint32 _scanLineNum)
{
  return  startStopPoints.SuccEntry (_scanLineNum);
}




VectorFloatPtr  ScannerFile::RecordRateByTimeIntervals (int intervalSecs)
{
  float  secsPerFrame = 1.0f;
  if  (scanRate > 0.0f)
    secsPerFrame = (float)frameHeight  / scanRate;
  else
    secsPerFrame = (float)frameHeight  / 5000.0f;  // Assume 5k ScanLines per second

  float  totalTime = (float)(frameOffsets.size ()) * secsPerFrame;
  int  totalNumIntervals = (int)(0.5f + totalTime / (float)intervalSecs);

  VectorFloatPtr  recordRates = new VectorFloat (totalNumIntervals, 0.0f);
  int  idx = 0;

  kkuint32 frameOffsetsIdx = 0;
  kkint64  bytesThisFrame = frameOffsets[frameOffsetsIdx];
  float    frameOffsetsStartTime = 0;
  float    frameOffsetsEndTime   = secsPerFrame;
  float    frameOffsetsRecRate = (float)bytesThisFrame / secsPerFrame;

  while  (idx <  totalNumIntervals)
  {
    float  timeIntervalStartTime = (float)(idx * intervalSecs);
    float  timeIntervalEndTime   = timeIntervalStartTime + (float)intervalSecs;

    float  timeIntervalCurTime = timeIntervalStartTime;
    while  (timeIntervalCurTime < timeIntervalEndTime)
    {
      float  nextMark = Min (timeIntervalEndTime, frameOffsetsEndTime);
      float  deltaTime = nextMark - timeIntervalCurTime;
      (*recordRates)[idx] += frameOffsetsRecRate *  (deltaTime / secsPerFrame);
      timeIntervalCurTime = nextMark;

      ++frameOffsetsIdx;
      frameOffsetsStartTime = (float)frameOffsetsIdx * secsPerFrame;
      frameOffsetsEndTime = frameOffsetsStartTime + (float)secsPerFrame;
      if  (frameOffsetsIdx < frameOffsets.size ())
        bytesThisFrame = frameOffsets[frameOffsetsIdx] - frameOffsets[frameOffsetsIdx - 1];
      else
        bytesThisFrame  = 0;
      frameOffsetsRecRate = (float)bytesThisFrame / secsPerFrame;
    }
    ++idx;
  }

  return  recordRates;
}  /* RecordRateByTimeIntervals */







void  ScannerFile::UpdateFrameOffset (kkuint32 frameNum,
                                      kkuint32 scanLineNum,
                                      kkint64  byteOffset
                                     )
{
  CreateGoalie ();
  goalie->StartBlock ();

  if  (frameNum == frameOffsets.size ())
  {
    frameOffsets.push_back (byteOffset);
  }
  else if  (frameNum < frameOffsets.size ())
  {
    frameOffsets[frameNum] = byteOffset;
  }
  else
  {
    while  (frameOffsets.size () < frameNum)
      frameOffsets.push_back (-1);
    frameOffsets.push_back (byteOffset);
  }

  if  (indexFile)
    *indexFile << "IndexEntry" << "\t" << frameNum << "\t" << scanLineNum << "\t" << byteOffset << endl;

  goalie->EndBlock ();
}  /* UpdateFrameOffset */



kkint64  ScannerFile::GetFrameOffset (kkuint32  frameNum)
{
  kkint64  offset = -1;
  goalie->StartBlock ();
  if  (frameNum < frameOffsets.size ())
    offset = frameOffsets[frameNum];
  goalie->EndBlock ();
  return  offset;
}  /* GetFrameOffset */




void  ScannerFile::FrameRead (kkuint32  frameNum,
                              bool&   found
                             )
{
  if  (frameNum >= frameOffsets.size ())
  {
    found = false;
    return;
  }

  goalie->StartBlock ();
  FSeek (frameOffsets[frameNum]);

  frameBufferNumScanLines = ReadBufferFrame ();

  frameBufferNextLine = 0;
  frameBufferLastReadEof = (feof (file) != 0);
  frameNumCurLoaded = frameNum;

  if  (frameBufferNumScanLines > 1)
  {
    found = true;
    nextScanLine = frameNum * frameHeight;
    lastScanLine = nextScanLine - 1;
    kkint32  lastScanLineInFrame = nextScanLine + frameBufferNumScanLines - 1;
    if  (lastScanLineInFrame > largestKnownScanLine)
      largestKnownScanLine = lastScanLineInFrame;
    UpdateFrameOffset (frameNum + 1, (nextScanLine + frameHeight), frameBufferFileOffsetNext);
  }
  else
  {
    // Only time I can think this would happen is while reading the last frame;  meaning that the last frame
    // in the file has exactly 'frameHeight' scan lines.  This way when you read the next frame there are no scan lines.
    cerr << endl << "ScannerFile::FrameRead   ***ERROR***    No scan lines were loaded." << endl << endl;
  }
  goalie->EndBlock ();
}  /* FrameRead */





void  ScannerFile::GetNextLine (uchar*     lineBuff,
                                kkuint32   lineBuffSize,
                                kkuint32&  lineSize,
                                kkuint32   colCount[],
                                kkuint32&  pixelsInRow
                               )
{
  if  (eof)
    return;

  lastScanLine = nextScanLine;

  if  (frameBufferNextLine >= frameBufferNumScanLines)
  {
    if  (frameBufferLastReadEof)
    {
      eof = true;
      return;
    }     
    
    goalie->StartBlock ();
    frameBufferNumScanLines = ReadBufferFrame ();
    frameBufferNextLine = 0;
    frameBufferLastReadEof = (feof (file) != 0);

    if  ((frameBufferNumScanLines == 0) &&  frameBufferLastReadEof)
    {
      // We already are at EOF.
      eof = true;
      return;
    }

    if  (frameBufferNumScanLines > 0)
    {
      frameNumCurLoaded = nextScanLine / frameHeight;
      kkint32  lastScanLineInFrame = nextScanLine + frameBufferNumScanLines - 1;
      if  (lastScanLineInFrame > largestKnownScanLine)
        largestKnownScanLine = lastScanLineInFrame;
      UpdateFrameOffset (frameNumCurLoaded, nextScanLine, frameBufferFileOffsetNext);
    }
    else
    {
      // There were no scan lines loaded.
      cerr << endl << "ScannerFile::GetNextLine   ***ERROR***   No scan lines loaded." << endl << endl;
    }
    goalie->EndBlock ();
  }

  kkuint32  frameBufferOffset = frameBufferNextLine * pixelsPerScanLine;

  lineSize= 0;
  kkuint32  bytesToCopy = Min (lineBuffSize, pixelsPerScanLine);
  for  (kkuint32 x = 0;  x < bytesToCopy;  ++x)
  {
    uchar  pixel = frameBuffer[frameBufferOffset];
    lineBuff[x] = pixel;
    if  (pixel > 0)
    {
      colCount[x] += 1;
      ++pixelsInRow;
    }
    ++lineSize;
    ++frameBufferOffset;
  }

  ++frameBufferNextLine;
  ++nextScanLine;
}  /* GetNextLine */





void  ScannerFile::SkipNextLine ()
{
  if  (eof)
    return;

  lastScanLine = nextScanLine;

  if  (frameBufferNextLine >= frameBufferNumScanLines)
  {
    if  (frameBufferLastReadEof)
    {
      eof = true;
      return;
    }     
    
    goalie->StartBlock ();
    frameBufferNumScanLines = ReadBufferFrame ();
    frameBufferNextLine = 0;
    frameBufferLastReadEof = (feof (file) != 0);

    if  (frameBufferNumScanLines > 0)
    {
      frameNumCurLoaded = nextScanLine / frameHeight;
      kkint32  lastScanLineInFrame = nextScanLine + frameBufferNumScanLines - 1;
      if  (lastScanLineInFrame > largestKnownScanLine)
        largestKnownScanLine = lastScanLineInFrame;
      UpdateFrameOffset (frameNumCurLoaded, nextScanLine, frameBufferFileOffsetNext);
    }
    else
    {
      cerr << endl << "ScannerFile::SkipNextLine   ***ERROR***     No scan lines were loaded." << endl << endl;
    }
    goalie->EndBlock ();
  }

  ++frameBufferNextLine;
  ++nextScanLine;
}  /* SkipNextLine */




void   ScannerFile::WriteScanLine (const uchar*  buffer,
                                   kkuint32      bufferLen
                                  )
{
  lastScanLine = nextScanLine;

  if  (frameBufferNextLine >= frameHeight)
  {
    WriteBufferFrame ();
    frameBufferNextLine = 0;
    frameNumCurLoaded = nextScanLine / frameHeight;
    UpdateFrameOffset (frameNumCurLoaded, nextScanLine, frameBufferFileOffsetNext);
  }

  kkuint32  byteOffset = (frameBufferNextLine * pixelsPerScanLine);
  kkuint32  bytesToCopy = Min (bufferLen, pixelsPerScanLine);
  memcpy (frameBuffer + byteOffset, buffer, bytesToCopy);
  
  ++nextScanLine;
  ++frameBufferNextLine;
}  /* WriteScanLine */




void  ScannerFile::SkipToScanLine (kkuint32  scanLine)
{
  kkuint32  frameNum = scanLine / frameHeight;
  if  (frameNum >= frameOffsets.size ())
    frameNum = (kkuint32)frameOffsets.size () - 1;

  if  (frameNum != frameNumCurLoaded)
  {
    bool found = false;
    FrameRead (frameNum, found);
    if  (!found)
    {
      eof = true;
      return;
    }
  }

  frameBufferNextLine = scanLine % frameHeight;
  if  (frameBufferNextLine > frameBufferNumScanLines)
  {
    // I believe that the only way that this could happen is if 'frameNum' is the last frame in the file.
    cerr << endl << endl << "ScannerFile::SkipToScanLine   ***ERROR***   FrameBufer is short scan lines." << endl << endl;
  }

  lastScanLine = scanLine - 1;
  nextScanLine = scanLine;
}  /* SkipToScanLine */



/**
 *@brief   Adds Instrument data to the underlying Scanner files as text.
 */
void   ScannerFile::WriteInstrumentDataWord (uchar             idNum,
                                             kkuint32          scanLineNum,
                                             WordFormat32Bits  dataWord
                                            )
{
  KKStr  s (100);
  s << "InstrumentDataWord" << "\t" << (int)idNum << "\t" << scanLineNum << "\t" << dataWord.unsigned32BitInt;
  WriteTextBlock ((const uchar*)s.Str (), s.Len ());
}  /* WriteInstrumentDataWord */





ScannerFile::Format  ScannerFile::GuessFormatOfFile (const KKStr&  _fileName,
                                                     RunLog&       _log
                                                    )
{
  // Will guess what file format by trying to open each one until one is considered valid.

  _log.Level (10) << "ScannerFile::GuessFormatOfFile   _fileName[" << _fileName << "]." << endl;

  FILE*  f = osFOPEN (_fileName.Str (), "rb");
  if  (!f)
    return Format::sfUnKnown;

  bool  endOfText = false;
  KKStr ln (100);
  ReadHeaderOneLine (f, endOfText, ln);
  fclose (f);

  KKStr fieldName = ln.ExtractToken2 ("\t");
  if (!fieldName.EqualIgnoreCase ("ScannerFile"))
  {
    _log.Level (-1) << "ScannerFile::GuessFormatOfFile   _fileName[" << _fileName << "]   unknown format." << endl;
    return Format::sfUnKnown;
  }
  else
  {
    KKStr  scannerFileFormatStr = ln.ExtractToken2 ("\t");
    _log.Level (10) << "ScannerFile::GuessFormatOfFile   _fileName[" << _fileName << "]   Format[" << scannerFileFormatStr << "]." << endl;
    return  ScannerFileFormatFromStr (scannerFileFormatStr);
  }
}  /*  GuessFormatOfFile */



void   ScannerFile::GetScannerFileParameters (const KKStr&             _scannerFileName,
                                              ScannerHeaderFieldsPtr&  _headerFields,
                                              Format&                  _scannerFileFormat,
                                              kkint32&                 _frameHeight,
                                              kkint32&                 _frameWidth,
                                              float&                   _scanRate,
                                              bool&                    _successful,
                                              RunLog&                  _log
                                             )
{
  _successful = false;
  ScannerFilePtr  sf = CreateScannerFile (_scannerFileName, _log);
  if  (sf == NULL)
  {
    _log.Level (-1) << "Could not determine scanner file format." << endl;
    return;
  }

  delete  _headerFields;
  _headerFields = new ScannerHeaderFields (*(sf->HeaderFields ()));
  _scannerFileFormat = sf->FileFormat        ();
  _frameHeight       = sf->FrameHeight       ();
  _frameWidth        = sf->PixelsPerScanLine ();
  _scanRate          = sf->ScanRate          ();

  _successful = true;
  delete  sf;
  sf = NULL;
}  /* GetScannerFileParameters */
                                     



ScannerFilePtr  ScannerFile::CreateScannerFile (KKStr    _fileName,
                                                RunLog&  _log
                                               )
{
  Format  format = GuessFormatOfFile (_fileName, _log);
  if  (format == Format::sfUnKnown)
    return NULL;

  if  (format == Format::sfSimple)
    return new ScannerFileSimple (_fileName, _log);

  if  (format == Format::sf2BitEncoded)
    return new ScannerFile2BitEncoded (_fileName, _log);

  if  (format == Format::sf3BitEncoded)
    return new ScannerFile3BitEncoded (_fileName, _log);

  if  (format == Format::sf4BitEncoded)
    return new ScannerFile4BitEncoded (_fileName, _log);

  if  (format == Format::sfZlib3BitEncoded)
    return new ScannerFileZLib3BitEncoded (_fileName, _log);

  return NULL;
}  /* CreateScanLineFile */




const uchar*  ScannerFile::ConpensationTable (Format  format)
{
  const uchar*  result = NULL;

  switch  (format)
  {
  case  Format::sfSimple:      
    result = ScannerFileSimple::CompensationTable ();
    break;

  case  Format::sf2BitEncoded:
    result = ScannerFile2BitEncoded::CompensationTable ();
    break;

  case  Format::sf3BitEncoded:
    result = ScannerFile3BitEncoded::CompensationTable ();
    break;

  case  Format::sf4BitEncoded:
    result = ScannerFile4BitEncoded::CompensationTable ();
    break;

  case  Format::sfUnKnown:
  case  Format::sfZlib3BitEncoded:
    result = NULL;
    break;
  }
  return  result;
}  /* ConpensationTable */



ScannerFilePtr  ScannerFile::CreateScannerFileForOutput (const KKStr&  _fileName,
                                                         Format        _format,
                                                         kkuint32      _pixelsPerScanLine,
                                                         kkuint32      _frameHeight,
                                                         RunLog&       _log
                                                        )
{
  ScannerFilePtr  scannerFile = NULL;
  if  (osFileExists (_fileName))
  {
    KKStr  errMsg;
    errMsg << "ScannerFile::CreateScannerFileForOutput   ***ERROR***   ScannerFile[" << _fileName << "]  already exists.";
    _log.Level (-1) << endl << endl << errMsg << endl << endl;
  }
  else
  {
    switch  (_format)
    {
    case  Format::sfSimple:      
      scannerFile = new ScannerFileSimple (_fileName, _pixelsPerScanLine, _frameHeight, _log);
      break;

    case  Format::sf2BitEncoded:
      scannerFile = new ScannerFile2BitEncoded (_fileName, _pixelsPerScanLine, _frameHeight, _log);
      break;

    case  Format::sf3BitEncoded:
      scannerFile = new ScannerFile3BitEncoded (_fileName, _pixelsPerScanLine, _frameHeight, _log);
      break;

    case  Format::sf4BitEncoded:
      scannerFile = new ScannerFile4BitEncoded (_fileName, _pixelsPerScanLine, _frameHeight, _log);
      break;

    case  Format::sfZlib3BitEncoded:
      scannerFile = new ScannerFileZLib3BitEncoded (_fileName, _pixelsPerScanLine, _frameHeight, _log);
      break;

    case  Format::sfUnKnown:
      scannerFile = NULL;
      break;

    }

    if  (!scannerFile)
    {
      KKStr  errMsg;
      errMsg << "ScannerFile::CreateScannerFileForOutput   ***ERROR***   Invalid ScannerFile Format specified.";
      _log.Level (-1) << endl << endl << errMsg << endl << endl;
    }
  }

  return  scannerFile;
}  /* CreateScannerFileForOutput */



  
ScannerFilePtr  ScannerFile::CreateScannerFileForOutput (const KKStr&   _fileName,
                                                         const KKStr&   _formatStr,
                                                         kkuint32       _pixelsPerScanLine,
                                                         kkuint32       _frameHeight,
                                                         RunLog&        _log
                                                        )
{
  ScannerFilePtr  scannerFile = NULL;

  Format  format = ScannerFileFormatFromStr (_formatStr);
  if  (format == Format::sfUnKnown)
  {
    _log.Level (-1) << endl << endl << "ScannerFile::CreateScannerFileForOutput  ***ERROR***   Invalid Format[" << _formatStr << "]" << endl << endl;
  }
  else
  {
    scannerFile = ScannerFile::CreateScannerFileForOutput
    		(_fileName, (Format)format, _pixelsPerScanLine, _frameHeight, _log);
  }

  return  scannerFile;
}  /* CreateScannerFileForOutput */





const  KKStr  ScannerFile::fileFormatOptions[]
    = {"Simple",
       "2BitEncoded",
       "3BitEncoded",
       "4BitEncoded",
       "Zlib3BitEncoded",
       "UnKnown"
      };


const KKStr&  ScannerFile::ScannerFileFormatToStr (Format  fileFormat)
{
  if  (((int)fileFormat < 0)  ||  (fileFormat >= Format::sfUnKnown))
    return fileFormatOptions[(int)Format::sfUnKnown];
  else
    return fileFormatOptions[(int)fileFormat];
}  /* ScannerFileFormatToStr */



KKStr  ScannerFile::FileFormatStr ()  const
{
  return ScannerFile::ScannerFileFormatToStr (FileFormat ());
}




ScannerFile::Format  ScannerFile::ScannerFileFormatFromStr (const KKStr&  fileFormatStr)
{
  kkint32  x = 0;
  while  (x < (kkint32)Format::sfUnKnown)
  {
    if  (fileFormatStr.EqualIgnoreCase (fileFormatOptions[x]))
      return (Format)x;
    ++x;
  }

  return  Format::sfUnKnown;
}  /* ScannerFileFormatFromStr */



void  ScannerFile::InitiateWritting ()
{
  AddHeaderField ("ScanRate",          StrFormatDouble (scanRate,          "##0.00"));
  AddHeaderField ("PixelsPerScanLine", StrFormatInt    (pixelsPerScanLine, "#####0"));
  AddHeaderField ("FrameHeight",       StrFormatInt    (frameHeight,       "#####0"));
  WriteHeader ();

  lastScanLine = 0;
  nextScanLine = 0;
  frameBufferFileOffsetLast = -1;
  frameBufferFileOffsetNext = osFTELL (file);
  byteOffsetScanLineZero = frameBufferFileOffsetNext;
  UpdateFrameOffset (0, 0, byteOffsetScanLineZero);
}  /* InitiateWritting */



void  ScannerFile::WriteHeader ()
{
  KKStr  ln (100);
  ln << "ScannerFile" << "\t" << FileFormatStr () << "\n";
  fwrite (ln.Str (), 1, ln.Len (), file);
  
  ScannerHeaderFields::const_iterator  idx;
  for  (idx = headerFields->begin ();  idx != headerFields->end ();  ++idx)
  {
    ln = "";
    ln << idx->first << "\t" << idx->second << "\n";
    fwrite (ln.Str (), 1, ln.Len (), file);
  }

  // Write End of text Marker
  char  ch = 0;
  fwrite (&ch, 1, 1, file);
  headerDataWritten = true;
}  /* WriteHeader */



void  ScannerFile::ReadHeaderOneLine (FILE*   f,
                                      bool&   endOfText,
                                      KKStr&  line
                                     )
{
  char  ch;
  line = "";
  endOfText = false;

  size_t  bytesReturned = fread (&ch, 1, 1, f);
  while  ((ch != 0)  &&  (ch != '\n')  &&  (bytesReturned > 0))
  {
    line.Append (ch);
    bytesReturned = fread (&ch, 1, 1, f);
  }

  if  ((ch == 0)  ||  (bytesReturned == 0))
    endOfText = true;
}  /* ReadHeaderOneLine */



void  ScannerFile::ReadHeader ()
{
  delete  headerFields;
  headerFields = new ScannerHeaderFields ();
  bool  endOfText = false;
  KKStr  ln (100);

  ReadHeaderOneLine (file, endOfText, ln);
  while  (!endOfText)
  {
    KKStr  fieldName   = ln.ExtractToken2 ("\t");
    KKStr  fieldValue  = ln.ExtractToken2 ("\t");
    headerFields->Add (fieldName, fieldValue);
    ExtractHeaderField (fieldName, fieldValue);
    ReadHeaderOneLine (file, endOfText, ln);
  }
}  /* ReadHeader */




void  ScannerFile::ExtractHeaderField (const KKStr&  fieldName,
                                       const KKStr&  fieldValue
                                      )
{
  if  (fieldName.EqualIgnoreCase ("FrameHeight"))
  {
    kkuint32 fieldValueUint = fieldValue.ToInt32 ();
    if  ((fieldValueUint > 0)  &&  (fieldValueUint < (1024 * 1024)))
      frameHeight = fieldValue.ToInt32 ();
  }

  else if  (fieldName.EqualIgnoreCase ("PixelsPerScanLine"))
    pixelsPerScanLine = fieldValue.ToInt ();
  
  else if  (fieldName.EqualIgnoreCase ("ScanRate"))
    scanRate = fieldValue.ToFloat ();

  else if  (fieldName.EqualIgnoreCase ("FlatFieldCorrectionEnabled"))
    flatFieldEnabled = fieldValue.ToBool ();

}  /* ExtractHeaderField */



void  ScannerFile::AddHeaderField (const KKStr&  _fieldName,
                                   const KKStr&  _fieldValue
                                  )
{
  if  (!headerFields)
    headerFields = new ScannerHeaderFields ();
  headerFields->Add (_fieldName, _fieldValue);
  ExtractHeaderField (_fieldName, _fieldValue);
} /* AddHeaderField */



void  ScannerFile::AddHeaderFields (const ScannerHeaderFieldsPtr  _headerFields)
{
  _headerFields->StartBlock ();

  ScannerHeaderFields::const_iterator  idx;
  for  (idx = _headerFields->begin ();  idx != _headerFields->end ();  ++idx)
    AddHeaderField (idx->first, idx->second);

  _headerFields->EndBlock ();
}  /* AddHeaderFields */



const KKStr&  ScannerFile::GetValue (const KKStr&  fieldName)
{
  if  (!headerFields)
    return KKStr::EmptyStr ();
  else
    return headerFields->GetValue (fieldName);
}


float  ScannerFile::GetValueFloat (const KKStr&  fieldName)
{
  if  (!headerFields)
    return 0.0f;
  else
    return headerFields->GetValueFloat (fieldName);
}



void  ScannerFile::ReportTextMsg (const char*  textBuff, 
                                  kkint32      numTextBytes
                                 )
{
  KKStr  s (textBuff, 0, numTextBytes - 1);
  if  (s.StartsWith ("InstrumentDataWord\t", true))
  {
    s.ExtractToken2 ("\t");
    kkint32  idNum = s.ExtractTokenInt ("\t");
    kkuint32 scanLineNum = s.ExtractTokenUint ("\t");
    kkuint32 dataWord = s.ExtractTokenUint ("\t");
    WordFormat32Bits  w (dataWord);
    ReportInstrumentDataWord ((uchar)idNum, scanLineNum, w);
  }
  else
  {
    /**
     *@todo  Need to add code to do something with the textBuff message.
     */
  }
}  /* ReportTextMsg */





void  ScannerFile::ReportInstrumentDataWord (uchar             idNum,
                                             kkuint32          scanLineNum,
                                             WordFormat32Bits  dataWord
                                            )
{
  /**
   *@todo  Need to add code to do something the data word just posted.
   */

  if  (idNum == 0)
  {
    // This is the FlowMeterCounter field,
    flowMeterCounter = dataWord.unsigned32BitInt;
    flowMeterCounterScanLine = scanLineNum;
  }
}  /* ReportInstrumentDataWord */




/**
 *@brief  Updates the byte offset for specified entry in 'frameOffsets' that is currently flagged with "-1".
 *@details  This is meant to update entries in 'frameOffsets' that were flagged with '-1'.  It will not work
 * for frameNum == 0;  this is okay since frameNum == 0 is updated when the scanner file is first opened.
 * It is also assumed that the entry in frameOfsets just before 'frameNum' is already properly updated.
 */
void  ScannerFile::DetermineFrameOffsetForFrame (kkuint32  frameNum)
{
  if  ((frameNum < 1)  ||  (frameNum >= frameOffsets.size ()))
    return;

  CreateGoalie ();
  goalie->StartBlock ();

  FSeek (frameOffsets[frameNum - 1]);

  frameOffsets[frameNum] = SkipToNextFrame ();

  goalie->EndBlock ();
}  /* DetermineFrameOffsetForFrame */



void  ScannerFile::BuildFrameOffsets (const volatile bool&  cancelFlag)
{
  CreateGoalie ();
  goalie->StartBlock ();

  kkint64  origFilePos = osFTELL (file);

  frameOffsetsBuildRunning = true;

  bool  changesMadeToIndexFile = false;
  bool  loadSuccessful = false;
  LoadIndexFile (loadSuccessful);
  if  (!loadSuccessful)
    changesMadeToIndexFile = true;

  {
    if  (frameOffsets.size () < 1)
      frameOffsets.push_back (byteOffsetScanLineZero);

    // Update any entries that are pointing to -1.
    for  (kkuint32 frameNum = 0;  frameNum < frameOffsets.size ();  ++frameNum)
    {
      if  (frameOffsets[frameNum] < 0)
      {
        if  (goalie->NumBlockedThreads () > 0)
        {
          // Since other threads are currently blocked trying to get access to this resource;  we will release
          // it for 10 mili-secs before we continue.
          FSeek (origFilePos);
          goalie->EndBlock ();
          osSleepMiliSecs (10);
          goalie->StartBlock ();
          origFilePos = osFTELL (file);
        }
         changesMadeToIndexFile = true;
        DetermineFrameOffsetForFrame (frameNum);
      }
    }

    bool  readToEOF = false;
    while  ((!readToEOF)  &&  (!cancelFlag))
    {
      kkuint32  loopCount = 0;

      if  (goalie->NumBlockedThreads () > 0)
      {
        // Give other threads a chance to access this instance of ScannerFile.
        FSeek (origFilePos);
        goalie->EndBlock ();
        while  ((goalie->NumBlockedThreads () > 0)  &&  (loopCount < 10))
        {
          osSleepMiliSecs (10);
          ++loopCount;
        }
        goalie->StartBlock ();
        origFilePos = osFTELL (file);
      }

      kkuint32  lastFrameNum = (kkint32)frameOffsets.size () - 1;

      // Reposition to beginning of last frame that is recorded in frameOfsets.
      FSeek (frameOffsets[lastFrameNum]);
      
      kkint64  nextFrameByteOffset = SkipToNextFrame ();
      while  ((nextFrameByteOffset >= 0)  &&  (goalie->NumBlockedThreads () < 1))
      {
        kkint32 fileSizeInScanLines = (lastFrameNum + 1) * frameHeight;
        if  (fileSizeInScanLines > largestKnownScanLine)
          largestKnownScanLine = fileSizeInScanLines;
        frameOffsets.push_back (nextFrameByteOffset);
        ++lastFrameNum;
        changesMadeToIndexFile = true;
        nextFrameByteOffset = SkipToNextFrame ();
      }

      if  (nextFrameByteOffset < 0)
       readToEOF = true;
    }

    if  ((!cancelFlag)  &&  changesMadeToIndexFile)
      SaveIndexFile ();
  }

  if  (!cancelFlag)
    frameOffsetsLoaded = true;

  FSeek (origFilePos);

  goalie->EndBlock ();

  frameOffsetsBuildRunning = false;

  return;
}  /* BuildFrameOffsets */



void  ScannerFile::SaveIndexFile ()
{
  indexFileName = osRemoveExtension (fileName) + ".idx";
  ofstream f (indexFileName.Str ());
  f << "IndexFile"                                     << endl
    << "ScannerFile" << "\t" << fileName               << endl
    << "DateTime"    << "\t" << osGetLocalDateTime ()  << endl;

  f << "IndexEntryFields" << "\t" << "FrameNum"
                          << "\t" << "ScanLineNum"
                          << "\t" << "ByteOffset"
                          << endl;

  for  (kkuint32 frameNum = 0;  frameNum < frameOffsets.size ();  ++frameNum)
  {
    f << "IndexEntry" << "\t" << frameNum << "\t" << (frameNum * frameHeight) << "\t" << frameOffsets[frameNum] << endl;
  }

  StartStopPointList::const_iterator  idx;
  for  (idx = startStopPoints.begin ();  idx != startStopPoints.end ();  ++idx)
  {
    StartStopPointPtr ssp = *idx;
    f << "StartStopPoint" << "\t" << ssp->ToTabDelStr () << endl;
  }

  f.close ();
}  /* SaveIndexFile */




void  ScannerFile::LoadIndexFile (bool&  successful)
{
  if  (indexFile)
  {
    delete  indexFile;
    indexFile = NULL;
  }

  indexFileName = osRemoveExtension (fileName) + ".idx";
  FILE* f = osFOPEN (indexFileName.Str (), "r");

  if  (!f)
  {
    log.Level (-1) << "LoadIndexFile  IndexFile[" << indexFileName << "] does not exist." << endl;
    successful = false;
    return;
  }

  KKStrPtr  ln = NULL;

  while  (true)
  {
    delete ln;
    ln = KKB::osReadRestOfLine (f, eof);
    if  (eof)  break;
    if  (!ln)  continue;

    KKStr lineName = ln->ExtractToken2 ("\t\n\r");

    if  (lineName.EqualIgnoreCase ("IndexEntry"))
    {
      kkuint32 frameNum    = ln->ExtractTokenUint   ("\t\n\r");
      kkint32  scanLineNum = ln->ExtractTokenInt    ("\t\n\r");
      kkuint64 byteOffset  = ln->ExtractTokenUint64 ("\t\n\r");
      UpdateFrameOffset (frameNum, scanLineNum, byteOffset);
      if  (scanLineNum > largestKnownScanLine)
        largestKnownScanLine = scanLineNum;
    }

    else if  (lineName.EqualIgnoreCase ("ClearStartStopPoints"))
    {
      startStopPoints.Clear ();
    }

    else if  (lineName.EqualIgnoreCase ("StartStopPoint"))
    {
      StartStopPointPtr  entry = new StartStopPoint (*ln);
      if  (entry->Type () == StartStopPoint::StartStopType::Invalid)
      {
        delete  entry;
        entry = NULL;
      }
      else
      {
        startStopPoints.AddEntry (entry);
      }
    }

    else if  (lineName.EqualIgnoreCase ("DeleteStartStopPoint"))
    {
      kkint32  scanLineNum = ln->ExtractTokenInt ("\n\t\r");
      startStopPoints.DeleteEntry (scanLineNum);
    }
  }
  delete  ln;
  ln = NULL;

  fclose (f);
  successful = true;
}  /* LoadIndexFile */




void  ScannerFile::SkipBytesForward (kkuint32  numBytes)
{
  kkint32  returnCd = osFSEEK (file, numBytes, SEEK_CUR);
  if  (returnCd != 0)
  {
	log.Level (-1) << "ScannerFile::SkipBytesForward   ReturnCd = " << returnCd << endl;
  }
}  /* SkipBytesForward */





kkint32  ScannerFile::FSeek (kkint64  filePos)
{
  kkint32  returnCd = 0;
  returnCd = osFSEEK (file, filePos, SEEK_SET);
  return  returnCd;
}  /* FSeek */
