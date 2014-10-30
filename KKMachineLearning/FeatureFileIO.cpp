#include  "FirstIncludes.h"

#include <stdio.h>
#include <math.h>


#include <ctype.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include "MemoryDebug.h"


using namespace std;


#include "KKBaseTypes.h"
#include "DateTime.h"
#include "GlobalGoalKeeper.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "FeatureFileIO.h"
#include "FeatureFileIOArff.h"
#include "FeatureFileIOC45.h"
#include "FeatureFileIOColumn.h"
#include "FeatureFileIODstWeb.h"
//#include "FeatureFileIOKK.h"
#include "FeatureFileIORoberts.h"
#include "FeatureFileIOSparse.h"
#include "FeatureFileIOUCI.h"

#include "FileDesc.h"
#include "GoalKeeper.h"
#include "MLClass.h"
using namespace KKMachineLearning;






void  ReportError (RunLog&        log,
                   const KKStr&  fileName,
                   const KKStr&  funcName,
                   kkint32          lineCount,
                   const KKStr&  errorDesc
                  )
{
  log.Level (-1) << endl
                 << funcName << "     *** ERROR ***" << endl
                 << "           File     [" << fileName      << "]" << endl
                 << "           LineCount[" << lineCount     << "]" << endl
                 << "           Error    [" << errorDesc     << "]" << endl
                 << endl;
}  /* ReportError */







vector<FeatureFileIOPtr>*  FeatureFileIO::registeredDrivers = NULL;


std::vector<FeatureFileIOPtr>*  FeatureFileIO::RegisteredDrivers  ()
{
  if  (registeredDrivers == NULL)
  {
    RegisterAllDrivers ();
  }

  return  registeredDrivers;
}


void  FeatureFileIO::RegisterFeatureFileIODriver (FeatureFileIOPtr  _driver)
{
  GlobalGoalKeeper::StartBlock ();
  if  (!registeredDrivers)
    registeredDrivers = new std::vector<FeatureFileIOPtr> ();

  registeredDrivers->push_back (_driver);

  GlobalGoalKeeper::EndBlock ();
} /* RegisterFeatureFileIODriver */



void  FeatureFileIO::RegisterAllDrivers ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (registeredDrivers == NULL)
  {
    registeredDrivers = new std::vector<FeatureFileIOPtr> ();
    registeredDrivers->push_back (new FeatureFileIOArff    ());
    registeredDrivers->push_back (new FeatureFileIOC45     ());
    registeredDrivers->push_back (new FeatureFileIOColumn  ());
    registeredDrivers->push_back (new FeatureFileIODstWeb  ());
    //registeredDrivers->push_back (new FeatureFileIOKK      ());
    registeredDrivers->push_back (new FeatureFileIORoberts ());
    registeredDrivers->push_back (new FeatureFileIOSparse  ());
    registeredDrivers->push_back (new FeatureFileIOUCI     ());
  }

  GlobalGoalKeeper::EndBlock ();
}  /* RegisterAllDrivers */




/**
 * @brief  Before you terminate your application and after all FeatureFileIO activity is done
 *         call this method to destroy statically dynamically allocated data structures.
 * @details This method undoes everything that 'RegisterAllDrivers' does.  Its primary
 *          usefulness is aid in tracking down memory leaks.  This way the '_CrtSetDbgFlag'
 *          debugging functionality will not report the memory allocated by the different
 *          feature FileIO drivers.
 */
void FeatureFileIO::FinalCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (registeredDrivers)
  {
    while  (registeredDrivers->size () > 0)
    {
      FeatureFileIOPtr  f = registeredDrivers->back ();
      registeredDrivers->pop_back ();
      delete  f;
    }

    delete  registeredDrivers;
    registeredDrivers = NULL;
  }

  GlobalGoalKeeper::EndBlock ();
}  /* CleanUpFeatureFileIO */





FeatureFileIOPtr  FeatureFileIO::LookUpDriver (const KKStr&  _driverName)
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  KKStr  driverNameLower = _driverName.ToLower ();
  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    if  ((*idx)->driverNameLower == driverNameLower)
      return  *idx;
  }
  
  return  NULL;
}  /* LookUpDriver */




