#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;

#include "KKBaseTypes.h"
#include "KKException.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace KKB;


#include "ModelParamOldSVM.h"
#include "BinaryClassParms.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
using namespace KKMLL;


ModelParamOldSVM::ModelParamOldSVM  (RunLog&  _log):

  ModelParam (_log),
  svmParameters (NULL)
{
  svmParameters = new SVMparam (_log);
}



ModelParamOldSVM::ModelParamOldSVM  (const ModelParamOldSVM&  _param):
  ModelParam (_param),
  svmParameters (NULL)

{
  if  (_param.svmParameters)
    svmParameters = new SVMparam (*_param.svmParameters);
}



ModelParamOldSVM::~ModelParamOldSVM  ()
{
  delete  svmParameters;
  svmParameters = NULL;
}


ModelParamOldSVMPtr  ModelParamOldSVM::Duplicate () const
{
  return new ModelParamOldSVM (*this);
}



void  ModelParamOldSVM::A_Param (float  _A)
{
  ModelParam::A_Param (_A);
  svmParameters->A_Param (_A);
}


const
BinaryClassParmsListPtr  ModelParamOldSVM::BinaryParmsList () const
{
  return  svmParameters->BinaryParmsList ();
}


void  ModelParamOldSVM::C_Param (double  _CC)
{
  ModelParam::Cost (_CC);
  svmParameters->C_Param (_CC);
}




void  ModelParamOldSVM::C_Param (MLClassPtr  class1,
                                 MLClassPtr  class2,
                                 double      cParam
                                )
{
  if  (!this->BinaryParmsList ())
  {
    cerr << endl << endl
      << "ModelParamOldSVM::C_Param   ***ERROR***     'binaryParmsList'  is not defined." << endl
      << endl;
    return;
  }

  BinaryClassParmsPtr  binaryParms = BinaryParmsList ()->LookUp (class1, class2);
  if  (!binaryParms)
  {
    svm_parameter binaryParam = this->Param ();
    binaryParam.C = C_Param ();
    AddBinaryClassParms (class1, class2, binaryParam, SelectedFeatures (), 1.0f);
  }
  else
  {
    binaryParms->C (cParam);
  }
}  /* C_Param */




double  ModelParamOldSVM::C_Param (MLClassPtr  class1,
                                   MLClassPtr  class2
                                  )  const
{
  if  (!BinaryParmsList ())
  {
    cerr << endl << endl
      << "ModelParamOldSVM::C_Param   ***ERROR***     'binaryParmsList'  is not defined." << endl
      << endl;
    return 0.0;
  }

  BinaryClassParmsPtr  binaryParms = BinaryParmsList ()->LookUp (class1, class2);
  if  (!binaryParms)
  {
    cerr << endl << endl 
      << "ModelParamOldSVM::C_Param   ***ERROR***    No entry for Class[" << class1->Name () << ", " << class2->Name () << "]" << endl
      << endl;
    return 0.0;
  }
  else
  {
    return binaryParms->C ();
  }
}


void  ModelParamOldSVM::EncodingMethod (SVM_EncodingMethod _encodingMethod)
{
  svmParameters->EncodingMethod (_encodingMethod);
  ModelParam::EncodingMethod ((EncodingMethodType)_encodingMethod);
}



void  ModelParamOldSVM::EncodingMethod (EncodingMethodType _encodingMethod)
{
  svmParameters->EncodingMethod ((SVM_EncodingMethod)_encodingMethod);
  ModelParam::EncodingMethod (_encodingMethod);
}


void  ModelParamOldSVM::Gamma (double _gamma)
{
  ModelParam::Gamma (_gamma);
  svmParameters->Gamma (_gamma);
}



void  ModelParamOldSVM::Gamma_Param (double _gamma)
{
  ModelParam::Gamma (_gamma);
  svmParameters->Gamma (_gamma);
}


void  ModelParamOldSVM::KernalType (SVM_KernalType   _kernalType)
{
  svmParameters->KernalType (_kernalType);
}




kkint32 ModelParamOldSVM::NumOfFeaturesAfterEncoding (FileDescPtr  fileDesc) const
{
  return  svmParameters->NumOfFeaturesAfterEncoding (fileDesc);
}



void  ModelParamOldSVM::MachineType (SVM_MachineType  _machineType)
{
  svmParameters->MachineType (_machineType);
}



void  ModelParamOldSVM::SamplingRate (float _samplingRate)
{
  svmParameters->SamplingRate (_samplingRate);
}


void  ModelParamOldSVM::SelectedFeatures (const FeatureNumList&  _selectedFeatures)
{
  svmParameters->SelectedFeatures (_selectedFeatures);
  ModelParam::SelectedFeatures (_selectedFeatures);
}


FeatureNumListConstPtr   ModelParamOldSVM::SelectedFeatures () const
{
  return svmParameters->SelectedFeatures ();
}


