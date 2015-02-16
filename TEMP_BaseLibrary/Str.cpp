/* Str.cpp -- String Management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */
#include "FirstIncludes.h"
#include <algorithm>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

#include "MemoryDebug.h"
using namespace std;


#include "KKException.h"
#include "Str.h"
using namespace KKU;



char*  KKU::STRCOPY (char*        dest,
                     uint16       destSize,
                     const char*  src
                    )
{
# ifdef  USE_SECURE_FUNCS
    strcpy_s  (dest, destSize, src);
# else
    strcpy (dest, src);
# endif
  return  dest;
}  /* STRCOPY */



char*  KKU::STRCOPY (char*        dest,
                     int32        destSize,
                     const char*  src
                    )
{
# ifdef  USE_SECURE_FUNCS
    strcpy_s  (dest, destSize, src);
# else
    strcpy (dest, src);
# endif
  return  dest;
}  /* STRCOPY */



char*  KKU::STRDUP (const char* src)  
{
# ifdef  USE_SECURE_FUNCS
    return _strdup (src);
# else
    return  strdup (src);
# endif
}  /* STRDUP */




char*  KKU::STRCAT (char*        dest, 
                    int32        destSize,
                    const char*  src
                   )
{
# ifdef  USE_SECURE_FUNCS
    strcat_s  (dest, destSize, src);
# else
    strcat (dest, src);
# endif

  return  dest;
}  /* STRCAT */



int32  KKU::STRICMP (const char*  left,
                     const char*  right
                    )
{
  if  (left == NULL)
  {
    if  (right == NULL)
      return 0;
    else
      return -1;
  }
  else if  (!right)
    return 1;

  int32  zed = (toupper (*left)) - (toupper (*right));

  while  ((zed == 0)  &&  (*left != 0))
  {
    ++left;    ++right;
    zed = (toupper (*left)) - (toupper (*right));
  }

  if  (zed < 0)
    return -1;
  else if  (zed == 0)  
    return 0;
  else
    return 1;

}  /* STRICMP */




int32  KKU::STRNICMP (const char*  left,
                      const char*  right,
                      int32        len
                     )
{
  if  (left == NULL)
  {
    if  (right == NULL)
      return 0;
    else
      return -1;
  }
  else if  (!right)
    return 1;

  if  (len < 1)
    return 0;

  int32  x = 0;
  int32  zed = (toupper (*left)) - (toupper (*right));
  while  ((zed == 0)  &&  (*left != 0)  &&  (x < len))
  {
    ++left;    ++right;     ++x;
    zed = (toupper (*left)) - (toupper (*right));
  }

  if  (zed < 0)
    return -1;
  else if  (zed == 0)  
    return 0;
  else
    return 1;
}  /* STRNICMP */



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     int16        right
                    )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
# endif
}



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     uint16       right
                    )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
#endif
}




int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     int32        right
                    )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
# endif
}



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     uint32       right
                    )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
#endif
}



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     int64        right
                    )
{
  
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
# endif
}



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     uint64       right
                    )
{

# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return sprintf (buff, formatSpec, right);
# endif
}



int32  KKU::SPRINTF (char*        buff,
                     int32        buffSize,
                     const char*  formatSpec,
                     int32        precision,
                     double       d
                    )
{
#ifdef  USE_SECURE_FUNCS
  return  sprintf_s (buff, buffSize, formatSpec, precision, d);
#else
  return  sprintf (buff, formatSpec, precision, d);
#endif
}



int32  KKU::SPRINTF (char*         buff,
                     int32         buffSize,
                     char  const*  formatSpec,
                     double        d
                    )
{
  #ifdef  USE_SECURE_FUNCS
  return sprintf_s (buff, buffSize, formatSpec, d);
  #else
  return sprintf (buff, formatSpec, d);
  #endif
}



const char*  KKStr::Str (const char*  s)
{
  if  (!s)
    return "";
  else
    return  s;
}



void  KKStr::StrDelete  (char**  str)
{
  if  (*str)
  {
     delete [] *str;
     *str = NULL;      
  }  
}



int32  KKStr::StrCompareIgnoreCase (const char* s1, 
                                    const char* s2
                                   )
{
  if  (s1 == NULL)
  {
    if  (s2 == NULL)
      return 0;
    else
      return -1;
  }
  else if  (s2 == NULL)
    return 1;


  while  ((*s1)  &&  (*s2)  &&  (toupper (*s1) == toupper (*s2)))
  {
    s1++;
    s2++;
  }

  if  (*s1 == 0)
  {
    if  (*s2 == 0)
    {
      return 0;
    }
    else
    {
      // s1 < s2
      return -1;
    }
  }
  else
  {
    if  (*s2 == 0)
    {
      return 1;
    }
    else
    {
      if  (*s1 < *s2)
        return -1;
      else
        return 1;
    }
  }


  //return  _stricmp (s1, s2);
}  /* StrCompareIgnoreCase */




bool  KKStr::StrEqual (const char* s1,
                       const char* s2
                      )
{
  if  ((!s1) &&  (!s2))
     return  true;

  if  ((!s1)  ||  (!s2))
     return  false;

  return  (strcmp (s1, s2) == 0);
}




bool  KKStr::StrEqualNoCase (const char* s1,
                             const char* s2
                            )
{
  if  ((!s1) &&  (!s2))
     return  true;

  if  ((!s1)  ||  (!s2))
     return  false;

  size_t l1 = strlen (s1);
  size_t l2 = strlen (s2);

  if  (l1 != l2)
    return  false;


  for  (size_t i = 0;  i < l1;  i++)
  {
    if  (toupper (s1[i]) != toupper (s2[i]))
      return false;
  }

  return true;
}  /* StrEqualNoCase */



void  KKStr::StrReplace (char**      dest,  
                         const char* src
                        )
{
  if  (*dest)
     delete [] *dest;

  int32  spaceNeeded;
  
  if  (src)
  {
    spaceNeeded = (int32)strlen (src) + 1;
    *dest = new char[spaceNeeded];
    
    if  (*dest == NULL)
    {
      KKStr errMsg = "KKStr::StrReplace  ***ERROR***   Failed to allocate SpaceNeeded[" + StrFormatInt (spaceNeeded, "#####0") + "].";
      cerr << errMsg << std::endl  << std::endl;
      throw  errMsg;
    }

    STRCOPY (*dest, spaceNeeded, src);
  }
  else
  {
    *dest = new char[1];

    if  (*dest == NULL)
    {
      KKStr  errMsg = "StrReplace   ***ERROR***   Failed to allocate Empty KKStr.";
      cerr << std::endl << errMsg << std::endl;
      throw  errMsg;
    }

    (*dest)[0] = 0;
  }
} /* StrReplace */




KKStrList&  KKStrMatrix::operator[] (uint32 row)
{
  if  (row >= (uint32)data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
      + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }
  return  data[row];
}




KKStrList&  KKStrMatrix::operator() (uint32 row)
{
  if  (row >= (uint32)data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
      + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }
  return  data[row];
}



