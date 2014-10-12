/* kku_fftw.cpp -- Implements a Fast Fourier transform via a template.
 * Copyright (C) 2012-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _KKU_FFTW_
#define _KKU_FFTW_

#include <stdlib.h>
#include <stdio.h>
#include <complex>

#include  "KKBaseTypes.h"

namespace  KKB
{
  /*********************************************
   * Complex numbers and operations 
   *********************************************/
  typedef struct 
  {
    double re;
    double im;
  } fftw_complex;


  /* flags for the planner */
  #define  FFTW_ESTIMATE (0)
  #define  FFTW_MEASURE  (1)

  typedef enum {FFTW_FORWARD = -1, FFTW_BACKWARD = 1}  fftw_direction;


  class  fftw_plan_class
  {
  public:
    fftw_plan_class (int32           _n,
                     fftw_direction  _dir,
                     int32           _flags
                    );

    fftw_direction  Dir   ()  const  {return dir;}
    int32           Flags ()  const  {return flags;}
    int32           N     ()  const  {return n;}


  private:
    int32             n;
    fftw_direction    dir;
    int32             flags;
  };  /* fftw_plan_class */


  class  fftwnd_plan_class
  {
  public:
    fftwnd_plan_class (int32           nx,
                       int32           ny, 
                       fftw_direction  dir,
                       int32           flags
                      );

    fftw_direction  Dir   ()  const  {return dir;}
    int32           Flags ()  const  {return flags;}
    int32           NX    ()  const  {return nx;}
    int32           NY    ()  const  {return ny;}

  private:
    int32           nx;
    int32           ny; 
    fftw_direction  dir;
		int32           flags;
  };  /* fftwnd_plan_class */


  typedef  fftw_plan_class*    fftw_plan;
  typedef  fftwnd_plan_class*  fftwnd_plan;


  fftw_plan    fftw_create_plan (int32           n, 
                                 fftw_direction  dir, 
                                 int32           flags
                                );
 
  fftwnd_plan  fftw2d_create_plan (int32           nx, 
                                   int32           ny, 
                                   fftw_direction  dir,
				                   int32           flags
                                  );

  void  fftw_destroy_plan (fftw_plan  plan);

  void  fftwnd_destroy_plan (fftwnd_plan  plan);


  void  fftw_one (fftw_plan      plan, 
                  fftw_complex*  in, 
                  fftw_complex*  out
                 );


  void  fftwnd_one (fftwnd_plan    p, 
                    fftw_complex*  in, 
                    fftw_complex*  out
                   );



  template<typename DftType>
  class  KK_DFT
  {
  public:
    typedef  std::complex<DftType>  DftComplexType;

    KK_DFT (int32 _size,
            bool  _forwardTransform
           );


    void  Transform (DftComplexType*  src,
                     DftComplexType*  dest
                    );

    void  Transform (KKB::uchar*      src,
                     DftComplexType*  dest
                    );

    void  TransformNR (DftComplexType*  src);


    const DftComplexType**  FourierMask () const {return fourierMask;}

  private:
    void  BuildMask ();

    DftComplexType  MinusOne; 
    DftComplexType  One; 
    DftComplexType  Pi;  
    DftComplexType  Two;
    DftComplexType  Zero;

    bool              forwardTransform;
    DftComplexType**  fourierMask;
    DftComplexType*   fourierMaskArea;
    int32             size;
  };  /* KK_DFT */



  template<typename DftType>
  class  KK_DFT2D
  {
  public:
    typedef  std::complex<DftType>  DftComplexType;

    KK_DFT2D (int32 _height,
              int32 _width,
              bool  _forwardTransform
             );

    ~KK_DFT2D ();

    void  Transform (DftComplexType**  src,
                     DftComplexType**  dest
                    );

  private:
    int32  height;
    int32  width;
    bool forwardTransform;

    DftComplexType    Zero;
    DftComplexType**  workArray;
    DftComplexType*   workArrayArea;

    KK_DFT<DftType>*  rowDFT;
    KK_DFT<DftType>*  colDFT;
  };  /* KK_DFT2D */



  template<typename DftType>
  KK_DFT<DftType>::KK_DFT (int32 _size,
                           bool  _forwardTransform
                          ):
      MinusOne         ((DftType)-1.0,          (DftType)0.0),
      One              ((DftType)1.0,           (DftType)0.0),
      Pi               ((DftType)3.14159265359, (DftType)0.0),
      Two              ((DftType)2.0,           (DftType)0.0),
      Zero             ((DftType)0.0,           (DftType)0.0),
      forwardTransform (_forwardTransform),
      fourierMask      (NULL),
      fourierMaskArea  (NULL),
      size             (_size)
  {
    BuildMask ();
  }


  template<typename DftType>
  void  KK_DFT<DftType>::BuildMask ()
  {
    DftComplexType  N((DftType)size, 0);
    DftComplexType  M((DftType)size, 0);

    int32  x;

    DftComplexType direction;
    if  (forwardTransform)
      direction = DftComplexType ((DftType)-1.0, (DftType)0.0);
    else
      direction = DftComplexType ((DftType)1.0, (DftType)0.0);

    DftComplexType**  fourierMask = new DftComplexType*[size];
    for  (x = 0;  x < size;  x++)
      fourierMask[x] = new DftComplexType[size];

    DftComplexType  j;
    j = sqrt (MinusOne);

    for  (int32 m = 0;  m < size;  m++)
    {
      DftComplexType  mc ((DftType)m, (DftType)0);

      for  (int32 k = 0; k < size; k++)
      {
        DftComplexType  kc ((DftType)k, (DftType)0);
        fourierMask[m][k] = exp (direction * j * Two * Pi * kc * mc / M);

        DftType  exponetPart = (DftType)2.0 * (DftType)3.14159265359 * (DftType)k * (DftType)m / (DftType)size;
        DftType  realPart = cos (exponetPart);
        DftType  imgPart  = -sin (exponetPart);

        if  (realPart != fourierMask[m][k].real ())
        {
          continue;
        }

        if  (imgPart != fourierMask[m][k].imag ())
        {
          continue;
        }
      }
    }

    return;
  }  /* BuildMask */



  template<typename DftType>
  void  KK_DFT<DftType>::Transform (DftComplexType*  src,
                                    DftComplexType*  dest
                                   )
  {
    for  (int32  l = 0;  l < size;  l++)
    {
      dest[l] = Zero;
      for  (int32 k = 0;  k < size;  k++)
      {
        dest[l] = dest[l] + src[k] * fourierMask[l][k];
      }
    }
    return;
  }  /* Transform */



  template<typename DftType>
  void  KK_DFT<DftType>::Transform (KKB::uchar*      src,
                                    DftComplexType*  dest
                                   )
  {
    for  (int32  l = 0;  l < size;  l++)
    {
      dest[l] = Zero;
      for  (int32 k = 0;  k < size;  k++)
      {
        dest[l] = dest[l] + (DftComplexType)(src[k]) * fourierMask[l][k];
      }
    }
    return;
  }  /* Transform */




  template<typename DftType>
  void  KK_DFT<DftType>::TransformNR (DftComplexType*  src)
  {
    int32  n = 0;
    int32  mmax = 0;
    int32  m = 0;
    int32  j = 0;
    int32  istep = 0;
    int32  i = 0;
    int32  isign = (forwardTransform ? 1 : -1);

    DftType  wtemp = (DftType)0.0;
    DftType  wr    = (DftType)0.0;
    DftType  wpr   = (DftType)0.0;
    DftType  wpi   = (DftType)0.0;
    DftType  wi    = (DftType)0.0;
    DftType  theta = (DftType)0.0;
    DftType  tempr = (DftType)0.0;
    DftType  tempi = (DftType)0.0;

    int32  nn = this->size;
    n = nn << 1;
    j = 1;
    for  (i = 1; i < nn;  ++i)
    {
      if  (j > i)
      {
        Swap (src[j].imag, src[i].imag);
        Swap (src[j].real, src[i].real);
      }

      m = nn;
      while  ((m >= 2)  &&  (j > m))
      {
        j -= m;
        m >>= 1;
      }
      j += m;
    }

    mmax = 2;
    while  (n > mmax)
    {
      istep = mmax << 1;
      theta = isign * (TwoPie / mmax);
      wtemp = sin (0.5 * theta);
      wpr = -2.0 * wtemp * wtemp;
      wpi = sin (theta);
      wr = 1.0;
      wi = 0.0;
      for  (m = 1;  m < mmax;  m += 2)
      {
        j = i + mmax;

      }
    }
  }  /* TransformNR */




  template<typename DftType>
  KK_DFT2D<DftType>::KK_DFT2D (int32 _height,
                               int32 _width,
                               bool  _forwardTransform
                              ):
    height           (_height),
    width            (width),
    forwardTransform (_forwardTransform),
    rowDFT           (NULL),
    colDFT           (NULL),
    workArray        (NULL),
    workArrayArea    (NULL),
    Zero             ((DftType)0.0, (DftType)0.0)
  {
    rowDFT = new KK_DFT<DftType> (_width,  _forwardTransform);
    colDFT = new KK_DFT<DftType> (_height, _forwardTransform);


    workArrayArea = new DftComplexType[height * width];
    DftComplexType*  workArrayAreaPtr = workArrayArea;
    for  (int32 row = 0;  row < height;  ++row)
    {
      workArray[row] = workArrayAreaPtr;
      workArrayAreaPtr += width;
    }
  }


  template<typename DftType>
  KK_DFT2D<DftType>::~KK_DFT2D ()
  {
    delete  rowDFT;         rowDFT        = NULL;
    delete  colDFT;         colDFT        = NULL;
    delete  workArray;      workArray     = NULL;
    delete  workArrayArea;  workArrayArea = NULL;
  }



  template<typename DftType>
  void  KK_DFT2D<DftType>::Transform (DftComplexType**  src,
                                      DftComplexType**  dest
                                     )
  {
    for  (int32 row = 0;  row < height;  ++row)
      rowDFT->Transform (src[row], workArrayArea[row]);

    const DftComplexType**  colFourierMask = colDFT->FourierMask ();

    for  (int32 col = 0;  col < width;  ++col)
    {
      for  (int32  row = 0;  row < height;  row++)
      {
        dest[row][col] = Zero;
        for  (int32 k = 0;  k < height;  k++)
        {
          dest[row][col] = dest[row] + src[k] * colFourierMask[row][k];
        }
      }
    }

  }  /* Transform*/
}  /* KKB */
#endif			/* _KKU_FFTW_ */

