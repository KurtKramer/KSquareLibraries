#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace KKB;


#include "ModelParam.h"
#include "ModelParamKnn.h"
#include "ModelParamOldSVM.h"
#include "ModelParamSvmBase.h"
#include "ModelParamUsfCasCor.h"
#include "ModelParamDual.h"
#include "KKMLLTypes.h"
#include "Model.h"
//#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;



ModelParam::ModelParam  (RunLog&  _log):

  encodingMethod            (NoEncoding),
  examplesPerClass          (int32_max),
  fileName                  (),
  log                       (_log),
  normalizeNominalFeatures  (false),
  selectedFeatures          (NULL), 
  validParam                (true),
  // SVM related parameters
  cost                      (0.0),
  gamma                     (0.0),
  prob                      (0.0f)
{
  /*
  if  (!fileDesc)
  {
    log.Level (-1) << endl
                   << "ModelParam::ModelParam      *** ERROR ***" << endl
                   << "                       fileDesc == NULL" << endl
                   << endl;
    osWaitForEnter ();
    exit (-1);
  }
  */
}





ModelParam::ModelParam  (const ModelParam&  _param):

  encodingMethod             (_param.encodingMethod),
  examplesPerClass           (_param.examplesPerClass),
  fileName                   (_param.fileName),
  log                        (_param.log),
  normalizeNominalFeatures   (_param.normalizeNominalFeatures),
  selectedFeatures           (NULL),
  validParam                 (_param.validParam),
  // SVM related parameters
  cost                       (_param.cost),
  gamma                      (_param.gamma),
  prob                       (_param.prob)
{
  if  (_param.selectedFeatures)
    selectedFeatures = new FeatureNumList (*(_param.selectedFeatures));
}



ModelParam::~ModelParam  ()
{
}


kkint32  ModelParam::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (ModelParam)
    +  fileName.MemoryConsumedEstimated ();

  if  (selectedFeatures)
    memoryConsumedEstimated += selectedFeatures->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



float   ModelParam::AvgMumOfFeatures () const 
{
  if  (selectedFeatures)
    return   (float)selectedFeatures->NumOfFeatures ();
  else
    return  0;
}
  


void  ModelParam::SelectedFeatures   (const FeatureNumList&  _selectedFeatures)   
{
  delete  selectedFeatures;
  selectedFeatures = new FeatureNumList (_selectedFeatures);
}



ModelParamPtr  ModelParam::CreateModelParam (istream&     i,
                                             FileDescPtr  fileDesc,
                                             RunLog&      _log
                                            )
{
  istream::pos_type startPos = i.tellg ();

  char  buff[20480];
  KKStr  ln;

  ModelParamTypes  modelParamType = mptNULL;


  // First we need to determine which type of model this is.  We will
  // scan through the file until we locate a ModelParamType field.
  while  (i.getline (buff, sizeof (buff)))
  {
    ln = buff;
    KKStr  lineName= ln.ExtractToken2 ("\t");

    if  (lineName.EqualIgnoreCase ("</ModelParam>"))
    {
      // We did not find the parameter type 
      break;
    }

    if  (lineName.EqualIgnoreCase ("ModelParamType"))
    {
      KKStr  modelParamTypeStr = ln.ExtractToken2 ("\t");
      modelParamType = ModelParamTypeFromStr (modelParamTypeStr);
      if  (modelParamType == mptNULL)
      {
        _log.Level (-1) << endl
          << "ModelParam::CreateModelParam  ***ERROR***   Invalid ModelParamType[" << modelParamTypeStr << "]." << endl
          << endl;
        
      }
      break;
    }
  }

  if  (modelParamType == mptNULL)
  {
    // We never found the type of parameter we are looking for.
    _log.Level (-1) << endl
      << "ModelParam::CreateModelParam  ***ERROR***   No Parameter Type was defined." << endl
      << endl;
    return NULL;
  }

  i.seekg (startPos);

  ModelParamPtr modelParam = NULL;
  switch  (modelParamType)
  {
  case  mptDual:      modelParam = new ModelParamDual      (_log);
                      break;

  case  mptKNN:       modelParam = new ModelParamKnn       (_log);
                      break;

  case  mptOldSVM:    modelParam = new ModelParamOldSVM    (_log);
                      break;
 
  case  mptSvmBase:   modelParam = new ModelParamSvmBase   (_log);
                      break;

  case  mptUsfCasCor: modelParam = new ModelParamUsfCasCor (_log);
                      break;

  }

  if  (!modelParam)
    return  NULL;

  modelParam->ReadXML (i, fileDesc);
  return  modelParam;
}  /* CreateModelParam */





