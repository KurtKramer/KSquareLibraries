#ifndef  _FEATUREFILEIOROBERTS_
#define  _FEATUREFILEIOROBERTS_
/**
  ************************************************************************************************
  *  @author  Kurt Krame                                                                         *
  *  Date:  2009-04-16                                                                           *
  ************************************************************************************************
  */

#include  "FeatureFileIO.h"


namespace KKMachineLearning
{


  /**
    @class  FeatureFileIORoberts
    @brief  Supports the writting of Feature Data to a file that can then be read by OpenDT.
    @details
    @code
    * ************************************************************************************************
    * *  FeatureFileIOSRoberts  Sub-classed from FeatureFileIO.  It supports the writing of a file   *
    * *  that can be read by OpenDT. http://opendt.sourceforge.net/.  This file format is similar to *    
    * *  C45 except the contents of the names file are included in the data file.                    *
    * ************************************************************************************************
    @endcode
    * @see  FeatureFileIO
    */
  class FeatureFileIORoberts:  public  FeatureFileIO
  {
  public:
    FeatureFileIORoberts ();
    ~FeatureFileIORoberts ();

    typedef  FeatureFileIORoberts*  FeatureFileIORobertsPtr;

    static  FeatureFileIORobertsPtr  Driver ()  {return &driver;}

    virtual  FileDescPtr  GetFileDesc (const KKStr&       _fileName,
                                       istream&           _in,
                                       MLClassListPtr  _classList,
                                       int32&             _estSize,
                                       KKStr&             _errorMessage,
                                       RunLog&            _log
                                      );


    virtual  FeatureVectorListPtr  LoadFile (const KKStr&          _fileName,
                                             const FileDescPtr     _fileDesc,
                                             MLClassList&       _classes,
                                             istream&              _in,
                                             int32                 _maxCount,    // Maximum # images to load.
                                             volatile const bool&  _cancelFlag,
                                             bool&                 _changesMade,
                                             KKStr&                _errorMessage,
                                             RunLog&               _log
                                            );


    virtual  void   SaveFile (FeatureVectorList&     _data,
                              const KKStr&           _fileName,
                              const FeatureNumList&  _selFeatures,
                              ostream&               _out,
                              uint32&                _numExamplesWritten,
                              volatile const bool&   _cancelFlag,
                              bool&                  _successful,
                              KKStr&                 _errorMessage,
                              RunLog&                _log
                             );



  private:
    static  FeatureFileIORoberts  driver;

  };

}  /* namespace KKMachineLearning */

#endif
