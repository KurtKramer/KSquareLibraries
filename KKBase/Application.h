/* Application.h -- Generic Application class.  
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _APPLICATION_
#define  _APPLICATION_

#include "KKStr.h"
#include "RunLog.h"




namespace KKB 
{
  /** 
   *@class Application
   *@brief The base class for all standalone application.
   *@details  This class is meant to be a general class that all standalone applications should be inherited 
   *          from.  It supports command line processing, and logging facilities.
   */

  class  Application
  {
  public:
    /** 
     *@brief  Constructor for Application class that will start with a default logger(RunLog),
     *@details After creating an instance of this class you initialize it by calling InitalizeApplication.
     */
    Application ();


    /** 
     *@brief  Copy Constructor for Application class.
     *@param[in]  _application  Application instance to copy.
     */
    Application (const Application&  _application);


    /**
     *@brief  Constructor for Application class where we already have an existing logger '_log'.
     *@details After creating an instance of this class you initialize it by calling InitalizeApplication.
     *@param[in]  _log  A reference to a RunLog object.
     */
    Application (RunLog&  _log);


    virtual
    ~Application ();

    /**
     *@brief  Returns 'true' if application has been aborted.
     *@details  You would typically call this method after you are done processing the command line to
     *          make sure that the application is to keep on running.
     */
    bool  Abort ()  {return  abort;}

    /** 
     *@brief Used to specify that the application is been aborted.
     *@details If you have a reason to abort the processing of this application you would call this method to set the 'abort' 
     *         flag to true. It will be the responsibility of the derived class to monitor the 'Abort' flag. If is set to 
     *         true they should terminate as quickly as they can;  they should also release any resources they have taken.
     *@param[in] _abort  Abort status to set; if set to true you are telling the application that the program needs to be terminated.
     */
    void  Abort (bool _abort)  {abort = _abort;}

    /** Specify the name of the application */
    virtual 
    const char*  ApplicationName ();

    void         AssignLog (RunLog&  _log);  /**< @brief Replaces the Log file to write to.  */

    virtual
    KKStr        BuildDate ()  const;



    /** 
     *@brief  Initialized Application Instance; 1st method to be called after instance construction.
     *@details This method will scan the command line parameters for the log file options  (-L, -Log, or -LogFile)  and use its
     *        parameter as the LogFile name.  If none is provided it will assume the stdout as the Log File to write to.  It will take
     *        ownership of this log file and delete it in its destructor.  Right after calling this constructor you will need to
     *        call the method ProcessCmdLineParameters.
     *@see  RunLog
     *@see  ProcessCmdLineParameters
     *@param[in]  argc  Number of arguments in argv.
     *@param[in]  argv  List of asciiz strings; one string for each argument.
     */
    virtual
    void     InitalizeApplication (kkint32  argc,
                                   char**   argv
                                  );



  protected:
    /**
     *@brief  Will display Command Lone parameters that the 'Application' class will manage.
     *@details  Derived classes that implement this method need to call their immediate base class version of this method
     * to include these parameters.
     */
    virtual
    void  DisplayCommandLineParameters ();


    /**
     *@brief This method will get called once for each parameter specified in the command line.
     *@details  Derived classes should define this method to intercept parameters that they are interested in.
     *          Parameters are treated as pairs, Switched and Values where switches have a leading dash("-").
     *          The CmdLineExpander class will be used to expand any "-CmdFile" parameters.
     *@param[in] parmSwitch      The fill switch parameter.
     *@param[in] parmValue       Any value parameter that followed the switch parameter.
     */
    virtual 
    bool    ProcessCmdLineParameter (const KKStr&  parmSwitch, 
                                     const KKStr&  parmValue
                                    );


  private: 
    /**
     *@brief Processes all the command line parameters; will also expand the -CmdFile option.
     *@details This method assumes that the command line consists of pairs of Switches and Operands.  Switches are proceeded by the
     *         dash character("-").  For each pair it will call the 'ProcessCmdLineParameter' method which should be implemented by
     *         the derived class.  Before calling 'ProcessCmdLineParameter' though it will scan the parameters for the "-CmdFile" 
     *         switch.  This switch specifies a text file that is to be read for additional command parameters.
     *@param[in] argc   Number of parameters.
     *@param[in] argv   The actual parameters.
     */
    void        ProcessCmdLineParameters (kkint32  argc,
                                          char**   argv
                                         );


    bool        abort;

  public:
    RunLog&     log;  

  private:
    vector<KKStrPair>  expandedParameterPairs;
    KKStr              logFileName;
    RunLogPtr          ourLog;  // We use this Log file if one is not provided,
  };  /* Application */
}  /* NameSpace KKB */

#endif
