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


#include "SVMparam.h"
#include "BinaryClassParms.h"
#include "ClassAssignments.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
using namespace KKMLL;



SVMparam::SVMparam  (KKStr&                 _cmdLineStr,
                     FeatureNumListConstPtr _selectedFeatures,
                     RunLog&                _log,
                     bool&                  _validFormat
                    ):

  binaryParmsList           (NULL),
  encodingMethod            (SVM_EncodingMethod::NoEncoding),
  fileName                  (),
  log                       (_log),
  param                     (),
  probClassPairs             (),
  samplingRate              (0.0f),
  selectedFeatures          (), 
  selectionMethod           (SVM_SelectionMethod::SelectByVoting),
  useProbabilityToBreakTies (false),
  validParam                (false)
{
  if  (_selectedFeatures)
    selectedFeatures = new FeatureNumList (*_selectedFeatures);
  ParseCmdLine (_cmdLineStr, _validFormat);
}





SVMparam::SVMparam  (RunLog&  _log):

  binaryParmsList           (NULL),
  encodingMethod            (SVM_EncodingMethod::NoEncoding),
  fileName                  (),
  log                       (_log),
  machineType               (SVM_MachineType::OneVsOne),
  param                     (),
  probClassPairs            (),
  samplingRate              (0.0f),
  selectedFeatures          (NULL), 
  selectionMethod           (SVM_SelectionMethod::SelectByVoting),
  useProbabilityToBreakTies (false),
  validParam                (false)
{
}





SVMparam::SVMparam  (const SVMparam&  _svmParam):

  binaryParmsList            (NULL),
  encodingMethod             (_svmParam.encodingMethod),
  fileName                   (_svmParam.fileName),
  log                        (_svmParam.log),
  machineType                (_svmParam.machineType),
  param                      (_svmParam.param),
  probClassPairs             (_svmParam.probClassPairs),
  samplingRate               (_svmParam.samplingRate),
  selectedFeatures           (_svmParam.selectedFeatures),
  selectionMethod            (_svmParam.selectionMethod),
  useProbabilityToBreakTies  (_svmParam.useProbabilityToBreakTies),
  validParam                 (_svmParam.validParam)
{
  if  (_svmParam.selectedFeatures)
    selectedFeatures = new FeatureNumList (*_svmParam.selectedFeatures);

  if  (_svmParam.binaryParmsList)
  {
    binaryParmsList = _svmParam.binaryParmsList->DuplicateListAndContents ();
      
    //  new BinaryClassParmsList (true, _svmParam.binaryParmsList->QueueSize ());
    //  binaryParmsList  = new BinaryClassParmsList (*(_svmParam.binaryParmsList));
  }
  param = _svmParam.param;
}



SVMparam::~SVMparam  ()
{
  delete  binaryParmsList;  binaryParmsList  = NULL;
  delete  selectedFeatures; selectedFeatures = NULL;
}



FeatureNumListConstPtr  SVMparam::SelectedFeatures (FileDescPtr  fileDesc)  const
{
  if  (!selectedFeatures)
  {
    selectedFeatures = new FeatureNumList (fileDesc);
    selectedFeatures->SetAllFeatures (fileDesc);
  }
  return  selectedFeatures;
}



