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
using namespace KKB;

#include "ModelParamKnn.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace KKMLL;



ModelParamKnn::ModelParamKnn  ():
  ModelParam (),
  k(1)
{
}



ModelParamKnn::ModelParamKnn  (const ModelParamKnn&  _param):
    ModelParam (_param),
    k          (_param.k)
{
}



ModelParamKnn::~ModelParamKnn  ()
{
}



ModelParamKnnPtr  ModelParamKnn::Duplicate ()  const
{
  return new ModelParamKnn (*this);
}



KKStr  ModelParamKnn::ToCmdLineStr (RunLog&  log)  const
{
  log.Level(50) << "ModelParamKnn::ToCmdLineStr" << endl;
  return  ModelParam::ToCmdLineStr () + "  -K " + StrFormatInt (k, "###0");
}



void  ModelParamKnn::ParseCmdLineParameter (const KKStr&  parameter,
                                            const KKStr&  value,
                                            bool&         parameterUsed,
                                            RunLog&       log
                                           )
{
  parameterUsed = true;

  if  (parameter.EqualIgnoreCase ("-K"))
  {
    k = value.ToInt ();
    if  ((k < 1)  ||  (k > 1000))
    {
      log.Level (-1) << "ModelParamKnn::ParseCmdLineParameter  ***ERROR***     Invalid -K parameter[" << value << "]" << endl;
      validParam = false;
    }
  }
  else
  {
    parameterUsed = false;
  }
}  /* ParseCmdLineParameter*/



void  ModelParamKnn::WriteXML (const KKStr&  varName,
                               ostream&      o
                              )  const
{
  XmlTag  startTag ("ModelParamDual",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteXMLFields (o);




  XmlTag  endTag ("ModelParamDual", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  ModelParamKnn::ReadXML (XmlStream&      s,
                              XmlTagConstPtr  tag,
                              VolConstBool&   cancelFlag,
                              RunLog&         log
                             )
{
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    t = ReadXMLModelParamToken (t);
    if  (t)
    {
      const KKStr&  varName = t->VarName ();

      if  (varName.EqualIgnoreCase ("k"))
        k = dynamic_cast<XmlElementInt32Ptr> (t)->Value ();

      else if  (varName.EqualIgnoreCase ("fileName"))
        fileName = *(dynamic_cast<XmlElementKKStrPtr> (t)->Value ());
    }

    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete t;
  t = NULL;
}  /* ReadXML */

XmlFactoryMacro(ModelParamKnn)