FeatureFileIOPtr  FeatureFileIO::FileFormatFromStr  (const KKStr&  _fileFormatStr)
{
  return  LookUpDriver (_fileFormatStr);
}




FeatureFileIOPtr   FeatureFileIO::FileFormatFromStr (const KKStr&  _fileFormatStr,
                                                     bool          _canRead,
                                                     bool          _canWrite
                                                    )
{
  FeatureFileIOPtr   driver = LookUpDriver (_fileFormatStr);
  if  (!driver)
    return NULL;

  if  (_canRead  &&  (!driver->CanRead ()))
    return NULL;

  if  (_canWrite  &&  (!driver->CanWrite ()))
    return NULL;

  return  driver;
}  /* FileFormatFromStr */





KKStr  FeatureFileIO::FileFormatsReadOptionsStr ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  KKStr  driversThatCanRead (128);

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanRead ())
    {
      if  (!driversThatCanRead.Empty ())
        driversThatCanRead << ", ";
      driversThatCanRead << driver->DriverName ();
    }
  }
  return driversThatCanRead;
}  /* FileFormatsReadOptionsStr */



KKStr  FeatureFileIO::FileFormatsWrittenOptionsStr ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  KKStr  driversThatCanWrite (128);

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanWrite ())
    {
      if  (!driversThatCanWrite.Empty ())
        driversThatCanWrite << ", ";
      driversThatCanWrite << driver->DriverName ();
    }
  }
  return driversThatCanWrite;
}  /* FileFormatsWrittenOptionsStr */




KKStr  FeatureFileIO::FileFormatsReadAndWriteOptionsStr ()
{
  KKStr  driversThatCanReadAndWrite (128);

  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanWrite ()  &&  driver->CanWrite ())
    {
      if  (!driversThatCanReadAndWrite.Empty ())
        driversThatCanReadAndWrite << ", ";
      driversThatCanReadAndWrite << driver->DriverName ();
    }
  }
  return driversThatCanReadAndWrite;
}  /* FileFormatsReadAndWriteOptionsStr */




VectorKKStr  FeatureFileIO::RegisteredDriverNames ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();
  VectorKKStr  names;
  vector<FeatureFileIOPtr>::iterator  idx;

  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
    names.push_back ((*idx)->DriverName ());

  return  names;
}  /* RegisteredDriverNames */




void  FeatureFileIO::RegisterDriver (FeatureFileIOPtr  _driver)
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();
  FeatureFileIOPtr  existingDriver = LookUpDriver (_driver->driverName);
  if  (existingDriver)
  {
    // We are trying to register two drivers with the same name;  we can not do that.
    cerr << endl << endl 
         << "FeatureFileIO::RegisterDriver     ***ERROR***" << endl 
         << endl
         << "             trying to register more than one FeatureFileIO driver with " << endl
         << "             the same name[" << _driver->driverName << "]." << endl
         << endl;
    return;
  }

  drivers->push_back (_driver);
}  /* RegisterDriver */






FeatureFileIO::FeatureFileIO (const KKStr&  _driverName,
                              bool          _canRead,
                              bool          _canWrite
                             ):
   canRead         (_canRead),
   canWrite        (_canWrite),
   driverName      (_driverName),
   driverNameLower (_driverName.ToLower ())
{
}


   
FeatureFileIO::~FeatureFileIO ()
{
}



void  FeatureFileIO::GetLine (istream&  _in,
                              KKStr&    _line,
                              bool&     _eof
                             )
{
  _line = "";
  if  (_in.eof ())
  {
    _eof = true;
    return;
  }

  kkint32  ch = _in.peek ();
  while  ((ch != '\n')  &&  (ch != '\r')  &&  (!_in.eof ()))
  {
    ch = _in.get ();
    _line.Append (ch);
    ch = _in.peek ();
  }

  if  (!_in.eof ())
  {
    _in.get ();  // Skip over eond of line character.
    if  (ch == '\n')
    {
      ch = _in.peek ();
      if  (ch == '\r')
        _in.get ();  // line is terminated by LineFeed + CarrageReturn;  we need to skip over both.
    }
    else if  (ch  == '\r')
    {
      ch = _in.peek ();
      if  (ch == '\n')
        _in.get ();  // line is terminated by CarrageReturn + LineFeed;  we need to skip over both.
    }
  }

  _eof = false;

  return;
}  /* GetLine */



