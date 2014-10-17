#include  "FirstIncludes.h"

#include <math.h>
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <string>
#include <stdarg.h>
#include <vector>
#include <assert.h>
#include <iostream>


#include  "MemoryDebug.h"
using namespace std;


#include  "KKBaseTypes.h"
#include  "KKException.h"
#include  "OSservices.h"
using namespace KKB;



#pragma warning(disable : 4996)




#include "svm.h"


// Forward defining several functions.
namespace  SVM233
{
  // Generalized SMO+SVMlight algorithm
  // Solves:
  //
  //  min 0.5(\alpha^T Q \alpha) + b^T \alpha
  //
  //    y^T \alpha = \delta
  //    y_i = +1 or -1
  //    0 <= alpha_i <= Cp for y_i = 1
  //    0 <= alpha_i <= Cn for y_i = -1
  //
  // Given:
  //
  //  Q, b, y, Cp, Cn, and an initial feasible point \alpha
  //  l is the size of vectors and matrices
  //  eps is the stopping criterion
  //
  // solution will be put in \alpha, objective value will be put in obj
  //
  
  typedef float Qfloat;
  typedef signed char schar;

  class  Cache;
  class  Kernel;
  class  Solver;
  class  Solver_NU;
  class  SVC_Q;
  class  ONE_CLASS_Q;
  class  SVR_Q;

  class  Solver 
  {
  public:
    Solver() {};
    virtual ~Solver() {};
  
    struct SolutionInfo {
      double obj;
      double rho;
      double upper_bound_p;
      double upper_bound_n;
      double r;	// for Solver_NU
    };
  
    void Solve(int32          l, 
               const Kernel&  Q, 
               const double*  b_, 
               const schar*   y_,
               double*        alpha_, 
               double         Cp, 
               double         Cn, 
               double         eps,
               SolutionInfo*  si, 
               int32          shrinking
              );
  
      void Solve (int32          l, 
                  const Kernel&  Q, 
                  const double*  b_, 
                  const schar*   y_,
                  double*        alpha_, 
                  double*        C_, 
                  double         eps,
                  SolutionInfo*  si, 
                  int32          shrinking
                 );

  protected:
    int32          active_size;
    schar*         y;
    double*        G;              // gradient of objective function
    enum  {LOWER_BOUND, UPPER_BOUND, FREE};
    char*          alpha_status;   // LOWER_BOUND, UPPER_BOUND, FREE
    double*        alpha;
    const Kernel*  Q;
    double         eps;
    double*        C;
    double*        b;
    int32*         active_set;
    double*        G_bar;          // gradient, if we treat free variables as 0
    int32          l;
    bool           unshrinked;     // XXX
  
    double get_C(int32 i)
    {
      return C[i];
    }
    void update_alpha_status(int32 i)
    {
      if  (alpha[i] >= get_C (i))
        alpha_status[i] = UPPER_BOUND;
  
      else if(alpha[i] <= 0)
        alpha_status[i] = LOWER_BOUND;
  
      else 
        alpha_status[i] = FREE;
    }
  
    bool is_upper_bound(int32 i) { return alpha_status[i] == UPPER_BOUND; }
    bool is_lower_bound(int32 i) { return alpha_status[i] == LOWER_BOUND; }
    bool is_free(int32 i) { return alpha_status[i] == FREE; }
    void swap_index(int32 i, int32 j);
    void reconstruct_gradient();
    virtual int32 select_working_set(int32 &i, int32 &j);
    virtual double calculate_rho();
    virtual void do_shrinking();
  };


  static void  solve_c_svc (const svm_problem*     prob, 
                            const svm_parameter*   param,
                            double*                alpha, 
                            Solver::SolutionInfo*  si, 
                            double                 Cp, 
                            double                 Cn
                           );

  static void  solve_c_svc (const svm_problem*     prob, 
                            const svm_parameter*   param,
                            double*                alpha, 
                            Solver::SolutionInfo*  si, 
                            double*                C_
                           );



  static void  solve_nu_svc (const svm_problem*    prob, 
                             const svm_parameter*  param,
                             double*               alpha, 
                             Solver::SolutionInfo* si
                            );


  static void  solve_one_class (const svm_problem*     prob, 
                                const svm_parameter*   param,
                                double*                alpha, 
                                Solver::SolutionInfo*  si
                               );

  static void  solve_epsilon_svr (const svm_problem*     prob, 
                                  const svm_parameter*   param,
                                  double*                alpha, 
                                  Solver::SolutionInfo*  si
                                 );

  static void solve_nu_svr (const svm_problem*     prob, 
                            const svm_parameter*   param,
                            double*                alpha, 
                            Solver::SolutionInfo*  si
                           );

  struct  decision_function
  {
    double *alpha;
    double rho;
  };


  decision_function  svm_train_one (const svm_problem*    prob, 
                                    const svm_parameter*  param
                                    );

  decision_function  svm_train_one (const svm_problem*    prob, 
                                    const svm_parameter*  param,
                                    double                Cp, 
                                    double                Cn, 
                                    std::set<int32>&      BSVIndex
                                   );


#ifdef  WIN32

#ifndef min
  template <class T> inline T min(T x,T y) { return (x<y)?x:y; }
#endif

#ifndef max
  template <class T> inline T max(T x,T y) { return (x>y)?x:y; }
#endif

#endif



template <class T> inline void  Swap(T& x, T& y) { T t=x; x=y; y=t; }


template <class S, class T> inline void  clone(T*& dst, S* src, int32 n)
{
  dst = new T[n];
  KKStr::MemCpy((void *)dst, (void *)src, sizeof(T)*n);
}



#define INF HUGE_VAL
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

#if 0
  void info(char *fmt,...)
  {
    va_list ap;
    va_start(ap,fmt);
    vprintf(fmt,ap);
    va_end(ap);
  }

  void info_flush()
  {
    fflush(stdout);
  }
#else
  void info(char *fmt,...) {}
  void info_flush() {}
#endif

}  /* SVM233 */


using namespace  SVM233;







svm_parameter::svm_parameter ():
   coef0           (0.0),
   degree          (3),
   gamma           (0.0),
   kernel_type     (RBF),
   svm_type        (C_SVC),

   /* these are for training only */
   C               (1),
   cache_size      (40.0),
   eps             (1e-3),
   nr_weight       (0),
   nu              (0.5),
   p               (0.1),
   probability     (0),
   shrinking       (1),
   weight          (NULL),
   weight_label    (NULL),

   A               (1.0f),
   boosting        (0),
   cBoost          (1.0),
   confidence      (0),
   dim             (0),
   dimSelect       (-1),
   featureWeight   (NULL),
   hist            (0),
   nr_class        (0),
   numSVM          (1),
   sample          (100),
   sampleSV        (0),
   threshold       (0.0f)
{
}




svm_parameter::svm_parameter (const svm_parameter&  _param)
  :
   coef0           (_param.coef0),
   degree          (_param.degree),
   gamma           (_param.gamma),
   kernel_type     (_param.kernel_type),
   svm_type        (_param.svm_type),

   /* these are for training only */
   C               (_param.C),
   cache_size      (_param.cache_size),
   eps             (_param.eps),
   nr_weight       (_param.nr_weight),
   nu              (_param.nu),
   p               (_param.p),
   probability     (_param.probability),
   shrinking       (_param.shrinking),
   weight          (NULL),
   weight_label    (NULL),

   A                (_param.A),
   boosting         (_param.boosting),
   cBoost           ( _param.cBoost),
   confidence       (_param.confidence),
   dim              (_param.dim),
   dimSelect        (_param.dimSelect),
   featureWeight    (NULL),
   hist             (_param.hist),
   nr_class         (_param.nr_class),
   numSVM           (_param.numSVM),
   sample           (_param.sample),
   sampleSV         (_param.sampleSV),
   threshold        (_param.threshold)
  {
  if  (_param.weight_label)
  {
    weight_label = new int32[nr_weight];
    for  (int32 x = 0;  x < nr_weight;  x++)
      weight_label[x] = _param.weight_label[x];
  }

  if  (_param.weight)
  {
    weight = new double[nr_weight];
    for  (int32 x = 0;  x < nr_weight;  x++)
      weight[x] = _param.weight[x];
  }

  if  (_param.featureWeight)
  {
    featureWeight = new double[dim];
    for  (int32 x = 0;  x < dim;  x++)
      featureWeight[x] = _param.featureWeight[x];
  }
}  





svm_parameter&  svm_parameter::operator= (const svm_parameter&  right)
{
  coef0           = right.coef0;
  degree          = right.degree;
  gamma           = right.gamma;
  kernel_type     = right.kernel_type;
  svm_type        = right.svm_type;

  cache_size      = right.cache_size;
  eps             = right.eps;
  C               = right.C;
  nr_weight       = right.nr_weight;
  weight_label    = NULL;
  weight          = NULL;
  nu              = right.nu;
  p               = right.p;
  shrinking       = right.shrinking;
  probability     = right.probability;
  numSVM          = right.numSVM;
  sampleSV        = right.sampleSV;
  hist            = right.hist;
  boosting        = right.boosting;
  cBoost          = right.cBoost;
  dimSelect       = right.dimSelect;
  dim             = right.dim;
  featureWeight   = NULL;
  confidence      = right.confidence;
  A               = right.A;
  nr_class        = right.nr_class;
  threshold       = right.threshold;
  sample          = right.sample;

  if  (right.weight_label)
  {
    weight_label = new int32[nr_weight];
    for  (int32 x = 0;  x < nr_weight;  x++)
      weight_label[x] = right.weight_label[x];
  }

  if  (right.weight)
  {
    weight = new double[nr_weight];
    for  (int32 x = 0;  x < nr_weight;  x++)
      weight[x] = right.weight[x];
  }

  if  (right.featureWeight)
  {
    featureWeight = new double[dim];
    for  (int32 x = 0;  x < dim;  x++)
      featureWeight[x] = right.featureWeight[x];
  }

  return  *this;
}  /* svm_parameter::operator= */



int32   svm_parameter::MemoryConsumedEstimated ()  const
{
  int32  memoryConsumedEstimated = sizeof (svm_parameter);
  if  (weight)
    memoryConsumedEstimated += ((sizeof (double) + sizeof(int32)) * nr_weight);

  if  (featureWeight)
    memoryConsumedEstimated += (sizeof (double) * dim);

  return  memoryConsumedEstimated;
}



void  svm_parameter::ProcessSvmParameter (KKStr   cmd,
                                          KKStr   value,
                                          double  valueNum,
                                          bool&   parmUsed
                                         )
{
  parmUsed = true;

  cmd.Upper ();

  if  (cmd == "-A")
    numSVM  = (int32)valueNum;

  else if  (cmd == "-C")
    C       = valueNum;

  else if  (cmd == "-D")
    degree = valueNum;

  else if  (cmd == "-E")
    eps  = valueNum;

  else if  ((cmd == "-G")  ||  (cmd == "-GAMMA"))
    gamma = valueNum;

  else if  (cmd == "-H")
    shrinking = (int32)valueNum;

  else if  (cmd == "-I")
    threshold = (float)valueNum;

  else if  (cmd == "-J")
    hist  = (int32)valueNum;

  else if  (cmd == "-K")
    boosting = (int32)valueNum;

  else if  (cmd == "-L")
    cBoost = (float)valueNum;

  else if  (cmd == "-M")
    cache_size  = valueNum;

  else if  (cmd == "-N")
    nu          = valueNum;
    
  else if  (cmd == "-O")
    dimSelect   = (int32)valueNum;
    
  else if  (cmd == "-P")
    p           = valueNum;

  else if  (cmd == "-Q")
    confidence  = valueNum;

  else if  (cmd == "-R")
    coef0       = valueNum;
    
  else if  (cmd == "-S")
    svm_type    = (int32)valueNum;

  else if  (cmd == "-T")
    kernel_type = (int32)valueNum;
    
  else if  (cmd == "-U")
    A           = (float)valueNum;

  else if  (cmd == "-W")
  {
    ++nr_weight;
    weight_label = (int32 *) realloc (weight_label, sizeof (int32) * nr_weight);
    weight = (double *) realloc (weight, sizeof (double) * nr_weight);
    weight_label[nr_weight - 1] = atoi (cmd.SubStrPart (2).Str ());
    weight[nr_weight - 1]       = valueNum;
  }

  else
    parmUsed = false;
}  /* ProcessSvmParameter */



svm_parameter::svm_parameter (KKStr&  _paramStr):
   coef0           (0.0),
   degree          (3),
   gamma           (0.0),
   kernel_type     (0),
   svm_type        (C_SVC),

   /* these are for training only */
   C               (1),
   cache_size      (40.0),
   eps             (1e-3),
   nr_weight       (0),
   nu              (0.5),
   p               (0.1),
   probability     (0),
   shrinking       (1),
   weight          (NULL),
   weight_label    (NULL),

  //luo add
   A               (1.0),
   boosting        (0),
   cBoost          (1.0),
   confidence      (0),
   dim             (0),
   dimSelect       (-1),
   featureWeight   (NULL),
   hist            (0),
   nr_class        (0),
   numSVM          (1),
   sample          (100),
   sampleSV        (0),
   threshold       (0.0f)

{
  KKStr  leftOverStr;

  KKStr  field (_paramStr.ExtractToken (" \t\n\r"));
  KKStr  value;

  double  valueNum;

  while  (!field.Empty ())
  {
    if  (field[0] != '-')  
    {
      leftOverStr << "  " << field;
    }
    else
    {
      _paramStr.TrimLeft (" \t\n\r");
      value == "";
      if  (_paramStr.Len () > 0)
      {
        if  (_paramStr[0] != '-')
          value = _paramStr.ExtractToken (" \t\n\r");
      }

      valueNum = atof (value.Str ()); 

      bool  parmUsed = false;

      if  (field.Len () > 2)
      {
        leftOverStr << "  " << field << " " << value;
      }
      else
      {
        ProcessSvmParameter (field, value, valueNum, parmUsed);
        if  (!parmUsed) 
        {
          leftOverStr << "  " << field << " " << value;
        }  
      }
    }
 
    field = _paramStr.ExtractToken (" \t\n\r");
  } 

  _paramStr = leftOverStr;
  
}  /* svm_parameter (KKStr  paramStr) */




