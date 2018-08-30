#ifndef _SVM2_
#define _SVM2_

#include <string.h>
#include <string>


#define LIBSVM_VERSION 289

#include "KKStr.h"

#include "KKMLLTypes.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"

using namespace KKMLL;


/**
 *@namespace  SVM289_MFS   
 *@brief Namespce used to wrap implementation of libSVM version 2.89.
 *@details  There is more than one version of libSVM implemented in the library.  To prevent
 * name conflicts between them each one was wrapped in their own namespace.
 *<br/>
 * libSVM is a Support Vector Machine implementation done by "Chih-Chung Chang"  and  "Chih-Jen Lin". It 
 * was downloaded from http://www.csie.ntu.edu.tw/~cjlin/libsvm/.  The source code was modified by 
 * Kurt Kramer.    The primary changes to this implementation involves the replacement of the sparse data-structure 
 * in the original implementation with fixed length array implemented through the "FeatureVector" class and 
 * the ability to specify a sub-set of features to be utilized via the "FeatureNumList" class.  This allows 
 * us to load in a single set of training data with all its features that can then be used for multiple Support 
 * Vector Machine instances where each instance utilizes a different set of features.  The use of this 
 * version of libSVM(SVM289_MFS) is via the "ModelSvmBase" class. 
 */
namespace  SVM289_MFS
{
  //#ifdef __cplusplus
  //extern "C" {
  //#endif

  extern kkint32 libsvm_version;


  struct svm_problem 
  {
    svm_problem (const svm_problem&  _prob);

    svm_problem (const FeatureVectorList&  _x,
                 const float*              _y,
                 const FeatureNumList&     _selFeatures
                );

    svm_problem (const FeatureNumList&  _selFeatures,
                 FileDescConstPtr       _fileDesc,
                 RunLog&                _log
                );

    ~svm_problem ();

    FileDescConstPtr  FileDesc ()  const;

    const FeatureNumList&   SelFeatures ()  const  {return selFeatures;}

    kkint32           numTrainExamples;
    FeatureNumList    selFeatures;
    FeatureVectorList x;
    double*           y;
  };  /* svm_problem */

  

  enum class  SVM_Type     
  {
    SVM_NULL,
    C_SVC,
    NU_SVC,
    ONE_CLASS,
    EPSILON_SVR,
    NU_SVR
  };


  enum class  Kernel_Type
  {
    Kernel_NULL,
    LINEAR,
    POLY,
    RBF,
    SIGMOID,
    PRECOMPUTED
  };


  SVM_Type  SVM_Type_FromStr (KKStr     s);
  KKStr     SVM_Type_ToStr   (SVM_Type  svmType);

  Kernel_Type  Kernel_Type_FromStr (KKStr        s);
  KKStr        Kernel_Type_ToStr   (Kernel_Type  kernelType);


  struct  svm_parameter
  {
    svm_parameter ();
    svm_parameter (const svm_parameter&  _param);
    svm_parameter (KKStr&  paramStr);

    ~svm_parameter ();

    void  Cost        (double      _cost)       {C           = _cost;}
    void  Gamma       (double      _gamma)      {gamma       = _gamma;}
    void  KernalType  (Kernel_Type _kernalType) {kernel_type = _kernalType;}
    void  SvmType     (SVM_Type    _svm_type)   {svm_type    = _svm_type;}

    double       Cost       ()  const {return  C;}
    double       Gamma      ()  const {return  gamma;}
    Kernel_Type  KernalType ()  const {return  kernel_type;}
    SVM_Type     SvmType    ()  const {return  svm_type;}


    svm_parameter&  operator= (const svm_parameter& right);

    KKStr   ToCmdLineStr ()  const;
    KKStr   ToTabDelStr  ()  const;
    void    ParseTabDelStr (const KKStr&  _str);

    void    ProcessSvmParameter (const KKStr&  cmd,
                                 const KKStr&  value,
                                 bool&         parmUsed
                                );

    SVM_Type     svm_type;
    Kernel_Type  kernel_type;
    kkint32      degree;         /* for poly              */
    double       gamma;          /* for poly/rbf/sigmoid  */
    double       coef0;          /* for poly/sigmoid      */

    /* these are for training only */
    double       cache_size;     /* in MB                             */
    double       eps;            /* stopping criteria                 */
    double       C;              /* for C_SVC, EPSILON_SVR and NU_SVR */
    kkint32      nr_weight;      /* for C_SVC                         */
    kkint32*     weight_label;   /* for C_SVC                         */
    double*      weight;         /* for C_SVC                         */
    double       nu;             /* for NU_SVC, ONE_CLASS, and NU_SVR */
    double       p;              /* for EPSILON_SVR                   */
    kkint32      shrinking;      /* use the shrinking heuristics      */
    kkint32      probability;    /* do probability estimates          */