kkint32  SVMparam::MemoryConsumedEstimated () const
{
  kkint32  memoryConsumedEstimated = sizeof (SVMparam) + fileName.MemoryConsumedEstimated ();
  if  (selectedFeatures)
    memoryConsumedEstimated += selectedFeatures->MemoryConsumedEstimated ();

  if  (binaryParmsList)
    memoryConsumedEstimated += binaryParmsList->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated  */



void  SVMparam::SelectedFeatures (const FeatureNumList&  _selectedFeatures)
{
  delete  selectedFeatures;
  selectedFeatures  = new FeatureNumList (_selectedFeatures);
}


void  SVMparam::SelectedFeatures  (FeatureNumListConstPtr  _selectedFeatures)
{
  delete  selectedFeatures;
  selectedFeatures  = new FeatureNumList (*_selectedFeatures);
}



void  SVMparam::ProbClassPairsInitialize (const ClassAssignments&  assignments)
{
  kkuint32  numClasses = assignments.size ();
  if  (numClasses < 1)
    return;

  kkuint32  numPairs   = numClasses * (numClasses - 1) / 2;
  probClassPairs.clear ();
  if  (binaryParmsList == NULL)
  {
    for  (kkuint32 x = 0;  x < numPairs;  ++x)
      probClassPairs.push_back (param.A);
  }
  else
  {
    for (kkuint32 class1IDX = 0;  class1IDX < (numClasses - 1);  ++class1IDX)
    {
      MLClassPtr       class1 = assignments.GetMLClass ((kkint16)class1IDX);
      for (kkuint32 class2IDX = class1IDX + 1;  class2IDX < numClasses;  ++class2IDX)
      {
        MLClassPtr       class2 = assignments.GetMLClass ((kkint16)class2IDX);
        BinaryClassParmsPtr  bcp = binaryParmsList->LookUp (class1, class2);
        if  (bcp)
          probClassPairs.push_back ((float)bcp->AParam ());
        else
          probClassPairs.push_back ((float)param.A);
      }
    }
  }
}  /* ProbClassPairsInitialize */




void  SVMparam::A_Param (float  _A)
{
  param.A = _A;
}




void  SVMparam::C_Param (double  _CC)
{
  param.C = _CC;
}



void  SVMparam::ParseCmdLineParameter (const KKStr&  parameter,
                                       const KKStr&  value,
                                       bool&         parameterUsed,
                                       bool&         _validFormat
                                      )
{
  parameterUsed = true;
  _validFormat  = true;

  KKStr  field = parameter;
  double  valueNum = value.ToDouble ();

  field.Upper ();
  KKStr valueUpper (value);
  valueUpper.Upper ();

  param.ProcessSvmParameter (parameter,value, valueNum, parameterUsed);
  if  (parameterUsed)
    return;

  parameterUsed = true;

  if ((field == "-ENCODE"))
  {
    encodingMethod = EncodingMethodFromStr (valueUpper);
  }

  else if  (field == "-MT")
  {
    machineType = MachineTypeFromStr (valueUpper);
    if  (machineType == SVM_MachineType::MachineType_NULL)
    {
      _validFormat = false;
      log.Level (-1) << endl 
        << "SVMparam::ParseCmdLineParameter     *** ERROR ***" << endl
        << "                Invalid -MT Parm[" << value << "]" << endl
        << endl;
    }
  }

  else if  ((field == "-SM")  ||  (field == "-SELECTIONMETHOD"))
  {
    selectionMethod  = SelectionMethodFromStr (valueUpper);
    if  (selectionMethod == SVM_SelectionMethod::SelectionMethod_NULL)
    {
      log.Level (-1) << endl
        << "SVMparam::ParseCmdLineParameter    *** ERROR ***"  << endl
        << "            Invalid SelectionMethod (-SM)[" << value << "]." << endl
        << endl;
      _validFormat = false;
    }
  }

  else if  ((field == "-SR")  ||  (field == "-SAMPLINGRATE"))
  {
    samplingRate = float (atof (value.Str ()));
  }


  else if  ((field == "-UP")  ||  (field == "-USEPROBABILITY"))
  {
    useProbabilityToBreakTies = true;
  }


  else
  {
    parameterUsed = false;
  }
}  /* ParseCmdLineParameter */




void  SVMparam::ParseCmdLine (KKStr  _cmdLineStr,
                              bool&   _validFormat
                             )
{
  _validFormat = true;

  {
    // Will create a svm_parameter object from _cmdLineStr,  the paramters that 
    // are not specific to svm_parameter will be left over and then we can 
    // process them.
    svm_parameter  tempParam (_cmdLineStr);
    param = tempParam;
  }

  //DecodeParamStr (_cmdLineStr, param);

  machineType = SVM_MachineType::OneVsOne;

  useProbabilityToBreakTies = false;

  KKStr  field (_cmdLineStr.ExtractToken (" \t\n\r"));
  KKStr  value;

  double  valueNum;

  while  (!field.Empty ()  &&  _validFormat)
  {
    if  (field[(kkint16)0] != '-')
    {
      log.Level (-1) << "SVMparam::ParseCmdLine  *** Invalid Parameter["
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
      if  (_cmdLineStr[(kkint16)0] != '-')
        value = _cmdLineStr.ExtractToken (" \t\n\r");
    }

    valueNum = atof (value.Str ()); 

    field.Upper ();
    KKStr valueUpper (value);

    valueUpper.Upper ();

    bool  parameterUsed = false;
    ParseCmdLineParameter (field, value, parameterUsed, _validFormat);
    if  (!parameterUsed)
    {
      log.Level (-1) << "SVMparam::ParseCmdLine - Invalid Parameter["  
        << field << "]  Value[" << value << "]."
        << endl;
      _validFormat = false;
      break;
    }
    field = _cmdLineStr.ExtractToken (" \t\n\r");
  } 


  validParam = _validFormat;
}  /* ParseCmdLine */




/**
 * @brief Convert a svm_parameter struct to a cmdline str.
*/
KKStr   SVMparam::SvmParamToString (const svm_parameter&  _param)  const
{
  log.Level (60) << "SVMparam::SvmParamToString Entered." << endl;

  KKStr  cmdStr (300);

  cmdStr << _param.ToCmdLineStr ();

  return  cmdStr;
}  /* SvmParamToString */




/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   SVMparam::ToString () const
{
  log.Level (60) << "SVMparam::ToString - Entered." << endl;

  KKStr  cmdStr (300);

  cmdStr = SvmParamToString (param);

  if  (encodingMethod != SVM_EncodingMethod::NoEncoding)
    cmdStr << "  -Encode " + EncodingMethodToStr (encodingMethod);

  cmdStr << "  -MT " << MachineTypeToStr (machineType);
  cmdStr << "  -SM " << SelectionMethodToStr (selectionMethod);

  if  (machineType == SVM_MachineType::BoostSVM)
  {
    cmdStr << "  -SamplingRate " << samplingRate;
  }

  if  (useProbabilityToBreakTies)
    cmdStr << "  -UseProbability";

  return  cmdStr;
}  /* ToString */






void  SVMparam::Save (KKStr&  _fileName,
                      bool&    _successful
                     )
{
  log.Level (20) << "SVMparam::Save - File[" << fileName << "]." << endl;

  _successful = true;

  fileName = _fileName;

  ofstream outFile (fileName.Str ());

  WriteXML (outFile);

  outFile.close ();
}  /* Save */




void  SVMparam::WriteXML (ostream&  o)  const
{
  log.Level (20) << "SVMparam::Save to XML to ostream." << endl;

  o << "<SVMparam>" << endl;

  o << "<CmdLine>"  << "\t" << ToString ().QuotedStr () << "\t" << "</CmdLine>" << endl;

  if  (selectedFeatures)
    o << "<SelectedFeatures>" << "\t" << selectedFeatures->ToString () << "\t" << "</SelectedFeatures>" << endl;

  if  (binaryParmsList)
    binaryParmsList->WriteXML (o);

  o << "</SVMparam>" << endl;
}  /* WriteXML */



void  SVMparam::ReadXML (istream&     i,
                         FileDescPtr  fileDesc
                        )
{
  log.Level (20) << "SVMparam::Read from XML file." << endl;

  char  buff[20480];
  
  delete  binaryParmsList;  binaryParmsList = NULL;

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
    field.Upper ();

    if  (field == "</SVMPARAM>")
      break;

    else if  (field == "<CMDLINE>")
    {
      bool  validFormat;
      KKStr  cmdLine = ln.ExtractQuotedStr ("\n\r\t", 
                                             true      // true = decode escape characters
                                            );
      ParseCmdLine (cmdLine, validFormat);
    }

    else if  (field == "<SELECTEDFEATURES>")
    {
      delete  selectedFeatures;
      KKStr  featureNumStr = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
      selectedFeatures = FeatureNumList::ExtractFeatureNumsFromStr (featureNumStr);
    }

    else if  (field == "<BINARYCLASSPARMSLIST>")
    {
      binaryParmsList = BinaryClassParmsList::CreateFromXML (i, fileDesc, log);
    }
  }
}  /* ReadXML */



