#include "FirstIncludes.h"
#include <stdlib.h>
#include <memory>
#include <math.h>

#include <complex>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "MemoryDebug.h"
using namespace std;

#if  defined(FFTW_AVAILABLE)
#endif

#include "KKBaseTypes.h"
#include "GoalKeeper.h"
#include "kku_fftw.h"
using namespace KKB;


/*
fftw_plan_class::fftw_plan_class (kkint32           _n,
                                  fftw_direction  _dir,
                                  kkint32           _flags
                                 ):
   n     (_n),
   dir   (_dir),
   flags (_flags)
{
}



fftwnd_plan_class::fftwnd_plan_class (kkint32           _nx,
                                      kkint32           _ny, 
                                      fftw_direction  _dir,
		                                  kkint32           _flags
                                     ):
   nx    (_nx),
   ny    (_ny),
   dir   (_dir),
   flags (_flags)
{
}



fftw_plan  KKB::fftw_create_plan (kkint32           n, 
                                  fftw_direction  dir, 
                                  kkint32           flags
                                 )
{
  return  new fftw_plan_class (n, dir, flags);
}



void  KKB::fftw_destroy_plan (fftw_plan  plan)
{
  delete  plan;
  plan = NULL;
}



fftwnd_plan  KKB::fftw2d_create_plan (kkint32           nx, 
                                      kkint32           ny, 
                                      fftw_direction  dir,
				                              kkint32           flags
                                     )
{
  return new fftwnd_plan_class (nx, ny, dir, flags);
}  // fftw2d_create_plan



void  KKB::fftwnd_destroy_plan (fftwnd_plan  plan)
{
  delete  plan;
}



void  KKB::fftw_one (fftw_plan      plan, 
                     fftw_complex*  in, 
                     fftw_complex*  out
                    )
{
  cerr << endl << endl
    << "fftw_one    ****ERROR***    This function is not implemented."  << endl
    << endl;

  for  (kkint32  x = 0;  x < plan->N ();  ++x)
  {
    out[x].im = 0.0;
    out[x].re = 0.0;
  }
}  // fftw_one 




void  KKB::fftwnd_one (fftwnd_plan    p, 
                       fftw_complex*  in, 
                       fftw_complex*  out
                      )
{
  cerr << endl << endl
    << "fftw_one    ****ERROR***    This function is not implemented."  << endl
    << endl;

  kkint32  totCells = p->NX () * p->NY ();

  for  (kkint32  x = 0;  x < totCells;  ++x)
  {
    out[x].im = 0.0;
    out[x].re = 0.0;
  }
}  // fftwnd_one 

*/





void  SWAP (float& a,  float& b)
{
  float zed = a;
  a = b;
  b = zed;
}



