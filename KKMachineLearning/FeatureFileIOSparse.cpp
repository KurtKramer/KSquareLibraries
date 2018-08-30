#include "FirstIncludes.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "FeatureFileIOSparse.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;


FeatureFileIOSparse  FeatureFileIOSparse::driver;


FeatureFileIOSparse::FeatureFileIOSparse ():
   FeatureFileIO ("Sparse", true, true)
{
}



FeatureFileIOSparse::~FeatureFileIOSparse ()
{
}



FileDescConstPtr  FeatureFileIOSparse::GetFileDesc (const KKStr&    _fileName,
                                                    istream&        _in,
                                                    MLClassListPtr  _classes,
                                                    kkint32&        _estSize,
                                                    KKStr&          _errorMessage,
                                                    RunLog&         _log
                                                   )
{
  _log.Level (20) << "FeatureFileIOSparse::GetFileDesc     FileName[" << _fileName << "]." << endl;
  bool  eof = false;
  bool  eol = true;

  _estSize = 0;

  _errorMessage = "";

  kkint32  featureNumMin = int32_max;
  kkint32  featureNumMax = int32_min;

  while  (!eof)
  {
    KKStr  className;
    GetToken (_in, " \t", className, eof, eol);
    if  (eof)
      break;

    if  (eol)
    {
      //We have a blank line; we will ignore this line.
      continue;
    }

    if  (className.SubStrPart (0, 1) == "//")
    {
      // We have a comment line.  We will skip to end of line
      while  ((!eol)  &&  (!eof))
        GetToken (_in, " \t", className, eof, eol);
      continue;
    }

    MLClassPtr  mlClass = _classes->GetMLClassPtr (className);

    mlClass = _classes->GetMLClassPtr (className);

    KKStr  field;
    GetToken (_in, " \t", field, eof, eol);
    while  (!eol)
    {
      KKStr  featureNumStr = field.ExtractToken (":");
      kkint32 featureNum = atoi (featureNumStr.Str ());

      if  (featureNum > featureNumMax)
        featureNumMax = featureNum;

      if  (featureNum < featureNumMin)
        featureNumMin = featureNum;
     GetToken (_in, " \t", field, eof, eol);
    }
    _estSize++;
  }

  FileDescPtr  fileDesc = new FileDesc ();

  fileDesc->SparseMinFeatureNum (featureNumMin);

  for  (kkint32 fieldNum = featureNumMin;  fieldNum <= featureNumMax;  fieldNum++)
  {
    bool  alreadyExists = false;
    fileDesc->AddAAttribute ("Field_" + StrFormatInt (fieldNum, "ZZZZ0"), AttributeType::Numeric, alreadyExists);
  }

  return  fileDesc;
}  /* GetFileDesc */



