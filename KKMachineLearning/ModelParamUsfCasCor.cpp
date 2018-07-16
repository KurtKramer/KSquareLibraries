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


#include "ModelParamUsfCasCor.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
using namespace  KKMLL;


ModelParamUsfCasCor::ModelParamUsfCasCor  ():
  ModelParam (),
  in_limit         (500),
  out_limit        (500),
  number_of_rounds (-1),
  number_of_trials (1),
  random_seed      (0),
  useCache         (false)
{
}




ModelParamUsfCasCor::ModelParamUsfCasCor  (int       _in_limit,
                                           int       _out_limit,
                                           int       _number_of_rounds,
                                           int       _number_of_trials,
                                           kkint64   _random_seed,
                                           bool      _useCache
                                          ):
      ModelParam (),
      in_limit         (_in_limit),
      out_limit        (_out_limit),
      number_of_rounds (_number_of_rounds),
      number_of_trials (_number_of_trials),
      random_seed      (_random_seed),
      useCache         (_useCache)
{}
  



ModelParamUsfCasCor::~ModelParamUsfCasCor  ()
{
}



ModelParamUsfCasCorPtr  ModelParamUsfCasCor::Duplicate ()  const
{
  return  new ModelParamUsfCasCor (*this);
}  /* Duplicate */



void  ModelParamUsfCasCor::ParseCmdLineParameter (const KKStr&  parameter,
                                                  const KKStr&  value,
                                                  bool&         parameterUsed,
                                                  RunLog&       log
                                                 )
{
  log.Level(50) << "ModelParamUsfCasCor::ParseCmdLineParameter  parameter: " << parameter << "  value: " << value << std::endl;

  parameterUsed = true;
  if  (parameter.EqualIgnoreCase ("-InLimit")  ||
       parameter.EqualIgnoreCase ("-IL")       ||
       parameter.EqualIgnoreCase ("-I")
      )
    in_limit = value.ToInt ();

  else if  (parameter.EqualIgnoreCase ("-OutLimit")  ||
            parameter.EqualIgnoreCase ("-OL")        ||
            parameter.EqualIgnoreCase ("-O")
      )
    out_limit = value.ToInt ();

  else if  (parameter.EqualIgnoreCase ("-NumberOfRounds")  ||
            parameter.EqualIgnoreCase ("-NOR")             ||
            parameter.EqualIgnoreCase ("-Rounds")          ||
            parameter.EqualIgnoreCase ("-R")
      )
    number_of_rounds = value.ToInt ();

  else if  (parameter.EqualIgnoreCase ("-NumberOfTrials")  ||
            parameter.EqualIgnoreCase ("-NOT")             ||
            parameter.EqualIgnoreCase ("-T")
      )
    number_of_trials = value.ToInt ();


  else if  (parameter.EqualIgnoreCase ("-RandomSeed")  ||
            parameter.EqualIgnoreCase ("-RS")          ||
            parameter.EqualIgnoreCase ("-S")
      )
    random_seed = value.ToInt ();

  else if  (parameter.EqualIgnoreCase ("-UseCache")  ||
            parameter.EqualIgnoreCase ("-UC")        ||
            parameter.EqualIgnoreCase ("-Cache")
      )
  {
    if  (value.Empty ())
      useCache = true;
    else
      useCache = value.ToBool ();
  }

  else
    parameterUsed = false;
}  /* ParseCmdLineParameter */




/**
 *@brief  // Will get called after the entire parameter string has been processed.
 */
void   ModelParamUsfCasCor::ParseCmdLinePost ()
{
}




/**
 * @brief Convert all parameters to a command line string.
*/
KKStr   ModelParamUsfCasCor::ToCmdLineStr () const
{
  KKStr  cmdStr (300);

  cmdStr << "-InLimit "  << in_limit  << "  "
         << "-OutLimit " << out_limit;

  if  (number_of_rounds > 0)
    cmdStr << " -R " << number_of_rounds;

  cmdStr << " -T " << number_of_trials;

  if  (random_seed > 0)
    cmdStr << " -S " << random_seed;

  cmdStr << " -UseCache " << (useCache ? "Yes" : "No");

  return  cmdStr;
}  /* ToCmdLineStr */




void  ModelParamUsfCasCor::WriteXML (const KKStr&  varName,
                                     ostream&      o
                                    )  const
{
  XmlTag  startTag ("ModelParamUsfCasCor",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  WriteXMLFields (o);

  XmlElementInt32::WriteXML (in_limit,          "in_limit",         o);
  XmlElementInt32::WriteXML (number_of_rounds,  "number_of_rounds", o);
  XmlElementInt32::WriteXML (number_of_trials,  "number_of_trials", o);
  XmlElementInt64::WriteXML (random_seed,       "random_seed",      o);
  XmlElementBool::WriteXML  (useCache,          "useCache",         o);
  
  XmlTag  endTag ("ModelParamUsfCasCor", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  ModelParamUsfCasCor::ReadXML (XmlStream&      s,
                                    XmlTagConstPtr  tag,
                                    VolConstBool&   cancelFlag,
                                    RunLog&         log
                                   )
{
  log.Level (50) << "ModelParamUsfCasCor::ReadXML  tag: " << tag->Name () << endl;
  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    t = ReadXMLModelParamToken (t);
    if  ((t != NULL)  &&  (t->TokenType () == XmlToken::TokenTypes::tokElement))
    {
      XmlElementPtr e = dynamic_cast<XmlElementPtr> (t);
      const KKStr&  varName = e->VarName ();

      if  (varName.EqualIgnoreCase ("in_limit"))
        in_limit =  dynamic_cast<XmlElementInt32Ptr> (e)->Value ();

      else if  (varName.EqualIgnoreCase ("number_of_rounds"))
        number_of_rounds =  dynamic_cast<XmlElementInt32Ptr> (e)->Value ();

      else if  (varName.EqualIgnoreCase ("number_of_trials"))
        number_of_trials =  dynamic_cast<XmlElementInt32Ptr> (e)->Value ();

      else if  (varName.EqualIgnoreCase ("random_seed"))
        random_seed =  dynamic_cast<XmlElementInt64Ptr> (e)->Value ();

      else if  (varName.EqualIgnoreCase ("useCache"))
        useCache =  dynamic_cast<XmlElementBoolPtr> (e)->Value ();

      else
      {
        log.Level (-1) << endl
          << "ModelParamUsfCasCor::ReadXM   ***ERROR***   Unexpected Element: " << e->NameTag ()->ToString () << endl
          << endl;
      }
    }
    delete  t;
    t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */


XmlFactoryMacro(ModelParamUsfCasCor)
