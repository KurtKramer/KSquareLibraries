#ifndef  _SVMPARAM_
#define  _SVMPARAM_


#include "ClassAssignments.h"
#include "FeatureNumList.h"
#include "FileDesc.h"
#include "KKStr.h"
#include "MLClass.h"
#include "RunLog.h"
#include "svm.h"
using namespace SVM233;


namespace KKMLL
{

  #ifndef  _BINARYCLASSPARMS_
  class  BinaryClassParms;
  typedef  BinaryClassParms*  BinaryClassParmsPtr;
  class  BinaryClassParmsList;
  typedef  BinaryClassParmsList*  BinaryClassParmsListPtr;
  #endif


  enum  class SVM_MachineType 
                 {MachineType_NULL, 
                  OneVsOne, 
                  OneVsAll, 
                  BinaryCombos,
                  BoostSVM
                 };

  enum  class  SVM_SelectionMethod  
                 {SelectionMethod_NULL, 
                  SelectByVoting, 
                  SelectByProbability
                 };


  /***
   *SVM_EncodingMethod  refers to how Nominal and Symbolic features encoded.
   *BinaryEncoding   
   */

  enum  class  SVM_EncodingMethod  
                 {NoEncoding, 
                  BinaryEncoding, 
                  ScaledEncoding, 
                  Encoding_NULL
                 };


  enum  class  SVM_KernalType 
                 {KT_Linear,
                  KT_Polynomial,
                  KT_RBF,
                  KT_NULL
                 };


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
    SVMparam  (KKStr&                  _cmdLineStr,
               FeatureNumListConstPtr  _selectedFeatures,  /**< Will make own instance; caller maintains ownership status. */
               bool&                   _validFormat,
               RunLog&                 _log
              );


    SVMparam  ();

    SVMparam  (const SVMparam&  _svmParam);

    ~SVMparam  ();


    void    Load (KKStr&            _fileName,
                  FileDescPtr       _fileDesc,
                  ClassAssignments& _mlClasses,
                  bool&             _successful,
                  RunLog&           _log
                 );

    void    ReadXML (FILE*        i,
                     FileDescPtr  fileDesc,
                     RunLog&      log
                    );

    void    ReadXML (istream&     i,
                     FileDescPtr  fileDesc,
                     RunLog&      log
                    );

    void    Save (KKStr&  _fileName,
                  bool&   _successful
                 );

    void    WriteXML (std::ostream&  o)  const;


    void    AddBinaryClassParms (BinaryClassParmsPtr  binaryClassParms);

    void    AddBinaryClassParms (MLClassPtr              class1,
                                 MLClassPtr              class2,
                                 const svm_parameter&    _param,
                                 FeatureNumListConstPtr  _selectedFeatures,  /**< We will make our own copy; caller will retain ownership status.  */
                                 float                   _weight
                                );

    float   AvgMumOfFeatures (FileDescPtr fileDesc) const;
    float   AvgNumOfFeatures (FeatureVectorListPtr  trainExamples)  const;


    /** If binary class parms don't exist will return NULL. */
    BinaryClassParmsPtr   GetBinaryClassParms (MLClassPtr  class1,
                                               MLClassPtr  class2
                                              );

    BinaryClassParmsPtr  GetParamtersToUseFor2ClassCombo (MLClassPtr  class1,
                                                          MLClassPtr  class2
                                                         );

    FeatureNumListConstPtr GetFeatureNums ()  const;

    FeatureNumListConstPtr GetFeatureNums (FileDescPtr  fileDesc)  const;

    FeatureNumListConstPtr GetFeatureNums (FileDescPtr  fileDesc,
                                           MLClassPtr   class1,
                                           MLClassPtr   class2
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

    kkint32                  NumOfFeaturesAfterEncoding (FileDescPtr  fileDesc,
                                                         RunLog&      log
                                                        ) const;

    const svm_parameter&     Param                      () const {return param;}

    const VectorFloat&       ProbClassPairs             () const {return probClassPairs;}

    float                    SamplingRate               () const {return samplingRate;}

    FeatureNumListConstPtr   SelectedFeatures           () const {return selectedFeatures;}

    FeatureNumListConstPtr    SelectedFeatures           (FileDescPtr  fileDesc)  const;

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
    void  KernalType         (SVM_KernalType         _kernalType)         {param.KernalType ((int)_kernalType);}

    void  MachineType        (SVM_MachineType        _machineType)        {machineType        = _machineType;}
    void  SamplingRate       (float                  _samplingRate)       {samplingRate       = _samplingRate;}
    void  SelectedFeatures   (FeatureNumListConst&    _selectedFeatures);
    void  SelectedFeatures   (FeatureNumListConstPtr  _selectedFeatures);


    void  SetBinaryClassFields (MLClassPtr              class1,
                                MLClassPtr              class2,
                                const svm_parameter&    _param,
                                FeatureNumListConstPtr  _features,
                                float                   _weight
                               );

    void  SetFeatureNums (FeatureNumListConstPtr  _features);
    void  SetFeatureNums (FeatureNumListConst&    _features);

    void  SetFeatureNums    (MLClassPtr              class1,
                             MLClassPtr              class2,
                             FeatureNumListConstPtr  _features,
                             float                   _weight = -1  /**< -1 Indicates use existing value.  */
                            );

    void  SelectionMethod   (SVM_SelectionMethod   _selectionMethod)  {selectionMethod  = _selectionMethod;}


    // Other Methods
    KKStr  SvmParamToString (const  svm_parameter&  _param)  const;

    KKStr  ToString ()  const;


    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed,
                                 bool&         _validFormat,
                                 RunLog&       log
                                );

    void  ProbClassPairsInitialize (const ClassAssignments&  assignments);


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;


  private:
    //void  DecodeParamStr (KKStr&         _paramStr,
    //                      svm_parameter&  _param
    //                     );

    void  ParseCmdLine (KKStr    _cmdLineStr,
                        bool&    _validFormat,
                        RunLog&  _log
                       );

    BinaryClassParmsListPtr  binaryParmsList;

    SVM_EncodingMethod       encodingMethod;

    KKStr                    fileName;

    SVM_MachineType          machineType;

    svm_parameter            param;             // From SVMlib2

    VectorFloat              probClassPairs;   

    float                    samplingRate;      // USed with BoostSVM

    mutable
    FeatureNumListPtr        selectedFeatures;  /**< Feature Numbers to use. */

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



  typedef  XmlElementTemplate<SVMparam>  XmlElementSVMparam;
  typedef  XmlElementSVMparam*  XmlElementSVMparamPtr;

}  /* namespace KKMLL */



#endif


