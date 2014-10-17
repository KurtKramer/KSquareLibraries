#ifndef _SVM2_
#define _SVM2_

#define LIBSVM_VERSION 289

#include  "FeatureNumList.h"
#include  "FeatureVector.h"
#include  "KKStr.h"

using namespace KKMachineLearning;


/**
 *@namespace  SVM289_MFS
 *@brief   This is a vesion 2.89 of "Chih-Chung Chang" and "Chih-Jen Lin"  libSVM (Support Vector Machine) that selects different features per pair of classes.
 *@details 
 *@code
 *   Downloaded from "http://www.csie.ntu.edu.tw/~cjlin/libsvm/"
 *@endcode
 *@details For each pair of classes a 'FeatureNumList" instance specifies which features are to be used during kernal computations.
 */

namespace  SVM289_MFS
{
  //#ifdef __cplusplus
  //extern "C" {
  //#endif

  extern int32 libsvm_version;


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

    int32               numTrainExamples;
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
    int32        degree;         /* for poly              */
    double       gamma;          /* for poly/rbf/sigmoid  */
    double       coef0;          /* for poly/sigmoid      */

    /* these are for training only */
    double       cache_size;     /* in MB                             */
    double       eps;            /* stopping criteria                 */
    double       C;              /* for C_SVC, EPSILON_SVR and NU_SVR */
    int32        nr_weight;      /* for C_SVC                         */
    int32*       weight_label;   /* for C_SVC                         */
    double*      weight;         /* for C_SVC                         */
    double       nu;             /* for NU_SVC, ONE_CLASS, and NU_SVR */
    double       p;              /* for EPSILON_SVR                   */
    int32        shrinking;      /* use the shrinking heuristics      */
    int32        probability;    /* do probability estimates          */

    double       probParam;      /*  probability parameter as done using USF multi class prob calc */

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

    int32  MemoryConsumedEstimated ()  const;

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
    int32              nr_class;   // number of classes, = 2 in regression/one class svm
    int32              numSVs;     /**< total #SV  */
    FeatureVectorList  SV;         // SVs (SV[l])
    double**           sv_coef;    // coefficients for SVs in decision functions (sv_coef[k-1][l])
    double*            rho;        // constants in decision functions (rho[k*(k-1)/2])
    double*            probA;      // pariwise probability information
    double*            probB;
    FeatureNumList     selFeatures;

    // for classification only

    int32*  label;   // label of each class (label[k])
    int32*  nSV;     // number of SVs for each class (nSV[k])
    // nSV[0] + nSV[1] + ... + nSV[k-1] = l
    // XXX
    bool  weOwnSupportVectors;    // 1 if svm_model is created by svm_load_model
    // 0 if svm_model is created by svm_train


    // Support Prediction Calcs
    double*    dec_values;
    double**   pairwise_prob;
    double*    prob_estimates;

  };


  svm_model*  svm_train  (const svm_problem&     prob, 
                          const svm_parameter&   param,
                          RunLog&                log
                         );

  int32  svm_save_model (const char*              model_file_name, 
                         const struct svm_model*  model
                        );

  void  svm_save_model_XML (ostream&          o, 
                            const svm_model&  model
                           );

  svm_model*  svm_load_model (const char *model_file_name);

  svm_model*  svm_load_model_XML (istream&     i,
                                  FileDescPtr  fileDesc,
                                  RunLog&      log
                                 );

  int32  svm_get_svm_type (const struct svm_model *model);

  int32  svm_get_nr_class (const struct svm_model *model);

  void  svm_get_labels  (const struct svm_model*  model, 
                         int32*                   label
                        );

  double  svm_get_svr_probability (const struct svm_model *model);


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
                                  int32*                votes
                                 );

  void svm_destroy_model (struct svm_model*&  model);


  void svm_destroy_param (struct svm_parameter*&  param);


  const char *svm_check_parameter (const struct svm_problem*    prob, 
                                   const struct svm_parameter*  param
                                  );


  int32 svm_check_probability_model(const struct svm_model *model);

  extern void (*svm_print_string) (const char *);

  //#ifdef __cplusplus
  //}
  //#endif



  template <class T> inline void swap(T& x, T& y) { T t=x; x=y; y=t; }



  typedef float Qfloat;

  typedef signed char schar;

 
  template <class S, class T> inline void clone(T*& dst, S* src, int32 n)
  {
    dst = new T[n];
    memcpy((void *)dst,(void *)src,sizeof(T)*n);
  }

  inline double powi (double base, int32 times);

  class  QMatrix;
  class  Cache;
  class  Kernel;
  class  Solver;
  class  Solver_NU;
  class  SVC_Q;
  class  ONE_CLASS_Q;
  class  SVR_Q;
  struct decision_function;

}  /* Svm289 */


#endif /* _LIBSVM_H */
