#include  "FirstIncludes.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
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

#include "FeatureFileIOC45.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;



FeatureFileIOC45   FeatureFileIOC45::driver;


FeatureFileIOC45::FeatureFileIOC45 ():
  FeatureFileIO ("C45", true, true)
{
}



FeatureFileIOC45::~FeatureFileIOC45()
{
}



FeatureVectorListPtr  FeatureFileIOC45::LoadFeatureFile 
                                      (const KKStr&   _fileName,
                                       MLClassList&   _mlClasses,
                                       kkint32        _maxCount,
                                       VolConstBool&  _cancelFlag,    /**< will be monitored,  if set to True  Load will terminate. */
                                       bool&          _successful,
                                       bool&          _changesMade,
                                       RunLog&        _log
                                      )
{
  _log.Level (10) << "FeatureFileIOC45::LoadFeatureFile  File[" << _fileName << "]  FileFormat[" << DriverName () << "]" << endl;
  
  if  (_maxCount < 0)
    _maxCount = int32_max;

  KKStr  namesFileName;
  KKStr  dataFileName;

  {
    // First determine the name of the names and data file.
    auto lastDotPos = _fileName.LocateLastOccurrence ('.');
    if  (!lastDotPos)
    {
      namesFileName = _fileName + ".names";
      dataFileName  = _fileName;
      if  (!osFileExists (dataFileName))
        dataFileName  = _fileName + ".data";
    }
    else
    {
      KKStr leadingPart = _fileName.SubStrPart (0, lastDotPos.value () - 1);
      KKStr extension   = _fileName.SubStrPart (lastDotPos.value () + 1);
      extension.Upper ();
      if  (extension == "NAMES")
      {
        // The file name was already the names file, can use as is
        namesFileName = _fileName;
        dataFileName  = leadingPart;
        if  (!osFileExists (dataFileName))
        {
          dataFileName  = leadingPart + ".data";
          if  (!osFileExists (dataFileName))
            dataFileName  = leadingPart + ".test";
        }
      }

      else if  ((extension == "DATA") ||  (extension == "TEST"))
      {
        dataFileName  = _fileName;
        namesFileName = leadingPart + ".names";
        if  (!osFileExists (namesFileName))
        {
          namesFileName = leadingPart + ".names";
          if  (!osFileExists (namesFileName))
          {
            namesFileName = _fileName + ".names";
            if  (!osFileExists (namesFileName))
              namesFileName = leadingPart + ".names";
          }
        }
      }

      else
      {
        dataFileName  = _fileName;
        namesFileName = _fileName + ".names";
      }
    }
  }

  _changesMade = false;

  kkint32  estimatedNumOfDataItems = -1;
  
  _successful = true;

  ifstream  namesFile (namesFileName.Str (), ios_base::in);
  if  (!namesFile.is_open ())
  {
    _log.Level (-1) << "FeatureFileIOC45::LoadFeatureFile   ***ERROR***      Error Opening File[" << dataFileName << "]." << endl;
    _successful = false;
    return  NULL;
  }

  KKStr  errorMessage;
  
  FileDescConstPtr fileDesc = GetFileDesc (namesFileName, namesFile, &_mlClasses, estimatedNumOfDataItems, errorMessage, _log);
  if  (fileDesc == NULL)
  {
    _log.Level (-1) << endl << endl 
                   << "FeatureFileIOC45::LoadFeatureFile    ***ERROR***     Loading Feature File[" << namesFileName << "]" << endl
                   << endl;
    _successful = false;
    return NULL;
  }

  namesFile.close ();
  fileDesc = FileDesc::GetExistingFileDesc (fileDesc);



  ifstream  dataFile (dataFileName.Str (), ios_base::in);
  if  (!dataFile.is_open ())
  {
    _log.Level (-1) << "FeatureFileIOC45::LoadFeatureFile   ***ERROR***      Error Opening File[" << dataFileName << "]." << endl;
    _successful = false;
    return  NULL;
  }


  FeatureVectorListPtr  examples = LoadFile (dataFileName, fileDesc, _mlClasses, dataFile, _maxCount, _cancelFlag, _changesMade, errorMessage, _log);
  if  (examples == NULL)
  {
    _successful = false;
  }
  else
  {
    _successful = true;
  }

  dataFile.close ();

  return  examples;
}  /* LoadFeatureFile */