    double       probParam;      /*  probability parameter as done using USF multi class prob calc */

    static  const char*  svm_type_table[];

    static  const char*  kernel_type_table[];

  };  /* svm_parameter */


  
  struct  Svm_Model
  {
    Svm_Model ();

    Svm_Model (const Svm_Model&  _model,
               FileDescConstPtr  _fileDesc
              );

    Svm_Model (FileDescConstPtr  _fileDesc);

    Svm_Model (const svm_parameter&   _param,
               const FeatureNumList&  _selFeatures,
               FileDescConstPtr       _fileDesc
              );

    virtual ~Svm_Model ();

    void  CleanUpMemory ();

    double*  DecValues     ();
    double*  ProbEstimates ();
    double** PairwiseProb  ();

    kkMemSize  MemoryConsumedEstimated ()  const;

    void  NormalizeProbability ();


    virtual  void  ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelFlag,
                            RunLog&         log
                           );


    virtual  void  WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const;

    FileDescConstPtr   fileDesc;
    svm_parameter      param;      // parameter
    kkuint32           nr_class;   // number of classes, = 2 in regression/one class svm
    kkint32            numSVs;     /**< total #SV  */
    FeatureVectorList  SV;         // SVs (SV[l])
    double**           sv_coef;    // coefficients for SVs in decision functions (sv_coef[k-1][l])
    double*            rho;        // constants in decision functions (rho[k*(k-1)/2])
    double*            probA;      // pair-wise probability information
    double*            probB;
    FeatureNumList     selFeatures;

    // for classification only

    kkint32*  label;   // label of each class (label[k])
    kkint32*  nSV;     // number of SVs for each class (nSV[k])
    // nSV[0] + nSV[1] + ... + nSV[k-1] = l
    // XXX
    bool  weOwnSupportVectors;    // 1 if Svm_Model is created by svm_load_model
    // 0 if Svm_Model is created by svm_train


    // Support Prediction Calculations
    double*    dec_values;
    double**   pairwise_prob;
    double*    prob_estimates;
  };

  typedef  XmlElementTemplate<Svm_Model>  XmlElementSvm_Model;
  typedef  XmlElementSvm_Model*  XmlElementSvm_ModelPtr;


  Svm_Model*  svm_train  (const svm_problem&    prob,
                          const svm_parameter&  param,
                          RunLog&               log
                         );

  kkint32  svm_get_svm_type (const struct Svm_Model *model);

  kkint32  svm_get_nr_class (const struct Svm_Model *model);

  void  svm_get_labels  (const struct Svm_Model*  model, 
                         kkint32*                 label
                        );

  double  svm_get_svr_probability (const struct Svm_Model *model);


  void  svm_predict_values  (const Svm_Model*      model, 
                             const FeatureVector&  x,
                             double*               dec_values
                            );


  double  svm_predict  (const struct Svm_Model*  model, 
                        const FeatureVector&     x
                       );


  double svm_predict_probability (      Svm_Model*      model, 
                                  const FeatureVector&  x, 
                                  double*               prob_estimates,
                                  kkint32*              votes
                                 );

  void svm_destroy_model (struct Svm_Model*&  model);


  void svm_destroy_param (struct svm_parameter*&  param);


  const char *svm_check_parameter (const struct svm_problem*    prob, 
                                   const struct svm_parameter*  param
                                  );


  kkint32 svm_check_probability_model(const struct Svm_Model *model);

  extern void (*svm_print_string) (const char *);


  template <class T> inline void swap(T& x, T& y) { T t=x; x=y; y=t; }



  typedef float Qfloat;

  typedef signed char schar;

 
  template <class S, class T> inline void clone(T*& dst, S* src, kkint32 n)
  {
    dst = new T[n];

    kkint32  sizeOfT = sizeof(T);
    KKStr::MemCpy ((void *)dst, (void *)src, sizeOfT * n);
  }

  inline double powi (double base, kkint32 times);

  class  QMatrix;
  class  Cache;
  class  Kernel;
  class  Solver;
  class  Solver_NU;
  class  SVC_Q;
  class  ONE_CLASS_Q;
  class  SVR_Q;
  struct decision_function;


  typedef  XmlElementTemplate<Svm_Model>  XmlElementSvm_Model;
  typedef  XmlElementSvm_Model*  XmlElementSvm_ModelPtr;

}  /* SVM289_MFS */


#endif /* _LIBSVM_H */