KKStr&  KKStrMatrix::operator() (uint32  row,
                                 uint32  col
                                )
{
  if  (row >= (uint32)data.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Row dimension[" + StrFormatInt (row, "0") + "] invalid;  Rows Available[" 
                  + StrFormatInt (data.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }

  KKStrList&  rowOfData = data[row];
  if  (col >= (uint32)rowOfData.QueueSize ())
  {
    KKStr  errMsg = "KKStrMatrix::operator[]    Col dimension[" + StrFormatInt (col, "0") + "] invalid;  Rows Available["
                  + StrFormatInt (rowOfData.QueueSize (), "0") + "]";
    cerr << errMsg << std::endl;
    throw  errMsg;
  }

  return  rowOfData[col];
}



/** @brief Add a row of 'KKStr' instances; will take ownership of newRowData */
void  KKStrMatrix::AddRow (KKStrListPtr  newRowData)
{
  while  ((uint32)newRowData->QueueSize () < numCols)
    newRowData->PushOnBack (new KKStr ());
  data.PushOnBack (newRowData);
}



void  KKStrMatrix::AddRow ()  // Will add one row of empty Strings
{
  KKStrListPtr  newRowData = new KKStrList (true, numCols);
  while  ((uint32)newRowData->QueueSize () < numCols)
    newRowData->PushOnBack (new KKStr ());
  data.PushOnBack (newRowData);
}



KKStr::KKStr (): 
   val (NULL)
{
  AllocateStrSpace (10);
  val[0] = 0;
  len = 0;
}



KKStr::KKStr (const char*  str):
        val (NULL)
{
  if  (!str)
  {
    AllocateStrSpace (1);
    val[0] = 0;
    len = 0;
    return;
  }
  
  uint32  newLen = strlen (str);
  AllocateStrSpace (newLen + 1);

  STRCOPY (val, (uint16)allocatedSize, str);

  len = (KKStrInt)newLen;
}



/**
 *@brief Copy Constructor.
 */
KKStr::KKStr (const KKStr&  str): 
        val (NULL)
{
  if  (!str.val)
  {
    AllocateStrSpace (1);
    len = 0;
    return;
  }

  if  (str.val[str.len] != 0)
  {
    cerr << std::endl 
         << std::endl 
         << "KKStr::KKStr    ***ERROR***    Missing terminating NULL" << std::endl
         << std::endl;
  }

  KKStrInt  neededSpace = str.len + 1;
  if  (neededSpace > str.allocatedSize)
  {
    cerr << std::endl;
    cerr << "KKStr::KKStr (const KKStr&  str)   **** ERROR ****" << std::endl;
    cerr << "        AllocatedSize["  << str.allocatedSize << "]  on Source KKStr is to Short" << std::endl;
    cerr << "        for Str[" << str.val << "]." << std::endl;
    exit (-1);
  }

  AllocateStrSpace (str.allocatedSize);
  memcpy (val, str.val, str.len);
  len = str.len;
}




/**
 @brief  Constructs a new KKStr from a pointer to a KKStr.
 */
KKStr::KKStr (const KKStrConstPtr&  str): 
        val (NULL)
{
  if  (!str)
  {
    AllocateStrSpace (1);
    len = 0;
    return;
  }

  if  (!(str->val))
  {
    AllocateStrSpace (1);
    len = 0;
    return;
  }
  
  KKStrInt  neededSpace = str->len + 1;

  if  (neededSpace > str->allocatedSize)
  {
    cerr << std::endl;
    cerr << "KKStr::KKStr (const KKStr&  str)   **** ERROR ****" << std::endl;
    cerr << "        AllocatedSize["  << str->allocatedSize << "]  on Source KKStr is to Short" << std::endl;
    cerr << "        for Str[" << str->val << "]." << std::endl;
    throw "KKStr  Constructor (const KKStrConstPtr&  str)   'AllocatedSize' on source is too small.";
  }

  AllocateStrSpace (str->allocatedSize);

  STRCOPY (val, allocatedSize, str->val);

  len = str->len;
}



/**
 *@brief Creates a String that is populated with 'd' as displayable characters and precision of 'precision'.
 */
KKStr::KKStr (double  d, 
              int32   precision
             ):
    val (NULL)
{
  char  buff[60];

  if  ((precision < 0)  ||  (precision > 10))
  {
    SPRINTF (buff, sizeof (buff), "%f", d);
  }
  else
  {
    SPRINTF (buff, sizeof (buff), "%.*f", precision, d);
  }

  KKStrInt  newLen = (KKStrInt)strlen (buff); 
  AllocateStrSpace (newLen + 1);

  STRCOPY (val, allocatedSize, buff);

  len = newLen;
}



//KKStr::KKStr (KKStr   str): 
//        val (NULL)
//{
//  StrReplace (&val, str.val);
//}




/**
 *@brief Creates a KKStr object that has 'size' charaters preallocatd; and set to empty string.
 */
KKStr::KKStr (int32  size):
        val (NULL)
{
  if  (size <= 0)
    size = 1;

  else if  ((uint32)size >= StrIntMax)
  {
    cerr  << std::endl 
          << "KKStr::KKStr    ***WARNNING***   Trying to allocate Size[" << size << "]  which is >= StrIntMax[" << StrIntMax << "]." << std::endl
          << std::endl;
    size = StrIntMax - 1;
  }

  AllocateStrSpace (size);
  val[0] = 0;
  len = 0;
}




KKStr::KKStr (const std::string&  s):
        allocatedSize (0),
        len (0),
        val (NULL)
{
  AllocateStrSpace ((int32)(s.size () + 1));

  len = (KKStrInt)s.size ();
  for  (int32 x = 0;  x < len;  ++x)
    val[x] = s[x];
  val[len] = 0;
}


/** @brief  Constructs a KKStr instance fomm a substr of 'src'.  */
KKStr::KKStr (const char*  src,
              uint32       startPos,
              uint32       endPos
             ):
        allocatedSize (0),
        len (0),
        val (NULL)
{
  if  (startPos > endPos)
  {
    AllocateStrSpace (1);
    return;
  }

  uint32  subStrLen = 1 + endPos - startPos;
  if  (subStrLen > (StrIntMax - 1))
  {
    cerr << "KKStr::KKStr   ***ERROR***  requested SubStr[" << startPos << ", " << endPos << "]  len[" << subStrLen << "] is greater than StrIntMax[" << (StrIntMax - 1) << "]" << std::endl;
    endPos = (startPos + StrIntMax - 2);
    subStrLen = 1 + endPos - startPos;
  }

  AllocateStrSpace (1 + subStrLen);             // Need one extra byte for NULL terminating chracter.

  memcpy (val, src + startPos, subStrLen);
  len = (KKStrInt)subStrLen;
  val[subStrLen] = 0;
}



void  KKStr::AllocateStrSpace (uint32  size)
{
  if  (size < 1)
    size = 1;

  if  (val)
  {
    cerr << std::endl
         << "KKStr::AllocateStrSpace   ***ERROR***      Previous val was not deleted." 
         << std::endl;
  }

  if  (size >= StrIntMax)
  {
    //  Can not allocate this much space;  This string has gotten out of control.
    cerr << "KKStr::AllocateStrSpace   ***ERROR***      Size["  << size << "] is larger than StrIntMax[" << StrIntMax << "]" << std::endl;
    KKStr  errStr (150);
    errStr << "KKStr::AllocateStrSpace   ***ERROR***      Size["  << size << "] is larger than StrIntMax[" << StrIntMax << "]";
    throw  KKException (errStr);
  }

  val = new char[size];
  if  (val == NULL)
  {
    cerr << std::endl;
    cerr << "KKStr::AllocateStrSpace  ***ERROR***"  << std::endl;
    cerr << "Could not allocate Memory for KKStr, size[" << size << "]." << std::endl;
    throw  "KKStr::AllocateStrSpace    Allocation of memory failed.";
  }

  memset (val, 0, size);

  val[0] = 0;
  allocatedSize = (KKStrInt)size;
  len = 0;
}  /* AllocateStrSpace */



int32  KKStr::MemoryConsumedEstimated () const  
{
  return sizeof (char*) + 2 * sizeof (KKStrInt) + allocatedSize;
}



void  KKStr::GrowAllocatedStrSpace (uint32  newAllocatedSize)
{
  if  (newAllocatedSize < allocatedSize)
  {
    KKStr  errMsg (128);
    errMsg << "KKStr::GrowAllocatedStrSpace  ***ERROR***" << "  newAllocatedSize[" << newAllocatedSize << "]  is smaller than allocatedSize[" << allocatedSize << "]";
    cerr  << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }

  if  (newAllocatedSize >= (StrIntMax - 5))
  {
    //  Can not allocate this much space;  This strig has gotten out of control.
    cerr << std::endl
         << "KKStr::GrowAllocatedStrSpace   ***ERROR***      NewAllocatedSize["  << newAllocatedSize << "] is larger than StrIntMax[" << (StrIntMax - 5) << "]" << std::endl
         << std::endl;
    newAllocatedSize = StrIntMax - 6;
  }

  newAllocatedSize += 5;  // Lets allocate a little extra space on the hope that we will save a lot of cycles reallocating again this string.

  if  (val == NULL)
  {
    val = new char[newAllocatedSize];
    memset (val, 0, newAllocatedSize);
    allocatedSize = (KKStrInt)newAllocatedSize;
  }
  else
  {
    char*  newVal = new char[newAllocatedSize];
    memset (newVal, 0, newAllocatedSize);
    memcpy (newVal, val, allocatedSize);
    delete  val;
    val = newVal;
    allocatedSize = (KKStrInt)newAllocatedSize;
  }
}  /* GrowAllocatedStrSpace */



KKStr::~KKStr ()
{
  if  (val)
  {
    delete [] val;
    val = NULL;
  }
}



int32  KKStr::Compare (const KKStr&  s2)  const
{
  int32  zed = Min (len, s2.len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.val;

  for  (int32 x = 0;  x < zed;  x++)
  {
    if  ((*s1Ptr) < (*s2Ptr))
      return -1;

    else if  ((*s1Ptr) > (*s2Ptr))
      return 1;

    s1Ptr++;
    s2Ptr++;
  }

  if  (len == s2.len)
    return 0;

  else if  (len < s2.len)
    return -1;

  else 
    return 1;

}  /* Compare */



/**
 *@Brief  Compares with STL string.
 *@param[in]  s2  STL String  std::string that we will compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
int32  KKStr::Compare (const std::string&  s2)  const
{
  KKStrInt  s2Len = (KKStrInt)s2.size ();
  int32  zed = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (int32 x = 0;  x < zed;  x++)
  {
    if  ((*s1Ptr) < (*s2Ptr))
      return -1;

    else if  ((*s1Ptr) > (*s2Ptr))
      return 1;

    s1Ptr++;
    s2Ptr++;
  }

  if  (len == s2Len)
    return 0;

  else if  (len < s2Len)
    return -1;

  else 
    return 1;

}  /* Compare */



/**
 *@Brief  Compares with another KKStr, ignoring case.
 *@param[in]  s2  Other String to compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
int32  KKStr::CompareIgnoreCase (const KKStr&  s2)  const
{
  int32  zed = Min (len, s2.len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.val;

  for  (int32 x = 0;  x < zed;  x++)
  {
    if  (toupper (*s1Ptr) < toupper (*s2Ptr))
      return -1;

    else if  (toupper (*s1Ptr) > toupper (*s2Ptr))
      return 1;

    s1Ptr++;
    s2Ptr++;
  }

  if  (len == s2.len)
    return 0;

  else if  (len < s2.len)
    return -1;

  else 
    return 1;
}  /* CompareIgnoreCase */



/**
 *@Brief  Compares with ascii-z string ignoring case.
 *@param[in]  s2  Ascii-z string to compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
int32  KKStr::CompareIgnoreCase (const char* s2)  const
{
  if  (s2 == NULL)
  {
    if  (len == 0)
      return  0;
    else
      return  1;
  }

  uint32  s2Len = 0;
  if  (s2 != NULL)
    s2Len = strlen (s2);
  uint32  zed = Min ((uint32)len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2;

  for  (KKStrInt x = 0;  x < zed;  x++)
  {
    if  (toupper (*s1Ptr) < toupper (*s2Ptr))
      return -1;

    else if  (toupper (*s1Ptr) > toupper (*s2Ptr))
      return 1;

    s1Ptr++;
    s2Ptr++;
  }

  if  (len == s2Len)
    return 0;

  else if  (len < s2Len)
    return -1;

  else 
    return 1;
}  /* CompareIgnoreCase */




int32  KKStr::CompareIgnoreCase (const std::string&  s2)  const
{
  KKStrInt  s2Len = (KKStrInt)s2.size ();
  int32  zed = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (int32 x = 0;  x < zed;  x++)
  {
    if  (toupper (*s1Ptr) < toupper (*s2Ptr))
      return -1;

    else if  (toupper (*s1Ptr) > toupper (*s2Ptr))
      return 1;

    s1Ptr++;
    s2Ptr++;
  }

  if  (len == s2Len)
    return 0;

  else if  (len < s2Len)
    return -1;

  else 
    return 1;

}  /* CompareIgnoreCase */



KKStr  KKStr::Concat(const char**  values)
{
  if  (values == NULL)
    return "";

  uint32  len = 0;
  int32  x = 0;
  while  (values[x] != NULL)
  {
    len += (uint32)strlen (values[x]);
    x++;
  }

  KKStr  result (len);
  x = 0;
  while  (values[x] != NULL)
  {
    result.Append (values[x]);
    x++;
  }

  return  result;
}  /* Concat */



KKStr  KKStr::Concat (const VectorKKStr&  values)
{
  uint32 x   = 0;
  int32 len  = 0;

  for  (x = 0;  x < values.size ();  x++)
    len += (int32)values.size ();

  KKStr  result (len);
  x = 0;
  for  (x = 0;  x < values.size ();  x++)
  {
    result.Append (values[x]);
    x++;
  }

  return  result;
}  /* Concat */



/** 
 *@brief Concatenates the list of 'std::string' strings.
 *@details  Iterates through values Concatenating each one onto a result string.
 */
KKStr  KKStr::Concat (const std::vector<std::string>&  values)
{
  uint32 x   = 0;
  int32 len  = 0;

  for  (x = 0;  x < values.size ();  x++)
    len += (int32)values.size ();

  KKStr  result (len);
  x = 0;
  for  (x = 0;  x < values.size ();  x++)
  {
    result.Append (values[x]);
    x++;
  }

  return  result;
}  /* Concat */



bool  KKStr::Contains (const KKStr& value)    
{
  if  (value.Empty ())
    return true;
  else
    return StrInStr (value);
}


bool  KKStr::Contains (const char*  value)
{
  if  ((value == NULL)  ||  (*value == 0))
    return true;
  else
    return (Find (value, 0) >= 0);
}



bool   KKStr::EndsWith (const KKStr& value)
{
  return  EndsWith (value, true);
}



bool   KKStr::EndsWith (const char* value)
{
  return  EndsWith (value, true);
}



bool   KKStr::EndsWith (const KKStr& value,   
                        bool  ignoreCase
                       )
{
  if  (value.len == 0)
    return true;

  int32  startPos = 1 + len - value.Len ();
  if  (startPos < 0)
    return false;

  if  (ignoreCase)
    return  StrEqualNoCase (val + startPos, value.val);
  else
    return  StrEqual  (val + startPos, value.val);
}



bool   KKStr::EndsWith (const char* value,   
                        bool ignoreCase
                       )
{
  if  (value == NULL)
    return true;

  int32  valueLen = (int32)strlen (value);

  int32  startPos = 1 + len - valueLen;
  if  (startPos < 0)
    return false;

  if  (ignoreCase)
    return  StrEqualNoCase (val + startPos, value);
  else
    return  StrEqual (val + startPos, value);
}



bool  KKStr::StartsWith (const KKStr&  value)  const
{
  uint32  valueLen = value.Len ();

  if  (valueLen < 1)     return true;
  if  (len < valueLen)   return false;
  return  (strncmp (val, value.val, valueLen) == 0);
}


bool  KKStr::StartsWith (const char*  value)  const
{
  if  (value == NULL)
    return true;
  
  uint32  valueLen = strlen (value);
  if  (valueLen < 1)
    return true;

  if  (len < valueLen)
    return false;

  return  (strncmp (val, value, valueLen) == 0);
}  /* StartsWith */



bool  KKStr::StartsWith (const KKStr&  value,   
                         bool          ignoreCase
                        ) const
{
  uint32  valueLen = value.Len ();
  if  (len < valueLen)
    return false;

  if  (valueLen < 1)
    return true;

  if  (ignoreCase)
    return  (strncmp (val, value.val, valueLen) == 0);
  else
    return  (STRNICMP (val, value.val, valueLen) == 0);
}  /* StartsWith */




bool  KKStr::StartsWith (const char*  value,   
                         bool         ignoreCase
                        ) const
{
  if  (value == NULL)
    return true;

  uint32  valueLen = strlen (value);
  if  (valueLen < 1)
    return true;

  if  (len < valueLen)
    return false;

  if  (ignoreCase)
    return  (strncmp (val, value, valueLen) == 0);
  else
    return  (STRNICMP (val, value, valueLen) == 0);
}  /* StartsWith */



bool  KKStr::EqualIgnoreCase (const KKStr&  s2)  const
{
  return  (CompareIgnoreCase (s2) == 0);
}  /* EqualIgnoreCase */



bool  KKStr::EqualIgnoreCase (const char* s2)  const
{
  return  (StrCompareIgnoreCase (val, s2) == 0);
}  /* EqualIgnoreCase */



wchar_t*  KKStr::StrWide ()  const
{
  wchar_t*  w = NULL;

  if  (!val)
  {
    w = new wchar_t[1];
    w[0] = 0;
    return w;
  }

  int32  x;
  w = new wchar_t[len + 1];
  for  (x = 0; x < len;  x++)
    w[x] = (wchar_t) val[x];
  return w;
}  /* StrWide */




void  KKStr::ValidateLen ()  const
{
  if  (!val)
  {
    if  (len < 1)
    {
      return;
    }
    else
    {
      cerr << std::endl
           << std::endl
           << std::endl
           << "        *** ERROR ***" << std::endl
           << std::endl
           << "'KKStr::ValidateLen'  Something has gone very Wrong with the KKStr Library." << std::endl
           << std::endl
           << "len[" << len << "]" << std::endl
           << "strlen (val)[" << strlen (val) << "]" << std::endl
           << std::endl
           << std::endl
           << "Press Enter to Continue." << std::endl;

      char buff[100];
      cin >> buff;
    }
  }

  if  (val[len] != 0)
  {
    cerr << std::endl 
         << std::endl
         << "'KKStr::ValidateLen'  Something has gone very Wrong with the KKStr Library." << std::endl
         << std::endl
         << "len[" << len << "]" << std::endl
         << "strlen (val)[" << strlen (val) << "]" << std::endl
         << std::endl
         << std::endl
         << "Press Enter to Continue." << std::endl;
  }
}




KKStr&  KKStr::operator= (const KKStr&  src)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  src.ValidateLen ();
  #endif

  if  (&src == this)
  {
    // We are assigning our selves to our selves;  there is nothing to do.
    return *this;
  }

  KKStrInt  spaceNeeded = src.len + 1;
  if  ((spaceNeeded > allocatedSize)  ||  (!val))
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
  }
  else
  {
    memset (val, 0, allocatedSize);
  }

  if  (src.val)
    memcpy (val, src.val, src.len);

  len = src.len;
  val[len] = 0;

  return *this;
}






//KKStr&  KKStr::operator= (KKStr  src)
//{
//  StrReplace (&val, src.val);
//  return *this;
//}





KKStr&  KKStr::operator= (const char* src)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!src)
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (10);
    len = 0;
    return *this;
  }

  KKStrInt  newLen = (KKStrInt)strlen (src);
  KKStrInt  spaceNeeded = newLen + 1;

  if  (spaceNeeded > allocatedSize)
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
  }

  STRCOPY (val, allocatedSize, src);
  len = newLen;
 
  return *this;
}




