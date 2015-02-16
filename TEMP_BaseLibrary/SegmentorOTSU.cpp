/* SegmentorOTSU.cpp -- Used to segment Raster images using OTSU algorithm.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include  "FirstIncludes.h"

#include  <memory>
#include  <math.h>

#include  <float.h>
#include  <limits.h>
#include  <algorithm>
#include  <fstream>
#include  <map>
#include  <string>
#include  <iostream>
#include  <vector>
using namespace std;

#include  "MemoryDebug.h"

#include  "SegmentorOTSU.h"

#include  "BasicTypes.h"
#include  "Raster.h"
using namespace  KKU;


SegmentorOTSU::SegmentorOTSU (RunLog&  _log):
  log (_log)
{
  double z = 0.0;
  NaN = 1.0 / z;
}


SegmentorOTSU::~SegmentorOTSU ()
{
}



void   SegmentorOTSU::LabelRaster (RasterPtr  result,
                                   uchar      pixelValue,
                                   uchar      label,
                                   RasterPtr  srcImage
                                  )
{
  uchar*  resultArea = result->GreenArea ();
  uchar*  srcArea    = srcImage->GreenArea ();
  int32   totPixels  = srcImage->TotPixels ();
  for  (int32  x = 0;  x < totPixels;  ++x)
  {
    if  (srcArea[x] == pixelValue)
    {
      resultArea[x] = label;
    }
  }
}  /* LabelRaster */


void   SegmentorOTSU::LabelRaster (RasterPtr  result,
                                   RasterPtr  mask,
                                   uchar      pixelValue,
                                   uchar      label,
                                   RasterPtr  srcImage
                                  )
{
  uchar*  maskArea = NULL;
  uchar   maskTh   = 0;
  if  (mask)
  {
    maskArea = mask->GreenArea ();
    maskTh   = mask->BackgroundPixelTH ();
  }

  uchar*  resultArea = result->GreenArea ();
  uchar*  srcArea    = srcImage->GreenArea ();
  int32   totPixels  = srcImage->TotPixels ();
  for  (int32  x = 0;  x < totPixels;  ++x)
  {
    if  ((srcArea[x] == pixelValue)  &&  (maskArea[x] > maskTh))
    {
      resultArea[x] = label;
    }
  }
}  /* LabelRaster */


template<typename T>
vector<T>  SegmentorOTSU::BDV (T  start,
                               T  inc,
                               T  end
                              )
{
  vector<T>  r;
  while  (start <= end)
  {
    r.push_back (start);
    start += inc;
  }
  return  r;
}  /* BDV */



/**  @brief  Returns a vector that represents the accumulatd sum of the input vector. */
template<typename T>
vector<T>   SegmentorOTSU::CumSum (const vector<T>&  v)
{
  vector<T>  r;

  T  total = 0.0;
  for  (uint32  x = 0;  x < v.size ();  ++x)
  {
    total += v[x];
    r.push_back (total);
  }

  return  r;
}  /* CumSum */



VectorDouble  SegmentorOTSU::DotMult (const VectorDouble&  left,
                                      const VectorDouble&  right
                                     )
{
  uint32  lenMax = Max (left.size (), right.size ());
  uint32  lenMin = Min (left.size (), right.size ());

  VectorDouble  r (lenMax, 0.0);
  for  (uint32 x = 0;  x < lenMin;  ++x)
    r[x] = left[x] * right[x];

  return  r;
}  /* DotMult */



Matrix  SegmentorOTSU::DotMult (const Matrix&  left,
                                const Matrix&  right
                               )
{
  int32  maxNumOfRows = Max (left.NumOfRows (), right.NumOfRows ());
  int32  maxNumOfCols = Max (left.NumOfCols (), right.NumOfCols ());
  
  int32  minNumOfRows = Min (left.NumOfRows (), right.NumOfRows ());
  int32  minNumOfCols = Min (left.NumOfCols (), right.NumOfCols ());

  Matrix  result (maxNumOfRows, maxNumOfCols);

  double** leftData  = left.Data  ();
  double** rightData = right.Data ();
  double** resultData = result.Data ();

  int32  r, c;

  for  (r = 0;  r < minNumOfRows;  ++r)
  {
    double*  leftDataRow   = leftData[r];
    double*  rightDataRow  = rightData[r];
    double*  resultDataRow = resultData[r];

    for  (c = 0;  c < minNumOfCols;  ++c)
      resultDataRow[c] = leftDataRow[c] * rightDataRow[c];
  }

  for  (r = minNumOfRows;  r < maxNumOfRows;  ++r)
  {
    double*  resultDataRow = resultData[r];
    for  (c = minNumOfCols;  c < maxNumOfCols;  ++c)
      resultDataRow[c] = 0.0;
  }
  return  result;
}  /* DotMult */


VectorDouble  SegmentorOTSU::DotDiv (const VectorDouble&  left,
                                     const VectorDouble&  right
                                    )
{
  uint32 x = 0;
  uint32  lenMax = Max (left.size (), right.size ());
  uint32  lenMin = Min (left.size (), right.size ());

  VectorDouble  r (lenMax, 0.0);
  for  (x = 0;  x < lenMin;  ++x)
  {
    double  rs = right[x];
    if  (rs == 0.0)
      r[x] = NaN;
    else
      r[x] = left[x] / rs;
  }

  for  (x = lenMin;  x < lenMax;  ++x)
    r[x] = (double)NaN;

  return  r;
}  /* DotDiv */



