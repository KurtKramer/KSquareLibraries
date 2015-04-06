#ifndef  _SVMPARAM_
#define  _SVMPARAM_




#include  "ClassAssignments.h"
#include  "FeatureNumList.h"
#include  "MLClass.h"
#include  "FileDesc.h"
#include  "RunLog.h"
#include  "KKStr.h"
#include  "svm.h"
using namespace SVM233;


namespace KKMachineLearning
{

  #ifndef  _BINARYCLASSPARMS_
  class  BinaryClassParms;
  typedef  BinaryClassParms*  BinaryClassParmsPtr;
  class  BinaryClassParmsList;
  typedef  BinaryClassParmsList*  BinaryClassParmsListPtr;
  #endif

  typedef  enum  {MachineType_NULL, 
                  OneVsOne, 
                  OneVsAll, 
                  BinaryCombos,
                  BoostSVM
                 }         
                  SVM_MachineType;

  typedef  enum  {SelectionMethod_NULL, SelectByVoting, SelectByProbability}  SVM_SelectionMethod;

  typedef  enum  {NoEncoding, BinaryEncoding, ScaledEncoding, Encoding_NULL}  SVM_EncodingMethod;

  typedef  enum  {BRNull,
                  BRnoCompression, 
                  BRcompressionPost, 
                  BRcompressionPre, 
                  BRunBalancedVariance,
                  BRunBalancedSpecified    // Meaning we will tell you which features are unbalanced.
                 }       
                    SVM_CompressionMethod;

  typedef  enum  {KT_Linear, KT_Polynomial, KT_RBF, KT_NULL}                  SVM_KernalType;


  #if  !defined(_FeatureVectorList_Defined_)
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  /**
    *@brief This class encapsulates are the information necessary to build a SVMModel class.
    *@see  SVMModel
    *@see  SVM233
    *@see  BinaryClassParms
    */
  class  SVMparam
  {
  public:
    SVMparam  (KKStr&                 _cmdLineStr,
               const FeatureNumList&  _selectedFeatures,
               FileDescPtr            _fileDesc,
               RunLog&                _log,
               bool&                  _validFormat
              );


    SVMparam  (FileDescPtr  _fileDesc,
               RunLog&      _log
              );


    SVMparam  (const SVMparam&  _svmParam);

    ~SVMparam  ();


    void    Load (ClassAssignments& _mlClasses,
                  KKStr&            _fileName,
                  bool&             _successful
                 );

    void    ReadXML (FILE*  i);

    void    ReadXML (istream&  i);

    void    Save (KKStr&  _fileName,
                  bool&    _successful
                 );

    void    WriteXML (std::ostream&  o)  const;


    void    AddBinaryClassParms (BinaryClassParmsPtr  binaryClassParms);

    void    AddBinaryClassParms (MLClassPtr             class1,
                                 MLClassPtr             class2,
                                 const svm_parameter&   _param,
                                 const FeatureNumList&  _selectedFeatures,
                                 float                  _weight
                                );

    float   AvgMumOfFeatures ();
    float   AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const;

    BinaryClassParmsPtr  GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                          MLClassPtr  class2
                                                         );

    const FeatureNumList&  GetFeatureNums ()  const;

    const FeatureNumList&  GetFeatureNums (MLClassPtr  class1,
                                           MLClassPtr  class2
                                          )  const;

    void    ProcessSvmParameter (svm_parameter&  _param,
                                 KKStr           cmd,
                                 KKStr           value,
                                 double          valueNum,
                                 bool&           parmUsed
                                );

    // Member access methods
    float                    A_Param                    () const {return param.A;}

    const
    BinaryClassParmsListPtr  BinaryParmsList            () const {return binaryParmsList;}

    double                   C_Param                    () const {return param.C;}

    double                   C_Param  (MLClassPtr  class1,
                                       MLClassPtr  class2
                                      )  const;
    SVM_EncodingMethod       EncodingMethod             () const {return encodingMethod;}

    float                    FeatureCountNet            () const;

    double                   Gamma                      () const {return param.Gamma ();}

    SVM_KernalType           KernalType                 () const {return (SVM_KernalType)param.KernalType ();}