KKStr&  KKStr::operator= (int32  right)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%d", right);

  KKStrInt  newLen = (KKStrInt)strlen (buff);

  KKStrInt  spaceNeeded = newLen + 1;

  if  (spaceNeeded > allocatedSize)
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
  }

  memset (val, 0, allocatedSize);

  STRCOPY (val, allocatedSize, buff);
  len = newLen;

  return *this;
}



KKStr&  KKStr::operator= (const vector<KKStr>& right)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  int32  spaceNeeded = 2;  /* Start with 2 bytes for overhead. */
  uint32  x = 0;
  for  (x = 0;  x < right.size ();  x++)
    spaceNeeded = right[x].Len ();

  if  (spaceNeeded > allocatedSize)
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
  }

  char*  ptr = val;
  int32  allocatedSpaceNotUsed = allocatedSize - 1;
  for  (x = 0;  x < right.size ();  x++)
  {
    int32  rightLen = right[x].Len ();
#ifdef  USE_SECURE_FUNCS
    strncpy_s (ptr, allocatedSpaceNotUsed, right[x].Str (), rightLen);
#else
    strncpy   (ptr, right[x].Str (), rightLen);
#endif
    ptr = ptr + rightLen;
    allocatedSpaceNotUsed -= rightLen;
    *ptr = 0;
  }
  return  *this;
}



bool  KKStr::operator== (const KKStr& right)  const
{
  return  (Compare (right) == 0);
}




bool  KKStr::operator!= (const KKStr& right)  const
{
  return  (Compare (right) != 0);
}




bool  KKStr::operator== (const KKStrConstPtr& right)  const
{
  if  (!right)
    return false;

  return  (Compare (*right) == 0);
}




bool  KKStr::operator!= (const KKStrConstPtr&  right)  const
{
  if  (!right)
    return true;

  return  (Compare (*right) != 0);
}




bool  KKStr::operator== (const char*  rtStr)  const
{
  return  StrEqual (val, rtStr);
}




bool  KKStr::operator!= (const char*  rtStr)  const
{
  return  (!StrEqual (val, rtStr));
}



bool  KKStr::operator== (const std::string right) const
{
  return  (Compare (right) == 0);
}




bool  KKStr::operator!= (const std::string right) const
{
  return  (Compare (right) != 0);
}




bool  KKStr::operator> (const KKStr& right)  const
{
  return  (Compare (right) > 0);
}




bool  KKStr::operator>= (const KKStr& right)  const
{
  return  (Compare (right) >= 0);
}




bool  KKStr::operator< (const KKStr& right)  const
{
  return  (Compare (right) < 0);
}



bool  KKStr::operator<= (const KKStr& right)  const
{
  return  (Compare (right) <= 0);
}



void  KKStr::ChopLastChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)  return;

  if  (len > 0)
  {
    len--;
    val[len] = 0;
  }
}  /* ChopLastChar */



void  KKStr::ChopFirstChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)  return;

  if  (len > 0)
  {
    for  (int x = 0;  x < len;  ++x)
      val[x] = val[x + 1];
    len--;
    val[len] = 0;
  }
}  /* ChopLastChar */



KKStr&  KKStr::TrimRight (const char* whiteSpaceChars)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
  {
    AllocateStrSpace (1);
    len = 0;
    return *this;
  }

  int32  x = len - 1;

  while  ((len > 0)  && (strchr (whiteSpaceChars, val[x])))
  {
    val[x] = 0;
    x--;
    len--;
  }

  return *this;
}  /* TrimRight */





void  KKStr::TrimRightChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
  {
    AllocateStrSpace (1);
    len  = 0;
    return;
  }

  if  (len > 0)
  {
    len--;
    val[len] = 0;
  }
}  /* TrimRightChar */



void  KKStr::TrimLeft (const char* whiteSpaceChars)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif


  if  (!val)
  {
    AllocateStrSpace (1);
    len = 0;
    return;
  }

  KKStrInt  x = 0;

  while  ((strchr (whiteSpaceChars, val[x]))  &&  (val[x] != 0))
    x++;

  if  (x == 0)
    return;

  KKStrInt  y = 0;

  while  (x < len)
  {
    val[y] = val[x];
    x++;
    y++;
  }

  len = y;
  val[len] = 0;
}  /* TrimLeft */




void  KKStr::Append (const char* buff)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!buff)  
    return;

  uint32  buffLen = strlen (buff);
  uint32  newLen = len + buffLen;
  uint32  neededSpace = newLen + 1;

  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= StrIntMax)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                buffLen[" << buffLen         << "]" << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      return;
    }
    GrowAllocatedStrSpace (neededSpace);
  }

  uint32  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }
  val[len] = 0;
}  /* Append */



void  KKStr::Append (const char*   buff,
                           uint32  buffLen
                    )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (buffLen == 0)  
    return;

  uint32  newLen = len + buffLen;
  uint32  neededSpace = newLen + 1;

  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= StrIntMax)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                buffLen[" << buffLen         << "]" << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      return;
    }
    GrowAllocatedStrSpace (neededSpace);
  }

  uint32  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }

  val[len] = 0;
}  /* Append*/






void  KKStr::Append (char ch)
{
  uint32  neededSpace = len + 2;
  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= StrIntMax)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      return;
    }
    GrowAllocatedStrSpace (neededSpace);
  }
  val[len] = ch;
  len++;
  val[len] = 0;
}  /* Append */





void  KKStr::Append (const KKStr&  str)
{
  Append (str.val, str.len);
}



void  KKStr::Append (const std::string&  str)
{
  Append (str.c_str ());
}



