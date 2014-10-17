/* Application.cpp -- Generic Application class.  
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "MemoryDebug.h"
using  namespace  std;

#include "Application.h"
#include "CmdLineExpander.h"
#include "DateTime.h"
#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "OSservices.h"
using  namespace  KKB;




Application::Application (RunLog&  _log):
  abort       (false),
  log         (_log),
  logFileName (),
  ourLog      (NULL)
{
}





Application::Application (const Application&  _application):
  abort        (_application.abort),
  log          (_application.log),
  logFileName  (_application.logFileName),
  ourLog       (_application.ourLog)

{
}


 Application::Application (KKB::int32  argc,
                           char**      argv
                          ):
  abort       (false),
  log         (*(new RunLog ())),
  logFileName (),
  ourLog      (NULL)

{
  ourLog = &log;
  ProcessCmdLineParameters (argc, argv);
  if  (!logFileName.Empty ())
    log.AttachFile (logFileName);
}


Application::~Application ()
{
  delete  ourLog;
  ourLog = NULL;
}


const char*  Application::ApplicationName ()
{
  return  "No-Application-Name-Provided";
}  /* ApplicationName */



void  Application::AssignLog (RunLog&  _log)
{
  log = _log;

  if  (ourLog)
  {
    delete  ourLog;
    ourLog = NULL;
  }
} /* AssignRunLog */




void  Application::ProcessCmdLineParameters (KKB::int32  argc,
                                             char**      argv
                                            )
{
  uint32  x;

  bool  allParmsGood = true;

  CmdLineExpander  cmdLineExpander (ApplicationName (), log, argc, argv);

  if  (!cmdLineExpander.ParmsGood ())
    Abort (true);

  if  (!cmdLineExpander.LogFileName ().Empty ())
    logFileName = cmdLineExpander.LogFileName ();


  const  VectorKKStr&  expandedParameters = cmdLineExpander.ExpandedParameters ();

  x = 0;
  while  (x < expandedParameters.size ())
  {
    KKStr  nextField = expandedParameters[x];
    x++;

    bool   parmGood       = true;
    KKStr  parmValue      = "";
    KKStr  parmSwitch     = "";
    char   parmSwitchCode = 0;

    if  (nextField[(kkint16)0] == '-')
    {
      parmSwitch = nextField;
      KKStr  parmSwitchUpper = parmSwitch.ToUpper ();

      parmSwitchCode = nextField[(kkint16)1];

      if  (x < expandedParameters.size ())
      {
        if  (!ParameterIsASwitch (expandedParameters[x]))
        {
          parmValue = expandedParameters[x];
          x++;
        }
      }

      parmGood = ProcessCmdLineParameter (parmSwitchCode, parmSwitch, parmValue);
    }

    else
    {
      parmValue = nextField;
      parmGood = ProcessCmdLineParameter (parmSwitchCode, parmSwitch, parmValue);
    }

    if  (!parmGood)
      allParmsGood = false;
  }  /*  end of for loop  */

  if  (!allParmsGood)
    abort = true;
}  /* ProcessCmdLineParameters */



bool  Application::ParameterIsASwitch (const KKStr&  parm)
{
  if  (parm.Len () < 1)
    return false;

  if  (parm[(kkint16)0] != '-')
    return false;

  if  (parm.Len () == 1)
    return true;

  double  parmValue = 0.0;
  if  (parm.ValidNum (parmValue))
    return false;

  return true;
}



bool  Application::ProcessCmdLineParameter (char    parmSwitchCode, 
                                            KKStr  parmSwitch, 
                                            KKStr  parmValue
                                           )
{
  return  true;
}




KKStr   Application::BuildDate ()  const
{
  DateType  buildDate (__DATE__);
  TimeType  buildTime (__TIME__);
  return  buildDate.YYYY_MM_DD () + "-" + buildTime.HH_MM_SS ();
}  /* BuildDate */