KKStr  svm_parameter::ToCmdLineStr ()  const
{
  KKStr cmdStr (200); // Initialized char* allocation to 200

  cmdStr << "-a " << numSVM         << "  "
         //<< "-b " << sample         << "  "  
         << "-c " << C              << "  "
         << "-d " << degree         << "  "
         << "-e " << eps            << "  "
         << "-g " << gamma          << "  "
         << "-h " << shrinking      << "  "
         << "-i " << threshold      << "  "
         << "-j " << hist           << "  "
         << "-k " << boosting       << "  "
         << "-l " << cBoost         << "  "
         << "-m " << cache_size     << "  "
         << "-n " << nu             << "  ";

  if  (dimSelect >= 0)
    cmdStr  << "-o " << dimSelect   << "  ";

  cmdStr << "-p " << p              << "  "
         << "-q " << confidence     << "  "
         << "-r " << coef0          << "  "
         << "-s " << svm_type       << "  "
         << "-t " << kernel_type    << "  "
         << "-u " << A              << "  ";

  return  cmdStr;
}  /* ToCmdLineStr */





KKStr  svm_parameter::ToTabDelStr  ()  const
{
  int32  x;

  KKStr  result (300);

  result << "svm_type"    << "\t"  << svm_type       << "\t"
         << "kernel_type" << "\t"  << kernel_type    << "\t"
         << "degree"      << "\t"  << degree         << "\t"
         << "gamma"       << "\t"  << gamma          << "\t"
         << "coef0"       << "\t"  << coef0          << "\t"
         << "cache_size"  << "\t"  << cache_size     << "\t"
         << "eps"         << "\t"  << eps            << "\t"
         << "C"           << "\t"  << C              << "\t";

  result << "nr_weight"   << "\t"  << nr_weight      << "\t";
  if  (nr_weight > 0)
  {
    for  (x = 0;  x < nr_weight;  x++)
    {
      if  (x > 0)  result << ",";
      result << weight_label[x];
    }
    result << "\t";

    for  (x = 0;  x < nr_weight;  x++)
    {
      if  (x > 0)  result << ",";
      result << weight[x];
    }
    result << "\t";
  }

  result << "nu"          << "\t" << nu           << "\t"
         << "p"           << "\t" << p            << "\t"
         << "shrinking"   << "\t" << shrinking    << "\t"
         << "probability" << "\t" << probability  << "\t"
         << "numSVM"      << "\t" << numSVM       << "\t"
         << "sampleSV"    << "\t" << sampleSV     << "\t"
         << "hist"        << "\t" << hist         << "\t"
         << "boosting"    << "\t" << boosting     << "\t"
         << "cBoost"      << "\t" << cBoost       << "\t"
         << "dimSelect"   << "\t" << dimSelect    << "\t"
         << "dim"         << "\t" << dim          << "\t";

  if  (featureWeight)
  {
    result << "featureWeight" << "\t";
    for  (int32 x = 0;  x < dim;  x++)
    {
      if  (x > 0)
        result << ",";
      result << featureWeight[x];
    }
    result << "\t";
  }

  result << "confidence" << "\t"  << confidence << "\t"
         << "A"          << "\t"  << A          << "\t"
         << "nr_class"   << "\t"  << nr_class   << "\t"
         << "threshold"  << "\t"  << threshold  << "\t"
         << "sample"     << "\t"  << sample     << "\t";

  return  result;
}  /* ToTabDelStr */







void  svm_parameter::ParseTabDelStr (const KKStr&  _str)
{
  KKStr  str = _str;

  int32  x;


/*
  

  if  (featureWeight)
  {
    result << "featureWeight" << "\t";
    for  (int32 x = 0;  x < dim;  x++)
    {
      if  (x > 0)
        result << ",";
      result << featureWeight[x];
    }
    result << "\t";
  }
*/




  while  (!str.Empty ())
  {
    KKStr   field = str.ExtractToken2 ("\t");
    KKStr   value = str.ExtractToken2 ("\t");
    int32   valueI = value.ToInt    ();
    double  valueD = value.ToDouble ();
    float   valueF = value.ToFloat  ();

    if  (field == "svm_type")
      svm_type = valueI;

    else if  (field == "kernel_type")
      kernel_type = valueI;

    else if  (field == "degree")      
      degree = valueD;

    else if  (field == "gamma")
      gamma = valueD;

    else if  (field == "coef0")
      coef0 = valueD;
         
    else if  (field == "cache_size")
      cache_size = valueD;

    else if  (field == "eps")
      eps = valueD;

    else if  (field == "C")
      C = valueD;

    else if  (field == "nr_weight")
    {
      nr_weight = valueI;
      if  (nr_weight > 0)
      {
        delete[]  weight_label;
        weight_label = new int32[nr_weight];

        // value = weight label.
        for  (x = 0;  x < nr_weight;  x++)
        {
          weight_label[x] = value.ExtractTokenInt (",");
        }

        delete[]  weight;
        weight = new double [nr_weight];
        KKStr  weightStr = str.ExtractToken2 ("\t");
        for  (x = 0;  x < nr_weight;  x++)
        {
          weight[x] = weightStr.ExtractTokenDouble (",");
        }
      }
    }

    else if  (field == "nu")
      nu = valueD;

    else if  (field == "p")
      p = valueD;

    else if  (field == "shrinking")
      shrinking = valueI;

    else if  (field == "probability")
      probability = valueI;

    else if  (field == "numSVM")
      numSVM = valueI;

    else if  (field == "sampleSV")
      sampleSV = valueI;

    else if  (field == "hist")
      hist = valueI;

    else if  (field == "boosting")
      boosting = valueI;

    else if  (field == "cBoost")
      cBoost = valueF;

    else if  (field == "dimSelect")
      dimSelect = valueI;

    else if  (field == "dim")
      dim = valueI;

    else if  (field == "featureWeight")
    {
      delete[]  featureWeight;
      featureWeight = new double[dim];
      for  (x = 0;  x < dim;  x++)
      {
        featureWeight[x] = value.ExtractTokenDouble (",");
      }
    }

    else if  (field == "confidence" )
       confidence = valueD;

    else if  (field == "A")
       A = valueF;

    else if  (field == "nr_class")
      nr_class = valueI;

    else if  (field == "threshold")
      threshold  = valueF;

    else if  (field == "sample")
      sample = valueF;
  }
}  /* ParseTabDelStr */




svm_problem::svm_problem ()
{
  l     = 0;
  y     = NULL;
  index = NULL;
  x     = NULL;
  W     = NULL;
  weOwnContents = false;
};




svm_problem::~svm_problem ()
{
  if  (!weOwnContents)
    return;

  for  (int32 zed = 0;  zed < l;  zed++)
  {
    delete  x[zed];
    x[zed] = NULL;
  }

  delete  y;      y     = NULL;
  delete  index;  index = NULL;
  delete  x;      x     = NULL;
  delete  W;      W     = NULL;
}





//
// Kernel Cache
//
// l is the number of total data items
// size is the cache size limit in bytes
//
class SVM233::Cache
{
public:
  Cache (int32 l,  
         int32 size
        );
  ~Cache();

  // request data [0,len)
  // return some position p where [p,len) need to be filled
  // (p >= len if nothing needs to be filled)
  int32 get_data (const int32  index, 
                  Qfloat**     data, 
                  int32        len
                 );

  void swap_index (int32 i, 
                   int32 j
                  );  // future_option

private:
  int32 l;
  int32 size;

  struct head_t
  {
    head_t *prev, *next;  // a cicular list
    Qfloat *data;
    int32 len;              // data[0,len) is cached in this entry
  };

  head_t* head;
  head_t lru_head;
  void lru_delete (head_t *h);
  void lru_insert (head_t *h);
};



SVM233::Cache::Cache (int32 l_,
                      int32 size_
                     ):

l    (l_),
size (size_)

{
  head = (head_t *)calloc (l, sizeof (head_t));	// initialized to 0
  size /= sizeof (Qfloat);
  size -= l * sizeof (head_t) / sizeof (Qfloat);
  lru_head.next = lru_head.prev = &lru_head;
}



SVM233::Cache::~Cache()
{
  for (head_t *h = lru_head.next;  h != &lru_head;  h = h->next)
    free(h->data);
  free(head);
}



void SVM233::Cache::lru_delete (head_t *h)
{
  // delete from current location
  h->prev->next = h->next;
  h->next->prev = h->prev;
}



void SVM233::Cache::lru_insert(head_t *h)
{
  // insert to last position
  h->next = &lru_head;
  h->prev = lru_head.prev;
  h->prev->next = h;
  h->next->prev = h;
}



int32  SVM233::Cache::get_data (const int32 index, 
                              Qfloat**  data, 
                              int32       len
                             )
{
  head_t *h = &head[index];

  if  (h->len) 
    lru_delete(h);

  int32 more = len - h->len;

  if  (more > 0)
  {
    // free old space
    while(size < more)
    {
      head_t *old = lru_head.next;
      lru_delete(old);
      free(old->data);
      size += old->len;
      old->data = 0;
      old->len = 0;
    }

    // allocate new space
    h->data = (Qfloat *)realloc(h->data,sizeof(Qfloat)*len);
    size -= more;
    Swap(h->len, len);
  }

  lru_insert (h);
  *data = h->data;
  return len;
}



void SVM233::Cache::swap_index (int32 i, 
                                int32 j
                               )
{
  if(i==j) return;

  if(head[i].len) lru_delete(&head[i]);
  if(head[j].len) lru_delete(&head[j]);
  Swap(head[i].data,head[j].data);
  Swap(head[i].len,head[j].len);
  if(head[i].len) lru_insert(&head[i]);
  if(head[j].len) lru_insert(&head[j]);

  if(i>j) Swap(i,j);
  for(head_t *h = lru_head.next; h!=&lru_head; h=h->next)
  {
    if(h->len > i)
    {
      if(h->len > j)
        Swap(h->data[i],h->data[j]);
      else
      {
        // give up
        lru_delete(h);
        free(h->data);
        size += h->len;
        h->data = 0;
        h->len = 0;
      }
    }
  }
}



//
// Kernel evaluation
//
// the static method k_function is for doing single kernel evaluation
// the constructor of Kernel prepares to calculate the l*l kernel matrix
// the member function get_Q is for getting one column from the Q Matrix
//
class  SVM233::Kernel {
public:
  Kernel (int32                   l, 
          svm_node * const *    x, 
          const svm_parameter&  param
         );

  virtual ~Kernel();

  static double k_function (const svm_node*       x,
                            const svm_node*       y,
                            const svm_parameter&  param
                           );

  static double k_function_subspace (const svm_node*       x,
                                     const svm_node*       y,
                                     const svm_parameter&  param,
                                     const double*         featureWeight
                                    );

  virtual Qfloat *get_Q (int32 column, int32 len) const = 0;

  virtual void swap_index(int32 i, int32 j) const  // no so const...
  {
    Swap (x[i],x[j]);
    if(x_square) Swap(x_square[i],x_square[j]);
  }

protected:

  double (Kernel::*kernel_function)(int32 i, int32 j) const;

private:
  const svm_node **x;
  double*        x_square;

  // svm_parameter
  const int32 kernel_type;
  const double degree;
  const double gamma;
  const double coef0;
  //luo add
  int32 dim;
  double *featureWeight;

  static double dot(const svm_node *px, const svm_node *py);
  static double dotSubspace(const svm_node *px, const svm_node *py, const double *featureWeight);

  double kernel_linear (int32 i, int32 j) const
  {
    return dot(x[i],x[j]);
  }
  double kernel_linear_subspace(int32 i, int32 j) const
  {
    return dotSubspace(x[i],x[j], featureWeight);
  }
  double kernel_poly (int32 i, int32 j) const
  {
    return pow(gamma*dot(x[i],x[j])+coef0,degree);
  }
  double kernel_poly_subspace(int32 i, int32 j) const
  {
    return pow(gamma*dotSubspace(x[i],x[j], featureWeight)+coef0,degree);
  }
  double kernel_rbf (int32 i, int32 j) const
  {
    return exp(-gamma*(x_square[i]+x_square[j]-2*dot(x[i],x[j])));
  }
  double kernel_rbf_subspace(int32 i, int32 j) const
  {
    return exp(-gamma*(x_square[i]+x_square[j]-2*dotSubspace(x[i],x[j],featureWeight)));
  }
  double kernel_sigmoid(int32 i, int32 j) const
  {
    return tanh(gamma*dot(x[i],x[j])+coef0);
  }
  double kernel_sigmoid_subspace(int32 i, int32 j) const
  {
    return tanh(gamma*dotSubspace(x[i],x[j],featureWeight)+coef0);
  }
};



SVM233::Kernel::Kernel  (int32                   l,
                         svm_node * const *    x_, 
                         const svm_parameter&  param
                        )
 :
   kernel_type      (param.kernel_type), 
   degree           (param.degree),
   gamma            (param.gamma), 
   coef0        (param.coef0)

{
  dim=0;
  if  (param.dimSelect >0)
  {
    dim = param.dim;
    featureWeight = new double[dim];
    std::copy (param.featureWeight, param.featureWeight + dim, featureWeight);
  }

  switch (kernel_type)
  {
  case LINEAR:
    if(param.dimSelect >0)
      kernel_function = &Kernel::kernel_linear_subspace;
    else
      kernel_function = &Kernel::kernel_linear;
    break;

  case POLY:
    if(param.dimSelect >0)
      kernel_function = &Kernel::kernel_poly_subspace;
    else
      kernel_function = &Kernel::kernel_poly;
    break;

  case RBF:
    if(param.dimSelect >0)
      kernel_function = &Kernel::kernel_rbf_subspace;
    else
      kernel_function = &Kernel::kernel_rbf;
    break;

  case SIGMOID:
    if(param.dimSelect >0)
      kernel_function = &Kernel::kernel_sigmoid_subspace;
    else
      kernel_function = &Kernel::kernel_sigmoid;
    break;
  }

  clone (x, x_,  l);

  if  (kernel_type == RBF)
  {
    x_square = new double[l];
    for(int32 i=0;i<l;i++)
    {
      if(param.dimSelect > 0)
        x_square[i] = dotSubspace(x[i],x[i],featureWeight);
      else
        x_square[i] = dot(x[i],x[i]);
    }
  }
  else
    x_square = 0;
}

SVM233::Kernel::~Kernel()
{
  if (dim>0)
    delete[] featureWeight;
  delete[] x;
  delete[] x_square;
}



