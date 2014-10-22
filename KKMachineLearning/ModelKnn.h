#ifndef  _MODELKNN_
#define  _MODELKNN_

#include "FeatureVector.h"
#include "Model.h"
#include "ModelParamKnn.h"

namespace  KKMachineLearning
{
  class  ModelKnn: public Model
  {
  public:
    typedef  ModelKnn*  ModelKnnPtr;

    ModelKnn (FileDescPtr    _fileDesc,
              VolConstBool&  _cancelFlag,
              RunLog&        _log
             );
  
    ModelKnn (const KKStr&          _name,
              const ModelParamKnn&  _param,         // Create new model from
              FileDescPtr           _fileDesc,
              VolConstBool&         _cancelFlag,
              RunLog&               _log
             );
  
    ModelKnn (const ModelKnn&  _madel);

    virtual ~ModelKnn();

    virtual
    ModelKnnPtr  Duplicate ()  const;

    virtual ModelTypes   ModelType ()  const  {return mtKNN;}


    ModelParamKnnPtr   Param ();


    virtual
    MLClassPtr      Predict (FeatureVectorPtr  example);
  

    virtual
    void               Predict (FeatureVectorPtr  example,
                                MLClassPtr     knownClass,
                                MLClassPtr&    predClass1,
                                MLClassPtr&    predClass2,
                                kkint32&          predClass1Votes,
                                kkint32&          predClass2Votes,
                                double&           probOfKnownClass,
                                double&           probOfPredClass1,
                                double&           probOfPredClass2,
                                kkint32&          numOfWinners,
                                bool&             knownClassOneOfTheWinners,
                                double&           breakTie
                               );


    virtual
    ClassProbListPtr  ProbabilitiesByClass (FeatureVectorPtr  example);

    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr       example,
                                const MLClassList&  _mlClasses,
                                kkint32*                   _votes,
                                double*                _probabilities
                               );


    /**
     * @brief Derives predicted probabilities by class.
     * @details Will get the probabilities assigned to each class by the classifier.  The '_mlClasses' parameter dictates the order
     *  of the classes. That is the probabilities for any given index in '_probabilities' will be for the class Specified in the same
     *  index in '_mlClasses'.
     *
     * @param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     * @param[in]  _ImageClasses  List of classes that caller is aware of. This should be the same list that was used
     *                            when constructing this Model object. The list must be the same but not necessarily in
     *                            the same order as when Model was 1st constructed.
     *@param[out] _probabilities  An array that must be as big as the number of classes as in mlClasses.  The probability
     *                            of class in mlClasses[x] will be returned in probabilities[x].
     */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr       _example,
                                const MLClassList&  _mlClasses,
                                double*                _probabilities
                               );


    virtual  void  ReadSpecificImplementationXML (istream&  i,
                                                  bool&     _successful
                                                 );


    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership  /**< Model will take ownership of these examples */
                              );


    virtual  void  WriteSpecificImplementationXML (ostream&  o);


  private:
    ModelParamKnnPtr   param;   /**<   We will NOT own this instance. It will point to same instance defined in parent class Model.  */
  };  /* ModelKnn */

  typedef  ModelKnn::ModelKnnPtr  ModelKnnPtr;
}



#endif