void  FeatureFileIOC45::C45StripComments (KKStr&  ln)
{
  auto  lastBarPos = ln.LocateLastOccurrence ('|');
  if  (lastBarPos)
    ln = ln.SubStrPart (0, lastBarPos.value () - 1);

  ln.TrimLeft  (" \n\r\t");
  ln.TrimRight (" \n\r\t");
}  /* C45StripComments */



//  deal with comments, white spaces, and escape characters.
void  FeatureFileIOC45::C45StrPreProcessName (KKStr&  ln)
{
  ln.TrimRight ();
  ln.TrimLeft ();

  KKStr  newLine (ln.Len ());  

  kkuint32 x = 0;
  char thisChar;
  char nextChar;
  while  (x < ln.Len ())
  {
    thisChar = ln[x];
    nextChar = ln[x + 1];

    if  (strchr (" \t", thisChar))
    {
      // We have a white space character, will want to compress into a single white space character
      while  ((x < ln.Len ())  &&  (strchr (" \t", nextChar)))
      {
        x++;
        thisChar = nextChar;
        nextChar = ln[x + 1];
      }
    }

    else if  (thisChar == '\\')
    {
      // A possible escape sequence
      if  (strchr (",:?", nextChar))
      {
        x++;
        thisChar = nextChar;
        nextChar = ln[x + 1];
      }
    }

    newLine.Append (thisChar);
  
    x++;
  }

  ln = newLine;
}  /* C45StrPreProcessName */



/**
 *@brief  Locates first 'ch' in 'txt' that is  not preceded by an escape character('\\').
 */
kkint32  FeatureFileIOC45::C45LocateNextCharacter (const KKStr&  txt,
                                                   char          ch
                                                  )
{
  kkuint32 x = 0;
  while  (x < txt.Len ())
  {
    if  (txt[x] == ch)
    {
      if  (x == 0)              break;
      if  (txt[x - 1] != '\\')  break;
    }
    x++;
  }

  if  (x < txt.Len ())  return (kkint32)x;  else  return -1;
}  /* C45LocateNextCharacter */



