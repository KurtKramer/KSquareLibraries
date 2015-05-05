#ifndef  _MODELSVMBASE_
#define  _MODELSVMBASE_
//***********************************************************************
//*                           ModelSvmBase                              *
//*                                                                     *
//*  This will be the base model for implementations that utilize       *
//*  the Support Vector Machine.                                        *
//***********************************************************************


#include "RunLog.h"
#include "KKStr.h"

#include "Model.h"
#include "ModelParam.h"
#include "ModelParamSvmBase.h"
#include "svm2.h"


namespace  KKMLL  
{
  class  ModelSvmBase: public Model
  {
  public:

    typedef  ModelSvmBase*  ModelSvmBasePtr;


    ModelSvmBase ();

    ModelSvmBase (FileDescPtr _fileDesc);

    ModelSvmBase (const KKStr&             _name,
                  const ModelParamSvmBase& _param,         // Create new model from
                  FileDescPtr              _fileDesc
                 );
  
    ModelSvmBase (const ModelSvmBase&   _model);

    virtual  ~ModelSvmBase ();

    virtual  void  CancelFlag (bool  _cancelFlag);

    virtual  KKStr        Description ()  const;

    virtual  ModelSvmBasePtr    Duplicate ()  const;

    virtual  kkint32      MemoryConsumedEstimated ()  const;

    virtual ModelTypes    ModelType () const  {return ModelTypes::mtSvmBase;}

    virtual  kkint32      NumOfSupportVectors () const;

    ModelParamSvmBasePtr  Param ();

    virtual  MLClassPtr   Predict (FeatureVectorPtr  image,
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
    void  ProbabilitiesByClass (FeatureVectorPtr    example,
                                const MLClassList&  _mlClasses,
                                kkint32*            _votes,
                                double*             _probabilities,
                                RunLog&             _log
                               );

    /**
     *@brief Derives predicted probabilities by class.
     *@details Will get the probabilities assigned to each class by the classifier.  The 
     *         '_mlClasses' parameter dictates the order of the classes. That is the 
     *         probabilities for any given index in '_probabilities' will be for the class
     *         specified in the same index in '_mlClasses'.
     *@param[in]  _example       FeatureVector object to calculate predicted probabilities for.
     *@param[in]  _mlClasses  List image classes that caller is aware of.  This should be the
     *            same list that was used when constructing this Model object.  The list must 
     *            be the same but not necessarily in the same order as when Model was 1st 
     *            constructed.
     *@param[out] _probabilities An array that must be as big as the number of classes as in
     *            mlClasses.  The probability of class in mlClasses[x] will be returned 
     *            in probabilities[x].
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


    virtual  
    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**      crossProbTable,  /**< two dimension matrix that needs to be classes.QueueSize () squared. */
                                  RunLog&       log
                                 );



    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership,  /*!< Model will take ownership of these examples */
                               RunLog&               _log
                              );


    virtual  void  WriteSpecificImplementationXML (ostream&  o,
                                                   RunLog&   log
                                                  );


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;



  protected:
    SVM289_MFS::Svm_Model*  svmModel;
    ModelParamSvmBasePtr    param;   /*!<   We will NOT own this instance. It will point to same instance defined in parent class Model.  */
  };  /* ModelSvmBase */
  
  typedef  ModelSvmBase::ModelSvmBasePtr  ModelSvmBasePtr;


  typedef  XmlElementModelTemplate<ModelSvmBase>  XmlElementModelSvmBase;
  typedef  XmlElementModelSvmBase*  XmlElementModelSvmBasePtr;
} /* namespace  KKMLL */

#endif
