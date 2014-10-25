#ifndef _LIBSVM_H
#define _LIBSVM_H
#include <iostream>

//#pragma warning (disable:4786)

#include <set>
#include <vector>

#include  "KKException.h"
#include  "KKStr.h"
#include  "RunLog.h"

/**
 *@namespace  SVM233   
 *@brief Namespce used to wrap implementation of libSVM version 2.33.
 *@details  There is more than obe version of libSVM implemented in the library.  To prevent
 * name coflicts between them each one was wrapped in their own namespace.
 *<br/>
 * libSVM is a Support Vector Machine implementaton done by "Chih-Chung Chang"  and  "Chih-Jen Lin". It 
 * was downloaded from http://www.csie.ntu.edu.tw/~cjlin/libsvm/.  The source code was modified by 
 * Tong Luo  and  Kurt Kramer.  
 */
namespace  SVM233
{
  const kkint32 SCALE = 100;


  struct svm_node
  {
    svm_node ()
    {
      index = -1;
      value = 0.0;
    }

    short index;
    double value;
  };


  struct svm_problem
  {
    svm_problem ();
    ~svm_problem ();

    kkint32            l;
    double*            y;
    VectorKKStr        exampleNames;    /*!< allows the user to provide names to the labels  */
    kkint32*           index; //luo add
    struct svm_node**  x;
    double*            W;

    bool               weOwnContents;

    //luo: the list of data in this bin
    //struct svm_node **elements; 
  };


  enum  { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };  /* svm_type */
  enum  { LINEAR, POLY, RBF, SIGMOID };  /* kernel_type */

  struct svm_parameter
  {
  public:
    svm_parameter ();
    svm_parameter (const svm_parameter&  parameters);
    svm_parameter (KKStr&  paramStr);
    svm_parameter&  operator= (const svm_parameter& right);

    KKStr   ToCmdLineStr ()  const;
    KKStr   ToTabDelStr  ()  const;
    void    ParseTabDelStr (const KKStr&  _str);

    double   coef0;        /* for poly/sigmoid */
    double   degree;       /* for poly */
    double   gamma;        /* for poly/rbf/sigmoid */
    kkint32  kernel_type;
    kkint32  svm_type;

    /* these are for training only */
    double   C;            /* for C_SVC, EPSILON_SVR and NU_SVR */
    double   cache_size;   /* in MB */
    double   eps;          /* stopping criteria */
    kkint32  nr_weight;    /* for C_SVC */
    double   nu;           /* for NU_SVC, ONE_CLASS, and NU_SVR */
    double   p;            /* for EPSILON_SVR */
    kkint32  probability;  /* do probability estimates */
    kkint32  shrinking;    /* use the shrinking heuristics */
    double*  weight;       /* for C_SVC */
    kkint32* weight_label; /* for C_SVC */


    //luo add
    float    A;
    kkint32  boosting;
    float    cBoost;
    double   confidence;
    kkint32  dim;
    kkint32  dimSelect;
    double*  featureWeight;
    kkint32  hist;
    kkint32  nr_class;
    kkint32  numSVM;
    float    sample;      //random sample ratio
    kkint32  sampleSV;
    float    threshold;   //threshold for merging two training images


    // luo

    void  Gamma      (double  _gamma)      {gamma       = _gamma;}
    void  KernalType (kkint32 _kernalType) {kernel_type = _kernalType;}

    double  Gamma      ()  const {return  gamma;}
    kkint32 KernalType ()  const {return  kernel_type;}

    kkint32 MemoryConsumedEstimated ()  const;

    void  ProcessSvmParameter (KKStr   cmd,
                               KKStr   value,
                               double  valueNum,
                               bool&   parmUsed
                              );
  };  /* svm_parameter */




//
// svm_model
//
struct svm_model
{
  svm_parameter param;        // parameter
  kkint32       nr_class;     // number of classes, = 2 in regression/one class svm
  kkint32       l;            // total #SV
  VectorKKStr   exampleNames; /*!< allows the user to provide names to the labels  */
  svm_node**    SV;           /*!< SVs (SV[l])    Support vector feature numbers   */
  double**      sv_coef;      // coefficients for SVs in decision functions (sv_coef[n-1][l])
  double*       rho;          // constants in decision functions (rho[n*(n-1)/2])

  // for classification only

  kkint32*  label;  // label of each class (label[n]) 
  kkint32*  nSV;    /**< number of SVs for each class (nSV[n])
                     * nSV[0] + nSV[1] + ... + nSV[n-1] = l
                     */
                     
  //luo add
  kkint32*   SVIndex;
  kkint32*   nonSVIndex;
  kkint32    numNonSV;

  std::set<kkint32>  BSVIndex;
  double*            margin;
  double             weight;

  kkint32            dim;
  double*            featureWeight;
  //luo

  double*            kValueTable;

  svm_node*          xSpace;    // Needed when we load from data file.
  bool               weOwnXspace;