    SVM_MachineType          MachineType                () const {return machineType;}

    kkint32                  MemoryConsumedEstimated    () const;

    kkint32                  NumOfFeaturesAfterEncoding () const;

    const svm_parameter&     Param                      () const {return param;}

    float                    SamplingRate               () const {return samplingRate;}

    const FeatureNumList&    SelectedFeatures           () const {return selectedFeatures;}

    SVM_SelectionMethod      SelectionMethod            () const {return selectionMethod;}

    bool                     UseProbabilityToBreakTies  () const {return useProbabilityToBreakTies;}


    // Member update methods
    void  A_Param            (float       _A);
    void  C_Param            (double      _CC);

    void  C_Param            (MLClassPtr  class1,
                              MLClassPtr  class2,
                              double      cParam
                             );
    void  EncodingMethod     (SVM_EncodingMethod     _encodingMethod)     {encodingMethod    = _encodingMethod;}
    void  Gamma              (double                 _gamma)              {param.Gamma (_gamma);}
    void  Gamma_Param        (double                 _gamma)              {Gamma (_gamma);}
    void  KernalType         (SVM_KernalType         _kernalType)         {param.KernalType (_kernalType);}

    void  MachineType        (SVM_MachineType        _machineType)        {machineType        = _machineType;}
    void  SamplingRate       (float                  _samplingRate)       {samplingRate       = _samplingRate;}
    void  SelectedFeatures   (const FeatureNumList&  _selectedFeatures)   {selectedFeatures   = _selectedFeatures;}

    void  SetBinaryClassFields (MLClassPtr             class1,
                                MLClassPtr             class2,
                                const svm_parameter&   _param,
                                const FeatureNumList&  _features,
                                float                  _weight
                               );

    void  SetFeatureNums (const FeatureNumList&  _features);

    void  SetFeatureNums    (MLClassPtr             class1,
                             MLClassPtr             class2,
                             const FeatureNumList&  _features,
                             float                  _weight = -1  /**< -1 Indicates use existing value.  */
                            );

    void  SelectionMethod   (SVM_SelectionMethod    _selectionMethod)  {selectionMethod  = _selectionMethod;}


    // Other Methods
    KKStr  SvmParamToString (const  svm_parameter&  _param)  const;

    KKStr  ToString ()  const;


    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed,
                                 bool&         _validFormat
                                );


  private:
    //void  DecodeParamStr (KKStr&         _paramStr,
    //                      svm_parameter&  _param
    //                     );

    void  ParseCmdLine (KKStr   _cmdLineStr,
                        bool&   _validFormat
                       );

    BinaryClassParmsListPtr  binaryParmsList;

    SVM_EncodingMethod       encodingMethod;

    FileDescPtr              fileDesc;

    KKStr                    fileName;

    RunLog&                  log;

    SVM_MachineType          machineType;

    svm_parameter            param;             // From SVMlib2

    float                    samplingRate;      // USed with BoostSVM

    FeatureNumList           selectedFeatures;  // Feature Number to use.

    SVM_SelectionMethod      selectionMethod;

    bool                     useProbabilityToBreakTies;  //  When true use the probability function to break
                                                         //  voting ties.

    bool                     validParam;

  };  /* SVMparam */




  typedef  SVMparam*   SVMparamPtr;


  KKStr   EncodingMethodToStr  (SVM_EncodingMethod     encodingMethod);
  KKStr   KernalTypeToStr      (SVM_KernalType         kernalType);
  KKStr   MachineTypeToStr     (SVM_MachineType        machineType);
  KKStr   SelectionMethodToStr (SVM_SelectionMethod    selectionMethod);


  SVM_EncodingMethod     EncodingMethodFromStr    (const KKStr&  encodingMethodStr);
  SVM_KernalType         KernalTypeFromStr        (const KKStr&  kernalTypeStr);
  SVM_MachineType        MachineTypeFromStr       (const KKStr&  machineTypeStr);
  SVM_SelectionMethod    SelectionMethodFromStr   (const KKStr&  selectionMethodStr);


}  /* namespace KKMachineLearning */



#endif


