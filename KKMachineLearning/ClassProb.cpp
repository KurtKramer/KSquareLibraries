#include "FirstIncludes.h"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include "MemoryDebug.h"
using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "OSservices.h"
using namespace  KKB;


#include "ClassProb.h"
#include "MLClass.h"
using namespace  KKMLL;


ClassProb::ClassProb (MLClassPtr _classLabel,
                      double     _probability,
                      float      _votes
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
 

kkint32  ClassProbList::MemoryConsumedEstimated ()  const
{
  return  ((kkint32)size ())  *  sizeof(ClassProb);
}  /* MemoryConsumedEstimated */




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




const ClassProbPtr  ClassProbList::LookUp (MLClassPtr       targetClass) const
{
  MLClassIndexType::const_iterator idx;
  idx = classIndex.find (targetClass);
  if  (idx == classIndex.end ())
    return NULL;
  else
    return idx->second;

}  /* LookUp */




kkint32   ClassProbList::LookUpPlace (MLClassPtr       targetClass)  const
{
  for  (kkint32  x = 0;  x < (kkint32)size ();  ++x)
  {
    if  (IdxToPtr (x)->classLabel == targetClass)
      return x;
  }

  return -1;
}  /* LookUpPlace */



void  ClassProbList::DeleteEntry (ClassProbPtr  cp)
{
  MLClassIndexType::const_iterator idx;
  idx = classIndex.find (cp->classLabel);
  if  (idx != classIndex.end ())
    classIndex.erase (idx);
  KKQueue<ClassProb>::DeleteEntry (cp);
}



void  ClassProbList::DeleteEntry (kkuint32 idx)
{
  if  (idx >= size ())
  {
    KKStr  errMsg = "ClassProbList::DeleteEntry   ***ERROR***   idx [" + StrFormatInt (idx, "#,###,##0") + "]";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  DeleteEntry (IdxToPtr (idx));
}




void  ClassProbList::PushOnBack (ClassProbPtr  cp)
{
  try
  {
    classIndex.insert (MLClassIndexPair (cp->classLabel, cp));
  }
  catch  (const std::exception&)
  {
    KKStr errMsg = "ClassProbList::PushOnBack  ***ERROR***   Exception occurred adding Class[" + cp->classLabel->Name () + "]";
    cerr << errMsg << endl;
    throw KKException (errMsg);
  }
  KKQueue<ClassProb>::PushOnBack (cp);
}



void  ClassProbList::PushOnFront (ClassProbPtr  cp)
{
  try
  {
    classIndex.insert (MLClassIndexPair (cp->classLabel, cp));
  }
  catch  (const std::exception&)
  {
    KKStr errMsg = "ClassProbList::PushOnFront  ***ERROR***   Exception occurred adding Class[" + cp->classLabel->Name () + "]";
    cerr << errMsg << endl;
    throw KKException (errMsg);
  }
  KKQueue<ClassProb>::PushOnFront (cp);
}



void  ClassProbList::AddIn (const ClassProbListPtr  otherPredictions)
{
  if  (!otherPredictions)
    return;

  ClassProbList::const_iterator  idx;
  for  (idx = otherPredictions->begin ();  idx != otherPredictions->end ();  ++idx)
  {
    const ClassProbPtr  otherPrediction = *idx;
    MergeIn (otherPrediction);
  }
}  /* AddIn */




/**
 *@brief Adds the Prediction in 'cp' into this list.
 *@details If the class indicated by 'cp->classLabel' already exist in this 
 * list will then add to existing entry otherwise will create a new entry for 
 * the class.
 */
void  ClassProbList::MergeIn (const ClassProbPtr cp)
{
  MergeIn (cp->classLabel, cp->probability, cp->votes);
}  /* MergeIn */




void  ClassProbList::MergeIn (MLClassPtr       target,
                              double           probability,
                              float            votes
                             )
{
  ClassProbPtr  existingEntry = LookUp (target);
  if  (existingEntry)
  {
    existingEntry->probability += probability;
    existingEntry->votes       += votes;
  }
  else
  {
    PushOnBack (new ClassProb (target, probability, votes));
  }
}  /* MergeIn */




/**
 *@brief Merges in the predictions in 'subPredictions' by replacing the entry in our list with 
 * label 'target' with contents of 'subPredictions'.
 *@details It is assumed that the sum of probabilities of 'subPredictions' is equal to 1.0.
 */
void  ClassProbList::MergeIn (MLClassPtr              target,
                              const ClassProbListPtr  subPredictions
                             )
{
  ClassProbPtr  existingEntry = LookUp (target);
  if  (existingEntry)
  {
    DeleteEntry (existingEntry);
    float   totalSubVotes = (float)(subPredictions->size () - 1);
    double  baseProb = existingEntry->probability;
    float   baseVotes = existingEntry->votes;
    ClassProbList::const_iterator  idx;
    for  (idx = subPredictions->begin ();  idx != subPredictions->end ();  ++idx)
    {
      const ClassProbPtr  subPrediction = *idx;
      double subProbability = baseProb * subPrediction->probability;
      float  subVotes       = baseVotes * (subPrediction->votes / totalSubVotes);
      MergeIn (subPrediction->classLabel, subProbability, subVotes);
    }
  }
  else
  {
  ClassProbList::const_iterator  idx;
    for  (idx = subPredictions->begin ();  idx != subPredictions->end ();  ++idx)
    {
      const ClassProbPtr  subPrediction = *idx;
      MergeIn (subPrediction->classLabel, subPrediction->probability, subPrediction->votes);
    }
  }
}  /* MergeIn */





void  ClassProbList::NormalizeToOne ()
{
  double  totalProb  = 0.0;
  float   totalVotes = 0.0f;

  float expectedTotalNumOfVotes = (float)(size () * (size () - 1) / 2);

  iterator idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    ClassProbPtr  cp = *idx;
    totalProb  += cp->probability;
    totalVotes += cp->votes;
  }

  for  (idx = begin ();  idx != end ();  ++idx)
  {
    ClassProbPtr  cp = *idx;
    cp->probability = cp->probability / totalProb;
    cp->votes = (cp->votes / totalVotes) * expectedTotalNumOfVotes;
  }
}  /* NormalizeToOne */



void  ClassProbList::WriteXML (ostream&      o, 
                               const KKStr&  name
                              )
                             const
{
  o << "<ClassProbList,name=\"" << name << "\">" << endl;
  
  ClassProbList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
  {
    ClassProbPtr  cp = *idx;
    o << cp->classLabel->Name () << "\t" << cp->probability << "\t" << cp->votes << endl;
  }
  o << "</ClassProbList>" << endl;
}  /* WriteXML */




ClassProbListPtr  ClassProbList::CreateFromXMLStream (istream& i)
{
  ClassProbListPtr  result = new ClassProbList (true);

  bool eof = false;

  KKStr line = osReadRestOfLine (i, eof);

  while  (!eof)
  {
    if  (line.StartsWith ("</ClassProbList>"))
      break;

    if  (line.StartsWith ("<ClassProbList"))
      continue;

    KKStr  className = line.ExtractToken2 ("\n\r\t");
    double  prob   = line.ExtractTokenDouble ("\t\n\r");
    double  votes  = line.ExtractTokenDouble ("\t\n\r");
    if  (!className.Empty ())
    {
      MLClassPtr      c = MLClass::CreateNewMLClass (className, -1);

      result->PushOnBack (new ClassProb (c, prob, (float)votes));
    }
    line = osReadRestOfLine (i, eof);
  }

  return result;
}  /* CreateFromXMLStream */








void  ClassProbList::WriteXML (const KKStr&  varName,
                               ostream&      o
                              )  const
{
  XmlTag  startTag ("ClassProbList",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  for  (auto idx:  *this)
  {
    ClassProbPtr  cp = idx;
    o << cp->classLabel->Name () << "\t" << cp->probability << "\t" << cp->votes << endl;
  }
  
  XmlTag  endTag ("ClassProbList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  ClassProbList::ReadXML (XmlStream&      s,
                              XmlTagConstPtr  tag,
                              RunLog&         log
                             )
{
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr content = dynamic_cast<XmlContentPtr> (t);
      if  (!content)
        continue;
      if  (content->Content ()->Empty ())
        continue;

      KKStrParser p (content->Content ());
      p.TrimWhiteSpace (" ");
      KKStr    className = p.GetNextToken ("\t");
      double   probability = p.GetNextTokenDouble ("\t");
      float    votes = p.GetNextTokenFloat ("\t");
      if  (className.Empty ())
        continue;

      if  ((probability < 0.0f)  ||  (probability > 1.0f))
      {
        log.Level (-1)
          << "ClassProbList::ReadXML  ***ERROR***   Probability: " << probability << "  is out of range for Class: " << className << endl
          << endl;
      }

      PushOnBack (new ClassProb (MLClass::CreateNewMLClass (className), probability, votes));
    }
    delete  t;
    t = s.GetNextToken (log);
  }

}  /* ReadXML */



XmlFactoryMacro(ClassProbList)

