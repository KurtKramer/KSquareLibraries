#include "FirstIncludes.h"
#include <map>
#include <ostream>
#include <stdio.h>
#include "MemoryDebug.h"
using namespace  std;

#include "KKBaseTypes.h"
using namespace  KKB;


#include "ClassProb.h"
#include "MLClass.h"
using namespace  KKMachineLearning;


ClassProb::ClassProb (MLClassPtr _classLabel,
                      double        _probability,
                      float         _votes
                     ):
  classLabel  (_classLabel),
  probability (_probability),
  votes       (_votes)
{
}


ClassProb::ClassProb (const ClassProb&  _pair):
  classLabel  (_pair.classLabel),
  probability (_pair.probability),
  votes       (_pair.votes)
{
}



ClassProbList::ClassProbList ():
   KKQueue<ClassProb>  (true)
{
}


ClassProbList::ClassProbList (bool owner):
   KKQueue<ClassProb>  (owner)
{
}



ClassProbList::ClassProbList (const ClassProbList&  pairList):
   KKQueue<ClassProb>  (pairList)
{
}
 


bool  ClassProbList::CompairByClassName (const ClassProbPtr left, 
                                                    const ClassProbPtr right
                                                   )
{
  return  left->classLabel->UpperName () < right->classLabel->UpperName ();
}


void  ClassProbList::SortByClassName ()
{
  sort (begin (), end (), CompairByClassName);
}


class  ClassProbList::ProbabilityComparer
{
public:
  ProbabilityComparer (bool  _highToLow):  highToLow (_highToLow)  {}

  bool operator () (const ClassProbPtr left, 
                    const ClassProbPtr right
                   )
  { 
    if  (highToLow)
      return (left->probability > right->probability);
    else
      return (left->probability < right->probability);
  }

private:
  bool  highToLow;
};  /* ProbabilityComparer */



class  ClassProbList::VotesComparer
{
public:
  VotesComparer (bool  _highToLow):  highToLow (_highToLow)  {}

  bool operator () (const ClassProbPtr left, 
                    const ClassProbPtr right
                   )
  { 
    if  (left->votes == right->votes)
    {
      if  (highToLow)
        return (left->probability > right->probability);
      else
        return (left->probability < right->probability);
    }
    else
    {
      if  (highToLow)
        return (left->votes > right->votes);
      else
        return (left->votes < right->votes);
    }
  }

private:
  bool  highToLow;
};  /* VotesComparer */




void  ClassProbList::SortByProbability (bool highToLow)
{
  ProbabilityComparer comparator (highToLow);
  sort (begin (), end (), comparator);
}  /* SortByProbability */
 


void  ClassProbList::SortByVotes (bool highToLow)
{
  VotesComparer comparator (highToLow);
  sort (begin (), end (), comparator);
}  /* SortByVotes */




const ClassProbPtr  ClassProbList::LookUp (MLClassPtr  targetImageClass) const
{
  ClassProbList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    if  ((*idx)->classLabel == targetImageClass)
      return *idx;
  }
  return NULL;
}  /* LookUp */

