#ifndef  _FEATUREFILEIO_
#define  _FEATUREFILEIO_


/**
 *@class  KKMachineLearning::FeatureFileIO
 *@brief  Base class for all FeatureFileIO classes.
 *@details  This is a abstract class.  For each type of FeatureFile you will need to implement
 *          a separate class derived from this class that supports the specific file format.
 *          You only need to implement the pure virtual functions.
 *
 *          If you create a new FeatureFileIO class you will need to modify 'RegisterAllDrivers'
 *          method in FeatureFileIO.cpp.
 */

#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "GoalKeeper.h"
#include "MLClass.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"

namespace KKMachineLearning 
{
  class  FeatureFileIO;
  typedef  FeatureFileIO*  FeatureFileIOPtr;


  #if  !defined(_FeatureVector_Defined_)
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  #endif


  #if  !defined(_FeatureVectorList_Defined_)
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #if  !defined(_FactoryFVProducer_Defined_)
  class  FactoryFVProducer;
  typedef  FactoryFVProducer*  FactoryFVProducerPtr;
  #endif


  class  FeatureFileIO
  {
  public:
    typedef  FeatureFileIO*  FeatureFileIOPtr;

    FeatureFileIO (const KKStr&  _driverName,
                   bool          _canRead,
                   bool          _canWrite
                  );

    virtual ~FeatureFileIO ();

    bool  CanRead  ()  {return  canRead;}
    bool  CanWrite ()  {return  canWrite;}


    void   AppendToFile (const KKStr&           _fileName,
                         const FeatureNumList&  _selFeatures,
                         FeatureVectorList&     _examples,
                         kkuint32&              _numExamplesWritten,
                         VolConstBool&          _cancelFlag,
                         bool&                  _successful,
                         RunLog&                log
                        );


    /**
     *@brief Loads the contents of a feature data file and returns a ImageFeaturesList container object.
     *@param[in]  _fileName   Feature file that is being synchronized.
     *@param[in,out] _mlClasses All classes encountered during the loading of the feature file will be added to this list.
     *@param[in]  _maxCount Maximum number of examples to load, -1 = load all
     *@param[in]  _cancelFlag If this flag turns true the load will terminate and return to caller.
     *@param[out] _successful False will be returned if load failed.
     *@param[out] _changesMade If the routine had loaded the feature data determined that it needed to make
     *            changes this flag will be set 'true'.
     *@param[in]  _log Where to send diagnostic messages to.
     *@return  A ImageFeaturesList container object; this object will own all the examples loaded.
     */
    virtual  
      FeatureVectorListPtr  LoadFeatureFile (const KKStr&   _fileName,
                                             MLClassList&   _mlClasses,
                                             kkint32        _maxCount,
                                             VolConstBool&  _cancelFlag,    // will be monitored,  if set to True  Load will terminate.
                                             bool&          _successful,
                                             bool&          _changesMade,
                                             RunLog&        _log
                                            );

      
     /**                       SaveFeatureFile
     *@brief Save examples to 'fileName'
     *@param[in]  _fileName  Name of file top same examples/images to.
     *@param[in]  _selFeatures Specify specific features to save, typically all features.
     *@param[in]  _examples Examples that are to be saved.
     *@param[in]  _numExamplesWritten Will reflect the num examples written, caller will be able to monitor.
     *@param[in]  _cancelFlag If this flag turns true the writing of data will terminate and return to caller.
     *@param[out] _successful False will be returned if the save failed.
     *@param[in]  _log log file to send messages to.
     */
    virtual
      void  SaveFeatureFile (const KKStr&           _fileName, 
                             const FeatureNumList&  _selFeatures,
                             FeatureVectorList&     _examples,
                             kkuint32&              _numExamplesWritten,  // caller will be able to manitor this variable.
                             VolConstBool&          _cancelFlag,
                             bool&                  _successful,
                             RunLog&                _log
                            );


    /**
     *@brief Saves the feature file in multiple parts with no one single part larger that 64k examples.
     *@details Same as 'SaveFeatureFile', if more than 64k examples will break into multiple files.
     *         If there are more than 64k examples, will save all images into 'fileName', but also
     *         a second copy of them into files with same name plus seq num with max of 64k samples
     *         in each one.
     *
     *@param[in]  _fileName  Name of file top same examples/images to.
     *@param[in]  _selFeatures Specify specific features to save, typically all features.
     *@param[in]  _examples Examples that are to be saved.
     *@param[in]  _cancelFlag If this flag turns true the writing of data will terminate and return to caller.
     *@param[out] _successful False will be returned if the save failed.
     *@param[in]  _log log file to send messages to.
     */
    void  SaveFeatureFileMultipleParts (const KKStr&           _fileName, 
                                        const FeatureNumList&  _selFeatures,
                                        FeatureVectorList&     _examples,
                                        VolConstBool&          _cancelFlag,
                                        bool&                  _successful,
                                        RunLog&                _log
                                       );