void  FeatureFileIOC45::ProcessC45AttrStr (FileDescPtr  fileDesc,
                                           KKStr&       attrStr,
                                           bool&        validStr,
                                           RunLog&      _log
                                          )
{
  validStr = true;

  attrStr.TrimLeft ();
  attrStr.TrimRight ();
  if  (attrStr.Empty ())
    return;

  kkint32  colPos = C45LocateNextCharacter (attrStr, ':');
  if  (colPos < 0)
  {
    // Missing Attribute Specification
    _log.Level (-1) << endl
                    << "FeatureFileIOC45::ProcessC45AttrStr   ***ERROR***  Missing Attribute Specification (No Colon)." << endl
                    << "     attrStr[" << attrStr << "]" << endl
                    << endl;
    validStr = false;
    return;
  }

  KKStr  name    = attrStr.SubStrPart (0, colPos - 1);
  KKStr  typeStr = attrStr.SubStrPart (colPos + 1);
  C45StrPreProcessName (name);

  if  (name.Empty ())
  {
    _log.Level (-1) << endl
                    << "FeatureFileIOC45::ProcessC45AttrStr     ***ERROR***  Field Name is Empty" << endl
                    << "       AttrStr[" << attrStr << "]" << endl
                    << endl;
    validStr = false;
    return;
  }

  // Lets determine type now
  typeStr.TrimLeft  ();
  typeStr.TrimRight ();
  KKStr  typeStrUpper = typeStr.ToUpper ();

  AttributeType  attributeType = AttributeType::NULLAttribute;

  if  (typeStrUpper == "CONTINUOUS")
  {
    attributeType = AttributeType::Numeric;
  }

  else if  (typeStrUpper == "IGNORE")
  {
    attributeType = AttributeType::Ignore;
  }

  else if  (typeStrUpper == "SYMBOLIC")
  {
    attributeType = AttributeType::Symbolic;
  }

  else
  {
    // We have a nominal field
     attributeType = AttributeType::Nominal;
  }

  bool  alreadyExists = false;
  fileDesc->AddAAttribute (name, attributeType, alreadyExists);
  if  (alreadyExists)
  {
    // Two fields with the same name,   *** VERY BAD ***
    _log.Level (-1) << endl
                    << "FeatureFileIOC45::ProcessC45AttrStr     *** ERROR ***  Field Name Occurs more than once."
                    << "       AttrStr[" << attrStr << "]" << endl
                    << endl;
    validStr = false;
    return;
  }

  if  (attributeType == AttributeType::Nominal)
  {
    // Will now parse out the nominal values.
    while  (!typeStr.Empty ())
    {
      KKStr  nominalValue;

      kkint32 commaPos = C45LocateNextCharacter (typeStr, ',');
      if  (commaPos < 0)
      {
        nominalValue = typeStr;
        typeStr = "";
      }
      else
      {
        nominalValue = typeStr.SubStrPart (0, commaPos - 1);
        typeStr = typeStr.SubStrPart (commaPos + 1);
      }

      C45StrPreProcessName (nominalValue);

      if  (nominalValue.Empty ())
      {
        // Must have some characters in name.
        _log.Level (-1) << endl
                        << "FileDesc::AddANominalValue    *** ERROR ***" << endl
                        << "    Blank NominalValue (\"\")"               << endl
                        << "    AttrStr       [" << attrStr      << "]." << endl
                        << endl;
        validStr = false;
        return;
      }

      fileDesc->AddANominalValue (nominalValue, alreadyExists, _log);
      if  (alreadyExists) 
      {
        _log.Level (-1) << endl
                        << "FileDesc::AddANominalValue    *** ERROR ***" << endl
                        << "    Nominal Value [" << nominalValue << "] occurs more than once." << endl
                        << "    AttrStr       [" << attrStr      << "]."  << endl
                        << endl;
        validStr = false;
        return;
      }
    }
  }
} /* ProcessC45AttrStr */