void  ModelParamOldSVM::SelectionMethod (SVM_SelectionMethod  _selectionMethod)
{
  svmParameters->SelectionMethod (_selectionMethod);
}



SVMparamPtr  ModelParamOldSVM::SvmParameters ()  const
{
  return  svmParameters;
}



float  ModelParamOldSVM::A_Param ()  const
{
  return svmParameters->A_Param ();
}



double  ModelParamOldSVM::C_Param ()  const
{
  return svmParameters->C_Param ();
}



double  ModelParamOldSVM::Gamma () const 
{
  return svmParameters->Gamma ();
}



bool  ModelParamOldSVM::UseProbabilityToBreakTies ()  const
{
  return svmParameters->UseProbabilityToBreakTies ();
}



SVM_KernalType  ModelParamOldSVM::KernalType () const 
{
  return svmParameters->KernalType ();
}



SVM_MachineType  ModelParamOldSVM::MachineType () const 
{
  return svmParameters->MachineType ();
}


const svm_parameter&  ModelParamOldSVM::Param () const 
{
  return svmParameters->Param ();
}


float  ModelParamOldSVM::SamplingRate () const 
{
  return svmParameters->SamplingRate ();
}


SVM_SelectionMethod ModelParamOldSVM::SelectionMethod () const 
{
  return svmParameters->SelectionMethod ();
}



void  ModelParamOldSVM::ParseCmdLine (KKStr   _cmdLineStr,
                                      bool&   _validFormat
                                     )
{
  _validFormat = true;

  //DecodeParamStr (_cmdLineStr, param);

  KKStr  field (_cmdLineStr.ExtractToken (" \t\n\r"));
  KKStr  value;


  while  (!field.Empty ()  &&  _validFormat)
  {
    if  (field[0] != '-')  
    {
      log.Level (-1) << "ModelParam::ParseCmdLine  *** Invalid Parameter["
        << field << "] ***"
        << endl;
      _validFormat = false;
      break;
    }

    // See if next field is a Switch field or a parameter.
    _cmdLineStr.TrimLeft (" \t\n\r");
    value = "";
    if  (_cmdLineStr.Len () > 0)
    {
      if  (_cmdLineStr[0] != '-')
        value = _cmdLineStr.ExtractToken (" \t\n\r");
    }

    field.Upper ();
    KKStr valueUpper (value);

    valueUpper.Upper ();

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

    field = _cmdLineStr.ExtractToken (" \t\n\r");
  }


  // Since this class is a special case that handles the old SvmParam paradigm we need
  // to update the local Model:: parameters with those that were updated in 'SvmParam'.
  ModelParam::EncodingMethod ((ModelParam::EncodingMethodType)svmParameters->EncodingMethod     ());
  
  ModelParam::A_Param  (svmParameters->A_Param ());
  ModelParam::C_Param  (svmParameters->C_Param ());
  ModelParam::Gamma    (svmParameters->Gamma   ());

  ValidParam (_validFormat);
}  /* ParseCmdLine */




void  ModelParamOldSVM::ParseCmdLineParameter (const KKStr&  parameter,
                                               const KKStr&  value,
                                               bool&         parameterUsed
                                              )
{
  bool  validFormat = true;
  svmParameters->ParseCmdLineParameter (parameter, value, parameterUsed, validFormat);
  if  (!validFormat)
  {
    log.Level (-1) << endl << endl
      << "ModelParamOldSVM::ParseCmdLineParameter  ***ERROR***  Invalid Parameters" << endl
      << "      Parameter[" << parameter << "]     Value[" << value << "]." << endl
      << endl;
    ValidParam (false);
  }
}  /* ParseCmdLineParameter */




/**
 * @brief Convert a svm_parameter struct to a cmdline str.
*/
KKStr   ModelParamOldSVM::SvmParamToString (const svm_parameter&  _param)  const
{
  log.Level (60) << "ModelParamOldSVM::SvmParamToString Entered." << endl;

  KKStr  cmdStr (300);

  cmdStr << _param.ToCmdLineStr ();

  return  cmdStr;
}  /* SvmParamToString */




/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParamOldSVM::ToCmdLineStr () const
{
  log.Level (60) << "ModelParamOldSVM::ToCmdLineStr - Entered." << endl;

  KKStr  cmdStr (300);

  cmdStr = svmParameters->ToString ();
  return  cmdStr;
}  /* ToString */




void  ModelParamOldSVM::WriteSpecificImplementationXML (ostream&  o)  const
{
  log.Level (20) << "ModelParamOldSVM::WriteSpecificImplementationXML to XML to ostream." << endl;
  o << "<ModelParamOldSVM>" << std::endl;
  svmParameters->WriteXML (o);
  o << "</ModelParamOldSVM>" << endl;
}  /* WriteSpecificImplementationXML */