void  FeatureFileIO::GetToken (istream&     _in,
                               const char*  _delimiters,
                               KKStr&       _token,
                               bool&        _eof, 
                               bool&        _eol
                              )
{
  _token = "";
  _eof = false;
  _eol = false;

  if  (_in.eof ())
  {
    _eof = true;
    _eol = true;
    return;
  }

  kkint32  ch;

  // Skip past any leading white space.
  ch = _in.peek ();
  while  ((ch == ' ')  &&  (!_in.eof ()))
  {
    _in.get ();
    ch = _in.peek ();
  }

  if  (_in.eof ())
  {
    _eof = true;
    _eol = true;
    return;
  }

  if  (ch == '\n')
  {
    _eol = true;
    _in.get ();
    if  (_in.peek () == '\r')
      _in.get ();
    return;
  }

  if  (ch == '\r')
  {
    _eol = true;
    _in.get ();
    if  (_in.peek () == '\n')
      _in.get ();
    return;
  }


  while  ((!_in.eof ())  &&  (ch != '\n')  &&  (ch != '\r')  &&  (strchr (_delimiters, ch) == NULL))
  {
    _in.get ();
    _token.Append ((char)ch);
    ch = _in.peek ();
  }

  if  (strchr (_delimiters, ch) != NULL)
  {
    // the next character was a delimiter;  in this case we want to remove from stream.
    _in.get ();
  }


  return;
}  /* GetToken */







FeatureVectorListPtr  FeatureFileIO::LoadFeatureFile 
                                      (const KKStr&   _fileName,
                                       MLClassList&   _mlClasses,
                                       kkint32        _maxCount,
                                       VolConstBool&  _cancelFlag,    // will be monitored,  if set to True  Load will terminate.
                                       bool&          _successful,
                                       bool&          _changesMade,
                                       RunLog&        _log
                                      )
{
  _log.Level (10) << "LoadFeatureFile  File[" << _fileName << "]  FileFormat[" << driverName << "]" << endl;
  
  _changesMade = false;

  kkint32  estimatedNumOfDataItems = -1;
  
  _successful = true;

  ifstream  in (_fileName.Str (), ios_base::in);
  if  (!in.is_open ())
  {
    _log.Level (-1) << "LoadFeatureFile   ***ERROR***      Error Opening File[" << _fileName << "]." << endl;
    _successful = false;
    return  NULL;
  }

  KKStr  errorMessage;
  
  FileDescPtr  fileDescFromFile = GetFileDesc (_fileName, in, &_mlClasses, estimatedNumOfDataItems, errorMessage, _log);
  if  (fileDescFromFile == NULL)
  {
    _log.Level (-1) << endl << endl 
                   << "FeatureFileIO::LoadFeatureFile    ***ERROR***     Loading Feature File[" << _fileName << "]" << endl
                   << endl;
    _successful = false;
    return NULL;
  }

  FileDescPtr fileDesc = FileDesc::GetExistingFileDesc (fileDescFromFile);

  in.clear ();
  in.seekg (0, ios::beg);

  FeatureVectorListPtr  examples = LoadFile (_fileName, fileDesc, _mlClasses, in, _maxCount, _cancelFlag, _changesMade, errorMessage, _log);
  if  (examples == NULL)
  {
    _successful = false;
  }
  else
  {
    examples->Compress ();
    _successful = true;
  }

  in.close ();

  return  examples;
}  /* LoadFeatureFile */




