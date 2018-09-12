#ifndef  _KKJOB_
#define  _KKJOB_

//****************************************************************************
//*                                                                          *
//*  Kurt Kramer                                                             *
//*   The RandomSplits process uses this library.                            *
//*                                                                          *
//*  Kurt Kramer                                                             *
//*  2009-10-10                                                              *
//*                                                                          *
//*   This code is currently not in use.  I am toying with the idea to make  *
//*   a general KKJob Management Frame Work.  The Idea is that 'KKJob' and   *
//*   'KKJobManager' would be used to build specific KKJob Management code   *
//*   around.  In the case of 'FeatureSeletion' we would derive subclasses   *
//*   to manage the Major Steps and Jobs which would then in turn call a     *
//*   different set of derived classes that would manage Binary, MultiClass, *
//*   Parameter Tuning, and Feature Selection tasks.                         *
//*                                                                          *
//*   ex:  KKJobManager would be replaced by.                                *
//*      ParameterManager:: public KKJobManager                              *
//*      FeatureSelectionManager:: public KKJobManager                       *
//*                                                                          *
//*   Would also need to create a set of classes to manage the RandomSplits  *
//*                                                                          *
//****************************************************************************


#include "KKQueue.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"


/**
 * @Namespace KKJobManagment
 * @brief  A framework for managing a large number of processes(Jobs) in a multi-cpu/ multi-o/s environment.
 * @details The Idea is that 'KKJob' and  'KKJobManager' would be used to build specific KKJob Management code
 * around. In the case of 'FeatureSeletion'  we would derive sub-classes to manage the Major Steps and 
 * Jobs which would then in turn call a different set of derived classes that would manage Binary, MultiClass,  
 * Parameter Tuning, and Feature Selection tasks.
 * @code
 *   ex:  KKJobManager would be replaced by.
 *      ParameterManager:: public KKJobManager
 *      FeatureSelectionManager:: public KKJobManager
 * @endcode
 *
 *  The RandomSplits application makes use of the framework.
 */
namespace  KKJobManagment
{
  class   KKJobList;
  typedef KKJobList* KKJobListPtr;


  #if  !defined(_JobManagerDefined_)
  class  KKJobManager;
  typedef  KKJobManager*  JobManagerPtr;
  #endif


  class  KKJob
  {
  public:
    typedef  enum   {jsNULL, jsOpen, jsStarted, jsDone, jsExpanded}  JobStatus;

    typedef  enum   {jtNULL, jtBinaryCombo, jtMultiClass, jtRandomSplit}  JobTypes;

    typedef  KKJob*  KKJobPtr;

    KKJob (const KKJob&  j);

    /**  To create a new job that has not been processed yet.  */
    KKJob (JobManagerPtr  _manager,
           kkint32        _jobId,
           kkint32        _parentId,
           kkint32        _numPorcessesAllowed,
           RunLog&        _log
          );


    KKJob (JobManagerPtr  _manager);


    ~KKJob ();


    /**************************************************************************/
    /*   Types and methods required for supporting KKJob factory creation.    */
    /**************************************************************************/
    typedef  KKJobPtr (*ConstructorPtr)(JobManagerPtr _manager);

    typedef  map<KKStr, ConstructorPtr>  ConstructorIndex;


    /** 
     * Tracks the different constructors that can create new KKJob derived classes.  When reading the status file 
     * reading the status file the name of the derived KKJob class will be specified; using the following data structure
     * 'CallAppropriateConstructor' will know which constructor to use.
     */
    static  ConstructorIndex  registeredConstructors;  

    static  void  RegisterConstructor (const KKStr&    _name,
                                       ConstructorPtr  _constructor
                                      );

    /**
     * Given name of KKJob derived class('_jobTypeName') and its initial values in '_statusStr' will call registered constructor.
     */
    static  KKJobPtr  CallAppropriateConstructor (JobManagerPtr  _manager,
                                                  const KKStr&   _jobTypeName,
                                                  const KKStr&   _statusStr
                                                 );



    //***************************************** Access Methods **********************************************
    kkint32            JobId         () const  {return jobId;}
    kkint32            ParentId      () const  {return parentId;}
    JobManagerPtr      Manager       () const  {return manager;}
    const VectorInt&   Prerequisites () const  {return prerequisites;}
    RunLog&            Log           ()        {return log;}
    JobStatus          Status        () const  {return status;}
    KKStr              StatusStr     () const;

    void   JobId     (kkint32    _jobId)   {jobId   = _jobId;}
    void   Status    (JobStatus  _status)  {status  = _status;}