FileDescConstPtr  FeatureFileIOC45::GetFileDesc (const KKStr&    _fileName,
                                                 istream&        _in,
                                                 MLClassListPtr  _classes,
                                                 kkint32&        _estSize,
                                                 KKStr&          _errorMessage,
                                                 RunLog&         _log
                                                )
{
  _log.Level (50) << "FeatureFileIOC45::GetFileDesc   FileName: " << _fileName << endl;
  KKStr  namesFileName;
   
  bool  classLineRead = false;

  KKStr  ln (1024);
  bool  eof = false;
  kkint32  lineNum = 0;
  GetLine (_in, ln, eof);  lineNum++;

  while  ((!eof)  &&  (!classLineRead))
  {
    C45StripComments (ln);

    if  (!ln.Empty ())
    {
      // We have our very first line,  this should consist 
      // of all the class names.

      // Eliminate any trailing period.
      if  (ln.LastChar () == '.')
        ln.ChopLastChar ();

      VectorKKStr  classNames = ln.Split ("\n\r\t,");
      for  (kkint32  idx = 0;  idx < (kkint32)classNames.size ();   idx++)
      {
        KKStr  className = classNames[idx];
        C45StrPreProcessName (className);
        // Will add instance of 'className' to '_classes' if it does not already exist.
        _classes->GetMLClassPtr (className);
      }

      classLineRead = true;
    }
    else
    {
      GetLine (_in, ln, eof);  lineNum++;
    }
  }

  if  (!classLineRead)
  {
    _log.Level (-1) << endl
                   << "FeatureFileIOC45::GetFileDesc  *** ERROR ***    No class line in Names File." << endl
                   << endl;
    _errorMessage = "No class line in Names File.";
    return  NULL;
  }

  FileDescPtr  fileDesc = new FileDesc ();
  fileDesc->AddClasses (*_classes);
  _estSize = 0;
  // Can now load in attribute data
  GetLine (_in, ln, eof);  lineNum++;
  while  (!eof)
  {
    C45StripComments (ln);

    // "ln"  may consist of more than one attribute descriptor separated by periods.
    while  (!ln.Empty ())
    {
      bool  validStr = true;

      // Locate a period followed by white space, if not followed by 
      // white space, will be part of a name.
      kkuint32  dotPos = 0;
      while  (dotPos < ln.Len ())
      {
        if  (ln[dotPos] == '.')
        {
          if  (dotPos >= (ln.Len () - 1))
            break;

          else if  (strchr (" \t\r\n", ln[dotPos + 1]))
            break;
        }

        dotPos++;
      }

      KKStr  attrStr;

      if  (dotPos >= ln.Len ())
      {
        attrStr = ln;
        ln = "";
      }
      else
      {
        attrStr = ln.SubStrPart (0, dotPos - 1);
        ln = ln.SubStrPart (dotPos + 1);
      }

      KKStr  origAttrStr (attrStr);
      ProcessC45AttrStr (fileDesc, attrStr, validStr, _log);
      if  (!validStr)
      {
        _log.Level (-1) << endl
                        << "FeatureFileIOC45::GetFileDesc  ***ERROR***    Invalid AttributeStr[" << origAttrStr  << "]." << endl
                        << "                                              LineNum["              << lineNum      << "]" << endl
                        << endl;
        _errorMessage = "No class line in Names File.";
        _errorMessage << "Invalid AttributeStr[" << origAttrStr << "],  LineNum[" << lineNum << "]";

        // Can not delete an instance of a 'FileDesc' class once it has been created.
        // delete  fileDesc;  fileDesc = NULL;
        return  NULL;
      }
    }

    ++_estSize;
    GetLine (_in, ln, eof);  lineNum++;
  }

  return  fileDesc;
}  /* GetFileDesc */