Matrix  SegmentorOTSU::DotDiv (const Matrix&  left,
                               const Matrix&  right
                              )
{
  int32  maxNumOfRows = Max (left.NumOfRows (), right.NumOfRows ());
  int32  maxNumOfCols = Max (left.NumOfCols (), right.NumOfCols ());
  
  int32  minNumOfRows = Min (left.NumOfRows (), right.NumOfRows ());
  int32  minNumOfCols = Min (left.NumOfCols (), right.NumOfCols ());

  Matrix  result (maxNumOfRows, maxNumOfCols);

  double** leftData  = left.Data  ();
  double** rightData = right.Data ();
  double** resultData = result.Data ();

  int32  r, c;

  for  (r = 0;  r < minNumOfRows;  ++r)
  {
    double*  leftDataRow   = leftData[r];
    double*  rightDataRow  = rightData[r];
    double*  resultDataRow = resultData[r];

    for  (c = 0;  c < minNumOfCols;  ++c)
      resultDataRow[c] = leftDataRow[c] / rightDataRow[c];
  }

  for  (r = minNumOfRows;  r < maxNumOfRows;  ++r)
  {
    double*  resultDataRow = resultData[r];
    for  (c = minNumOfCols;  c < maxNumOfCols;  ++c)
    {
      if  ((r >= right.NumOfRows ())  ||  (c >= right.NumOfCols ()))
        resultDataRow[c] = NaN;

      else if  (rightData[r][c] == 0.0)
        resultDataRow[c] = NaN;

      else
        resultDataRow[c] = 0.0;
    }
  }

  return  result;
}  /* DotDiv */



template<typename T>
vector<T>   SegmentorOTSU::FlipLeftRight (const vector<T>&  v)
{
  
  vector<T>  r(v.size (), (T)0);
  reverse_copy (v.begin (), v.end (), r.begin ());
  return  r;
}  /* FlipLeftRight */




KKU::VectorDouble  SegmentorOTSU::Add (const VectorDouble&  left,
                                       double               right
                                      )
{
  uint32 x = 0;
  uint32 len = left.size ();

  VectorDouble  r (len, 0.0);
  for  (x = 0;  x < len;  ++x)
  {
    double  z1 = left[x];
    double  z2 = z1 + right;
    r[x] = z2;
    //r[x] = (left[r] - right);
  }

  return  r;
}  /* Add */



KKU::VectorDouble  SegmentorOTSU::Subt (const VectorDouble&  left,
                                        const VectorDouble&  right
                                       )
{
  uint32 x = 0;
  uint32  lenMax = Max (left.size (), right.size ());
  uint32  lenMin = Min (left.size (), right.size ());

  VectorDouble  r (lenMax, 0.0);
  for  (x = 0;  x < lenMin;  ++x)
    r[x] = left[x] - right[x];

  for  (x = lenMin;  x < lenMax;  ++x)
  {
    if  (x < left.size ())
      r[x] = left[x];
    else
      r[x] = 0.0 - right[x];
  }

  return  r;
}  /* Subt */



KKU::VectorDouble  SegmentorOTSU::Subt (const VectorDouble&  left,
                                        double               right
                                       )
{
  uint32 x = 0;
  uint32 len = left.size ();

  VectorDouble  r (len, 0.0);
  for  (x = 0;  x < len;  ++x)
  {
    double  z1 = left[x];
    double  z2 = z1 - right;
    r[x] = z2;
    //r[x] = (left[r] - right);
  }

  return  r;
}  /* Subt */



KKU::VectorDouble  SegmentorOTSU::Subt (double               left,
                                        const VectorDouble&  right
                                       )
{
  uint32 x = 0;
  uint32 len = right.size ();

  VectorDouble  r (len, 0.0);
  for  (x = 0;  x < len;  ++x)
  {
    r[x] = left - right[x];
  }

  return  r;
}  /* Subt */





VectorDouble  operator- (const VectorDouble&  left,
                         double               right
                        )
{
  KKU::uint32 x = 0;
  KKU::uint32 len = left.size ();

  VectorDouble  r (len, 0.0);
  for  (x = 0;  x < len;  ++x)
  {
    double  z1 = left[x];
    double  z2 = z1 - right;
    r[x] = z2;
    //r[x] = (left[r] - right);
  }

  return  r;
}  /* operator- */




template<typename T>
vector<T>  operator- (T                 left,
                      const vector<T>&  right
                     )
{
  KKU::uint32 x = 0;
  KKU::uint32 len = right.size ();

  vector<T>  r (len, (T)0);
  for  (x = 0;  x < len;  ++x)
    r[x] = left - right[r];

  return  r;
}  /* operator- */



VectorDouble  operator* (const VectorDouble& left,
                         double              right
                        )
{
  VectorDouble  result (left.size (), 0.0);

  for  (KKU::uint32 x = 0;  x < left.size ();  ++x)
    result[x] = left[x] * right;

  return  result;
}


VectorDouble  operator* (double               left,
                         const VectorDouble&  right
                        )
{
  VectorDouble  result (right.size (), 0.0);

  for  (KKU::uint32 x = 0;  x < right.size ();  ++x)
    result[x] = left * right[x];

  return  result;
}




VectorDouble  SegmentorOTSU::Power (const VectorDouble&  left,
                                    double               right
                                   )
{
  uint32 x = 0;
  uint32 len = left.size ();
  VectorDouble  r (len, 0.0);
  for  (x = 0;  x < len;  ++x)
    r[x] = pow (left[x], right);

  return  r;
}  /* Power */



Matrix  SegmentorOTSU::Power (const Matrix&  left,
                              double         right
                             )
{
  uint32 numOfRows = left.NumOfRows ();
  uint32 numOfCols = left.NumOfCols ();

  Matrix  result (numOfRows, numOfCols);
  double**  leftData = left.Data ();
  double**  resultData = result.Data ();

  uint32  r, c;
  for  (r = 0;  r < numOfRows;  ++r)
  {
    double*  leftDataRow = leftData[r];
    double*  resultDataRow = resultData[r];
    for  (c = 0;  c < numOfCols;  ++c)
      resultDataRow[c] = pow (leftDataRow[c], right);
  }
  return  result;
}  /* Power */