KKStr   ModelParam::ModelParamTypeToStr (ModelParamTypes _modelParamType)
{
  if  (_modelParamType == mptNULL)
    return "NULL";
  
  else if  (_modelParamType == mptDual)
    return "ModelParamDual";

  else if  (_modelParamType == mptKNN)
    return "ModelParamKnn";

  else if  (_modelParamType == mptOldSVM)
    return "ModelParamOldSVM";
  
  else if  (_modelParamType == mptSvmBase)
    return "ModelParamSvmBase";

  else if  (_modelParamType == mptUsfCasCor)
    return  "UsfCasCor";

  else
    return "NULL";
}



ModelParam::ModelParamTypes  ModelParam::ModelParamTypeFromStr (const KKStr&  _modelParamTypeStr)
{
  if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamDual"))
    return mptDual;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamOldSVM"))
    return mptOldSVM;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamSvmBase"))
    return mptSvmBase;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamKnn"))
    return mptKNN;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("UsfCasCor"))
    return mptUsfCasCor;

  else
    return mptNULL;
}




float  ModelParam::A_Param  () const
{
  return  prob;
}

double  ModelParam::C_Param  () const
{
  return cost;
}

double  ModelParam::Cost     () const
{
  return cost;
}

double  ModelParam::Gamma    () const
{
  return  gamma;
}

float  ModelParam::Prob () const
{
  return  prob;
}





void  ModelParam::A_Param  (float   _prob)
{
  prob = _prob;
}


void  ModelParam::C_Param  (double  _cost)
{
  cost = _cost;
}


void  ModelParam::Cost (double  _cost)
{
  cost = _cost;
}


void  ModelParam::Gamma (double  _gamma)
{
  gamma = _gamma;
}


void  ModelParam::Prob (float _prob)
{
  prob = _prob;
}





void  ModelParam::ParseCmdLine (KKStr   _cmdLineStr,
                                bool&   _validFormat
                               )
{
  _validFormat = true;

  //DecodeParamStr (_cmdLineStr, param);

  KKStr  field (_cmdLineStr.ExtractToken (" \t\n\r"));
  KKStr  value;

  double  valueNum;

  while  (!field.Empty ()  &&  _validFormat)
  {
    if  (field.FirstChar () != '-')
    {
      log.Level (-1) << "ModelParam::ParseCmdLine  *** Invalid Parameter["
        << field << "] ***"
        << endl;
      _validFormat = false;
      break;
    }

    // See if next field is a Switch field or a parameter.
    _cmdLineStr.TrimLeft (" \t\n\r");
    value == "";
    if  (_cmdLineStr.Len () > 0)
    {
      if  (_cmdLineStr.FirstChar () != '-')
        value = _cmdLineStr.ExtractToken (" \t\n\r");
    }

    valueNum = atof (value.Str ()); 

    field.Upper ();
    KKStr valueUpper (value);

    valueUpper.Upper ();

    if  ((field == "-FS")  ||  (field == "-FEATURESSELECTED")  ||  (field == "-FEATURESSEL")  ||  (field == "FEATURESEL"))
    {
      delete  selectedFeatures;
      bool  valid = true;
      selectedFeatures = FeatureNumList::ExtractFeatureNumsFromStr (value);
      if  (!selectedFeatures)
        _validFormat= false;
    }

    else if  (field.EqualIgnoreCase ("-C")  ||  field.EqualIgnoreCase ("-Cost"))
    {
      Cost (valueNum);
    }

    else if ((field == "-ENCODE"))
    {
      encodingMethod = EncodingMethodFromStr (valueUpper);
    }

    else if  ((field.EqualIgnoreCase ("-EPC"))  ||  
              (field.EqualIgnoreCase ("-ExamplesPerClass"))
             )
    {
      examplesPerClass = value.ToInt ();
      if  (examplesPerClass < 1)
      {
        log.Level (-1) 
          << endl << endl
          << "ModelParam::ParseCmdLine ***ERROR***  Invalid '-ExamplsPerClass' parameter specified[" << value << "]" << endl
          << endl;
        _validFormat = false;
        examplesPerClass = int32_max;
        break;
      }
    }

    else if  (field.EqualIgnoreCase ("-G")  ||  field.EqualIgnoreCase ("-Gamma"))
    {
      Gamma (valueNum);
    }

    else if  ((field.EqualIgnoreCase ("-NormNominal"))               ||
              (field.EqualIgnoreCase ("-NormalizeNominal"))          ||
              (field.EqualIgnoreCase ("-NormalizeNominalFeatures"))  ||
              (field.EqualIgnoreCase ("-NN"))
             )
    {
      if  (value.Empty ())
        normalizeNominalFeatures = true;
      else
        normalizeNominalFeatures = value.ExtractTokenBool ("\t");
    }

    else
    {
      bool  parameterUsed = false;
      ParseCmdLineParameter (field, value, parameterUsed);
      if  (!parameterUsed)
      {
        log.Level (-1) << "ModelParam::ParseCmdLine - Invalid Parameter["  
          << field << "]  Value[" << value << "]."
          << endl;
        _validFormat = false;
        break;
      }
    }

    field = _cmdLineStr.ExtractToken (" \t\n\r");
  } 

  ParseCmdLinePost ();

  validParam = _validFormat;
}  /* ParseCmdLine */



