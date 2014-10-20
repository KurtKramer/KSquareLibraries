#ifndef  _NORMALIZATIONPARMS_
#define  _NORMALIZATIONPARMS_

#include "Attribute.h"
#include "KKBaseTypes.h"
#include "FeatureNumList.h"
#include "KKStr.h"



namespace KKMachineLearning
{

  #ifndef  _RUNLOG_
  class  RunLog;
  typedef  RunLog*  RunLogPtr;
  #endif


  #ifndef  _FeatureVector_Defined_
  class  FeatureVector;
  typedef  FeatureVector*  FeatureVectorPtr;
  #endif


  #ifndef  _FeatureVectorList_Defined_
  class  FeatureVectorList;
  typedef  FeatureVectorList*  FeatureVectorListPtr;
  #endif


  #ifndef  _FILEDESC_
  class  FileDesc;
  typedef  FileDesc*  FileDescPtr;
  #endif


  #ifndef  _TrainingConfiguration2Defined_
  class    TrainingConfiguration2;
  typedef  TrainingConfiguration2*  TrainingConfiguration2Ptr;
  #endif

  #ifndef  _MODELPARAM_
  class  ModelParam;
  typedef  ModelParam*  ModelParamPtr;
  #endif



  /**
   *@brief  Normalization Parameters;  calculation and implementation.
   *@details Normalization parameters will be calculated for all features but 
   *         when individual examples are normalized, only the ones specified by 
   *        _featuresToNormalize' will be normalized.              
   */

  class  NormalizationParms
  {
  public:
    NormalizationParms (const ModelParam&   _param,
                        FeatureVectorList&  _examples,
                        RunLog&             _log
                       );

    NormalizationParms (TrainingConfiguration2Ptr  _config,
                        FeatureVectorList&         _examples,
                        RunLog&                    _log
                       );

    NormalizationParms (FileDescPtr   _fileDesc,
                        KKStr         _fileName,
                        bool&         _successfull,
                        RunLog&       _log
                       );

    NormalizationParms (FileDescPtr   _fileDesc,
                        FILE*         _in,
                        bool&         _successfull,
                        RunLog&       _log
                       );

    NormalizationParms (FileDescPtr   _fileDesc,
                        istream&      _in,
                        bool&         _successfull,
                        RunLog&       _log
                       );

    ~NormalizationParms ();

    int32   MemoryConsumedEstimated ()  const;

    void  NormalizeExamples (FeatureVectorListPtr  examples);

    void  NormalizeAExample (FeatureVectorPtr  example);

    FeatureVectorPtr  ToNormalized (FeatureVectorPtr  example)  const;

    int32   NumOfFeatures ()  {return numOfFeatures;}


    void  Read (FILE*  i,
                bool&  sucessful
               );


    void  Read (istream&  i,
                bool&     sucessful
               );


    void  Save (const KKStr&  _fileName,
                bool&         _successfull
               );

    void  Write (std::ostream&  o);

    float  Mean (int32 i);
    float  Sigma (int32 i);

    const float*  Mean ()  {return  mean;}
    const float*  Sigma () {return  sigma;}


  private:
    void  ConstructNormalizeFeatureVector ();

    KKMachineLearning::AttributeTypeVector  attriuteTypes;
    FileDescPtr               fileDesc;
    KKStr                     fileName;
    RunLog&                   log;
    float*                   mean;
    bool*                     normalizeFeature;
    bool                      normalizeNominalFeatures;
    int32                       numOfFeatures;
    float                    numOfExamples;
    float*                   sigma;
  };  /* NormalizationParms */


  typedef  NormalizationParms*  NormalizationParmsPtr;


  #define  _NormalizationParmsDefined_

}  /* namespace KKMachineLearning { */


#endif