VectorDouble  SegmentorOTSU::Round (const VectorDouble&  v)
{
  VectorDouble  r (v.size (), 0.0);
  for  (uint32 x = 0;  x < v.size ();  ++x)
  {
    r[x] = (int32)(v[x] + 0.5f);
  }

  return  r;
}  /* Round */




template<typename T>
T  SegmentorOTSU::Sum (const vector<T>&  v)
{
  T  sum = (T)0;
  uint32  x = 0;
  for  (x = 0;  x < v.size ();  ++x)
    sum += v[x];
  return  sum;
}  /* Sum */




    //[w0,w2] = ndgrid(w0,w2);

void  SegmentorOTSU::NdGrid (const VectorDouble&  x,
                             const VectorDouble&  y,
                             Matrix&              xm,
                             Matrix&              ym
                            )
{
  uint32  xLen = x.size ();
  uint32  yLen = y.size ();

  xm.ReSize (xLen, xLen);
  ym.ReSize (yLen, yLen);

  uint32  row, col;

  for  (row = 0;  row < xLen;  ++row)
  {
    for  (col = 0;  col < yLen;  ++col)
      xm[row][col] = x[row];
  }

  for  (row = 0;  row < xLen;  ++row)
  {
    for  (col = 0;  col < yLen;  ++col)
      ym[row][col] = y[col];
  }
}



void  SegmentorOTSU::MakeNanWhenLesOrEqualZero (Matrix&  m)
{
  int32  r, c;
  double**  data = m.Data ();
  for  (r = 0;  r < m.NumOfRows ();  ++r)
  {
    double*  dataRow = data[r];
    for  (c = 0;  c < m.NumOfCols ();  ++c)
    {
      if  (dataRow[c] <= 0.0)
        dataRow[c] = NaN;
    }
  }
}  /* MakeNanWhenLesOrEqualZero */



template<typename T>
vector<T>  SegmentorOTSU::SubSet (const vector<T>&  P, 
                                  int32             start,
                                  int32             end
                                 )
{
  vector<T>  subSet;
  for  (int32 i = start;  i <= end;  ++i)
    subSet.push_back (P[i]);
  return  subSet;
}



template<typename T>
T  SegmentorOTSU::SumSubSet (const vector<T>&  P, 
                             int32             start,
                             int32             end
                            )
{
  T  sum = (T)0.0;
  for  (int32 i = start;  i <= end;  ++i)
    sum += P[i];
  return  sum;
}


template<typename T>
void  SegmentorOTSU::ZeroOutNaN (vector<T>&  v)
{
  uint32  x = 0;
  for  (x = 0;  x < v.size ();  ++x)
  {
    if  (_isnan (v[x]))
      v[x] = (T)0.0;
  }
}

#if  !defined(OS_WINDOWS)
bool  _isnan (double&   d)
{
  return  !(isnan (d) == 0);
}
#endif



void  SegmentorOTSU::ZeroOutNaN (Matrix&  m)
{
  double**  data = m.Data ();
  uint32  numOfRows = m.NumOfRows ();
  uint32  numOfCols = m.NumOfCols ();

  uint32  r, c;
  for  (r = 0;  r < numOfRows;  ++r)
  {
    double*  dataRow = data[r];
    for  (c = 0;  c < numOfCols;  ++c)
    {
      if  (IsNaN (dataRow[c]))
        dataRow[c] = 0.0;
    }
  }
}  /* ZeroOutNaN */


VectorDouble  SegmentorOTSU::LinSpace (double  start,
                                       double  end,
                                       int32   numPoints
                                      )
{
  // The First and last point are known leaving (numPoints - 2) left to distribute between "start" and "end".
  // which means there will be a total of (numPoints - 1) intervals.
  double  inc = (end - start) / (double)(numPoints - 1);
  VectorDouble  result;
  while  (result.size () < (uint32)numPoints)
  {
    result.push_back (start);
    start += inc;
  }

  return  result;
}  /* LinSpace */



double  SegmentorOTSU::sig_func (VectorDouble          k,
                                 int32                 nbins,
                                 const VectorDouble&   P,
                                 int32                 numClasses
                                )
{
  int32 x = 0, y = 0;

  // muT = sum((1:nbins).*P);
  double  muT = 0.0;
  for  (x = 0, y = 1;  x < nbins;  ++x, ++y)
    muT += y * P[x];

  //sigma2T = sum(((1:nbins)-muT).^2.*P);
  double  sigma2T = 0.0;
  for  (x = 0, y = 1;  x < nbins;  ++x, ++y)
    sigma2T +=  (pow ((y - muT), 2.0) * P[x]);

  //k = round(k*(nbins-1)+1);
  for  (x = 0;  x < (int32)k.size ();  ++x)
    k[x] = floor (k[x] * (nbins - 1) + 1.0 + 0.5);

  //k = sort(k);
  sort (k.begin (), k.end ());

  //if any(k<1 | k>nbins), y = 1; return, end
  if  ((k[0] < 1.0)  ||  (k[k.size () - 1] > nbins))
    return 1.0;
        
  //k = [0 k nbins];   %  Puts '0' at beginning and 'nbins' at the end.
  k.insert (k.begin (), 0.0);
  k.push_back (nbins);

  // sigma2B = 0;
  double  sigma2B = 0.0;

  //for j = 1:numClasses
  for  (int32 j = 0;  j < numClasses;  ++j)
  {
    //wj = sum(P(k(j)+1:k(j+1)));
    double  wj = SumSubSet (P, (int32)(k[j] + 1), (int32)k[j + 1]);

    //if wj==0, y = 1; return, end
    if  (wj == 0.0)
      return 1.0;

    //muj = sum((k(j)+1:k(j+1)).*P(k(j)+1:k(j+1)))/wj;
    int32  idxStart = (int32)(k[j] + 1);
    int32  idxEnd   = (int32)(k[j + 1]);
    double  muj = 0.0;
    for  (int32 i = idxStart;  i <= idxEnd;  ++i)
      muj += (i * P[i] / wj);

    //sigma2B = sigma2B + wj*(muj-muT)^2;
    sigma2B = sigma2B + wj * pow ((muj - muT), 2.0);
  }

  //y = 1-sigma2B/sigma2T; % within the range [0 1]
  return  1.0 - sigma2B / sigma2T;
}  /* sig_func */





