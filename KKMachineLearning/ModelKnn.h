#ifndef  _MODELKNN_
#define  _MODELKNN_

#include "FeatureVector.h"
#include "Model.h"
#include "ModelParamKnn.h"

namespace  KKMLL
{
  class  ModelKnn: public Model
  {
  public:
    typedef  ModelKnn*  ModelKnnPtr;

    ModelKnn ();

    ModelKnn (FileDescPtr    _fileDesc,
              VolConstBool&  _cancelFlag
             );
  
    ModelKnn (const KKStr&          _name,
              const ModelParamKnn&  _param,         // Create new model from
              FileDescPtr           _fileDesc,
              VolConstBool&         _cancelFlag
             );
  
    ModelKnn (const ModelKnn&  _madel);

    virtual ~ModelKnn();

    virtual
    ModelKnnPtr  Duplicate ()  const;

    virtual ModelTypes   ModelType ()  const  {return ModelTypes::mtKNN;}


    ModelParamKnnPtr   Param ();


    virtual
    MLClassPtr  Predict (FeatureVectorPtr  example,
                         RunLog&           log
                        );
  

    virtual
    void        Predict (FeatureVectorPtr  example,
                         MLClassPtr        knownClass,
                         MLClassPtr&       predClass1,
                         MLClassPtr&       predClass2,
                         kkint32&          predClass1Votes,
                         kkint32&          predClass2Votes,
                         double&           probOfKnownClass,
                         double&           predClass1Prob,
                         double&           predClass2Prob,
                         kkint32&          numOfWinners,
                         bool&             knownClassOneOfTheWinners,
                         double&           breakTie,
                         RunLog&           log
                        );


    virtual
    ClassProbListPtr  ProbabilitiesByClass (FeatureVectorPtr  example,
                                            RunLog&           log
                                           );

    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    example,
                                const MLClassList&  _mlClasses,
                                kkint32*            _votes,
                                double*             _probabilities,
                                RunLog&             log
                               );


    /**
     *@brief Derives predicted probabilities by class.
     *@details Will get the probabilities assigned to each class by the classifier. The '_mlClasses' parameter
     *  dictates the order of the classes. That is the probabilities for any given index in '_probabilities'
     *  will be for the class specified in the same index in '_mlClasses'.
     *@param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _mlClasses  List image classes that caller is aware of. This should be the same list that was
     *                        used when constructing this Model object. The list must be the same but not
     *                        necessarily in the same order as when Model was 1st constructed.
     *@param[out] _probabilities An array that must be as big as the number of classes as in
     *           mlClasses. The probability of class in mlClasses[x] will be returned
     *           in probabilities[x].
     */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    _example,
                                const MLClassList&  _mlClasses,
                                double*             _probabilities,
                                RunLog&             _log
                               );


    virtual  void  ReadSpecificImplementationXML (istream&  i,
                                                  bool&     _successful,
                                                  RunLog&   log
                                                 );


    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership,  /**< Model will take ownership of these examples */
                               RunLog&               _log
                              );


    virtual  void  WriteSpecificImplementationXML (ostream&  o,
                                                   RunLog&   log
                                                  );


    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   RunLog&         log
                  );


    void  WriteXML (const KKStr&  varName,
                    ostream&      o
                   )  const;


  private:
    ModelParamKnnPtr   param;   /**<   We will NOT own this instance; it will point to same instance defined in parent class Model.  */
  };  /* ModelKnn */

  typedef  ModelKnn::ModelKnnPtr  ModelKnnPtr;



  typedef  XmlElementTemplate<ModelKnn>  XmlElementModelKnn;
  typedef  XmlElementModelKnn*  XmlElementModelKnnPtr;


}



#endif
