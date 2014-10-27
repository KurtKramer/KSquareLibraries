#include  "FirstIncludes.h"

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <string.h>

#include  "MemoryDebug.h"

using namespace std;


#include "KKBaseTypes.h"
#include "DateTime.h"
#include "ImageIO.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "FeatureFileIOKK.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "PostLarvaeFV.h"
using namespace KKMachineLearning;


FeatureFileIOKK  FeatureFileIOKK::driver;





FeatureFileIOKK::FeatureFileIOKK ():
   FeatureFileIO ("KK",
                  true,     // true = _canRead,
                  true      // true = _canWrite,
                 )                
{
}



FeatureFileIOKK::~FeatureFileIOKK ()
{
}


void  FeatureFileIOKK::Parse_FEATURE_DATA_FILE_Line (KKStr&   line,
                                                     kkint32&  version,
                                                     kkint32&  numOfFields,
                                                     kkint32&  numOfExamples
                                                    )
{
  KKStr  field = line.ExtractToken (" ,\t\n\r");  // Skip past FEATURE_DATA_FILE field.
  KKStr  fieldValue;
  field = line.ExtractToken (" ,\t\n\r");
  while  (!field.Empty ())
  {
    fieldValue = line.ExtractToken (" ,\t\n\r");

    if  (field.EqualIgnoreCase ("Version"))
    {
      version = atoi (fieldValue.Str ());
    }

    else if  (field.EqualIgnoreCase ("NumOfFeatures"))
    {
      numOfFields = atoi (fieldValue.Str ());
    }

    else if  (field.EqualIgnoreCase ("NumOfExamples"))
    {
      numOfExamples = atoi (fieldValue.Str ());
    }

    field = line.ExtractToken (" ,\t\n\r").ToUpper ();
  }
}  /* Parse_FEATURE_DATA_FILE_Line */




VectorInt  FeatureFileIOKK::CreateIndirectionTable (const VectorKKStr&  fields,
                                                    kkint32             numOfFeatures
                                                   )
{
  VectorInt   indirectionTable;
  kkint32  fieldNum = 0;

  for  (fieldNum = 0;  fieldNum < numOfFeatures;  fieldNum++)
    indirectionTable.push_back (rfFeatureData);

  for  (fieldNum = numOfFeatures;  fieldNum < (kkint32)fields.size ();  fieldNum++)
  {
    KKStr  fieldName = fields[fieldNum].ToLower ();
    if  (fieldName == "classname")
      indirectionTable.push_back (rfClassName);

    else if  (fieldName == "classnameidx")
      indirectionTable.push_back (rfImageClassIDX);

    else if  (fieldName == "filename") 
      indirectionTable.push_back (rfImageFileName);

    else if  (fieldName == "origsize") 
      indirectionTable.push_back (rfOrigSize);

    else if  (fieldName == "edgepixels") 
      indirectionTable.push_back (rfNumOfEdgePixels);

    else if  (fieldName == "probability") 
      indirectionTable.push_back (rfProbability);

    else if  (fieldName == "disttoborder") 
      indirectionTable.push_back (rfUnKnown);

    else if  (fieldName == "centroidrow")
      indirectionTable.push_back (rfCentroidRow);

    else if  (fieldName == "centroidcol")
      indirectionTable.push_back (rfCentroidCol);

    else if  (fieldName == "predictedclass")
      indirectionTable.push_back (rfPredictedClass);

    else if  (fieldName == "predictedclassidx")
      indirectionTable.push_back (rfPredictedClassIDX);

    else if  (fieldName == "breaktie")
      indirectionTable.push_back (rfBreakTie);

    else
      indirectionTable.push_back (rfUnKnown);

  }

  return  indirectionTable;
}  /* CreateIndirectionTable */