RasterPtr  SegmentorOTSU::SegmentImage (RasterPtr  srcImage,
                                        int32      numClasses,
                                        double&    sep
                                       )
{
  /*
  function [IDX,sep] = otsu (srcImage, numClasses)

  %OTSU Global image thresholding/segmentation using Otsu's method.
  %   IDX = OTSU(srcImage,N) segments the image srcImage into N classes by means of Otsu's
  %   N-thresholding method. OTSU returns an array IDX containing the cluster
  %   indices (from 1 to N) of each point. Zero values are assigned to
  %   non-finite (NaN or Inf) pixels.
  %
  %   IDX = OTSU(srcImage) uses two classes (N=2, default value).
  %
  %   [IDX,sep] = OTSU(...) also returns the value (sep) of the separability
  %   criterion within the range [0 1]. Zero is obtained only with data
  %   having less than N values, whereas one (optimal value) is obtained only
  %   with N-valued arrays.
  %
  %   Notes:
  %   -----
  %   It should be noticed that the thresholds generally become less credible
  %   as the number of classes (N) to be separated increases (see Otsu's
  %   paper for more details).
  %
  %   If srcImage is an RGB image, a Karhunen-Loeve transform is first performed on
  %   the three R,G,B channels. The segmentation is then carried out on the
  %   image component that contains most of the energy. 
  %
  %   Example:
  %   -------
  %   load clown
  %   subplot(221)
  %   X = ind2rgb(X,map);
  %   imshow(X)
  %   title('Original','FontWeight','bold')
  %   for numClasses = 2:4
  %     IDX = otsu(X,numClasses);
  %     subplot(2,2,numClasses)
  %     imagesc(IDX), axis image off
  %     title(['numClasses = ' int2str(numClasses)],'FontWeight','bold')
  %   end
  %   colormap(gray)
  %
  %   Reference:
  %   ---------
  %   Otsu N, <a href="matlab:web('http://dx.doi.org/doi:10.1109/TSMC.1979.4310076')">A Threshold Selection Method from Gray-Level Histograms</a>,
  %   IEEE Trans. Syst. Man Cybern. 9:62-66;1979 
  %
  %   See also GRAYTHRESH, IM2BW
  %
  %   -- Damien Garcia -- 2007/08, revised 2010/03
  %   Visit my <a
  %   href="matlab:web('http://www.biomecardio.com/matlab/otsu.html')">website</a> for more details about OTSU
  */
  
  int32  pixelIdx = 0;
  int32  x        = 0;
  bool  isColorImage = srcImage->Color ();

  //  Checking numClasses (number of classes)
  
  if  (numClasses == 1)
  {
    //IDX = NaN(size(srcImage));
    //sep = 0;
    return  NULL;
  }

  else if  ((numClasses < 1)  ||  (numClasses > 255))
  {
    log.Level (-1) << "SegmentorOTSU::SegmentImage  ***ERROR***   'numClasses must be a between 1 and 255 !'" << endl;
    sep = 0;
    return NULL;
  }

  if  (isColorImage)
  {
    srcImage = srcImage->CreateGrayScaleKLT ();
  }
  else
  {
    srcImage = new Raster (*srcImage);
  }

  int32  totPixels = srcImage->TotPixels ();
  VectorInt  unI;
  VectorInt  unICounts;
  
  {
    // %% Convert to 256 levels
    // srcImage = srcImage-min(srcImage(:));
    // srcImage = round(srcImage/max(srcImage(:))*255);
    //  Re-scale Source Image to utilize range of 0 thru 255.

    //%% Probability dCistribution
    //unI = sort(unique(srcImage));
    //nbins = min(length(unI),256);
 

    uchar*  greenArea = srcImage->GreenArea ();

    uchar  pixelMin = greenArea[0];
    uchar  pixelMax = greenArea[0];

    for  (pixelIdx = 1;  pixelIdx < totPixels;  ++pixelIdx)
    {
      if  (greenArea[pixelIdx] < pixelMin)
        pixelMin = greenArea[pixelIdx];

      if  (greenArea[pixelIdx] > pixelMax)
        pixelMax = greenArea[pixelIdx];
    }

    VectorInt  counts (256, 0);

    for  (pixelIdx = 0;  pixelIdx < totPixels;  ++pixelIdx)
    {
      /*
      double  pixelFraction = (double)((double)(greenArea[pixelIdx]) - pixelMin) / (double)srcRange;
      uint32  newPixelVal = (uchar)(pixelFraction * 256.0 + 0.5);
      if  (newPixelVal > 255)
        newPixelVal = 255;

      greenArea[pixelIdx] = (uchar)newPixelVal;
      counts[newPixelVal]++;
      */
      counts[greenArea[pixelIdx]]++;
    }

    for  (x = 0;  x < (int32)counts.size ();  ++x)
    {
      if  (counts[x] > 0)
      {
        unI.push_back (x);
        unICounts.push_back (counts[x]);
      }
    }
  }

  int32  nbins = unI.size ();

  if  (nbins <= numClasses)
  {
    // IDX = ones (size(srcImage));
    //for i = 1:numClasses, IDX(srcImage==unI(i)) = i; end
    //sep = 1;
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    for  (x = 0;  x < nbins;  ++x)
    {
      LabelRaster (result, unI[x], x, srcImage);
    }
    sep = 1;
    delete  srcImage;   
    srcImage = NULL;
    return  result;
  }

  //elseif  (nbins < 256)
  //  [histo,pixval] = hist(srcImage(:),unI);
  //else
  //  [histo,pixval] = hist(srcImage(:),256);
  VectorInt  histo  = unICounts;
  VectorInt  pixval = unI;

  //P = histo/sum(histo);
  VectorDouble  P (histo.size (), 0.0);
  for  (x = 0;  x < (int32)histo.size ();  ++x)
    P[x] = (double)(histo[x]) / (double)totPixels;

  //clear unI
  unI.clear ();

  //%% Zeroth- and first-order cumulative moments
  //w = cumsum(P);
  VectorDouble  w (P.size (), 0.0);
  w[0] = P[0];
  for  (x = 1;  x < (int32)P.size (); ++x)
    w[x] = w[x - 1] + P[x];

  //mu = cumsum((1:nbins).*P);
  VectorDouble  mu (P.size (), 0.0);
  mu[0] = 1.0 * P[0];
  for  (x = 1;  x < (int32)P.size ();  ++x)
    mu[x] = mu[x - 1] + ((x + 1) * P[x]);
  double  muEnd = mu[mu.size () - 1];
  
  //%% Maximal sigmaB^2 and Segmented image
  if  (numClasses == 2)
  {
    //sigma2B =...
    //    (mu(end) * w(2:end-1) - mu(2:end-1)) .^2  ./  w(2:end-1)./(1-w(2:end-1));
    //    ------------------- P1 -----------------      ----------- P2 -----------
    //[maxsig,k] = max(sigma2B);

    VectorDouble  wSubSet  = SubSet (w,  1, w.size  () - 2);
    VectorDouble  muSubSet = SubSet (mu, 1, mu.size () - 2);
   
    VectorDouble P1 = Power (Subt (muEnd * wSubSet, muSubSet), 2.0);
    VectorDouble P2 = DotDiv (wSubSet, Subt (1.0, wSubSet));
    VectorDouble sigma2B = DotDiv (P1, P2);
    double  maxSig = sigma2B[0];
    int32   maxSigIdx = 0;
    for  (x = 1;  x < (int32)sigma2B.size ();  ++x)
    {
      if  (sigma2B[x] > maxSig)
      {
        maxSig = sigma2B[x];
        maxSigIdx = x;
      }
    }
      
    //[maxsig,k] = max(sigma2B);
    int32  k = maxSigIdx;
    
    //% segmented image
    //IDX = ones(size(srcImage));
    //IDX(srcImage>pixval(k+1)) = 2;
    int32  threshold = pixval[k + 1];
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    uchar*  resultArea = result->GreenArea ();
    uchar*  srcArea    = srcImage->GreenArea ();
    for  (x = 0;  x < totPixels;  ++x)
    {
      if  (srcArea[x] > threshold)
        resultArea[x] = 2;
      else
        resultArea[x] = 1;
    }
    
    //% separability criterion
    //sep = maxsig/sum(((1:nbins)-mu(end)).^2.*P);
    double  sum = 0.0;
    int32  y = 0;
    muEnd = mu[mu.size () - 1];
    for  (x = 0, y = 1;  x < nbins;  ++x, ++y)
      sum = pow (((double)y - muEnd), 2.0) * P[x];

    sep = maxSig / sum;
    delete  srcImage;
    srcImage = NULL;
    return  result;
  }
    
  if  (numClasses == 3)
  {
    //w0 = w;
    //w2 = fliplr(cumsum(fliplr(P)));
    VectorDouble  w0 = w;
    VectorDouble  w2 = FlipLeftRight (CumSum (FlipLeftRight (P)));


    //[w0,w2] = ndgrid(w0,w2);
    Matrix w0M (w0.size (), w0.size ());
    Matrix w2M (w2.size (), w2.size ());
    NdGrid (w0, w2, w0M, w2M);

    
    //mu0 = mu./w;
    VectorDouble  mu0 = DotDiv (mu, w);

    //mu2 = fliplr(cumsum(fliplr((1:nbins).*P)) ./ cumsum(fliplr(P)));
    //            1      2      34       4   32        3      4 321
    //             ---------- P1 -------------     ------ P2 --------
    VectorDouble  P1 = CumSum (FlipLeftRight (DotMult (BDV (1.0, 1.0, (double)nbins), P)));
    VectorDouble  P2 = CumSum (FlipLeftRight (P));
    VectorDouble  mu2 = FlipLeftRight (DotDiv (P1, P2));

    // TODO  
    //[mu0,mu2] = ndgrid(mu0,mu2);
    Matrix  mu0M (mu0.size (), mu0.size ());
    Matrix  mu2M (mu2.size (), mu2.size ());
    NdGrid (mu0, mu2, mu0M, mu2M);
    
    //w1 = 1-w0-w2;
    Matrix  w1M = 1.0 - w0M - w2M;

    //w1(w1<=0) = NaN;
    MakeNanWhenLesOrEqualZero (w1M);

    //sigma2B =...
    //    w0.*(mu0-mu(end)).^2 + w2.*(mu2-mu(end)).^2 +...
    //    (w0.*(mu0-mu(end)) + w2.*(mu2-mu(end))).^2./w1;
    Matrix  P1M = (DotMult (w0M, Power ((mu0M - muEnd), 2.0)))  +  (DotMult (w2M, Power ((mu2M - muEnd), 2.0)));
    Matrix  P2M = DotDiv (Power ((DotMult (w0M, (mu0M - muEnd)) + DotMult (w2M, (mu2M - muEnd))), 2.0), w1M);
    Matrix  sigma2B = P1M + P2M;


    //sigma2B(isnan(sigma2B)) = 0; % zeroing if k1 >= k2
    ZeroOutNaN (sigma2B);

    //[maxsig,k] = max(sigma2B(:));         % Turns sigma2B into 1D Array then locates largest value and index.
    // [k1,k2] = ind2sub([nbins nbins],k);  % Sets k1 and k2 to the indexes for k mapped into a 2D square matrix that is (nbins x nbins)
    int32  k1, k2;
    double  maxsig = 0.0;
    sigma2B.FindMaxValue (maxsig, k1, k2);
   
    //% segmented image
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    {
      //IDX = ones(size(srcImage))*3;
      //IDX(srcImage<=pixval(k1)) = 1;
      //IDX(srcImage>pixval(k1) & srcImage<=pixval(k2)) = 2;
      uchar*  srcData = srcImage->GreenArea ();
      uchar*  data = result->GreenArea ();
      double  th1 = pixval[k1];
      double  th2 = pixval[k2];
      for  (x = 0;  x < totPixels;  ++x)
      {
        if  (srcData[x] <= th1)
          data[x] = 1;
        else if  (srcData[x] <= th2)
          data[x] = 2;
        else
          data[x] = 3;
      }
    }
    
    //% separability criterion
    //sep = maxsig / sum (((1:nbins)-mu(end)).^2.*P);

    //VectorDouble  xxx = BDV (1.0, 1.0, (double)nbins);
    //VectorDouble  yyy = Subt (xxx, muEnd);
    //VectorDouble  zzz = Power (yyy, 2.0);
    sep = maxsig / Sum (DotMult (Power (Subt (BDV (1.0, 1.0, (double)nbins), muEnd), 2.0), P));
    delete  srcImage;
    srcImage = NULL;
    return  result;
  }
    
  {
    /*
    //k0 = linspace(0,1,numClasses+1);   %  k0 = row vector of linear spaced points between 0 and 1  with (numClasses + 1) points
    VectorDouble  k0 = LinSpace (0, 1, numClasses + 1);

    //k0 = k0(2:numClasses);
    

    [k,y] = fminsearch(@sig_func,k0,optimset('TolX',1));
    k = round(k*(nbins-1)+1);
    
    % segmented image
    IDX = ones(size(srcImage))*numClasses;
    IDX(srcImage<=pixval(k(1))) = 1;
    for i = 1:numClasses-2
        IDX(srcImage>pixval(k(i)) & srcImage<=pixval(k(i+1))) = i+1;
    end
    
    % separability criterion
    sep = 1-y;
    */
  }

  delete  srcImage;
  srcImage = NULL;
    
  return NULL;
}  /* SegmentImage */