double SVM233::Kernel::dot(const svm_node *px, const svm_node *py)
{
  double sum = 0;
  while(px->index != -1 && py->index != -1)
  {
    if(px->index == py->index)
    {
      sum += px->value * py->value;
      ++px;
      ++py;
    }
    else
  {
      if(px->index > py->index)
        ++py;
      else
        ++px;
    }
  }
  return sum;
}



double SVM233::Kernel::dotSubspace(const svm_node *px, const svm_node *py, const double *featureWeight)
{
  double sum = 0;
  while(px->index != -1 && py->index != -1)
  {
    if(px->index == py->index)
    {
      sum += px->value * py->value * featureWeight[px->index-1];
      ++px;
      ++py;
    }
    else
  {
      if(px->index > py->index)
        ++py;
      else
        ++px;
    }
  }
  return sum;
}




double  SVM233::Kernel::k_function (const svm_node*       x,
                                    const svm_node*       y,
                                    const svm_parameter&  param
                                   )
{

  switch(param.kernel_type)
  {
  case LINEAR:
    return dot(x,y);

  case POLY:
    return pow(param.gamma*dot(x,y)+param.coef0,param.degree);

  case RBF:
    {
      double sum = 0;
      while(x->index != -1 && y->index !=-1)
      {
        if(x->index == y->index)
        {
          double d = x->value - y->value;
          sum += d*d;
          ++x;
          ++y;
        }
        else
        {
          if(x->index > y->index)
          {
            double d=y->value;
            sum += d*d;
            ++y;
          }
          else
          {
            double d=x->value;
            sum += d*d;
            ++x;
          }
        }
      }

      while(x->index != -1)
      {
        double d = x->value;
        sum += d*d;
        //sum += x->value * x->value;
        ++x;
      }

      while(y->index != -1)
      {
        double d=y->value;
        sum += d * d;
        //sum += y->value * y->value;
        ++y;
      }

      return exp (-param.gamma * sum);
    }
  case SIGMOID:
    return tanh(param.gamma*dot(x,y)+param.coef0);
  default:
    return 0;  /* Unreachable */
  }
}



double  SVM233::Kernel::k_function_subspace (const svm_node*       x,
                                             const svm_node*       y,
                                             const svm_parameter&  param,
                                             const double*         featureWeight
                                            )
{
  switch  (param.kernel_type)
  {
  case LINEAR:
    return dotSubspace(x,y,featureWeight);
  case POLY:
    return pow(param.gamma*dotSubspace(x,y,featureWeight)+param.coef0,param.degree);

  case RBF:
    {
      double sum = 0;
      while(x->index != -1 && y->index !=-1)
      {
        if(x->index == y->index)
        {
          double d = x->value - y->value;
          sum+=d*d*featureWeight[x->index-1];

          ++x;
          ++y;
        }
        else
        {
          if(x->index > y->index)
          {
            double d=y->value;
            sum+=d*d*featureWeight[y->index-1];
            ++y;
          }
          else
          {
            double d=x->value;
            sum+=d*d*featureWeight[x->index-1];
            ++x;
          }
        }
      }

      while(x->index != -1)
      {
        double d = x->value;
        sum+=d*d*featureWeight[x->index-1];

        //sum += x->value * x->value;
        ++x;
      }

      while(y->index != -1)
      {
        double d=y->value;

        sum+=d*d*featureWeight[y->index-1];

        //sum += y->value * y->value;
        ++y;
      }

      return exp (-param.gamma * sum);
    }
  case SIGMOID:
    return tanh(param.gamma*dotSubspace(x,y, featureWeight)+param.coef0);
  default:
    return 0;  /* Unreachable */
  }
}  /* k_function_subspace */






void SVM233::Solver::swap_index(int32 i, int32 j)
{
  Q->swap_index(i,j);
  Swap(y[i],y[j]);
  Swap(G[i],G[j]);
  Swap(alpha_status[i],alpha_status[j]);
  Swap(alpha[i],alpha[j]);
  Swap(b[i],b[j]);
  Swap(active_set[i],active_set[j]);
  Swap(G_bar[i],G_bar[j]);
  Swap(C[i], C[j]);
}




void SVM233::Solver::reconstruct_gradient()
{
  // reconstruct inactive elements of G from G_bar and free variables

  if  (active_size == l) 
    return;

  int32 i;
  for(i=active_size;i<l;i++)
    G[i] = G_bar[i] + b[i];

  for(i=0;i<active_size;i++)
    if(is_free(i))
    {
      const Qfloat *Q_i = Q->get_Q(i,l);
      double alpha_i = alpha[i];
      for(int32 j=active_size;j<l;j++)
        G[j] += alpha_i * Q_i[j];
    }
}



void  SVM233::Solver::Solve(int32            l,
                            const Kernel&  Q, 
                            const double*  b_, 
                            const schar*   y_,
                            double*        alpha_, 
                            double*        C_, 
                            double         eps,
                            SolutionInfo*  si, 
                            int32            shrinking
                           )
{
  this->l = l;
  this->Q = &Q;
  clone (b, b_,l);
  clone (y, y_,l);
  clone (alpha,alpha_,l);
  clone (C, C_, l);
  this->eps = eps;
  unshrinked = false;

  // initialize alpha_status
  {
    alpha_status = new char[l];
    for (int32 i=0;i<l;i++){
      update_alpha_status(i);
    }
  }

  // initialize active set (for shrinking)
  {
    active_set = new int32[l];
    for(int32 i=0;i<l;i++)
      active_set[i] = i;
    active_size = l;
  }

  // initialize gradient
  {
    G = new double[l];
    G_bar = new double[l];
    int32 i;
    for(i=0;i<l;i++)
    {
      G[i] = b[i];
      G_bar[i] = 0;
    }
    for (i=0;i<l;i++)
      if  (!is_lower_bound(i))
      {
        Qfloat *Q_i = Q.get_Q(i,l);
        double alpha_i = alpha[i];
        int32 j;
        for(j=0;j<l;j++)
          G[j] += alpha_i*Q_i[j];
        if(is_upper_bound(i))
          for(j=0;j<l;j++)
            G_bar[j] += get_C(i) * Q_i[j];
      }
  }

  // optimization step

  int32 iter = 0;
  int32 counter = min ((int32)l, (int32)1000) + 1;

  while(1)
  {
    // show progress and do shrinking

    if(--counter == 0)
    {
      counter = min(l,1000);
      if(shrinking) do_shrinking();
      info("."); info_flush();
    }

    int32 i,j;
    if(select_working_set(i,j)!=0)
    {
      // reconstruct the whole gradient
      reconstruct_gradient();
      // reset active set size and check
      active_size = l;
      info("*"); info_flush();
      if(select_working_set(i,j)!=0)
        break;
      else
        counter = 1;  // do shrinking next iteration
    }

    ++iter;

    // update alpha[i] and alpha[j], handle bounds carefully

    const Qfloat *Q_i = Q.get_Q(i,active_size);
    const Qfloat *Q_j = Q.get_Q(j,active_size);

    double C_i = get_C(i);
    double C_j = get_C(j);

    double old_alpha_i = alpha[i];
    double old_alpha_j = alpha[j];

    if(y[i]!=y[j])
    {
      double delta = (-G[i]-G[j])/max(Q_i[i]+Q_j[j]+2*Q_i[j],(Qfloat)0);
      double diff = alpha[i] - alpha[j];
      alpha[i] += delta;
      alpha[j] += delta;

      if(diff > 0)
      {
        if(alpha[j] < 0)
        {
          alpha[j] = 0;
          alpha[i] = diff;
        }
      }
      else
      {
        if(alpha[i] < 0)
        {
          alpha[i] = 0;
          alpha[j] = -diff;
        }
      }
      if(diff > C_i - C_j)
      {
        if(alpha[i] > C_i)
        {
          alpha[i] = C_i;
          alpha[j] = C_i - diff;
        }
      }
      else
      {
        if(alpha[j] > C_j)
        {
          alpha[j] = C_j;
          alpha[i] = C_j + diff;
        }
      }
    }
    else
    {
      double delta = (G[i]-G[j])/max(Q_i[i]+Q_j[j]-2*Q_i[j],(Qfloat)0);
      double sum = alpha[i] + alpha[j];
      alpha[i] -= delta;
      alpha[j] += delta;
      if(sum > C_i)
      {
        if(alpha[i] > C_i)
        {
          alpha[i] = C_i;
          alpha[j] = sum - C_i;
        }
      }
      else
      {
        if(alpha[j] < 0)
        {
          alpha[j] = 0;
          alpha[i] = sum;
        }
      }
      if(sum > C_j)
      {
        if(alpha[j] > C_j)
        {
          alpha[j] = C_j;
          alpha[i] = sum - C_j;
        }
      }
      else
      {
        if(alpha[i] < 0)
        {
          alpha[i] = 0;
          alpha[j] = sum;
        }
      }
    }

    // update G

    double delta_alpha_i = alpha[i] - old_alpha_i;
    double delta_alpha_j = alpha[j] - old_alpha_j;

    for(int32 k=0;k<active_size;k++)
    {
      G[k] += Q_i[k]*delta_alpha_i + Q_j[k]*delta_alpha_j;
    }

    // update alpha_status and G_bar

    {
      bool ui = is_upper_bound(i);
      bool uj = is_upper_bound(j);
      update_alpha_status(i);
      update_alpha_status(j);
      int32 k;
      if(ui != is_upper_bound(i))
      {
        Q_i = Q.get_Q(i,l);
        if(ui)
          for(k=0;k<l;k++)
            G_bar[k] -= C_i * Q_i[k];
        else
          for(k=0;k<l;k++)
            G_bar[k] += C_i * Q_i[k];
      }

      if(uj != is_upper_bound(j))
      {
        Q_j = Q.get_Q(j,l);
        if(uj)
          for(k=0;k<l;k++)
            G_bar[k] -= C_j * Q_j[k];
        else
          for(k=0;k<l;k++)
            G_bar[k] += C_j * Q_j[k];
      }
    }
  }

  // calculate rho

  si->rho = calculate_rho();

  // calculate objective value
  {
    double v = 0;
    int32 i;
    for(i=0;i<l;i++)
      v += alpha[i] * (G[i] + b[i]);

    si->obj = v/2;
  }

  // put back the solution
  {
    for(int32 i=0;i<l;i++)
      alpha_[active_set[i]] = alpha[i];
  }

  // juggle everything back
  /*{
  for(int32 i=0;i<l;i++)
  while(active_set[i] != i)
  swap_index(i,active_set[i]);
  // or Q.swap_index(i,active_set[i]);
  }*/

  info("\noptimization finished, #iter = %d\n",iter);

  delete[] C;
  delete[] b;
  delete[] y;
  delete[] alpha;
  delete[] alpha_status;
  delete[] active_set;
  delete[] G;
  delete[] G_bar;
}


void SVM233::Solver::Solve(int32            l,
                           const Kernel&  Q,
                           const double*  b_,
                           const schar*   y_,
                           double*        alpha_,
                           double         Cp,
                           double         Cn,
                           double         eps,
                           SolutionInfo*  si, 
                           int32            shrinking
                          )
{
  double* C_ = new double[l];
  for(int32 i=0;i<l;i++)
    C_[i] = (y_[i] > 0 ? Cp : Cn);

  Solve (l, Q, b_, y_, alpha_, C_, eps, si, shrinking);

  si->upper_bound_p = Cp;
  si->upper_bound_n = Cn;
  delete [] C_;
}



// return 1 if already optimal, return 0 otherwise
int32  SVM233::Solver::select_working_set(int32 &out_i, int32 &out_j)
{
  // return i,j which maximize -grad(f)^T d , under constraint
  // if alpha_i == C, d != +1
  // if alpha_i == 0, d != -1

  double Gmax1 = -INF;  // max { -grad(f)_i * d | y_i*d = +1 }
  int32 Gmax1_idx = -1;

  double Gmax2 = -INF;  // max { -grad(f)_i * d | y_i*d = -1 }
  int32 Gmax2_idx = -1;

  for(int32 i=0;i<active_size;i++)
  {
    if(y[i]==+1)  // y = +1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] > Gmax1)
        {
          Gmax1 = -G[i];
          Gmax1_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] > Gmax2)
        {
          Gmax2 = G[i];
          Gmax2_idx = i;
        }
      }
    }
    else  // y = -1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] > Gmax2)
        {
          Gmax2 = -G[i];
          Gmax2_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] > Gmax1)
        {
          Gmax1 = G[i];
          Gmax1_idx = i;
        }
      }
    }
  }

  if(Gmax1+Gmax2 < eps)
    return 1;

  out_i = Gmax1_idx;
  out_j = Gmax2_idx;
  return 0;
}



void  SVM233::Solver::do_shrinking()
{
  int32 i,j,k;
  if  (select_working_set (i,j) != 0) 
    return;

  double Gm1 = -y[j]*G[j];
  double Gm2 = y[i]*G[i];

  // shrink

  for  (k = 0;  k < active_size;  k++)
  {
    if  (is_lower_bound (k))
    {
      if  (y[k]==+1)
      {
        if(-G[k] >= Gm1) continue;
      }
      else if  (-G[k] >= Gm2) continue;
    }
    else if  (is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] >= Gm2) continue;
      }
      else if(G[k] >= Gm1) continue;
    }
    else continue;

    --active_size;
    swap_index(k,active_size);
    --k;  // look at the newcomer
  }

  // unshrink, check all variables again before final iterations

  if(unshrinked || -(Gm1 + Gm2) > eps*10) return;

  unshrinked = true;
  reconstruct_gradient();

  for(k=l-1;k>=active_size;k--)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] < Gm1) continue;
      }
      else if(-G[k] < Gm2) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] < Gm2) continue;
      }
      else if(G[k] < Gm1) continue;
    }
    else continue;

    swap_index(k,active_size);
    active_size++;
    ++k;  // look at the newcomer
  }
}




double  SVM233::Solver::calculate_rho()
{
  double r;
  int32 nr_free = 0;
  double ub = INF, lb = -INF, sum_free = 0;
  for  (int32 i = 0;  i < active_size;  i++)
  {
    double yG = y[i]*G[i];

    if(is_lower_bound(i))
    {
      if(y[i] > 0)
        ub = min(ub,yG);
      else
        lb = max(lb,yG);
    }
    else if(is_upper_bound(i))
    {
      if(y[i] < 0)
        ub = min(ub,yG);
      else
        lb = max(lb,yG);
    }
    else
    {
      ++nr_free;
      sum_free += yG;
    }
  }

  if(nr_free>0)
    r = sum_free/nr_free;
  else
    r = (ub+lb)/2;

  return r;
}