void  KKStr::AppendInt32 (int32  i)
{
  if  (i == 0)
  {
    Append ('0');
    return;
  }

  char  buff[20];
  int16  bi = sizeof (buff) - 1;
  buff[bi] = 0;

  bool  negative = false;
  if  (i < 0)
  {
    negative = true;
    i = 0 - i;
  }

  while  (i > 0)
  {
    --bi;
    int16  digit = i % 10;
    i = i / 10;
    buff[bi] = '0' + (char)digit;
  }

  if  (negative)
  {
    --bi;
    buff[bi] = '-';
  }

  Append (buff + bi);
  return;
}  /* AppendInt32 */



void  KKStr::AppendUInt32 (uint32  i)
{
  if  (i == 0)
  {
    Append ('0');
    return;
  }

  char  buff[20];
  int16  bi = sizeof (buff) - 1;
  buff[bi] = 0;

  while  (i > 0)
  {
    --bi;
    int16  digit = i % 10;
    i = i / 10;
    buff[bi] = '0' + (char)digit;
  }

  Append (buff + bi);
  return;
}  /* AppendUInt32 */







char  KKStr::FirstChar ()  const
{
  if  (val == NULL)
    return 0;

  return  val[0];
}




char  KKStr::LastChar ()  const
{
  if  (!val)
    return 0;

  if  (val[0] == 0)
    return 0;
  else
    return val[len - 1];
}  /* LastChar */




int32  KKStr::LocateCharacter (char  ch)  const
{
  if  (!val)
    return  -1;

  int32  idx    = 0;

  while  (idx < len)
  {
    if  (val[idx] == ch)
      return  idx;
    idx++;
  }

  return  -1;
}  /* LocateCharacter */




int32  KKStr::InstancesOfChar (char ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (val == NULL)
    return  0;

  int32  count = 0;

  for  (KKStrInt x = 0; x < len;  x++)
  {
    if  (val[x] == ch)
      count++;
  }

  return  count;
}  /* InstancesOfChar */



int32  MemCompare (const char* s1,
                   const char* s2,
                   int32       s1Idx,
                   int32       s2Idx,
                   int32       len
                )
{
  for  (int32 x = 0;  x < len;  x++)
  {
    if  (s1[s1Idx] < s2[s2Idx])
      return -1;
    
    else if  (s1[s1Idx] > s2[s2Idx])
      return 1;

    s1Idx++;
    s2Idx++;
  }

  return 0;
}  /* MemCompare */



/**
 *@brief returns the position of the 1st occurrence of the string 'searchStr'.
 *@details A return of -1 indicates that there is no occurrence of 'searchStr' in the string.
 */
int32  KKStr::LocateStr (const KKStr&  searchStr)  const
{
  if  ((!val)  ||  (!(searchStr.val)))
    return  -1;

  int32  idx = 0;

  int32  lastIdx = len - searchStr.len;

  while  (idx <= lastIdx)
  {
    if  (MemCompare (val, searchStr.val, idx, 0, searchStr.len) == 0)
      return idx;
    idx++;
  }

  return -1;
}; // LocateStr

  



/**
 *@brief  Returns the position of the last occurrence of the character 'ch'.
 *@details A return of -1 indicates that there is no occurrence of 'ch' in the string.
 */
int32  KKStr::LocateLastOccurrence (char  ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return -1;

  bool   found  = false;
  int32  idx    = len - 1;

  while  ((idx >= 0)  &&  (!found))
  {
    if  (val[idx] == ch)
      found = true;
    else
      idx--;
  }

  if  (found)
    return  idx;
  else
    return  -1;
}  /* LocateLastOccurrence */




/**
 *@brief  Returns the position of the last occurrence of the string 's'.
 *@details A return of -1 indicates that there is no occurrence of 's' in the string.
 */
int32  KKStr::LocateLastOccurrence (const KKStr&  s)  const
{
  int32  sLen = s.Len ();

  if  ((!val)  ||  (!s.val)  ||  (sLen <= 0)  ||  (sLen > len))
    return -1;

  bool  found  = false;
  int32 idx    = len - sLen;

  char*  sVal = s.val;

  while  ((idx >= 0)  &&  (!found))
  {
    if  (strncmp (val + idx, sVal, sLen) == 0)
      found = true;
    else
      idx--;
  }

  if  (found)
    return  idx;
  else
    return  -1;
}  /* LocateLastOccurrence */





KKStr  KKStr::Wide (int32 width,
                    char  dir
                   ) const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif


  KKStr  str (val);
  if  ((dir == 'L')  ||  (dir == 'l'))
     str.LeftPad  (width, ' ');

  else if  ((dir == 'C')  ||  (dir == 'c'))
  {
    str.TrimRight ();
    str.TrimLeft ();

    int32  x = (width - str.Len ()) / 2;

    if  (x > 0)
    {
      str = Spaces (x).Str () + str;
      str.RightPad (width, ' ');
    }
  }

  else
    str.RightPad (width, ' ');

  return str;
}  /* Wide */




void  KKStr::RightPad (int32  width,
                       char ch
                      )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (width < 0)
  {
    cerr << std::endl;
    cerr << "KKStr::RightPad (int32  width,  char ch)    **** ERROR ****" << std::endl;
    cerr << "                width[" << width << "]  invalid." << std::endl;
    cerr << std::endl;
    exit (-1);
  }

  if  (!val)
  {
    AllocateStrSpace (width + 1);
    len = 0;
  }
 
  if  (len > (KKStrInt)width)
  {
    len = (KKStrInt)width;
    for  (int32 x = len;  x < allocatedSize;  x++)
      val[x] = 0;
  }

  else
  {
    uint32  neededSpace = width + 1;

    if  (neededSpace > allocatedSize)
    {
      if  (neededSpace >= StrIntMax)
      {
        cerr << std::endl 
             << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
             << "                neededSpace[" << neededSpace << "]" << std::endl
             << std::endl;
        return;
      }
      GrowAllocatedStrSpace (neededSpace); 
    }
   
    while (len < (KKStrInt)width)
    {
      val[len] = ch;
      len++;
    }

    val[len] = 0;
  }
}  /* RightPad */




void  KKStr::LeftPad (int32 width, 
                      uchar ch
                     )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (width < 0)
  {
    cerr << std::endl;
    cerr << "KKStr::LeftPad (int32  width,  char ch)    **** ERROR ****" << std::endl;
    cerr << "                width[" << width << "]  invalid." << std::endl;
    cerr << std::endl;
    width = 0;
  }

  if  (!val)
  {
    len = 0;
    allocatedSize = 0;
  }

  uint32  neededSpace = (uint32)width + 1;
  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= StrIntMax)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      return;
    }
    GrowAllocatedStrSpace (neededSpace); 
  }

  if  (len >= (KKStrInt)width)
  {
    // 2010-04-20
    // This code has never been debugged.  So the first time we run it
    // we want to make sure that it is doing what I say it is doing.
    /** @todo  Need to properly debug thru 'KKStr::LeftPad; */
    KKStrInt  toIdx    = 0;
    KKStrInt  fromIdx  = len - (KKStrInt)width;
    while  (fromIdx < len)
    {
      val[toIdx] = val[fromIdx];
      val[fromIdx] = 0;
      ++toIdx;
      ++fromIdx;
    }
    len = (KKStrInt)width;
    val[len] = 0;
  }
  else
  {
    int32  fromIdx = len - 1;
    int32  toIdx   = width - 1;
    while  (fromIdx >= 0)
    {
      val[toIdx] = val[fromIdx];
      --fromIdx;
      --toIdx;
    }

    while  (toIdx >= 0)
    {
      val[toIdx] = ch;
      --toIdx;
    }

    len = (KKStrInt)width;
    val[len] = 0;
  }
  return;
}  /* LeftPad */





char  KKStr::EnterStr ()
{
  int32 bp = 0;
  char  buff[256];
  uchar  ch;
  
  ch = (uchar)getchar ();
  if  (ch == 10)
    ch = EnterChar;

  while  ((ch != EnterChar) && (ch != EscapeChar))
    {
      if  (ch == 8)
        {
          // Back Space 
          if  (bp > 0)  {
             bp--;
             buff[bp] = 0;
             putchar (ch);
            }
        }
      
      else if  (ch == 0)
        {
          // We have a control chracter.
          ch = (uchar)getchar ();
        }

      else
        {
          // putchar (ch);
          buff[bp] = ch;
          bp++;
        }

      ch = (uchar)getchar ();
      if  (ch == 10)
        ch = EnterChar;
    }

  buff[bp] = 0;

  KKStrInt  newLen = (KKStrInt)strlen (buff);

  KKStrInt  neededSpace = newLen + 1;

  if  (neededSpace > allocatedSize)
  {
    delete  [] val;
    val = NULL;
    AllocateStrSpace (neededSpace);
  }

  STRCOPY (val, allocatedSize, buff);
  len = newLen;

  return  ch;
}  /* EnterStr */





/**
 *@brief  Converts all characters in string to their Upper case equivalents via 'toupper'.
 *@see ToUpper
 */
void  KKStr::Upper ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return;

  uint32  x;

  for  (x = 0; x < len; x++)
    val[x] = (uchar)toupper (val[x]);
}  /* Upper */



/**
 *@brief  Converts all characters in string to their Lower case equivalents via 'tolower'.
 *@see ToLower
 */
void  KKStr::Lower ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return;

  uint32  x;

  for  (x = 0; x < len; x++)
    val[x] = (uchar)tolower (val[x]);
}  /* Lower */



KKStr  KKStr::MaxLen (int32  maxLen)  const
{
  maxLen = Max ((int32)0, maxLen);
  if  (len < maxLen)
    return *this;
  else
    return SubStrPart (0, maxLen - 1);
}



KKStr  KKStr::ToUpper ()  const
{
  if  (!val)
    return "";

  KKStr  upperStr (*this);
  upperStr.Upper ();
  return  upperStr;
}  /* ToUpper */



KKStr  KKStr::ToLower ()  const
{
  if  (!val)
    return "";

  KKStr  lowerStr (*this);
  lowerStr.Lower ();
  return  lowerStr;
}  /* ToLower */





bool  KKStr::ValidInt (int32  &value)
{

  int32  sign = 1;

  value = 0;

  if  (!val)
     return false;
  else
    {
       char*  ch = val;

       // Skip over white space

       while  ((strchr (" \n\t", *ch))  &&  (*ch))
         ch++;

       if  (!(*ch))
          return  false;

       if  (*ch == '-')
         {
           ch++;
           sign = -1;
         }

       int32  digit;
               
       digit = (*ch - '0');

       while  ((digit >= 0)  &&  (digit <= 9))
         {
            value = value * 10 + digit;
            ch++;
            digit = (*ch - '0');
         }

       value = value * sign;

       return  (*ch == 0);
    }
}



