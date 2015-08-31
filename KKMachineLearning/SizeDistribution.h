#ifndef  _SIZEDISTRIBUTION_
#define  _SIZEDISTRIBUTION_


//#define  BucketCount  50
//#define  BucketSize   100


#include  "MLClass.h"
#include  "FeatureVector.h"
#include  "RunLog.h"

class  ClassTotals;
typedef  ClassTotals*  ClassTotalsPtr;

class  ClassTotalsList;
typedef  ClassTotalsList*  ClassTotalsListPtr;


namespace  KKMLL
{
  /**
   *@brief Used to keep track of examples by size; typically used by 'CrossValidation'; for each example predicted
   * it would call the ;Increment' method keeping track 
   */
  class  SizeDistribution
  {
  public:
    SizeDistribution (kkint32  _bucketCount,
                      kkint32  _bucketSize,
                      RunLog&  _log
                     );
  
    ~SizeDistribution ();


    void   Increment (MLClassPtr  mlClass,
                      kkint32     size
                     );


    void   PrintFormatedDistributionMatrix (ostream&  _outFile)  const;


    void   PrintCSVDistributionMatrix (ostream&  _outFile)  const;


    void   PrintTabDelDistributionMatrix (ostream&  _outFile)  const;


    void   PrintByClassCollumns (ostream&      o,
                                 VectorUlong*  scanLinesPerMeter 
                                )  const;


  private:
    MLClassListPtr   BuildMLClassList ()  const;

    void  PrintCSVHeader      (ostream&  o)  const;
    void  PrintFormatedHeader (ostream&  o)  const;
    void  PrintTabDelHeader   (ostream&  o)  const;


    class  ClassTotals;
    class  ClassTotalsList;

    typedef  ClassTotalsList*  ClassTotalsListPtr;



    kkint32             bucketCount;
    kkint32             bucketSize;
    RunLog&             log;
    ClassTotalsListPtr  totals;
  };  /* SizeDistribution */

  typedef  SizeDistribution*  SizeDistributionPtr;

}  /* namespace  KKMLL */

#endif
