/* EigenVector.h  Implements simploified Eigen Vector routine for 2 x 2 matrix 
 * code was derived from "Numerical Recipes in c++"
 */
#ifndef  _EIGENVECTOR_
#define  _EIGENVECTOR_

#include  <vector>

#include "BasicTypes.h"
//#include "Matrix.h"

namespace  KKU
{
  void  Tred2 (int32     n,
               double    a[2][2],
               double*   d,
               double*   e
              );


  void  tqli (int32     n,
              double*   d,
              double*   e,
              double    z[2][2]
             );
}

#endif

