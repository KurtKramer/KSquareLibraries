#ifndef  _CLASSSTATISTIC_

#include "KKBaseTypes.h"
#include "MLClass.h"
#include "KKStr.h"


namespace KKMachineLearning
{

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
    kkuint32    count;        /**< represents the number of PostLarvaeFV instances in a FeatureVectorList that point to mageClass */
  };



  typedef  ClassStatistic*  ClassStatisticPtr;



  class  ClassStatisticList:  public KKQueue<ClassStatistic> 
  {
  public:
    ClassStatisticList (bool  _owner);

    ClassStatisticPtr  LookUpByImageClass (MLClassPtr  mlClass)  const;

    void  SortByClassName  ();
    void  SortByCount  ();

    void  PushOnBack  (ClassStatisticPtr  stat);
    void  PushOnFront (ClassStatisticPtr  stat);

    void  PrintReport (ostream& r);

    kkint32 operator[]  (MLClassPtr  mlClass);
    const  ClassStatisticList&  operator+= (const ClassStatisticList&  right);


  private:
    class  ClassStatisticSortComparrison;

    std::map<MLClassPtr, ClassStatisticPtr>  imageClassIndex;

    class  ClassStatSortByCount;
  };  /* ClassStatisticList */



  typedef  ClassStatisticList*  ClassStatisticListPtr;



  #define  _CLASSSTATISTIC_

}  /* namespace KKMachineLearning */


#endif