bool  KKStr::ValidMoney (float  &value)  const
{
  int32  digit = 0;
  int32  sign  = 1;
  value = 0;
  if  (!val)
    return false;

  char*  ch = val;

  // Skip over white space
  while  ((strchr (" \n\t", *ch))  &&  (*ch))
    ch++;
  if  (!(*ch))
    return  false;

  bool  decimalFound  = false;
  int32 decimalDigits = 0;

  if  (*ch == '-')
  {
    ch++;
    sign = -1;
  }

  digit = (*ch - '0');
  while  (((digit >= 0)  &&  (digit <= 9))  ||  (*ch == '.'))
  {
   if  (*ch == '.')
   {
     if  (decimalFound)
       return false;
     decimalFound = true;
   }
   else
   {
     if  (decimalFound)
       decimalDigits++;
     value = value * 10 + digit;
   }
   ch++;
   digit = (*ch - '0');
  }

  if  (decimalDigits > 2)
    return false;

  while  (decimalDigits > 0)
  {
    value = value / 10;
    decimalDigits--;
  }

  value = value * sign;
  return  (*ch == 0);
}  //  ValidMoney



bool  KKStr::ValidNum (double&  value)  const
{
  int32  digit = 0;
  int32  sign  = 1;
  value = 0.0;
  value = 0;
  if  (!val)
    return false;

  char*  ch = val;

  // Skip over white space
  while  ((strchr (" \n\t", *ch))  &&  (*ch))
    ch++;
  if  (!(*ch))
    return  false;

  bool  decimalFound  = false;
  int32 decimalDigits = 0;

  if  (*ch == '-')
  {
    ch++;
    sign = -1;
  }

  digit = (*ch - '0');

  while  (((digit >= 0)  &&  (digit <= 9))  ||  (*ch == '.'))
  {
   if  (*ch == '.')
   {
     if  (decimalFound)
       return false;
     decimalFound = true;
   }
   else
   {
     if  (decimalFound)
       decimalDigits++;
     value = value * 10 + digit;
   }
   ch++;
   digit = (*ch - '0');
  }

  while  (decimalDigits > 0)
  {
    value = value / 10;
    decimalDigits--;
  }

  value = value * sign;
  return  (*ch == 0);
}  /* ValidNum */




bool   KKStr::CharInStr (char  ch)
{
  if  (!val)
    return false;

  for  (int32 x = 0;  x < len;  x++)
  {
    if  (val[x] == ch)
      return true;
  }
  return false;
}




/**
 *@brief  Searches for the occurrence of 'searchField' and where in the string.  If found will return 'true' otherwise 'false'.
 */
bool   KKStr::StrInStr (const KKStr&  searchField)  const
{
  return  StrInStr (val, searchField.val);
}



/*
KKStr  KKStr::SubStr (int32  firstChar,
                      int32  subStrLen
                     )
{
  uint32  lastChar;
  KKStr  subStr;
  uint32  x;
  uint32  y;


  if  ((subStrLen < 1)  ||  (!val))
  {
    subStr = "";
    return  subStr;
  }
   

  lastChar = firstChar + subStrLen - 1;

  if  (lastChar >= len)
     lastChar = len - 1;   

  int32  neededSpace = 1 + subStrLen;

  subStr.AllocateStrSpace (neededSpace);

  y = 0;
  for  (x = firstChar; x <= lastChar; x++)
  {
    subStr.val[y] = val[x];
    y++;
  }

  subStr.val[subStrLen] = 0;
  subStr.len = subStrLen;
  return  subStr;
} 
*/



KKStr  KKStr::SubStrPart (int32  firstChar)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  ((KKStrInt)firstChar >= len)
    return "";

  if  (firstChar < 0)
    firstChar = 0;

  KKStrInt  subStrLen = len - (KKStrInt)firstChar;
  KKStr  subStr (subStrLen + 1);
  subStr.Append (((char*)&(val[firstChar])), subStrLen);

  return  subStr;
}  /* SubStrPart */




KKStr  KKStr::SubStrPart (int32  firstChar,
                          int32  lastChar
                         )  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (((KKStrInt)firstChar >= len)  ||  (lastChar < firstChar))
    return  "";

  if  (firstChar < 0)
    firstChar = 0;

  if  (lastChar >= len)
    lastChar = len - 1;


  KKStrInt  subStrLen = ((KKStrInt)lastChar - (KKStrInt)firstChar) + 1;
  KKStr  subStr (subStrLen + 2);

  KKStrInt  x = (KKStrInt)firstChar;
  KKStrInt  y = 0;

  for  (x = (KKStrInt)firstChar; x <= (KKStrInt)lastChar;  x++, y++)
  {
    subStr.val[y] = val[x];
  }
  
  subStr.val[y] = 0;
  subStr.len = subStrLen;
  return  subStr;
}  /* SubStrPart */



/**
 *@brief  Returns a string consisting of the 'tailLen' characters from the end of the string.
 *@details
 *@code
 *ex:
 *   if  test = "Hello World.";
 *       test.Tail (2) will return "d.".
 *@endcode
 */
KKStr  KKStr::Tail (int32 tailLen)  const   // Return back the last 'len' characters.
{
  if  (tailLen <= 0)
    return "";

  KKStrInt  firstChar = Max ((KKStrInt)(len - (KKStrInt)tailLen), (KKStrInt)0);
  return  SubStrPart (firstChar);
}  /* Tail */




/**
 *@brief Remove characters from the end of the string.
 *@details Removes characters from end of string starting at position 'lastCharPos'.  If 'lastCharPos'
 *         is greater than length of string will do nothing.  If 'lastCharPos' is less than or
 *         equal to '0' will delete all characters from string.
 *@param[in] lastCharPos Will remove all characters starting at 'lastCharPos' from end of string.
 */
void  KKStr::LopOff (int32 lastCharPos)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (lastCharPos >= len)
    return;

  if  (lastCharPos < -1)
    lastCharPos = -1;

  KKStrInt  newLen = (KKStrInt)(lastCharPos + 1);
  while  (len > newLen)
  {
    len--;
    val[len] = 0;
  }
}  /* LoppOff */





KKStr  KKStr::QuotedStr ()  const
{
  if  ((!val)  ||  (len < 1))
  {
    return "\"\"";
  }
  
  
  KKStr  result (Len () + 5);

  result.Append ('"');

  int32  idx = 0;

  while  (idx < len)
  {
    switch  (val[idx])
    {
      case  '\"': result.Append ("\\\"");  break;
      case  '\t': result.Append ("\\t");   break;
      case  '\n': result.Append ("\\n");   break;
      case  '\r': result.Append ("\\r");   break;
      case  '\\': result.Append ("\\\\");  break;
      case     0: result.Append ("\\0");   break;
         
      default:     result.Append (val[idx]);   break;
    }

    idx++;
  }

  result.Append ('"');

  return  result;
}  /* QuotedStr */




KKStr  KKStr::ExtractToken (const char* delStr)
{
  if  (!val)
     return  "";
  

  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  token;
  
  char*  tokenStart = val;
  
  // Skip leading Delimiters
  while  ((*tokenStart  != 0)  &&  (strchr (delStr, *tokenStart)))
     tokenStart++;

  if  (*tokenStart == 0)
  {
    delete  [] val;
    val = NULL;
    AllocateStrSpace (1);
    return  token;
  }

  char*  tokenNext = tokenStart;

  while  ((*tokenNext != 0)  &&  (!strchr (delStr, *tokenNext)))
     tokenNext++;
  
  if  (*tokenNext)
  {
    *tokenNext = 0;
    token = tokenStart;

    *tokenNext = ' ';
    tokenNext++;

    len = (KKStrInt)strlen (tokenNext);
    memmove (val, tokenNext, len);
    val[len] = 0;
  }
  
  else
  {
    token = tokenStart;
    memset (val, 0, allocatedSize);
    len = 0;
  }   

  return  token;
} /* ExtractToken */




KKStr  KKStr::ExtractToken2 (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif


  if  (!val)
     return  KKStr ();
  
  KKStr  token;
  
  char*  tokenStart = val;
  
  // Skip Leading Blanks
  while  ((*tokenStart  != 0)  &&  (*tokenStart == ' '))
     tokenStart++;

  if  (*tokenStart == 0)
  {
    delete  [] val;
    val = NULL;
    AllocateStrSpace (1);
    return  token;
  }

  char*  tokenNext = tokenStart;

  while  ((*tokenNext != 0)  &&  (!strchr (delStr, *tokenNext)))
     tokenNext++;
  
  // Remove trailing spaces
  char*  tokenEnd = tokenNext;
  tokenEnd--;
  while  ((tokenEnd != tokenStart)  &&  ((*tokenEnd == ' ')  ||  (*tokenEnd == '\n')  ||  (*tokenEnd == '\r')))
    *tokenEnd = 0;

  if  (*tokenNext)
  {
    *tokenNext = 0;
    token = tokenStart;

    *tokenNext = ' ';
    tokenNext++;

    len = (KKStrInt)strlen (tokenNext);
    memmove (val, tokenNext, len);
    val[len] = 0;
  }
  
  else
  {
    token = tokenStart;
    memset (val, 0, allocatedSize);
    len = 0;
  }   

  return  token;
} /* ExtractToken2 */




KKStr   KKStr::GetNextToken2 (const char* delStr) const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return "";
  
  int32  startCharPos = 0;

  while  (startCharPos < len)
  {
    if  (val[startCharPos] != ' ')
      break;
    startCharPos++;
  }

  if  (startCharPos >= len)
    return  "";

  int32  lastCharPos = startCharPos;

  while  (lastCharPos < len)
  {
    if  (strchr (delStr, val[lastCharPos]) != NULL)
    {
      // We just found the first delimeter
      lastCharPos--;
      return  SubStrPart (startCharPos, lastCharPos);
    }
    lastCharPos++;
  }

  return  SubStrPart (startCharPos);
}  /* GetNextToken2 */




int32  KKStr::ExtractTokenInt (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);
  return  atoi (workStr.Str ());
}



KKU::uint32  KKStr::ExtractTokenUint (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);
  return  (uint32)atol (workStr.Str ());
}


bool  KKStr::ExtractTokenBool (const char* delStr)
{
  KKStr  workStr = ExtractToken2 (delStr);
  workStr.Upper ();

  return  ((workStr == "YES")  ||
           (workStr == "Y")    ||
           (workStr == "TRUE") ||
           (workStr == "T")    ||
           (workStr == "1")
          );
}  /* ExtractTokenBool */



double  KKStr::ExtractTokenDouble (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);

  if  (workStr.Len () == 0)
    return 0.0;

  return  atof (workStr.Str ());
}




char  KKStr::ExtractChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  (len <= 0)
    return 0;

  char  returnChar = val[0];
   
  KKStrInt  newLen = len - 1;

  for  (KKStrInt  x = 0;  x < newLen; x++)
  {
    val[x] = val[x + 1];
  }

  len = newLen;
  val[len] = 0;

  return  returnChar;
}  /* ExtractChar */



