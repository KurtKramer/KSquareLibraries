#ifndef  _MODELOLDSVM_
#define  _MODELOLDSVM_


/**
 *@class KKMachineLearning::ModelOldSVM
 *@browse
 *@details
 *@code
 ************************************************************************
 **                           ModelOldSVM                               *
 **                                                                     *
 **  This is a specialization of 'Model'.  It is meant to Wrap the      *
 **  original version of 'SvmModel'.  This class will allow us to use   *
 **  the original implementation using version 2.39 of LibSVM.          *
 **                                                                     *
 ************************************************************************
 *@endcode
 */


#include  "KKStr.h"
#include  "ClassAssignments.h"
#include  "FileDesc.h"
#include  "MLClass.h"
#include  "FeatureEncoder2.h"
#include  "FeatureNumList.h"
#include  "FeatureVector.h"
#include  "RunLog.h"
#include  "svm.h"
#include  "Model.h"
#include  "ModelParamOldSVM.h"
#include  "SVMModel.h"



namespace KKMachineLearning
{

#ifndef  _FEATUREENCODER_
class  FeatureEncoder2;
typedef  FeatureEncoder2*  FeatureEncoder2Ptr;
#endif


  class  ModelOldSVM:  public Model
  {
  public:
    typedef  ModelOldSVM*  ModelOldSVMPtr;

    ModelOldSVM (FileDescPtr    _fileDesc,
                 VolConstBool&  _cancelFlag,
                 RunLog&        _log
                );

    /**
     *@brief Creates a new svm model from the provided example (example) data
     *@param[in] _name  
     *@param[in] _param The parameters for the svm, and for creating the model.
     *@param[in] _fileDesc A description of the data file.
     *@param[in] _cancelFlag  If you want this instance to stop processing set this field to true in another thread.
     *@param[in] _log A log-file stream. All important events will be output to this stream
     */
    ModelOldSVM (const KKStr&             _name,
                 const ModelParamOldSVM&  _param,
                 FileDescPtr              _fileDesc,
                 VolConstBool&            _cancelFlag,
                 RunLog&                  _log
                );
  
    ModelOldSVM (const ModelOldSVM&   _madel);
  
    virtual ~ModelOldSVM ();

    virtual
    ModelOldSVMPtr  Duplicate ()  const;

    virtual ModelTypes       ModelType () const  {return mtOldSVM;}

    const ClassAssignments&  Assignments () const;

    const FeatureNumList&    GetFeatureNums ()  const;

    FeatureNumList           GetFeatureNums (MLClassPtr  class1,
                                             MLClassPtr  class2
                                            );

    virtual
    kkint32                  MemoryConsumedEstimated ()  const;

    virtual
    bool                     NormalizeNominalAttributes ()  const;

    kkint32                  NumOfSupportVectors  () const;

    void                     SupportVectorStatistics (kkint32& numSVs,
                                                      kkint32& totalNumSVs
                                                     );

    ModelParamOldSVMPtr      Param                () const;

    SVM_SelectionMethod      SelectionMethod      () const;

    SVMModelPtr              SvmModel             () const {return svmModel;}


    virtual
    MLClassPtr   Predict (FeatureVectorPtr  image);
  
    virtual
    void         Predict (FeatureVectorPtr  example,
                          MLClassPtr        knownClass,
                          MLClassPtr&       predClass1,
                          MLClassPtr&       predClass2,
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

    
    
    
    
     /**
      *@brief  Will get the probabilities assigned to each class.
      *@param[in]  example         unknown example that we want to get predicted probabilities for. 
      *@param[in]  _ImageClasses   List classes that caller is aware of.  This should be the same list that 
      *                            was used when constructing this ModelOldSVM object.  The list must be the
      *                            same but not necessarily in the same order as when ModelOldSVM was 1st
      *                            constructed.  The ordering of this instance dictates how the parameters
      *                            '_votes' and '_probabilities' are populated.
      *@param[out] _votes          Number of votes for each class
      *@param[out] _probabilities  An array that must be as big as the number of classes in _mlClasses.  
      *                            The probability of class in _mlClasses[x] will be returned in _probabilities[x].
      */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    example,
                                const MLClassList&  _mlClasses,
                                kkint32*            _votes,
                                double*             _probabilities
                               );