void  SVMparam::ReadXML (FILE*        i,
                         FileDescPtr  fileDesc
                        )
{
  log.Level (20) << "SVMparam::Read from XML file." << endl;

  char  buff[10240];
  
  delete  binaryParmsList;  binaryParmsList = NULL;

  while  (fgets (buff, sizeof (buff), i))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", 
                                         true      // true = decode escape characters
                                        );
    field.Upper ();

    if  (field == "</SVMPARAM>")
    {
      break;
    }

    else if  (field == "<CMDLINE>")
    {
      bool  validFormat;
      KKStr  cmdLine = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
      ParseCmdLine (cmdLine, validFormat);
    }

    else if  (field == "<SELECTEDFEATURES>")
    {
      delete  selectedFeatures;
      KKStr  featureNumStr = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
      selectedFeatures = FeatureNumList::ExtractFeatureNumsFromStr (featureNumStr);
    }

    else if  (field == "<BINARYCLASSPARMSLIST>")
    {
      binaryParmsList = BinaryClassParmsList::CreateFromXML (i, fileDesc, log);
    }
  }
}  /* ReadXML */





void  SVMparam::Load (KKStr&            _fileName,
                      FileDescPtr       _fileDesc,
                      ClassAssignments& _assignments,
                      bool&             _successful
                     )
{
  log.Level (10) << "SVMparam::Load - File[" << _fileName << "]." << endl;

  _successful = true;

  fileName = _fileName;
  FILE*  inputFile = osFOPEN (fileName.Str (), "r");

  if  (!inputFile)
  {
    log.Level (-1) << "SVMparam::Load      *** ERROR ***" << endl;
    log.Level (-1) << "                Could Not Open File[" << fileName << "]." << endl;
    _successful = false;
    return;
  }

  this->ReadXML (inputFile, _fileDesc);
   
  fclose (inputFile);
}  /* Load */