//
// Solver for nu-svm classification and regression
//
// additional constraint: e^T \alpha = constant
//
class  SVM233::Solver_NU : public Solver
{
public:
  Solver_NU() {}
  void Solve (int32 l, 
              const Kernel& Q, 
              const double *b, 
              const schar *y,
              double *alpha, 
              double Cp, 
              double Cn, 
              double eps,
              SolutionInfo* si, 
              int32 shrinking
             )
  {
    this->si = si;
    Solver::Solve(l,Q,b,y,alpha,Cp,Cn,eps,si,shrinking);
  }

private:
  SolutionInfo *si;
  int32 select_working_set(int32 &i, int32 &j);
  double calculate_rho();
  void do_shrinking();
};




int32 SVM233::Solver_NU::select_working_set(int32 &out_i, int32 &out_j)
{
  // return i,j which maximize -grad(f)^T d , under constraint
  // if alpha_i == C, d != +1
  // if alpha_i == 0, d != -1

  double Gmax1 = -INF;  // max { -grad(f)_i * d | y_i = +1, d = +1 }
  int32 Gmax1_idx = -1;

  double Gmax2 = -INF;  // max { -grad(f)_i * d | y_i = +1, d = -1 }
  int32 Gmax2_idx = -1;

  double Gmax3 = -INF;  // max { -grad(f)_i * d | y_i = -1, d = +1 }
  int32 Gmax3_idx = -1;

  double Gmax4 = -INF;  // max { -grad(f)_i * d | y_i = -1, d = -1 }
  int32 Gmax4_idx = -1;

  for(int32 i=0;i<active_size;i++)
  {
    if(y[i]==+1)  // y == +1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] > Gmax1)
        {
          Gmax1 = -G[i];
          Gmax1_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] > Gmax2)
        {
          Gmax2 = G[i];
          Gmax2_idx = i;
        }
      }
    }
    else  // y == -1
    {
      if(!is_upper_bound(i))  // d = +1
      {
        if(-G[i] > Gmax3)
        {
          Gmax3 = -G[i];
          Gmax3_idx = i;
        }
      }
      if(!is_lower_bound(i))  // d = -1
      {
        if(G[i] > Gmax4)
        {
          Gmax4 = G[i];
          Gmax4_idx = i;
        }
      }
    }
  }

  if(max(Gmax1+Gmax2,Gmax3+Gmax4) < eps)
    return 1;

  if(Gmax1+Gmax2 > Gmax3+Gmax4)
  {
    out_i = Gmax1_idx;
    out_j = Gmax2_idx;
  }
  else
  {
    out_i = Gmax3_idx;
    out_j = Gmax4_idx;
  }
  return 0;
}




void SVM233::Solver_NU::do_shrinking()
{
  double Gmax1 = -INF;  // max { -grad(f)_i * d | y_i = +1, d = +1 }
  double Gmax2 = -INF;  // max { -grad(f)_i * d | y_i = +1, d = -1 }
  double Gmax3 = -INF;  // max { -grad(f)_i * d | y_i = -1, d = +1 }
  double Gmax4 = -INF;  // max { -grad(f)_i * d | y_i = -1, d = -1 }

  int32 k;
  for  (k = 0;  k < active_size;  k++)
  {
    if(!is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] > Gmax1) Gmax1 = -G[k];
      }
      else  if(-G[k] > Gmax3) Gmax3 = -G[k];
    }
    if(!is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] > Gmax2) Gmax2 = G[k];
      }
      else  if(G[k] > Gmax4) Gmax4 = G[k];
    }
  }

  double Gm1 = -Gmax2;
  double Gm2 = -Gmax1;
  double Gm3 = -Gmax4;
  double Gm4 = -Gmax3;

  for(k=0;k<active_size;k++)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] >= Gm1) continue;
      }
      else  if(-G[k] >= Gm3) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] >= Gm2) continue;
      }
      else  if(G[k] >= Gm4) continue;
    }
    else continue;

    --active_size;
    swap_index(k,active_size);
    --k;  // look at the newcomer
  }

  // unshrink, check all variables again before final iterations

  if(unshrinked || max(-(Gm1+Gm2),-(Gm3+Gm4)) > eps*10) return;

  unshrinked = true;
  reconstruct_gradient();

  for(k=l-1;k>=active_size;k--)
  {
    if(is_lower_bound(k))
    {
      if(y[k]==+1)
      {
        if(-G[k] < Gm1) continue;
      }
      else  if(-G[k] < Gm3) continue;
    }
    else if(is_upper_bound(k))
    {
      if(y[k]==+1)
      {
        if(G[k] < Gm2) continue;
      }
      else  if(G[k] < Gm4) continue;
    }
    else continue;

    swap_index(k,active_size);
    active_size++;
    ++k;  // look at the newcomer
  }
}




double SVM233::Solver_NU::calculate_rho ()
{
  int32 nr_free1 = 0,nr_free2 = 0;
  double ub1 = INF, ub2 = INF;
  double lb1 = -INF, lb2 = -INF;
  double sum_free1 = 0, sum_free2 = 0;

  for(int32 i=0;i<active_size;i++)
  {
    if(y[i]==+1)
    {
      if(is_lower_bound(i))
        ub1 = min(ub1,G[i]);
      else if(is_upper_bound(i))
        lb1 = max(lb1,G[i]);
      else
      {
        ++nr_free1;
        sum_free1 += G[i];
      }
    }
    else
    {
      if(is_lower_bound(i))
        ub2 = min(ub2,G[i]);
      else if(is_upper_bound(i))
        lb2 = max(lb2,G[i]);
      else
      {
        ++nr_free2;
        sum_free2 += G[i];
      }
    }
  }

  double r1,r2;
  if(nr_free1 > 0)
    r1 = sum_free1/nr_free1;
  else
    r1 = (ub1+lb1)/2;

  if(nr_free2 > 0)
    r2 = sum_free2/nr_free2;
  else
    r2 = (ub2+lb2)/2;

  si->r = (r1+r2)/2;
  return (r1-r2)/2;
}

//
// Q matrices for various formulations
//
class  SVM233::SVC_Q: public Kernel
{ 
public:
  SVC_Q (const svm_problem&    prob,
         const svm_parameter&  param,
         const schar *y_
        )
    :Kernel (prob.l, prob.x, param)
  {
    clone(y,y_,prob.l);
    cache = new Cache(prob.l,(int32)(param.cache_size*(1<<20)));
  }

  Qfloat *get_Q(int32 i, int32 len) const
  {
    Qfloat *data;
    int32 start;
    if  ((start = cache->get_data (i, &data, len)) < len)
    {
      for  (int32 j = start;  j < len;  j++)
        data[j] = (Qfloat)(y[i] * y[j] * (this->*kernel_function)(i,j));
      //luo add data[j] = (Qfloat)(w[i]*w[j]*y[i]*y[j]*(this->*kernel_function)(i,j));
    }
    return data;
  }

  void swap_index(int32 i, int32 j) const
  {
    cache->swap_index(i,j);
    Kernel::swap_index(i,j);
    Swap(y[i],y[j]);
  }

  ~SVC_Q()
  {
    delete[] y;
    delete cache;
  }

private:
  schar *y;
  Cache *cache;
};




class  SVM233::ONE_CLASS_Q: public Kernel
{
public:
  ONE_CLASS_Q (const svm_problem&    prob,
               const svm_parameter&  param
              )
    :Kernel (prob.l, prob.x, param)
  {
    cache = new Cache(prob.l,(int32)(param.cache_size*(1<<20)));
  }

  Qfloat *get_Q(int32 i, int32 len) const
  {
    Qfloat *data;
    int32 start;
    if((start = cache->get_data(i,&data,len)) < len)
    {
      for(int32 j=start;j<len;j++)
        data[j] = (Qfloat)(this->*kernel_function)(i,j);
    }
    return data;
  }

  void swap_index(int32 i, int32 j) const
  {
    cache->swap_index(i,j);
    Kernel::swap_index(i,j);
  }


  ~ONE_CLASS_Q()
  {
    delete cache;
  }

private:
  Cache *cache;
};



class  SVM233::SVR_Q: public Kernel
{ 
public:
  SVR_Q (const svm_problem& prob,
         const svm_parameter& param
        )
    :Kernel (prob.l, prob.x, param)
  {
    l = prob.l;
    cache = new Cache(l,(int32)(param.cache_size*(1<<20)));
    sign = new schar[2*l];
    index = new int32[2*l];
    for(int32 k=0;k<l;k++)
    {
      sign[k] = 1;
      sign[k+l] = -1;
      index[k] = k;
      index[k+l] = k;
    }
    buffer[0] = new Qfloat[2*l];
    buffer[1] = new Qfloat[2*l];
    next_buffer = 0;
  }

  void swap_index(int32 i, int32 j) const
  {
    Swap(sign[i],sign[j]);
    Swap(index[i],index[j]);
  }


  Qfloat *get_Q(int32 i, int32 len) const
  {
    Qfloat *data;
    int32 real_i = index[i];
    if(cache->get_data(real_i,&data,l) < l)
    {
      for(int32 j=0;j<l;j++)
        data[j] = (Qfloat)(this->*kernel_function)(real_i,j);
    }

    // reorder and copy
    Qfloat *buf = buffer[next_buffer];
    next_buffer = 1 - next_buffer;
    schar si = sign[i];
    for(int32 j=0;j<len;j++)
      buf[j] = si * sign[j] * data[index[j]];
    return buf;
  }

  ~SVR_Q()
  {
    delete cache;
    delete[] sign;
    delete[] index;
    delete[] buffer[0];
    delete[] buffer[1];
  }
private:
  int32 l;
  Cache *cache;
  schar *sign;
  int32 *index;
  mutable int32 next_buffer;
  Qfloat* buffer[2];
};




//
// construct and solve various formulations
//
static void  SVM233::solve_c_svc (const svm_problem*     prob, 
                                  const svm_parameter*   param,
                                  double*                alpha, 
                                  Solver::SolutionInfo*  si, 
                                  double                 Cp, 
                                  double                 Cn
                                 )
{
  int32 l = prob->l;
  double *minus_ones = new double[l];
  schar *y = new schar[l];

  int32 i;

  for(i=0;i<l;i++)
  {
    alpha[i] = 0;
    minus_ones[i] = -1;
    if  (prob->y[i] > 0) 
      y[i] = +1; 
    else 
      y[i]=-1;
  }

  Solver s;
  s.Solve (l, 
           SVC_Q(*prob,*param,y), 
           minus_ones, 
           y,
           alpha, 
           Cp, 
           Cn,
           param->eps,
           si,
           param->shrinking
          );

  double sum_alpha=0;
  for(i=0;i<l;i++)
    sum_alpha += alpha[i];

  info("nu = %f\n", sum_alpha/(param->C*prob->l));

  for(i=0;i<l;i++)
    alpha[i] *= y[i];

  delete[] minus_ones;
  delete[] y;
}  /* solve_c_svc */



//luo: the weighted training
static void  SVM233::solve_c_svc (const svm_problem*     prob, 
                                  const svm_parameter*   param,
                                  double*                alpha, 
                                  Solver::SolutionInfo*  si, 
                                  double*                C_
                                 )
{
  int32 l = prob->l;
  double *minus_ones = new double[l];
  schar *y = new schar[l];

  int32 i;

  for  (i = 0;  i < l;  i++)
  {
    alpha[i] = 0;
    minus_ones[i] = -1;
    if  (prob->y[i] > 0) 
      y[i] = +1; 
    else 
      y[i] = -1;
  }

  Solver s;
  s.Solve (l, 
           SVC_Q (*prob, *param, y), 
           minus_ones, 
           y,
           alpha, 
           C_, 
           param->eps, 
           si, 
           param->shrinking
          );

  double sum_alpha=0;
  for  (i = 0;  i < l;  i++)
    sum_alpha += alpha[i];

  info("nu = %f\n", sum_alpha/(param->C * prob->l));

  for  (i = 0;  i < l;  i++)
    alpha[i] *= y[i];

  delete[] minus_ones;
  delete[] y;
}  /* solve_c_svc */  



static void  SVM233::solve_nu_svc (const svm_problem*    prob, 
                                   const svm_parameter*  param,
                                   double*               alpha, 
                                   Solver::SolutionInfo* si
                                  )
{
  int32 i;
  int32 l = prob->l;
  double nu = param->nu;

  schar *y = new schar[l];

  for(i=0;i<l;i++)
    if(prob->y[i]>0)
      y[i] = +1;
    else
      y[i] = -1;

  double sum_pos = nu*l/2;
  double sum_neg = nu*l/2;

  for(i=0;i<l;i++)
    if(y[i] == +1)
    {
      alpha[i] = min(1.0,sum_pos);
      sum_pos -= alpha[i];
    }
    else
    {
      alpha[i] = min(1.0,sum_neg);
      sum_neg -= alpha[i];
    }

    double *zeros = new double[l];

    for(i=0;i<l;i++)
      zeros[i] = 0;

    Solver_NU s;
    s.Solve(l, SVC_Q(*prob,*param,y), zeros, y,
      alpha, 1.0, 1.0, param->eps, si,  param->shrinking);
    double r = si->r;

    info("C = %f\n",1/r);

    for(i=0;i<l;i++)
      alpha[i] *= y[i]/r;

    si->rho /= r;
    si->obj /= (r*r);
    si->upper_bound_p = 1/r;
    si->upper_bound_n = 1/r;

    delete[] y;
    delete[] zeros;
}  /* solve_nu_svc */



static void  SVM233::solve_one_class (const svm_problem*     prob, 
                                      const svm_parameter*   param,
                                      double*                alpha, 
                                      Solver::SolutionInfo*  si
                                     )
{
  int32 l = prob->l;
  double *zeros = new double[l];
  schar *ones = new schar[l];
  int32 i;

  int32 n = (int32)(param->nu*prob->l);  // # of alpha's at upper bound

  for(i=0;i<n;i++)
    alpha[i] = 1;
  alpha[n] = param->nu * prob->l - n;
  for(i=n+1;i<l;i++)
    alpha[i] = 0;

  for(i=0;i<l;i++)
  {
    zeros[i] = 0;
    ones[i] = 1;
  }

  Solver s;
  s.Solve(l, ONE_CLASS_Q(*prob,*param), zeros, ones,
    alpha, 1.0, 1.0, param->eps, si, param->shrinking);

  delete[] zeros;
  delete[] ones;
}  /* solve_one_class */