KKStr  KKStr::ExtractQuotedStr (const char*  delChars,
                                bool         decodeEscapeCharacters
                               )
{
  if  ((!val)  ||  (len == 0))
    return  EmptyStr ();

  KKStr  result (len);
  int32  idx = 0;
   
  bool  lookForTerminatingQuote = false;

  if  (val[idx] == '"')
  {
    lookForTerminatingQuote = true;
    idx++;
  }


  if  (idx >= len)
  {
    delete  [] val;
    val = NULL;
    AllocateStrSpace (1);
    return  result;
  }

  // Search for matching terminating Quote
  while  (idx < len)
  {
    if  (lookForTerminatingQuote)
    {
      if  (val[idx] == '"')
      {
        idx++;
        if  (idx < len)
        {
          if  (strchr (delChars, val[idx]))
            idx++;
        }

        break;
      }
    }

    else 
    {
      if  (strchr (delChars, val[idx]))
      {
        idx++;
        break;
      }
    }

    if  ((val[idx] == '\\')  &&  (decodeEscapeCharacters))
    {
      idx++;
      if  (idx < len)
      {
        switch  (val[idx])
        {
         case  '"': result.Append ('"');      break;
         case  't': result.Append ('\t');     break;
         case  'n': result.Append ('\n');     break;
         case  'r': result.Append ('\r');     break;
         case '\\': result.Append ('\\');     break;
         case  '0': result.Append (char (0)); break;
         case    0:                           break;
         default:   result.Append (val[idx]); break;
        }
        idx++;
      }
    }
    else
    {
      result.Append (val[idx]);
      idx++;
    }
  }

  if  (idx < len)
  {
    len = (KKStrInt)(len - idx);
    memmove (val, &(val[idx]), len);
    val[len] = 0;
  }
  else
  {
    val[0] = 0;
    len = 0;
  }

  return  result;
}  /* ExtractQuotedStr */



char  KKStr::operator[] (int16 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  ((i < 0)  ||  ((KKStrInt)i >= len))
    return 0;
  else
    return val[i];
}



char  KKStr::operator[] (uint16 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  (i >= len)
    return 0;
  else
    return val[i];
}



char  KKStr::operator[] (int32 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  ((i < 0)  ||  ((KKStrInt)i >= len))
    return 0;
  else
    return val[i];
}



char  KKStr::operator[] (uint32 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  (i >= len)
    return 0;
  else
    return val[i];
}





/**
 *@brief  Static method that returns an Empty String.
 *@return  a empty String.
 */
const KKStr&  KKStr::EmptyStr ()
{
  static  KKStr  emptyStr = "";
  return  emptyStr;
}



VectorKKStr  KKStr::Parse (const char* delStr)  const
{
  KKStr  wrkStr (*this);
  wrkStr.TrimLeft (" ");
  wrkStr.TrimRight (" ");

  VectorKKStr  result;

  while  (!wrkStr.Empty ())
  {
    KKStr  field = wrkStr.ExtractToken2 (delStr);
    result.push_back (field);
  }

  return  result;
}  /* Parse */



VectorKKStr  KKStr::Split (const char* delStr)  const
{
  KKStr  wrkStr (*this);
  wrkStr.TrimLeft (" ");
  wrkStr.TrimRight (" ");

  VectorKKStr  result;

  while  (!wrkStr.Empty ())
  {
    KKStr  field = wrkStr.ExtractToken2 (delStr);
    result.push_back (field);
  }

  return  result;
}  /* Split */




VectorKKStr  KKStr::Split (char del)  const
{
  char  delStr[2];
  delStr[0] = del;
  delStr[1] = 0;

  KKStr  wrkStr (*this);
  wrkStr.TrimLeft (" ");
  wrkStr.TrimRight (" ");

  VectorKKStr  result;

  while  (!wrkStr.Empty ())
  {
    KKStr  field = wrkStr.ExtractToken2 (delStr);
    result.push_back (field);
  }

  return  result;
}  /* Parse */



bool  KKStr::ToBool () const
{
  if  (len < 1)
    return false;

  if  ((STRICMP (val, "y")    == 0)  ||
       (STRICMP (val, "yes")  == 0)  ||
       (STRICMP (val, "t")    == 0)  ||
       (STRICMP (val, "true") == 0)  ||
       (STRICMP (val, "1")    == 0)
      )
    return true;
  else
    return false;
}



double  KKStr::ToDouble () const
{
  if  (!val)
    return 0.0;

  double  d = atof (val);
  return d;
}  /* ToDouble */




float  KKStr::ToFloat  () const
{
  if  (!val)
    return 0.0f;

  float  f = (float)atof (val);
  return f;
}   /* ToFloat */




int32  KKStr::ToInt () const
{
  if  (!val)
    return 0;

  int32  i = atoi (val);
  return i;
}  /* ToInt*/



int16  KKStr::ToInt16 () const
{
  if  (!val)
    return 0;

  int16  i = (int16)atoi (val);
  return i;
}  /* ToInt32*/



int32  KKStr::ToInt32 () const
{
  if  (!val)
    return 0;

  int32  i = atoi (val);
  return i;
}  /* ToInt32*/



KKU::int64  KKStr::ToInt64 () const
{
  if  (!val)  return 0;

  #if  defined(__GNUC__)
    return atoll (val);
  #else
    return  (int64)_atoi64 (val);
  #endif
}



long   KKStr::ToLong   () const
{
  if  (!val)
    return 0;

  long  l = atol (val);
  return l;
}  /* ToLong */




float  KKStr::ToPercentage () const
{
  if  (LastChar () == '%')
  {
    KKStr  workStr = this->SubStrPart (0, Len () - 2);
    return  workStr.ToFloat ();
  }

  return  100.0f * ToFloat ();
}



KKU::uint32 KKStr::ToUint () const
{
  if  (!val)  return 0;
  return  (uint32)atol (val);
}



KKU::ulong  KKStr::ToUlong () const
{
  if  (!val)  return 0;
  return  (ulong)atol (val);
}



KKU::uint32  KKStr::ToUint32 () const
{
  if  (!val)  return 0;
  return  (uint32)atol (val);
}



KKU::uint64  KKStr::ToUint64 () const
{
  if  (!val)  return 0;
  #if  defined(__GNUC__)
    return  (uint64)atoll (val);
  #else
    return  (uint64)_atoi64 (val);
  #endif
}



wchar_t*  KKStr::ToWchar_t () const
{
  wchar_t* wa = NULL;
  if  (val == NULL)
  {
    wa = new wchar_t[1];
    mbstowcs (wa, "", 1);
    return  wa;
  }
  else
  {
    size_t  wideLen = len + 1;
    wa = new wchar_t[wideLen];
    mbstowcs (wa, val, wideLen);
  }
  return wa;
}





double  KKStr::ToLatitude ()  const
{
  KKStr latitudeStr (*this);
  bool  north = true;
  char  lastChar = (char)toupper (latitudeStr.LastChar ());
  if  (lastChar == 'N')
  {
    north = true;
    latitudeStr.ChopLastChar ();
  }
  else if  (lastChar == 'S')
  {
    north = false;
    latitudeStr.ChopLastChar ();
  }

  if  (latitudeStr.FirstChar () == '-')
  {
    latitudeStr.ChopFirstChar ();
    north = !north;
  }

  double  degrees = 0.0;
  double  minutes = 0.0;
  double  seconds = 0.0;

  KKStr  degreesStr = latitudeStr.ExtractToken2 (":");
  KKStr  minutesStr = latitudeStr.ExtractToken2 (":");
  
  degrees = degreesStr.ToDouble ();
  minutes = minutesStr.ToDouble ();
  seconds = latitudeStr.ToDouble ();

  double  latitude = degrees + (minutes / 60.0) + (seconds / 3600.0);
  if  (latitude < 0.0)
  {
    north = !north;
    latitude = 0.0 - latitude;
  }

  while  (latitude > 90.0)
    latitude = latitude - 180.0;

  if  (!north)
    latitude = 0.0 - latitude;

  return  latitude;
}  /* ToLatitude */




double  KKStr::ToLongitude ()  const
{
  KKStr longitudeStr (*this);
  bool  east = true;
  char  lastChar = (char)toupper (longitudeStr.LastChar ());
  if  (lastChar == 'E')
  {
    east = true;
    longitudeStr.ChopLastChar ();
  }
  else if  (lastChar == 'W')
  {
    east = false;
    longitudeStr.ChopLastChar ();
  }

  if  (longitudeStr.FirstChar () == '-')
  {
    longitudeStr.ChopFirstChar ();
    east = !east;
  }

  double  degrees = 0.0;
  double  minutes = 0.0;
  double  seconds = 0.0;

  KKStr  degreesStr = longitudeStr.ExtractToken2 (":");
  KKStr  minutesStr = longitudeStr.ExtractToken2 (":");
  
  degrees = degreesStr.ToDouble ();
  minutes = minutesStr.ToDouble ();
  seconds = longitudeStr.ToDouble ();

  double  longitude = degrees + (minutes / 60.0) + (seconds / 3600.0);
  if  (longitude < 0.0)
  {
    east = !east;
    longitude = 0.0 - longitude;
  }

  while  (longitude > 180.0)
    longitude = longitude - 360.0;

  if  (!east)
    longitude = 0.0 - longitude;

  return  longitude;
}  /* ToLongitude */



int32  SearchStr (const char*  src,
                  int32        srcLen,
                  int32        startPos,
                  const char*  srchStr,
                  int32        srchStrLen
                 )
{
  if  ((!src)  ||  (!srchStr))
    return -1;

  int32 numIter = (srcLen - (startPos + srchStrLen - 1));
  const char* startCh = src + startPos;

  int32  x;
  for  (x = 0;  x < numIter;  ++x, ++startCh)
  {
    if  (strncmp (startCh, srchStr, srchStrLen) == 0)
      return  startPos + x;
  }
  return -1;
}



int32  KKStr::Find (const KKStr&  str, int32 pos) const
{
  return  SearchStr (val, len, pos, str.Str (), str.Len ());
}



int32  KKStr::Find (const char*   s,   int32 pos, int32 n)  const
{
  return  SearchStr (val, len, pos, s, n);
}



int32  KKStr::Find (const char* s,  int32 pos) const
{
  return  SearchStr (val, len, pos, s, (int32)strlen (s));
}



int32  KKStr::Find (char c, int32 pos) const
{
  for  (int32 x = pos;  x < len;  x++)
  {
    if  (val[x] == c)
      return  x;
  }
  return -1;
}



KKStr  KKU::operator+ (const char    left,
                       const KKStr&  right
                      )
{
  KKStr  result (right.Len () + 3);
  result.Append (left);
  result.Append (right);
  return  result;
}



KKStr  KKU::operator+ (const char*   left,
                       const KKStr&  right
                      )
{
  return  KKStr (left) + right;
}




KKStr  KKStr::operator+ (const char*  right)  const
{
  int32  resultStrLen = len + (int32)strlen (right);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (right);
  return  result;
}




KKStr  KKStr::operator+ (const KKStr&  right)  const
{
  int32  resultStrLen = len + right.len;

  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (right);
  return  result;
}



KKStr  KKStr::operator+ (int16  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%-ld", right);
  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}


