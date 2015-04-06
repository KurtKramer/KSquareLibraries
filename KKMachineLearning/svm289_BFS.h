#ifndef _SVM289_BFS_
#define _SVM289_BFS_

#define LIBSVM_VERSION 289

#include  "FeatureNumList.h"
#include  "FeatureVector.h"
#include  "KKStr.h"

using namespace KKMachineLearning;


/**
 *@namespace  SVM289_BFS   
 *@brief Namespce used to wrap implementation of libSVM version 2.89  to be used as a pair-wise SVM.
 *@details  There is more than one version of libSVM implemented in the library.  To prevent
 * name conflicts between them each one was wrapped in their own namespace.
 *<br/>
 * libSVM is a Support Vector Machine implementor done by "Chih-Chung Chang"  and  "Chih-Jen Lin". It 
 * was downloaded from http://www.csie.ntu.edu.tw/~cjlin/libsvm/.  The source code was modified by 
 * Kurt Kramer.  The primary changes to this implementation involves the replacement of the sparse data-structure 
 * in the original implementation with fixed length array implemented through the "FeatureVector" class and 
 * the ability to specify a sub-set of features to be utilized via the "FeatureNumList" class.  This allows 
 * us to load in a single set of training data with all its features that can then be used for multiple Support 
 * Vector Machine instances where each instance utilizes a different set of features.  This particular implementation
 * SVM289_BFS was meant to work with the Binary-Feature-Selection (Pair-Wise) version of the support vector machine as 
 * described by "Increased classification accuracy and speedup through pair-wise feature selection for support vector machines."
 */
namespace  SVM289_BFS
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
                 RunLog&                _log
                );

    ~svm_problem ();

    RunLog&      Log ();

    FileDescPtr  FileDesc ()  const;

    const FeatureNumList&   SelFeatures ()  const  {return selFeatures;}

    kkint32             l;
    FeatureNumList      selFeatures;
    FeatureVectorList   x;
    double*             y;
  };  /* svm_problem */



  typedef  enum  { SVM_NULL, C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR }  SVM_Type;    /* svm_type */
  typedef  enum  { Kernel_NULL, LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED }     Kernel_Type; /* kernel_type */


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

    double       probParam;      /*  probability parameter as done using USF multi class prob calculation */

    static  const char*  svm_type_table[];

    static  const char*  kernel_type_table[];

  };  /* svm_parameter */




  struct  svm_model
  {
    svm_model (const svm_model&  _model,
               FileDescPtr       _fileDesc,
               RunLog&           _log
              );

    svm_model (FileDescPtr   _fileDesc,
               RunLog&       _log
              );

    svm_model (const svm_parameter&   _param,
               const FeatureNumList&  _selFeatures,
               FileDescPtr            _fileDesc,
               RunLog&                _log
              );

    svm_model (const KKStr&  _fileName,
               FileDescPtr   _fileDesc,
               RunLog&       _log
              );

    svm_model (istream&     _fileName,
               FileDescPtr  _fileDesc,
               RunLog&      _log
              );

    ~svm_model ();

    double*  DecValues     ();
    double*  ProbEstimates ();
    double** PairwiseProb  ();


    void  Save (const KKStr&  fileName,
                RunLog&       log
               );

    void  Write (ostream& o);

    void  Load (const KKStr&  fileName,
                FileDescPtr   fileDesc,
                RunLog&       log
               );

    void  Read (istream&     i, 
                FileDescPtr  fileDesc,
                RunLog&      log
               );

    void  NormalizeProbability ();


    svm_parameter      param;      // parameter
    kkint32            nr_class;   /**< number of classes, = 2 in regression/one class svm           */
    kkint32            l;          /**< total #SV                                                    */
    FeatureVectorList  SV;         /**< SVs (SV[l])                                                  */
    double**           sv_coef;    /**< Coefficients for SVs in decision functions (sv_coef[k-1][l]) */
    double*            rho;        /**< constants in decision functions (rho[k*(k-1)/2])             */
    double*            probA;      /**< pair-wise probability information                            */
    double*            probB;
    FeatureNumList     selFeatures;

    // for classification only

    kkint32*    label;   /**< label of each class (label[k])         */
    kkint32*    nSV;     /**< number of SVs for each class (nSV[k])  */
    // nSV[0] + nSV[1] + ... + nSV[k-1] = l
    // XXX
    bool  weOwnSupportVectors;    // 1 if svm_model is created by svm_load_model
                                  // 0 if svm_model is created by svm_train


    // Support Prediction Calculations
    double*    dec_values;
    double**   pairwise_prob;
    double*    prob_estimates;
  };


  svm_model*  svm_train  (const svm_problem&    prob, 
                          const svm_parameter&  param,
                          RunLog&               log
                         );

  kkint32  svm_save_model (const char*        model_file_name, 
                           const  svm_model*  model
                          );

  void  svm_save_model_XML (ostream&          o, 
                            const svm_model&  model
                           );

  svm_model*  svm_load_model (const char *model_file_name);

  svm_model*  svm_load_model_XML (istream&     i,
                                  FileDescPtr  fileDesc,
                                  RunLog&      log
                                 );

  kkint32  svm_get_svm_type (const svm_model *model);

  kkint32  svm_get_nr_class (const svm_model *model);

  void  svm_get_labels  (const svm_model*  model, 
                         kkint32*          label
                        );

  double  svm_get_svr_probability (const svm_model *model);


  void  svm_predict_values  (const svm_model*      model, 
                             const FeatureVector&  x,
                             double*               dec_values
                            );


  double  svm_predict  (const struct svm_model*  model, 
                        const FeatureVector&     x
                       );


  double svm_predict_probability (      svm_model*      model, 
                                  const FeatureVector&  x, 
                                  double*               prob_estimates,
                                  kkint32*                votes
                                 );

  void svm_destroy_model (struct svm_model*&  model);


  void svm_destroy_param (struct svm_parameter*&  param);


  const char *svm_check_parameter (const struct svm_problem*    prob, 
                                   const struct svm_parameter*  param
                                  );


  kkint32 svm_check_probability_model(const struct svm_model *model);

  extern void (*svm_print_string) (const char *);

  //#ifdef __cplusplus
  //}
  //#endif



  template <class T> inline void swap(T& x, T& y) { T t=x; x=y; y=t; }



  typedef float Qfloat;

  typedef signed char schar;

 
  template <class S, class T> inline void clone(T*& dst, S* src, kkint32 n)
  {
    dst = new T[n];
    memcpy((void *)dst,(void *)src,sizeof(T)*n);
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

}  /* SVM289_BFS */


#endif /* _LIBSVM_H */