  svm_model ()
  {
    margin        = NULL;
    featureWeight = NULL;
    SVIndex       = NULL;
    nonSVIndex    = NULL;
    label         = NULL;
    nSV           = NULL;
    SV            = NULL;
    sv_coef       = NULL;
    rho           = NULL;
    kValueTable   = NULL;
    nr_class      = -1;
    l             = -1;
    numNonSV      = -1;
    weight        = -1;
    dim           = -1;
    weOwnXspace = false;
  }


  svm_model (const svm_model& _model)
  {
    throw KKException ("svm_model::svm_model (const   svm_model& _model)  ***ERROR***  Not Supported");
  }

  kkint32  MemoryConsumedEstimated ()  const
  {
    kkint32  memoryConsumedEstimated = sizeof (svm_model)
      +  param.MemoryConsumedEstimated ()
      +  exampleNames.size () * 40;

    if  (SV)             memoryConsumedEstimated  += sizeof (svm_node*) * l;
    if  (sv_coef)        memoryConsumedEstimated  += (nr_class - 1) * sizeof (double*) + l * (nr_class - 1) * sizeof (double);
    if  (rho)            memoryConsumedEstimated  += l * sizeof (double);
    if  (label)          memoryConsumedEstimated  += nr_class * sizeof (kkint32);
    if  (nSV)            memoryConsumedEstimated  += nr_class * sizeof (kkint32);
    if  (featureWeight)  memoryConsumedEstimated  += dim * sizeof (double);
    if  (kValueTable)    memoryConsumedEstimated  += sizeof (double) * l;
    if  ((xSpace != NULL) &&  weOwnXspace)  
      memoryConsumedEstimated  += sizeof (svm_node) * l;

    return  memoryConsumedEstimated;
  }


  void  Dispose ()
  {
    if  (weOwnXspace)
    {
      delete  xSpace;  
      xSpace = NULL;
    }

    if  (sv_coef)
    {
      for  (kkint32 i = 0;  i < (nr_class - 1);  i++)
      {
        free (sv_coef[i]);
        sv_coef[i] = NULL;
      }
    }

    free (SV);       SV      = NULL;
    free (sv_coef);  sv_coef = NULL;
    free (rho);      rho     = NULL;
    free (label);    label   = NULL;
    free (nSV);      nSV     = NULL;

    //luo add
    if  (dim > 0 )
    {
      delete[] (featureWeight);
      featureWeight = NULL;
    }

    free (SVIndex);     SVIndex    = NULL;
    free (nonSVIndex);  nonSVIndex = NULL;
    delete[]  margin;   margin     = NULL;

    if  (kValueTable)
    {
      delete[]  kValueTable;  kValueTable = NULL;
    }
  }

  KKStr  SupportVectorName (kkint32 svIDX)
  {
    if  (svIDX < (kkint32)exampleNames.size ())
      return  exampleNames[svIDX];
    else
      return  "SV" + StrFormatInt (svIDX, "ZZZ#");
  }
};




struct svm_model*  svm_train  (const struct svm_problem*   prob, 
                               const struct svm_parameter* param
                              );


struct svm_model*  svm_load_model (const char*  model_file_name);


struct svm_model*  Svm_Load_Model (istream&  f,
                                   RunLog&   log
                                  );


kkint32 svm_save_model (const char*              model_file_name, 
                        const struct svm_model*  model
                       );

void    Svm_Save_Model (std::ostream&      o, 
                        const svm_model*   model
                       );


kkint32 svm_get_nr_class  (const struct svm_model*  model);


void    svm_get_labels    (const struct svm_model*  model, 
                           kkint32*                 label
                          );

void    svm_GetSupportVectorStatistics (const struct svm_model*  model,
                                        kkint32&                 numSVs,         // The number of training examp
                                        kkint32&                 totalNumSVs
                                       );


double  svm_predict  (const struct svm_model*  model, 
                      const struct svm_node*   x
                     );


double  svm_predict  (const struct svm_model*  model, 
                      const svm_node*          x, 
                      std::vector<double>&     dist,
                      std::vector<kkint32>&    winners,
                      kkint32                  excludeSupportVectorIDX  /**< Specify index of a S/V to remove from computation. */
                     );


double  svm_predictTwoClasses (const svm_model*  model,
                               const svm_node*   x,
                               double&           dist,
                               kkint32           excludeSupportVectorIDX  /*!<  Specify index of a S/V to remove from computation. */
                              );

svm_problem*  svm_BuildProbFromTwoClassModel (const svm_model*  model,
                                              kkint32           excludeSupportVectorIDX  /*!<  Specify index of a S/V to remove from computation. */
                                             );


void          svm_destroy_model   (struct svm_model *model);

const char*   svm_check_parameter (const struct svm_problem *prob, const struct svm_parameter *param);


//luo add
void     svm_margin (svm_model *model); // compute the margin
//double   svm_dist   (const svm_model* model, 
//                     const svm_node  *x
//                    ); // compute the normalized distance
//luo


}  /* SVM233 */


#endif /* _LIBSVM_H */
