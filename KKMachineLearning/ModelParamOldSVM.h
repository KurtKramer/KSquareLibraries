#if  !defined(_MODELPARAMOLDSVM_)
#define  _MODELPARAMOLDSVM_

#include "KKStr.h"
#include "RunLog.h"

#include "BinaryClassParms.h"
#include "FeatureNumList.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "ModelParam.h"
#include "svm.h"
#include "SVMparam.h"
using namespace SVM233;


namespace KKMLL 
{

  #ifndef  _BINARYCLASSPARMS_
  class  BinaryClassParms;
  typedef  BinaryClassParms*  BinaryClassParmsPtr;
  class  BinaryClassParmsList;
  typedef  BinaryClassParmsList*  BinaryClassParmsListPtr;
  #endif


  /**
    ************************************************************************************************
    * this class encapsulates are the information necessary to build a SVMModel class.             *
    ************************************************************************************************
    * @see  ModelSVM
    * @see  SVM233
    * @see  BinaryClassParms
    */
  class  ModelParamOldSVM:  public  ModelParam
  {
  public:
    typedef  ModelParamOldSVM*  ModelParamOldSVMPtr;
    typedef  SVM233::svm_parameter   svm_parameter;

    ModelParamOldSVM  ();
  
  
    ModelParamOldSVM  (const ModelParamOldSVM&  _param);
  
    virtual 
    ~ModelParamOldSVM  ();
  
    virtual
    ModelParamOldSVMPtr  Duplicate () const;

    virtual ModelParamTypes  ModelParamType () const {return ModelParamTypes::OldSVM;}

    void    AddBinaryClassParms (BinaryClassParmsPtr  binaryClassParms);

    void    AddBinaryClassParms (MLClassPtr              class1,
                                 MLClassPtr              class2,
                                 const svm_parameter&    _param,
                                 FeatureNumListConstPtr  _selectedFeatures,
                                 float                   _weight
                                );

    virtual  float   AvgMumOfFeatures (FileDescConstPtr  fileDesc);


    /** If no entry exists for class pair then NULL will be returned. */
    BinaryClassParmsPtr   GetBinaryClassParms (MLClassPtr       class1,
                                               MLClassPtr       class2
                                              )  const;

    /**
     * If no entry exists for class pair a new one will be created using the global parameters from 
     *the underlying 'svmParameters' object.
     */
    BinaryClassParmsPtr  GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                          MLClassPtr  class2
                                                         );

    FeatureNumListConstPtr  GetFeatureNums (FileDescConstPtr  fileDesc,
                                            MLClassPtr        class1,
                                            MLClassPtr        class2
                                           )  const;


    // Member access methods
    BinaryClassParmsList const *     BinaryParmsList            () const;
    virtual float                    A_Param                    () const;
    virtual double                   C_Param                    () const;

    virtual double                   C_Param   (MLClassPtr  class1,
                                                MLClassPtr  class2
                                               )  const;

    virtual double                   Gamma                      () const;
    virtual SVM_KernalType           KernalType                 () const;
    virtual SVM_MachineType          MachineType                () const;

    virtual kkint32                  NumOfFeaturesAfterEncoding (FileDescConstPtr  fileDesc,
                                                                 RunLog&           log
                                                                ) const;

    virtual const svm_parameter&     Param                      () const;
    virtual float                    SamplingRate               () const;
    virtual FeatureNumListConstPtr   SelectedFeatures           () const;
    virtual SVM_SelectionMethod      SelectionMethod            () const;
    virtual SVMparamPtr              SvmParameters              () const;
    virtual bool                     UseProbabilityToBreakTies  () const;


    // Member update methods
    virtual void  A_Param            (float       _A);
    virtual void  C_Param            (double      _CC);

    virtual void  C_Param            (MLClassPtr  class1,
                                      MLClassPtr  class2,
                                      double      cParam
                                     );

    virtual void  EncodingMethod     (SVM_EncodingMethod    _encodingMethod);
    virtual void  EncodingMethod     (EncodingMethodType    _encodingMethod);
    virtual void  Gamma              (double                _gamma);
    virtual void  Gamma_Param        (double                _gamma);
    virtual void  KernalType         (SVM_KernalType        _kernalType);
    virtual void  MachineType        (SVM_MachineType       _machineType);
    virtual void  SamplingRate       (float                 _samplingRate);
    virtual void  SelectedFeatures   (const FeatureNumList& _selectedFeatures);
    virtual void  SelectionMethod    (SVM_SelectionMethod   _selectionMethod);


    /**
     *@brief  converts Encoding variables from  "SVM_EncodingMethod"  to  "ModelParam::EncodingMethodType"
     */
    EncodingMethodType  SVM_EncodingMethodToModelParamEncodingMethodType (SVM_EncodingMethod  _encodingMethod);


    void  SetBinaryClassFields (MLClassPtr               class1,
                                MLClassPtr               class2,
                                const svm_parameter&     _param,
                                FeatureNumListConstPtr   _features,
                                float                    _weight
                               );

    void  SetFeatureNums    (MLClassPtr              class1,
                             MLClassPtr              class2,
                             FeatureNumListConstPtr  _features,
                             float                   _weight = -1  /**< -1 Indicates use existing value. */
                            );

    void  ParseCmdLine (KKStr    _cmdLineStr,
                        bool&    _validFormat,
                        RunLog&  _log
                       );

    KKStr  SvmParamToString (const  svm_parameter&  _param)  const;

    KKStr  ToCmdLineStr ()  const;

    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelFlag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&   varName,
                             std::ostream&  o
                            )  const;

  private:
    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed,
                                 RunLog&       log
                                );

    SVMparamPtr  svmParameters;
  };  /* ModelParamOldSVM */

  typedef  ModelParamOldSVM*   ModelParamOldSVMPtr;


  typedef  XmlElementModelParamTemplate<ModelParamOldSVM>  XmlElementModelParamOldSVM;
  typedef  XmlElementModelParamOldSVM*  XmlElementModelParamOldSVMPtr;

}  /* namespace KKMLL */



#endif


