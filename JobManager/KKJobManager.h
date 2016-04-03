#ifndef  _JOBMANAGER_
#define  _JOBMANAGER_

namespace  KKJobManagment
{
}


#include "KKJob.h"


#include "RunLog.h"


/**
 *@namespace  KKJobManagment
 *@brief  NameSpace for the JobManagement Library.
 */

namespace  KKJobManagment
{
  /**
   *@class  KKJobManager
   *@brief  Responsible for keeping track of a list of jobs.
   *@details See the application called RandomSplitsJobManager. It is the first application to use 
   *         this library; any class that is derived from this class are required to implement these
   *        method  "StatusFileProcessLine",  "StatusFileProcessLineJobStatusChange", "ToStatusStr"
   */
  class  KKJobManager: public  KKJob
  {
  public:
    typedef  KKJobManager*  JobManagerPtr;

    KKJobManager (const KKJobManager& j);

    KKJobManager (JobManagerPtr _manager,               /**< Ptr to job that is managing this 'KKJobManager'  */
                  kkint32       _jobId,
                  kkint32       _parentId,
                  kkint32       _numPorcessesAllowed,
                  const KKStr&  _managerName,           /**< Name of this 'KKJobManager' ; status and lock file will be based on it.               */
                  kkint32       _numJobsAtATime,        /**< The number of jobs that can be allocated at one time for a single process to execute. */
                  RunLog&       _log
                 );

    ~KKJobManager ();


    /**
     *@brief  Initialize the KKJobManager object.
     *@details  This is the first Method that needs to be called right after the object is constructed.
     */
    void  InitilizeJobManager (bool&  successful);


    virtual  KKJobPtr  Duplicate ()  const = 0;  /**< Create a duplicate instance.  */

    virtual  const char*   JobType ()  const;   /**< Allows us to know which specific implementation of 'KKJob'  an instance really is. */


    bool          SupportCompletedJobData () const  {return  supportCompletedJobData;}

    void          SupportCompletedJobData (bool  _supportCompletedJobData)  {supportCompletedJobData = _supportCompletedJobData;}


    double        CpuTimeTotalUsed  () const  {return  cpuTimeTotalUsed;}
    kkint32       ExpansionCount    () const  {return  expansionCount;}
    KKJobListPtr  Jobs              () const  {return  jobs;}
    const KKStr&  ManagerName       () const  {return  managerName;}
    kkint32       NextJobId         () const  {return  nextJobId;}
    bool          QuitRunning       () const  {return  quitRunning;}


    kkint32       AllocateNextJobId ();  /**< Used to get next available KKJob Id */

    bool          AreAllJobsDone ();

    void          Block ();
 
    void          EndBlock ();

    kkint32       GetNextJobId ();   /**< Returns 'nextJobId' then increments by '1'. */
  
    void          Restart ();
  
    void          Run ();
  
    void          SetQuitRunningFlag ();

    void          Update (JobManagerPtr  p);

  private:
    virtual
      KKJobListPtr  JobsCreateInitialSet () = 0;   /**< Derived classes use this method to seed the initial set of jobs. */


    /**
     *@brief  KKJob manager calls this method when ever it is ready to start processing another group of jobs.
     *@details Derived classes need to implement this method; it will be called when all jobs have been completed.  The 
     * variable 'jobs' will contain a list of all jobs.
     *@params[in,out]  jobsJustCompletd -  Contains a list of jobs that have been completed since the last time this 
     *                  method was called or 'JobsCreateInitialSet' was called.
     *@Returns  List of new jobs that are to be processed.  A NULL signifies that no more jobs to process.
     */
    virtual  KKJobListPtr  JobsExpandNextSetOfJobs (const KKJobListPtr  jobsJustCompletd) = 0;

    
    virtual  void  GenerateFinalResultsReport () = 0;


    
   /**
    *@brief  Load any run time data that will be needed.
    *@details  'KKJobManager' will call this method just before it calls 'StatusFileInitialize' if
    * no existing StausFile otherwise it will call it just after it loads the StatusFile.
    */
    virtual  void  LoadRunTimeData () = 0;