/**
 * @brief Called after 'ParseCmdLine' is completed.  Classed derived from 'ModelParam' can implement this
 *        method to do any processing that they want after the entire command line has been processed.
 * @details  An example use of this is in 'ModelParamSvmBase' where the local 'ParseCmdLineParameter'
 *           routine processes parameters that 'Model' needs to be aware of.
 */
void  ModelParam::ParseCmdLinePost ()
{
}



/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParam::ToCmdLineStr () const
{
  log.Level (60) << "ModelParam::ToCmdLineStr - Entered." << endl;

  KKStr  cmdStr (300);

  if  (selectedFeatures)
    cmdStr << "-SF " + selectedFeatures->ToCommaDelStr ();

  if  (examplesPerClass < int32_max)
    cmdStr << "  -EPC " << examplesPerClass;

  if  (encodingMethod != NoEncoding)
    cmdStr << "  -Encode " + EncodingMethodToStr (encodingMethod);

  return  cmdStr;
}  /* ToCmdLineStr */




void  ModelParam::WriteXML (ostream&  o)  const
{
  log.Level (20) << "ModelParam::Save to XML to ostream." << endl;

  o << "<ModelParam>" << endl
    << "ModelParamType"     << "\t"  << ModelParamTypeStr    ()             << std::endl
    << "EncodingMethod"     << "\t"  << EncodingMethodStr    ()             << std::endl
    << "ExamplesPerClass"   << "\t"  << examplesPerClass                    << std::endl
    << "FileName"           << "\t"  << fileName                            << std::endl;

  if  (selectedFeatures)
    o << "SelectedFeatures"   << "\t"  << selectedFeatures->ToCommaDelStr ()  << std::endl;

  o  << endl;
  
  o << "<SpecificImplementation>"  << endl;
  WriteSpecificImplementationXML (o);
  o << "</SpecificImplementation>"  << endl;

 o << "</ModelParam>" << endl;
}  /* WriteXML */