KKStr  FeatureFileIOC45::C45ReadNextToken (istream&     in, 
                                           const char*  delimiters,
                                           bool&        eof,
                                           bool&        eol
                                          )
{
  eof = false;
  eol = false;

  const kkint32  maxTokenLen = 1024;
  char  token[maxTokenLen];

  // lets skip leading white space
  kkint32  ch = in.get (); eof = in.eof ();
  while  ((!eof)  &&  ((ch == ' ') || (ch == '\r') || (ch == '\t'))  &&  (ch != '\n'))
    {ch = in.get (); eof = in.eof ();}

  if  (ch == '\n')
  {
    eol = true;
    if  (in.peek () == '\r')
      in.get ();
    return "";
  }

  if  (ch == '\r')
  {
    eol = true;
    if  (in.peek () == '\n')
      in.get ();
    return "";
  }

  else if  (ch == '.')
  {
    // If next character is a white space, eol, eof, of '|' 
    // then treat as end of entry or '\n'.
    char nextCh = (char)in.peek (); 
    if  (strchr (" \t\r\n|", nextCh))
    {
      eol = true;
      return "";
    }
  }
  
  else if  (ch == '|')
  {
    // The rest of the line is meant to be a comment,  
    // we can skip all the following characters.
    while  ((!eof)  &&  (ch != '\n')  &&  (ch != '\r'))
      {ch = in.get (); eof = in.eof ();}
    eol = true;

    if  (!eof)
    {
      if  ((ch == '\n')  &&  (in.peek () == '\r'))
        in.get ();  //  I want to leave ch with '\n' in it.

      else if  ((ch == '\r')  &&  (in.peek () == '\n'))
        ch = in.get ();
    }
    eof = in.eof ();
  }

  kkint32 tokenLen = 0;

  // Read till first delimiter or eof
  while  ((!eof)  &&  (!strchr (delimiters, ch)))
  {
    if  ((ch == '\n')  ||  (ch == '|'))
    {
      in.putback ((char)ch);
      break;
    }

    else if  (ch == '.')
    {
      // Dots have special meaning when at the end of the line or followed 
      // by a white space character.  In these cases they delimit a separated entry.
      char nextCh = (char)in.get (); bool nextEOF = in.eof ();
      if  (nextEOF)
      {
        // Since end of file will treat as end of line
        in.putback (nextCh);
        nextCh = (char)in.get (); nextEOF = in.eof ();
        ch = ' ';
        break;
      }
      else
      {
        if  (strchr (" \r\t", nextCh))
        {
          // Since next character is end of line, or white space them this period is 
          // marking end of entry, will put back into stream so that next call to 
          // C45ReadNextToken will get it next and treat as end of line.
          in.putback ('.');
          break;
        }
        else if  ((nextCh == '\n')  ||  (nextCh == '|'))
        {
          // This is a period that is at end of line, in this case
          // we discard '.' and end token.
          in.putback (nextCh);
          break;
        }
        else
        {
          // This is a valid period, dot '.' character., so lets put the following
          // character back in the stream
          in.putback (nextCh);
        }
      }
    }

    else
    {
      if  (ch == '\\')
      {
        // We may have an escape character that c45 Allows
        // http://www.cs.washington.edu/dm/vfml/appendixes/c45.htm

        char nextCh = (char)in.get (); bool nextEOF = in.eof ();
        if  (nextEOF)
        {
          // Not Sure what will happen in this case,  i don't want to set eof yet,
          // but do on the next call, so will put character back on line and re-read
          // this way the next call to this function will get a eof.
          in.putback (nextCh);
          nextCh = (char)in.get (); nextEOF = in.eof ();
        }
        else
        {
          if  (strchr (",?:", nextCh))
          {
            ch = nextCh;
          }
          else
          {
            // not a Escape Sequence, put following character back so we get it next time.
            in.putback (nextCh);
          }
        }
      } 

      else if  (strchr (" \t\r", ch))
      {
        // We will compress the white space characters to just one blank.
        ch = ' ';

        char nextCh = (char)in.get (); bool nextEOF = in.eof ();
        while  ((!nextEOF)  &&  (strchr (" \t\r", nextCh)))
        {nextCh = (char)in.get (); nextEOF = in.eof ();}

        if  (nextEOF)
        {
          // Not Sure what will happen in this case,  i don't want to set eof yet,
          // but do on the next call, so will put character back on line and reread
          // this way the next call to this function will get a eof.
          in.putback (nextCh);
          nextCh = (char)in.get ();  nextEOF = in.eof ();
        }
        else
        {
          // This should be a valid characte, so will put back into file
          // to read on next loop
          in.putback (nextCh);
        }
      }
    }

    token[tokenLen] = (char)ch;
    tokenLen++;
    ch = in.get (); eof = in.eof ();
  }

  token[tokenLen] = 0;  // Terminating NULL character.
  
  // Remove Training whitespace
  while  (tokenLen > 0)
  {
    if  (strchr (" \r\t", token[tokenLen - 1]) == 0)
      break;
    tokenLen--;
    token[tokenLen] = 0;
  }


  return  token;
}  /* C45ReadNextToken */



