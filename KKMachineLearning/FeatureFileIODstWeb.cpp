#include  "FirstIncludes.h"

#include <stdio.h>
#include <math.h>
#include  <ctype.h>
#include  <time.h>

#include  <string>
#include  <iostream>
#include  <fstream>
#include  <vector>

#include  "MemoryDebug.h"

using namespace std;


#include "KKBaseTypes.h"
#include "DateTime.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "FeatureFileIODstWeb.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace KKMachineLearning;



FeatureFileIODstWeb  FeatureFileIODstWeb::driver;


//  Used for DstWeb  data set 
class  FeatureFileIODstWeb::AttrDescLine
{
public:
    AttrDescLine (KKStr  desc)
    {
      KKStr  aStr = desc.ExtractToken (",\n\r\t");

      code = desc.ExtractToken (",\n\r\t");
      code.TrimLeft ();
      code.TrimRight ();

      KKStr  oneStr = desc.ExtractToken (",\n\r\t");
     
      title = desc.ExtractToken (",\n\r\t");
      if  (title.FirstChar () == '"')
        title = title.SubStrPart (1);
      if  (title.LastChar () == '"')
        title.ChopLastChar ();

      root  = desc.ExtractToken ();
      if  (root.FirstChar () == '"')
        root = root.SubStrPart (1);
      if  (root.LastChar () == '"')
        root.ChopLastChar ();
    }
   
    KKStr  code;
    KKStr  title;
    KKStr  root;

};  /* AttrDescLine */




class  FeatureFileIODstWeb::AttrDescLineComparator  
{
public:
  AttrDescLineComparator ()
  {}

  bool  operator()  (AttrDescLinePtr p1,
                     AttrDescLinePtr p2
                    )
  {
    return ((p1->code) < (p2->code));
  }
};




FeatureFileIODstWeb::FeatureFileIODstWeb ():
    FeatureFileIO ("DST", true, false)   // only read is implemented.
{
}



FeatureFileIODstWeb::~FeatureFileIODstWeb ()
{
}





FileDescPtr  FeatureFileIODstWeb::GetFileDesc (const KKStr&       _fileName,
                                               istream&           _in,
                                               MLClassListPtr  _classes,
                                               kkint32&             _estSize,
                                               KKStr&             _errorMessage,
                                               RunLog&            _log
                                              )
{
  KKStr  line (1024);
  bool   eof;
  KKStr  classNameAttribute;

  {
    // Make sure that the True and False _classes exist.
    MLClassPtr  trueClass  = _classes->GetMLClassPtr ("True");
    MLClassPtr  falseClass = _classes->GetMLClassPtr ("True");
    trueClass  = NULL;
    falseClass = NULL;
  }

  // We must first determine which Attribute line represents class.
  // this is done with a line that is added at beginning of file 
  // added by user with text editor.  Must have the format of 
  // class = xxxx  where xxxx is the attribute.

  GetLine (_in, line, eof);
  if  (eof)
  {
    _log.Level (-1) << endl << endl 
                    << "FeatureFileIODstWeb::GetFileDesc      ***ERROR***      File is empty." << endl
                    << endl;
    return  NULL;
  }

  {
    line.TrimLeft ();
    line.TrimRight ();

    kkint32  equalLoc = line.LocateCharacter ('=');
    if  (equalLoc < 0)
    {
      _log.Level (-1) << endl << endl
                      << "FeatureFileIODstWeb::GetFileDesc     *** ERROR ***   First Line is not Class Identifier." << endl
                      << endl;
      return  NULL;
    }

    KKStr  leftSide  = line.SubStrPart (0, equalLoc - 1);
    KKStr  rightSide = line.SubStrPart (equalLoc + 1);

    leftSide.Upper ();
    if  (leftSide != "CLASS")
    {
      _log.Level (-1) << endl << endl
                      << "FeatureFileIODstWeb::GetFileDesc     *** ERROR ***   First Line is not Class Identifier." << endl
                      << endl;
      return  NULL;
    }

    rightSide.TrimLeft ();
    rightSide.TrimRight ();

    classNameAttribute = rightSide;
  }


  FileDescPtr  fileDesc = new FileDesc ();


  vector<AttrDescLinePtr> attributes;


  GetLine (_in, line, eof);
  while  (!eof)
  {
    line.TrimLeft ();
    line.TrimRight ();

    if  (line.FirstChar () != 'A')
      continue;


    // We have an attribute Line
    AttrDescLinePtr  a = new AttrDescLine (line);
    if  (a->code == classNameAttribute)
    {
      delete  a;
    }
    else
    {
      attributes.push_back (a);
    }
    GetLine (_in, line, eof);
  }

  AttrDescLineComparator  c;
  sort (attributes.begin (), attributes.end (), c);

  kkuint32  x;
  for  (x = 0;  x < attributes.size ();  x++)
  {
    bool  alreadyExists = false;
    fileDesc->AddAAttribute (attributes[x]->code, NominalAttribute, alreadyExists);
    if  (alreadyExists)
    {
      _log.Level (-1) << endl
                      << endl
                      << "FeatureFileIODstWeb::GetFileDesc    *** ERROR ***    Attribute Code Occurs more than once" << endl
                      << "    code [" << attributes[x]->code << "]." << endl
                      << endl;
      // Can not delete an instance of a 'FileDesc' class once it has been created.
      // delete  fileDesc;
      return NULL;
    }

    fileDesc->AddANominalValue ("F", alreadyExists, _log);
    fileDesc->AddANominalValue ("T", alreadyExists, _log);
  }

  for  (x = 0;  x < attributes.size ();  x++)
    delete attributes[x];

  return  fileDesc;
}  /* ReadDstWebFile */