/**
 *@brief  Segments image into 'numClasses' taking into account only pixels 
 *        indicated by 'mask' image.
 *@param[in]  srcImage  Image to segment.  If it is a color image will be 
 *                      converted to Grayscale using 'CreateGrayScaleKLTOnMaskedArea'
 *@param[in]  mask  Indictes which pixels to consider when thresholding image.  Pixels 
 *                  that are not part of mask will be assigned label '0'.
 *@param[in]  numClasses Number of classes to segment image into.  Curent only '2' and '3' are supported.
 *@param[out]  sep  
 *@return  Labeled grayscal image where pixels will be labele into their respectice class; betwen '1' and 'numClasses'.
 */
RasterPtr  SegmentorOTSU::SegmentMaskedImage (RasterPtr  srcImage,
                                              RasterPtr  mask,
                                              int32      numClasses,
                                              double&    sep
                                             )
{
  int32  pixelIdx = 0;
  int32  x        = 0;
  bool  isColorImage = srcImage->Color ();

  uchar*  maskArea = NULL;
  uchar   maskTh   = 0;
  if  (mask)
  {
    maskArea = mask->GreenArea ();
    maskTh   = mask->BackgroundPixelTH ();
  }

  if  (numClasses == 1)
  {
    return  NULL;
  }

  else if  ((numClasses < 1)  ||  (numClasses > 255))
  {
    log.Level (-1) << endl << endl
      << "SegmentorOTSU::SegmentMaskedImage  ***ERROR***   'numClasses must be a between 1 and 255 !'" << endl
      << endl;
    sep = 0;
    return NULL;
  }

  if  (isColorImage)
  {
    if  (mask)
      srcImage = srcImage->CreateGrayScaleKLTOnMaskedArea (*mask);
    else
      srcImage = srcImage->CreateGrayScaleKLT ();
  }
  else
  {
    srcImage = new Raster (*srcImage);
  }

  int32  totPixels = srcImage->TotPixels ();
  int32  totMaskPixels = totPixels;
  if  (mask)
    totMaskPixels = mask->TotalBackgroundPixels ();

  VectorInt  unI;
  VectorInt  unICounts;
  
  {
    uchar*  greenArea = srcImage->GreenArea ();

    uchar  pixelMin = greenArea[0];
    uchar  pixelMax = greenArea[0];

    for  (pixelIdx = 1;  pixelIdx < totPixels;  ++pixelIdx)
    {
      if  ((!mask)  ||  (maskArea[pixelIdx] > maskTh))
      {
        if  (greenArea[pixelIdx] < pixelMin)
          pixelMin = greenArea[pixelIdx];

        if  (greenArea[pixelIdx] > pixelMax)
          pixelMax = greenArea[pixelIdx];
      }
    }

    //int32  srcRange = pixelMax - pixelMin + 1;
    VectorInt  counts (256, 0);

    for  (pixelIdx = 0;  pixelIdx < totPixels;  ++pixelIdx)
    {
      if  ((!mask)  ||  (maskArea[pixelIdx] > maskTh))
        counts[greenArea[pixelIdx]]++;
    }

    for  (x = 0;  x < (int32)counts.size ();  ++x)
    {
      if  (counts[x] > 0)
      {
        unI.push_back (x);
        unICounts.push_back (counts[x]);
      }
    }
  }

  int32  nbins = unI.size ();

  if  (nbins <= numClasses)
  {
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    for  (x = 0;  x < nbins;  ++x)
    {
      LabelRaster (result, mask, unI[x], x, srcImage);
    }
    sep = 1;
    delete  srcImage;   
    srcImage = NULL;
    return  result;
  }

  VectorInt  histo  = unICounts;
  VectorInt  pixval = unI;

  VectorDouble  P (histo.size (), 0.0);
  for  (x = 0;  x < (int32)histo.size ();  ++x)
    P[x] = (double)(histo[x]) / (double)totMaskPixels;

  unI.clear ();

  //%% Zeroth- and first-order cumulative moments
  //w = cumsum(P);
  VectorDouble  w (P.size (), 0.0);
  w[0] = P[0];
  for  (x = 1;  x < (int32)P.size (); ++x)
    w[x] = w[x - 1] + P[x];

  //mu = cumsum((1:nbins).*P);
  VectorDouble  mu (P.size (), 0.0);
  mu[0] = 1.0 * P[0];
  for  (x = 1;  x < (int32)P.size ();  ++x)
    mu[x] = mu[x - 1] + ((x + 1) * P[x]);
  double  muEnd = mu[mu.size () - 1];
  
  //%% Maximal sigmaB^2 and Segmented image
  if  (numClasses == 2)
  {
    //sigma2B =...
    //    (mu(end) * w(2:end-1) - mu(2:end-1)) .^2  ./  w(2:end-1)./(1-w(2:end-1));
    //    ------------------- P1 -----------------      ----------- P2 -----------
    //[maxsig,k] = max(sigma2B);

    VectorDouble  wSubSet  = SubSet (w,  1, w.size  () - 2);
    VectorDouble  muSubSet = SubSet (mu, 1, mu.size () - 2);
   
    VectorDouble P1 = Power (Subt (muEnd * wSubSet, muSubSet), 2.0);
    VectorDouble P2 = DotDiv (wSubSet, Subt (1.0, wSubSet));
    VectorDouble sigma2B = DotDiv (P1, P2);
    double  maxSig = sigma2B[0];
    int32   maxSigIdx = 0;
    for  (x = 1;  x < (int32)sigma2B.size ();  ++x)
    {
      if  (sigma2B[x] > maxSig)
      {
        maxSig = sigma2B[x];
        maxSigIdx = x;
      }
    }
      
    //[maxsig,k] = max(sigma2B);
    int32  k = maxSigIdx;
    
    //% segmented image
    //IDX = ones(size(srcImage));
    //IDX(srcImage>pixval(k+1)) = 2;
    int32  threshold = pixval[k + 1];
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    uchar*  resultArea = result->GreenArea ();
    uchar*  srcArea    = srcImage->GreenArea ();
    for  (x = 0;  x < totPixels;  ++x)
    {
      if  ((!maskArea)  ||  (maskArea[x] > maskTh))
      {
        if  (srcArea[x] > threshold)
          resultArea[x] = 2;
        else
          resultArea[x] = 1;
      }
      else
      {
        resultArea[x] = 0;
      }
    }
    
    //% separability criterion
    //sep = maxsig/sum(((1:nbins)-mu(end)).^2.*P);
    double  sum = 0.0;
    int32  y = 0;
    muEnd = mu[mu.size () - 1];
    for  (x = 0, y = 1;  x < nbins;  ++x, ++y)
      sum = pow (((double)y - muEnd), 2.0) * P[x];

    sep = maxSig / sum;
    delete  srcImage;
    srcImage = NULL;
    return  result;
  }
    
  if  (numClasses == 3)
  {
    VectorDouble  w0 = w;
    VectorDouble  w2 = FlipLeftRight (CumSum (FlipLeftRight (P)));

    Matrix w0M (w0.size (), w0.size ());
    Matrix w2M (w2.size (), w2.size ());
    NdGrid (w0, w2, w0M, w2M);
    
    VectorDouble  mu0 = DotDiv (mu, w);

    //mu2 = fliplr(cumsum(fliplr((1:nbins).*P)) ./ cumsum(fliplr(P)));
    //            1      2      34       4   32        3      4 321
    //             ---------- P1 -------------     ------ P2 --------
    VectorDouble  P1 = CumSum (FlipLeftRight (DotMult (BDV (1.0, 1.0, (double)nbins), P)));
    VectorDouble  P2 = CumSum (FlipLeftRight (P));
    VectorDouble  mu2 = FlipLeftRight (DotDiv (P1, P2));

    //[mu0,mu2] = ndgrid(mu0,mu2);
    Matrix  mu0M (mu0.size (), mu0.size ());
    Matrix  mu2M (mu2.size (), mu2.size ());
    NdGrid (mu0, mu2, mu0M, mu2M);
    
    //w1 = 1-w0-w2;
    Matrix  w1M = 1.0 - w0M - w2M;

    //w1(w1<=0) = NaN;
    MakeNanWhenLesOrEqualZero (w1M);

    //sigma2B =...
    //    w0.*(mu0-mu(end)).^2 + w2.*(mu2-mu(end)).^2 +...
    //    (w0.*(mu0-mu(end)) + w2.*(mu2-mu(end))).^2./w1;
    Matrix  P1M = (DotMult (w0M, Power ((mu0M - muEnd), 2.0)))  + (DotMult (w2M, Power ((mu2M - muEnd), 2.0)));
    Matrix  P2M = DotDiv (Power ((DotMult (w0M, (mu0M - muEnd)) +  DotMult (w2M, (mu2M - muEnd))), 2.0), w1M);
    Matrix  sigma2B = P1M + P2M;

    //sigma2B(isnan(sigma2B)) = 0; % zeroing if k1 >= k2
    ZeroOutNaN (sigma2B);

    //[maxsig,k] = max(sigma2B(:));         % Turns sigma2B into 1D Array then locates largest value and index.
    // [k1,k2] = ind2sub([nbins nbins],k);  % Sets k1 and k2 to the indexes for k mapped into a 2D square matrix that is (nbins x nbins)
    int32  k1, k2;
    double  maxsig = 0.0;
    sigma2B.FindMaxValue (maxsig, k1, k2);
   
    //% segmented image
    RasterPtr  result = new Raster (srcImage->Height (), srcImage->Width (), false);
    {
      //IDX = ones(size(srcImage))*3;
      //IDX(srcImage<=pixval(k1)) = 1;
      //IDX(srcImage>pixval(k1) & srcImage<=pixval(k2)) = 2;
      uchar*  srcData = srcImage->GreenArea ();
      uchar*  data = result->GreenArea ();
      double  th1 = pixval[k1];
      double  th2 = pixval[k2];
      for  (x = 0;  x < totPixels;  ++x)
      {
        if  ((!maskArea)  ||  (maskArea[x] > maskTh))
        {
          if  (srcData[x] <= th1)
            data[x] = 1;
          else if  (srcData[x] <= th2)
            data[x] = 2;
          else
            data[x] = 3;
        }
        else
        {
          data[x] = 0;
        }
      }
    }
    
    sep = maxsig / Sum (DotMult (Power (Subt (BDV (1.0, 1.0, (double)nbins), muEnd), 2.0), P));
    delete  srcImage;
    srcImage = NULL;
    return  result;
  }
    
  delete  srcImage;
  srcImage = NULL;
    
  return NULL;
}  /* SegmentMaskedImage */



