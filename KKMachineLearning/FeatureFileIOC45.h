#ifndef  _FEATUREFILEIOC45_
#define  _FEATUREFILEIOC45_

#include  "FeatureFileIO.h"

namespace KKMLL
{
/**
 *@class  FeatureFileIOC45
 *@brief  Supports the reading and writing of feature data from C45 formated feature files.
 *@details
 *@code
 *************************************************************************************************
 **  FeatureFileIODC45   Sub-classed from FeatureFileIO.  Supports the reading and writing of    *
 **  the C45 file format.  For each data set there will be two files.  'Names'  and  'Data'      *
 **  http://www.cs.washington.edu/dm/vfml/appendixes/c45.htm  for description                    *
 *************************************************************************************************
 *@endcode
 *@see  FeatureFileIO
 */
class  FeatureFileIOC45: public FeatureFileIO
{
public:
  FeatureFileIOC45 ();

  ~FeatureFileIOC45 ();

  typedef  FeatureFileIOC45*  FeatureFileIOC45Ptr;

  static  FeatureFileIOC45Ptr  Driver ()  {return &driver;}

  virtual  FileDescPtr  GetFileDesc (const KKStr&    _fileName,
                                     istream&        _in,
                                     MLClassListPtr  _classList,
                                     kkint32&        _estSize,
                                     KKStr&          _errorMessage,
                                     RunLog&         log
                                    );


  virtual  
    FeatureVectorListPtr  LoadFeatureFile (const KKStr&   _fileName,
                                           MLClassList&   _mlClasses,
                                           kkint32        _maxCount,
                                           VolConstBool&  _cancelFlag,
                                           bool&          _successful,
                                           bool&          _changesMade,
                                           RunLog&        _log
                                          );



  virtual  
    FeatureVectorListPtr  LoadFile (const KKStr&       _fileName,
                                    const FileDescPtr  _fileDesc,
                                    MLClassList&       _classes, 
                                    istream&           _in,
                                    kkint32            _maxCount,    // Maximum # images to load.
                                    VolConstBool&      _cancelFlag,
                                    bool&              _changesMade,
                                    KKStr&             _errorMessage,
                                    RunLog&            _log
                                   );


  virtual  
    void   SaveFile (FeatureVectorList&    _data,
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
  static  FeatureFileIOC45  driver;

  KKStr  C45AdjName (const  KKStr&  oldName);

  void  C45StripComments (KKStr&  ln);

  void  C45StrPreProcessName (KKStr&  ln);

  kkint32  C45LocateNextCharacter (const KKStr& txt,
                                   char          ch
                                  );

  void  C45ConstructFileNameForWritting (const KKStr&  fileName,
                                         KKStr&        namesFileName,
                                         KKStr&        dataFileName
                                        );

  KKStr  C45ReadNextToken (istream&     in, 
                           const char*  delimiters,
                           bool&        eof,
                           bool&        eol
                          );

  void  ProcessC45AttrStr (FileDescPtr  fileDesc,
                           KKStr&       attrStr,
                           bool&        validStr,
                           RunLog&      _log
                          );

};  /* FeatureFileIOC45 */


typedef  FeatureFileIOC45::FeatureFileIOC45Ptr   FeatureFileIOC45Ptr;

}  /* namespace KKMLL */
#endif