FileDescPtr  FeatureFileIOKK::GetFileDesc (const KKStr&    _fileName,
                                           istream&        _in,
                                           MLClassListPtr  _classes,
                                           kkint32&        _estSize,
                                           KKStr&          _errorMessage,
                                           RunLog&         _log
                                          )
{
  char  buff[102400];

  _errorMessage = "";
  
  _in.getline (buff, sizeof (buff) - 1, '\n');
  KKStr  firstLine (buff);
  firstLine.Upper ();

  if  (firstLine.SubStrPart (0, 16) != "FEATURE_DATA_FILE")
  {
    // We are looking at a very old RAW file,  need to scan for number of fields.
    // Lets count the number of fields in each lime,  
    _log.Level (-1) << endl << endl 
                    << "FeatureFileIOKK::GetFileDesc    ***ERROR***    the first line must contain" << endl
                    << "                                                  'FEATURE_DATA_FILE'" << endl
                    << endl;
    _errorMessage = "The first line must contain FEATURE_DATA_FILE.";
    return NULL;
  }


  kkint32  version       = -1;
  kkint32  numOfFeatures = -1;
  kkint32  numOfExamples = -1;
  Parse_FEATURE_DATA_FILE_Line (firstLine, version, numOfFeatures, numOfExamples);
   if  (version < 39)
  {
    _log.Level (-1) << endl << endl 
                    << "FeatureFileIOKK::GetFileDesc    ***ERROR***"  << endl
                    << endl
                    << "                     Version Number[" << version << "]" << endl
                    << "                     Can not load versions prior to 39." << endl
                    << endl;
    _errorMessage << "Version[" << version << "]  is prior to '39'.";
    return NULL;
  }

  _in.getline (buff, sizeof (buff) - 1);
  KKStr  fieldDescLn (buff);
  VectorKKStr  fields = fieldDescLn.Split ("\n\r\t");
  if  (fields.size () < (kkuint32)numOfFeatures)
  {
    _log.Level (-1) << endl << endl << endl
                    << "FeatureFileIOKK::GetFileDesc     **** Error ***"  << endl
                    << endl
                    << "               Num fields[" << (kkuint32)fields.size () << "]  is less than the number specified." << endl
                    << endl
                    << endl;
    _errorMessage << "Num fields[" << (kkuint32)fields.size () << "]  is less than the number specified." << endl;
    return NULL;
  }
  
   
  FileDescPtr  fd = new FileDesc ();
  fd->Version (version);


  bool  alreadyExists;
  kkint32  featureNum = 0;
  for  (featureNum = 0;  featureNum < numOfFeatures;  featureNum++)
  {
    KKStr&  fieldName = fields[featureNum];
    fd->AddAAttribute (fieldName, NumericAttribute, alreadyExists);
    if  (alreadyExists)
    {
      _log.Level (-1) << endl << endl 
                      << "FeatureFileIOKK::GetFileDesc    ***ERROR***"  << endl
                      << endl
                      << "                     Duplicate Attribute Name[" << fieldName << "]" << endl
                      << endl;
      _errorMessage << "Duplicate Attribute Name[" << fieldName << "]";

      // Can not delete an instance of a 'FileDesc' class once it has been created.
      // delete  fd;
      return NULL;
    }
  }

  return  fd;
}  /* GetFileDesc */





