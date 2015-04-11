#if  !defined(_CLASSPROB_)
#define  _CLASSPROB_
/**
 *@class  KKMLL::ClassProb
 *@brief Used to record probability for a specified class;  and a list of classes.
 *@author  Kurt Kramer
 */

#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "KKStr.h"


namespace KKMLL
{
  #if  !defined(_MLCLASS_)
  class  MLClass;
  typedef  MLClass*      MLClassPtr;
  typedef  MLClass const *  MLClassConstPtr;
  class  MLClassList;
  typedef  MLClassList*  MLClassListPtr;
  #endif


  class  ClassProb
  {
  public:
    ClassProb (MLClassPtr _classLabel,
               double     _probability,
               float      _votes
              );

    ClassProb (const ClassProb&  _pair);

    MLClassPtr  classLabel;
    double      probability;
    float       votes;
  };
  typedef  ClassProb*  ClassProbPtr;

#define  _ClassProb_Defined_


  class  ClassProbList:  public  KKQueue<ClassProb>
  {
  public:
    typedef  ClassProbList*  ClassProbListPtr;

    ClassProbList ();
    ClassProbList (bool owner);
    ClassProbList (const ClassProbList&  pairList);

    kkint32  MemoryConsumedEstimated ()  const;

    void  SortByClassName ();
    void  SortByProbability (bool highToLow = true);
    void  SortByVotes       (bool highToLow = true);

    const ClassProbPtr  LookUp (MLClassPtr  targetClass)  const;

    virtual  void  DeleteEntry (ClassProbPtr  cp);
    virtual  void  DeleteEntry (kkuint32      idx); 
    virtual  void  PushOnBack  (ClassProbPtr  cp);
    virtual  void  PushOnFront (ClassProbPtr  cp);


    /**
     *@brief Adds the contents of 'otherPredictions' to this list.
     *@details there os not assumption that this list or 'otherPredictions' sum up to "1.0".
     */
    void  AddIn (const ClassProbListPtr  otherPredictions);


    /**
     *@brief Returns the position that 'targetClass' has in the order;  good time to use would be after sorting by probability.
     */
    kkint32  LookUpPlace (MLClassPtr  targetClass)  const;


    /**
     *@brief Adds the Prediction in 'cp' into this list.
     *@details If the class indicated by 'cp->classLabel' already exist in this 
     * list will then add to existing entry otherwise will create a new entry for 
     * the class.
     */
    void  MergeIn (const ClassProbPtr cp);

    /**
     *@brief Adds the prediction of 'target' with 'probability' into this list.
     *@details If the class indicated by 'target' already exist in this 
     * list will then add to existing entry otherwise will create a new entry for 
     * the class.
     */
    void  MergeIn (MLClassPtr target,
                   double     probability,
                   float      votes
                  );

    /**
     *@brief Merges in the predictions in 'subPredictions' by replacing the entry in our list with 
     * label 'target' with contents of 'subPredictions'.
     *@details It is assumed that the sum of probabilities of 'subPredictions' is equal to 1.0.
     */
    void  MergeIn (MLClassPtr              target,
                   const ClassProbListPtr  subPredictions
                  );


    /**
     **@brief  Will normalize the list of predictions such that the total probability will equal 1.0 and 
     * total votes will = (N)(N-1)/2 where N = number of predictions.
     */
    void  NormalizeToOne ();

    void  WriteXML (ostream&      o, 
                    const KKStr&  name
                   )
                   const;

    static
      ClassProbListPtr  CreateFromXMLStream (istream& i);

  private:
    typedef  map<MLClassPtr,ClassProbPtr>   MLClassIndexType;
    typedef  pair<MLClassPtr,ClassProbPtr>  MLClassIndexPair;

    static bool  CompairByClassName (const ClassProbPtr left, const ClassProbPtr right);

    class  ProbabilityComparer;
    class  VotesComparer;

    MLClassIndexType  classIndex;
  };
  typedef  ClassProbList::ClassProbListPtr  ClassProbListPtr;

#define  _ClassProbList_Defined_

}  /* namespace MML */

#endif
