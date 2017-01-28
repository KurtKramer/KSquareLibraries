#if  !defined(_FEATUREENCODER2_)
#define  _FEATUREENCODER2_
/**
 *@class  KKMLL::FeatureEncoder2
 *@code
 ***********************************************************************
 *                       FeatureEncoder2                               *
 *                                                                     *
 *  This will eventually replace "FeatureEncoder" when we shift over   *
 *  to the new paradigm where all Machine Learning Algorithms will be  *
 *  derived from 'Model'.                                              *
 ***********************************************************************
 *@endcode
 */
#include "RunLog.h"

#include "Attribute.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "ModelParam.h"



namespace KKMLL 
{
  class  FeatureEncoder2
  {
  public:
    typedef  FeatureEncoder2*  FeatureEncoder2Ptr;

    enum  class  FeWhatToDo  {FeAsIs, FeBinary, FeScale};
    typedef  FeWhatToDo*  FeWhatToDoPtr;


    /**
     * @brief Constructs a Feature Encoder object.
     * @param[in] _param
     * @param[in] _fileDesc
     * @param[in] _log A log-file stream. All important events will be output to this stream
     */
    FeatureEncoder2 (const ModelParam&  _param,
                     FileDescConstPtr   _fileDesc
                   );
  
    FeatureEncoder2 (const FeatureEncoder2&  _encoder);


    /**
     * @brief Frees any memory allocated by, and owned by the FeatureEncoder2
     */
    ~FeatureEncoder2 ();


    kkint32  CodedNumOfFeatures () const  {return codedNumOfFeatures;}

    FileDescConstPtr  CreateEncodedFileDesc (ostream*  o,
                                             RunLog&   log
                                            )  const;  /**< If 'o' is not NULL will write out a table showing assignments from old to new.  */

    FeatureVectorListPtr  EncodedFeatureVectorList (const  FeatureVectorList&  srcData)  const;

    FeatureVectorListPtr  EncodeAllExamples (const FeatureVectorListPtr  srcData);

    FeatureVectorPtr  EncodeAExample (FeatureVectorPtr  src)  const;

    kkMemSize         MemoryConsumedEstimated ()  const;

    kkint32           NumEncodedFeatures ()  const;

    void              ReadXML (istream&  i);

    void              WriteXML (istream&  o);


  private:
    const AttributeTypeVector&        attributeVector;
    kkint32*                          cardinalityDest;
    const VectorInt32&                cardinalityVector;   /**< Will not own, passed in by creator. */
    kkint32                           codedNumOfFeatures;
    kkint32*                          destFeatureNums;
    FeWhatToDoPtr                     destWhatToDo;
    FileDescConstPtr                  encodedFileDesc;
    ModelParam::EncodingMethodType    encodingMethod;
    FileDescConstPtr                  fileDesc;
    kkint32                           numOfFeatures;
    kkuint16*                         srcFeatureNums;
    const ModelParam&                 param;

    struct  FeatureVar2;
    typedef  FeatureVar2* FeatureVar2Ptr;
    class  FeatureVar2List;
    typedef  FeatureVar2List*  FeatureVar2ListPtr;
  };  /* FeatureEncoder2 */

#define  _FeatureEncoder2_Defined_

  typedef  FeatureEncoder2::FeatureEncoder2Ptr  FeatureEncoder2Ptr;

} /* namespace KKMLL */

#endif