BinaryClassParmsPtr   SVMparam::GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                                 MLClassPtr  class2
                                                                )
{
  BinaryClassParmsPtr  twoClassComboParms = NULL;

  if  (binaryParmsList == NULL)
  {
    binaryParmsList = new BinaryClassParmsList (true);
    twoClassComboParms = NULL;
  }
  else
  {
    twoClassComboParms = binaryParmsList->LookUp (class1, class2);
  }

  if  (!twoClassComboParms)
  {
    twoClassComboParms = new BinaryClassParms (class1, class2, param, selectedFeatures, 1.0f);
    binaryParmsList->PushOnBack (twoClassComboParms);
  }

  return  twoClassComboParms;
}  /* GetParamtersToUSeFor2ClassCombo */





BinaryClassParmsPtr   SVMparam::GetBinaryClassParms (MLClassPtr       class1,
                                                     MLClassPtr       class2
                                                    )
{
  if  (binaryParmsList == NULL)
    return NULL;
  else
    return binaryParmsList->LookUp (class1, class2);
}  /* GetBinaryClassParms */




FeatureNumListConstPtr  SVMparam::GetFeatureNums ()  const
{
  return  selectedFeatures;
}



FeatureNumListConstPtr  SVMparam::GetFeatureNums (FileDescPtr  fileDesc)  const
{
  if  (selectedFeatures == NULL)
    return SelectedFeatures (fileDesc);
  else
    return selectedFeatures;
}



FeatureNumListConstPtr SVMparam::GetFeatureNums (FileDescPtr  fileDesc,
                                                 MLClassPtr   class1,
                                                 MLClassPtr   class2
                                                )  const
{
  if  (!binaryParmsList)
  {
    // This configuration file does not specify feature selection by binary classes,
    // so return the general one specified for model.
    return  SelectedFeatures (fileDesc);
  }

  BinaryClassParmsPtr  twoClassComboParm =  binaryParmsList->LookUp (class1, class2);
  if  (!twoClassComboParm)
    return  SelectedFeatures (fileDesc);
  else
    return  twoClassComboParm->SelectedFeaturesFD  (fileDesc);
}  /* GetFeatureNums */




void   SVMparam::AddBinaryClassParms (BinaryClassParmsPtr  binaryClassParms)
{
  if  (!binaryParmsList)
    binaryParmsList = new BinaryClassParmsList (true);

  binaryParmsList->PushOnBack (binaryClassParms);
}  /* AddBinaryClassParms */



float   SVMparam::AvgMumOfFeatures (FileDescPtr fileDesc)  const
{
  float  avgNumOfFeatures = 0.0f;

  if  ((machineType == SVM_MachineType::BinaryCombos)  &&  (binaryParmsList))
    avgNumOfFeatures = binaryParmsList->FeatureCountNet (fileDesc);

  if  (avgNumOfFeatures == 0.0f)
    avgNumOfFeatures = (float)SelectedFeatures (fileDesc)->NumOfFeatures ();

  return  avgNumOfFeatures;
}  /* AvgMumOfFeatures */



