/* CmdLineExpander.h -- Pre-process Command Line parameters.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _CMDLINEEXPANDER_
#define  _CMDLINEEXPANDER_

#include "RunLog.h"
#include "Str.h"

// Disable "assignment operator could not be generated" warning.
#pragma warning(disable : 4512) 

namespace  KKU
{
  /**
   *@brief  Expands command line parameters, by parsing for special parameters and expanding them to their full value.
   *@details A couple of parameters are specifically searched for and processed. The "-log" and "-CmdFile" parameters.
   * The "-log" parameter is provides the name of a file where logging messages are to be written to via the "RunLog"
   * class. The "-CmdFile" parameter provides the name of a text file that contains more command-line parameters that
   * are to be processed along with all other parameters; the text in this file will be inserted where the original
   * -"CmdLine" parameter appeared.
   *@author  Kurt Kramer
   */

  class  CmdLineExpander
  {
  public:
    /**
     *@brief Constructor using the parameters passed through on the command line into main().
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

    const VectorKKStr&        ExpandedParameters     ()  const  {return expandedParameters;}
    const vector<KKStrPair>&  ExpandedParameterPairs ()  const  {return expandedParameterPairs;}
    const KKStr&              LogFileName            ()  const  {return logFileName;}
    bool                      ParmsGood              ()  const  {return parmsGood;}

  private:
     void  BuildCmdLineParameters (const VectorKKStr&  argv);

     void  BuildExpandedParameterPairs ();

     void  ExtractParametersFromFile (const KKStr&  cmdFileName, 
                                      VectorKKStr&  cmdFileParameters,
                                      bool&         validFile
                                     );

     bool  ParameterIsASwitch (const KKStr&  parm);

     KKStr              applicationName;
     VectorKKStr        cmdFileStack;
     vector<KKStrPair>  expandedParameterPairs;
     VectorKKStr        expandedParameters;
     RunLog&            log;
     KKStr              logFileName;
     bool               parmsGood;
  };  /* CmdLineExpander */


  typedef  CmdLineExpander*  CmdLineExpanderPtr;

}  /* namespace  KKU */


#endif
