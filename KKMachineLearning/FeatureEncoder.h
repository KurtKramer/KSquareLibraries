#ifndef  _FEATUREENCODER_
#define  _FEATUREENCODER_
//***********************************************************************
//*                           FeatureEncoder                            *
//*                                                                     *
//*                                                                     *
//*---------------------------------------------------------------------*
//*  History                                                            *
//*                                                                     *
//*    Date      Programmer   Description                               *
//*                                                                     *
//*  2005-09-10  Kurt Kramer  Removing Feature compression and encoding *
//*                           code from FeatureEncoder                  *
//*                                                                     *
//***********************************************************************
#include "RunLog.h"

#include "Attribute.h"
#include "FeatureVector.h"
#include "FileDesc.h"
#include "MLClass.h"
#include "svm.h"
#include "SVMparam.h"



namespace KKMLL
{

  #ifndef  _FEATURENUMLIST_
  class  FeatureNumList;
  typedef  FeatureNumList*  FeatureNumListPtr;
  #endif


  typedef  struct svm_node*     XSpacePtr;


  typedef  enum  {FeAsIs, FeBinary, FeScale}  FeWhatToDo;
  typedef  FeWhatToDo*  FeWhatToDoPtr;


  class  FeatureEncoder
  {
  public:
    FeatureEncoder (const SVMparam&       _svmParam,
                    FileDescPtr           _fileDesc,
                    AttributeTypeVector&  _attributeTypes,   
                    VectorInt32&          _cardinalityTable,
                    MLClassPtr            _class1,
                    MLClassPtr            _class2,
                    RunLog&               _log
                   );
    
    
    ~FeatureEncoder ();


    kkint32  CodedNumOfFeatures () const  {return codedNumOfFeatures;}


    MLClassPtr  Class1 () const  {return class1;}
    MLClassPtr  Class2 () const  {return class2;}

    /**
     *@brief  Compresses 'src' images, allocating new 'xSpace' data structure.
     *@param[in]  src              Images that are to be compressed
     *@param[in]  assignments      Class Assignments
     *@param[in]  xSpace           will allocate enough xSpace nodes and place compressed results in this structure.
     *@param[out] totalxSpaceUsed  number nodes used in xSpace</param>
     *@param[out] prob             Data Structure that is used by SVMLib
     */
    void  EncodeIntoSparseMatrix (FeatureVectorListPtr  src,
                                  ClassAssignments&     assignments,
                                  XSpacePtr&            xSpace,
                                  kkint32&              totalxSpaceUsed,
                                  struct svm_problem&   prob
                                 );

    FileDescPtr       CreateEncodedFileDesc (ostream*  o);

    FeatureVectorListPtr  CreateEncodedFeatureVector (FeatureVectorList&  srcData);

    XSpacePtr         EncodeAExample (FeatureVectorPtr  example);

    void              EncodeAExample (FeatureVectorPtr  example,
                                      svm_node*         xSpace,
                                      kkint32&          xSpaceUsed
                                     );

    FeatureVectorListPtr  EncodeAllExamples (const FeatureVectorListPtr  srcData);


    FeatureVectorPtr  EncodeAExample (FileDescPtr       encodedFileDesc,
                                      FeatureVectorPtr  src
                                     );

    kkint32           MemoryConsumedEstimated ()  const;

    kkint32           XSpaceNeededPerImage ()  {return xSpaceNeededPerImage;}


  private:
    /**  
     * Computes the number of XSpace nodes that need to be allocated to represent the contents of a specified FeatureVectorList.  Will
     * account for features that have a value of 0;  that is features that have a value of '0' do not need a xSpace node allocated for
     * them.
     */
    kkint32  DetermineNumberOfNeededXspaceNodes (FeatureVectorListPtr   src)  const;



    AttributeTypeVector&         attributeTypes;     /**< Will not own, passed in by creator. */
    kkint32*                     cardinalityDest;
    VectorInt32&                 cardinalityTable;   /**< Will not own, passed in by creator. */
    MLClassPtr                   class1;
    MLClassPtr                   class2;
    kkint32                      codedNumOfFeatures;
    SVM_CompressionMethod        compressionMethod;
    kkint32*                     destFeatureNums;
    FileDescPtr                  destFileDesc;
    FeWhatToDoPtr                destWhatToDo;
    SVM_EncodingMethod           encodingMethod;
    FileDescPtr                  fileDesc;
    RunLog&                      log;
    kkint32                      numEncodedFeatures;
    kkint32                      numOfFeatures;
    FeatureNumList               selectedFeatures;
    kkint32*                     srcFeatureNums;
    const SVMparam&              svmParam;
    kkint32                      xSpaceNeededPerImage;
  };  /* FeatureEncoder */


  typedef  FeatureEncoder*  FeatureEncoderPtr;


} /* namespace KKMLL */

#endif