void  ModelParam::ReadXML (istream&     i,
                           FileDescPtr  fileDesc
                          )
{
  log.Level (20) << "ModelParam::Read from XML file." << endl;

  char  buff[10240];
  
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", 
                                         true      // true = decode escape characters
                                        );
    field.Upper ();

    if  (field.EqualIgnoreCase ("</ModelParam>"))
    {
      break;
    }

    else if  (field == "<CMDLINE>")
    {
      bool  validFormat;
      KKStr  cmdLine = ln.ExtractQuotedStr ("\n\r\t", 
                                             true      // true = decode escape characters
                                            );
      ParseCmdLine (cmdLine, validFormat);
    }

    else if  (field.EqualIgnoreCase ("EncodingMethod"))
    {
      KKStr  encodingMethodStr =  ln.ExtractToken2 ("\t");
      encodingMethod = EncodingMethodFromStr (encodingMethodStr);
      if  (encodingMethod == Encoding_NULL)
      {
        log.Level (-1) << endl << "ModelParam::ReadXML   ***ERROR***  Invalid EncodingMethod Method[" << encodingMethodStr << "]" << endl;
        validParam = false;
      }
    }

    else if  (field.EqualIgnoreCase ("ExamplesPerClass"))
    {
      examplesPerClass = ln.ExtractTokenInt ("\t");
      if  (examplesPerClass < 1)
      {
        log.Level (-1) 
          << endl << endl
          << "ModelParam::ReadXML   ***ERROR***  Invalid ExamplesPerClass[" << examplesPerClass << "]" << endl
          << endl;
        validParam = false;
        examplesPerClass = int32_max;
      }
    }

    else if  (field.EqualIgnoreCase ("FileName"))
    {
      fileName = ln.ExtractToken2 ("\t");
    }
  
    else if  (field.EqualIgnoreCase ("SelectedFeatures"))
    {
      delete selectedFeatures;
      KKStr  selectedFeaturesStr =  ln.ExtractToken2 ("\t");
      selectedFeatures = FeatureNumList::ExtractFeatureNumsFromStr (selectedFeaturesStr);
      if  (!selectedFeatures)
      {
        log.Level (-1) << "ModelParam::ReadXML   ***ERROR***  Invalid selected Features[" << selectedFeaturesStr << "]" << endl;
        validParam = false;
      }
    }

    else if  (field.EqualIgnoreCase ("<SpecificImplementation>"))
    {
      ReadSpecificImplementationXML (i, fileDesc);
    }
  }
}  /* ReadXML */




void  ModelParam::ReadXML (KKStr&       _fileName,
                           FileDescPtr  _fileDesc,
                           bool&        _successful
                          )
{
  log.Level (10) << "ModelParam::ReadXML - File[" << _fileName << "]." << endl;

  _successful = true;

  fileName = _fileName;

  ifstream  inputFile (fileName.Str ());

  if  (!inputFile.is_open ())
  {
    log.Level (-1) << "ModelParam::ReadXML      *** ERROR ***" << endl;
    log.Level (-1) << "                     Could Not Open File[" << fileName << "]." << endl;
    _successful = false;
    return;
  }

  ReadXML (inputFile, _fileDesc);
   
  inputFile.close ();
}  /* ReadXML */



kkint32  ModelParam::NumOfFeaturesAfterEncoding (FileDescPtr  fileDesc)  const
{
  kkint32 z;
  kkint32 numFeaturesAfterEncoding = 0;

  if  (!selectedFeatures)
    selectedFeatures = new FeatureNumList (fileDesc);

  kkint32 numOfFeaturesSelected = selectedFeatures->NumOfFeatures ();

  switch (EncodingMethod ())
  {
  case BinaryEncoding:
    for  (z = 0; z < numOfFeaturesSelected; z++)
    {
      kkint32  fieldNum = (*selectedFeatures)[z];
      if  ((fileDesc->Type (fieldNum) == NominalAttribute)  ||  (fileDesc->Type (fieldNum) == SymbolicAttribute))
        numFeaturesAfterEncoding += fileDesc->Cardinality (fieldNum, log);
      else
        numFeaturesAfterEncoding ++;
    }
    break;

  case ScaledEncoding:
  case NoEncoding:
  default:
    //numFeaturesAfterEncoding = fileDesc->NumOfFields ( );
    numFeaturesAfterEncoding = selectedFeatures->NumOfFeatures ();
    break;
  }

  return  numFeaturesAfterEncoding;
}  // NumOfFeaturesAfterEncoding 




KKStr  ModelParam::EncodingMethodToStr (EncodingMethodType  encodingMethod)
{
  if  (encodingMethod == BinaryEncoding)
    return  "Binary";

  else if  (encodingMethod == ScaledEncoding)
    return  "Scale";

  else
    return  "None";
}  /* EncodingMethodToStr */




        
ModelParam::EncodingMethodType  ModelParam::EncodingMethodFromStr (const KKStr&  encodingMethodStr)
{
  KKStr  encodingMethodUpper = encodingMethodStr.ToUpper ();

  if  ((encodingMethodUpper == "BINARY")  ||  (encodingMethodUpper == "BIN"))
     return  BinaryEncoding;

  if  (encodingMethodUpper == "SCALE")
     return  ScaledEncoding;

  if  (encodingMethodUpper == "NONE")
    return  NoEncoding;

  return  Encoding_NULL;
}  /* EncodingMethodFromStr */


