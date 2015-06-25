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


#include "ModelParamDual.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
#include "TrainingConfiguration2.h"
using namespace KKMLL;



ModelParamDual::ModelParamDual  ():
  ModelParam (),
  configFileName1        (),
  configFileName2        (),
  fullHierarchyMustMatch (false),
  otherClass             (NULL),
  probFusionMethod       (pfmOr)
{
}




ModelParamDual::ModelParamDual  (const KKStr&  _configFileName1,
                                 const KKStr&  _configFileName2,
                                 bool          _fullHierarchyMustMatch
                                ):

  ModelParam             (),
  configFileName1        (_configFileName1),
  configFileName2        (_configFileName2),
  fullHierarchyMustMatch (_fullHierarchyMustMatch),
  otherClass             (NULL),
  probFusionMethod       (pfmOr)
{
}








ModelParamDual::ModelParamDual  (const ModelParamDual&  _param):
  ModelParam             (_param),
  configFileName1        (_param.configFileName1),
  configFileName2        (_param.configFileName2),
  fullHierarchyMustMatch (_param.fullHierarchyMustMatch),
  otherClass             (_param.otherClass),
  probFusionMethod       (_param.probFusionMethod)
{
}



ModelParamDual::~ModelParamDual  ()
{
}


ModelParamDual::ProbFusionMethodType  ModelParamDual::ProbFusionMethodFromStr (const KKStr& s)
{
  if  (s.EqualIgnoreCase ("And"))
    return pfmAnd;

  else if  (s.EqualIgnoreCase ("Or"))
    return pfmOr;

  else
    return pfmNULL;
}


KKStr  ModelParamDual::ProbFusionMethodToStr (ProbFusionMethodType  pfm)
{
  if  ((pfm <  0)  ||  (pfm > 2))
    return KKStr::EmptyStr ();

  switch  (pfm)
  {
    case  pfmAnd:
      return "And";
      break;

    case  pfmOr:
     return  "Or";
     break;
  }

  return  KKStr::EmptyStr ();
}




ModelParamDualPtr  ModelParamDual::Duplicate ()  const
{
  return  new ModelParamDual (*this);
}  /* Duplicate */




void  ModelParamDual::ParseCmdLineParameter (const KKStr&  parameter,
                                             const KKStr&  value,
                                             bool&         parameterUsed,
                                             RunLog&       log
                                            )
{
  if  (parameter.EqualIgnoreCase ("-ConfigFileName1") ||
       parameter.EqualIgnoreCase ("-Config1")         ||
       parameter.EqualIgnoreCase ("-cfn1")            ||
       parameter.EqualIgnoreCase ("-classifier1")     ||
       parameter.EqualIgnoreCase ("-c1")
      )
  {
    configFileName1 = value;
    parameterUsed = true;
    if  (!TrainingConfiguration2::ConfigFileExists (configFileName1))
    {
      log.Level (-1) << "ModelParamDual::ParseCmdLineParameter   ***ERROR***  Configuration File1[" << configFileName1 << "]  Does not exist." << endl;
      ValidParam (false);
    }
  }

  else 
  if  (parameter.EqualIgnoreCase ("-ConfigFileName2") ||
       parameter.EqualIgnoreCase ("-Config2")         ||
       parameter.EqualIgnoreCase ("-cfn2")            ||
       parameter.EqualIgnoreCase ("-classifier2")     ||
       parameter.EqualIgnoreCase ("-c2")
      )
  {
    configFileName2 = value;
    parameterUsed = true;
    if  (!TrainingConfiguration2::ConfigFileExists (configFileName2))
    {
      log.Level (-1) << "ModelParamDual::ParseCmdLineParameter   ***ERROR***  Cinfiguration File2[" << configFileName2 << "]  Does not exist." << endl;
      ValidParam (false);
    }
  }

  else 
  if  (parameter.EqualIgnoreCase ("-FullHierarchyMustMatch") ||
       parameter.EqualIgnoreCase ("-FHMM")
      )
  {
    if  (value.Empty ())
      fullHierarchyMustMatch = true;
    else
      fullHierarchyMustMatch = value.ToBool ();
    parameterUsed = true;
  }

  else 
  if  (parameter.EqualIgnoreCase ("-OtherClass") ||
       parameter.EqualIgnoreCase ("-Other")      ||
       parameter.EqualIgnoreCase ("-OC")
      )
  {
    if  (value.Empty ())
    {
      log.Level (-1) << "ModelParamDual::ParseCmdLineParameter   ***ERROR***  -OtherClass parameter must specify a class." << endl;
      ValidParam (false);
    }
    else
    {
      otherClass = MLClass::CreateNewMLClass (value);
    }
    parameterUsed = true;
  }

  else
  if  (parameter.EqualIgnoreCase ("-ProbFusionMethod") ||
       parameter.EqualIgnoreCase ("-PFM")
      )
  {
    probFusionMethod = ProbFusionMethodFromStr (value);
  }
}  /* ParseCmdLineParameter */




