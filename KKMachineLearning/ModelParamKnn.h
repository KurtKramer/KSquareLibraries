#ifndef  _MODELPARAMKNN_
#define  _MODELPARAMKNN_

#include  "ModelParam.h"

namespace KKMLL
{
  class ModelParamKnn: public  ModelParam
  {
  public:
    typedef  ModelParamKnn*  ModelParamKnnPtr;

    ModelParamKnn  ();


    ModelParamKnn  (const ModelParamKnn&  _param);


    virtual
    ~ModelParamKnn  ();


    virtual
    ModelParamKnnPtr  Duplicate ()  const;

    virtual ModelParamTypes  ModelParamType () const {return ModelParamTypes::KNN;}

    /*! 
     @brief Creates a Command Line String that represents these parameters.
     */
    virtual
    KKStr   ToCmdLineStr (RunLog&  log)  const;


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;


  private:
    virtual
    void  ParseCmdLineParameter (const KKStr&  parameter,
                                 const KKStr&  value,
                                 bool&         parameterUsed,
                                 RunLog&       log
                                );



    FileDescPtr              fileDesc;

    KKStr                    fileName;

    kkint32                  k;                 /**< The number of nearest neighbors to process. */

    bool                     validParam;
  };  /* ModelParamKnn */

  typedef  ModelParamKnn::ModelParamKnnPtr  ModelParamKnnPtr;

  typedef  XmlElementModelParamTemplate<ModelParamKnn>  XmlElementModelParamKnn;
  typedef  XmlElementModelParamKnn*  XmlElementModelParamKnnPtr;
}




#endif