FeatureVectorListPtr  FeatureFileIODstWeb::LoadFile (const KKStr&      _fileName,
                                                     const FileDescPtr _fileDesc,
                                                     MLClassList&      _classes, 
                                                     istream&          _in,
                                                     kkint32           _maxCount,    /**< Maximum # images to load. */
                                                     VolConstBool&     _cancelFlag,
                                                     bool&             _changesMade,
                                                     KKStr&            _errorMessage,
                                                     RunLog&           _log
                                                   )
{
  _log.Level (20) << "FeatureFileIODstWeb::LoadFile   FileName[" << _fileName << "]" << endl;


  MLClassPtr  trueClass  = _classes.GetMLClassPtr ("TRUE");
  MLClassPtr  falseClass = _classes.GetMLClassPtr ("FALSE");

  kkint32  lineCount = 0;

  kkint32  numOfFeatures = _fileDesc->NumOfFields ();

  KKStr fileRootName = osGetRootName (_fileName);

  const
    AttributePtr*  attributeTable = _fileDesc->CreateAAttributeTable ();  // Caller will be responsible for deleting

  KKStr  line;
  bool   eof = false;

  // Skip all leading lines, until we reach a C line.
  GetLine (_in, line, eof);
  while  (!eof)
  {
    if  (line.FirstChar () == 'C')
      break;
    GetLine (_in, line, eof);
  }

  if  (eof)
  {
    delete  attributeTable;
    _errorMessage = "no 'C' line detected.";
    return  NULL;
  }

  FeatureVectorListPtr  examples = new FeatureVectorList (_fileDesc, true, _log);

  KKStr  classNameAttributeUpper (_fileDesc->ClassNameAttribute ());
  classNameAttributeUpper.Upper ();

  while  (!eof)
  {
    // We have a new user

    KKStr  cStr = line.ExtractToken (",\n\r\t");
    KKStr  idStr = line.ExtractToken (",\n\r\t");
    if  (idStr.FirstChar () == '"')
      idStr = idStr.SubStrPart (1);
    if  (idStr.LastChar () == '"')
      idStr.ChopLastChar ();

    FeatureVectorPtr  example = new FeatureVector (numOfFeatures);
    example->MLClass (falseClass);
    example->ImageFileName (idStr);
    {
      // Set all fields to False
      kkint32  x;
      for  (x = 0;  x < numOfFeatures;  x++)
      {
        kkint32 code = attributeTable[x]->GetNominalCode ("F");
        example->AddFeatureData (x, (float)code);
      }
    }

    GetLine (_in, line, eof);  lineCount++;

    while  ((!eof)  &&  (line.FirstChar () == 'V'))
    {
      KKStr  vStr  = line.ExtractToken (",\n\r\t");
      KKStr  idStr = line.ExtractToken (",\n\r\t");
      idStr.Upper ();
      if  (idStr == classNameAttributeUpper)
      {
        example->MLClass (trueClass);
      }
      else
      {
        kkint32  fieldNum = _fileDesc->GetFieldNumFromAttributeName (idStr);
        if  (fieldNum < 0)
        {
          _errorMessage << "Invalid Attribute[" << idStr + "]  Line[" << lineCount << "]";
          _log.Level (-1) << endl
                          << "FeatureFileIODstWeb::LoadFile     ***ERROR***"  << endl
                          << endl
                          << "                        " << _errorMessage << endl
                          << endl;
          delete  examples;
          delete  example;
          return  NULL;
        }

        kkint32 code = attributeTable[fieldNum]->GetNominalCode ("T");
        example->AddFeatureData (fieldNum, (float)code);
      }

      GetLine (_in, line, eof);  lineCount++;
    }

    examples->PushOnBack (example);
  }

  delete  [] attributeTable;

  return  examples;
}  /* LoadFile */





void   FeatureFileIODstWeb::SaveFile (FeatureVectorList&     _data,
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
  _log.Level (-1) << endl << endl
                  << "FeatureFileIODstWeb::SaveFile     FileName[" << _fileName << "]            ***ERROR***." << endl
                  << endl
                  << "                       SaveFile   not implemented." << endl
                  << endl;

  _errorMessage = "FeatureFileIODstWeb::SaveFile    Not Implemented.";
  _successful = false;
  _numExamplesWritten = 0;
  return;
}  /* SaveFile */