// http://www.codeproject.com/Articles/9388/How-to-implement-the-FFT-algorithm
//data -> float array that represent the array of complex samples
//number_of_complex_samples -> number of samples (N^2 order number) 
//isign -> 1 to calculate FFT and -1 to calculate Reverse FFT
void  KKB::FFT (float    data[], 
                kkuint32 number_of_complex_samples, 
                kkint32  isign
               )
{
  //variables for trigonometric recurrences
  kkuint32  n = 0;
  kkuint32  mmax = 0;
  kkuint32  m = 0;
  kkuint32  j = 0;
  kkuint32  istep = 0; 
  kkuint32  i = 0;

  float  wtemp  = 0.0;
  float  wr     = 0.0;
  float  wpr    = 0.0;
  float  wpi    = 0.0;
  float  wi     = 0.0;
  float  theta  = 0.0;
  float  tempr  = 0.0;
  float  tempi  = 0.0;

  /*
   * The Bit-Reversal Method
   *
   * First, the original array must be transformed in order to perform the Danielson-Lanzcos 
   * algorithm. For example, the complex[index] will swap places with the 
   * complex[bit-reverse-order-index]. If the index (in binary) is 0b00001, the 
   * bit-reverse-order-index will be 0b10000. In figure 1, you can see what happens to the 
   * data array after the transformation.
   *
   * The implementation of this method according to Numerical Recipes In C goes like this:
   */

  //the complex array is real+complex so the array 
  //as a size n = 2* number of complex samples
  // real part is the data[index] and 
  //the complex part is the data[index+1]
  n = number_of_complex_samples * 2; 

  //binary inversion (note that the indexes 
  //start from 0 witch means that the
  //real part of the complex is on the even-indexes 
  //and the complex part is on the odd-indexes
  j = 0;
  for  (i = 0;  i < n / 2;  i += 2)
  {
    if  (j > i) 
    {
      //swap the real part
      SWAP (data[j],  data[i]);

      //swap the complex part
      SWAP (data[j + 1],data[i + 1]);

      // checks if the changes occurs in the first half
      // and use the mirrored effect on the second half
      if  ((j / 2) < (n / 4))
      {
        //swap the real part
        SWAP (data[(n - (i + 2))], data[(n - (j + 2))]);
        //swap the complex part
        SWAP (data[(n - (i + 2)) + 1], data[(n - (j + 2)) + 1]);
      }
    }
    m = n / 2;
    while  (m >= 2 && j >= m) 
    {
      j -= m;
      m = m / 2;
    }
    j += m;
  }

  //Danielson-Lanzcos routine 
  mmax = 2;
  //external loop
  while  (n > mmax)
  {
    istep = mmax <<  1;
    theta = isign * (2 * (float)PIE / mmax);
    wtemp = sin (0.5f * theta);
    wpr = -2.0f * wtemp * wtemp;
    wpi = sin (theta);
    wr = 1.0f;
    wi = 0.0f;
    //internal loops
    for (m = 1;  m < mmax;  m += 2)
    {
      for (i = m;  i <= n;  i += istep)
      {
        j = i + mmax;
        tempr = wr * data[j - 1] - wi * data[j];
        tempi = wr * data[j]     + wi * data[j - 1];
        data[j - 1] = data[i - 1] - tempr;
        data[j]     = data[i]     - tempi;
        data[i - 1] += tempr;
        data[i]     += tempi;
      }
      wr = (wtemp = wr) * wpr - wi * wpi + wr;
      wi = wi * wpr + wtemp * wpi + wi;
    }
    mmax = istep;
  } 
}  /* FFT */


float  KKB::Log2 (float x)
{
  return  log10 (x) / log10 (2.0f);
}





volatile KKB::GoalKeeperPtr   fftwGoalKeeper = NULL;

#if  defined(FFTW_AVAILABLE)
fftwf_plan  KKB::fftwCreateTwoDPlan (kkint32         height,
                                     kkint32         width,
                                     fftwf_complex*  src,
                                     fftwf_complex*  dest,
                                     int             sign,
                                     int             flag
                                    )
{
  if  (!fftwGoalKeeper)
  {
    GoalKeeper::Create ("fftwGoalKeeper", fftwGoalKeeper);
  }

  fftwGoalKeeper->StartBlock ();
  fftwf_plan  plan = fftwf_plan_dft_2d (height, width, src, dest, sign, flag);
  fftwGoalKeeper->EndBlock ();
  return  plan;
}



void  KKB::fftwDestroyPlan (fftwf_plan&  plan)
{
  if  (!fftwGoalKeeper)
    GoalKeeper::Create ("fftwGoalKeeper", fftwGoalKeeper);

  fftwGoalKeeper->StartBlock ();
  fftwf_destroy_plan (plan);
  plan = NULL;
  fftwGoalKeeper->EndBlock ();
}



fftwf_plan  KKB::fftwCreateOneDPlan (kkint32         len,
                                     fftwf_complex*  src,
                                     fftwf_complex*  dest,
                                     int             sign,
                                     int             flag
                                    )
{
  if  (!fftwGoalKeeper)
    GoalKeeper::Create ("fftwGoalKeeper", fftwGoalKeeper);

  fftwGoalKeeper->StartBlock ();
  fftwf_plan  plan = fftwf_plan_dft_1d (len, src, dest, sign, flag);
  fftwGoalKeeper->EndBlock ();
  return  plan;
}

#endif



