#ifndef  _FEATUREFILEIOKK_
#define  _FEATUREFILEIOKK_

#include  "FeatureFileIO.h"
#include  "PostLarvaeFV.h"


namespace KKMachineLearning 
{

  /**
    @class  FeatureFileIOKK
    @brief  Supports the reading and writing of feature data from "KK" feature data files.
    @details
    @code
    * *************************************************************************************************
    * *  The KK feature data file is a very simple format where the first few rows contain a          *
    * *  description of the rest of the data file.  Any row that starts with "//" will be treated as  *
    * *  comment line.                                                                                *
    * *  First Row:  Identifies the file as a feature Data File.  An example on the next line.        *
    * *              "FEATURE_DATA_FILE Version	101	NumOfFeatures	20	NumOfExamples  258"             *
    * *              Each field will be separated by a tab character.  This line says that the        *
    * *              version number is 101. There are 20 features and 258 examples or feature vectors.*
    * *              The version number gets incremented whenever there is a change in the way        *
    * *              tha feature data is computed.                                                    *
    * *                                                                                               *
    * *  Second Row: List all the fields in the feature data file.  The first fields are feature      *
    * *              data.  The first non feature field will be: "ClassName".  There are several non  *
    * *              feature data fields.                                                             *
    * *************************************************************************************************
    @endcode
    @see  FeatureFileIO
    */
  class  FeatureFileIOKK:  public  FeatureFileIO
  {
  public:
    typedef  FeatureFileIOKK*  FeatureFileIOKKPtr;

    FeatureFileIOKK ();
    virtual ~FeatureFileIOKK ();

    virtual  FileDescPtr  GetFileDesc (const KKStr&    _fileName,
                                       std::istream&   _in,
                                       MLClassListPtr  _classes,
                                       kkint32&        _estSize,
                                       KKStr&          _errorMessage,
                                       RunLog&         _runLog
                                      );


    virtual  PostLarvaeFVListPtr  LoadFile (const KKStr&       _fileName,
                                            const FileDescPtr  _fileDesc,
                                            MLClassList&       _classes, 
                                            std::istream&      _in,
                                            kkint32            _maxCount,    // Maximum # images to load.
                                            VolConstBool&      _cancelFlag,
                                            bool&              _changesMade,
                                            KKStr&             _errorMessage,
                                            RunLog&            _log
                                           );


    virtual  void   SaveFile (FeatureVectorList&     _data,
                              const KKStr&           _fileName,
                              const FeatureNumList&  _selFeatures,
                              std::ostream&          _out,
                              kkuint32&              _numExamplesWritten,
                              VolConstBool&          _cancelFlag,
                              bool&                  _successful,
                              KKStr&                 _errorMessage,
                              RunLog&                _log
                             );


    static  FeatureFileIOKKPtr  Driver () {return &driver;}
   

    /**                       FeatureDataReSink
     * @brief Synchronizes the contents of a feature data file with a directory of images.
     *        Used with  applications to verify that feature file is up-to-date.
     *        Was specifically meant to work with training libraries, to account for
     *        images being added and deleted from training library.  If there are no 
     *        changes, then function will run very quickly.
     * @param[in] dirName,      Directory where source images are located.
     * @param[in] fileName,     Feature file that is being synchronized.
     * @param[in] unknownClass, Class to be used when class is unknown
     * @param[in] useDirectoryNameForClassName, if true then class name of each entry
     *            will be set to directory name.
     * @param[in] mlClasses,  list of classes
     * @param[in] log, where to send diagnostic messages to.
     * @param[out] changesMade, If returns as true then there were changes made to the 
     *             feature file 'fileName'.  If set to false, then no changes were made.
     * @param[out] Timestamp of feature file.
     * @return - A PostLarvaeFVList container object.  This object will own all the examples loaded
     *
     * A change in feature file version number would also cause all entries in the feature
     * file to be recomputed.  The feature file version number gets incremented whenever we change
     * the feature file computation routine.
     */
    static
    PostLarvaeFVListPtr  FeatureDataReSink (KKStr           _dirName, 
                                            const KKStr&    _fileName, 
                                            MLClassPtr      _unknownClass,
                                            bool            _useDirectoryNameForClassName,
                                            MLClassList&    _mlClasses,
                                            VolConstBool&   _cancelFlag,    // will be monitored,  if set to True  Load will terminate.
                                            bool&           _changesMade,
                                            KKB::DateTime&  _timeStamp,
                                            RunLog&         _log
                                           );






    /**                       LoadInSubDirectoryTree
     * @brief Creates a feature vector list of all images located in the specified sub-directory tree.
     * @details Meant to work with images, it starts at a specified sub-directory and
     *        processes all sub-directories.  It makes use of FeatureDataReSink for each specific
     *        sub-directory.  Will make use of FeatureData files that already exist in any of the
     *        sub-directories.
     * @param[in] rootDir  Starting directory.
     * @param[in,out] mlClasses, List of classes, any new classes in fileName will be added.
     * @param[in] useDirectoryNameForClassName, if true set class names to sub-directory name.
     *            This happens because the user may manually move images between directories using
     *            the sub-directory name as the class name.
     * @param[in] log, where to send diagnostic messages to.
     * @param[in] rewiteRootFeatureFile, If true rewrite the feature file in the specified 'rootDir'.  This
     *            feature file will contain all entries from all sub-directories below it.
     * @return - A PostLarvaeFVList container object.  This object will own all the examples loaded.
     */
    static
      PostLarvaeFVListPtr  LoadInSubDirectoryTree (KKStr          _rootDir,
                                                   MLClassList&   _mlClasses,
                                                   bool           _useDirectoryNameForClassName,
                                                   VolConstBool&  _cancelFlag,    /**< will be monitored, if set to True  Load will terminate. */
                                                   bool           _rewiteRootFeatureFile,
                                                   RunLog&        _log
                                                  );



  private:
    static FeatureFileIOKK  driver;


    typedef  enum  {rfFeatureData,
                    rfClassName,
                    rfImageClassIDX,
                    rfImageFileName, 
                    rfOrigSize,
                    rfNumOfEdgePixels,
                    rfProbability,
                    rfCentroidRow,
                    rfCentroidCol,
                    rfPredictedClass,
                    rfPredictedClassIDX,
                    rfBreakTie,
                    rfUnKnown
                   }  
                   KK_FieldTypes;



    void  Parse_FEATURE_DATA_FILE_Line (KKStr&    line,
                                        kkint32&  version,
                                        kkint32&  numOfFields,
                                        kkint32&  numOfExamples
                                       );


    VectorInt  CreateIndirectionTable (const VectorKKStr&  fields,
                                       kkint32             numOfFeatures
                                      );

  };

  typedef  FeatureFileIOKK::FeatureFileIOKKPtr  FeatureFileIOKKPtr;

}  /* namespace KKMachineLearning */


#endif