PostLarvaeFVListPtr  FeatureFileIOKK::LoadFile (const KKStr&      _fileName,
                                                const FileDescPtr _fileDesc,
                                                MLClassList&      _classes, 
                                                istream&          _in,
                                                kkint32           _maxCount,    // Maximum # images to load,  less than '0'  indicates all.
                                                VolConstBool&     _cancelFlag,
                                                bool&             _changesMade,
                                                KKStr&            _errorMessage,
                                                RunLog&           _log
                                               )
{
  _log.Level (10) << "FeatureFileIOKK::LoadFile   Loading file[" << _fileName << "]." << endl;
  _log.Flush ();


  VectorInt  featureFieldIndexTable;

  char buff[20480];

  _in.getline (buff, sizeof (buff) - 1);
  KKStr  firstLine (buff);

  firstLine.Upper ();

  if  (firstLine.SubStrPart (0, 16) != "FEATURE_DATA_FILE")
  {
    _log.Level (-1) << endl << endl << endl 
                    << "FeatureFileIOKK::LoadFile     **** Error missing 'FEATURE_DATA_FILE' line ****    can not load" << endl
                    << endl;
    _errorMessage = "Error missing 'FEATURE_DATA_FILE' line";
    return  NULL;
  }

  kkint32  version         = -1;
  kkint32  numOfFeatures   = -1;
  kkint32  numOfExamples   = -1;
  Parse_FEATURE_DATA_FILE_Line (firstLine, version, numOfFeatures, numOfExamples);
  if  (version < 38)
  {
    _log.Level (-1) << endl << endl << endl
                   << "FeatureFileIOKK::LoadFile     ***Error***    File version[" << version << "]  prior to 38.   can not load."
                   << endl
                   << endl;
    _errorMessage << "File version[" << version << "]  prior to 38.   can not load.";
    return  NULL;
  }
  

  // The second line will have a list of fields;  we will use ths line to build
  // an indirection table.
  _in.getline (buff, sizeof (buff) - 1);
  KKStr  fieldDescLine (buff);
  VectorKKStr  fields = fieldDescLine.Parse ("\n\r\t");
  if  ((kkint32)fields.size () < numOfFeatures)
  {
    _log.Level (-1) << endl << endl << endl
                   << "FeatureFileIOKK::LoadFile     **** Error ***"  << endl
                   << endl
                   << "               Num fields[" << (kkuint32)fields.size () << "]  is less than the number specified." << endl
                   << endl
                   << endl;
    _errorMessage << "Num fields[" << (kkuint32)fields.size () << "]  is less than the number specified." << endl;
    return  NULL;
  }


  VectorInt   indirectionTable = CreateIndirectionTable (fields, numOfFeatures);


  PostLarvaeFVListPtr examples = new PostLarvaeFVList (_fileDesc, 
                                                       true,          // true=Owner  examples will own the PostLarvaeFV instances it contains.
                                                       _log
                                                      );

  examples->Version (version);

  kkint32  fieldNum = 0;

  bool  eof = false;
  bool  eol = false;

  KKStr  className;
  char   firstChar;
  kkint32  lineCount = 0;

  float  value;

  kkint32  lineNum = 0;

  eof = false;
  eol = false;

  KKStr field (128);

  GetToken (_in, ",\t", field, eof, eol);
  while  ((!eof)   &&  (!_cancelFlag)   &&  (((kkint32)examples->size () < _maxCount)  ||  (_maxCount < 0)))
  {
    if  (eol)
    {
      // We must have a blank line.  There is nothing to do in this case.
    }

    else if  (field.SubStrPart (0, 1) == "//")
    {
      // We are looking at a comment line.  Will skip to end of line
      while  ((!eol)  &&  (!eof))
        GetToken (_in, ",\t", field, eof, eol);
    }
    else
    {
      // We have an example.
      fieldNum  = 0;
      PostLarvaeFVPtr  example = new PostLarvaeFV (numOfFeatures);
      example->Version (version);

      while  ((!eol)  &&  (!eof))
      {
        field.TrimRight ();

        if  (fieldNum >= (kkint32)indirectionTable.size ())
        {
          _log.Level (-1) << endl << endl << endl
                          << "FeatureFileIOKK::LoadFile     **** Error ***"  << endl
                          << endl
                          << "               LineNum[" << lineNum << "]  has to many fields defined." << endl
                          << endl
                          << endl;
          _errorMessage << "LineNum[" << lineNum << "]  has to many fields defined.";
          delete  examples;
          delete  example;
          return NULL;
        }
      
        switch  (indirectionTable[fieldNum])
        {
        case rfFeatureData:
          firstChar = field.FirstChar ();
          if  (((firstChar < '0')  ||  (firstChar > '9'))  &&  (firstChar != '-'))
          {
            KKStr  temp = field.ToUpper ();
            if  ((temp == "INFINITY")  || (temp == "NAN"))
            {
              example->AddFeatureData (fieldNum,  KKB::FloatMax);
            }
            else
            {
              _log.Level (-1) << endl << endl << endl
                              << "FeatureFileIOKK::LoadFile     **** Error ***"  << endl
                              << endl
                              << "               Invalid Feature Field[" << fieldNum << "]  on LineNum[" << lineNum << "]." << endl
                              << endl
                              << endl;
              _errorMessage << "Invalid Feature Field[" << fieldNum << "]  on LineNum[" << lineNum << "].";
              delete  examples;
              delete  example;
              return NULL;
            }
          }

          else
          {
            value = (float)atof (field.Str ());
            example->AddFeatureData (fieldNum,  value);
          }
          break;


        case rfClassName: 
          className = field;
          if  (className.Empty ())
             className = "UnKnown";
          example->MLClass (_classes.GetMLClassPtr (className));
          break;

        case rfImageClassIDX:
          // Ignore this field.
          break;

        case rfImageFileName: 
          example->ImageFileName (field);
          break;

        case rfOrigSize: 
          example->OrigSize ((float)atof (field.Str ()));
          break;

        case rfNumOfEdgePixels: 
          example->NumOfEdgePixels (atoi (field.Str ()));
          break;

        case rfProbability: 
          example->Probability ((float)atof (field.Str ()));
          break;

        case rfCentroidRow: 
          example->CentroidRow((float)atof (field.Str ()));
          break;

        case rfCentroidCol: 
          example->CentroidCol ((float)atof (field.Str ()));
          break;

        case rfPredictedClass: 
          if  (!field.Empty ())
          example->PredictedClass (_classes.GetMLClassPtr (field));
          break;
  
        case rfPredictedClassIDX:
          // We ignore this field
          break;

        case rfBreakTie: 
          example->BreakTie  ((float)atof (field.Str ()));
          break;

        case rfUnKnown:
          // ignore this field.
          break;
                  
        default:
          _log.Level(-1) << endl << endl << endl
                         << "FeatureFileIOKK::LoadFile   *** Undefined File Format Specified." << endl
                         << endl;
          break;
        }

        GetToken (_in, ",\t", field, eof, eol);
        fieldNum++;
      }

      if  (example->OrigSize () <= 0)
        example->OrigSize (example->FeatureData (0));

      if  (example->ImageFileName ().Empty ())
      {
        KKStr  imageFileName = _fileName + "_" + StrFormatInt (lineCount, "ZZZZZZ0");
        example->ImageFileName (imageFileName);
      }

      if  (example->ImageFileName ().Empty ())
        example->ImageFileName (osGetRootName (_fileName) + "_" + StrFormatInt (lineCount, "000000"));


      if  (example->PredictedClass () == NULL)
        example->PredictedClass (example->MLClass ());

      examples->PushOnBack (example);

      lineNum++;
    }

    GetToken (_in, ",\t", field, eof, eol);
  }

  _log.Level (10) << "LoadFile - Loaded [" << (kkint32)examples->size () << "] images from file[" << _fileName << "]." << endl;

  return  examples;
}  /* LoadFile */