/**
 @brief  // Will get called after the entire parameter string has been processed.
 */
void   ModelParamDual::ParseCmdLinePost (RunLog&   log)
{
  if  (configFileName1.Empty ()  ||  configFileName2.Empty ())
  {
    log.Level (-1) << "ModelParamDual::ParseCmdLinePost  ***ERROR***  You need to specify two configuration files  -config1  and  -config2" << endl;
    ValidParam (false);
  }
}


/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParamDual::ToCmdLineStr () const
{
  KKStr  cmdStr (300);
  cmdStr = ModelParam::ToCmdLineStr ();

  if  (!configFileName1.Empty ())
    cmdStr << "  " << "-Classifier1" << " " << configFileName1;

  if  (!configFileName2.Empty ())
    cmdStr << "  " << "-Classifier2" << " " << configFileName2;

  if  (fullHierarchyMustMatch)
    cmdStr << "  " << "-FullHierarchyMustMatch" << " " << "Yes";

  if  (otherClass)
    cmdStr << "  " << "-OtherClass" << " " << otherClass->Name ();

  cmdStr << "  " << "-ProbFusionMethod" << " "  << ProbFusionMethodToStr (this->probFusionMethod);

  return  cmdStr;
}  /* ToCmdLineStr */




void  ModelParamDual::WriteXML (const KKStr&  varName,
                                ostream&      o
                               )  const
{
  XmlTag  startTag ("ModelParamDual",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;


  WriteXMLFields (o);

  configFileName1.WriteXML ("ConfigFileName1", o);
  configFileName2.WriteXML ("ConfigFileName1", o);

  XmlElementBool::WriteXML (fullHierarchyMustMatch, "FullHierarchyMustMatch", o);
  if  (otherClass)
    otherClass->WriteXML ("OtherClass", o);

  ProbFusionMethodToStr (probFusionMethod).WriteXML ("ProbFusionMethod", o);

  XmlTag  endTag ("ModelParamDual", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ModelParamDual::ReadXML (XmlStream&      s,
                               XmlTagConstPtr  tag,
                               RunLog&         log
                              )
{
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    t = ReadXMLModelParamToken (t);
    if  (t)
    {
      const KKStr&  varName = t->VarName ();

      if  (varName.EqualIgnoreCase ("ConfigFileName1"))
        configFileName1 = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());

      else if  (varName.EqualIgnoreCase ("ConfigFileName2"))
        configFileName2 = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());

      else if  (varName.EqualIgnoreCase ("FullHierarchyMustMatch"))
        fullHierarchyMustMatch = dynamic_cast<XmlElementBoolPtr> (t)->Value ();

      else if  ((varName.EqualIgnoreCase ("OtherClass"))  &&  (typeid (*t) == typeid (XmlElementMLClass)))
        otherClass = dynamic_cast<XmlElementMLClassPtr> (t)->Value ();

      else if  (varName.EqualIgnoreCase ("ProbFusionMethod"))
        probFusionMethod = ProbFusionMethodFromStr (*(dynamic_cast<XmlElementKKStrPtr> (t)->Value ()));
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}  /* ReadXML */



XmlFactoryMacro(ModelParamDual)

