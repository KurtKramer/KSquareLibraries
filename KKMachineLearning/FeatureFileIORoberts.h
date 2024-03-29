#ifndef  _FEATUREFILEIOROBERTS_
#define  _FEATUREFILEIOROBERTS_

#include  "FeatureFileIO.h"


namespace KKMLL
{


  /**
    @class  FeatureFileIORoberts
    @brief  Supports the writing of Feature Data to a file that can then be read by OpenDT.
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

    virtual  void   SaveFile (FeatureVectorList&    _data,
                              const KKStr&          _fileName,
                              FeatureNumListConst&  _selFeatures,
                              ostream&              _out,
                              kkuint32&             _numExamplesWritten,
                              VolConstBool&         _cancelFlag,
                              bool&                 _successful,
                              KKStr&                _errorMessage,
                              RunLog&               _log
                             )  override;

  private:
    static  FeatureFileIORoberts  driver;

  };

}  /* namespace KKMLL */

#endif
