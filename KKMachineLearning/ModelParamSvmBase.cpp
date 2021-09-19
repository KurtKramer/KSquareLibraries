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

#include "ModelParamSvmBase.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
using namespace KKMLL;

#include  "svm2.h"
using namespace  SVM289_MFS;



ModelParamSvmBase::ModelParamSvmBase ():

  ModelParam (),
  svmParam   ()
{
}



ModelParamSvmBase::ModelParamSvmBase (SVM_Type     _svm_type,
                                      Kernel_Type  _kernelType,
                                      double       _cost,
                                      double       _gamma
                                     ):

  ModelParam (),
  svmParam   ()
{
  svmParam.SvmType    (_svm_type);
  svmParam.KernalType (_kernelType);
  svmParam.Cost       (_cost);
  svmParam.Gamma      (_gamma);
}



ModelParamSvmBase::ModelParamSvmBase (const ModelParamSvmBase&  _param):
  ModelParam (_param),
  svmParam   (_param.svmParam)
{
}



ModelParamSvmBase::~ModelParamSvmBase ()
{
}



ModelParamSvmBasePtr  ModelParamSvmBase::Duplicate ()  const
{
  return  new ModelParamSvmBase (*this);
}  /* Duplicate */



void  ModelParamSvmBase::Cost (double _cost)
{
  svmParam.Cost (_cost);
  ModelParam::Cost (_cost);
}



void  ModelParamSvmBase::Gamma (double _gamma)
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
                                                bool&         parameterUsed,
                                                RunLog&       log
                                               )
{
  log.Level (70) << "ModelParamSvmBase::ParseCmdLineParameter   parameter: " << parameter << "  value: " << parameter << endl;
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
  KKStr  cmdStr (300U);
  cmdStr = ModelParam::ToCmdLineStr () + " " + svmParam.ToCmdLineStr ();
  return  cmdStr;
}  /* ToCmdLineStr */



void  ModelParamSvmBase::WriteXML (const KKStr&  varName,
                                   ostream&      o
                                  )  const
{
  XmlTag  startTag ("ModelParamOldSVM",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteXMLFields (o);
  
  svmParam.ToTabDelStr ().WriteXML ("SvmParam", o);

  XmlTag  endTag ("ModelParamOldSVM", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  ModelParamSvmBase::ReadXML (XmlStream&      s,
                                  XmlTagConstPtr  tag,
                                  VolConstBool&   cancelFlag,
                                  RunLog&         log
                                 )
{
  log.Level(50) << "ModelParamSvmBase::ReadXML   tag->name: " << tag->Name() << std::endl;
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    t = ReadXMLModelParamToken (t);
    if  (t)
    {
      if  (t->VarName ().EqualIgnoreCase ("SvmParam"))
      {
        svmParam.ParseTabDelStr (*(dynamic_cast<XmlElementKKStrPtr> (t)->Value ()));
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */

XmlFactoryMacro(ModelParamSvmBase)
