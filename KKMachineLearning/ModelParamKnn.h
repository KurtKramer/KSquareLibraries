#ifndef  _MODELPARAMKNN_
#define  _MODELPARAMKNN_

#include  "ModelParam.h"

namespace KKMachineLearning
{
  class ModelParamKnn: public  ModelParam
  {
  public:
    typedef  ModelParamKnn*  ModelParamKnnPtr;

    ModelParamKnn  (FileDescPtr  _fileDesc,
                    RunLog&      _log
                   );


    ModelParamKnn  (const ModelParamKnn&  _param);


    virtual
    ~ModelParamKnn  ();


    virtual
    ModelParamKnnPtr  Duplicate ()  const;

    virtual ModelParamTypes  ModelParamType () const {return mptKNN;}

    virtual
    void    ReadSpecificImplementationXML (istream&  i);

    /*! 
     @brief Creates a Command Line String that represents these parameters.
     */
    virtual
    KKStr   ToCmdLineStr ()  const;

    virtual
    void    WriteSpecificImplementationXML (std::ostream&  o)  const;



  private:
    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed
                                );



    FileDescPtr              fileDesc;

    KKStr                    fileName;

    kkint32                  k;                 /**< The number of nearest neighbors to process. */

    bool                     validParam;
  };  /* ModelParamKnn */

  typedef  ModelParamKnn::ModelParamKnnPtr  ModelParamKnnPtr;
}


#endif