/**
 *@brief  Returns back the class weighted average number of features per training example.
 *@details  Will calculate the average number of features per training example. For each
 *          binary class combination, multiplies the number of training examples for that pair 
 *          by the number of features for that pair.  the sum of all class pairs are then 
 *          divided by the total number of examples.
 *@param[in] trainExamples  List of training examples that were or are to be used to train with.
 */
float  SVMparam::AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const
{
  FileDescPtr  fileDesc = trainExamples->FileDesc ();
  if  (!selectedFeatures)
  {
    // The method "SelectedFeatures" will set 'selectedFeatures' to a new instance using 'fileDesc'.
    // This way we make sure that the variable exists.
    SelectedFeatures (fileDesc);
  }
  if  (machineType == SVM_MachineType::BinaryCombos)
  {
    kkint32 totalNumFeaturesUsed = 0;
    kkint32 toatlNumExamples     = 0;
    ClassStatisticListPtr  stats = trainExamples->GetClassStatistics ();
    if  (!stats)
    {
      return (float)selectedFeatures->NumOfFeatures ();
    }

    kkuint32  idx1 = 0;
    kkuint32  idx2 = 0;
    for  (idx1 = 0;  idx1 < (stats->size() - 1);  idx1++)
    {
      ClassStatisticPtr  class1Stats = stats->IdxToPtr (idx1);
      MLClassPtr  class1    = class1Stats->MLClass ();
      kkuint32    class1Qty = class1Stats->Count ();

      for  (idx2 = idx1 + 1;  idx2 < (stats->size());  idx2++)
      {
        ClassStatisticPtr  class2Stats = stats->IdxToPtr (idx2);
        MLClassPtr  class2    = class2Stats->MLClass ();
        kkuint32    class2Qty = class2Stats->Count ();

        kkint32  numFeaturesThisCombo = 0;
        FeatureNumListConstPtr  cpfn = GetFeatureNums (fileDesc, class1, class2);
        if  (cpfn)
          numFeaturesThisCombo = cpfn->NumSelFeatures ();

        kkint32  numExamplesThisCombo = class1Qty + class2Qty;

        totalNumFeaturesUsed += numFeaturesThisCombo * numExamplesThisCombo;
        toatlNumExamples     += numExamplesThisCombo;
      }
    }
    delete  stats;  stats = NULL;
    return  (float)totalNumFeaturesUsed / (float)toatlNumExamples;
  }
  else
  {
    return  (float)selectedFeatures->NumOfFeatures ();
  }
}  /* AvgMumOfFeatures */



kkint32  SVMparam::NumOfFeaturesAfterEncoding (FileDescPtr  fileDesc)  const
{
  if  (!selectedFeatures)
  {
    // The method "SelectedFeatures" will set 'selectedFeatures' to a new instance using 'fileDesc'.
    // This way we make sure that the variable exists.
    SelectedFeatures (fileDesc);
  }

  kkint32 z;
  kkint32 numFeaturesAfterEncoding = 0;
  kkint32 numOfFeaturesSelected = selectedFeatures->NumOfFeatures ();

  switch (EncodingMethod ())
  {
  case  SVM_EncodingMethod::BinaryEncoding:
    for  (z = 0; z < numOfFeaturesSelected; z++)
    {
      kkint32  fieldNum = (*selectedFeatures)[z];
      if  ((fileDesc->Type (fieldNum) == AttributeType::NominalAttribute)  ||
           (fileDesc->Type (fieldNum) == AttributeType::SymbolicAttribute)
          )
        numFeaturesAfterEncoding += fileDesc->Cardinality (fieldNum, log);
      else
        numFeaturesAfterEncoding ++;
    }
    break;

  case SVM_EncodingMethod::ScaledEncoding:
  case SVM_EncodingMethod::NoEncoding:
  default:
    //numFeaturesAfterEncoding = fileDesc->NumOfFields ( );
    numFeaturesAfterEncoding = selectedFeatures->NumOfFeatures ();
    break;
  }

  return  numFeaturesAfterEncoding;
}  /* NumOfFeaturesAfterEncoding */




void  SVMparam::SetFeatureNums (MLClassPtr              class1,
                                MLClassPtr              class2,
                                FeatureNumListConstPtr  _features,
                                float                   _weight
                               )
{
  if  (!binaryParmsList)
  {
    if  (_weight < 0)
      _weight = 1;
    AddBinaryClassParms (class1, class2, param, _features, _weight);
  }
  else
  {
    BinaryClassParmsPtr  binaryParms = binaryParmsList->LookUp (class1, class2);
    if  (binaryParms)
    {
      if  (_weight < 0)
        _weight = binaryParms->Weight ();
      binaryParms->SelectedFeatures (_features);
    }
    else
    {
      if  (_weight < 0)
        _weight = 1.0f;
      AddBinaryClassParms (class1, class2, param, _features, _weight);
    }
  }
}  /* SetFeatureNums */