void   FeatureFileIO::AppendToFile (const KKStr&           _fileName,
                                    const FeatureNumList&  _selFeatures,
                                    FeatureVectorList&     _examples,
                                    kkuint32&              _numExamplesWritten,
                                    VolConstBool&          _cancelFlag,
                                    bool&                  _successful,
                                    RunLog&                _log
                                   )
{
  _log.Level (10) << "FeatureFileIO::AppendToFile - File[" << _fileName << "]." << endl;

  _successful = true;

  FileDescPtr  fileDesc = _examples.FileDesc ();

  ofstream out (_fileName.Str (), ios::app);

  if  (!out.is_open())
  {
    KKStr  err;
    err << "AppendToFile  Error Opening File[" << _fileName << "]";
    _log.Level (-1)  << endl
                     << "FeatureFileIO::AppendToFile    ***ERROR***"  << endl
                     << endl
                     << "                   " << err  << endl
                     << endl;
    osDisplayWarning (err);
    _successful = false;
    return;
  }

  KKStr  errorMessage;
  SaveFile (_examples, _fileName,  _selFeatures, out, _numExamplesWritten, _cancelFlag, _successful, errorMessage, _log);

  out.close ();

  return;
}  /* AppendToFile */




void  FeatureFileIO::SaveFeatureFile (const KKStr&           _fileName, 
                                      const FeatureNumList&  _selFeatures,
                                      FeatureVectorList&     _examples,
                                      kkuint32&              _numExamplesWritten,
                                      VolConstBool&          _cancelFlag,
                                      bool&                  _successful,
                                      RunLog&                _log
                                     )
{
  _log.Level (10) << "FeatureFileIO::SaveFeatureFile - File[" << _fileName << "]." << endl;

  ofstream  out (_fileName.Str ());
  if  (!out.is_open())
  {
    _log.Level (-1) << "***ERROR***, SaveFeatureFile, Opening File[" << _fileName << "]" << endl;
    _successful = false;
  }

  out.precision (9);

  FileDescPtr  fileDesc = _examples.FileDesc ();

  KKStr  errorMessage;
  SaveFile (_examples, _fileName, _selFeatures, out, _numExamplesWritten, _cancelFlag, _successful, errorMessage, _log);

  out.close ();
}  /* SaveFeatureFile */





void  FeatureFileIO::SaveFeatureFileMultipleParts (const KKStr&           _fileName, 
                                                   const FeatureNumList&  _selFeatures,
                                                   FeatureVectorList&     _examples,
                                                   VolConstBool&          _cancelFlag,
                                                   bool&                  _successful,
                                                   RunLog&                _log
                                                  )
{
  kkuint32  numExamplesWritten = 0;
  SaveFeatureFile (_fileName, _selFeatures, _examples, numExamplesWritten, _cancelFlag, _successful, _log);

  if  (_cancelFlag  ||  (!_successful))
    return;

  if  (_examples.QueueSize () > 64000)
  {
    kkint32  numPartsNeeded = (_examples.QueueSize () / 64000);
    if  ((_examples.QueueSize () % 64000) > 0)
      numPartsNeeded++;

    kkint32  maxPartSize = (_examples.QueueSize () / numPartsNeeded) + 1;

    kkint32  partNum = 0;
    FeatureVectorList::const_iterator idx = _examples.begin ();

    while  ((idx != _examples.end ())  &&  (_successful)  &&  (!_cancelFlag))
    {
      FeatureVectorListPtr  part = new FeatureVectorList (_examples.FileDesc (), false, _log);

      while  ((idx != _examples.end ())  &&  (part->QueueSize () < maxPartSize))
      {
        part->PushOnBack (*idx);
        idx++;
      }

      KKStr  partFileName = osRemoveExtension (_fileName) + "-" + 
                            StrFormatInt (partNum, "00") + "." +
                            osGetFileExtension (_fileName);

      SaveFeatureFile (partFileName, _selFeatures, *part, numExamplesWritten, _cancelFlag, _successful, _log);

      partNum++;
      delete  part; part = NULL;
    }
  }
}  /* SaveFeatureFileMultipleParts */





FeatureVectorListPtr  FeatureFileIO::FeatureDataReSink (KKStr           _dirName, 
                                                        const KKStr&    _fileName, 
                                                        MLClassPtr      _unknownClass,
                                                        bool            _useDirectoryNameForClassName,
                                                        MLClassList&    _mlClasses,
                                                        VolConstBool&   _cancelFlag,
                                                        bool&           _changesMade,
                                                        KKB::DateTime&  _timeStamp,
                                                        RunLog&         _log
                                                      )
{
  return NULL;
}