PixelValue SegmentorOTSU::ClassAverageRGB (const RasterPtr  origImage,
                                           const RasterPtr  segmentedImage,
                                           uchar            segmentedClass
                                          )
{
  uint32  totalRed   = 0;
  uint32  totalGreen = 0;
  uint32  totalBlue  = 0;

  const uchar*  origRed   = origImage->RedArea   ();
  const uchar*  origGreen = origImage->GreenArea ();
  const uchar*  origBlue  = origImage->BlueArea  ();

  const uchar*  mask = segmentedImage->GreenArea ();

  bool  origImageColor = origImage->Color ();

  uint32  totalPixels = (uint32)origImage->TotPixels ();

  for  (uint32 x = 0;  x < totalPixels;  ++x)
  {
    if  (mask[x] == segmentedClass)
    {
      totalGreen += origGreen[x];
      if  (origImageColor)
      {
        totalRed   += origRed  [x];
        totalBlue  += origBlue [x];
      }
    }
  }

  totalGreen = (uchar)(0.5 + (double)totalGreen / (double)totalPixels);
  if  (origImageColor)
  {
    totalRed  = (uchar)(0.5 + (double)totalRed  / (double)totalPixels);
    totalBlue = (uchar)(0.5 + (double)totalBlue / (double)totalPixels);
  }

  return  PixelValue ((uchar)totalRed, (uchar)totalGreen, (uchar)totalBlue);
}  /* AverageRegionRGB */