/**
 * @brief  Used to quickly locate Index in MLClassList object; needed by RawFile format.
 * @param[out] classIdx   Map template, from MLClass to Index in a MLClassList object.
 * @param[out] mlClass  Image class that the Index of is needed for.
 * @return[out] Index indicated by 'classIdx' that mlClass is at.
*/
kkint32  GetClassIdx (const map<MLClassPtr, kkint32>&  classIdx,
                      const MLClassPtr                 mlClass
                     )
{
  if  (!mlClass)
    return -1;

  map<MLClassPtr, kkint32>::const_iterator p;
  p = classIdx.find (mlClass);
  if (p == classIdx.end ())
    return -1;
  return p->second;
}  /* GetClassIdx */




void   FeatureFileIOKK::SaveFile (FeatureVectorList&     _data,
                                  const KKStr&           _fileName,
                                  const FeatureNumList&  _selFeatures,
                                  ostream&               _out,
                                  kkuint32&              _numExamplesWritten,
                                  VolConstBool&          _cancelFlag,
                                  bool&                  _successful,
                                  KKStr&                 _errorMessage,
                                  RunLog&                _log
                                 )
{
  PostLarvaeFVListPtr  examples  = NULL;

  _numExamplesWritten = 0;

  bool  weOwnImages = false;

  //if  (strcmp (_data.UnderlyingClass (), "PostLarvaeFV") == 0)
  if  (typeid (_data) == typeid (PostLarvaeFV))
  {
    examples = dynamic_cast<PostLarvaeFVListPtr>(&_data);
  }
  else
  {
    weOwnImages = true;
    examples = new PostLarvaeFVList (_data, true);
  }

  const FileDescPtr  fileDesc = _data.FileDesc ();

  ClassStatisticListPtr  classStatistics = examples->GetClassStatistics ();

  kkint32  fileVersionNum = examples->Version ();
  if  (fileVersionNum < 1)
  {
    fileVersionNum = CurrentFeatureFileVersionNum;
    examples->Version (fileVersionNum);
  }

  map<MLClassPtr, kkint32>  classIdx;

  _out << "FEATURE_DATA_FILE"                                                << "\t"  
       << "Version"        << "\t" << StrFormatInt (fileVersionNum, "ZZZ0")  << "\t" 
       << "NumOfFeatures"  << "\t" << _selFeatures.NumOfFeatures ()          << "\t" 
       << "NumOfExamples"  << "\t" << examples->QueueSize ()
       << endl;

  {
    kkint32  idx;
    for  (idx = 0;  idx < _selFeatures.NumOfFeatures ();  idx++)
    { 
      kkint32  featureNum = _selFeatures[idx];
      _out << fileDesc->FieldName (featureNum) << "\t";
    }
 
    _out << "ClassName"         << "\t"
         << "ClassNameIdx"      << "\t"
         << "FileName"          << "\t"
         << "OrigSize"          << "\t"
         << "EdgePixels"        << "\t"
         << "Probability"       << "\t"
         << "CentroidRow"       << "\t"
         << "CentroidCol"       << "\t"
         << "PredictedClass"    << "\t"
         << "PredictedClassIdx" << "\t"
         << "BreakTie"
         << endl;
  }

  {
    // Write out Class Statistics
    kkint32  seqNum = 0;
    ClassStatisticList::iterator  idx;
    for  (idx = classStatistics->begin ();  idx != classStatistics->end ();  idx++)
    {
      ClassStatisticPtr  cs = *idx;
      _out << "//" << cs->Name () << "\t" 
           <<         seqNum      << "\t"
           <<         cs->Count ()
           << endl;

      classIdx.insert (pair<MLClassPtr, kkint32> (cs->MLClass (), seqNum));
      seqNum++;
    }
  }

  {
    // Write out the actual examples.
    PostLarvaeFVPtr   example = NULL;
    for  (kkint32 idx = 0; idx < (kkint32)examples->size (); idx++)
    {
      example = examples->IdxToPtr (idx);

      kkint32  x;  
      for  (x = 0; x < _selFeatures.NumOfFeatures (); x++)
      {
        kkint32  featureNum = _selFeatures[x];
        _out << example->FeatureData (featureNum) << "\t";
      }

      {
        kkint32  imageClassIdx = GetClassIdx (classIdx, example->MLClass ());
        kkint32  predClassIdx  = GetClassIdx (classIdx, example->PredictedClass ());
        kkint32  p = _out.precision ();
        _out.precision (11);
        _out << example->ClassName          () << "\t" 
             << imageClassIdx                  << "\t"
             << example->ImageFileName      () << "\t" 
             << example->OrigSize           () << "\t"
             << example->NumOfEdgePixels    () << "\t"
             << example->Probability        () << "\t"
             << example->CentroidRow        () << "\t"
             << example->CentroidCol        () << "\t"
             << example->PredictedClassName () << "\t"
             << predClassIdx                   << "\t"
             << example->BreakTie           ()
             << endl;
        _out.precision (p);
      }
      _numExamplesWritten++;
    }
  }

  delete  classStatistics;
  if  (weOwnImages)
  {
    delete  examples;
    examples = NULL;
  }

  _successful = true;
  return;

}  /* SaveFile */