void  ModelParamOldSVM::ReadSpecificImplementationXML (istream&     i,
                                                       FileDescPtr  fileDesc
                                                      )
{
  log.Level (20) << "ModelParamOldSVM::ReadSpecificImplementationXML file." << endl;

  char  buff[20480];
  
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
    field.Upper ();

    if  (field.EqualIgnoreCase ("</ModelParamOldSVM>"))
      break;

    if  (field.EqualIgnoreCase ("<SVMparam>"))
    {
      svmParameters->ReadXML (i, fileDesc);
    }
  }
}  /* ReadSpecificImplementationXML */



BinaryClassParmsPtr   ModelParamOldSVM::GetBinaryClassParms (MLClassPtr       class1,
                                                             MLClassPtr       class2
                                                            )  const
{
  return svmParameters->GetBinaryClassParms (class1, class2);
}



BinaryClassParmsPtr   ModelParamOldSVM::GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                                         MLClassPtr  class2
                                                                        )
{
  return svmParameters->GetParamtersToUseFor2ClassCombo (class1, class2);
}  /* GetParamtersToUSeFor2ClassCombo */



FeatureNumListConstPtr  ModelParamOldSVM::GetFeatureNums (FileDescPtr  fileDesc,
                                                          MLClassPtr   class1,
                                                          MLClassPtr   class2
                                                        )  const
{
  return svmParameters->GetFeatureNums (fileDesc, class1, class2);
}  /* GetFeatureNums */




void    ModelParamOldSVM::AddBinaryClassParms (BinaryClassParmsPtr  binaryClassParms)
{
  svmParameters->AddBinaryClassParms (binaryClassParms);
}  /* AddBinaryClassParms */




float   ModelParamOldSVM::AvgMumOfFeatures (FileDescPtr  fileDesc)
{
  return  svmParameters->AvgMumOfFeatures (fileDesc);
}  /* AvgMumOfFeatures */





void  ModelParamOldSVM::SetFeatureNums (MLClassPtr              class1,
                                        MLClassPtr              class2,
                                        FeatureNumListConstPtr  _features,
                                        float                  _weight
                                       )
{
  svmParameters->SetFeatureNums (class1, class2, _features, _weight);
}  /* SetFeatureNums */





void  ModelParamOldSVM::SetBinaryClassFields (MLClassPtr              class1,
                                              MLClassPtr              class2,
                                              const svm_parameter&    _param,
                                              FeatureNumListConstPtr  _features,
                                              float                   _weight
                                             )
{
  svmParameters->SetBinaryClassFields (class1, class2, _param, _features, _weight);
}  /* SetBinaryClassFields */





/**
 * @brief  Add a Binary parameters using svm_parametr cmd line str.
 *         Typically used by TrainingConfiguration.
*/
void  ModelParamOldSVM::AddBinaryClassParms (MLClassPtr              class1,
                                             MLClassPtr              class2,
                                             const svm_parameter&    _param,
                                             FeatureNumListConstPtr  _selectedFeatures,
                                             float                   _weight
                                            )
{
  svmParameters->AddBinaryClassParms (class1, class2, _param, _selectedFeatures, _weight);
}  /* AddBinaryClassParms */





void  ModelParamOldSVM::WriteXML (const KKStr&  varName,
                                  ostream&      o
                                 )  const
{
  XmlTag  startTag ("ModelParamOldSVM",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);

  WriteXMLFields (o);

  svmParameters->ToString ().WriteXML ("SvmParameters", o);

  XmlTag  endTag ("ModelParamOldSVM", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelParamOldSVM::ReadXML (XmlStream&      s,
                                 XmlTagConstPtr  tag,
                                 RunLog&         log
                                )
{
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    t = XmlProcessToken (t);
    if  (t)
    {
      if  (t->VarName ().EqualIgnoreCase ("SvmParameters"))
      {
        delete  svmParameters;
        KKStr  svmParameters = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());

        svmParameters = new  SVMparam  (KKStr&                  _cmdLineStr,
               FeatureNumListConstPtr  _selectedFeatures,  /**< Will make own instance; caller maintains ownership status. */
               RunLog&                 _log,
               bool&                   _validFormat
              );
      }







      const KKStr&  varName = t->VarName ();

      if  (varName.EqualIgnoreCase ("ConfigFileName1"))
        configFileName1 = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());

      else if  (varName.EqualIgnoreCase ("ConfigFileName2"))
        configFileName2 = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());

      else if  (varName.EqualIgnoreCase ("FullHierarchyMustMatch"))
        fullHierarchyMustMatch = dynamic_cast<XmlElementBoolPtr> (t)->Value ();

      else if  (varName.EqualIgnoreCase ("OtherClass"))
        otherClass = MLClass::CreateNewMLClass (*(dynamic_cast<XmlElementKKStrPtr> (t)->Value ()));

      else if  (varName.EqualIgnoreCase ("ProbFusionMethod"))
        probFusionMethod = ProbFusionMethodFromStr (*(dynamic_cast<XmlElementKKStrPtr> (t)->Value ()));
    }
    t = s.GetNextToken (log);
  }
}  /* ReadXML */