FeatureVectorListPtr  FeatureFileIOC45::LoadFile (const KKStr&      _fileName,
                                                  FileDescConstPtr  _fileDesc,
                                                  MLClassList&      _classes, 
                                                  istream&          _in,
                                                  OptionUInt32      _maxCount,    // Maximum # images to load.
                                                  VolConstBool&     _cancelFlag,
                                                  bool&             _changesMade,
                                                  KKStr&            _errorMessage,
                                                  RunLog&           _log
                                                )
{
  _log.Level (10) << "FeatureFileIOC45::LoadFile   FileName[" << _fileName << "]" << endl;

  bool  eof = false;
  bool  eol = false;

  kkint32  numOfFeatures = _fileDesc->NumOfFields ();

  KKStr fileRootName = osGetRootName (_fileName);

  kkint32  lineCount = 0;

  auto  attributeTable = _fileDesc->CreateAAttributeConstTable ();  // Caller will be responsible for deleting

  bool  lineIsValid = true;

  KKStr  imageFileName = "";

  FeatureVectorListPtr  examples = new FeatureVectorList (_fileDesc, true);

  while  ((!eof)  &&  (!_cancelFlag))
  {
    lineIsValid = true;
    imageFileName = "";
    KKStr  field = C45ReadNextToken (_in, ",", eof, eol);
    if  (eof)
      break;

    if  (eol)
    {
      // We have a blank line
      continue;
    }

    FeatureVectorPtr  example = new FeatureVector (numOfFeatures);
    kkint32  fieldNum = 0;

    // Process all fields for this row  'numOfFeatures'

    for  (fieldNum = 0;  fieldNum < numOfFeatures;  fieldNum++)
    {
      if  (eol  ||  eof)
      {
        _errorMessage << "Not all Features were accounted for on Line[" << lineCount << "].";
        _log.Level (-1) << endl << endl
                        << "FeatureFileIOC45::LoadFile    " << _errorMessage << endl
                        << endl;
        delete  examples;  examples = NULL;
        delete  example;   example  = NULL;
        return NULL;
      }

      switch  (attributeTable[fieldNum]->Type ())
      {
      case AttributeType::Ignore:  
        example->AddFeatureData (fieldNum, field.ToFloat ());
        break;
              
      case AttributeType::Numeric: 
        example->AddFeatureData (fieldNum, field.ToFloat ());
        break;

      case AttributeType::Nominal: 
      {
        kkint32  code = -1;  // Initialize to value for missing data.
        if  (field == "?")
        {
          // Will flag this entry as having missing data.
          example->MissingData (true);
        }
        else
        {
          // This is not a missing data.
          code = attributeTable[fieldNum]->GetNominalCode (field);
          if  (code < 0)
          {
            _errorMessage << "Invalid NominalValue[" << field << "]  on line[" << lineCount << "].";
            _log.Level (-1) << endl << endl
                            << "FeatureFileIOC45::LoadFile    " << _errorMessage << endl
                            << endl;
            delete  examples;  examples = NULL;
            delete  example;   example  = NULL;
            return NULL;
          }
        }

        example->AddFeatureData (fieldNum, (float)code);
        break;
      }

      case AttributeType::Symbolic: 
      {
        kkint32  code = -1;  // Initialize to value for missing data.
        if  (field == "?")
        {
          // Will flag this entry as having missing data.
          example->MissingData (true);
        }
        else
        {
          // This is not a missing data.

          if  (attributeTable[fieldNum]->Name ().EqualIgnoreCase ("ExampleFileName"))
            imageFileName = field;

          code = attributeTable[fieldNum]->GetNominalCode (field);
          if  (code < 0)
          {
            _errorMessage << "Invalid NominalValue[" << field << "]  on line[" << lineCount << "].";
            _log.Level (-1) << endl << endl
              << "FeatureFileIOC45::LoadFile    " << _errorMessage << endl
              << endl;
          }
        }

        example->AddFeatureData (fieldNum, (float)code);
        break;
      }

      default:
        _log.Level (-1) << endl << endl
                        << "FeatureFileIOC45::LoadFile    *** Undefined Field Type ***" << endl
                        << endl;
        break;

      }  /* End of switch */

      field = C45ReadNextToken (_in, " ,", eof, eol);
    }

    // 'field' should have the class name in it
    if  ((field.Empty ())  ||  eol)
    {
      _errorMessage << "Line[" << lineCount << "]  Missing ClassName.";
      _log.Level (-1) << endl << endl 
                      << "FeatureFileIOC45::LoadFile    " << _errorMessage << endl
                      << endl;
      
      delete  examples;  examples = NULL;
      delete  example;   example  = NULL;
      return  NULL;
    }
    
    MLClassPtr mlClass = NULL;

    if  (field == "?")
    {
      // The class is unknown
      mlClass = _fileDesc->LookUpUnKnownMLClass ();
    }
    else
    {
      mlClass = _fileDesc->LookUpMLClassByName (field);
      if  (!mlClass)
      {
        lineIsValid = false;
        _errorMessage << "Line[" << lineCount << "]  Invalid Class[" << field << "]";
        _log.Level (-1) << endl << endl
                        << "FeatureFileIOC45::LoadFile  " << _errorMessage << endl
                        << endl;
        delete  examples;  examples = NULL;
        delete  example;   example  = NULL;
        return  NULL;
      }
    }

    example->MLClass (mlClass);

    if  (imageFileName.Empty ())
      imageFileName = fileRootName + "_" + StrFormatInt (lineCount, "ZZZZZ0");

    example->ExampleFileName (imageFileName);

    if  (lineIsValid)
      examples->PushOnBack (example);

    lineCount++;

    if  (!eof)
    {
      // Consume the rest of the characters in the line so that the next pass starts at
      // the beginning of the next line.
      kkint32  ch = _in.peek ();
      while  ((ch != '\n')  &&  (ch != '\r')  &&  (!_in.eof ()))
      {
        _in.get ();
        ch = _in.peek ();
      }
      if  (!_in.eof ())
      {
        _in.get ();
        if  ((ch == '\n')  &&  (_in.peek () == '\r'))
           _in.get ();

        else if  ((ch == '\r')  &&  (_in.peek () == '\n'))
           _in.get ();
      }
    }

    if  ((lineCount % 1000) == 0)
      cout  << "Records Loaded " << lineCount << endl;

    if  (examples->QueueSize () > _maxCount)
      break;
  }

  _log.Level (50) << "FeatureFileIOC45::LoadFile   _changesMade: " << _changesMade << "   _changesMade: " << _changesMade  << endl 
    << "_classes: " << _classes.ToString() << endl;

  delete[] attributeTable;
  attributeTable = NULL;

  return  examples;
}  /* LoadFile */