PostLarvaeFVListPtr  FeatureFileIOKK::LoadInSubDirectoryTree 
                                             (KKStr          _rootDir,
                                              MLClassList&   _mlClasses,
                                              bool           _useDirectoryNameForClassName,
                                              VolConstBool&  _cancelFlag, 
                                              bool           _rewiteRootFeatureFile,
                                              RunLog&        _log
                                             )
{
  _log.Level (10) << "FeatureFileIOKK::LoadInSubDirectoryTree    rootDir[" << _rootDir << "]." << endl;

  osAddLastSlash (_rootDir);

  KKStr  featureFileName ("");
  KKStr  fullFeatureFileName ("");

  if  (!_rootDir.Empty ())
  {
    featureFileName = osGetRootNameOfDirectory (_rootDir) + ".data";
    fullFeatureFileName = _rootDir + featureFileName;
  }
  else
  {
    featureFileName     = "Root.data";
    fullFeatureFileName = "Root.data";
  }

  MLClassPtr  unKnownClass = _mlClasses.GetUnKnownClass ();
  if  (_useDirectoryNameForClassName)
  {
    KKStr className = MLClass::GetClassNameFromDirName (_rootDir);
    unKnownClass    = _mlClasses.GetMLClassPtr (className);
  }

  bool  changesMade = false;

  PostLarvaeFVListPtr  dirImages = NULL;

  FileDescPtr  fileDesc = PostLarvaeFV::PostLarvaeFeaturesFileDesc ();

  if  (_rewiteRootFeatureFile)
  {
    DateTime  timeStamp;
    dirImages = FeatureDataReSink (_rootDir,
                                   featureFileName,
                                   unKnownClass,
                                   _useDirectoryNameForClassName,
                                   _mlClasses,
                                   _cancelFlag,
                                   changesMade,
                                   timeStamp,
                                   _log
                                  );
    if  (_useDirectoryNameForClassName)
    {
      PostLarvaeFVList::iterator  idx;
      for  (idx = dirImages->begin ();  idx != dirImages->end ();  idx++)
      {
        if  ((*idx)->MLClass () != unKnownClass)
        {
          (*idx)->MLClass (unKnownClass);
          changesMade = true;
        }
      }

      if  (changesMade)
      {
        KKStr  fullFileName = osAddSlash (_rootDir) + featureFileName;
        kkuint32  numExamplesWritten = 0;
        bool  cancel     = false;
        bool  successful = false;
        driver.SaveFeatureFile (fullFileName, 
                                dirImages->AllFeatures (), 
                                *dirImages, 
                                numExamplesWritten,
                                cancel,
                                successful,
                                _log
                              );
      }
    }
  }
  else
  {
	  dirImages =  new PostLarvaeFVList (fileDesc, true, _log);
  }


  // Now that we have processed all image files in "rootDir",
  // lets process any sub-directories.

  KKStr  dirSearchPath = osAddSlash (_rootDir) + "*.*";

  KKStrListPtr  subDirectories = osGetListOfDirectories (dirSearchPath);
  if  (subDirectories)
  {
    KKStrList::iterator  idx;

    for  (idx = subDirectories->begin ();  (idx != subDirectories->end ()  &&  (!_cancelFlag));   idx++)
    {
      KKStr  subDirName (**idx);
      if  (subDirName == "BorderImages")
      {
        // We ignore this director
        continue;
      }

      KKStr  newDirPath = osAddSlash (_rootDir) + subDirName;

      PostLarvaeFVListPtr  subDirImages = LoadInSubDirectoryTree (newDirPath, 
                                                                  _mlClasses, 
                                                                  _useDirectoryNameForClassName, 
                                                                  _cancelFlag,
                                                                  true,     // true = ReWriteRootFeatureFile
                                                                  _log
                                                                 );
      PostLarvaeFVPtr  image = NULL;

      osAddLastSlash (subDirName);

      // We want to add the directory path to the ImageFileName so that we can later locate the source image.
      PostLarvaeFVList::iterator  idx = subDirImages->begin ();
      for  (idx = subDirImages->begin ();  idx != subDirImages->end ();  idx++)
      {
        image = *idx;
        KKStr  newImageFileName = subDirName + image->ImageFileName ();
        image->ImageFileName (newImageFileName);
      }

      dirImages->AddQueue (*subDirImages);
      subDirImages->Owner (false);
      delete  subDirImages;
    }

    delete  subDirectories;  subDirectories = NULL;
  }

  _log.Level (10) << "LoadInSubDirectoryTree - Done" << endl;

  return  dirImages;
}  /* LoadInSubDirectoryTree */






