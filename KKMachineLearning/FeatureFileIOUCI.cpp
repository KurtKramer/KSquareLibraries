#include "FirstIncludes.h"

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "MemoryDebug.h"

using namespace std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;


#include "FeatureFileIOUCI.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;



FeatureFileIOUCI   FeatureFileIOUCI::driver;


FeatureFileIOUCI::FeatureFileIOUCI ():
  FeatureFileIO ("UCI", true, true)
{
}



FeatureFileIOUCI::~FeatureFileIOUCI ()
{
}



FileDescPtr  FeatureFileIOUCI::GetFileDesc (const KKStr&       _fileName,
                                            istream&           _in,
                                            MLClassListPtr  _classes,
                                            kkint32&           _estSize,
                                            KKStr&             _errorMessage,
                                            RunLog&            _log
                                           )
{
  _log.Level (20) << "FeatureFileIOUCI::GetFileDesc     FileName[" << _fileName << "]." << endl;


  // We are just going to read the first few lines to determine number of fields, etc
 
  kkint32  numOfFields       = 0;
  kkint32  numFieldsThisLine = 0;


  KKStr  ln;
  bool   eof;

  _estSize = 0;

  GetLine (_in, ln, eof);
  while  (!eof)
  {
    ln.TrimLeft ();
    ln.TrimRight ();
    if  ((ln.SubStrPart (0, 1) != "//")  &&  (!ln.Empty ()))
    {
      numFieldsThisLine = 0;

      KKStr  className = ln.ExtractToken (" ,\n\r\t");
      while (!ln.Empty ())
      {
        numFieldsThisLine++;
        className = ln.ExtractToken (" ,\n\r\t");
      }

      if  (className.Empty ())
        className = "UnKnown";

      // make sure that 'className' exists in '_classes'.
      _classes->GetMLClassPtr (className);

      numOfFields = Max (numOfFields, numFieldsThisLine);
      _estSize++;
    }
    GetLine (_in, ln, eof);
  }

  bool  alreadyExists = false;
  kkint32  fieldNum = 0;

  FileDescPtr  fileDesc = new FileDesc ();

  for  (fieldNum = 0;  fieldNum < numOfFields;  fieldNum++)
  {
    fileDesc->AddAAttribute ("Field_" + StrFormatInt (fieldNum, "ZZZZ0"), AttributeType::NumericAttribute, alreadyExists);
  }

  return  fileDesc;
}  /* GetFileDesc */






FeatureVectorListPtr  FeatureFileIOUCI::LoadFile (const KKStr&       _fileName,
                                                  const FileDescPtr  _fileDesc,
                                                  MLClassList&       _classes, 
                                                  istream&           _in,
                                                  kkint32            _maxCount,    // Maximum # images to load.
                                                  VolConstBool&      _cancelFlag,
                                                  bool&              _changesMade,
                                                  KKStr&             _errorMessage,
                                                  RunLog&            _log
                                                 )
{
  _log.Level (20) << "FeatureFileIOUCI::LoadFile   FileName[" << _fileName << "]" << endl;


  KKStr  rootName = osGetRootName (_fileName);


  kkint32  numOfFeatures = _fileDesc->NumOfFields ();
  kkint32  lineCount = 0;

  KKStr  ln (256);
  bool  eof;

  FeatureVectorListPtr  examples = new FeatureVectorList (_fileDesc, true, _log);

  GetLine (_in, ln, eof);
  while  (!eof)
  {
    ln.TrimLeft ();
    ln.TrimRight ();

    if  ((ln.SubStrPart (0, 1) != "//")  &&  (!ln.Empty ()))
    {

      kkint32  featureNum = 0;
      FeatureVectorPtr  example = new FeatureVector (numOfFeatures);
  
      for  (featureNum = 0;  featureNum < numOfFeatures;  featureNum++)
      {
        KKStr  featureStr = ln.ExtractToken (" ,\n\r\t");
        example->AddFeatureData (featureNum, (float)atof (featureStr.Str ()));
      }

      KKStr  className = ln.ExtractToken (" ,\n\r\t");
      MLClassPtr mlClass = _classes.GetMLClassPtr (className);
      example->MLClass (mlClass);

      KKStr  imageFileName = rootName + "_" + StrFormatInt (lineCount, "ZZZZZZ0");
      example->ExampleFileName (imageFileName);

      examples->PushOnBack (example);

      lineCount++;
    }
    GetLine (_in, ln, eof);
  }

  return  examples;
}  /* LoadFile */






void   FeatureFileIOUCI::SaveFile (FeatureVectorList&    _data,
                                   const KKStr&          _fileName,
                                   FeatureNumListConst&  _selFeatures,
                                   ostream&              _out,
                                   kkuint32&             _numExamplesWritten,
                                   VolConstBool&         _cancelFlag,
                                   bool&                 _successful,
                                   KKStr&                 _errorMessage,
                                   RunLog&               _log
                                  )

{
  FeatureVectorPtr   example = NULL;

  _numExamplesWritten = 0;

  kkint32  idx;
  kkint32  x;

  FileDescPtr  fileDesc = _data.FileDesc ();

  _out << "ExampleFileName";
  for  (x = 0; x < _selFeatures.NumOfFeatures (); x++)
  {
    kkint32 featureNum = _selFeatures[x];
    _out << "," << fileDesc->FieldName (featureNum);
  }
  _out << "," << "ClassLabel" << endl;

  for  (idx = 0; idx < _data.QueueSize (); idx++)
  {
    example = _data.IdxToPtr (idx);

    _out << ("Train_" + KKB::osGetRootName (example->ExampleFileName ())) << ",";

    for  (x = 0; x < _selFeatures.NumOfFeatures (); x++)
    {
      kkint32  featureNum = _selFeatures[x];
      _out << example->FeatureData (featureNum) << ",";
    }
    _out << example->ClassName ();
    _out << endl;
    _numExamplesWritten++;
  }

  _successful = true;
  return;
}  /* WriteUCIFile */