void  SVMparam::SetFeatureNums (const FeatureNumList&  _features)
{
  delete  selectedFeatures;
  selectedFeatures = new FeatureNumList (_features);
}  /* SetFeatureNums */




void  SVMparam::SetFeatureNums (FeatureNumListConstPtr  _features)
{
  delete  selectedFeatures;
  selectedFeatures = new FeatureNumList (*_features);
}  /* SetFeatureNums */



double  SVMparam::C_Param (MLClassPtr  class1,
                           MLClassPtr  class2
                          )  const
{
  if  (!binaryParmsList)
  {
    cerr << endl << endl
      << "SVMparam::C_Param   ***ERROR***     'binaryParmsList'  is not defined." << endl
      << endl;
    return 0.0;
  }

  BinaryClassParmsPtr  binaryParms = binaryParmsList->LookUp (class1, class2);
  if  (!binaryParms)
  {
    cerr << endl << endl 
      << "SVMparam::C_Param   ***ERROR***    No entry for Class[" << class1->Name () << ", " << class2->Name () << "]" << endl
      << endl;
    return 0.0;
  }
  else
  {
    return binaryParms->C ();
  }
}




void  SVMparam::C_Param (MLClassPtr  class1,
                         MLClassPtr  class2,
                         double      cParam
                        )
{
  if  (!binaryParmsList)
  {
    cerr << endl << endl
      << "SVMparam::C_Param   ***ERROR***     'binaryParmsList'  is not defined." << endl
      << endl;
    return;
  }

  BinaryClassParmsPtr  binaryParms = binaryParmsList->LookUp (class1, class2);
  if  (!binaryParms)
  {
    svm_parameter binaryParam = param;
    binaryParam.C = C_Param ();
    AddBinaryClassParms (class1, class2, binaryParam, SelectedFeatures (), 1.0f);
  }
  else
  {
    binaryParms->C (cParam);
  }
}  /* C_Param */







void  SVMparam::SetBinaryClassFields (MLClassPtr              class1,
                                      MLClassPtr              class2,
                                      const svm_parameter&    _param,
                                      FeatureNumListConstPtr  _features,
                                      float                   _weight
                                     )
{
  if  (!binaryParmsList)
  {
    AddBinaryClassParms (class1, class2, _param, _features, _weight);
  }
  else
  {
    BinaryClassParmsPtr  binaryParms = binaryParmsList->LookUp (class1, class2);
    if  (binaryParms)
    {
      binaryParms->Param            (_param); 
      binaryParms->SelectedFeatures (_features);
      binaryParms->Weight           (_weight);
    }
    else
    {
      AddBinaryClassParms (class1, class2, _param, _features, _weight);
    }
  }
}  /* SetBinaryClassFields */





/**
 * @brief  Add a Binary parameters using svm_parametr cmd line str.
 *         Typically used by TrainingConfiguration.
*/
void  SVMparam::AddBinaryClassParms (MLClassPtr              class1,
                                     MLClassPtr              class2,
                                     const svm_parameter&    _param,
                                     FeatureNumListConstPtr  _selectedFeatures,  /**< We will NOT be taking ownership; we will make our own copy. */
                                     float                   _weight
                                    )
{
  AddBinaryClassParms (new BinaryClassParms (class1, class2, _param, _selectedFeatures, _weight));
}  /* AddBinaryClassParms */




KKStr  KKMLL::EncodingMethodToStr (SVM_EncodingMethod  encodingMethod)
{
  if  (encodingMethod == SVM_EncodingMethod::BinaryEncoding)
    return  "Binary";

  else if  (encodingMethod == SVM_EncodingMethod::ScaledEncoding)
    return  "Scale";

  else
    return  "None";
}  /* EncodingMethodToStr */


        
SVM_EncodingMethod  KKMLL::EncodingMethodFromStr (const KKStr&  encodingMethodStr)
{
  KKStr  encodingMethodUpper = encodingMethodStr.ToUpper ();

  if  ((encodingMethodUpper == "BINARY")  ||  (encodingMethodUpper == "BIN"))
     return  SVM_EncodingMethod::BinaryEncoding;

  if  (encodingMethodUpper == "SCALE")
     return  SVM_EncodingMethod::ScaledEncoding;

  if  (encodingMethodUpper == "NONE")
    return  SVM_EncodingMethod::NoEncoding;

  return  SVM_EncodingMethod::Encoding_NULL;
}  /* EncodingMethodFromStr */



