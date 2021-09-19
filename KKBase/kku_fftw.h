/* kku_fftw.cpp -- Implements a Fast Fourier transform via a template.
 * Copyright (C) 2012-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _KKU_FFTW_
#define _KKU_FFTW_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex>

#if  defined(FFTW_AVAILABLE)
#include <fftw3.h>
#endif

#include "KKBaseTypes.h"


namespace  KKB
{
  /*********************************************
   * Complex numbers and operations 
   *********************************************/

  /*
  typedef struct 
  {
    double re;
    double im;
  } fftw_complex;
  

  // flags for the planner
  #define  FFTW_ESTIMATE (0)
  #define  FFTW_MEASURE  (1)

  typedef enum {FFTW_FORWARD = -1, FFTW_BACKWARD = 1}  fftw_direction;

  
  class  fftw_plan_class
  {
  public:
    fftw_plan_class (kkint32           _n,
                     fftw_direction  _dir,
                     kkint32           _flags
                    );

    fftw_direction  Dir   ()  const  {return dir;}
    kkint32           Flags ()  const  {return flags;}
    kkint32           N     ()  const  {return n;}


  private:
    kkint32             n;
    fftw_direction    dir;
    kkint32             flags;
  };  // fftw_plan_class


  class  fftwnd_plan_class
  {
  public:
    fftwnd_plan_class (kkint32           nx,
                       kkint32           ny, 
                       fftw_direction  dir,
                       kkint32           flags
                      );

    fftw_direction  Dir   ()  const  {return dir;}
    kkint32           Flags ()  const  {return flags;}
    kkint32           NX    ()  const  {return nx;}
    kkint32           NY    ()  const  {return ny;}

  private:
    kkint32           nx;
    kkint32           ny; 
    fftw_direction  dir;
		kkint32           flags;
  }; // fftwnd_plan_class 


  typedef  fftw_plan_class*    fftw_plan;
  typedef  fftwnd_plan_class*  fftwnd_plan;


  fftw_plan    fftw_create_plan (kkint32           n, 
                                 fftw_direction  dir, 
                                 kkint32           flags
                                );
 
  fftwnd_plan  fftw2d_create_plan (kkint32           nx, 
                                   kkint32           ny, 
                                   fftw_direction  dir,
				                   kkint32           flags
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
 */


  void  FFT (float  data[], 
             kkuint32 number_of_complex_samples, 
             kkint32  isign
            );

  float Log2 (float x);




  template<typename DftType>
  class  KK_DFT1D
  {
  public:
    typedef  std::complex<DftType>  DftComplexType;

    KK_DFT1D (kkint32 _size,
              bool    _forwardTransform
             );

    ~KK_DFT1D ();


    void  Transform (DftComplexType*  src,
                     DftComplexType*  dest
                    );

    void  Transform (KKB::uchar*      src,
                     DftComplexType*  dest
                    );

    void  TransformNR (DftComplexType*  src);


    DftComplexType**  FourierMask () 
    {
      if  (!fourierMask)
        BuildMask ();
      return fourierMask;
    }

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
    kkint32             size;
  };  /* KK_DFT1D */

  typedef  KK_DFT1D<float>   KK_DFT1D_Float;
  typedef  KK_DFT1D<double>  KK_DFT1D_Double;


  template<typename DftType>
  class  KK_DFT2D
  {
  public:
    typedef  std::complex<DftType>  DftComplexType;

    KK_DFT2D (kkint32 _height,
              kkint32 _width,
              bool  _forwardTransform
             );

    ~KK_DFT2D ();

    void  Transform (DftComplexType**  src,
                     DftComplexType**  dest
                    );

    void  Transform (KKB::uchar**      src,
                     DftComplexType**  dest
                    );

    void  Transform2 (KKB::uchar**      src,
                      DftComplexType**  dest
                     );

    void  AllocateArray (DftComplexType*   &arrayArea,
                         DftComplexType**  &array
                        )  const;

    void  DestroyArray (DftComplexType*   &arrayArea,
                        DftComplexType**  &array
                       )  const;

  private:
    kkint32  height;
    kkint32  width;
    bool forwardTransform;

    DftComplexType    Zero;
    DftComplexType**  workArray;
    DftComplexType*   workArrayArea;
    DftComplexType*   workCol;

    KK_DFT1D<DftType>*  rowDFT;
    KK_DFT1D<DftType>*  colDFT;
  };  /* KK_DFT2D */

  typedef  KK_DFT2D<float>   KK_DFT2D_Float;
  typedef  KK_DFT2D<double>  KK_DFT2D_Double;



  template<typename DftType>
  KK_DFT1D<DftType>::KK_DFT1D (kkint32 _size,
                               bool    _forwardTransform
                              ):
      MinusOne         (static_cast<DftType> (-1.0),          static_cast<DftType> (0.0)),
      One              (static_cast<DftType> (1.0),           static_cast<DftType> (0.0)),
      Pi               (static_cast<DftType> (3.14159265359), static_cast<DftType> (0.0)),
      Two              (static_cast<DftType> (2.0),           static_cast<DftType> (0.0)),
      Zero             (static_cast<DftType> (0.0),           static_cast<DftType> (0.0)),
      forwardTransform (_forwardTransform),
      fourierMask      (NULL),
      fourierMaskArea  (NULL),
      size             (_size)
  {
    //BuildMask ();
  }


  template<typename DftType>
  KK_DFT1D<DftType>::~KK_DFT1D ()
  {
    delete  fourierMask;      fourierMask     = NULL;
    delete  fourierMaskArea;  fourierMaskArea = NULL;
  }


  template<typename DftType>
  void  KK_DFT1D<DftType>::BuildMask ()
  {
    DftComplexType  N(static_cast<DftType> (size), 0);
    DftComplexType  M(static_cast<DftType> (size), 0);

    kkint32  x;

    DftComplexType direction;
    if  (forwardTransform)
      direction = DftComplexType (static_cast<DftType> (-1.0), static_cast<DftType> (0.0));
    else
      direction = DftComplexType (static_cast<DftType> (1.0), static_cast<DftType> (0.0));

    fourierMaskArea = new DftComplexType [size * size];
    DftComplexType* fourierMaskAreaPtr = fourierMaskArea;
    fourierMask = new DftComplexType*[size];
    for  (x = 0;  x < size;  x++)
    {
      fourierMask[x] = fourierMaskAreaPtr;
      fourierMaskAreaPtr += size;
    }

    DftComplexType  j;
    j = sqrt (MinusOne);

    for  (kkint32 m = 0;  m < size;  m++)
    {
      DftComplexType  mc (static_cast<DftType> (m), static_cast<DftType> (0));

      for  (kkint32 k = 0; k < size; k++)
      {
        DftComplexType  kc (static_cast<DftType> (k), static_cast<DftType> (0));
        fourierMask[m][k] = exp (direction * j * Two * Pi * kc * mc / M);
      }
    }

    return;
  }  /* BuildMask */

 

  template<typename DftType>
  void  KK_DFT1D<DftType>::Transform (DftComplexType*  src,
                                      DftComplexType*  dest
                                     )
  {
    DftComplexType  M(static_cast<DftType> (size), 0);

    DftComplexType direction;
    if  (forwardTransform)
      direction = DftComplexType (static_cast<DftType> (-1.0), static_cast<DftType> (0.0));
    else
      direction = DftComplexType (static_cast<DftType> (1.0), static_cast<DftType> (0.0));
 
    DftComplexType  j;
    j = sqrt (MinusOne);


    for  (kkint32  l = 0;  l < size;  l++)
    {
      dest[l] = Zero;

      DftComplexType  lc (static_cast<DftType> (l), static_cast<DftType> (0));
      for  (kkint32 k = 0;  k < size;  k++)
      {
        //DftType  exponetPart = static_cast<DftType> (2.0 * static_cast<DftType> (3.14159265359 * static_cast<DftType> (k * static_cast<DftType> (l / static_cast<DftType> (size;
        //DftType  realPart = cos (exponetPart);
        //DftType  imgPart  = -sin (exponetPart);

        DftComplexType  kc (static_cast<DftType> (k), static_cast<DftType> (0));
        DftComplexType  fm = exp (direction * j * Two * Pi * kc * lc / M);

        //dest[l] = dest[l] + src[k] * fourierMask[l][k];
        dest[l] = dest[l] + src[k] * fm;
      }
    }
  
    /*
    {
      // Performs a comparison with a alternatve result that performs much faster but is only good for 
      //  where size is = (n^2) for (n > 0)
      kkint32  newSize = (kkint32)pow (2.0f, (float)ceil (Log2 ((float)size)));

      // Lets do a comparison
      int x, y;
      float* wa = new float[newSize * 2];
      for  (x = 0;  x < (newSize * 2);  ++x)
        wa[x] = 0.0;

      for  (x = 0, y = 0;  x < size;  ++x, y += 2)
      {
        wa[y]     = src[x].real ();
        wa[y + 1] = src[x].imag ();
      }
      FFT (wa, size, 1); 

      for  (x = 0, y = 0;  x < size;  ++x, y += 2)
      {
        float  deltaR = (wa[y]     - dest[x].real ());
        float  deltaI = (wa[y + 1] - dest[x].imag ());

        //if  (deltaR != 0.0)
        //  cout << x << "\t" << wa[y]     << "\t" << dest[x].real () << "\t" << deltaR << endl;

        //if  (deltaI != 0.0)
        //  cout << x << "\t" << wa[y + 1] << "\t" << dest[x].imag () << "\t" << deltaI << endl;

        float waSquare   = wa[y]           * wa[y]            + wa[y + 1]       * wa[y + 1];
        float destSquare = dest[x].real () * dest[x].real ()  + dest[x].imag () * dest[x].imag ();

        float  waMag   = sqrt (waSquare);
        float  destMag = sqrt (destSquare);

        cout  << x << "\t" << waMag << "\t" << destMag << endl;
      }
      delete  wa;
      wa = NULL;
    }
    */

    return;
  }  /* Transform */



  template<typename DftType>
  void  KK_DFT1D<DftType>::Transform (KKB::uchar*      src,
                                      DftComplexType*  dest
                                     )
  {
    if  (!fourierMask)
      BuildMask ();
    
    kkint32  firstK = 0;
    bool   firstKFound = false;
    kkint32  lastK  = 0;

    for  (kkint32 k = 0;  k < size;  ++k)
    {
      if  (src[k] != 0)
      {
        lastK = k;
        if  (!firstKFound)
        {
          firstKFound = true;
          firstK = k;
        }
      }
    }

    for  (kkint32  l = 0;  l < size;  l++)
    {
      DftComplexType*  fourierMaskL = fourierMask[l];

      DftType  destReal = 0.0;
      DftType  destImag = 0.0;

      for  (kkint32 k = firstK;  k <= lastK;  k++)
      {
        destReal +=  src[k] * fourierMaskL[k].real ();
        destImag +=  src[k] * fourierMaskL[k].imag ();
      }

      dest[l] = complex<DftType> (destReal, destImag);
    }
    return;
  }  /* Transform */




  template<typename DftType>
  void  KK_DFT1D<DftType>::TransformNR (DftComplexType*  src)
  {
    kkint32  n = 0;
    kkint32  mmax = 0;
    kkint32  m = 0;
    kkint32  j = 0;
    kkint32  istep = 0;
    kkint32  i = 0;
    kkint32  isign = (forwardTransform ? 1 : -1);

    DftType  wtemp = static_cast<DftType> (0.0);
    DftType  wr    = static_cast<DftType> (0.0);
    DftType  wpr   = static_cast<DftType> (0.0);
    DftType  wpi   = static_cast<DftType> (0.0);
    DftType  wi    = static_cast<DftType> (0.0);
    DftType  theta = static_cast<DftType> (0.0);
    DftType  tempr = static_cast<DftType> (0.0);
    DftType  tempi = static_cast<DftType> (0.0);

    kkint32  nn = this->size;
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
  KK_DFT2D<DftType>::KK_DFT2D (kkint32 _height,
                               kkint32 _width,
                               bool  _forwardTransform
                              ):
    height           (_height),
    width            (_width),
    forwardTransform (_forwardTransform),
    Zero             (static_cast<DftType> (0.0), static_cast<DftType> (0.0)),
    workArray        (NULL),
    workArrayArea    (NULL),
    workCol          (NULL),
    rowDFT           (NULL),
    colDFT           (NULL)
  {
    rowDFT = new KK_DFT1D<DftType> (_width,  _forwardTransform);
    colDFT = new KK_DFT1D<DftType> (_height, _forwardTransform);

    workArrayArea = new DftComplexType[height * width];
    workArray     = new DftComplexType*[height];
    DftComplexType*  workArrayAreaPtr = workArrayArea;
    for  (kkint32 row = 0;  row < height;  ++row)
    {
      workArray[row] = workArrayAreaPtr;
      workArrayAreaPtr += width;
    }
    workCol = new DftComplexType[height];
  }



  template<typename DftType>
  KK_DFT2D<DftType>::~KK_DFT2D ()
  {
    delete  rowDFT;         rowDFT        = NULL;
    delete  colDFT;         colDFT        = NULL;
    delete  workArray;      workArray     = NULL;
    delete  workArrayArea;  workArrayArea = NULL;
    delete  workCol;        workCol       = NULL;
  }



  template<typename DftType>
  void  KK_DFT2D<DftType>::Transform (DftComplexType**  src,
                                      DftComplexType**  dest
                                     )
  {
    for  (kkint32 row = 0;  row < height;  ++row)
      rowDFT->Transform (src[row], workArray[row]);

    DftComplexType**  colFourierMask = colDFT->FourierMask ();

    for  (kkint32 col = 0;  col < width;  ++col)
    {
      for  (kkint32  row = 0;  row < height;  row++)
      {
        DftComplexType  v = Zero;
        for  (kkint32 k = 0;  k < height;  k++)
        {
          v = v  + workArray[k][col] * colFourierMask[row][k];
        }

        dest[row][col] = v;
      }
    }

  }  /* Transform*/


  template<typename DftType>
  void  KK_DFT2D<DftType>::Transform (KKB::uchar**      src,
                                      DftComplexType**  dest
                                     )
  {
    for  (kkint32 row = 0;  row < height;  ++row)
      rowDFT->Transform (src[row], workArray[row]);

    DftComplexType**  colFourierMask = colDFT->FourierMask ();

    for  (kkint32 col = 0;  col < width;  ++col)
    {
      kkuint32  firstRow = 0;
      bool    firstRowFound = false;
      kkuint32  lastRow = 0;

      for  (kkint32 l = 0;  l < height;  ++l)
      {
        workCol[l] = workArray[l][col];
        bool  nonZero = (workCol[l].imag () != 0.0f)  ||  (workCol[l].real () != 0.0f);
        if  (nonZero)
        {
          lastRow = l;
          if  (!firstRowFound)
          {
            firstRowFound = true;
            firstRow = l;
          }
        }
      }

      for  (kkint32  row = 0;  row < height;  row++)
      {
        DftComplexType*  colFourierMaskRow = colFourierMask[row];

        DftComplexType  v = Zero;
        for  (kkuint32 k = firstRow;  k <= lastRow;  k++)
        {
          v = v  + workCol[k] * colFourierMaskRow[k];
        }

        dest[row][col] = v;
      }
    }
  }  /* Transform */



  template<typename DftType>
  void  KK_DFT2D<DftType>::AllocateArray (DftComplexType*   &arrayArea,
                                          DftComplexType**  &array
                                         )  const
  {
    kkint32  total = height * width;
    array = NULL;
    arrayArea = new DftComplexType[total];
    if  (!arrayArea)
      return;

    array = new DftComplexType*[height];
    DftComplexType*  rowPtr = arrayArea;

    for  (kkint32  row = 0;  row < height;  ++row)
    {
      array[row] = rowPtr;
      rowPtr = rowPtr + width;
    }

    for  (kkint32  x = 0;  x < total;  ++x)
    {
      arrayArea[x].real (static_cast<DftType> (0.0));
      arrayArea[x].imag (static_cast<DftType> (0.0));
    }

    return;
  }  /* AllocateArray */



  template<typename DftType>
  void  KK_DFT2D<DftType>::DestroyArray (DftComplexType*   &arrayArea,
                                         DftComplexType**  &array
                                        )  const
  {
    delete  arrayArea;  arrayArea = NULL;
    delete  array;      array     = NULL;
    return;
  }  /* DestroyArray */



#if  !defined(FFTW_AVAILABLE)
#endif



#if  defined(FFTW_AVAILABLE)
  fftwf_plan  fftwCreateTwoDPlan (kkint32         height,
                                  kkint32         width,
                                  fftwf_complex*  src,
                                  fftwf_complex*  dest,
                                  int             sign,
                                  int             flag
                                 );

  fftwf_plan  fftwCreateOneDPlan (kkint32         len,
                                  fftwf_complex*  src,
                                  fftwf_complex*  dest,
                                  int             sign,
                                  int             flag
                                 );

  void  fftwDestroyPlan (fftwf_plan&  plan);
#endif

}  /* KKB */

#endif			/* _KKU_FFTW_ */