    //**************************  Methods that derived classes must implement  ******************************
    virtual  KKJobPtr  Duplicate ()  const;  /**< Create a duplicate instance. */


    virtual  const char*   JobType ()  const;     /**< Specifies the derived KKJob class; derived classes should return the their name. */

    virtual  void       ProcessNode ();

    virtual  void       ReFresh (KKJob&  j);

    virtual  KKStr      ToStatusStr ();      /**< Derived classes that implement this method must also call its base class version 1st. */


    /**
     * @brief Implementation specific field processing.
     * @details All derived classes of KKJob  are required to  define this method; whenever the 'ProcessStatusStr' method can not identify a field
     *  it will call this method to let the child Class process the field.
     */
    virtual
      void  ProcessStatusField (const KKStr&  fieldName,
                                const KKStr&  fieldValue
                               );


    /**
     * @brief  Write out completed job results to status file.
     * @details This method will get called right after the "KKJob" status line gets written when a KKJob is completed.  
     *  See 'KKJobManager::GetNextSetOfJobs'.  If a job needs to write more data to the Status file then you want to 
     *  put on a single status line this is where you would do it.   You write all the text in a format that you 
     *  want to support.  'KKJobManager' will bracket it with <KKJob JobType=KKJob::JobType, JobId=####>   and </KKJob>
     *@code
     * ex:
     *<RandomSplitJob>
     *    KKJob Data;  
     *    KKJob Data,
     *    etc. etc. etc.
     *</RandomSplitJob>
     *@endcode
     */
    virtual  
      void  CompletedJobDataWrite (ostream& o);


    /**@
     *@brief Works with 'WriteCompletedJobData';  You use this to load in data written by 'WriteCompletedJobData'
     */
    virtual  
      void  CompletedJobDataRead (istream& i);



    //*******************************************************************************************************


    void  AddPrerequisites (kkint32    _prerequisite);
    void  AddPrerequisites (VectorInt  _prerequisites);

    bool  InPrerequisites (kkint32 _jobId);   /**< Returns true if  '_jobId' is in 'prerequisites'. */

    static   KKStr      JobStatusToStr   (JobStatus  status);
    static   JobStatus  JobStatusFromStr (const KKStr&  statusStr);

    static   KKStr      JobTypeToStr   (JobTypes jt);
    static   void       JobTypeFromStr (const KKStr&  s);


    RunLog&  log;


  protected:
    KKStr  PrerequisitesToStr ()  const;
    void   PrerequisitesFromStr (const KKStr&  s);

    void   ProcessStatusStr (const KKStr&  statusStr);

    kkint32         jobId;
    kkint32         parentId;
    JobManagerPtr   manager;
    kkint32         numProcessors;         /**< Number of CPU's that are currently processing this node; that is 
                                            * the number that are currently calling the 'ProcessNode' method.
                                            */

    kkint32         numPorcessesAllowed;   /**< The number of Processes that are allowed to process at same time.
                                            * that is ('numPorcessesAllowed' <= 'numActiveProcessors').
                                            */

    VectorInt       prerequisites;         /**< List of JobId's that must complete (jobStatus == jsDone) before this 
                                            * process may be started.
                                            */

    JobStatus       status;
  };  /* KKJob */

  typedef  KKJob::KKJobPtr  KKJobPtr;

  #define  _KKJobDefined_



  class  KKJobList: public KKQueue<KKJob>
  {
  public:
    typedef  KKJobList*  KKJobListPtr;

    KKJobList (const KKJobList& jobs);

    KKJobList (JobManagerPtr  _manager);

    bool              AllPrequisitesDone (KKJobPtr job);

    bool              AreAllJobsDone ();

    bool              JobsStillRunning (); 

    KKJobPtr          LocateOpenJob ();

    RunLog&           Log ()  {return log;}

    KKJobPtr          LookUpByJobId (kkint32  jobId);

    void              PushOnBack (KKJobPtr  j);



  private:
    RunLog&          log;
    JobManagerPtr    manager;

    map<kkint32,KKJobPtr>              jobIdLookUpTable;
    map<kkint32,KKJobPtr>::iterator    jobIdLookUpTableIdx;
    typedef pair <kkint32, KKJobPtr>   JobIdLookUpTablePair;

  };  /* KKJobList */

  typedef  KKJobList::KKJobListPtr  KKJobListPtr;

  #define  _JobListDefined_
}  /* KKJobManagment */

#endif
