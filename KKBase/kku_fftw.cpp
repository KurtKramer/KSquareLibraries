/* kku_fftw.cpp -- Implements a Fast Fourier transform via a template.
 * Copyright (C) 2012-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in ImageExtractionManager.h
 */
#include  "FirstIncludes.h"
#include  <stdlib.h>
#include  <memory>
#include  <math.h>

#include  <complex>
#include  <fstream>
#include  <iostream>
#include  <string>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;

#if  defined(FFTW_AVAILABLE)
#endif

#include  "KKBaseTypes.h"
#include  "kku_fftw.h"
using namespace KKB;



fftw_plan_class::fftw_plan_class (int32           _n,
                                  fftw_direction  _dir,
                                  int32           _flags
                                 ):
   n     (_n),
   dir   (_dir),
   flags (_flags)
{
}



fftwnd_plan_class::fftwnd_plan_class (int32           _nx,
                                      int32           _ny, 
                                      fftw_direction  _dir,
		                              int32           _flags
                                     ):
   nx    (_nx),
   ny    (_ny),
   dir   (_dir),
   flags (_flags)
{
}



fftw_plan  KKB::fftw_create_plan (int32           n, 
                                  fftw_direction  dir, 
                                  int32           flags
                                 )
{
  return  new fftw_plan_class (n, dir, flags);
}



void  KKB::fftw_destroy_plan (fftw_plan  plan)
{
  delete  plan;
  plan = NULL;
}



fftwnd_plan  KKB::fftw2d_create_plan (int32           nx, 
                                      int32           ny, 
                                      fftw_direction  dir,
				                      int32           flags
                                     )
{
  return new fftwnd_plan_class (nx, ny, dir, flags);
}  /* fftw2d_create_plan */



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

  for  (int32  x = 0;  x < plan->N ();  ++x)
  {
    out[x].im = 0.0;
    out[x].re = 0.0;
  }
}  /* fftw_one */




void  KKB::fftwnd_one (fftwnd_plan    p, 
                       fftw_complex*  in, 
                       fftw_complex*  out
                      )
{
  cerr << endl << endl
    << "fftw_one    ****ERROR***    This function is not implemented."  << endl
    << endl;

  int32  totCells = p->NX () * p->NY ();

  for  (int32  x = 0;  x < totCells;  ++x)
  {
    out[x].im = 0.0;
    out[x].re = 0.0;
  }
}  /* fftwnd_one */





KK_DFT<float>  x (100, true);