static void  SVM233::solve_epsilon_svr (const svm_problem*     prob, 
                                        const svm_parameter*   param,
                                        double*                alpha, 
                                        Solver::SolutionInfo*  si
                                       )
{
  int32 l = prob->l;
  double *alpha2 = new double[2*l];
  double *linear_term = new double[2*l];
  schar *y = new schar[2*l];
  int32 i;

  for(i=0;i<l;i++)
  {
    alpha2[i] = 0;
    linear_term[i] = param->p - prob->y[i];
    y[i] = 1;

    alpha2[i+l] = 0;
    linear_term[i+l] = param->p + prob->y[i];
    y[i+l] = -1;
  }

  Solver s;
  s.Solve(2 * l,
          SVR_Q (*prob,*param), 
          linear_term, 
          y,
          alpha2, 
          param->C, 
          param->C, 
          param->eps, 
          si, 
          param->shrinking
         );

  double sum_alpha = 0;
  for(i=0;i<l;i++)
  {
    alpha[i] = alpha2[i] - alpha2[i+l];
    sum_alpha += fabs(alpha[i]);
  }
  info("nu = %f\n",sum_alpha/(param->C*l));

  delete[] alpha2;
  delete[] linear_term;
  delete[] y;
}



static void  SVM233::solve_nu_svr (const svm_problem*     prob, 
                                   const svm_parameter*   param,
                                   double*                alpha, 
                                   Solver::SolutionInfo*  si
                                  )
{
  int32 l = prob->l;
  double C = param->C;
  double *alpha2 = new double[2*l];
  double *linear_term = new double[2*l];
  schar *y = new schar[2*l];
  int32 i;

  double sum = C * param->nu * l / 2;
  for(i=0;i<l;i++)
  {
    alpha2[i] = alpha2[i+l] = min(sum,C);
    sum -= alpha2[i];

    linear_term[i] = - prob->y[i];
    y[i] = 1;

    linear_term[i+l] = prob->y[i];
    y[i+l] = -1;
  }

  Solver_NU s;
  s.Solve(2*l, SVR_Q(*prob,*param), linear_term, y,
    alpha2, C, C, param->eps, si, param->shrinking);

  info("epsilon = %f\n",-si->r);

  for (i = 0; i < l;  i++)
  {
    alpha[i] = alpha2[i] - alpha2[i + l];
  }

  delete[] alpha2;
  delete[] linear_term;
  delete[] y;
}







decision_function  SVM233::svm_train_one (const svm_problem*    prob, 
                                          const svm_parameter*  param
                                         )
{
  double *alpha = Malloc(double,prob->l);
  Solver::SolutionInfo si;
  switch(param->svm_type)
  {
  case C_SVC:
    solve_c_svc (prob, param, alpha,&si, prob->W);
    break;
  }

  info("obj = %f, rho = %f\n",si.obj,si.rho);

  // output SVs

  int32 nSV = 0;
  int32 nBSV = 0;
  for (int32 i = 0;  i < prob->l;  i++)
  {
    if  (fabs (alpha[i]) > 0)
    {
      ++nSV;
      if (prob->y[i] > 0)
      {
        if (fabs (alpha[i]) >= prob->W[i])
          ++nBSV;
      }
      else
      {
        if (fabs(alpha[i]) >= prob->W[i])
          ++nBSV;
      }
    }
  }

  info("nSV = %d, nBSV = %d\n",nSV,nBSV);

  decision_function f;
  f.alpha = alpha;
  f.rho = si.rho;
  return f;
}  /* svm_train_one */



decision_function  SVM233::svm_train_one (const svm_problem*    prob, 
                                          const svm_parameter*  param,
                                          double                Cp, 
                                          double                Cn, 
                                          std::set<int32>&        BSVIndex
                                         )
{
  double *alpha = Malloc (double, prob->l);
  Solver::SolutionInfo si;
  switch(param->svm_type)
  {
  case C_SVC:
    solve_c_svc (prob, param, alpha, &si, Cp, Cn);
    break;

  case NU_SVC:
    solve_nu_svc(prob,param,alpha,&si);
    break;

  case ONE_CLASS:
    solve_one_class(prob,param,alpha,&si);
    break;

  case EPSILON_SVR:
    solve_epsilon_svr(prob,param,alpha,&si);
    break;

  case NU_SVR:
    solve_nu_svr(prob,param,alpha,&si);
    break;
  }

  info("obj = %f, rho = %f\n",si.obj,si.rho);

  // output SVs

  std::vector<int32> SVIndex;

  int32 nSV = 0;
  int32 nBSV = 0;

  for  (int32 i = 0;  i < prob->l;  i++)
  {
    if  (fabs(alpha[i]) > 0)
    {
      ++nSV;
      SVIndex.push_back (i);
      if (prob->y[i] > 0)
      {
        if  (fabs (alpha[i]) >= si.upper_bound_p)
        {
          ++nBSV;
          BSVIndex.insert (prob->index[i]);
        }
      }
      else
      {
        if  (fabs(alpha[i]) >= si.upper_bound_n)
        {
          ++nBSV;
          BSVIndex.insert (prob->index[i]);
        }
      }
    }
  }

  info("nSV = %d, nBSV = %d\n",nSV,nBSV);

  double sum=0.0;
  std::vector<int32>::iterator it,it2;

  for (it = SVIndex.begin();  it < SVIndex.end();  it++)
  {
    for  (it2 = SVIndex.begin(); it2 < SVIndex.end();  it2++)
    {
      int32 k  = *it;
      int32 kk = *it2;

      double kvalue;

      if  (param->dimSelect > 0)
      {
        kvalue = Kernel::k_function_subspace (prob->x[k], 
          prob->x[kk], 
          *param, 
          param->featureWeight
          );
      }
      else
      {
        kvalue = Kernel::k_function(prob->x[k], prob->x[kk], *param);
      }

      sum+= prob->y[k]*prob->y[kk]*alpha[k]*alpha[kk]*kvalue;
    }
  }

  sum /= SVIndex.size();
  sum = sqrt(sum);

  for  (it = SVIndex.begin();  it < SVIndex.end();  it++)
  {
    alpha[*it] /= sum;
  }

  si.rho /= sum;

  decision_function f;

  f.alpha = alpha;
  f.rho   = si.rho;
  return f;
}  /* svm_train_one */





//
// Interface functions
//

svm_model* SVM233::svm_train (const svm_problem*    prob,
                              const svm_parameter*  param
                             )
{
  //svm_model *model = Malloc(svm_model,1);

  svm_model *model = new svm_model();
  //memset(model, 0, sizeof(svm_model)); - KNS
  model->param = *param;
  model->dim =0;

  if  (param->dimSelect > 0)
  {
    model->dim=param->dim;
    model->featureWeight = new double[model->dim];
    std::copy (param->featureWeight, 
               param->featureWeight + model->dim, 
               model->featureWeight
              );
  }

  if (param->svm_type == ONE_CLASS    ||
      param->svm_type == EPSILON_SVR  ||
      param->svm_type == NU_SVR
     )
  {
    // regression or one-class-svm
    model->nr_class = 2;
    model->label = NULL;
    model->nSV = NULL;
    model->sv_coef = Malloc(double *,1);
    decision_function  f = svm_train_one (prob, param, 0, 0, model->BSVIndex);
    model->rho = Malloc(double, 1);
    model->rho[0] = f.rho;

    int32 nSV = 0;
    int32 i;

    for  (i = 0;  i < prob->l;  i++)
      if  (fabs (f.alpha[i]) > 0) ++nSV;

    model->l  = nSV;
    model->SV = Malloc(svm_node *,nSV);

    //luo add
    model->numNonSV = prob->l - nSV;
    model->nonSVIndex = Malloc(int32, model->numNonSV);
    model->SVIndex=Malloc(int32, nSV);
    //model->rest=Malloc(svm_node *,model->numRest);
    //luo
    model->sv_coef[0] = Malloc (double, nSV);
    int32 j = 0;
    int32 jj=0;
    for  (i = 0;  i < prob->l;  i++)
    {
      if  (fabs (f.alpha[i]) > 0)
      {
        model->SVIndex[j]    = prob->index[i]; //luo add
        model->SV[j]         = prob->x[i];
        model->sv_coef[0][j] = f.alpha[i];
        ++j;
      }
      else
      {
        model->nonSVIndex[jj++]=prob->index[i]; //luo add
      }
    }

    free(f.alpha);
  }
  else
  {
    // classification
    // find out the number of classes
    int32 l = prob->l;
    int32 max_nr_class = 16;
    int32 nr_class = 0;
    int32 *label = Malloc(int32,max_nr_class);
    int32 *count = Malloc(int32,max_nr_class);
    int32 *index = Malloc(int32, l);

    bool  weHaveExampleNames = ((int32)prob->exampleNames.size () >= prob->l);

    int32 i;
    for  (i = 0;  i < l;  i++)
    {
      int32 this_label = (int32)prob->y[i];
      int32 j;
      for  (j = 0;  j < nr_class;  j++)
      {
        if  (this_label == label[j])
        {
          ++count[j];
          break;
        }
      }

      index[i] = j;
      if  (j == nr_class)
      {
        if  (nr_class == max_nr_class)
        {
          max_nr_class *= 2;
          label = (int32 *)realloc(label,max_nr_class*sizeof(int32));
          count = (int32 *)realloc(count,max_nr_class*sizeof(int32));
        }
        label[nr_class] = this_label;
        count[nr_class] = 1;
        ++nr_class;
      }
    }

    // group training data of the same class

    int32 *start = Malloc (int32, nr_class);
    start[0] = 0;
    for  (i = 1;  i < nr_class;  i++)
      start[i] = start[i-1] + count[i-1];


    svm_node **x = Malloc(svm_node *,l);
    double *W = NULL;
    if (prob->W != NULL)
      W = Malloc(double, l);
    std::vector<int32> reindex(l);

    for  (i = 0;  i < l;  i++)
    {
      x[start[index[i]]] = prob->x[i];
      reindex[start[index[i]]] = prob->index[i];
      if  (W != NULL)
        W[start[index[i]]] = prob->W[i];
      ++start[index[i]];
    }

    start[0] = 0;
    for  (i = 1;  i < nr_class;  i++)
      start[i] = start[i - 1]  + count[i - 1];

    // calculate weighted C

    double *weighted_C = Malloc(double, nr_class);
    for (i = 0;  i < nr_class;  i++)
      weighted_C[i] = param->C;

    for(i = 0;  i < param->nr_weight;  i++)
    {
      int32 j;

      for  (j = 0;  j < nr_class;  j++)
        if  (param->weight_label[i] == label[j])
          break;

      if  (j == nr_class)
        fprintf (stderr,"warning: class label %d specified in weight is not found\n", param->weight_label[i]);
      else
        weighted_C[j] *= param->weight[i];
    }

    // train n*(n-1)/2 models

    bool *nonzero = Malloc(bool,l);
    for(i=0;i<l;i++)
      nonzero[i] = false;

    decision_function *f = Malloc(decision_function,nr_class*(nr_class-1)/2);

    int32 p = 0;
    for(i=0;i<nr_class;i++)
    {
      for(int32 j=i+1;j<nr_class;j++)
      {
        svm_problem   sub_prob;
        //memset(&sub_prob, 0, sizeof(sub_prob));   // kak

        int32 si = start[i], sj = start[j];
        int32 ci = count[i], cj = count[j];
        sub_prob.l = ci+cj;
        sub_prob.x = Malloc(svm_node *,sub_prob.l);
        sub_prob.y = Malloc(double, sub_prob.l);
        if (W != NULL)
          sub_prob.W = Malloc(double, sub_prob.l);
        sub_prob.index = Malloc(int32, sub_prob.l);

        int32 k;
        for(k=0;k<ci;k++)
        {
          sub_prob.x[k] = x[si+k];
          sub_prob.y[k] = +1;
          if (W != NULL)
            sub_prob.W[k] = W[si+k];
          sub_prob.index[k]=reindex[si+k];  
        }
        for(k=0;k<cj;k++)
        {
          sub_prob.x[ci+k] = x[sj+k];
          sub_prob.y[ci+k] = -1;
          if (W != NULL)
            sub_prob.W[ci+k] = W[sj+k];
          sub_prob.index[ci+k]=reindex[sj+k];
        }

        if (W != NULL)
          f[p] = svm_train_one(&sub_prob, param);
        else
          f[p] = svm_train_one(&sub_prob,param,weighted_C[i],weighted_C[j],model->BSVIndex);

        //printf ("svm  Training Classes %d[%d] and %d[%d]\n",
        //  label[i], count[i],
        //  label[j], count[j]
        //  );

          fflush (stdout);

          // KNS - I believe this line is what was causing the training time to double
          //  f[p] = svm_train_one(&sub_prob,param,weighted_C[i],weighted_C[j],model->BSVIndex);

          for  (k = 0;  k < ci;  k++)
          {
            if(!nonzero[si+k] && fabs(f[p].alpha[k]) > 0)
              nonzero[si+k] = true;
          }

          for  (k = 0;  k < cj;  k++)
          {
            if  (!nonzero[sj+k] && fabs(f[p].alpha[ci+k]) > 0)
              nonzero[sj+k] = true;
          }

          free(sub_prob.x);
          free(sub_prob.y);
          if (sub_prob.W != NULL)
            free(sub_prob.W);
          free(sub_prob.index);
          ++p;
      }
    }
    // build output


    model->nr_class = nr_class;

    model->label = Malloc (int32, nr_class);
    for (i = 0;  i < nr_class;  i++)
      model->label[i] = label[i];

    model->rho = Malloc (double, nr_class*(nr_class-1)/2);
    for  (i = 0;  i < nr_class * (nr_class - 1) / 2;  i++)
      model->rho[i] = f[i].rho;

    int32 total_sv = 0;
    int32 *nz_count = Malloc (int32,nr_class);
    model->nSV    = Malloc (int32,nr_class);
    for  (i=0;  i < nr_class;  i++)
    {
      int32 nSV = 0;
      for  (int32 j = 0;  j < count[i];  j++)
      {
        if  (nonzero[start[i]+j])
        {  
          ++nSV;
          ++total_sv;
        }
        model->nSV[i] = nSV;
        nz_count[i]   = nSV;
      }
    }

    info("Total nSV = %d\n",total_sv);

    model->l = total_sv;
    model->SV = Malloc (svm_node *,total_sv);
    model->exampleNames.clear ();

    if  (weHaveExampleNames)
    {
      for  (int32 zed = 0;  zed < total_sv;  zed++)
        model->exampleNames.push_back ("");
    }
    
    
    //luo add
    int32 pp=0;
    model->numNonSV   = l - total_sv;
    model->SVIndex    = Malloc(int32, total_sv);
    model->nonSVIndex = Malloc(int32, model->numNonSV);

    //model->numRest = l - total_sv;
    //model->rest=Malloc(svm_node *, model->numRest);
    //luo

    p = 0;
    for  (i = 0;  i < l;  i++)
    {
      if  (nonzero[i])
      {
        model->SV[p] = x[i];
        model->SVIndex[p]=prob->index[i];//luo add
        if  (weHaveExampleNames)
          model->exampleNames[p] = prob->exampleNames[i];
        p++;
      }
      else
      {
        //model->rest[pp++]=x[i];
        model->nonSVIndex[pp] = prob->index[i];
        pp++;
      }
    }

    int32 *nz_start = Malloc (int32,nr_class);
    nz_start[0] = 0;
    for(i=1;i<nr_class;i++)
      nz_start[i] = nz_start[i-1]+nz_count[i-1];

    model->sv_coef = Malloc(double *,nr_class-1);
    for(i=0;i<nr_class-1;i++)
    {
      model->sv_coef[i] = Malloc(double,total_sv);
      for  (int32 zed = 0;  zed < total_sv;  zed++)
        model->sv_coef[i][zed] = 0.0;
    }

    p = 0;
    for  (i = 0;  i < nr_class;  i++)
      for  (int32 j = i + 1;  j < nr_class;  j++)
      {
        // classifier (i,j): coefficients with
        // i are in sv_coef[j-1][nz_start[i]...],
        // j are in sv_coef[i][nz_start[j]...]

        int32 si = start[i];
        int32 sj = start[j];
        int32 ci = count[i];
        int32 cj = count[j];

        int32 q = nz_start[i];
        int32 k;
        for (k = 0;  k < ci;  k++)
          if  (nonzero[si + k])
          {
            if  (model->sv_coef[j-1][q] != 0.0)
            {
              cerr << endl << endl
                   << "svm_train   ***ERROR***   Alpha Valude Being Overwritten." << endl
                   << endl;
            }
            model->sv_coef[j - 1][q++] = f[p].alpha[k];
          }
        q = nz_start[j];
        for(k=0;k<cj;k++)
        {
          if(nonzero[sj+k])
          {
            if  (model->sv_coef[i][q] != 0.0)
            {
              cerr << endl << endl
                   << "svm_train   ***ERROR***   Alpha Valude Being Overwritten." << endl
                   << endl;
            }
            model->sv_coef[i][q++] = f[p].alpha[ci + k];
          }
        }
        ++p;
      }

      free(label);
      free(count);
      free(index);
      free(start);
      if (W != NULL)
        free(W);
      free(x);
      free(weighted_C);
      free(nonzero);
      for(i=0;i<nr_class*(nr_class-1)/2;i++)
        free(f[i].alpha);
      free(f);
      free(nz_count);
      free(nz_start);

      // KNS - this was commecnted out in the pre-BR svm
      // svm_margin(model);

      model->kValueTable = new double[model->l];
  }

  //svm_margin (model);
  return model;
}  /* svm_train */






