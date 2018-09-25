#if  !defined(_NORMALIZATIONPARMS_)
#define  _NORMALIZATIONPARMS_

#include "Attribute.h"
#include "KKBaseTypes.h"
#include "FeatureNumList.h"
#include "KKStr.h"



namespace KKMLL
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


  #ifndef  _TrainingConfiguration2_Defined_
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
   * when individual examples are normalized, only the ones specified by 
   * _featuresToNormalize' will be normalized.              
   */
  class  NormalizationParms
  {
  public:
    typedef  NormalizationParms*  NormalizationParmsPtr;

    typedef  NormalizationParms const *  NormalizationParmsConstPtr;

    NormalizationParms ();

    NormalizationParms (bool                _normalizeNominalFeatures,
                        FeatureVectorList&  _examples,
                        RunLog&             _log
                       );

    NormalizationParms (const ModelParam&   _param,
                        FeatureVectorList&  _examples,
                        RunLog&             _log
                       );

    NormalizationParms (TrainingConfiguration2Ptr  _config,
                        FeatureVectorList&         _examples,
                        RunLog&                    _log
                       );

    ~NormalizationParms ();

    kkMemSize  MemoryConsumedEstimated ()  const;

    FileDescConstPtr  FileDesc ()  const  {return fileDesc;}

    void  NormalizeExamples (FeatureVectorListPtr  examples,
                             RunLog&               log
                            )  const;

    void  NormalizeAExample (FeatureVectorPtr  example)  const;

    bool  NormalizeNominalFeatures ()  const  {return normalizeNominalFeatures;}

    FeatureVectorPtr  ToNormalized (FeatureVectorPtr  example)  const;

    kkuint32 NumOfFeatures ()  const {return numOfFeatures;}

    float   NumOfExamples ()  const {return numOfExamples;}

    void  ReadXML (XmlStream&     s,
                   XmlTagPtr      tag,
                   VolConstBool&  cancelFlag,
                   RunLog&        log
                  );


    void  WriteToFile (const KKStr&  _fileName,  bool& _successfull,  RunLog& _log)  const;

    
    //void  Write (std::ostream&  o);


    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;


    double  Mean (kkuint32  i,
                  RunLog&   log
                 );


    double  Sigma (kkuint32 i,
                   RunLog&  log
                  );

    
    const double*  Mean  () const  {return  mean;}
    const double*  Sigma () const  {return  sigma;}

    static
    NormalizationParmsConstPtr  ReadFromFile (const KKStr&  fileName,  RunLog& log);

  private:
    void  ConstructNormalizeFeatureVector ();
    void  DeriveNormalizationParameters (FeatureVectorList&  _examples);

    AttributeTypeVector  attriuteTypes;
    FileDescConstPtr     fileDesc;
    mutable KKStr        fileName;
    double*              mean;
    bool*                normalizeFeature;
    bool                 normalizeNominalFeatures;
    kkuint32             numOfFeatures;
    float                numOfExamples;
    double*              sigma;
  };  /* NormalizationParms */


  typedef  NormalizationParms::NormalizationParmsPtr       NormalizationParmsPtr;
  typedef  NormalizationParms::NormalizationParmsConstPtr  NormalizationParmsConstPtr;


  #define  _NormalizationParms_Defined_



  typedef  XmlElementTemplate<NormalizationParms>  XmlElementNormalizationParms;
  typedef  XmlElementNormalizationParms*  XmlElementNormalizationParmsPtr;


}  /* KKMLL */


#endif
