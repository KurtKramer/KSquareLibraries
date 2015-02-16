// Kurt Kramer  2012-04-28
// I took this module from my Work at Larcos;  
// it is a very simple implementation of the Fourietr transform.  It has the advantage of 
// not requiring external libraries and easier to debug.


#ifndef _KKU_FFTW_
#define _KKU_FFTW_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <complex>

#if  defined(FFTW_AVAILABLE)
#include <fftw3.h>
#endif

#include "BasicTypes.h"


namespace  KKU
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
  };  // fftw_plan_class


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
  }; // fftwnd_plan_class 


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
 */


  void  FFT (float  data[], 
             uint32 number_of_complex_samples, 
             int32  isign
            );

  float Log2 (float x);




  template<typename DftType>
  class  KK_DFT1D
  {
  public:
    typedef  std::complex<DftType>  DftComplexType;

    KK_DFT1D (int32 _size,
              bool  _forwardTransform
             );

    ~KK_DFT1D ();


    void  Transform (DftComplexType*  src,
                     DftComplexType*  dest
                    );

    void  Transform (KKU::uchar*      src,
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
    int32             size;
  };  /* KK_DFT1D */

  typedef  KK_DFT1D<float>   KK_DFT1D_Float;
  typedef  KK_DFT1D<double>  KK_DFT1D_Double;



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

    void  Transform (KKU::uchar**      src,
                     DftComplexType**  dest
                    );

    void  Transform2 (KKU::uchar**      src,
                      DftComplexType**  dest
                     );

    void  AllocateArray (DftComplexType*   &arrayArea,
                         DftComplexType**  &array
                        )  const;

    void  DestroyArray (DftComplexType*   &arrayArea,
                        DftComplexType**  &array
                       )  const;

  private:
    int32  height;
    int32  width;
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
  KK_DFT1D<DftType>::KK_DFT1D (int32 _size,
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
    DftComplexType  N((DftType)size, 0);
    DftComplexType  M((DftType)size, 0);

    int32  x;

    DftComplexType direction;
    if  (forwardTransform)
      direction = DftComplexType ((DftType)-1.0, (DftType)0.0);
    else
      direction = DftComplexType ((DftType)1.0, (DftType)0.0);

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

    for  (int32 m = 0;  m < size;  m++)
    {
      DftComplexType  mc ((DftType)m, (DftType)0);

      for  (int32 k = 0; k < size; k++)
      {
        DftComplexType  kc ((DftType)k, (DftType)0);
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
    DftComplexType  M((DftType)size, 0);

    DftComplexType direction;
    if  (forwardTransform)
      direction = DftComplexType ((DftType)-1.0, (DftType)0.0);
    else
      direction = DftComplexType ((DftType)1.0, (DftType)0.0);
 
    DftComplexType  j;
    j = sqrt (MinusOne);


    for  (int32  l = 0;  l < size;  l++)
    {
      dest[l] = Zero;

      DftComplexType  lc ((DftType)l, (DftType)0);
      for  (int32 k = 0;  k < size;  k++)
      {
        //DftType  exponetPart = (DftType)2.0 * (DftType)3.14159265359 * (DftType)k * (DftType)l / (DftType)size;
        //DftType  realPart = cos (exponetPart);
        //DftType  imgPart  = -sin (exponetPart);

        DftComplexType  kc ((DftType)k, (DftType)0);
        DftComplexType  fm = exp (direction * j * Two * Pi * kc * lc / M);

        //dest[l] = dest[l] + src[k] * fourierMask[l][k];
        dest[l] = dest[l] + src[k] * fm;
      }
    }
  
    /*
    {
      // Performs a comparison with a alternatve result that performs much faster but is only good for 
      //  where size is = (n^2) for (n > 0)
      int32  newSize = (int32)pow (2.0f, (float)ceil (Log2 ((float)size)));

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
  void  KK_DFT1D<DftType>::Transform (KKU::uchar*      src,
                                      DftComplexType*  dest
                                     )
  {
    if  (!fourierMask)
      BuildMask ();
    
    int32  firstK = 0;
    bool   firstKFound = false;
    int32  lastK  = 0;

    for  (int32 k = 0;  k < size;  ++k)
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

    for  (int32  l = 0;  l < size;  l++)
    {
      DftComplexType*  fourierMaskL = fourierMask[l];

      DftType  destReal = 0.0;
      DftType  destImag = 0.0;

      for  (int32 k = firstK;  k <= lastK;  k++)
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
    width            (_width),
    forwardTransform (_forwardTransform),
    rowDFT           (NULL),
    colDFT           (NULL),
    workArray        (NULL),
    workArrayArea    (NULL),
    workCol          (NULL),
    Zero             ((DftType)0.0, (DftType)0.0)
  {
    rowDFT = new KK_DFT1D<DftType> (_width,  _forwardTransform);
    colDFT = new KK_DFT1D<DftType> (_height, _forwardTransform);

    workArrayArea = new DftComplexType[height * width];
    workArray     = new DftComplexType*[height];
    DftComplexType*  workArrayAreaPtr = workArrayArea;
    for  (int32 row = 0;  row < height;  ++row)
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
    for  (int32 row = 0;  row < height;  ++row)
      rowDFT->Transform (src[row], workArray[row]);

    DftComplexType**  colFourierMask = colDFT->FourierMask ();

    for  (int32 col = 0;  col < width;  ++col)
    {
      for  (int32  row = 0;  row < height;  row++)
      {
        DftComplexType  v = Zero;
        for  (int32 k = 0;  k < height;  k++)
        {
          v = v  + workArray[k][col] * colFourierMask[row][k];
        }

        dest[row][col] = v;
      }
    }

  }  /* Transform*/


  template<typename DftType>
  void  KK_DFT2D<DftType>::Transform (KKU::uchar**      src,
                                      DftComplexType**  dest
                                     )
  {
    for  (int32 row = 0;  row < height;  ++row)
      rowDFT->Transform (src[row], workArray[row]);

    DftComplexType**  colFourierMask = colDFT->FourierMask ();

    for  (int32 col = 0;  col < width;  ++col)
    {
      uint32  firstRow = 0;
      bool    firstRowFound = false;
      uint32  lastRow = 0;

      for  (int32 l = 0;  l < height;  ++l)
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

      for  (int32  row = 0;  row < height;  row++)
      {
        DftComplexType*  colFourierMaskRow = colFourierMask[row];

        DftComplexType  v = Zero;
        for  (uint32 k = firstRow;  k <= lastRow;  k++)
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
    int32  total = height * width;
    array = NULL;
    arrayArea = new DftComplexType[total];
    if  (!arrayArea)
      return;

    array = new DftComplexType*[height];
    DftComplexType*  rowPtr = arrayArea;

    for  (int32  row = 0;  row < height;  ++row)
    {
      array[row] = rowPtr;
      rowPtr = rowPtr + width;
    }

    for  (int32  x = 0;  x < total;  ++x)
    {
      arrayArea[x].real ((DftType)0.0);
      arrayArea[x].imag ((DftType)0.0);
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
  fftwf_plan  fftwCreateTwoDPlan (int32           height,
                                  int32           width,
                                  fftwf_complex*  src,
                                  fftwf_complex*  dest,
                                  int             sign,
                                  int             flag
                                 );

  fftwf_plan  fftwCreateOneDPlan (int32           len,
                                  fftwf_complex*  src,
                                  fftwf_complex*  dest,
                                  int             sign,
                                  int             flag
                                 );

  void  fftwDestroyPlan (fftwf_plan&  plan);
#endif

}  /* KKU */






#endif			/* _KKU_FFTW_ */