     /**
      ******************************************************************************************************************
      *@brief  Will get the probabilities assigned to each class.
      *@param[in]  example unknown example that we want to get predicted probabilities for. 
      *@param[in]  _ImageClasses  List classes that caller is aware of.  This should be the same list that 
      *                           was used when constructing this ModelOldSVM object.  The list must be the
      *                           same but not necessarily in the same order as when ModelOldSVM was 1st
      *                           constructed.
      *@param[out] _probabilities  An array that must be as big as the number of classes in _mlClasses.  
      *                            The probability of class in _mlClasses[x] will be returned in _probabilities[x].
      */
    virtual
    void  ProbabilitiesByClass (FeatureVectorPtr    _example,
                                const MLClassList&  _mlClasses,
                                double*             _probabilities
                               );



    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and recompute the decision boundary and probability.  It will then
     *          return the S/V's that when removed improve the probability in 'c1's 
     *          the most.
     *@param[in]  example  Example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    vector<ProbNamePair>  FindWorstSupportVectors (FeatureVectorPtr  example,
                                                   kkint32           numToFind,
                                                   MLClassPtr        c1,
                                                   MLClassPtr        c2
                                                  );


    /**
     *@brief  For a given two class pair return the names of the 'numToFind' worst S/V's.
     *@details  This method will iterate through all the S/V's removing them one at a 
     *          time and retraining a new SVM and then comparing with the new prediction results.
     *@param[in]  example  Example that was classified incorrectly.
     *@param[in]  numToFind  The number of the worst examples you are looking for.
     *@param[in]  c1  Class that the 'example; parameter should have been classed as.
     *@param[in]  c2  Class that it was classified as.
     */
    vector<ProbNamePair>  FindWorstSupportVectors2 (FeatureVectorPtr  example,
                                                    kkint32           numToFind,
                                                    MLClassPtr        c1,
                                                    MLClassPtr        c2
                                                   );


    /**
     *@brief  ModelOldSVM Specific 'PrepExampleForPrediction'  will only normalize data.
     *@param[in]  fv  Feature vector of example that needs to be prepared.
     *@param[out]  newExampleCreated  Indicates if either Feature Encoding and/or Normalization needed
     *             to be done.  If neither then the original instance is returned.  If Yes then 
     *             a new instance which the caller will have to be delete will be returned.
     */
    virtual
    FeatureVectorPtr   PrepExampleForPrediction (FeatureVectorPtr  fv,
                                                 bool&             newExampleCreated
                                                );    
    
    vector<KKStr>  SupportVectorNames ()  const;


    vector<KKStr>  SupportVectorNames (MLClassPtr  c1,
                                       MLClassPtr  c2
                                      )  const;


    void  RetrieveCrossProbTable (MLClassList&  classes,
                                  double**         crossProbTable  // two dimension matrix that needs to be classes.QueueSize ()  squared.
                                 );


    virtual  void  ReadSpecificImplementationXML (istream&  i,
                                                  bool&     _successful
                                                 );

    /**
     * @brief Use given training data to create a trained Model that can be used for classifying examples.
     * @param[in] _trainExamples      The example data we will be building the model from.
     * @param[in] _alreadyNormalized  Specifies weather the training data has already been normalized.
     * @param[in] _takeOwnership      If true this instance will take ownership of '_trainExamples' and delete it when done with it.
     */
    virtual  void  TrainModel (FeatureVectorListPtr  _trainExamples,
                               bool                  _alreadyNormalized,
                               bool                  _takeOwnership  /*!< Model will take ownership of these examples */
                              );


    virtual  void  WriteSpecificImplementationXML (ostream&  o);



  private:
    ClassAssignmentsPtr  assignments;
    SVMModelPtr          svmModel;
  };


  typedef  ModelOldSVM::ModelOldSVMPtr  ModelOldSVMPtr;
} /* namespace KKMachineLearning */



#endif