int32 svm_get_nr_class(const svm_model *model)
{
  return model->nr_class;
}



void svm_get_labels(const svm_model *model, int32* label)
{
  for  (int32 i = 0;  i < model->nr_class;  i++)
    label[i] = model->label[i];
}






const char *svm_type_table[] =
{
  "c_svc","nu_svc","one_class","epsilon_svr","nu_svr",NULL
};



const char *kernel_type_table[]=
{
  "linear","polynomial","rbf","sigmoid",NULL
};





int32   SVM233::svm_save_model (const char *model_file_name, const svm_model *model)
{
  FILE *fp = fopen (model_file_name, "w");
  if  (fp==NULL) return -1;

  const svm_parameter& param = model->param;

  fprintf (fp, "svm_type    %s\n", svm_type_table    [param.svm_type]);
  fprintf (fp, "kernel_type %s\n", kernel_type_table [param.kernel_type]);
  fprintf (fp, "dimSelect   %d\n", param.dimSelect);


  if  (param.kernel_type == POLY)
    fprintf(fp,"degree %g\n", param.degree);

  if  (param.kernel_type == POLY || param.kernel_type == RBF || param.kernel_type == SIGMOID)
    fprintf(fp,"gamma %g\n", param.gamma);

  if  (param.kernel_type == POLY || param.kernel_type == SIGMOID)
    fprintf(fp,"coef0 %g\n", param.coef0);

  int32 nr_class = model->nr_class;
  int32 l = model->l;
  fprintf  (fp, "nr_class %d\n", nr_class);
  fprintf  (fp, "total_sv %d\n",l);

  {
    fprintf(fp, "rho");
    for(int32 i=0;i<nr_class*(nr_class-1)/2;i++)
      fprintf(fp," %g",model->rho[i]);
    fprintf(fp, "\n");
  }

  if (model->label)
  {
    fprintf(fp, "label");
    for(int32 i=0;i<nr_class;i++)
      fprintf(fp," %d",model->label[i]);
    fprintf(fp, "\n");
  }

  if  (model->nSV)
  {
    fprintf(fp, "nr_sv");
    for(int32 i=0;i<nr_class;i++)
      fprintf(fp," %d",  model->nSV[i]);
    fprintf(fp, "\n");
  }

  fprintf(fp, "SV\n");
  const double * const *sv_coef = model->sv_coef;
  const svm_node * const *SV = model->SV;

  for (int32 i = 0;  i < l;  i++)
  {
    for  (int32 j = 0;  j < nr_class - 1;  j++)
      fprintf(fp, "%.16g ",sv_coef[j][i]);

    const svm_node *p = SV[i];
    while(p->index != -1)
    {
      fprintf(fp,"%d:%.8g ",p->index,p->value);
      p++;
    }
    fprintf(fp, "\n");
  }

  fclose (fp);
  return 0;
}




void  SVM233::Svm_Save_Model (ostream&         o, 
                              const svm_model* model
                             )
{
  const svm_parameter&  param = model->param;

  int32 nr_class = model->nr_class;
  int32 numberOfBinaryClassifiers = nr_class * (nr_class - 1) / 2;

  int32  origPrecision = o.precision ();
  o.precision (14);

  o << "<Svm233>"   << endl;

  o << "Parameters"    << "\t" << param.ToTabDelStr () << endl;
  o << "NumOfClasses"  << "\t" << nr_class  << endl;

  int32 totalNumSVs = model->l;
  o << "TotalNumOfSupportVectors" << "\t" << totalNumSVs    << endl;

  {
    o << "rho";
    for  (int32 i = 0;  i < numberOfBinaryClassifiers;  i++)
      o << "\t" << model->rho[i];
    o << endl;
  }

  if  (model->margin)
  {
    int32  oldPrecision = o.precision ();
    o.precision (9);
    o << "Margins";
    for  (int32 i = 0;  i < numberOfBinaryClassifiers;  i++)
      o << "\t" << model->margin[i];
    o << endl;
    o.precision (oldPrecision);
  }

  if (model->label)
  {
    o << "label";
    for  (int32 i = 0;  i < nr_class;  i++)
      o << "\t" << model->label[i];
    o << endl;
  }

  if  (model->nSV)
  {
    o << "nr_sv";
    for  (int32 i = 0;  i < nr_class;  i++)
      o << "\t" << model->nSV[i];
    o << endl;
  }

  const double * const *sv_coef = model->sv_coef;
  const svm_node * const *SV = model->SV;

  {
    // Determine total number of elememts
    int32  totalNumOfElements = 0;
    for  (int32 i = 0;  i < totalNumSVs;  i++)
    {
      const svm_node *p = SV[i];
      while  (p->index != -1)
      {
        totalNumOfElements++;
        p++;
      }

      totalNumOfElements++;  // Add one for terminating node
    }
    o << "TOTALNUMOFELEMENTS" << "\t" << totalNumOfElements << endl;
  }

  for  (int32 i = 0;  i < totalNumSVs;  i++)
  {
    if  ((int32)model->exampleNames.size () > i)
      o << "SuportVectorNamed"  << "\t" << model->exampleNames[i];
    else
      o << "SuportVector";

    o.precision (16);
    for (int32 j = 0;  j < nr_class - 1;  j++)
      o << "\t" << sv_coef[j][i];

    const svm_node *p = SV[i];
    while  (p->index != -1)
    {
      o << "\t" << p->index << ":" << p->value;
      p++;
    }

    o << endl;
  }

 
  o << "</Svm233>" << endl;

  o.precision (origPrecision);
 }  /* Svm_Save_Model */




struct svm_model*  SVM233::Svm_Load_Model (istream&  f,
                                           RunLog&   log
                                          )
{
  svm_model*  model = new svm_model ();

  char  buff[100000];

  int32  numOfClasses       = -1;
  int32  numSVsLoaded       = 0;
  int32  numElementsLoaded  = 0;
  int32  totalNumSVs        = -1;
  int32  totalNumOfElements = -1;
  svm_node*  x_space = NULL;


  bool  validFormat = true;

  KKStr  line (1024);  // Preallocating to at least 1024 chracters int32.

  // Get first non blank line.  It had better contain "<Smv239>"  otherwise we will consider this
  // an invalid Training Model.
  {
    while  (f.getline (buff, sizeof (buff)))
    {
      line = buff;
      line.TrimLeft ("\n\r\t ");
      line.TrimRight ("\n\r\t ");

      if  ((line.Len () > 0)  &&  (line.SubStrPart (0, 1) != "//"))
      {
        // We have our first 'non blank'  'non commented line'
        break;
      }
    }
  }

  if  (!line.EqualIgnoreCase ("<Svm233>"))
  {
    // We do not have a valid SVM239 model.   We will return a NULL
    log.Level (-1) << endl << endl
      << "SVM233::Svm_Load_Model    ***ERROR***    The '<Svm233>'  header is missing.  Not a valid model." << endl
      << endl;
    delete  model;
    model = NULL;
    return NULL;
  }

  /**
   @todo Modify SVM233::Svm_Load_Model to read one token at a time rather than a whol eline at a time.
   */

  while  (f.getline (buff, sizeof (buff)))
  {
    KKStr  line (buff);
    line.TrimLeft ();
    line.TrimRight ();

    if  (line == "<SvmMachine>")
      continue;

    if  (line == "</Svm233>")
      break;

    KKStr  lineName = line.ExtractToken2 ("\t");
    lineName.Upper ();

    if  (lineName == "PARAMETERS")
    {
      model->param.ParseTabDelStr (line);
    }

    else if  (lineName == "NUMOFCLASSES")
    {
      numOfClasses = line.ExtractTokenInt ("\t");
      model->nr_class = numOfClasses;
    }

    else if  (lineName == "TOTALNUMOFSUPPORTVECTORS")
    {
      totalNumSVs = line.ExtractTokenInt ("\t");
      model->l = totalNumSVs;
    }

    else if  (lineName == "RHO")
    {
      int32 n = numOfClasses * (numOfClasses - 1) / 2;
      model->rho = Malloc (double, n);
      for (int32 i = 0;  i < n;  i++)
        model->rho[i] = line.ExtractTokenDouble ("\t");
    }

    else if  (lineName == "LABEL")
    {
      model->label = Malloc (int32, numOfClasses);
      for  (int32 i=0;  i < numOfClasses;  i++)
        model->label[i] = line.ExtractTokenInt ("\t");
    }
  

    else if  (lineName == "NR_SV")
    {
      model->nSV = Malloc(int32, numOfClasses);
      for  (int32 i = 0;  i < numOfClasses;  i++)
        model->nSV[i] = line.ExtractTokenInt ("\t");
    }


    else if  (lineName.EqualIgnoreCase ("Margins"))
    {
      int32 n = numOfClasses * (numOfClasses - 1) / 2;
      delete  model->margin;
      model->margin = new double[n];
      for (int32 i = 0;  i < n;  i++)
        model->margin[i] = line.ExtractTokenDouble ("\t");
    }

    else if  (lineName == "TOTALNUMOFELEMENTS")
    {
      totalNumOfElements = line.ExtractTokenInt ("\t");

      int32 m = model->nr_class - 1;
      int32 l = model->l;
      model->sv_coef = Malloc (double*, m);

      for (int32 i = 0;  i < m;  i++)
        model->sv_coef[i] = Malloc (double, l);
      
      model->SV = Malloc (svm_node*, l);
      x_space = new svm_node[totalNumOfElements];
      model->xSpace      = x_space;
      model->weOwnXspace = true;   // 'model'  owns 'x_space'  and will be responsable for deleting it.
    }

    else if  (lineName.EqualIgnoreCase ("SUPORTVECTOR")  ||  lineName.EqualIgnoreCase ("SuportVectorNamed"))
    {
      if  (numSVsLoaded >= totalNumSVs)
      {
        log.Level (-1) << endl << endl << endl
             << "SvmLoadModel     **** ERROR ****         There are more 'SupportVector' lines defined that there should be." << endl
             << endl;
        validFormat = false;
        break;
      }

      if  (totalNumOfElements < 1)
      {
        log.Level (-1) << endl << endl << endl
             << "SvmLoadModel     **** ERROR ****           'totalNumOfElements'  was not defined." << endl
             << endl;
        validFormat = false;
        break;
      }

      if  (lineName.EqualIgnoreCase ("SuportVectorNamed"))
      {
        // this Support Vector has a mame to it.
        model->exampleNames.push_back (line.ExtractToken2 ("\t"));
      }

      for (int32 j = 0;  j < numOfClasses - 1;  j++)
        model->sv_coef[j][numSVsLoaded] = line.ExtractTokenDouble ("\t");

      model->SV[numSVsLoaded] = &(x_space[numElementsLoaded]);
      while  ((!line.Empty ())  &&  (numElementsLoaded < (totalNumOfElements - 1)))
      {
        x_space[numElementsLoaded].index = line.ExtractTokenInt (":");
        x_space[numElementsLoaded].value = line.ExtractTokenDouble ("\t");
        numElementsLoaded++;
      }

      if  (numElementsLoaded >= totalNumOfElements)
      {
        log.Level (-1) << endl << endl << endl
             << "SvmLoadModel     **** ERROR ****      'numElementsLoaded'  is greater than what was defined by 'totalNumOfElements'." << endl
             << endl;
        validFormat = false;
        break;
      }

      x_space[numElementsLoaded].index = -1;
      x_space[numElementsLoaded].value = 0.0;
      numElementsLoaded++;

      numSVsLoaded++;
    }
  }

  if  (validFormat)
  {
    if  (numSVsLoaded != totalNumSVs)
    {
      log.Level (-1) << endl << endl << endl
           << "SvmLoadModel     ***ERROR***      numSVsLoaded[" << numSVsLoaded << "] does not agree with totalNumSVs[" << totalNumSVs << "]" << endl
           << endl;
      validFormat = false;
    }

    if  (numElementsLoaded != totalNumOfElements)
    {
      log.Level (-1) << endl << endl << endl
           << "SvmLoadModel     ***ERROR***      numElementsLoaded[" << numElementsLoaded << "] does not agree with  totalNumOfElements[" << totalNumOfElements << "]" << endl
           << endl;
      validFormat = false;
    }

    else if  (numOfClasses < 1)
    {
      log.Level (-1) << endl << endl << endl
           << "SvmLoadModel     ***ERROR***      numOfClasses  was not specified." << endl
           << endl;
      validFormat = false;
    }
  }

  if  (!validFormat)
  {
    if  (model)
    {
      model->Dispose ();
      delete  model;
      model = NULL;
    }
  }

  if  (model)
    model->kValueTable = new double[model->l];

  return  model;
}  /* Svm_Load_Model */





