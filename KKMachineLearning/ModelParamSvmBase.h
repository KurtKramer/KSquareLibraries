#ifndef  _MODELPARAMSVMBASE_
#define  _MODELPARAMSVMBASE_


#include  "ModelParam.h"
#include  "svm2.h"

using namespace  SVM289_MFS;

namespace KKMachineLearning {

/**
  ************************************************************************************************
  * This class encapsulates general parameters that are common to all Machine Learning Models.   *
  ************************************************************************************************
  * @see  Model
  */
  class  ModelParamSvmBase: public  ModelParam
  {
  public:
    typedef  ModelParamSvmBase*  ModelParamSvmBasePtr;
  

    ModelParamSvmBase  (FileDescPtr  _fileDesc,
                        RunLog&      _log
                       );
  
    ModelParamSvmBase  (FileDescPtr  _fileDesc,
                        SVM_Type     _svm_type,
                        Kernel_Type  _kernelType,
                        double       _cost,
                        double       _gamma,
                        RunLog&      _log
                       );
  
    ModelParamSvmBase  (const ModelParamSvmBase&  _param);
  
    virtual
    ~ModelParamSvmBase  ();

    virtual ModelParamTypes  ModelParamType () const {return mptSvmBase;}

    virtual
    ModelParamSvmBasePtr  Duplicate () const;


    const SVM289_MFS::svm_parameter&  SvmParam ()  {return svmParam;}

    virtual void  Cost        (double       _cost);
    virtual void  Gamma       (double       _gamma);
    virtual void  KernalType  (Kernel_Type  _kernalType) {svmParam.KernalType (_kernalType);}
    virtual void  SvmType     (SVM_Type     _svm_type)   {svmParam.SvmType    (_svm_type);}

    virtual double       Cost       ()  const;
    virtual double       Gamma      ()  const;
    virtual Kernel_Type  KernalType ()  const {return  svmParam.KernalType ();}
    virtual SVM_Type     SvmType    ()  const {return  svmParam.SvmType    ();}



 
    virtual
    void    ReadSpecificImplementationXML (istream&  i);
  

    /*!
     @brief Creates a Command Line String that represents these parameters.
     @details  All derived classes should implement this method.  Thwy should first call this method and 
               then append there own parameters that are specific to their implementation.
     */
    virtual
    KKStr   ToCmdLineStr ()  const;
  

    virtual
    void    WriteSpecificImplementationXML (std::ostream&  o)  const;
  
  
  private:
    virtual
    void   ParseCmdLinePost ();  // Will get called after the entire parameter string has been processed.

    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed
                                );

    SVM289_MFS::svm_parameter   svmParam;

  };  /* ModelParamSvmBase */
  
  typedef  ModelParamSvmBase::ModelParamSvmBasePtr   ModelParamSvmBasePtr;



}  /* namespace KKMachineLearning */



#endif


