/* RunLog.h -- Logging Class.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#if  !defined(_RUNLOG_)
#define  _RUNLOG_

/** 
 *@class KKB::RunLog
 *@brief Used for logging messages.
 *@details
 * Has facilities to deal with different levels of logging.  You use the normal insertion operators to write
 * to this object.  You can specify at any point a Logging level.  If this level is less than or equal to a
 * specified threshold the message will be recorded in the log.
 * <br \>
 * The idea is that during normal production runs log level would be set to 10.  But if there appears to be
 * some issue then you would set the logging level to a higher level to get more details. The higher the
 * level the more detail you will get.
 */

/*--------------------------------------------------------------------
 *  History                                                         
 *--------------------------------------------------------------------
 *                                                                  
 *    date     Programmer   Description                             
 *  ---------  -----------  ----------------------------------------
 *  July-2002  Kurt Kramer  Original Development.
 *                                                                   
 */


#include "KKStr.h"


#ifndef  WIN32
#if  !defined(__cdecl)
#define  __cdecl  
#endif
#endif



namespace KKB 
{

  #if  !defined(_KKU_MSGQUEUE_)
  class  MsgQueue;
  typedef  MsgQueue*  MsgQueuePtr;
  #endif

  class  RunLog
  {
  public:
    /** @brief Creates an instance were logging will go to the console. */
    RunLog ();

    /** @brief Creates instance with a log messages written to file 'fileName'. */
    RunLog (const char*  _fileName);

    /** @brief Creates instance with a log messages written to file 'fileName'. */
    RunLog (const KKStr&  _fileName);

    /** @brief Creates instance where logging messages are written to file stream specified by 'errorLogStream'. */
    RunLog (std::ostream&  errorLogStream);

    /** @brief Creates instance where logging messages are written to file stream pointed to by 'logStream'. */
    RunLog (std::ostream*  logStream);

    RunLog (MsgQueuePtr  _msgQueue);

    ~RunLog ();


    ///<summary>
    /// Sets current message level you would normally call this method just before you wish to start logging a
    /// message. If you are not changing the level since previous call then no need to call.
    ///</summary>
    RunLog&  Level (kkint32 _level);

    kkint32  Level () const  {return curLevel;}


    /** @brief  Appends string to log; generally used by other Log methods but can be called by public. */
    void  Append (const char*  str);


    /**
     *@brief Call this method to start logging messages to a different file. 
     *@details  Will close out current log file and open new one specified by 'fileName'.
     */
    void  AttachFile (const KKStr&  _fileName);


    /**
     *@brief Call this method to start logging messages to a specific file stream. 
     *@details  Will close out current log file and write to the specified '_logFile' in the future.
     */
    void  AttachFile (std::ostream&  _logFile);


    /**
     *@brief Call this method to start appending logging messages to a different file. 
     *@details  Will close out current log file and open new one specified by 'fileName' with the append flag set.
     */
    void  AttachFileAppend (const KKStr&  _fileName);


    /**
     *@brief  All activity that is written to the log file will be added as messages to '_msgQueue'.
     */
    void  AttachMsgQueue (MsgQueuePtr  _msgQueue);

    /** @brief  Closes out current log file and starts writing to stdout aka 'cout'. */
    void  DetachFile ();

    KKStr  FileName ();

    /** @brief  Returns the last line of text written to the log file. */
    const KKStr&  LastLine ()  {return lastLine;}

    kkint32 LineCount ()  const  {return  lineCount;}

    void    SetLoggingLevel (kkint32 _loggingLevel);


    /**
     *@brief  Writes a line of text to log file; does not check logging level; and does not write any preview text such as DateTime or processId. 
     *@details  If there is any data in Current-Line it will be write out first before writing this line of text.
     */
    void    WriteLine (const KKStr&  s);

    /**
     *@brief  Writes a line of text to log file; does not check logging level; and does not write any preview text such as DateTime or processId. 
     *@details  If there is any data in Current-Line it will be write out first before writing this line of text.
    */
    void    WriteLine (const char* s);


    RunLog&  operator<< (bool           right);
    RunLog&  operator<< (kkint16        right);
    RunLog&  operator<< (kkuint16       right);
    RunLog&  operator<< (kkint32        right);
    RunLog&  operator<< (kkuint32       right);
    RunLog&  operator<< (kkint64        right);
    RunLog&  operator<< (kkuint64       right);
    RunLog&  operator<< (double         right);
    RunLog&  operator<< (char           right);
    RunLog&  operator<< (const char*    right);
    RunLog&  operator<< (const KKStr&   right);
    RunLog&  operator<< (KKStrConstPtr  right);
    RunLog&  operator<< (const VectorKKStr& right);

    RunLog&  operator<< (std::ostream& (* mf)(std::ostream &));

    //friend  RunLog& endl (RunLog& _outs);

    void    Flush ();

    void    SetLevel (kkint32 _level)  {loggingLevel = _level;}

    kkMemSize  MemoryConsumedEstimated ()  const;

    void  Info (std::function< KKStr() > lambda);
  
  private:
    void  DisplayTimeStamp ();

    void  GetLoggingLevel ();

    bool            callDisplayTimeStamp;

    kkint32         curLevel;     // Current level of Message being written.

    KKStr           curLine;

    KKStr           fileName;

    KKStr           lastLine;

    kkint32         lineCount;

    bool            lineEmpty;

    std::ostream*   logFile;      /**< File stream that we will be writing logging messages to; if fileName was
                                    * specified then 'logFile = ourLogFile'.
                                    */

    kkint32         loggingLevel; /**< Logging level that we are to be writing messages for any messages written
                                    * while 'curLevel' <= 'loggingLevel' will be written to '*logFile'.
                                    */
    
    MsgQueuePtr     msgQueue;


    std::ofstream*  ourLogFile;   // Will be NULL if we are writing to the console.

    kkint32         procId;       // Processor Id assigned by OS
  };


  typedef  RunLog*  RunLogPtr;

#define  _RunLog_Defined_

  /*
  #ifdef  WIN32
  RunLog& __cdecl operator<< (RunLog &  log,
                              RunLog & (__cdecl*)(RunLog &));

  RunLog& __cdecl endl(RunLog &  log);
  #else

  RunLog&  operator<< (RunLog &  log,
                       RunLog & (*)(RunLog &));


  RunLog&  endl(RunLog &  log);

  #endif
  */
}

#endif