void  FeatureFileIOC45::C45ConstructFileNameForWritting (const KKStr&  fileName,
                                                         KKStr&        namesFileName,
                                                         KKStr&        dataFileName
                                                        )
{
  KKStr  c45Name;

  auto  lastDotPos = fileName.LocateLastOccurrence ('.');

  if  (!lastDotPos)
  {
    // First try file name with ".data" extension, then with no extension
    namesFileName = fileName + ".names";
    dataFileName  = fileName + ".data";
  }
  else
  {
    KKStr  leedingPart = fileName.SubStrPart (0, lastDotPos.value ());
    KKStr  extension   = fileName.SubStrPart (lastDotPos.value () + 1);
    extension.Upper ();
    if  ((extension == "NAMES")  ||  (extension == "NAME"))
    {
      namesFileName = fileName;
      dataFileName  = leedingPart + "data";
    }

    else if  ((extension == "DATA")  ||  (extension == "TEST"))
    {
      // Since there is an extension and it is not the 'names'  extension,
      // will just assume the user knows what they are doing.
      namesFileName = leedingPart + "names";
      dataFileName  = fileName;
    }

    else
    {
      namesFileName = fileName + ".names";
      dataFileName  = fileName;
    }
  }

  return;
}  /* C45ConstructFileNameForWritting */



KKStr  FeatureFileIOC45::C45AdjName (const  KKStr&  oldName)
{
  kkuint32  x;
  KKStr  newName (oldName.Len () + 3);
  for  (x = 0; x < oldName.Len ();  x++)
  {
    char ch = oldName[x];

    if  (strchr (",:?", ch))
    {
      newName.Append ('\\');
    }
    newName.Append (ch);
  }

  return  newName;
} /* C45AdjName */