    /** 
     *@brief In one (Block - EndBlock)  sequence will update completed jobs and select next set of jobs to process.
     */
    KKJobListPtr  GetNextSetOfJobs (KKJobListPtr  completedJobs);



    //****************************************************************************************
    //*   Status File Routines.

    void    ReportCpuTimeUsed (ofstream* statusFile);

    void    StatusFileInitialize ();

    virtual 
      void  StatusFileInitialize (ostream& o) = 0;


    ofstream*  StatusFileOpen (ios::openmode  openMode);

    void       StatusFileLoad    ();  /**< To load initial contents of status file.  */
  
    void       StatusFileRefresh ();  /**< Will read in any changes from status file since last call.  */

    void       StatusFileWrite   ();  /**< Will rewrite the status file from scratch;  the old one will be deleted.  */


  protected:
    // The next 3 methods need to be implemented by any Derived classes.  Those derived class need to also
    // call the Base class version of this method.

    virtual void    StatusFileProcessLine (const KKStr&  ln,
                                           istream&      statusFile
                                          );

    virtual void    StatusFileProcessLineJobStatusChange (KKStr&  statusLineStr);

    virtual KKStr   ToStatusStr ();  /**<derived classes should implement this method.  They need to return a tab
                                      * delimited string with all there parameters
                                      * <FieldName1> <\t> <FieldValue1> <\t> <fieldName2> <\t> <FieldValue2>  etc etc etc ...
                                      * They should call the base Class version of this method and return that string first.
                                      * ex:  return KKJobManager::ToStatusStr () + "\t" + "FieldName1" + "\t" + "Value1" + "\t" + "FieldName2" + "\t" + "Value2";
                                      */

  private:
    //****************************************************************************************
    void  ProcessNextExpansion (ostream&  o);

    void  ProcessRestart ();

    void  ProcessJobXmlBlockOfText (const KKStr&  startStr,
                                    istream&      i
                                   );


    double          cpuTimeLastReported;
    double          cpuTimeTotalUsed;
    KKB::DateTime   dateTimeStarted;
    KKB::DateTime   dateTimeEnded;
    bool            dateTimeFirstOneFound;  /**< Flag that indicates weather we have read the first 
                                             * instance of a 'CurrentDateTime' in the Status file.
                                             */


    KKJobListPtr    jobs;                 /**<List of jobs that we are working with.  We
                                           * will be maintaining consistency with other
                                           * parallel running processes through the 
                                           * 'status' file.  As we make changes to each 
                                           * individual 'BinaryJob' we write them out to
                                           * the status file.  We periodically read from 
                                           * the status file to see if there have been
                                           * changes made by other processors.
                                           */
  
    kkint32         blockLevel;           /**< Starts at 0; increments with each Call to 'Block'  and  decrements with 'EndBlock'  */
    kkint32         lockFile;
    KKStr           lockFileName;
    bool            lockFileOpened;

    KKStr           managerName;
    bool            supportCompletedJobData;  /**< If set to 'true' will call 'WriteCompletedJobData' after each job is completed. */

    kkint32         expansionCount;
    kkint32         expansionFirstJobId;      /**< jobId of first job in the current expansion. */
    kkint32         nextJobId;
    kkint32         numJobsAtATime;
    kkint32         procId;                   /**< Processor Id  that O/S assigns to this thread.  */
    bool            quitRunning;              /**< If this flag is set true we are to terminate processing as soon as possible. */
  
    bool            restart;
  
    KKStr           statusFileName;
    long            statusFileNextByte;       /**< Byte offset of next bye to read from status file.  */
  
  };  /* KKJobManager */
  

  typedef  KKJobManager::JobManagerPtr  JobManagerPtr;

  #define  _JobManagerDefined_



  class  JobManagerList: public  KKQueue<KKJobManager>  
  {
  public:
    JobManagerList (const KKStr& _rootDir);
    ~JobManagerList ();

  private:
  };

}  /* KKJobManagment */



#endif