KKStr  KKStr::operator+ (uint16  right)  const
{
  char  buff[30];
  SPRINTF (buff, sizeof (buff), "%u", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (int32  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%-ld", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (uint32  right)  const
{
  char  buff[30];
  SPRINTF (buff, sizeof (buff), "%u", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (int64 right)  const
{
  char  buff[70];
  SPRINTF (buff, sizeof (buff), "%-lld", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (uint64 right)  const
{
  char  buff[70];
  SPRINTF (buff, sizeof (buff), "%-llu", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (float  right)  const
{
  char  buff[60];

  // KK 2013-09-15  Trial and Error experimentation shows that "%.8g" produced chracters that exactly matched what the Watch fuction in the debigger was showing in 'right'.
  SPRINTF (buff, sizeof (buff), "%.8g", right);

  int32  resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (double  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%.17g", right);

  int32 resultStrLen = len + (int32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr&  KKStr::operator<< (char  right)
{
  Append (right);
  return  *this;
}



KKStr&  KKStr::operator<< (const char*  right)
{
  if  (!right)
  {
    const char*  msg = "KKStr&  operator<<(const char*  right)    **** ERROR ****  right==NULL";
    cerr << std::endl << msg <<  std::endl << std::endl;
    throw KKException (msg);
  }

  Append (right);
  return  *this;
}



KKStr&  KKStr::operator<< (const KKStr&  right)
{
  Append (right.Str ());
  return  *this;
}



KKStr&  KKStr::operator<< (int16  right)
{
  AppendInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (uint16  right)
{
  AppendUInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (int32 right)
{
  AppendInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (uint32  right)
{
  AppendUInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (int64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



KKStr&  KKStr::operator<< (uint64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



KKStr&  KKStr::operator<< (float  right)
{  
  char  buff[60];
  // KK 2013-09-15  Trial and Error experimentation shows that "%.8g" produced chracters that exactly matched what the Watch fuction in the debigger was showing in 'right'.
  SPRINTF (buff,  sizeof (buff), "%.8g", right);

  if  (strchr (buff, '.') != NULL)
  {
    // Remove trailing Zeros
    int32  buffLen = (int32)strlen (buff);
    while  ((buffLen > 1)  &&  (buff[buffLen - 1] == '0')  &&  (buff[buffLen - 2] == '0'))
    {
      buffLen--;
      buff[buffLen] = 0;
    }
  }
  Append (buff);
  return *this;
}



KKStr&  KKStr::operator<< (double  right)
{  
  char  buff[70];
  SPRINTF (buff, sizeof (buff), "%.17g", right);
  if  (strchr (buff, '.') != NULL)
  {
    // Remove trailing Zeros
    int32  buffLen = (int32)strlen (buff);
    while  ((buffLen > 1)  &&  (buff[buffLen - 1] == '0')  &&  (buff[buffLen - 2] == '0'))
    {
      buffLen--;
      buff[buffLen] = 0;
    }
  }
  Append (buff);
  return *this;
}





void  Test2 (ostream& x1, const char* x2)
{
  x1 << x2;
}




#ifdef  WIN32
ostream& __cdecl  KKU::operator<< (      ostream&  os, 
                                   const KKStr&   strng
                                  )
{
  os << (strng.Str ());
  return os;
}



std::istream& __cdecl  KKU::operator>> (std::istream&  is,
                                        KKStr&         str
                                       )
{
  char  buff[10240];
  is >> buff;
  str = buff;
  str.TrimLeft ();
  str.TrimRight ();
  return  is;
}


#else

ostream& KKU::operator<< (      ostream&  os, 
                          const KKStr&   strng
                         )
{
  Test2 (os, strng.Str ());
  // os << (strng.Str ());
  return os;
}



std::istream&  KKU::operator>> (std::istream&  is,
                                KKStr&         str
                               )
{
  char  buff[10240];
  is >> buff;
  str = buff;
  str.TrimLeft ();
  str.TrimRight ();
  return  is;
}
#endif




KKStr  KKStr::Spaces (int32  c)
{
  KKStr s;
  s.RightPad (c);
  return  s;
}



void  KKStr::StrCapitalize (char*  str)
{
  if  (!str)
     return;
  
  char* ch = str;
  while  (*ch)
  {
    *ch = (char)toupper (*ch);
    ch++;
  }
}




bool  KKStr::StrInStr (const char*  target,
                       const char*  searchStr
                      )
{
  if  ((target == NULL)  ||  (searchStr == NULL))
    return false;

# ifdef  USE_SECURE_FUNCS
    char*  t = _strdup (target);
    char*  s = _strdup (searchStr);
# else
    char*  t = strdup (target);
    char*  s = strdup (searchStr);
#endif


  StrCapitalize (t);
  StrCapitalize (s);
  
  bool  f = (strstr (t, s) != NULL);

  free(t);
  free(s);
  return f;
}






KKStrList::KKStrList (bool owner,
                        int32  initSize):
  KKQueue<KKStr> (owner, initSize),
  sorted (false)
{
}



KKStrList::KKStrList (const char*  s[]):
    KKQueue<KKStr> (true, 10),
    sorted (false)
{
  if  (s == NULL)
    return;

  int  x = 0;
  while  (s[x] != NULL)
  {
    PushOnBack (new KKStr (s[x]));
    ++x;
  }
}




int32  KKStrList::MemoryConsumedEstimated ()  const
{
  int32  memoryConsumedEstimated = sizeof (KKStrList);
  KKStrList::const_iterator idx;
  for  (idx = begin ();  idx != end ();  ++idx)
    memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



bool  KKStrList::StringInList (KKStr& str)
{
  bool  found = false;
  int32 idx;
  int32 qSize = QueueSize ();

  for  (idx = 0; ((idx < qSize) && (!found)); idx++)
    found = (str == (*IdxToPtr (idx)));

  return  found;
}



KKStrPtr  KKStrList::BinarySearch (const KKStr&  searchStr)
{
  if  (!sorted)
  {
    cerr << std::endl
         << "KKStrList::BinarySearch     **** ERROR ****"  << std::endl
         << std::endl
         << "            KKStr List is Not Sorted" << std::endl
         << std::endl;
    exit (-1);
  }
  
  int32  low  = 0;
  int32  high = QueueSize () - 1;
  int32  mid;

  KKStrPtr  str = NULL;

  while  (low <= high)
  {
    mid = (low + high) / 2;

    str = IdxToPtr (mid);

    if  (*str  < searchStr)
    {
      low = mid + 1;
    }

    else if  (*str > searchStr)
    {
      high = mid - 1;
    }

    else
    {
      return  str;
    }
  }

  return  NULL;
}  /* BinarySearch */




  
void   KKStrList::AddString (KKStrPtr  str)
{
  PushOnBack (str);
  sorted = false;
}



KKStrListPtr  KKStrList::ParseDelimitedString (const KKStr&  str,
                                               const char*    delChars
                                              )
{
  KKStrListPtr  parms = new KKStrList (true, 10);

# ifdef  USE_SECURE_FUNCS
    char*  workStr =  _strdup (str.Str ());
# else
    char*  workStr =  strdup (str.Str ());
# endif

  char*  nextChar = workStr;

  while  (*nextChar)
  {
    // Skip Past Leading Blamks
    while  ((*nextChar)  &&  (*nextChar == ' '))
    {
      nextChar++;
    }

    if  (*nextChar == 0)
      break;

    const char*  startOfToken = nextChar;

    while  ((*nextChar)  &&  (strchr (delChars, *nextChar) == NULL))
    {
      nextChar++;
    }

    if  (*nextChar != 0)
    {
      *nextChar = 0;
      nextChar++;
    }

    KKStrPtr  token = new KKStr (startOfToken);
    token->TrimRight ();

    parms->PushOnBack (token);
  }

  delete  [] workStr;

  return  parms;
}  /* ParseDelimitedString */




/**
 @brief Compares to Strings and returns -1, 0, or 1,  indicating if less than, equal, or greater.
 */
int32  KKStr::CompareStrings (const KKStr&  s1, 
                            const KKStr&  s2
                           )
{
  if  (s1.val == NULL)
  {
    if  (s2.val == NULL)
      return 0;
    else
      return -1;
  }

  else if  (s2.val == NULL)
  {
    return  1;
  }

  else
  {
    return  strcmp (s1.val, s2.val);
  }
}  /* CompareStrings */




class  KKStrList::StringComparison
{
public:
   StringComparison (bool  _reversedOrder)
   {
     reversedOrder = _reversedOrder;
   }


   bool  operator() (KKStrPtr  p1,
                     KKStrPtr  p2
                    )
   {
     if  (reversedOrder)
     {
       return  (KKStr::CompareStrings (*p2, *p1) > 0);
     }
     else
     {
       return  (KKStr::CompareStrings (*p1, *p2) < 0);
     }
   }

private:
  bool  reversedOrder;
};  /* StringComparison */




void  KKStrList::Sort (bool  _reversedOrder)
{
  StringComparison  stringComparison (_reversedOrder);
  sort (begin (), end (), stringComparison);
  if  (!_reversedOrder)
    sorted = true;
}


KKStrListPtr  KKStrList::DuplicateListAndContents ()  const
{
  KKStrListPtr  newList = new KKStrList (true, QueueSize ());
  KKStrList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
    newList->PushOnBack (new KKStr (*(*idx)));

  return  newList;
}  /* DuplicateListAndContents */




int32  LocateLastOccurrence (const char*   str,
                             char          ch)
{
  if  (!str)
    return -1;

  bool    found  = false;
  size_t  len    = strlen (str);
  size_t  idx    = len - 1;

  while  ((idx >= 0)  &&  (!found))
  {
    if  (str[idx] == ch)
      found = true;
    else
      idx--;
  }

  if  (found)
    return  (int32)idx;
  else
    return  -1;

}  /* LocateLastOccurrence */



KKStr  KKU::StrFormatDouble (double       val,
                             const char*  mask
                            )
{
  // Get num of decimal Places

  char  buff[512];
  char* bp = buff + 511;
  *bp = 0;

  bool  negativePrinted = true;

  if  (val < 0)
  {
    negativePrinted = false;
    val = fabs (val);
  }

  bool    printDecimalPoint = false;

  int32   numOfDecimalPlaces = 0;

  int32   maskLen = (int32)strlen (mask);
  
  int32   decimalPosition = LocateLastOccurrence (mask, '.');

  const char*  maskPtr = mask + maskLen - 1; 

  long  intPart = (long)floor (val);

  int32  nextDigit = 0;

  int32  x;

  if  (decimalPosition >= 0)
  {
    numOfDecimalPlaces = maskLen - decimalPosition - 1;
    printDecimalPoint = true;
    maskPtr = mask + decimalPosition - 1;
    maskLen = decimalPosition;
  }

  if  (printDecimalPoint)
  {
    double  power = pow ((double)10, (double)numOfDecimalPlaces);

    double  frac = val - floor (val);

    int32  fracInt = (int32)(frac * power + 0.5);

    for  (x = 0; x < numOfDecimalPlaces; x++)
    {
      nextDigit = fracInt % 10;
      fracInt   = fracInt / 10;
      bp--;
      *bp = (char)('0' + nextDigit);
    }

    if  (fracInt != 0)
    {
      // This can occur,  
      //  ex:  mask = "#0.000",  val = 1.9997
      //  fracInt will end up equailng 1.000. because of rounding.  
      intPart = intPart + fracInt;
    }

    bp--;
    *bp = '.';
  }

  char  formatChar = ' ';
  char  lastFormatChar = ' ';

  while  (maskLen > 0)
  {
    formatChar = (char)toupper (*maskPtr);

    switch (formatChar)
    {
      case  '0': 
      case  '@': 
           nextDigit = intPart % 10;
           intPart = intPart / 10;
           bp--;
           *bp = '0' + (uchar)nextDigit;
           break;


      case  '#':
      case  '9':
           if (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           else
           {
             bp--;
             *bp = ' ';
           }
           break;


      case  'Z':
           if (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           break;


      case  '-':
           if  (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           else
           {
             if  (!negativePrinted)
             {
               negativePrinted = true;
               bp--;
               *bp = '-';
             }
           }
           break;


      case  ',':
           if  (intPart > 0)
           {
             bp--;
             *bp = ',';
           }

           else if  (lastFormatChar != 'Z')
           {
             bp--;
             *bp = ' ';
           }
           break;
       

      default:
           bp--;
           *bp = formatChar;
           break;
    }  /* end of Switch (*maskPtr) */


    lastFormatChar = formatChar;

    maskPtr--;
    maskLen--;
  }
  
  // If the mask was not large enough to include all digits then lets do it now.
  while  (intPart > 0)
  {
    nextDigit = intPart % 10;
    intPart = intPart / 10;
    bp--;
    *bp = '0' + (uchar)nextDigit;
  }

  if  (!negativePrinted)
  {
    bp--;
    *bp = '-';
  }

  return  KKStr (bp);
}  /* StrFormatDouble */




KKStr  KKU::StrFormatInt (int32        val,
                          const char*  mask
                         )
{
  return  KKU::StrFormatDouble ((double)val, mask);
}



KKStr  KKU::StrFormatInt64 (int64        val,
                            const char*  mask
                           )
{
  // Get num of decimal Places

  char  buff[128];
  char* bp = buff + 127;
  *bp = 0;

  bool  negativePrinted = true;

  if  (val < 0)
  {
    negativePrinted = false;
    val = 0 - val;
  }

  int32   maskLen = (int32)strlen (mask);
  const char*  maskPtr = mask + maskLen - 1; 

  int64  intPart = val;

  int32  nextDigit = 0;

  char  formatChar = ' ';
  char  lastFormatChar = ' ';

  while  (maskLen > 0)
  {
    formatChar = (uchar)toupper (*maskPtr);

    switch (formatChar)
    {
      case  '0': 
      case  '@': 
           nextDigit = intPart % 10;
           intPart = intPart / 10;
           bp--;
           *bp = '0' + (uchar)nextDigit;
           break;


      case  '#':
      case  '9':
           if (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           else
           {
             bp--;
             *bp = ' ';
           }
           break;


      case  'Z':
           if (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           break;


      case  '-':
           if  (intPart > 0)
           {
             nextDigit = intPart % 10;
             intPart = intPart / 10;
             bp--;
             *bp = '0' + (uchar)nextDigit;
           }
           else
           {
             if  (!negativePrinted)
             {
               negativePrinted = true;
               bp--;
               *bp = '-';
             }
           }
           break;


      case  ',':
           if  (intPart > 0)
           {
             bp--;
             *bp = ',';
           }

           else if  (lastFormatChar != 'Z')
           {
             bp--;
             *bp = ' ';
           }
           break;
       

      default:
           bp--;
           *bp = formatChar;
           break;
    }  /* end of Switch (*maskPtr) */


    lastFormatChar = formatChar;

    maskPtr--;
    maskLen--;
  }
  
  // If the mask was not large enough to include all digits then lets do it now.
  while  (intPart > 0)
  {
    nextDigit = intPart % 10;
    intPart = intPart / 10;
    bp--;
    *bp = '0' + (uchar)nextDigit;
  }

  if  (!negativePrinted)
  {
    bp--;
    *bp = '-';
  }

  return  KKStr (bp);
}  /* StrFormatInt */



KKStr  KKU::StrFromInt16 (int16 i)
{
  char  buff[50];

  SPRINTF (buff, sizeof (buff), "%d", i);
  KKStr s (buff);
  return  s;
}  /* StrFromInt16 */




KKStr  KKU::StrFromUint16 (uint16 ui)
{
  char  buff[50];

  SPRINTF (buff, sizeof (buff), "%u", ui);
  KKStr s (buff);
  return  s;
}  /* StrFromUint16 */



KKStr  KKU::StrFromInt32 (int32 i)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%ld", i);
  KKStr s (buff);
  return  s;
}  /* StrFromInt32 */




KKStr  KKU::StrFromUint32 (uint32 ui)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%lu", ui);
  KKStr s (buff);
  return  s;
}  /* StrFromUint32 */



KKStr  KKU::StrFromInt64 (int64 i64)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%lld", i64);
  KKStr s (buff);
  return  s;
}  /* StrFromInt64 */



KKStr  KKU::StrFromUint64 (uint64 ul)
{
  char  buff[50];
  SPRINTF (buff, sizeof (buff), "%llu", ul);
  KKStr s (buff);
  return  s;
}  /* StrFromUint64 */



KKStr& KKStr::operator<< (ostream& (* mf)(ostream &))
{
  ostringstream  o;
  mf (o);
  Append (o.str ().c_str ());
  return  *this;
}



const  uint32  KKU::KKStr::StrIntMax = USHRT_MAX;



KKStrListIndexed::KKStrPtrComp::KKStrPtrComp (bool  _caseSensitive):
  caseSensitive (_caseSensitive)
{}

  
KKStrListIndexed::KKStrPtrComp::KKStrPtrComp (const KKStrPtrComp&  comparator):
  caseSensitive (comparator.caseSensitive)
{}



bool  KKStrListIndexed::KKStrPtrComp::operator() (const KKStrConstPtr& lhs, const KKStrConstPtr& rhs) const
{
  if  (caseSensitive)
    return ((*lhs) < (*rhs));
  else
    return  (lhs->Compare (*rhs) < 0);
}




KKStrListIndexed::KKStrListIndexed (bool _owner,
                                    bool _caseSensitive
                                   ):
  comparator              (_caseSensitive),
  indexIndex              (),
  memoryConsumedEstimated (0),
  nextIndex               (0),
  owner                   (_owner),
  strIndex                (NULL)
{
  strIndex = new StrIndex (comparator);
  memoryConsumedEstimated = sizeof (*this);
}



KKStrListIndexed::KKStrListIndexed (const KKStrListIndexed&  list):
   comparator              (list.comparator),
   indexIndex              (),
   memoryConsumedEstimated (0),
   nextIndex               (0),
   owner                   (list.owner),
   strIndex                (NULL)
{
  strIndex = new StrIndex (comparator);
  memoryConsumedEstimated = sizeof (*this);
  IndexIndex::const_iterator  idx;
  for  (idx = list.indexIndex.begin ();  idx != list.indexIndex.end ();  ++idx)
  {
    if  (owner)
      Add (new KKStr (*(idx->second)));
    else
      Add (idx->second);
  }
}




KKStrListIndexed::~KKStrListIndexed ()
{
  if  (owner)
  {
    StrIndex::iterator  idx;
    for  (idx = strIndex->begin ();  idx != strIndex->end ();  ++idx)
    {
      KKStrPtr s = idx->first;
      delete s;
    }
  }

  delete  strIndex;
  strIndex = NULL;
}


uint32  KKStrListIndexed::size ()  const
{
  return  indexIndex.size ();
}



int32  KKStrListIndexed::MemoryConsumedEstimated ()  const
{
  return  memoryConsumedEstimated;
}  /* MemoryConsumedEstimated */




bool  KKStrListIndexed::operator== (const KKStrListIndexed&  right)
{
  if  (indexIndex.size () != right.indexIndex.size ())
    return false;

  bool  caseSensativeComparison = caseSensative  ||  right.caseSensative;

  IndexIndex::const_iterator  idxLeft  = indexIndex.begin ();
  IndexIndex::const_iterator  idxRight = right.indexIndex.begin ();

  while  ((idxLeft != indexIndex.end ())  &&  (idxRight != right.indexIndex.end ()))
  {
    if  (idxLeft->first != idxRight->first)
      return false;

    if  (caseSensativeComparison)
    {
      if  (idxLeft->second->Compare (*(idxRight->second)) != 0)
        return false;
    }
    else
    {
      if  (idxLeft->second->CompareIgnoreCase (*(idxRight->second)) != 0)
        return false;
    }

    if  ((*(idxLeft->second)) != (*(idxRight->second)))

    ++idxLeft;
    ++idxRight;
  }

  return  true;
}  /* operator== */



bool  KKStrListIndexed::operator!= (const KKStrListIndexed&  right)
{
  return  !((*this) == right);
}  /* operator!= */



int32  KKStrListIndexed::Add (KKStrPtr  s)
{
  StrIndex::iterator  idx;
  idx = strIndex->find (s);
  if  (idx != strIndex->end ())
    return -1;

  int32  index = nextIndex;
  ++nextIndex;

  strIndex->insert (StrIndexPair (s, index));
  indexIndex.insert (IndexIndexPair (index, s));

  if  (owner)
    memoryConsumedEstimated += s->MemoryConsumedEstimated ();
  memoryConsumedEstimated += 8;
  return  index;
}  /* Add */



int32   KKStrListIndexed::Delete (KKStr&  s)
{
  StrIndex::iterator  idx;
  idx = strIndex->find (&s);
  if  (idx == strIndex->end ())
    return -1;
 
  KKStrPtr  strIndexStr = idx->first;

  int32  index = idx->second;
  strIndex->erase (idx);

  IndexIndex::iterator  idx2;
  idx2 = indexIndex.find (index);
  if  (idx2 != indexIndex.end ())
    indexIndex.erase (idx2);

  if  (owner)
  {
    memoryConsumedEstimated -= strIndexStr->MemoryConsumedEstimated ();
    delete  strIndexStr;
    strIndexStr = NULL;
  }
  memoryConsumedEstimated -= 8;
  return index;
}  /* Delete */



int32  KKStrListIndexed::LookUp (const KKStr& s)  const
{
  StrIndex::const_iterator  idx;

  KKStr  sNotConst (s);

  idx = strIndex->find (&sNotConst);
  if  (idx == strIndex->end ())
    return -1;
  else
    return idx->second;
}


int32  KKStrListIndexed::LookUp (KKStrPtr s)  const
{
  StrIndex::iterator  idx;
  idx = strIndex->find (s);
  if  (idx == strIndex->end ())
    return -1;
  else
    return idx->second;
}



const  KKStrConstPtr  KKStrListIndexed::LookUp (int32 x)
{
  IndexIndex::iterator  idx;
  idx = indexIndex.find (x);
  if  (idx == indexIndex.end ())
    return NULL;
  else
    return idx->second;
}




















