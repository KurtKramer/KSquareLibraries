#ifndef  _FEATUREFILEIOARFF_
#define  _FEATUREFILEIOARFF_
/**
  ************************************************************************************************
  *  @author  Kurt Kramer                                                                        *
  *  Date:  2009-04-16                                                                           *
  ************************************************************************************************
  */


#include  "FeatureFileIO.h"


namespace KKMLL
{

/**
 * @class  KKMLL::FeatureFileIOArff
 * @brief Support the writing of ARFF Formatted Feature Files.
 * @details
 * Supports the writing of the ARFF format as supported by the WEKA machine learning system
 * The ARFF format is very similar to the C45 file format; but there is only one file that
 * contains the file description and examples in it.
 * see http://weka.wikispaces.com/ARFF+(book+version  for description.
 * @see  FeatureFileIO
 */
class FeatureFileIOArff:  public  FeatureFileIO
{
public:
  typedef  FeatureFileIOArff*  FeatureFileIOArffPtr;


  FeatureFileIOArff ();

  ~FeatureFileIOArff ();

  static   FeatureFileIOArffPtr  Driver ()  {return &driver;}



private:
  virtual  FileDescConstPtr  GetFileDesc (const KKStr&    _fileName,
                                          istream&        _in,
                                          MLClassListPtr  _classList,
                                          kkint32&        _estSize,
                                          KKStr&          _errorMessage,
                                          RunLog&         _log
                                         );


  virtual  FeatureVectorListPtr  LoadFile (const KKStr&      _fileName,
                                           FileDescConstPtr  _fileDesc,
                                           MLClassList&      _classes, 
                                           istream&          _in,
                                           kkint32           _maxCount,    // Maximum # images to load.
                                           VolConstBool&     _cancelFlag,
                                           bool&             _changesMade,
                                           KKStr&            _errorMessage,
                                           RunLog&           _log
                                          );


  virtual  void   SaveFile (FeatureVectorList&    _data,
                            const KKStr&          _fileName,
                            FeatureNumListConst&  _selFeatures,
                            ostream&              _out,
                            kkuint32&             _numExamplesWritten,
                            VolConstBool&         _cancelFlag,
                            bool&                 _successful,
                            KKStr&                _errorMessage,
                            RunLog&               _log
                           );



private:
  static  FeatureFileIOArff  driver;

};

}  /* namespace KKMLL */

#endif

