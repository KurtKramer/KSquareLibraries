#ifndef  _FEATUREENCODER2_
#define  _FEATUREENCODER2_
/*!
 \class  KKMachineLearning::FeatureEncoder2
 \code
 ***********************************************************************
 *                       FeatureEncoder2                               *
 *                                                                     *
 *  This will eventually replace "FeatureEncoder" when we shift over   *
 *  to the new paradigm where all Machine Learning Algorithms will be  *
 *  derived from 'Model'.                                              *
 ***********************************************************************
 \endcode
 */
#include  "RunLog.h"

#include  "Attribute.h"
#include  "FeatureVector.h"
#include  "FileDesc.h"
#include  "MLClass.h"
#include  "ModelParam.h"



namespace KKMachineLearning 
{
  class  FeatureEncoder2
  {
  public:
    typedef  FeatureEncoder2*  FeatureEncoder2Ptr;

    typedef  enum  {FeAsIs, FeBinary, FeScale}  FeWhatToDo;
    typedef  FeWhatToDo*  FeWhatToDoPtr;


    /**
     * @brief Constructs a Feature Encoder object.
     * @param[in] _param
     * @param[in] _fileDesc
     * @param[in] _attributeTypes   caller keeps ownership,  but encoder will continue to reference it.
     * @param[in] _cardinalityTable caller keeps ownership,  but encoder will continue to reference it.
     * @param[in] _log A logfile stream. All important events will be output to this stream
     */
    FeatureEncoder2 (const ModelParam&       _param,
                     FileDescPtr             _fileDesc,
                     RunLog&                 _log
                   );
  
    FeatureEncoder2 (const FeatureEncoder2&  _encoder);


    /**
     * @brief Frees any memory allocated by, and owned by the FeatureEncoder2
     */
    ~FeatureEncoder2 ();


    int32  CodedNumOfFeatures () const  {return codedNumOfFeatures;}

    FileDescPtr       CreateEncodedFileDesc (ostream*  o)  const;  /**< If 'o' is not NULL will write out a table showing assignments from old to new.  */

    FeatureVectorListPtr  EncodedFeatureVectorList (const  FeatureVectorList&  srcData)  const;

    FeatureVectorListPtr  EncodeAllExamples (const FeatureVectorListPtr  srcData);

    FeatureVectorPtr  EncodeAExample (FeatureVectorPtr  src)  const;

    int32             MemoryConsumedEstimated ()  const;

    int32             NumEncodedFeatures ()  const;

    void              ReadXML (istream&  i);

    void              WriteXML (istream&  o);


  private:
    const AttributeTypeVector&        attributeVector;
    int32*                            cardinalityDest;
    const VectorInt32&                cardinalityVector;   /**< Will not own, passed in by creator. */
    int32                             codedNumOfFeatures;
    int32*                            destFeatureNums;
    FeWhatToDoPtr                     destWhatToDo;
    FileDescPtr                       encodedFileDesc;
    ModelParam::EncodingMethodType    encodingMethod;
    FileDescPtr                       fileDesc;
    RunLog&                           log;
    int32                             numOfFeatures;
    int32*                            srcFeatureNums;
    const ModelParam&                 param;

    struct  FeatureVar2;
    typedef  FeatureVar2* FeatureVar2Ptr;
    class  FeatureVar2List;
    typedef  FeatureVar2List*  FeatureVar2ListPtr;
  };  /* FeatureEncoder2 */


  typedef  FeatureEncoder2::FeatureEncoder2Ptr  FeatureEncoder2Ptr;

} /* namespace KKMachineLearning */

#endif
