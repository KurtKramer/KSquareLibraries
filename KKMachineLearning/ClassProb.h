#if  !defined(_CLASSPROB_)
#define  _CLASSPROB_
/**
 *@class  KKMachineLearning::ClassProb
 *@brief Used to record probability for a specified class;  and a list of classes.
 *@author  Kurt Kramer
 */

#include "KKBaseTypes.h"
#include "KKStr.h"


namespace KKMachineLearning
{
  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*         MLClassPtr;
  class  MLClassList;
  typedef  MLClassList*     MLClassListPtr;
  #endif


  class  ClassProb
  {
  public:
    ClassProb (MLClassPtr _classLabel,
               double        _probability,
               float         _votes
              );

    ClassProb (const ClassProb&  _pair);

    MLClassPtr  classLabel;
    double         probability;
    float          votes;
  };
  typedef  ClassProb*  ClassProbPtr;



  class  ClassProbList:  public  KKQueue<ClassProb>
  {
  public:
    ClassProbList ();
    ClassProbList (bool owner);
    ClassProbList (const ClassProbList&  pairList);
    void  SortByClassName ();
    void  SortByProbability (bool highToLow = true);
    void  SortByVotes       (bool highToLow = true);

    const ClassProbPtr  LookUp (MLClassPtr  targetImageClass)  const;

  private:
    static bool  CompairByClassName (const ClassProbPtr left, const ClassProbPtr right);

    class  ProbabilityComparer;
    class  VotesComparer;
  };
  typedef  ClassProbList*  ClassProbListPtr;

}  /* namespace MML */

#endif