FeatureVectorListPtr  FeatureFileIOSparse::LoadFile (const KKStr&      _fileName,
                                                     FileDescConstPtr  _fileDesc,
                                                     MLClassList&      _classes, 
                                                     istream&          _in,
                                                     kkint32           _maxCount,    // Maximum # images to load.
                                                     VolConstBool&     _cancelFlag,
                                                     bool&             _changesMade,
                                                     KKStr&            _errorMessage,
                                                     RunLog&           _log
                                                   )
{
  _log.Level (10) << endl 
      << "FeatureFileIOSparse::LoadFile   ***ERROR***   Roberts  LoadFile Functionality not implemented." << endl 
      << "    _fileName   : " << _fileName << endl
      << "    _fileDesc   : " << _fileDesc->NumOfFields () << endl
      << "    _classes    : " << _classes.ToCommaDelimitedStr () << endl
      << "    _in.flags   : " << _in.flags () << endl
      << "    _maxCount   : " << _maxCount << endl
      << "    _cancelFlag : " << _cancelFlag << endl
      << "    _changesMade: " << _changesMade << endl
      << endl;
  
  bool  eof = false;
  bool  eol = true;

  KKStr  rootName = osGetRootName (_fileName);

  kkint32  numOfFeatures = _fileDesc->NumOfFields ();

  kkint32  lineCount = 0;

  kkint32  minFeatureNum = _fileDesc->SparseMinFeatureNum ();
  kkint32  maxFeatureNum = minFeatureNum + numOfFeatures - 1;

  if  (_maxCount < 1)
    _maxCount = int32_max;

  FeatureVectorListPtr  examples = new FeatureVectorList (_fileDesc, true);

  while  ((!eof)   &&  (!_cancelFlag)  &&  ((kkint32)examples->size () < _maxCount))
  {
    KKStr  className;
    
    GetToken (_in, " \t", className, eof, eol);
    if  (eof)
      break;

    if  (eol)
    {
      // We have a blank line;
      continue;
    }

    if  (className.SubStrPart (0, 1) == "//")
    {
      // We have a coment line.  We will skip to end of line
      while  ((!eol)  &&  (!eof))
        GetToken (_in, " \t", className, eof, eol);
      continue;
    }

    MLClassPtr mlClass = _classes.GetMLClassPtr (className);

    FeatureVectorPtr  example = new FeatureVector (numOfFeatures);
    example->MLClass (mlClass);

    KKStr  exampleName = rootName + "_" + StrFormatInt (lineCount, "ZZZZZZ0");
    example->ExampleFileName (exampleName);

    KKStr  field = "";
    GetToken (_in, " \t", field, eof, eol);
    while  ((!eol)  &&  (!eof))  
    {
      KKStr  featureNumStr = field.ExtractToken (":");
      kkint32 featureNum = atoi (featureNumStr.Str ());
      
      if  ((featureNum < minFeatureNum)  ||  (featureNum > maxFeatureNum))
      {
        _log << endl << endl
             << "FeatureFileIOSparse::LoadFile  FeatureNum[" <<  featureNumStr << "] out of range." << endl
             << "                               FileName["  << _fileName << "]  LineNum[" << lineCount << "]."  << endl
             << endl;
        _errorMessage << "FeatureNum[" << featureNumStr << "]  is out of range.";
        delete  example;   example  = NULL;
        delete  examples;  examples = NULL;
        return  NULL;
      }

      featureNum = featureNum - minFeatureNum;

      float  value = (float)atof (field.Str ());
      example->AddFeatureData (featureNum,  value);
      GetToken (_in, " \t", field, eof, eol);
    }

    examples->PushOnBack (example);

    lineCount++;
  }

  _log.Level (10) << "FeatureFileIOSparse::LoadFile  Done" << endl
      << "    _cancelFlag : " << _cancelFlag << endl
      << "    _changesMade: " << _changesMade << endl
      << endl;

  return  examples;
}  /* LoadFile */



void   FeatureFileIOSparse::SaveFile (FeatureVectorList&    _data,
                                      const KKStr&          _fileName,
                                      FeatureNumListConst&  _selFeatures,
                                      ostream&              _out,
                                      kkuint32&             _numExamplesWritten,
                                      VolConstBool&         _cancelFlag,
                                      bool&                 _successful,
                                      KKStr&                _errorMessage,
                                      RunLog&               _log
                                     )
{
  _log.Level (-1) << endl
      << "FeatureFileIOSparse::SaveFile    ***ERROR***   not implemented." << endl
      << "     _fileName    : " << _fileName << endl
      << "     _selFeatures : " << _selFeatures.ToCommaDelStr () << endl
      << "     _out.fail    : " << _out.fail () << endl
      << "     _cancelFlag  : " << _cancelFlag << endl
      << endl;

  _errorMessage = "";

  FeatureVectorPtr  example  = NULL;
  FileDescConstPtr  fileDesc = _data.FileDesc ();

  _numExamplesWritten = 0;

  kkint32  minFeatureNum = fileDesc->SparseMinFeatureNum ();

  for  (kkint32 idx = 0;  (idx < _data.QueueSize ()) && (!_cancelFlag);  idx++)
  {
    example = _data.IdxToPtr (idx);

    _out << example->MLClassName ();

    for  (kkint32 x = 0; x < _selFeatures.NumOfFeatures (); x++)
    {
      kkint32  featureNum = _selFeatures[x];
      float value = example->FeatureData (featureNum);
      if  (value != (float)0.0)
        _out << " " << (featureNum + minFeatureNum) << ":" << example->FeatureData (featureNum);
    }
    _out << endl;
    _numExamplesWritten++;
  }

  if  (!_cancelFlag)
    _successful = true;

  _log.Level (50) << "FeatureFileIOSparse::SaveFile  _successful: " << _successful << "  _cancelFlag: " << _cancelFlag << "  _numExamplesWritten: " << _numExamplesWritten << endl;
  return;
}  /* SaveFile */