uchar  SegmentorOTSU::GetClassClosestToTargetColor (const RasterPtr    origImage,
                                                    const RasterPtr    segmentedImage,
                                                    const PixelValue&  targetColor
                                                   )
{
  // We can safely assume there will be at least 2 classes.
  VectorUint32  totalReds   (3, 0);  // Initilized to 3 because vectors are '0' based; index '0' will not be used.
  VectorUint32  totalGreens (3, 0);
  VectorUint32  totalBlues  (3, 0);

  uchar  largestClass = 2;

  const uchar*  origRed   = origImage->RedArea   ();
  const uchar*  origGreen = origImage->GreenArea ();
  const uchar*  origBlue  = origImage->BlueArea  ();

  const uchar*  mask = segmentedImage->GreenArea ();

  bool  origImageColor = origImage->Color ();

  uint32  totalPixels = (uint32)origImage->TotPixels ();

  uchar  classValue = 0;

  for  (uint32 x = 0;  x < totalPixels;  ++x)
  {
    classValue = mask[x];
    while (classValue > largestClass)
    {
      totalReds.push_back   (0);
      totalGreens.push_back (0);
      totalBlues.push_back  (0);
      ++largestClass;
    }

    totalGreens[classValue] += origGreen[x];
    if  (origImageColor)
    {
      totalReds[classValue]  += origRed [x];
      totalBlues[classValue] += origBlue[x];
    }
  }

  VectorFloat distFromTarget (largestClass + 1, 0.0f);

  double  closestDistFound = 99999999.99;
  uchar   closestClass = 255;

  for  (uint x = 1;  x <= largestClass;  ++x)
  {
    double avgRed   = (double)totalReds  [x] / (double)totalPixels;
    double avgGreen = (double)totalGreens[x] / (double)totalPixels;
    double avgBlue  = (double)totalBlues [x] / (double)totalPixels;

    double  deltaRed   = fabs (targetColor.r - avgRed);
    double  deltaGreen = fabs (targetColor.g - avgGreen);
    double  deltaBlue  = fabs (targetColor.b - avgBlue);
    double  distToTarget = sqrt (deltaRed * deltaRed + deltaGreen * deltaGreen + deltaBlue * deltaBlue);

    if  (distToTarget < closestDistFound)
    {
      closestDistFound = distToTarget;
      closestClass = x;
    }
  }

  return  closestClass;
}  /* GetClassClosestToTargetColor */