    /**                       FeatureDataReSink
     *@brief Synchronizes the contents of a feature data file with a directory of images.
     *@details  Used with  applications to verify that feature file is up-to-date.
     * Was specifically meant to work with training libraries, to account for
     * images being added and deleted from training library.  If there are no 
     * changes, then function will run very quickly.
     *@param[in] _fvProducerFactory  Factory that specifoes the FeatureVector's we want to produce.
     *@param[in] _dirName,      Directory where source images are located.
     *@param[in] _fileName,     Feature file that is being synchronized.
     *@param[in] _unknownClass, Class to be used when class is unknown
     *@param[in] _useDirectoryNameForClassName, if true then class name of each entry will be set to directory name.
     *@param[in] _mlClasses,  list of classes
     *@param[in]  _cancelFlag  Will be monitored; if it goes to 'true'  will exit as soon as possible.
     *@param[out] _changesMade, If returns as true then there were changes made to the 
     *             feature file 'fileName'.  If set to false, then no changes were made.
     *@param[out] _timeStamp of feature file.
     *@param[in]  _log where to send diagnostic messages to.
     *@returnz  A FeatureVectorList derived instance ; This object will own all the examples loaded
     *
     * A change in feature file version number would also cause all entries in the feature
     * file to be recomputed.  The feature file version number gets incremented whenever we change
     * the feature file computation routine.
     */
    virtual
    FeatureVectorListPtr  FeatureDataReSink (FactoryFVProducerPtr  _fvProducerFactory,
                                             const KKStr&          _dirName, 
                                             const KKStr&          _fileName, 
                                             MLClassPtr            _unknownClass,
                                             bool                  _useDirectoryNameForClassName,
                                             MLClassList&          _mlClasses,
                                             VolConstBool&         _cancelFlag,
                                             bool&                 _changesMade,
                                             KKB::DateTime&        _timeStamp,
                                             RunLog&               _log
                                            );


    /**                       LoadInSubDirectoryTree
     *@brief Creates a feature vector list of all images located in the specified sub-directory tree.
     *@details Meant to work with images, it starts at a specified sub-directory and
     *        processes all sub-directories.  It makes use of FeatureDataReSink for each specific
     *        sub-directory.  Will make use of FeatureData files that already exist in any of the
     *        sub-directories.
     *@param[in] _fvProducerFactory  Factory that specifoes the FeatureVector's we want to produce.
     *@param[in] _rootDir  Starting directory.
     *@param[in,out] _mlClasses, List of classes, any new classes in fileName will be added.
     *@param[in] _useDirectoryNameForClassName, if true set class names to sub-directory name.
     *           This happens because the user may manually move images between directories using
     *           the sub-directory name as the class name.
     *@param[in] _cancelFlag  If turns to 'true' method is to exit asap.
     *@param[in] _rewiteRootFeatureFile, If true rewrite the feature file in the specified 'rootDir'.  This
     *           feature file will contain all entries from all sub-directories below it.
     *@param[in] _log, where to send diagnostic messages to.
     *@returns - A PostLarvaeFVList container object.  This object will own all the examples loaded.
     */
    FeatureVectorListPtr  LoadInSubDirectoryTree (FactoryFVProducerPtr  _fvProducerFactory,
                                                  KKStr                 _rootDir,
                                                  MLClassList&          _mlClasses,
                                                  bool                  _useDirectoryNameForClassName,
                                                  VolConstBool&         _cancelFlag,    /**< will be monitored, if set to True  Load will terminate. */
                                                  bool                  _rewiteRootFeatureFile,
                                                  RunLog&               _log
                                                 );





    //***************************************************************************
    //*    The following routines need to be implemented by derived classes.   *
    //***************************************************************************

    /**
     *@brief  Create a FileDesc object from the input stream '_in'.
     *@details  All derived classes must implement this method.  It is called by 'LoadFeatureFile'
     *          before it starts reading in the feature data.  
     *@param[in]  _fileName  Name of file to read top get FileDesc data from.  Ex  in c45 this would be the names file.
     *@param[in]  _in        Input Stream t read from.
     *@param[out] _classes   Must be pointing to a valid MLClassList object.  As class names are encountered add them to this list.
     *@param[out] _estSize   If you can drive the number of examples in the feature file populate this parameter.
     *@param[out] _errorMessage  If a error in processing occurs; place a description of the error in this parameter.
     *@param      _log
     */
    virtual  FileDescPtr  GetFileDesc (const KKStr&    _fileName,
                                       std::istream&   _in,
                                       MLClassListPtr  _classes,
                                       kkint32&        _estSize,
                                       KKStr&          _errorMessage,
                                       RunLog&         _log
                                      ) = 0;







