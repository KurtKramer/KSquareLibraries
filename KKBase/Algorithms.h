#if  !defined(_KKB_ALGORITHMS_)
#define _KKB_ALGORITHMS_

#include "KKBaseTypes.h"

namespace KKB
{

template<typename T>
T  FindKthValue (T*      values, 
                 kkint32 arraySize, 
                 kkint32 Kth
                )
{
  T    pv;
  kkint32  left  = 0;
  kkint32  right = arraySize - 1;

  kkint32  pivotIndex = right;

  kkint32  partitionIndex = -1;

  T temp;

  while  (partitionIndex != Kth)
  {
    pv = values[pivotIndex];
    
    partitionIndex = left;
    for  (kkint32 i = left;  i < right;  i++)
    {
      if  (values[i] <= pv)
      {
        if  (i != partitionIndex)
        {
          temp = values[i];
          values[i] = values[partitionIndex];
          values[partitionIndex] = temp;
        }
        partitionIndex = partitionIndex + 1;
      }
    }

    temp = values[partitionIndex];
    values[partitionIndex] = values[right];
    values[right] = temp;

    if  (Kth < partitionIndex)
      right = partitionIndex - 1;
    else
      left  = partitionIndex + 1;

    pivotIndex = right;
  }

  return  values[Kth];
}  /* FindKthValue */



}


#endif