svm_model*  SVM233::svm_load_model (const char *model_file_name)
{
  FILE *fp = fopen (model_file_name, "rb");
  if  (fp == NULL) 
    return NULL;

  // read parameters

  svm_model *model = new svm_model ();
  svm_parameter& param = model->param;
  model->rho   = NULL;
  model->label = NULL;
  model->nSV   = NULL;

  char cmd[82];
  while(1)
  {
    fscanf(fp,"%80s",cmd);

    if  (KKStr::StrEqualNoCase (cmd, "svm_type"))
    {
      fscanf(fp,"%80s",cmd);
      int32 i;
      for (i=0;svm_type_table[i];i++)
      {
        if  (KKStr::StrEqualNoCase (svm_type_table[i], cmd))
        {
          param.svm_type=i;
          break;
        }
      }

      if  (svm_type_table[i] == NULL)
      {
        fprintf (stderr,"unknown svm type.\n");
        free (model->rho);
        free (model->label);
        free (model->nSV);
        delete model;
        return NULL;
      }
    }

    else if  (KKStr::StrEqualNoCase (cmd, "kernel_type"))
    {
      fscanf(fp,"%80s",cmd);
      int32 i;
      for  (i = 0;  kernel_type_table[i];  i++)
      {
        if  (KKStr::StrEqualNoCase (kernel_type_table[i], cmd))
        {
          param.kernel_type=i;
          break;
        }
      }

      if  (kernel_type_table[i] == NULL)
      {
        fprintf(stderr,"unknown kernel function.\n");
        free(model->rho);
        free(model->label);
        free(model->nSV);
        delete model;
        return NULL;
      }
    }

    else if  (KKStr::StrEqualNoCase (cmd, "dimSelect"))
      fscanf (fp, "%d", &param.dimSelect);

    else if  (KKStr::StrEqualNoCase (cmd, "degree"))
      fscanf(fp,"%lf",&param.degree);

    else if  (KKStr::StrEqualNoCase (cmd, "gamma"))
      fscanf(fp,"%lf",&param.gamma);

    else if  (KKStr::StrEqualNoCase (cmd, "coef0"))
      fscanf(fp,"%lf",&param.coef0);

    else if  (KKStr::StrEqualNoCase (cmd, "nr_class"))
      fscanf(fp,"%d",&model->nr_class);

    else if  (KKStr::StrEqualNoCase (cmd, "total_sv"))
      fscanf(fp,"%d",&model->l);

    else if  (KKStr::StrEqualNoCase (cmd, "rho"))
    {
      int32 n = model->nr_class * (model->nr_class - 1) / 2;
      model->rho = Malloc (double, n);
      for  (int32 i=0; i < n;  i++)
        fscanf(fp,"%lf",&model->rho[i]);
    }

    else if  (KKStr::StrEqualNoCase (cmd, "label"))
    {
      int32 n = model->nr_class;
      model->label = Malloc(int32,n);
      for(int32 i=0;i<n;i++)
        fscanf(fp,"%d",&model->label[i]);
    }

    else if  (KKStr::StrEqualNoCase (cmd, "nr_sv"))
    {
      int32 n = model->nr_class;
      model->nSV = Malloc(int32,n);
      for  (int32 i = 0;  i < n;  i++)
        fscanf(fp, "%d", &model->nSV[i]);
    }

    else if  (KKStr::StrEqualNoCase (cmd, "SV"))
    {
      while(1)
      {
        int32 c = getc (fp);
        if  (c == EOF || c == '\n') break;
      }
      break;
    }

    else
    {
      fprintf (stderr,"unknown text in model file\n");
      free (model->rho);
      free (model->label);
      free (model->nSV);
      delete model;
      return NULL;
    }
  }

  // read sv_coef and SV

  int32 elements = 0;
  int32 pos = ftell(fp);

  while(1)
  {
    int32 c = fgetc(fp);
    switch(c)
    {
    case '\n':
      // count the '-1' element
    case ':':
      ++elements;
      break;
    case EOF:
      goto out;
    default:
      ;
    }
  }

out:
  fseek(fp, pos, SEEK_SET);

  int32 m = model->nr_class - 1;
  int32 l = model->l;
  model->sv_coef = Malloc(double *,m);
  int32 i;
  for  (i = 0;  i < m;  i++)
    model->sv_coef[i] = Malloc(double,l);

  model->SV = Malloc(svm_node*,l);
  svm_node *x_space = Malloc(svm_node,elements);

  int32 j=0;
  for  (i = 0;  i < l;  i++)
  {
    model->SV[i] = &x_space[j];
    for (int32 k = 0;  k < m;  k++)
      fscanf (fp, "%lf", &model->sv_coef[k][i]);
    while  (1)
    {
      int32 c;
      do {
        c = getc(fp);
        if(c=='\n') goto out2;
      } while(isspace(c));
      ungetc(c,fp);
      fscanf (fp, "%d:%lf", (x_space[j].index), (x_space[j].value));
      ++j;
      }
out2:
    x_space[j++].index = -1;
  }

  fclose(fp);


  // kak, 2006-Jul-03, Following array is needed during prediction, it is 
  //                   created in svm_train so need to do here also.
  model->kValueTable = new double[model->l];

  return model;
}  /* svm_load_model */




/****************************************************************************************/
double  SVM233::svm_predict (const svm_model*      model,
                             const svm_node*       x,
                             std::vector<double>&  dist,
                             std::vector<int32>&   winners,
                             int32                 excludeSupportVectorIDX  /*!<  Specify index of a S/V to remove from computation. */
                            )
{
  int32 dimSelect = model->param.dimSelect;

  winners.erase (winners.begin (), winners.end ());

  if  (model->param.svm_type == ONE_CLASS   ||
       model->param.svm_type == EPSILON_SVR ||
       model->param.svm_type == NU_SVR
      )
  {
    double*  sv_coef = model->sv_coef[0];
    double   sum = 0;

    for  (int32 i=0; i < model->l; i++)
    {
      if  (i == excludeSupportVectorIDX)
        continue;

      if  (dimSelect>0)
        sum += sv_coef[i] * Kernel::k_function_subspace (x, model->SV[i], model->param, model->featureWeight);
      else
        sum += sv_coef[i] * Kernel::k_function (x, model->SV[i], model->param);
    }

    sum -= model->rho[0];

    double  returnVal = 0.0;

    if  (model->param.svm_type == ONE_CLASS)
      returnVal = (sum > 0) ? 1:-1;
    else
      returnVal = sum;

    winners.push_back ((int32)returnVal);
    return  returnVal;
  }
  else
  {
    int32 i;
    int32 nr_class = model->nr_class;

    std::vector<int32>   voteTable (nr_class * nr_class, 0);
    int32 l = model->l;


    double*  kvalue = model->kValueTable;

    // Precompute S/V's for all classes.
    for  (i=0; i<l; i++)
    {
      if  (i == excludeSupportVectorIDX)
      {
        kvalue[i] = 0.0;
      }
      else
      {
        if  (dimSelect > 0)
          kvalue[i] = Kernel::k_function_subspace (x, model->SV[i], model->param, model->featureWeight);
        else
          kvalue[i] = Kernel::k_function (x, model->SV[i], model->param);
      }
    }


    // 'start'  will be built to point to the beginning of the list of S'V's for each class.
    int32 *start = Malloc (int32, nr_class);
    start[0] = 0;
    for  (i = 1;  i < nr_class;  i++)
      start[i] = start[i-1] + model->nSV[i-1];

    int32 *vote = Malloc (int32, nr_class);

    for  (i = 0;  i < nr_class;  i++)
      vote[i] = 0;

    int32 p = 0;

    std::vector<double>  distTable (nr_class * nr_class, 0);

    for  (i = 0;  i < nr_class;  i++)
    {
      for  (int32 j = i + 1;  j < nr_class;  j++)
      {
        double sum = 0;
        int32 si = start[i];
        int32 sj = start[j];
        int32 ci = model->nSV[i];
        int32 cj = model->nSV[j];

        int32 k;
        double *coef1 = model->sv_coef[j-1];
        double *coef2 = model->sv_coef[i];

        for  (k = 0;  k < ci;  k++)
          sum += coef1[si + k] * kvalue[si + k];

        for  (k = 0;  k < cj;  k++)
          sum += coef2[sj + k] * kvalue[sj + k];

        sum -= model->rho[p];

        if  (sum > 0)
        {
          voteTable[i * nr_class + j]++; // i vs. j
          ++vote[i];
        }
        else
        {
          voteTable[j * nr_class + i]++; // j vs. i
          ++vote[j];
        }

        int32 row = model->label[i];
        int32 col = model->label[j];

        distTable[row * nr_class + col] = sum;
        distTable[col * nr_class + row] = (-1.0 * sum);

        //dist[p] = sum;
        p++;
      }
    }

    p=0;

    {  // kk:  jun-06-2003

      for (int32 i = 0;  i < nr_class - 1; i++)
      {
        for (int32 j=i+1; j< nr_class; j++)
        {
          dist[p++] = distTable[i * nr_class + j];
        }
      }

    }  // kk:  jun-06-2003

    winners.erase (winners.begin (), winners.end ());
    int32 vote_max_idx = 0;
    int32 winningAmt   = vote[0];
    winners.push_back (model->label[0]);

    for  (i = 1;  i < nr_class;  i++)
    {
      if  (vote[i] > winningAmt)
      {
        winners.erase (winners.begin (), winners.end ());
        vote_max_idx = i;
        winningAmt   = vote[i];
        winners.push_back (model->label[i]);
      }
      else if  (vote[i] == winningAmt)
      {
        winners.push_back (model->label[i]);
      }
    }

    free (start);
    free (vote);

    int32  winningLabel = model->label[vote_max_idx];

    return  winningLabel;
  }
}  /* svm_predict */




/****************************************************************************************/
double  SVM233::svm_predictTwoClasses (const svm_model*    model,
                                       const svm_node*     x,
                                       double&             dist,
                                       int32                 excludeSupportVectorIDX  /*!<  Specify index of a S/V to remove from computation. */
                                      )
{
  int32  winner = 0;

  int32 dimSelect = model->param.dimSelect;

  if  (model->param.svm_type == ONE_CLASS   ||
       model->param.svm_type == EPSILON_SVR ||
       model->param.svm_type == NU_SVR
      )
  {
    double*  sv_coef = model->sv_coef[0];
    double   sum = 0;

    for  (int32 i=0; i < model->l; i++)
    {
      if  (i == excludeSupportVectorIDX)
        continue;

      if  (dimSelect > 0)
        sum += sv_coef[i] * Kernel::k_function_subspace (x, model->SV[i],model->param, model->featureWeight);
      else
        sum += sv_coef[i] * Kernel::k_function (x, model->SV[i], model->param);
    }

    sum -= model->rho[0];

    double  returnVal = 0.0;

    if  (model->param.svm_type == ONE_CLASS)
      returnVal = (sum > 0) ? 1:-1;
    else
      returnVal = sum;

    return  returnVal;
  }
  else
  {
    int32 i;

    int32 nr_class = model->nr_class;
    if  (nr_class != 2)
    {
      cerr << std::endl
           << "svm_predictTwoClasses  ***ERROR***  nr_class[" << nr_class << "] != 2" << std::endl
           << std::endl;
      exit (-1);
    }

    int32 l = model->l;
    //double *kvalue = Malloc (double, l);

    double* kvalue = model->kValueTable;

    if  (dimSelect > 0)
    {
      for  (i = 0;  i < l;  i++)
      {
        if  (i == excludeSupportVectorIDX)
          kvalue[i] = 0.0;
        else
          kvalue[i] = Kernel::k_function_subspace (x, model->SV[i], model->param, model->featureWeight);
      }
    }
    else
    {
      for  (i = 0;  i < l;  i++)
      {
        if  (i == excludeSupportVectorIDX)
          kvalue[i] = 0.0;
        else
          kvalue[i] = Kernel::k_function (x, model->SV[i], model->param);
      }
    }

    int32 start[2];
    start[0] = 0;
    for  (i = 1;  i < nr_class;  i++)
      start[i] = start[i-1] + model->nSV[i-1];

    int32 p = 0;

    i = 0;
    int32 j = 1;
    {
      double sum = 0;
      int32 si = start[i];
      int32 sj = start[j];
      int32 ci = model->nSV[i];
      int32 cj = model->nSV[j];

      int32 k;
      double *coef1 = model->sv_coef[j-1];
      double *coef2 = model->sv_coef[i];

      for  (k = 0;  k < ci;  k++)
        sum += coef1[si + k] * kvalue[si + k];

      for  (k = 0;  k < cj;  k++)
        sum += coef2[sj + k] * kvalue[sj + k];

      sum -= model->rho[p];

      if  (sum > 0)
        winner = 0;
      else
        winner = 1;

      dist = sum;

      //if  (model->margin)
      //{
      //  if  (model->margin[0] != 0.0)
      //    dist = dist / model->margin[0];
      //}
    }

    p=0;

    double  winningLabel = (double)model->label[winner];

    return  winningLabel;
  }
}  /* svm_predictTwoClasses */