void   FeatureFileIOC45::SaveFile (FeatureVectorList&     _data,
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
  _log.Level (30) << "FeatureFileIOC45::SaveFile   fileName: " << _fileName << endl;
  _errorMessage = "";
  KKStr  namesFileName;
  KKStr  dataFileName;

  _numExamplesWritten = 0;
  
  C45ConstructFileNameForWritting (_fileName, namesFileName, dataFileName);

  FileDescConstPtr fileDesc = _data.FileDesc ();

  AttributeConstPtr*  attrTable = fileDesc->CreateAAttributeConstTable ();

  {
    // Write out names file
    ofstream  nf (namesFileName.Str ());
    MLClassListPtr  classes = _data.ExtractListOfClasses ();
    for  (kkuint32 x = 0;  x < classes->QueueSize ();  x++)
    {
      if  (x > 0)
        nf << ", ";
      nf << C45AdjName (classes->IdxToPtr (x)->Name ());
    }
    nf << "." << endl;

    for  (kkuint16 x = 0;  x < _selFeatures.NumOfFeatures ();  x++)
    {
      kkint32  featureNum = _selFeatures[x];
      auto attr = attrTable[featureNum];
      nf << C45AdjName (attr->Name ()) << ": ";
      if  (attr->Type () == AttributeType::Nominal)
      {
        kkint32 y;
        for  (y = 0;  y < attr->Cardinality ();  y++)
        {
          if  (y > 0)  nf << ", ";
          nf << C45AdjName (attr->GetNominalValue (y));
        }
      }

      else if  (attr->Type () == AttributeType::Symbolic)
      {
        nf << "Symbolic";
      }

      else if  (attr->Type () == AttributeType::Ignore)
      {
        nf << "ignore";
      }

      else
      {
        nf << "continuous";
      }

      nf << "." << endl;
    }

    nf << "ExampleFileName" << ": " << "Symbolic" << "." << endl;

    nf.close ();
  }

  {
    // Write out class statistics as comments at top of file.
    ClassStatisticListPtr  stats = _data.GetClassStatistics ();
    ClassStatisticList::iterator  idx;

    _out << "|  FileName         [" << _fileName                     << "]" << endl;
    _out << "|  DateWritten      [" << osGetLocalDateTime ()         << "]" << endl;
    _out << "|  SelectedFeatures [" << _selFeatures.ToString ()      << "]" << endl;
    _out << "|  TotalRecords     [" << _data.QueueSize ()            << "]" << endl;
    _out << "|  NumAttributes    [" << _selFeatures.NumOfFeatures () << "]" << endl;
    _out << "|"                                                             << endl;
    _out << "|  Class Statistics"                                           << endl;
    _out << "|  Name" << "\t" << "Count"                                    << endl;

    for  (idx = stats->begin ();  idx != stats->end ();  idx++)
    {
      ClassStatisticPtr stat = *idx;
      _out << "|  " << stat->Name () << "\t" << stat->Count () << endl;
    }
    _out << "|" << endl;

    delete  stats;
  }

  kkint32  origPrecision = (kkint32)_out.precision ();
  _out.precision (9);

  FeatureVectorPtr   example = NULL;

  for  (kkuint32 idx = 0;  (idx < _data.QueueSize ())  &&  (!_cancelFlag);  ++idx)
  {
    example = _data.IdxToPtr (idx);

    for  (kkuint16 x = 0;  x < _selFeatures.NumOfFeatures ();  x++)
    {
      kkint32  featureNum = _selFeatures[x];

      if  ((attrTable[featureNum]->Type () == AttributeType::Nominal)  ||
           (attrTable[featureNum]->Type () == AttributeType::Symbolic)
          )
      {
        if  (example->FeatureData (featureNum) == -1.0)
        {
          // Missing Data
          _out << "?";
        }
        else
        {
          _out << C45AdjName (attrTable[featureNum]->GetNominalValue ((kkint32)(example->FeatureData (featureNum))));
        }
      }
      else
      {
        _out << example->FeatureData (featureNum);
      }
      _out << ",";
    }
    _out << example->ExampleFileName () << ",";
    _out << example->MLClassName ();
    _out << endl;
    _numExamplesWritten++;
  }

  _out.precision (origPrecision);

  if  (!_cancelFlag)
    _successful = true;

  delete  attrTable;
  return;
}  /* SaveFile */
