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


#include "ModelParamUsfCasCor.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "KKMLLTypes.h"
using namespace  KKMLL;


ModelParamUsfCasCor::ModelParamUsfCasCor  (RunLog&  _log):

  ModelParam (_log),
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
                                           bool      _useCache,
                                           RunLog&   _log
                                          ):
      ModelParam (_log),
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
                                                  bool&         parameterUsed
                                               )
{
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




void  ModelParamUsfCasCor::WriteSpecificImplementationXML (ostream&  o)  const
{
  log.Level (20) << "ModelParamUsfCasCor::WriteSpecificImplementationXML XML to ostream." << endl;

  o << "<ModelParamUsfCasCor>" << endl;

  o << "in_limit"         << "\t"  << in_limit                  << endl
    << "out_limit"        << "\t"  << out_limit                 << endl
    << "number_of_rounds" << "\t"  << number_of_rounds          << endl
    << "number_of_trials" << "\t"  << number_of_trials          << endl
    << "random_seed"      << "\t"  << random_seed               << endl
    << "useCache"         << "\t"  << (useCache ? "Yes" : "No") << endl;

  o << "</ModelParamUsfCasCor>" << endl;
}  /* WriteXML */





void  ModelParamUsfCasCor::ReadSpecificImplementationXML (istream&     i, 
                                                          FileDescPtr  fileDesc
                                                         )
{
  log.Level (20) << "ModelParamUsfCasCor::ReadSpecificImplementationXML from XML file." << endl;

  char  buff[20480];
  
  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    if  ((ln.Len () < 1)  || (ln.SubStrPart (0, 1) == "//"))
      continue;

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true); // true = decode escape characters
    field.Upper ();

    if  (field.EqualIgnoreCase ("<ModelParamUsfCasCor>"))
      continue;

    if  (field.EqualIgnoreCase ("</ModelParamUsfCasCor>"))
    {
      break;
    }

    else if  (field.EqualIgnoreCase ("<ModelParam>"))
    {
      ModelParam::ReadXML (i, fileDesc);
    }

    else if  (field.EqualIgnoreCase ("in_limit"))
    {
      in_limit = ln.ExtractTokenInt ("\t");
    }

    else if  (field.EqualIgnoreCase ("number_of_trials"))
    {
      number_of_trials = ln.ExtractTokenInt ("\t");
    }

    else if  (field.EqualIgnoreCase ("out_limit"))
    {
      out_limit = ln.ExtractTokenInt ("\t");
    }

    else if  (field.EqualIgnoreCase ("number_of_rounds"))
    {
      number_of_rounds = ln.ExtractTokenInt ("\t");
    }

    else if  (field.EqualIgnoreCase ("random_seed"))
    {
      KKStr  randomSeedStr = ln.ExtractToken2 ("\t");
      random_seed = randomSeedStr.ToInt64 ();
    }

    else if  (field.EqualIgnoreCase ("useCache"))
    {
      useCache = ln.ToBool ();
    }

    else
    {
      log.Level (-1) << endl << endl << "ModelParamUsfCasCor::ReadSpecificImplementationXML   ***ERROR**  Invalid Parameter[" << field << "]" << endl << endl;
      ValidParam (false);
    }
  }
}  /* ReadSpecificImplementationXML */