/****************************************************************************************/
svm_problem*  SVM233::svm_BuildProbFromTwoClassModel  (const svm_model*    model,
                                                       int32                 excludeSupportVectorIDX  /*!<  Specify index of a S/V to remove from computation. */
                                                      )
{
  if  (model->param.svm_type == ONE_CLASS   ||
       model->param.svm_type == EPSILON_SVR ||
       model->param.svm_type == NU_SVR
      )
  {
    return NULL;
  }

  int32 nr_class = model->nr_class;
  if  (nr_class != 2)
  {
    printf ("\n\n\n svm_predictTwoClasses  *** ERROR ***  NumOf Classes != 2\n\n");
    return NULL;
  }

  if  (model->dim > 0)
  {
    cerr << std::endl
         << "SVM233::svm_BuildProbFromTwoClassModel   ***ERROR***    model->dim[" << model->dim << "] > 0" << std::endl
         << std::endl;
    return NULL;
  }


  int32 l = model->l;
  int32  numSVsFirstClass = model->nSV[0];

  svm_problem*  newProb = new svm_problem ();
  newProb->weOwnContents = true;
  newProb->l = l;
  if  ((excludeSupportVectorIDX >= 0)  &&  (excludeSupportVectorIDX < l))
     (newProb->l)--;

  newProb->y     = new double[newProb->l];
  newProb->x     = new svm_node*[newProb->l];
  newProb->index = new int32[newProb->l];

  int32  newIDX = 0;
  for (int32 svIDX = 0;  svIDX < l;  svIDX++)
  {
    if  (svIDX == excludeSupportVectorIDX)
      continue;

    if  (svIDX < numSVsFirstClass)
      newProb->y[newIDX] = 0;
    else
      newProb->y[newIDX] = 1;

    newProb->index[newIDX] = newIDX;
    newProb->exampleNames.push_back (model->exampleNames[svIDX]);

    {
      int32  numElements = 0;
      svm_node* nextNode = model->SV[svIDX];
      while  (nextNode->index != -1)
      {
        numElements++;
        nextNode++;
      }
      numElements++;

      newProb->x[newIDX] = new svm_node[numElements];

      for  (int32  fnIDX = 0;  fnIDX < numElements;  fnIDX++)
      {
        newProb->x[newIDX][fnIDX].index = model->SV[svIDX][fnIDX].index;
        newProb->x[newIDX][fnIDX].value = model->SV[svIDX][fnIDX].value;
      }
    }

    newIDX++;
  }

  return  newProb;
}  /* svm_BuildProbFromTwoClassModel */







void  SVM233::svm_destroy_model  (svm_model*  model)
{
  model->Dispose ();
  delete model;
}



const char*  SVM233::svm_check_parameter (const svm_problem *prob, const svm_parameter *param)
{
  // svm_type

  int32 svm_type = param->svm_type;
  if  (svm_type != C_SVC &&
       svm_type != NU_SVC &&
       svm_type != ONE_CLASS &&
       svm_type != EPSILON_SVR &&
       svm_type != NU_SVR
      )
   return "unknown svm type";

  // kernel_type

  int32 kernel_type = param->kernel_type;
  if  (kernel_type != LINEAR &&
       kernel_type != POLY &&
       kernel_type != RBF &&
       kernel_type != SIGMOID
      )
    return "unknown kernel type";

  // cache_size,eps,C,nu,p,shrinking

  if(param->cache_size <= 0)
    return "cache_size <= 0";

  if(param->eps <= 0)
    return "eps <= 0";

  if(svm_type == C_SVC ||
    svm_type == EPSILON_SVR ||
    svm_type == NU_SVR)
    if(param->C <= 0)
      return "C <= 0";

  if(svm_type == NU_SVC ||
    svm_type == ONE_CLASS ||
    svm_type == NU_SVR)
    if(param->nu < 0 || param->nu > 1)
      return "nu < 0 or nu > 1";

  if(svm_type == EPSILON_SVR)
    if(param->p < 0)
      return "p < 0";

  if(param->shrinking != 0 &&
    param->shrinking != 1)
    return "shrinking != 0 and shrinking != 1";


  // check whether nu-svc is feasible

  if(svm_type == NU_SVC)
  {
    int32 l = prob->l;
    int32 max_nr_class = 16;
    int32 nr_class = 0;
    int32 *label = Malloc(int32,max_nr_class);
    int32 *count = Malloc(int32,max_nr_class);

    int32 i;
    for(i=0;i<l;i++)
    {
      int32 this_label = (int32)prob->y[i];
      int32 j;
      for(j=0;j<nr_class;j++)
        if(this_label == label[j])
        {
          ++count[j];
          break;
        }
        if(j == nr_class)
        {
          if(nr_class == max_nr_class)
          {
            max_nr_class *= 2;
            label = (int32 *)realloc(label,max_nr_class*sizeof(int32));
            count = (int32 *)realloc(count,max_nr_class*sizeof(int32));
          }
          label[nr_class] = this_label;
          count[nr_class] = 1;
          ++nr_class;
        }
    }

    for(i=0;i<nr_class;i++)
    {
      int32 n1 = count[i];
      for(int32 j=i+1;j<nr_class;j++)
      {
        int32 n2 = count[j];
        if(param->nu*(n1+n2)/2 > min(n1,n2))
        {
          free(label);
          free(count);
          return "specified nu is infeasible";
        }
      }
    }
  }

  return NULL;
}  /* svm_check_parameter */




//luo add compute the distance of the data to the decision boundary, given the
//model and the input, Now it is only work for the binary case.
/*
double SVM233::svm_dist (const svm_model*  model, 
                         const svm_node*   x
                        )
{
  if (model->param.svm_type == ONE_CLASS ||
    model->param.svm_type == EPSILON_SVR ||
    model->param.svm_type == NU_SVR)
  {
    double *sv_coef = model->sv_coef[0];
    double sum = 0;
    for(int32 i=0;i<model->l;i++)
      sum += sv_coef[i] * Kernel::k_function(x,model->SV[i],model->param);
    sum -= model->rho[0];
    if(model->param.svm_type == ONE_CLASS)
      return (sum>0)?1:-1;
    else
      return sum;
  }
  else
  {
    int32 i;
    int32 nr_class = model->nr_class;
    int32 l = model->l;

    double *kvalue = Malloc(double,l);
    for(i=0;i<l;i++)
      kvalue[i] = Kernel::k_function(x,model->SV[i],model->param);

    int32 *start = Malloc(int32,nr_class);
    start[0] = 0;
    for(i=1;i<nr_class;i++)
      start[i] = start[i-1]+model->nSV[i-1];

    int32 *vote = Malloc(int32,nr_class);
    for(i=0;i<nr_class;i++)
      vote[i] = 0;
    int32 p=0;

    //luo add
    double distance=9999;

    for(i=0;i<nr_class;i++)
    {
      for(int32 j=i+1;j<nr_class;j++)
      {
        double sum = 0;
        int32 si = start[i];
        int32 sj = start[j];
        int32 ci = model->nSV[i];
        int32 cj = model->nSV[j];

        int32 k;
        double *coef1 = model->sv_coef[j-1];
        double *coef2 = model->sv_coef[i];
        for(k=0;k<ci;k++)
          sum += coef1[si+k] * kvalue[si+k];
        for(k=0;k<cj;k++)
          sum += coef2[sj+k] * kvalue[sj+k];
        sum -= model->rho[p];
        sum /= model->margin[p];

        distance=sum;

        if(sum > 0)
          ++vote[i];
        else
          ++vote[j];

        p++;

      }


      free(kvalue);
      free(start);
      free(vote);
      //luo add
      assert(distance!=9999);
      return distance;
    }
  }
}
//luo
*/




void  SVM233::svm_margin (svm_model *model)
//modify: model
//effects: if classification, model->margin[nr_class*(nr_class-1)/2] is computed
//      else if regression, doing nothing
{
  //int32 numClass=model->param.numClass;

  assert (model);

  if(model->param.svm_type == ONE_CLASS ||
    model->param.svm_type == EPSILON_SVR ||
    model->param.svm_type == NU_SVR)
  {
    /*
    double *sv_coef = model->sv_coef[0];
    double sum = 0;
    for(int32 i=0;i<model->l;i++)
    sum += sv_coef[i] * Kernel::k_function(x,model->SV[i],model->param);
    sum -= model->rho[0];
    if(model->param.svm_type == ONE_CLASS)
    return (sum>0)?1:-1;
    else
    return sum;
    */
  }
  else
  {
    int32 i;
    int32 nr_class = model->nr_class;

    model->margin = new double[nr_class * (nr_class - 1) / 2];

    std::vector<int32> start(nr_class,0);
    start[0] = 0;
    for(i=1;i<nr_class;i++)
      start[i] = start[i-1]+model->nSV[i-1];

    int32 p=0;
    for  (i = 0;  i < nr_class;  i++)
      for  (int32 j = i + 1;  j < nr_class;  j++)
      {
        double sum = 0; // computer the L of the K since it is symetric
        double trace=0; // compute the trace

        int32 si = start[i];
        int32 sj = start[j];
        int32 ci = model->nSV[i];
        int32 cj = model->nSV[j];

        int32 k;
        double *coef1 = model->sv_coef[j-1];
        double *coef2 = model->sv_coef[i];

        for  (k = 0;  k < ci;  k++)
          //y[k]=+1;
        {
          double kvalue = Kernel::k_function(model->SV[si+k],model->SV[si+k],model->param);
          trace += coef1[si+k]*coef1[si+k]*kvalue;

          for(int32 kk=k+1; kk<ci; kk++)
            //y[kk]=+1 and y[k]*y[kk]=+1;
          {
            kvalue = Kernel::k_function(model->SV[si+k],model->SV[si+kk],model->param);
            sum+= coef1[si+k]*coef1[si+kk]*kvalue;
          }
          //sum += coef1[si+k] * kvalue[si+k];

          {  // kk:  jun-06-2003
            for(int32 kk=0; kk<cj; kk++)
              //y[kk]=-1 and y[k]*y[kk]=-1
            {
              kvalue = Kernel::k_function(model->SV[si+k],model->SV[sj+kk],model->param);

              // kk  2010-06-13   I believe that since the coef already reflects the label(sign) of the training 
              //                  example that the following line should be adding not subtracting.
              // sum -= coef1[si+k]*coef2[sj+kk]*kvalue;
              sum += coef1[si+k]*coef2[sj+kk]*kvalue;
            }
          }  // kk:  jun-06-2003

        }

        for (k=0; k<cj-1; k++)
          //y[k]=-1
        {
          double kvalue = Kernel::k_function(model->SV[sj+k],model->SV[sj+k],model->param);
          trace += coef2[sj + k] * coef2[sj + k] * kvalue;

          for(int32 kk=k+1; kk<cj; kk++)
            //y[kk]=-1 and y[k]*y[kk]=+1
          {
            kvalue = Kernel::k_function(model->SV[sj+k],model->SV[sj+kk],model->param);

            // kk 2010-06-13
            // I suspect this next line should be subtracting but I am not doing anything 
            // about it at this point until I understand.
            sum += coef2[sj + k] * coef2[sj + kk] * kvalue;
          }
        }

        double marginSqr = 2 * sum + trace;
        if  (marginSqr <= 0.0)
        {
          cerr << endl << endl 
            << "SVM233::svm_margin   ***ERROR***    the computed margin <= 0.0.   This is very bad." << endl
            << endl;
          marginSqr = 0.1;
        }



        model->margin[p] = sqrt(marginSqr);
        p++;
      }
  }
}




/**
 *@brief  Extract Suport Vector statistics .
 *@param[in]   model         Traing Support Vector Machine.
 *@param[out]  numSVs        The number of training examples selected as Support Vectors.
 *@param[out]  totalNumSVs   Total number of SVs used by all the binary classifiers.
 */
void  SVM233::svm_GetSupportVectorStatistics (const struct svm_model*  model,
                                              int32&                     numSVs,         // The number of training examp
                                              int32&                     totalNumSVs
                                             )
{
  numSVs      = 0;
  totalNumSVs = 0;

  if  (model == NULL)
  {
    KKStr errMsg = "svm_GetupportVectorsStatistics  ***ERROR***   (model == NULL).   Model was not defined.";
    cerr << endl << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }

  if  (model->param.svm_type == ONE_CLASS   ||
       model->param.svm_type == EPSILON_SVR ||
       model->param.svm_type == NU_SVR
      )
  {
    cerr << endl << endl
      << "svm_GetupportVectorsStatistics   ***ERROR***   This function does not support for SVM Type[" << model->param.svm_type << "]" << endl
      << endl;
    return;
  }

  numSVs = model->l;

  {
    int32 i;
    int32 nr_class = model->nr_class;

    int32 l = model->l;


    // 'start'  will be built to point to the beginning of the list of S'V's for each class.
    int32*  start = new int32[nr_class];
    start[0] = 0;
    for  (i = 1;  i < nr_class;  i++)
      start[i] = start[i-1] + model->nSV[i-1];


    for  (i = 0;  i < nr_class;  i++)
    {
      for  (int32 j = i + 1;  j < nr_class;  j++)
      {
        double sum = 0;
        int32 si = start[i];
        int32 sj = start[j];
        int32 ci = model->nSV[i];
        int32 cj = model->nSV[j];

        int32 k;
        double *coef1 = model->sv_coef[j-1];
        double *coef2 = model->sv_coef[i];

        for  (k = 0;  k < ci;  k++)
          if  (coef1[si + k] != 0.0)
            totalNumSVs++;

        for  (k = 0;  k < cj;  k++)
          if  (coef2[sj + k] != 0.0)
            totalNumSVs++;
      }
    }

    delete  start;
    start = NULL;
  }

  return;
}  /* svm_GetupportVectorsStatistic */