PostLarvaeFVListPtr  FeatureFileIOKK::FeatureDataReSink 
                                        (KKStr          _dirName, 
                                         const KKStr&   _fileName, 
                                         MLClassPtr     _unknownClass,
                                         bool           _useDirectoryNameForClassName,
                                         MLClassList&   _mlClasses,
                                         VolConstBool&  _cancelFlag,    // will be monitored,  if set to True  Load will terminate.
                                         bool&          _changesMade,
                                         DateTime&      _timeStamp,
                                         RunLog&        _log
                                        )
{
  _changesMade = false;
  _timeStamp = DateTime ();

  FileDescPtr  fileDesc = PostLarvaeFV::PostLarvaeFeaturesFileDesc () ;
  if  (_unknownClass == NULL)
    _unknownClass = MLClass::GetUnKnownClassStatic ();

  bool  logImagesGettingFeaturesCalced = false;
  if  (_unknownClass != NULL)
  {
    if  (_unknownClass->Name ().EqualIgnoreCase ("detritus_blob"))
      logImagesGettingFeaturesCalced = true;
  }

  KKStr  className = _unknownClass->Name ();

  _log.Level (10) << "FeatureDataReSink, dirName      [" << _dirName   << "]." << endl;
  _log.Level (10) << "                   fileName     [" << _fileName  << "]." << endl;
  _log.Level (10) << "                   UnKnownClass [" << className  << "]." << endl;

  osAddLastSlash (_dirName);
  
  KKStr  fullFeatureFileName = osAddSlash (_dirName) +  _fileName;

  bool  successful;

  KKStr fileNameToOpen;
  if  (_dirName.Empty ())
    fileNameToOpen = _fileName;
  else
    fileNameToOpen = osAddSlash (_dirName) + _fileName;

  bool  versionsAreSame = false;

  FeatureVectorListPtr  origFeatureVectorData 
        = driver.LoadFeatureFile (fileNameToOpen, _mlClasses, -1, _cancelFlag, successful, _changesMade, _log);

  if  (origFeatureVectorData == NULL)
  {
    successful = false;
    origFeatureVectorData = new PostLarvaeFVList (fileDesc, true, _log);
  }


  if  (_cancelFlag)
  {
    delete  origFeatureVectorData;  origFeatureVectorData = NULL;
    return  new PostLarvaeFVList (fileDesc, true, _log);
  }

  PostLarvaeFVListPtr origFeatureData = NULL;

  if  (typeid (*origFeatureVectorData) == typeid (PostLarvaeFVList))
  {
     origFeatureData = dynamic_cast<PostLarvaeFVListPtr>(origFeatureVectorData);
  }
  else
  {
    origFeatureData = new PostLarvaeFVList (*origFeatureVectorData, true);
    delete  origFeatureVectorData;
    origFeatureVectorData = NULL;
  }

  KKStr  fileSpec = osAddSlash (_dirName) + "*.*";
  KKStrListPtr   fileNameList = osGetListOfFiles (fileSpec);

  if  (!fileNameList)
  {
    // There are no Image Files,  so we need to return a Empty List of Image Features.

    if  (origFeatureData->QueueSize () > 0)
      _changesMade = true;

    delete  origFeatureData;  origFeatureData = NULL;

    return  new PostLarvaeFVList (fileDesc, true, _log);
  }

  if  (successful)
  {
    if  (origFeatureData->Version () == CurrentFeatureFileVersionNum)
    {
      versionsAreSame = true;
      _timeStamp = osGetFileDateTime (fileNameToOpen);
    }

    else
    {
      _changesMade = true;
    }
  }
  else
  {
    delete  origFeatureData;
    origFeatureData = new PostLarvaeFVList (fileDesc, true, _log);
  }

  origFeatureData->SortByRootName (false);


  PostLarvaeFVListPtr  extractedFeatures = new PostLarvaeFVList (fileDesc, true, _log);
  extractedFeatures->Version (CurrentFeatureFileVersionNum);

  fileNameList->Sort (false);

  KKStrList::iterator  fnIDX;
  fnIDX = fileNameList->begin ();   // fileNameList

  KKStrPtr  imageFileName;

  kkint32  numImagesFoundInOrigFeatureData = 0;
  kkint32  numOfNewFeatureExtractions = 0;

  for  (fnIDX = fileNameList->begin ();  (fnIDX != fileNameList->end ())  &&  (!_cancelFlag);  ++fnIDX)
  {
    imageFileName = *fnIDX;
    bool validImageFileFormat = SupportedImageFileFormat (*imageFileName);
    
    if  (!validImageFileFormat)
      continue;

    PostLarvaeFVPtr  origImage = origFeatureData->BinarySearchByName (*imageFileName);
    if  (origImage)
      numImagesFoundInOrigFeatureData++;

    if  (origImage  &&  versionsAreSame)
    {
      if  (_useDirectoryNameForClassName)
      {
        if  (origImage->MLClass () != _unknownClass)
        {
          _changesMade = true;
          origImage->MLClass (_unknownClass);
        }
      }

      else if  ((origImage->MLClass ()->UnDefined ())  &&  (origImage->MLClass () != _unknownClass))
      {
        _changesMade = true;
        origImage->MLClass (_unknownClass);
      }

      extractedFeatures->PushOnBack (origImage);
      origFeatureData->DeleteEntry (origImage);
    }
    else
    {
      // We either  DON'T have an original image    or    versions are not the same.

      KKStr  fullFileName = _dirName + (*imageFileName);
      PostLarvaeFVPtr image = NULL;
      try
      {
        if  (logImagesGettingFeaturesCalced)
          _log.Level (10) << "FeatureDataReSink " << imageFileName << endl;
        image = new PostLarvaeFV (fullFileName, _unknownClass, successful, NULL);
        if  (logImagesGettingFeaturesCalced)
          _log.Level (10) << "FeatureDataReSink " << imageFileName << "  Computed." << endl;
      }
      catch  (...)
      {
        _log.Level (-1) << endl << endl
          << "FeatureDataReSink   ***ERROR***"  << endl
          << "       Exception occured calling constructor 'PostLarvaeFV'  trying to compute FeatureVector." << endl
          << endl;
        successful = false;
        image = NULL;
      }

      if  (!successful)
      {
        _log.Level (-1) << " FeatureFileIOKK::FeatureDataReSink  *** ERROR ***, Processing Image File["
                       << imageFileName << "]."
                       << endl;
        delete  image;
        image = NULL;
      }

      else
      {
        _changesMade = true;
        image->ImageFileName (*imageFileName);

        _log.Level (30) << image->ImageFileName () << "  " << image->OrigSize () << endl;
        extractedFeatures->PushOnBack (image);
        numOfNewFeatureExtractions++;

        if  ((numOfNewFeatureExtractions % 100) == 0)
          cout << numOfNewFeatureExtractions << " Images Extracted." << endl;
      }
    }
  }

  if  (numImagesFoundInOrigFeatureData != extractedFeatures->QueueSize ())
    _changesMade = true;
  
  extractedFeatures->Version (CurrentFeatureFileVersionNum);

  if  ((_changesMade)  &&  (!_cancelFlag))
  {
    //extractedFeatures->WriteImageFeaturesToFile (fullFeatureFileName, RawFormat, FeatureNumList::AllFeatures (extractedFeatures->FileDesc ()));

    kkuint32  numExamplesWritten = 0;

    driver.SaveFeatureFile (fullFeatureFileName,  
                            FeatureNumList::AllFeatures (extractedFeatures->FileDesc ()),
                            *extractedFeatures,
                            numExamplesWritten,
                            _cancelFlag,
                            successful,
                            _log
                           );

    _timeStamp = osGetLocalDateTime ();
  }

  delete  fileNameList;        fileNameList    = NULL;
  delete  origFeatureData;     origFeatureData = NULL;

  _log.Level (10) << "FeatureDataReSink  Exiting     Dir["  << _dirName << "]." << endl;

  return  extractedFeatures;
}  /* FeatureDataReSink */


