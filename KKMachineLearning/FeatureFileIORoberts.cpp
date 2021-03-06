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

#include "FeatureFileIORoberts.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;


FeatureFileIORoberts  FeatureFileIORoberts::driver;


FeatureFileIORoberts::FeatureFileIORoberts ():
   FeatureFileIO ("Roberts", false, true)
{
}



FeatureFileIORoberts::~FeatureFileIORoberts(void)
{
}



void   FeatureFileIORoberts::SaveFile (FeatureVectorList&    _data,
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
  _log.Level (20) << "FeatureFileIORoberts::SaveFile    FileName[" << _fileName << "]" << endl;

  _numExamplesWritten = 0;

  _errorMessage = "";
  
  FileDescConstPtr  fileDesc = _data.FileDesc ();

  AttributeConstPtr*  attrTable = fileDesc->CreateAAttributeConstTable ();

  {
    KKStr  namesFileName = _fileName + ".names";
    // Write _out names file
    ofstream  nf (namesFileName.Str ());

    MLClassListPtr classes = _data.ExtractListOfClasses ();
    classes->SortByName ();
    for  (kkuint32 x = 0;  x < classes->QueueSize ();  x++)
    {
      if  (x > 0)  nf << " ";
      nf << classes->IdxToPtr (x)->Name ();
    }
    delete  classes;
    classes = NULL;
    nf << endl << endl;

    for  (kkuint16 x = 0;  x < _selFeatures.NumOfFeatures ();  x++)
    {
      kkint32  featureNum = _selFeatures[x];
      AttributeConstPtr  attr = attrTable[featureNum];
      if  ((attr->Type () == AttributeType::Nominal)  ||
           (attr->Type () == AttributeType::Symbolic)
          )
      {
        kkint32 y;
        nf << "discrete"; 
        for  (y = 0;  y < attr->Cardinality ();  y++)
          nf << " " << attr->GetNominalValue (y);
      }
      else
      {
        nf << "continuous";
      }
      nf << endl;
    }
    nf.close ();
  }

  FeatureVectorPtr   example = NULL;

  for  (kkuint32 idx = 0;  (idx < _data.QueueSize ()) && (!_cancelFlag);  idx++)
  {
    example = _data.IdxToPtr (idx);

    for  (kkuint16 x = 0; x < _selFeatures.NumOfFeatures (); x++)
    {
      kkint32  featureNum = _selFeatures[x];
      AttributeConstPtr attr = attrTable[featureNum];

      if  ((attr->Type () == AttributeType::Nominal)  ||
           (attr->Type () == AttributeType::Symbolic)
          )
        _out << attr->GetNominalValue ((kkint32)(example->FeatureData (featureNum)));
      else
        _out << example->FeatureData (featureNum);
      _out << " ";
    }
    _out << example->MLClassName ();
    _out << endl;
    _numExamplesWritten++;
  }

  if  (!_cancelFlag)
    _successful = true;

  delete  attrTable;
  return;
} /* WriteRobertsFile */
