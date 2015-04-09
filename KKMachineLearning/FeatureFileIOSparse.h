#ifndef  _FEATUREFILEIOSPARSE_
#define  _FEATUREFILEIOSPARSE_

#include  "FeatureFileIO.h"


namespace KKMachineLearning
{

  /**
    @brief  Supports the reading and writing of Sparse feature files similar to the ones libSVM use.
    @details
    @code
    * *************************************************************************************************
    * *  FeatureFileIOSparse  Sub-classed from FeatureFileIO.  It supports the reading and writing    *
    * *  of Sparse Files as used by the libSVM.                                                       *
    * *                                                                                               *
    * *  One example per row.                                                                         *
    * *  any row that starts with '//' is ignored.                                                    *
    * *  On each row there will be several fields separated by the space(' ') character. The first    *
    * *  field will be the class name followed by the attribute data.                                 *
    * *                                                                                               *
    * *   - First Field is the Class Name.  This differers from libSVM which assumes that class-names *
    * *     are integers.  We support alpha numeric class names.                                      *
    * *   - Attribute Fields will be of the following format.  <FeatureNum>:<Feature value>.          *
    * *                                                                                               *
    * *  ex:                                                                                          *
    * *     "0 0:12.12 4:87.1242 12:-65.63 15:0.872"                                                  *
    * *     "1 2:-76.92 12:65.882 26:76"                                                              *
    * *     "1 1:28.74 3:8.452 11:22.72"                                                              *
    * *                                                                                               *
    * *************************************************************************************************
    @endcode
    @see  FeatureFileIO
    */
  class FeatureFileIOSparse: public FeatureFileIO
  {
  public:
    FeatureFileIOSparse ();
    ~FeatureFileIOSparse ();

    typedef  FeatureFileIOSparse*  FeatureFileIOSparsePtr;

    static  FeatureFileIOSparsePtr  Driver ()  {return &driver;}


    virtual  FileDescPtr  GetFileDesc (const KKStr&    _fileName,
                                       istream&        _in,
                                       MLClassListPtr  _classList,
                                       kkint32&        _estSize,
                                       KKStr&          _errorMessage,
                                       RunLog&         _log
                                      );


    virtual  FeatureVectorListPtr  LoadFile (const KKStr&       _fileName,
                                             const FileDescPtr  _fileDesc,
                                             MLClassList&       _classes, 
                                             istream&           _in,
                                             kkint32            _maxCount,    // Maximum # images to load.
                                             VolConstBool&      _cancelFlag,
                                             bool&              _changesMade,
                                             KKStr&             _errorMessage,
                                             RunLog&            _log
                                            );


    virtual  void   SaveFile (FeatureVectorList&      _data,
                              const KKStr&            _fileName,
                              const FeatureNumList&   _selFeatures,
                              ostream&                _out,
                              kkuint32&               _numExamplesWritten,
                              VolConstBool&           _cancelFlag,
                              bool&                   _successful,
                              KKStr&                  _errorMessage,
                              RunLog&                 _log
                             );



  private:
    static  FeatureFileIOSparse  driver;

  };  /* FeatureFileIOSparse */

}  /* namespace KKMachineLearning */

#endif
