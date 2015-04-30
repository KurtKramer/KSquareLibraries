#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "GlobalGoalKeeper.h"
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



ModelParam::ModelParam  ():

  encodingMethod            (NoEncoding),
  examplesPerClass          (int32_max),
  fileName                  (),
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
  


void  ModelParam::SelectedFeatures   (FeatureNumListConst&  _selectedFeatures)   
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

  ModelParamTypes  modelParamType = ModelParamTypes::mptNULL;

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
      if  (modelParamType == ModelParamTypes::mptNULL)
      {
        _log.Level (-1) << endl
          << "ModelParam::CreateModelParam  ***ERROR***   Invalid ModelParamType[" << modelParamTypeStr << "]." << endl
          << endl;
        
      }
      break;
    }
  }

  if  (modelParamType == ModelParamTypes::mptNULL)
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
  case  ModelParamTypes::mptDual:      modelParam = new ModelParamDual      ();  break;
  case  ModelParamTypes::mptKNN:       modelParam = new ModelParamKnn       ();  break;
  case  ModelParamTypes::mptOldSVM:    modelParam = new ModelParamOldSVM    ();  break;
  case  ModelParamTypes::mptSvmBase:   modelParam = new ModelParamSvmBase   ();  break;
  case  ModelParamTypes::mptUsfCasCor: modelParam = new ModelParamUsfCasCor ();  break;
  }

  if  (!modelParam)
    return  NULL;

  modelParam->ReadXML (i, fileDesc, _log);
  return  modelParam;
}  /* CreateModelParam */





KKStr   ModelParam::ModelParamTypeToStr (ModelParamTypes _modelParamType)
{
  if  (_modelParamType == ModelParamTypes::mptNULL)
    return "NULL";
  
  else if  (_modelParamType == ModelParamTypes::mptDual)
    return "ModelParamDual";

  else if  (_modelParamType == ModelParamTypes::mptKNN)
    return "ModelParamKnn";

  else if  (_modelParamType == ModelParamTypes::mptOldSVM)
    return "ModelParamOldSVM";
  
  else if  (_modelParamType == ModelParamTypes::mptSvmBase)
    return "ModelParamSvmBase";

  else if  (_modelParamType == ModelParamTypes::mptUsfCasCor)
    return  "UsfCasCor";

  else
    return "NULL";
}



ModelParam::ModelParamTypes  ModelParam::ModelParamTypeFromStr (const KKStr&  _modelParamTypeStr)
{
  if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamDual"))
    return ModelParamTypes::mptDual;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamOldSVM"))
    return ModelParamTypes::mptOldSVM;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamSvmBase"))
    return ModelParamTypes::mptSvmBase;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("ModelParamKnn"))
    return ModelParamTypes::mptKNN;

  else if  (_modelParamTypeStr.EqualIgnoreCase ("UsfCasCor"))
    return ModelParamTypes::mptUsfCasCor;

  else
    return ModelParamTypes::mptNULL;
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





void  ModelParam::ParseCmdLine (KKStr    _cmdLineStr,
                                bool&    _validFormat,
                                RunLog&  _log
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
      _log.Level (-1) << "ModelParam::ParseCmdLine  *** Invalid Parameter["
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
        _log.Level (-1) 
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
      ParseCmdLineParameter (field, value, parameterUsed, _log);
      if  (!parameterUsed)
      {
        _log.Level (-1) << "ModelParam::ParseCmdLine - Invalid Parameter["  
          << field << "]  Value[" << value << "]."
          << endl;
        _validFormat = false;
        break;
      }
    }

    field = _cmdLineStr.ExtractToken (" \t\n\r");
  } 

  ParseCmdLinePost (_log);

  validParam = _validFormat;
}  /* ParseCmdLine */



/**
 * @brief Called after 'ParseCmdLine' is completed.  Classed derived from 'ModelParam' can implement this
 *        method to do any processing that they want after the entire command line has been processed.
 * @details  An example use of this is in 'ModelParamSvmBase' where the local 'ParseCmdLineParameter'
 *           routine processes parameters that 'Model' needs to be aware of.
 */
void  ModelParam::ParseCmdLinePost (RunLog&  log)
{
}



/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParam::ToCmdLineStr () const
{
  KKStr  cmdStr (300);

  if  (selectedFeatures)
    cmdStr << "-SF " + selectedFeatures->ToCommaDelStr ();

  if  (examplesPerClass < int32_max)
    cmdStr << "  -EPC " << examplesPerClass;

  if  (encodingMethod != NoEncoding)
    cmdStr << "  -Encode " + EncodingMethodToStr (encodingMethod);

  return  cmdStr;
}  /* ToCmdLineStr */




