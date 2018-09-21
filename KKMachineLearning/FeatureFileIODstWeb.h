#ifndef  _FEATUREFILEIODSTWEB_
#define  _FEATUREFILEIODSTWEB_

#include  "FeatureFileIO.h"

namespace KKMLL
{
  /**
    ************************************************************************************************
    *  FeatureFileIODstWeb    Sub-classed from FeatureFileIO.  It supports the reading of the      *
    *  "dst" file format used by for the Anonymous web data from www.microsoft.com.                *
    *  For detailed description go to http://kdd.ics.uci.edu/databases/msweb/msweb.data.html       *
    *  In addition to the layout described above you will need to add a line at the top of the     *
    *  file of the format "ClassName = xxxx"  where xxxx = the name of the attribute that will be  *
    *  used as the Class-Name.                                                                     *
    ************************************************************************************************
    * @see  FeatureFileIO
    */
  class FeatureFileIODstWeb: public FeatureFileIO
  {
  public:
    FeatureFileIODstWeb ();
    ~FeatureFileIODstWeb ();

    typedef  FeatureFileIODstWeb*  FeatureFileIODstWebPtr;

    static   FeatureFileIODstWebPtr  Driver ()  {return &driver;}

    virtual  FileDescConstPtr  GetFileDesc (const KKStr&   _fileName,
                                            istream&       _in,
                                            MLClassListPtr _classList,
                                            kkint32&       _estSize,
                                            KKStr&         _errorMessage,
                                            RunLog&        _log
                                           );


    virtual  FeatureVectorListPtr  LoadFile (const KKStr&      _fileName,
                                             FileDescConstPtr  _fileDesc,
                                             MLClassList&      _classes, 
                                             istream&          _in,
                                             OptionUInt32      _maxCount,    // Maximum # images to load.
                                             VolConstBool&     _cancelFlag,
                                             bool&             _changesMade,
                                             KKStr&            _errorMessage,
                                             RunLog&           _log
                                            );


  private:
    static  FeatureFileIODstWeb  driver;

    class  AttrDescLine;
    typedef  AttrDescLine*  AttrDescLinePtr;
    class  AttrDescLineComparator;

  };  /* FeatureFileIODstWeb */


}  /* namespace KKMLL */


#endif



