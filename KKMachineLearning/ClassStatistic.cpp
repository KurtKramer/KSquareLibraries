#include "FirstIncludes.h"
#include <iostream>
#include <map>
#include <vector>
#include <fstream>


#include "MemoryDebug.h"
using namespace  std;


#include "KKBaseTypes.h"
#include "KKQueue.h"
using namespace  KKB;


#include "ClassStatistic.h"
#include "MLClass.h"
using namespace  KKMachineLearning;




ClassStatistic::ClassStatistic (const ClassStatistic&  right):
  mlClass (right.mlClass),
  count      (right.count)
{
}



ClassStatistic::ClassStatistic (MLClassPtr  _mlClass,
                                uint32         _count
                               ):
   mlClass (_mlClass),
   count      (_count)
{}



   
   
 const KKStr&   ClassStatistic::Name ()  const
 {
   if  (mlClass)
     return mlClass->Name ();
   else
     return  KKStr::EmptyStr ();
 }  /* Name */




const ClassStatistic&  ClassStatistic::operator+= (const ClassStatistic&  right)
{
  if  (mlClass != right.MLClass ())
  {
    cerr << std::endl << std::endl << std::endl
         << "ClassStatistic::operator+=        Class Names Do Not Match" << std::endl
         << "                LeftClass[" << mlClass->Name () << "]   RightClass["  << right.Name () << "[" << std::endl
         << std::endl;
    return  *this;
  }

  count += right.Count ();

  return  *this;
}




ClassStatisticList::ClassStatisticList  (bool  _owner):
   KKQueue<ClassStatistic> (_owner)
{
}



void  ClassStatisticList::PushOnBack (ClassStatisticPtr  stat)
{
  KKQueue<ClassStatistic>::PushOnBack (stat);
  imageClassIndex.insert (pair<MLClassPtr, ClassStatisticPtr> (stat->MLClass (), stat));
}



void  ClassStatisticList::PushOnFront (ClassStatisticPtr  stat)
{
  KKQueue<ClassStatistic>::PushOnFront (stat);
  imageClassIndex.insert (pair<MLClassPtr, ClassStatisticPtr> (stat->MLClass (), stat));
}




const  ClassStatisticList&  ClassStatisticList::operator+= (const ClassStatisticList&  right)
{
  ClassStatisticList::const_iterator  idx;
  for  (idx = right.begin ();  idx != right.end ();  idx++)
  {
    ClassStatisticPtr  rightStat = *idx;
    ClassStatisticPtr  leftStat = LookUpByImageClass (rightStat->MLClass ());

    if  (leftStat)
      (*leftStat) += (*rightStat);
    else
      PushOnBack (new ClassStatistic (*rightStat));
  }

  return  *this;
}  /* operator+=  */





ClassStatisticPtr  ClassStatisticList::LookUpByImageClass (MLClassPtr  mlClass)  const
{
  map<MLClassPtr, ClassStatisticPtr>::const_iterator  idx;
  idx = imageClassIndex.find (mlClass);
  if  (idx == imageClassIndex.end ())
    return NULL;
  else
    return idx->second;
}




class  ClassStatisticList::ClassStatisticSortComparrison
{
public:
   ClassStatisticSortComparrison () {} 

   bool  operator() (ClassStatisticPtr  p1,
                     ClassStatisticPtr  p2
                    )
   {
     return  (p1->MLClass ()->UpperName () < p2->MLClass ()->UpperName ());
   }
};  /* ClassStatisticSortComparrison */




class  ClassStatisticList::ClassStatSortByCount
{
public:
  ClassStatSortByCount () {}

   bool  operator () (ClassStatisticPtr  p1,
                      ClassStatisticPtr  p2
                     )
   {
     if  (p1->Count () > p2->Count ())
       return true;

     else if  (p1->Count () < p2->Count ())
       return false;

     return  p1->MLClass ()->UpperName () < p2->MLClass ()->UpperName ();
   }
};  /* ManagedClasssesSortByImageCount */





void  ClassStatisticList::SortByClassName ()
{
  ClassStatisticSortComparrison  c;
  //KKQueue<ClassStatistic>::Sort (&c);
  sort (begin (), end (), c);
}  /* Sort */




void  ClassStatisticList::SortByCount  ()
{
  ClassStatSortByCount  c;
  //KKQueue<ClassStatistic>::Sort (&c);
  sort (begin (), end (), c);
}



void  ClassStatisticList::PrintReport (ostream& r)
{
  r << endl
    << "Class Name" << "\t" << "Count" << endl;

  SortByClassName ();
  iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
  {
    ClassStatisticPtr  cs = *idx;
    r << cs->Name () << "\t" << cs->Count () << endl;
  }
  r << endl;
}


int32  ClassStatisticList::operator[]  (MLClassPtr  mlClass)
{
  std::map<MLClassPtr, ClassStatisticPtr>::const_iterator idx;
  idx = imageClassIndex.find (mlClass);
  if  (idx == imageClassIndex.end ())
    return -1;
  else
    return idx->second->Count ();
}