    /**
     *@brief To be implemented by derived classes; loads the contents of a feature data file and returns a ImageFeaturesList container object.
     *@param[in]  _fileName   Feature file that is being loaded.
     *@param[in]  _fileDesc  Description of feature data that is to be loaded.
     *@param[in,out] _classes All classes encounter during the loading of the feature file will be added to this list.
     *@param[in]  _in  input stream that feature data is to be loaded/read from.
     *@param[in]  _maxCount Maximum number of examples to load, -1 = load all
     *@param[in]  _cancelFlag If this flag turns true the load will terminate and return to caller.
     *@param[out] _changesMade If the routine had loaded the feature data determined that it needed to make
     *            changes this flag will be set 'true'.
     *@param[out] _errorMessage If an error occurs during the loading a description of this error will be placed here.
     *@param[in]  _log Where to send diagnostic messages to.
     *@return  A ImageFeaturesList container object; this object will own all the examples loaded;  if an error occurs NULL will be returned.
     */
    virtual  FeatureVectorListPtr  LoadFile (const KKStr&       _fileName,
                                             const FileDescPtr  _fileDesc,
                                             MLClassList&       _classes, 
                                             std::istream&      _in,
                                             kkint32            _maxCount,    /**< Maximum # images to load. */
                                             VolConstBool&      _cancelFlag,
                                             bool&              _changesMade,
                                             KKStr&             _errorMessage,
                                             RunLog&            _log
                                            ) = 0;


    /**
     *@brief  To be implemented by derived classes; save examples to output stream '_out'.
     *@param[in]  _data  Examples that are to be written to saved to the output stream.
     *@param[in]  _fileName  Name of file top same examples/images to.
     *@param[in]  _selFeatures Specify specific features to save, typically all features.
     *@param[out] _out  Output stream to save feature data to.
     *@param[out] _numExamplesWritten Will reflect the num examples written, caller will be able to monitor.
     *@param[in]  _cancelFlag If this flag turns true the writing of data will terminate and return to caller.
     *@param[out] _successful False will be returned if the save failed.
     *@param[out] _errorMessage If the save fails (_successful == false)  then a description of the error will be placed here.
     *@param[in]  _log log file to send messages to.
     */
    virtual  void   SaveFile (FeatureVectorList&      _data,
                              const KKStr&            _fileName,
                              const FeatureNumList&   _selFeatures,
                              std::ostream&           _out,
                              kkuint32&               _numExamplesWritten,
                              VolConstBool&           _cancelFlag,
                              bool&                   _successful,
                              KKStr&                  _errorMessage,
                              RunLog&                 _log
                             ) = 0;



    const  KKStr&   DriverName ()  {return  driverName;}



    static  FeatureFileIOPtr   FileFormatFromStr   (const KKStr&  _fileFormatStr);

    static  FeatureFileIOPtr   FileFormatFromStr   (const KKStr&  _fileFormatStr,
                                                    bool          _canRead,
                                                    bool          _canWrite
                                                   );

    static  KKStr              FileFormatsReadOptionsStr ();

    static  KKStr              FileFormatsWrittenOptionsStr ();

    static  KKStr              FileFormatsReadAndWriteOptionsStr ();

    static  VectorKKStr        RegisteredDriverNames ();

    static  void               FinalCleanUp ();

    /**
     *@brief  For each feature file format register the appropriate driver thru this static method.
     *@details  You will be giving ownership of the driver to this class; it will call the destructor 
     *when the application shutsdown.
     */
    static  void               RegisterFeatureFileIODriver (FeatureFileIOPtr  _driver);
      

  protected:
    /**
     *@brief Will retrieve the next token from the input stream. 
     *@details Leading and trailing blank characters will be skipped. A token will be separated
     *         by any character in '_delimiters' or 'EndOfLine', or 'EndOfFile'.  If a 'EndOfLine'
     *         or 'EndOfFile' occur while reading in a token the respective flags  '_eol'  and 
     *         '_eof'  will be set to false but the following call to this function will set the 
     *         respective flag to true and return a empty token.
     *
     *@param[in]  _in          Stream to read from,
     *@param[in]  _delimiters  List of valid delimiter characters.
     *@param[out] _token       token extracted from '_in'.  If either '_eof' or '_eol'
     *                         are set to true; then token will be empty.
     *@param[out] _eof         Set true if at end of file;
     *@param[out] _eol         Set true if at end of line.  
     */
     void  GetToken (std::istream&  _in,
                     const char*    _delimiters,
                     KKStr&         _token,
                     bool&          _eof, 
                     bool&          _eol
                    );


     void  GetLine (std::istream&  _in,
                    KKStr&         _line,
                    bool&          _eof
                   );

    static  void  RegisterDriver (FeatureFileIOPtr  driver);

  private:
    bool    canRead;
    bool    canWrite;
    KKStr   driverName;
    KKStr   driverNameLower;

    static void  RegisterAllDrivers ();


  static  bool  atExitDefined;
  static  std::vector<FeatureFileIOPtr>*  registeredDrivers;

  static  std::vector<FeatureFileIOPtr>*  RegisteredDrivers  ();

  static
    FeatureFileIOPtr  LookUpDriver (const KKStr&  _driverName);
  };  /* FeatureFileIO */



  typedef  FeatureFileIO::FeatureFileIOPtr   FeatureFileIOPtr;

  #define  _FeatureFileIO_Defined_


}  /* namespace KKMachineLearning  */



#endif



