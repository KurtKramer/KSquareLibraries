/* CmdLineExpander.cpp -- Pre-process Command Line parameters.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#include  <stdlib.h>
#include  <stdio.h>
#include  <fcntl.h>
#include  <string>
#include  <iomanip>
#include  <fstream>
#include  <iostream>
#include  <fstream>
#include  <vector>

#include  "MemoryDebug.h"

#include <sys/types.h>
#ifdef  WIN32
#include <io.h>
#include <windows.h>
#else
//#include  <sys/loadavg.h>
#include <unistd.h>
#endif

using namespace std;

#include "CmdLineExpander.h"
#include "OSservices.h"
#include "Str.h"
using namespace KKU;



CmdLineExpander::CmdLineExpander (const KKStr&  _applicationName,
                                  RunLog&        _log,
                                  int32          argc,
                                  char**         argv
                                 ):

  applicationName (_applicationName),
  log             (_log)

{
  ExpandCmdLine (argc, argv);
}



  
CmdLineExpander::CmdLineExpander (const KKStr&  _applicationName,
                                  RunLog&       _log,
                                  const KKStr&  _cmdLine
                                 ):
  applicationName (_applicationName),
  log             (_log)

{
  VectorKKStr  initialParameters;

  KKStr  cmdLine (_cmdLine);

  cmdLine.TrimLeft ();
  while  (!cmdLine.Empty ())
  {
    KKStr  nextField = cmdLine.ExtractQuotedStr ("\n\r\t ", false);  // false = Do not decode escape characters
    nextField.TrimRight ();
    if  (!nextField.Empty ())
      initialParameters.push_back (nextField);
    cmdLine.TrimLeft ();
  }

  BuildCmdLineParameters (initialParameters);
  BuildExpandedParameterPairs ();
}





CmdLineExpander::~CmdLineExpander ()
{
}





void  CmdLineExpander::ExpandCmdLine (int32   argc, 
                                      char**  argv
                                     )
{
  parmsGood = true;
  
  VectorKKStr  initialParameters;
  {
    int32 y;
    for  (y = 1; y < argc;  y++)
      initialParameters.push_back (argv[y]);
  }

  BuildCmdLineParameters (initialParameters);
  BuildExpandedParameterPairs ();

  return;
}  /* ExpandCmdLine */




bool  FileInStack (const KKStr&       cmdFileName, 
                   const VectorKKStr& cmdFileStack
                  )
{
  VectorKKStr::const_iterator  idx;
  for  (idx = cmdFileStack.begin ();  idx != cmdFileStack.end ();  idx++)
  {
    if  (*idx == cmdFileName)
      return true;
  }
  return  false;

}  /* FileInStack */




void  CmdLineExpander::BuildCmdLineParameters (const VectorKKStr&  argv)
{
  uint32  x = 0;

  while  (x < argv.size ())
  {
    KKStr  s = argv[x];
    x++;

    KKStr  sUpper = s.ToUpper();
    if  ((sUpper == "-L")  ||  (sUpper == "-LOGFILE"))
    {
      if  (x < argv.size ())
      {
        if  (argv[x][(int16)0] != '-')
        {
          logFileName = argv[x];
          if  (!logFileName.Empty ())
            log.AttachFile (logFileName);
          x++;
        }
      }

      if  (logFileName.Empty ())
      {
        log.Level (-1) << endl << endl;
        log.Level (-1) << applicationName   << " - Invalid Log File Parameter (-L)." << endl;
        log.Level (-1) << "                 Name of log file required."              << endl;
        log.Level (-1) << endl;
        parmsGood = false;
      }

    }

    else if  (sUpper == "-CMDFILE")
    {
      KKStr  cmdFileName = "";

      if  (x < argv.size ())
      {
        if  (argv[x][(int16)0] != '-')
        {
          cmdFileName = argv[x];
          x++;
        }
      }

      if  (cmdFileName.Empty ())
      {
        log.Level (-1) << endl << endl << endl
             << applicationName  << "  "  << "BuildCmdLineParameters             *** ERROR ***"  << endl << endl
             << "-CMDFILE option did not define a file name." << endl
             << endl;

        parmsGood = false;
      }

      else
      {
        if  (FileInStack (cmdFileName, cmdFileStack))
        {
          log.Level (-1) << endl << endl << endl
               << applicationName  << "  BuildCmdLineParameters             *** ERROR ***"  << endl 
               << endl
               << "-CMDFILE [" << cmdFileName << "]  is being called recursively."  << endl
               << endl;
 
          parmsGood = false;
        }
        else
        {
          bool  validFile = true;
          cmdFileStack.push_back (cmdFileName);
          VectorKKStr  cmdFileParameters;
          ExtractParametersFromFile (cmdFileName, cmdFileParameters, validFile);
          BuildCmdLineParameters (cmdFileParameters);
          cmdFileStack.pop_back ();
          if  (!validFile)
            parmsGood = false;
        }
      }
    }

    else
    {
      expandedParameters.push_back (s);
    }
  }
}  /* BuildCmdLineParameters */




void  CmdLineExpander::BuildExpandedParameterPairs ()
{
  kkuint32  x = 0;
  expandedParameterPairs.clear ();

  while  (x < expandedParameters.size ())
  {
    const KKStr&  nextField = expandedParameters[x];
    x++;

    KKStr  parmSwitch = "";
    KKStr  parmValue  = "";

    if  (!ParameterIsASwitch (nextField))
    {
      parmSwitch = "";
      parmValue  = nextField;
    }
    else
    {
      parmSwitch = nextField;
      if  (x < expandedParameters.size ())
      {
        if  (!ParameterIsASwitch (expandedParameters[x]))
        {
          parmValue = expandedParameters[x];
          x++;
        }
      }
    }

    expandedParameterPairs.push_back (KKStrPair (parmSwitch, parmValue));
  }

}  /* BuildExpandedParameterPairs */




bool  CmdLineExpander::ParameterIsASwitch (const KKStr&  parm)
{
  if  (parm.Len () < 1)
    return false;

  if  (parm[(int16)0] != '-')
    return false;

  if  (parm.Len () == 1)
    return true;

  double  parmValue = 0.0;
  if  (parm.ValidNum (parmValue))
    return false;

  return true;
}  /* ParameterIsASwitch */




void  CmdLineExpander::ExtractParametersFromFile (const KKStr&  cmdFileName, 
                                                  VectorKKStr&  cmdFileParameters,
                                                  bool&         validFile
                                                 )
{
  FILE*  in = osFOPEN (cmdFileName.Str (), "r");
  if  (!in)
  {
    log.Level (-1) << endl << endl << endl
         << "ExtractParametersFromFile     *** EROR ***" << endl
         << endl
         << "      Invalid CmdFile[" << cmdFileName << "]" << endl
         << endl;
    validFile = false;
    return;
  }

  KKStr  token;
  bool    eof = false;

  token  = osReadNextQuotedStr (in, " \n\r", eof);
  while  (!eof)
  {
    cmdFileParameters.push_back (token);
    token  = osReadNextQuotedStr (in, " \n\r", eof);
  }

  fclose (in);
}  /* ExtractParametersFromFile */

