/* CmdLineExpander.h -- Pre-process Command Line parameters.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _CMDLINEEXPANDER_
#define  _CMDLINEEXPANDER_

#include  "RunLog.h"
#include  "KKStr.h"


#if  !defined(OS_WINDOWS)
// Disable "assignment operator could not be generated" warning.
#pragma warning(disable : 4512)
#endif


namespace  KKB
{
  /**
   *@brief  Expands command line parameters, by parsing for special parameters and expanding them to their full value.
   *@details A couple of parameters are specifically searched for and processed.  The "-log" and "-CmdFile" parameters.
   * The "-log" parameter is processed for the log file.  The "-CmdFile" parameter will expand into multiple parameters.
   * This is done by reading in the text file specified by the "-CmdLine" parameter.  Each line in the text file processed 
   * for parameters that are then added to the original cmdline in place of '-CmdFile'.  The command line parameters in the
   * text file can specify another -CmdFile" parameter as long as it does not cause a circular loop.
   *@author  Kurt Kramer
   */

  class  CmdLineExpander
  {
  public:
    /**
     *@brief Construct using the parameters passed through on the command line into main().
     */
    CmdLineExpander (const KKStr&  _applicationName,
                     RunLog&       _log,
                     int32         argc,
                     char**        argv
                    );

    /**
     *@brief Pass in the command line as a single string.  Will parse first before processing.
     */
    CmdLineExpander (const KKStr&  _applicationName,
                     RunLog&       _log,
                     const KKStr&  _cmdLine
                    );


    ~CmdLineExpander ();

    void   ExpandCmdLine (int32   argc, 
                          char**  argv
                         );

    const VectorKKStr&  ExpandedParameters ()  const  {return expandedParameters;}
    const KKStr&        LogFileName        ()  const  {return logFileName;}
    bool                ParmsGood          ()  const  {return parmsGood;}

  private:
     void  BuildCmdLineParameters (const VectorKKStr&  argv);

     void  ExtractParametersFromFile (const KKStr&  cmdFileName, 
                                      VectorKKStr&  cmdFileParameters,
                                      bool&         validFile
                                     );

      KKStr        applicationName;
      VectorKKStr  cmdFileStack;
      VectorKKStr  expandedParameters;
      RunLog&      log;
      KKStr        logFileName;
      bool         parmsGood;
  };  /* CmdLineExpander */


  typedef  CmdLineExpander*  CmdLineExpanderPtr;

}  /* namespace  KKB */


#endif
