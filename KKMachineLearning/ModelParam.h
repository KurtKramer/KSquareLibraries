#ifndef  _MODELPARAM_
#define  _MODELPARAM_


#include  "FeatureNumList.h"
#include  "FileDesc.h"
#include  "RunLog.h"
#include  "KKStr.h"


namespace KKMachineLearning {


/**
  @brief  Abstract Base class for Machine Learning parameters.
  @author Kurt Kramer
  @details
  For each Machine Learning algorithm implemented you would create a specialization of this class
  to manage the parameters required by the algorithm.  Specifically for each new class that you
  create that is derived from 'Model' you will new to create a class derived from 'ModelParam'.
  This class encapsulates general parameters that are common to all Machine Learning Models.   *
  @see  Model
  */
  class  ModelParam
  {
  public:
    typedef  ModelParam*  ModelParamPtr;

    typedef  enum  {mptNULL, mptKNN, mptOldSVM, mptSvmBase, mptUsfCasCor}  ModelParamTypes;
    static KKStr            ModelParamTypeToStr   (ModelParamTypes _modelParamType);
    static ModelParamTypes  ModelParamTypeFromStr (const KKStr&    _modelParamTypeStr);
  
    typedef  enum  {NoEncoding, BinaryEncoding, ScaledEncoding, Encoding_NULL}  EncodingMethodType;
  
    ModelParam  (FileDescPtr  _fileDesc,
                 RunLog&      _log
                );
  
  
    ModelParam  (const ModelParam&  _param);
  
    virtual
    ~ModelParam  ();

    static 
    ModelParamPtr  CreateModelParam (istream&     i,
                                     FileDescPtr  _fileDesc,
                                     RunLog&      _log
                                    );

    virtual
    ModelParamPtr  Duplicate () const = 0;


    virtual
    kkint32   MemoryConsumedEstimated ()  const;


    virtual
    void    ParseCmdLine (KKStr  _cmdLineStr,
                          bool&   _validFormat
                         );


    virtual
    void   ParseCmdLinePost ();


    virtual
    void    ReadXML (KKStr&  _fileName,
                     bool&   _successful
                    );
  
    virtual
    void    ReadXML (istream&  i);
  
    virtual
    void    ReadSpecificImplementationXML (istream&  i) = 0;
  

    /*!
     @brief Creates a a Command Line String that represents these parameters.
     @details  All derived classes should implement this method.  They should first call this method and
               then append there own parameters that are specific to their implementation.
     */
    virtual
    KKStr   ToCmdLineStr ()  const;
  

    virtual
    void    WriteXML (std::ostream&  o)  const;
  
    virtual
    void    WriteSpecificImplementationXML (std::ostream&  o)  const = 0;


    // Member access methods

    virtual ModelParamTypes          ModelParamType             () const = 0;
    virtual KKStr                    ModelParamTypeStr          () const {return ModelParamTypeToStr (ModelParamType ());}

    virtual float                    AvgMumOfFeatures           () const {return (float)selectedFeatures.NumOfFeatures ();}
    virtual EncodingMethodType       EncodingMethod             () const {return encodingMethod;}
    virtual KKStr                    EncodingMethodStr          () const {return EncodingMethodToStr (encodingMethod);}
    virtual kkint32                  ExamplesPerClass           () const {return examplesPerClass;}
    virtual const KKStr&             FileName                   () const {return fileName;}
    virtual bool                     NormalizeNominalFeatures   () const {return normalizeNominalFeatures;}
    virtual kkint32                  NumOfFeaturesAfterEncoding () const;
    virtual const FeatureNumList&    SelectedFeatures           () const {return selectedFeatures;}
    virtual bool                     ValidParam                 () const {return validParam;}
  
    // Access members that were originally put here for 'ModelSVMBase'  and  'ModelOldSVM'
    virtual float   A_Param  () const;
    virtual double  C_Param  () const;  // Same as 'Cost'
    virtual double  Cost     () const;
    virtual double  Gamma    () const;
    virtual float   Prob     () const;


    // Member update methods
    virtual void  EncodingMethod     (EncodingMethodType     _encodingMethod)     {encodingMethod     = _encodingMethod;}
    virtual void  ExamplesPerClass   (kkint32                _examplesPerClass)   {examplesPerClass   = _examplesPerClass;}
    virtual void  FileName           (const KKStr&           _fileName)           {fileName           = _fileName;}
    virtual void  SelectedFeatures   (const FeatureNumList&  _selectedFeatures)   {selectedFeatures   = _selectedFeatures;}
    virtual void  ValidParam         (bool                   _validParam)         {validParam         = _validParam;}


    virtual void  A_Param  (float   _prob);
    virtual void  C_Param  (double  _cost);   // Same as 'Cost'
    virtual void  Cost     (double  _cost);
    virtual void  Gamma    (double  _gamma);
    virtual void  Prob     (float   _prob);

    static  KKStr   EncodingMethodToStr    (EncodingMethodType     encodingMethod);


    static  EncodingMethodType     EncodingMethodFromStr    (const KKStr&  encodingMethodStr);


  protected:
    FileDescPtr    fileDesc;

    RunLog&        log;
  

  private:
 
    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed
                                ) = 0;


    EncodingMethodType       encodingMethod;

    kkint32                  examplesPerClass;

    KKStr                    fileName;

    bool                     normalizeNominalFeatures;

    FeatureNumList           selectedFeatures;    // Feature Number to use.

    bool                     validParam;

    // following parameters are placed hear to support SVM based algorithms.
    // to help transition from old(CRAPPY) design to newer less crappy design.
    double  cost;
    double  gamma;
    float   prob;


  };  /* ModelParam */

  typedef  ModelParam::ModelParamPtr   ModelParamPtr;

}  /* namespace KKMachineLearning */



#endif
