#ifndef  _MODELDUAL_
#define  _MODELDUAL_
/**
 *@class  KKMLL::ModelDual
 *@brief  Will implement the Dual Classifier Model.
 *@details This model will actually load two different training models specified in the parameters
 * field.  It will utilize both classifiers for to make a prediction. Unknown examples will be
 * submitted to both classifiers. The returned class will be the common part of the class hierarchy
 * of the two predictions. If there is nothing in common between the two predictions then it will
 * return the "Other class.
 */

#include "RunLog.h"
#include "KKStr.h"

#include "Model.h"


namespace  KKMLL  
{
  #if  !defined(_MLCLASS_)
    class  MLClass;
    typedef  MLClass*  MLClassPtr;
    class  MLClassList;
    typedef  MLClassList*  MLClassListPtr;
  #endif

  #if  !defined(_FILEDESC_)
    class  FileDesc;
    typedef  FileDesc*  FileDescPtr;
  #endif

  #if  !defined(_FEATUREVECTOR_)
    class  FeatureVector;
    typedef  FeatureVector* FeatureVectorPtr;
    class  FeatureVectorList;
    typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif

  #if  !defined(_FEATURENUMLIST_)
    class  FeatureNumList;
    typedef  FeatureNumList*  FeatureNumListPtr;
  #endif

  #if  !defined(_TRAININGPROCESS2_)
    class  TrainingProcess2;
    typedef  TrainingProcess2*  TrainingProcess2Ptr;
  #endif

  #if  !defined(_CLASSIFIER2_)
    class  Classifier2;
    typedef  Classifier2*  Classifier2Ptr;
  #endif

  #if  !defined(_MODELPARAMDUAL_)
    class  ModelParamDual;
    typedef  ModelParamDual*  ModelParamDualPtr;
  #endif

  #if  !defined(_TRAININGCONFIGURATION2_)
    class  TrainingConfiguration2;
    typedef  TrainingConfiguration2* TrainingConfiguration2Ptr;
  #endif


  class  ModelDual: public Model
  {
  public:

    typedef  ModelDual*  ModelDualPtr;


    ModelDual ();

    ModelDual (FactoryFVProducerPtr  _factoryFVProducer);

    ModelDual (const KKStr&           _name,
               const ModelParamDual&  _param,         // Create new model from
               FactoryFVProducerPtr   _factoryFVProducer
              );
  
    ModelDual (const ModelDual&   _model);

    virtual
    ~ModelDual ();

    virtual
    kkint32               MemoryConsumedEstimated ()  const;

    virtual
    KKStr                 Description ()  const;  /**< Return short user readable description of model. */

    virtual ModelTypes    ModelType () const  {return ModelTypes::mtDual;}

    virtual
    kkint32               NumOfSupportVectors () const;

    TrainingProcess2Ptr   Trainer1 ()  {return trainer1;}

    TrainingProcess2Ptr   Trainer2 ()  {return trainer2;}

    virtual
    ModelDualPtr          Duplicate ()  const;

    ModelParamDualPtr     Param ();

    virtual
    MLClassPtr            Predict (FeatureVectorPtr  image,
                                   RunLog&           log
                                  );
  
    virtual
    void                  Predict (FeatureVectorPtr  example,
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
    void  ProbabilitiesByClassDual (FeatureVectorPtr   example,
                                    KKStr&             classifier1Desc,
                                    KKStr&             classifier2Desc,
                                    ClassProbListPtr&  classifier1Results,
                                    ClassProbListPtr&  classifier2Results,
                                    RunLog&            log
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
     *@details Will get the probabilities assigned to each class by the classifier. The
     *         '_mlClasses' parameter dictates the order of the classes. That is the 
     *         probabilities for any given index in '_probabilities' will be for the class
     *         specified in the same index in '_mlClasses'.
     *@param[in]  _example    FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _mlClasses  List image classes that caller is aware of. This should be the
     *            same list that was used when constructing this Model object. The list must
     *            be the same but not necessarily in the same order as when Model was 1st
     *            constructed.
     *@param[out] _probabilities An array that must be as big as the number of classes as in
     *            mlClasses. The probability of class in mlClasses[x] will be returned
     *            in probabilities[x].
    */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    _example,
                                const MLClassList&  _mlClasses,
                                double*             _probabilities,
                                RunLog&             _log
                               );


    virtual  
    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**      crossProbTable, /**< two dimension matrix that needs to be classes.QueueSize () squared. */
                                  RunLog&       log
                                 );



    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership,  /**< Model will take ownership of these examples */
                               VolConstBool&         _cancelFlag,
                               RunLog&               _log
                              );


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;


  protected:
    void  DeleteExistingClassifiers ();

    MLClassPtr  ReconcilePredictions (MLClassPtr  pred1, 
                                      MLClassPtr  pred2,
                                      RunLog&     log
                                     );

    void  ReconcileProbAndVotes (Classifier2Ptr    classifier,
                                 MLClassPtr        predClass,
                                 FeatureVectorPtr  encodedExample,
                                 double&           predClassProb,
                                 kkint32&            predClassVotes
                                );

    TrainingConfiguration2Ptr  config1;
    TrainingConfiguration2Ptr  config2;

    TrainingProcess2Ptr        trainer1;
    TrainingProcess2Ptr        trainer2;

    Classifier2Ptr             classifier1;
    Classifier2Ptr             classifier2;

    KKStr                      trainer1StatusMsg;
    KKStr                      trainer2StatusMsg;

    ModelParamDualPtr          param;   /*!<   We will NOT own this instance. It will point to same instance defined in parent class Model.  */
  };  /* ModelDual */


  typedef  ModelDual::ModelDualPtr  ModelDualPtr;


  typedef  XmlElementModelTemplate<ModelDual>  XmlElementModelDual;
  typedef  XmlElementModelDual*  XmlElementModelDualPtr;

} /* namespace  KKMLL */

#endif
