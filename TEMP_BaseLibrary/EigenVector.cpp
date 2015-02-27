/* EigenVector.cpp  Implements simplified Eigen Vector routine for 2 x 2 matrix 
 * code was derived from "Numerical Recipes in c++"
 */
#include  "FirstIncludes.h"

#include  <stdlib.h>
#include  <stdio.h>

#include  <cmath>
#include  <iostream>
#include  <vector>

#include  "MemoryDebug.h"

#include  "BasicTypes.h"

using namespace std;

#include "EigenVector.h"
#include "BasicTypes.h"
#include "OSservices.h"
using namespace KKU;


//void  Tred2 (Mat_IO_DP &a,
//             Vec_0_DP  &d,
//             Vec_0_DP, &e
//            )

void  KKU::Tred2 (int32     n,
                  double    a[2][2],
                  double*   d,
                  double*   e
                 )
{
  int32  i, j, k, l;

  double  scale, hh, h, g, f;

  for (i = n - 1; i > 0;  i--)
  {
    l = i - 1;
    h = scale = 0.0;

    if  (l > 0)
    {
      for (k = 0;  k < l + 1;  k++)
        scale += fabs (a[i][k]);

      if  (scale == 0.0)
      {
        e[i] = a[i][l];
      }

      else
      {
        for (k = 0;  k < l + 1;  k++)
        {
          a[i][k] /= scale;       // Use scaled a's for transformation
          h+= a[i][k] * a[i][k];  // h = sigma
        }
        
        f = a[i][l];
        g = (f > 0 ? -sqrt(h) : sqrt(h));
        e[i] = scale * g;
        h -= f * g;
        a[i][l] = f - g;
        f = 0.0;

        for  (j = 0;  j < l + 1;  j++)
        {
          a[j][i] = a[i][j] / h;
          g = 0.0;
          for  (k = 0;  k < j + 1;  k++)
            g += a[j][k] * a[i][k];

          for  (k = j + 1;  k < l + 1;  k++)
            g += a[k][j] * a[i][k];

          e[j] = g / h;

          f += e[j] * a[i][j];
        } /* for  (j = 0; */

        hh = f / (h + h);

        for  (j = 0;  j < l + 1;  j++)
        {
          f = a[i][j];
          e[j] = g = e[j] - hh * f;
          for  (k = 0;  k < j + 1;  k++)
            a[j][k] -= (f * e[k] + g * a[i][k]);
        }
      }
    }  /* if  (l > 0) */
    else
    {
      e[i] = a[i][l];
    }

    d[i] = h;
  }

  // We can now calculate Eigen Vectors

  d[0] = 0.0;
  e[0] = 0.0;

  for  (i = 0;  i < n;  i++)
  {
    l = i;

    if  (d[i] != 0.0)
    {
      for  (j = 0;  j < l;  j++)
      {
        g = 0.0;
        for  (k = 0;  k < l;  k++)
          g += a[i][k] * a[k][j];

        for  (k = 0;  k < l;  k++)
          a[k][j] -= g * a[k][i];
      }
    }

    d[i] = a[i][i];

    // Reset row and column of 'a' to identity matrix;
    a[i][i] = 1.0;
    for  (j = 0;  j < l;  j++)
      a[j][i] = a[i][j] = 0.0;
  }  /* for (i) */

}  /* Tred2 */



template<class T>
inline const T SIGN (const T& a,
                     const T& b
                    )
{
  if  (b >= 0)
  {
    if  (a >= 0)
      return  a;
    else
      return -a;
  }

  else
  {
    if  (a >- 0)
      return -a;
    else
      return  a;
  }
}  /* SIGN */


#define  SQR(X)  ((X) * (X))


// Computes sqrt (a^2 + b^2)  without destructive underflow or overflow
double  pythag (const double a,
                const double b
               )
{
  double  absa, absb;

  absa = fabs (a);
  absb = fabs (b);

  if  (absa > absb)
  {
    return  absa * sqrt (1.0 + SQR (absb / absa));
  }
  else
  {
    if  (absb == 0.0)
      return 0.0;
    else 
      return absb * sqrt (1.0 + SQR (absa / absb));
  }
}  /* pythag */




void  KKU::tqli (int32     n,
                 double*   d,
                 double*   e,
                 double    z[2][2]
                )
{
  int32  m, l, iter, i, k;
  double  s, r, p,g, f, dd, c, b;

  for  (i = 1;  i < n;  i++)
    e[i - 1] = e[i];

  e[n - 1] = 0.0;

  for  (l = 0;  l < n;  l++)
  {
    iter = 0;

    do 
    {
      for  (m = l;  m < n - 1;  m++)
      {
        // Looking for a single small sub-diagonal element
        // to split the matrix
        dd = fabs (d[m]) + fabs (d[m + 1]);
        if  (fabs (e[m]) + dd == dd)
          break;
      }

      if  (m != l)
      {
        if  (iter == 100)
        {
          cerr << std::endl << std::endl
               << "EigenVector::tqli    **** ERROR ****            To many iterations in tqli" << std::endl
               << std::endl;
          //osWaitForEnter ();
          //exit (-1);
          return;
        }
        iter++;

        g = (d[l + 1] - d[l]) / (2.0 * e[1]);
        r = pythag (g, 1.0);
        g = d[m] - d[l] + e[l] / (g + SIGN (r, g));
        s = c = 1.0;
        p = 0.0;

        for  (i = m - 1;  i >= l;  i--)
        {
          f = s * e[i];
          b = c * e[i];
          e[i + 1] = (r = pythag (f, g));
          if  (r == 0.0)
          {
            d[i + 1] -= p;
            e[m] = 0.0;
            break;
          }

          s = f / r;
          c = g / r;
          g = d[i + 1] - p;
          r = (d[i] - g) * s + 2.0 * c * b;
          d[i + 1] = g + (p = s * r);
          g = c * r - b;

          // Next loop can be omitted if eigenvectors not wanted

          for  (k = 0;  k < n;  k++)
          {
            f = z[k][i + 1];
            z[k][i + 1] = s * z[k][i] + c * f;
            z[k][i] = c * z[k][i] - s * f;
          }  /* for (k) */

        }  /* for (i) */

        if  ((r == 0.0)  &&  (i >= l))
          continue;

        d[l] -= p;
        e[l] = g;
        e[m] = 0.0;
      }
    } 
    while  (m != l);

  }  /* for (l) */

  return;
}  /* tqli */
