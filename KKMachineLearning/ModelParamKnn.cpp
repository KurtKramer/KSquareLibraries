#include "FirstIncludes.h"
#include <stdio.h>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
using namespace KKB;


#include "ModelParamKnn.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
#include "OSservices.h"
#include "RunLog.h"
using namespace KKMLL;





ModelParamKnn::ModelParamKnn  (FileDescPtr  _fileDesc,
                               RunLog&      _log
                               ):
  ModelParam (fileDesc, _log),
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
  return  new ModelParamKnn (*this);
}




KKStr  ModelParamKnn::ToCmdLineStr ()  const
{
  return  ModelParam::ToCmdLineStr () + "  -K " + StrFormatInt (k, "###0");
}





void  ModelParamKnn::ParseCmdLineParameter (const KKStr&  parameter,
                                            const KKStr&  value,
                                            bool&         parameterUsed
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




void    ModelParamKnn::WriteSpecificImplementationXML (std::ostream&  o)  const
{
  o << "<ModelParamKnn>"  << std::endl;
  o << "K" << "\t" << k << endl;
  o << "</ModelParamKnn>" << std::endl;
}



void    ModelParamKnn::ReadSpecificImplementationXML (istream&  i)
{
  log.Level (20) << "ModelParamKnn::ReadSpecificImplementationXML from XML file." << endl;

  char  buff[20480];
  
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);      // true = decode escape characters
    field.Upper ();

    if  (field.EqualIgnoreCase ("</ModelParamKnn>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<ModelParam>"))
    {
      ModelParam::ReadXML (i);
    }

    else if  (field.EqualIgnoreCase ("K"))
    {
      KKStr  kStr = ln.ExtractToken2 ("\t");
      k = kStr.ToInt ();
      if  ((k < 1)  ||  (k > 1000))
      {
        log.Level (-1) << "ModelParamKnn::ParseCmdLineParameter  ***ERROR***     Invalid -K parameter[" << kStr << "]" << endl;
        validParam = false;
      }
    }
  }
}  /* ReadSpecificImplementationXML */



