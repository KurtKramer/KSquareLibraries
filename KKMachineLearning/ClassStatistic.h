#if !defined(_CLASSSTATISTIC_)
#define  _CLASSSTATISTIC_

#include "KKBaseTypes.h"
#include "KKStr.h"

#include "MLClass.h"

namespace KKMLL
{
  /**
   *@brief Used by routines that retrieve Class statistics from FeatureVectorList instances.
   */
  class  ClassStatistic 
  {
  public:
    ClassStatistic (const ClassStatistic&  right);

    ClassStatistic (MLClassPtr  _mlClass,
                    kkuint32    _count
                   );

    kkuint32         Count      ()  const {return  count;}
    MLClassPtr       MLClass    ()  const {return  mlClass;}

    void             Increment  ()        {count++;}

    const KKStr&     Name       ()  const;

    const  ClassStatistic&  operator+= (const ClassStatistic&  right);

  private:
    MLClassPtr  mlClass;   /**< Does not own the mlClass object. */
    kkuint32    count;     /**< represents the number of FeatureVector derived instances in a FeatureVectorList that point to mageClass */
  };


  typedef  ClassStatistic*  ClassStatisticPtr;


  class  ClassStatisticList:  public KKQueue<ClassStatistic> 
  {
  public:
    ClassStatisticList (bool  _owner);

    ClassStatisticPtr  LookUpByMLClass (MLClassPtr  mlClass)  const;

    void  SortByClassName  ();
    void  SortByCount  ();

    void  PushOnBack  (ClassStatisticPtr  stat);
    void  PushOnFront (ClassStatisticPtr  stat);

    void  PrintReport (std::ostream& r);

    kkint32 operator[]  (MLClassPtr  mlClass);
    const  ClassStatisticList&  operator+= (const ClassStatisticList&  right);

  private:
    class  ClassStatisticSortComparrison;

    std::map<MLClassPtr, ClassStatisticPtr>  imageClassIndex;

    class  ClassStatSortByCount;
  };  /* ClassStatisticList */

  typedef  ClassStatisticList*  ClassStatisticListPtr;
}  /* namespace KKMLL */

#define  _ClassStatistic_Defined_

#endif

