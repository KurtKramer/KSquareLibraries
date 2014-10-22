#if  !defined(_STARTSTOPPOINT_)
#define  _STARTSTOPPOINT_

#include <vector>

#include "DateTime.h"
#include "GoalKeeper.h"
#include "KKStr.h"
#include "KKStrParser.h"

using namespace KKB;


namespace  KKLSC
{
  #if  !defined(_SCANNERFILE_)
    class  ScannerFile;
    typedef  ScannerFile*  ScannerFilePtr;
  #endif
  
  class  StartStopPointList;
  typedef  StartStopPointList*  StartStopPointListPtr;


  /**
   *@brief Class that keeps track of parameter details of a single scanner file.
   */
  class  StartStopPoint
  {
  public:
    typedef  StartStopPoint*  StartStopPointPtr;

    typedef  enum  {sspNULL,  sspStartPoint, sspStopPoint, sspInvalid}  StartStopType;
    static  const KKStr&   StartStopTypeToStr   (StartStopType  t);

    static  StartStopType  StartStopTypeFromStr (const KKStr&   s);

    StartStopPoint (kkint32        _scanLineNum,
                    StartStopType  _type
                   );

    StartStopPoint (const StartStopPoint&  entry);

    StartStopPoint (const KKStr&  s);

    ~StartStopPoint ();

    kkint32  MemoryConsumedEstimated ()  const;
  
    // Access Methods
    kkint32           ScanLineNum  () const  {return  scanLineNum;}
    StartStopType     Type         () const  {return  type;}
    const KKStr&      TypeStr      () const  {return  StartStopTypeToStr (type);}
  
    void  ScanLineNum  (const kkint32  _scanLineNum)  {scanLineNum = _scanLineNum;}
    void  Type         (StartStopType  _type)         {type        = _type;}
  
    KKStr   ToTabDelStr ()  const;
    void    ParseTabDelStr (KKStr  parser);

  private:
    static  KKStr           startStopPointStrs [];
    static  StartStopType   startStopPointTypes[];

    kkint32        scanLineNum;
    StartStopType  type;
  };  /* StartStopPoint */


  typedef  StartStopPoint::StartStopPointPtr  StartStopPointPtr;



  class  StartStopPointList: public std::vector<StartStopPointPtr>
  {
  public:
    typedef  StartStopPointList*  StartStopPointListPtr;
  
    StartStopPointList ();
    ~StartStopPointList ();

    kkint32  MemoryConsumedEstimated ()  const;

    StartStopPointPtr  AddEntry (kkint32                        _scanLineNum,
                                 StartStopPoint::StartStopType  _type
                               );

    StartStopPointPtr  AddEntry (StartStopPointPtr& _entry);

    void  Clear ();  /**< Clears all existing entries. */

    void  DeleteEntry (kkint32  _scanLineNum);

    StartStopPointPtr  NearestEntry (kkint32  _scanLineNum)  const;

    StartStopPointPtr  PrevEntry (kkint32  _scanLineNum)  const;

    StartStopPointPtr  SuccEntry (kkint32  _scanLineNum)  const;

  private:
    kkint32  FindEqual          (kkint32 _scanLineNum)  const;
    kkint32  FindGreaterOrEqual (kkint32 _scanLineNum)  const;
    kkint32  FindLessOrEqual    (kkint32 _scanLineNum)  const;

    iterator  idx;
  };  /* StartStopPointList */


  typedef  StartStopPointList::StartStopPointListPtr  StartStopPointListPtr;




  /**@brief  Defines a single region in a  Scanner File that is to be included in a count */
  class  StartStopRegion
  {
  public:
    StartStopRegion (kkint32  _start,  
                     kkint32  _end
                    );

    kkint32  Start ()  const  {return  start;}
    kkint32  End   ()  const  {return  end;}

  private:
    kkint32  start;
    kkint32  end;
  };

  typedef  StartStopRegion*  StartStopRegionPtr;


  class  StartStopRegionList:  public  KKQueue<StartStopRegion>
  {
  public:
    StartStopRegionList (bool  _owner);

    StartStopRegionList (const StartStopPointList&  startStopPoints);

    ~StartStopRegionList ();
  };

  typedef  StartStopRegionList*  StartStopRegionListPtr;

};  /* KKLSC */

#endif

