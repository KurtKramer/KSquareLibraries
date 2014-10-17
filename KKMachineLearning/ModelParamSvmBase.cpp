#include  "FirstIncludes.h"
#include  <stdio.h>
#include  <fstream>
#include  <string>
#include  <iostream>
#include  <vector>
#include  "MemoryDebug.h"
using namespace std;


#include  "KKBaseTypes.h"
#include  "OSservices.h"
#include  "RunLog.h"
using namespace KKB;


#include  "ModelParamSvmBase.h"
#include  "FileDesc.h"
#include  "MLClass.h"
using namespace KKMachineLearning;


#include  "svm2.h"
using namespace  SVM289_MFS;



ModelParamSvmBase::ModelParamSvmBase  (FileDescPtr  _fileDesc,
                                       RunLog&      _log
                                      ):

  ModelParam (_fileDesc, _log),
  svmParam   ()
{
}




ModelParamSvmBase::ModelParamSvmBase  (FileDescPtr  _fileDesc,
                                       SVM_Type     _svm_type,
                                       Kernel_Type  _kernelType,
                                       double       _cost,
                                       double       _gamma,
                                       RunLog&      _log
                                      ):

  ModelParam (_fileDesc, log),
  svmParam   ()
{
  svmParam.SvmType    (_svm_type);
  svmParam.KernalType (_kernelType);
  svmParam.Cost       (_cost);
  svmParam.Gamma      (_gamma);
}








ModelParamSvmBase::ModelParamSvmBase  (const ModelParamSvmBase&  _param):
  ModelParam (_param),
  svmParam   (_param.svmParam)
{
}



ModelParamSvmBase::~ModelParamSvmBase  ()
{
}


ModelParamSvmBasePtr  ModelParamSvmBase::Duplicate ()  const
{
  return  new ModelParamSvmBase (*this);
}  /* Duplicate */



void  ModelParamSvmBase::Cost   (double _cost)
{
  svmParam.Cost (_cost);
  ModelParam::Cost (_cost);
}


void  ModelParamSvmBase::Gamma  (double _gamma)
{
  svmParam.Gamma (_gamma);
  ModelParam::Gamma (_gamma);
}


double  ModelParamSvmBase::Cost ()  const 
{
  return  svmParam.Cost ();
}


double  ModelParamSvmBase::Gamma ()  const 
{
  return  svmParam.Gamma ();
}



void  ModelParamSvmBase::ParseCmdLineParameter (const KKStr&  parameter,
                                                const KKStr&  value,
                                                bool&         parameterUsed
                                               )
{
  svmParam.ProcessSvmParameter (parameter, value, parameterUsed);
}  /* ParseCmdLineParameter */




/**
 @brief  // Will get called after the entire parameter string has been processed.
 */
void   ModelParamSvmBase::ParseCmdLinePost ()
{
  svmParam.Gamma (Gamma ());
  svmParam.Cost  (Cost  ());
}




/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParamSvmBase::ToCmdLineStr () const
{
  log.Level (60) << "ModelParamSvmBase::ToCmdLineStr - Entered." << endl;
  KKStr  cmdStr (300);
  cmdStr = ModelParam::ToCmdLineStr () + " " + svmParam.ToCmdLineStr ();
  return  cmdStr;
}  /* ToCmdLineStr */




void  ModelParamSvmBase::WriteSpecificImplementationXML (ostream&  o)  const
{
  log.Level (20) << "ModelParamSvmBase::WriteSpecificImplementationXML XML to ostream." << endl;

  o << "<ModelParamSvmBase>" << endl;
  o << "SvmParam" << "\t" << svmParam.ToTabDelStr () << endl;
  o << "</ModelParamSvmBase>" << endl;
}  /* WriteXML */





void  ModelParamSvmBase::ReadSpecificImplementationXML (istream&  i)
{
  log.Level (20) << "ModelParamSvmBase::ReadSpecificImplementationXML from XML file." << endl;

  char  buff[20480];
  
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    if  ((ln.Len () < 1)  || (ln.SubStrPart (0, 1) == "//"))
      continue;

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true); // true = decode escape charaters
    field.Upper ();

    if  (field.EqualIgnoreCase ("<ModelParamSvmBase>"))
      continue;

    if  (field.EqualIgnoreCase ("</ModelParamSvmBase>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<ModelParam>"))
    {
      ModelParam::ReadXML (i);
    }

    else if  (field.EqualIgnoreCase ("SvmParam"))
    {
      svmParam.ParseTabDelStr (ln);
    }

    else
    {
      log.Level (-1) << endl << endl << "ModelParamSvmBase::ReadSpecificImplementationXML   ***ERROR**  Invalid Parameter[" << field << "]" << endl << endl;
      ValidParam (false);
    }
  }
}  /* ReadSpecificImplementationXML */