KKStr  KKMLL::KernalTypeToStr (SVM_KernalType  kernalType)
{
  switch  (kernalType)
  {
  case  SVM_KernalType::KT_Linear:      return "Linear";
  case  SVM_KernalType::KT_Polynomial:  return "Polynomial";
  case  SVM_KernalType::KT_RBF:         return "RBF";
  default: return "UnKnown";
  }

  return "";
}  /* KernalTypeToStr */



SVM_KernalType  KKMLL::KernalTypeFromStr (const KKStr&  kernalTypeStr)
{
  KKStr kernalTypeUpper = kernalTypeStr;
  kernalTypeUpper.Upper ();

  if  ((kernalTypeUpper == "LINEAR")  ||  
       (kernalTypeUpper == "LIN")     ||
       (kernalTypeUpper == "L")
      )
    return  SVM_KernalType::KT_Linear;

  if  ((kernalTypeUpper == "POLYNOMIAL")  ||
       (kernalTypeUpper == "POLY")        ||
       (kernalTypeUpper == "P")           ||
       (kernalTypeUpper == "PN")
       )
    return  SVM_KernalType::KT_Polynomial;
  

  if  ((kernalTypeUpper == "RBF")                 ||
       (kernalTypeUpper == "R")                   ||
       (kernalTypeUpper == "RADIALBASIS")         ||
       (kernalTypeUpper == "RADIALBASISFUNC")     ||
       (kernalTypeUpper == "RADIALBASISFUNCTION")
      )
    return  SVM_KernalType::KT_RBF;

  return  SVM_KernalType::KT_Linear;
}  /* KernalTypeToStr */




KKStr  KKMLL::MachineTypeToStr (SVM_MachineType  machineType)
{
  if  (machineType == SVM_MachineType::OneVsOne)
    return  "OneVsOne";

  else if  (machineType == SVM_MachineType::OneVsAll)
    return  "OneVsAll";

  else if  (machineType == SVM_MachineType::BinaryCombos)
    return "Binary";

  else if  (machineType == SVM_MachineType::BoostSVM)
    return "BoostSVM";

  return "";
}  /* MachineTypeToStr */





SVM_MachineType  KKMLL::MachineTypeFromStr (const KKStr&  machineTypeStr)
{
  KKStr  machineTypeUpper = machineTypeStr.ToUpper ();

  if  (machineTypeUpper == "ONEVSONE")
    return  SVM_MachineType::OneVsOne;

  else  if  (machineTypeUpper == "ONEVSALL")
    return  SVM_MachineType::OneVsAll;

  else  if  (machineTypeUpper == "BINARY")
    return  SVM_MachineType::BinaryCombos;

  else  if  (machineTypeUpper == "BOOSTSVM")
    return  SVM_MachineType::BoostSVM;

  return  SVM_MachineType::MachineType_NULL;
}  /* MachineTypeFromStr */
 



KKStr  KKMLL::SelectionMethodToStr (SVM_SelectionMethod  selectionMethod)
{
  if  (selectionMethod == SVM_SelectionMethod::SelectByVoting)
    return  "Voting";

  if  (selectionMethod == SVM_SelectionMethod::SelectByProbability)
    return  "Probability";

  return "";
}  /* SelectionMethodToStr */




SVM_SelectionMethod  KKMLL::SelectionMethodFromStr (const KKStr&  selectionMethodStr)
{
  KKStr  selectionMethodUpper = selectionMethodStr.ToUpper ();

  if  ((selectionMethodUpper == "VOTE")  ||  (selectionMethodUpper == "VOTING"))
    return  SVM_SelectionMethod::SelectByVoting;

  if  ((selectionMethodUpper == "P")     ||  
       (selectionMethodUpper == "PROB")  ||
       (selectionMethodUpper == "PROBABILITY")
      )
    return  SVM_SelectionMethod::SelectByProbability;

  return  SVM_SelectionMethod::SelectionMethod_NULL;

}  /* SelectionMethodFromStr */

