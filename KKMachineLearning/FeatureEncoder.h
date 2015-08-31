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
#include "XmlStream.h"

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


  enum class  FeWhatToDo  {FeAsIs, FeBinary, FeScale};

  typedef  FeWhatToDo*  FeWhatToDoPtr;


  class  FeatureEncoder
  {
  public:
    FeatureEncoder ();

    FeatureEncoder (FileDescPtr            _fileDesc,
                    MLClassPtr             _class1,
                    MLClassPtr             _class2,
                    const FeatureNumList&  _selectedFeatures,
                    SVM_EncodingMethod     _encodingMethod,
                    double                 _c_Param
                   );
    
    
    ~FeatureEncoder ();


    kkint32  CodedNumOfFeatures () const  {return codedNumOfFeatures;}


    MLClassPtr  Class1 () const  {return class1;}
    MLClassPtr  Class2 () const  {return class2;}

    /**
     * @brief  Left over from BitReduction days; removed all code except that which processed the NO bit reduction option.
     */
    void  CompressExamples (FeatureVectorListPtr  srcExamples,
                            FeatureVectorListPtr  compressedExamples,
                            ClassAssignments&     assignments
                           );


    FileDescPtr  CreateEncodedFileDesc (ostream*  o);

    FeatureVectorListPtr  CreateEncodedFeatureVector (FeatureVectorList&  srcData);


    /**
     *@brief  Compresses 'src' examples, allocating new 'xSpace' data structure.
     *@param[in]  src              Examples that are to be compressed
     *@param[in]  assignments      Class Assignments
     *@param[in]  xSpace           will allocate enough xSpace nodes and place compressed results in this structure.
     *@param[out] totalxSpaceUsed  number nodes used in xSpace</param>
     *@param[out] prob             Data Structure that is used by SVMLib
     *@param[in]  log
     */
    void  EncodeIntoSparseMatrix (FeatureVectorListPtr  src,
                                  ClassAssignments&     assignments,
                                  XSpacePtr&            xSpace,
                                  kkint32&              totalxSpaceUsed,
                                  struct svm_problem&   prob,
                                  RunLog&               log
                                 );

    XSpacePtr  EncodeAExample (FeatureVectorPtr  example);


    void   EncodeAExample (FeatureVectorPtr  example,
                           svm_node*         xSpace,
                           kkint32&          xSpaceUsed
                          );


    FeatureVectorListPtr  EncodeAllExamples (const FeatureVectorListPtr  srcData);


    FeatureVectorPtr  EncodeAExample (FileDescPtr       encodedFileDesc,
                                      FeatureVectorPtr  src
                                     );

    kkint32  MemoryConsumedEstimated ()  const;

    virtual
    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   RunLog&         log
                  );

    virtual  
    void  WriteXML (const KKStr&  varName,
                    ostream&      o
                   )  const;

    kkint32  XSpaceNeededPerExample ()  {return xSpaceNeededPerExample;}


  private:
    /**  
     * Computes the number of XSpace nodes that need to be allocated to represent the contents of a specified FeatureVectorList.  Will
     * account for features that have a value of 0;  that is features that have a value of '0' do not need a xSpace node allocated for
     * them.
     */
    kkint32  DetermineNumberOfNeededXspaceNodes (FeatureVectorListPtr   src)  const;



    kkint32*              cardinalityDest;
    MLClassPtr            class1;
    MLClassPtr            class2;
    kkint32               codedNumOfFeatures;
    double                c_Param;
    kkint32*              destFeatureNums;
    FileDescPtr           destFileDesc;
    FeWhatToDoPtr         destWhatToDo;
    SVM_EncodingMethod    encodingMethod;
    FileDescPtr           fileDesc;
    kkint32               numEncodedFeatures;
    kkint32               numOfFeatures;
    FeatureNumList        selectedFeatures;
    kkint32*              srcFeatureNums;
    kkint32               xSpaceNeededPerExample;
  };  /* FeatureEncoder */


  typedef  FeatureEncoder*  FeatureEncoderPtr;


} /* KKMLL */

#endif