void  ModelParam::WriteXML (ostream&  o,
                            RunLog&   log
                           )  const
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
                           FileDescPtr  fileDesc,
                           RunLog&      log
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
      ParseCmdLine (cmdLine, validFormat, log);
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
      ReadSpecificImplementationXML (i, fileDesc, log);
    }
  }
}  /* ReadXML */




void  ModelParam::ReadXML (KKStr&       _fileName,
                           FileDescPtr  _fileDesc,
                           bool&        _successful,
                           RunLog&      _log
                          )
{
  _log.Level (10) << "ModelParam::ReadXML - File[" << _fileName << "]." << endl;

  _successful = true;

  fileName = _fileName;

  ifstream  inputFile (fileName.Str ());

  if  (!inputFile.is_open ())
  {
    _log.Level (-1) << "ModelParam::ReadXML      *** ERROR ***" << endl;
    _log.Level (-1) << "                     Could Not Open File[" << fileName << "]." << endl;
    _successful = false;
    return;
  }

  ReadXML (inputFile, _fileDesc, _log);
   
  inputFile.close ();
}  /* ReadXML */



kkint32  ModelParam::NumOfFeaturesAfterEncoding (FileDescPtr  fileDesc,
                                                 RunLog&      log
                                                )  const
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
      if  ((fileDesc->Type (fieldNum) == AttributeType::NominalAttribute)  ||
           (fileDesc->Type (fieldNum) == AttributeType::SymbolicAttribute)
          )
        numFeaturesAfterEncoding += fileDesc->Cardinality (fieldNum);
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



XmlTokenPtr  ModelParam::ReadXMLModelParamToken (XmlTokenPtr  t)
{
  bool  tokenFound = true;

  const KKStr&  varName = t->VarName ();
  if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
  {
    XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);


    if  (varName.EqualIgnoreCase ("EncodingMethod"))
    {
      encodingMethod = EncodingMethodFromStr (dynamic_cast<XmlElementKKStrPtr> (e)->Value ());
    }

    else if  (varName.EqualIgnoreCase ("ExamplesPerClass"))
    {
      examplesPerClass = dynamic_cast<XmlElementInt32Ptr> (e)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("FileName"))
    {
      fileName = *(dynamic_cast<XmlElementKKStrPtr> (e)->Value ());
    }

    else if  (varName.EqualIgnoreCase ("NormalizeNominalFeatures"))
    {
      normalizeNominalFeatures = dynamic_cast<XmlElementBoolPtr> (e)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("SelectedFeatures"))
    {
      selectedFeatures = dynamic_cast<XmlElementFeatureNumListPtr> (e)->TakeOwnership ();
    }

    else if  (varName.EqualIgnoreCase ("Cost"))
    {
      cost = dynamic_cast<XmlElementDoublePtr> (e)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("Gamma"))
    {
      gamma = dynamic_cast<XmlElementDoublePtr> (e)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("Prob"))
    {
      prob = (float)dynamic_cast<XmlElementDoublePtr> (e)->Value ();
    }

    else if  (varName.EqualIgnoreCase ("ValidParam"))
    {
      validParam = dynamic_cast<XmlElementBoolPtr> (e)->Value ();
    }
    else
    {
      tokenFound = false;
    }
  }

  if  (tokenFound)
  {
    delete t;
    t = NULL;
  }

  return  t;
}  /* ReadXMLModelParamToken */



void  ModelParam::WriteXMLFields (ostream&  o)  const

{
  EncodingMethodToStr (encodingMethod).WriteXML ("EncodingMethod", o);

  XmlElementInt32::WriteXML (examplesPerClass, "ExamplesPerClass", o);

  fileName.WriteXML ("FileName", o);

  XmlElementBool::WriteXML (normalizeNominalFeatures, "NormalizeNominalFeatures", o);

  if  (selectedFeatures)
    selectedFeatures->WriteXML ("SelectedFeatures", o);

  XmlElementDouble::WriteXML (cost,       "Cost",       o);
  XmlElementDouble::WriteXML (gamma,      "Gamma",      o);
  XmlElementDouble::WriteXML (prob,       "Prob",       o);
  XmlElementBool::WriteXML   (validParam, "ValidParam", o);
}  /* WriteXML */

