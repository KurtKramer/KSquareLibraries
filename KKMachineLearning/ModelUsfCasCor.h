#if  !defined(_MODELUSFCASCOR_)
#define  _MODELUSFCASCOR_
//***********************************************************************
//*                           ModelUsfCasCor                              *
//*                                                                     *
//*  This will be the base model for implementations that utilize       *
//*  the Support Vector Machine.                                        *
//***********************************************************************


#include "RunLog.h"
#include "KKStr.h"

#include "Model.h"
#include "ModelParam.h"
#include "ModelParamUsfCasCor.h"

#include "UsfCasCor.h"


namespace  KKMLL  
{
  class  ModelUsfCasCor: public Model
  {
  public:

    typedef  ModelUsfCasCor*  ModelUsfCasCorPtr;


    ModelUsfCasCor (FileDescPtr    _fileDesc,
                    VolConstBool&  _cancelFlag,
                    RunLog&        _log
                  );

    ModelUsfCasCor (const KKStr&               _name,
                    const ModelParamUsfCasCor& _param,         // Create new model from
                    FileDescPtr                _fileDesc,
                    VolConstBool&              _cancelFlag,
                    RunLog&                    _log
                   );
  
    ModelUsfCasCor (const ModelUsfCasCor&   _model);

    virtual
    ~ModelUsfCasCor ();

    virtual
    kkint32                 MemoryConsumedEstimated ()  const;

    virtual ModelTypes      ModelType () const  {return ModelTypes::mtUsfCasCor;}

    virtual
    kkint32                 NumOfSupportVectors () const;

    virtual
    ModelUsfCasCorPtr       Duplicate ()  const;

    ModelParamUsfCasCorPtr  Param ();

    virtual
    MLClassPtr              Predict (FeatureVectorPtr  example);
  
    virtual
    void                    Predict (FeatureVectorPtr example,
                                     MLClassPtr       knownClass,
                                     MLClassPtr&      predClass1,
                                     MLClassPtr&      predClass2,
                                     kkint32&         predClass1Votes,
                                     kkint32&         predClass2Votes,
                                     double&          probOfKnownClass,
                                     double&          predClass1Prob,
                                     double&          predClass2Prob,
                                     kkint32&         numOfWinners,
                                     bool&            knownClassOneOfTheWinners,
                                     double&          breakTie
                                    );


    virtual
    ClassProbListPtr    ProbabilitiesByClass (FeatureVectorPtr  example);


    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr       example,
                                const MLClassList&  _mlClasses,
                                kkint32*                 _votes,
                                double*                _probabilities
                               );

    /**
     *@brief Derives predicted probabilities by class.
     *@details Will get the probabilities assigned to each class by the classifier. The 
     *         '_mlClasses' parameter dictates the order of the classes. That is the 
     *         probabilities for any given index in '_probabilities' will be for the class
     *         specified in the same index in '_mlClasses'.
     *@param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _mlClasses  List classes that caller is aware of. This should be the same
     *            list that was used when constructing this Model object.  The list must 
     *            be the same but not necessarily in the same order as when Model was 1st 
     *            constructed.
     *@param[out] _probabilities An array that must be as big as the number of classes as in
     *            mlClasses. The probability of class in mlClasses[x] will be returned 
     *            in probabilities[x].
     */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    _example,
                                const MLClassList&  _mlClasses,
                                double*             _probabilities
                               );
  
    virtual  void  ReadSpecificImplementationXML (istream&  i,
                                                  bool&     _successful
                                                 );


    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership  /**< Model will take ownership of these examples */
                              );


    virtual  void  WriteSpecificImplementationXML (ostream&  o);



  protected:
    UsfCasCorPtr              usfCasCorClassifier;
    ModelParamUsfCasCorPtr    param;                  /**<   We will NOT own this instance. It will point to same instance defined in parent class Model.  */
  };  /* ModelUsfCasCor */



typedef  ModelUsfCasCor::ModelUsfCasCorPtr  ModelUsfCasCorPtr;

} /* namespace  KKMLL */

#endif
