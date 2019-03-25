/* Str.cpp -- String Management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <cstring>
#include <ctype.h>
#include <exception>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;

#include "KKQueue.h"

#include "KKStr.h"

#include "RNBase64.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "Option.h"
#include "RunLog.h"
#include "XmlStream.h"
using namespace KKB;


char*  KKB::STRCOPY (char*        dest,
                     kkStrUint    destSize,
                     const char*  src
                    )
{
# ifdef  USE_SECURE_FUNCS
    strcpy_s  (dest, destSize, src);
# else
    if  ((strlen(src) + 1) > destSize)
    {
      stringstream errMsg;
      errMsg << "KKB::STRCOPY   ***ERROR***   length of src[" << strlen(src) << "] >  destSize[" << destSize << "]"; 
      std::cerr << errMsg.str () << std::endl;
      throw KKB::KKException (errMsg.str ());
    }

    strcpy (dest, src);
# endif
  return  dest;
}  /* STRCOPY */



char*  KKB::STRCOPY (char*        dest,
                     kkint32      destSize,
                     const char*  src
                    )
{
# ifdef  USE_SECURE_FUNCS
    strcpy_s  (dest, destSize, src);
# else
    if  ((kkint32)(strlen(src) + 1) > destSize)
    {
      stringstream errMsg;
      errMsg << "KKB::STRCOPY   ***ERROR***   length of src[" << strlen(src) << "] >  destSize[" << destSize<< "]";
      cerr << errMsg.str () << endl;
      throw KKB::KKException (errMsg.str ());
    }

    strcpy (dest, src);
# endif
  return  dest;
}  /* STRCOPY */



char*  KKB::STRDUP (const char* src)  
{
# ifdef  USE_SECURE_FUNCS
    return _strdup (src);
# else
    return  strdup (src);
# endif
}  /* STRDUP */



char*  KKB::STRCAT (char*        dest, 
                    kkint32      destSize,
                    const char*  src
                   )
{
# ifdef  USE_SECURE_FUNCS
    strcat_s  (dest, destSize, src);
# else
    kkint32 zed = destSize - ((kkint32)strlen (dest) + 1);
    strncat (dest, src, zed);
# endif

  return  dest;
}  /* STRCAT */



kkint32  KKB::STRICMP (const char*  left,
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

  kkint32  zed = (toupper (*left)) - (toupper (*right));

  while  ((zed == 0)  &&  (*left != 0))
  {
    left++;    right++;
    zed = (toupper (*left)) - (toupper (*right));
  }

  if  (zed < 0)
    return -1;
  else if  (zed == 0)  
    return 0;
  else
    return 1;

}  /* STRICMP */



kkint32  KKB::STRNICMP (const char*  left,
                        const char*  right,
                        kkint32      len
                       )
{
  if  (left == NULL)
    return  (right == NULL) ? 0 : -1;

  else if  (!right)
    return 1;

  if  (len < 1)
    return 0;

  kkint32  x = 0;
  kkint32  zed = (toupper (*left)) - (toupper (*right));
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



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkint16      right
                      )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
# endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkuint16     right
                      )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
#endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkint32      right
                      )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
# endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkuint32     right
                      )
{
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
#endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkint64      right
                      )
{
  
# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
# endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkuint64     right
                      )
{

# ifdef  USE_SECURE_FUNCS
    return sprintf_s (buff, buffSize, formatSpec, right);
# else
    return snprintf(buff, buffSize, formatSpec, right);
# endif
}



kkint32  KKB::SPRINTF (char*        buff,
                       kkint32      buffSize,
                       const char*  formatSpec,
                       kkint32      precision,
                       double       d
                      )
{
#ifdef  USE_SECURE_FUNCS
  return  sprintf_s (buff, buffSize, formatSpec, precision, d);
#else
  return snprintf(buff, buffSize, formatSpec, precision, d);
#endif
}



kkint32  KKB::SPRINTF (char*         buff,
                       kkint32       buffSize,
                       char  const*  formatSpec,
                       double        d
                      )
{
  #ifdef  USE_SECURE_FUNCS
  return sprintf_s (buff, buffSize, formatSpec, d);
  #else
  return snprintf(buff, buffSize, formatSpec, d);
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



const char*  KKStr::StrChr (const char*  str,
                            int          ch
                           )
{
  return strchr (str, ch);
}



kkint32  KKStr::StrCompareIgnoreCase (const char* s1, 
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



bool  KKStr::StrEqualN (const char* s1,
                        const char* s2,
                        kkuint32    len
                       )
{
  if  ((!s1)  &&  (!s2))
     return  true;

  if  ((!s1)  ||  (!s2))
     return  false;

  for (kkuint32  x = 0;  x < len;  ++x)
    if  (s1[x] != s2[x])
      return false;

  return  true;
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



bool  KKStr::StrEqualNoCaseN (const char* s1,
                              const char* s2,
                              kkuint32    len
                             )
{
  if  ((!s1)  &&  (!s2))
     return  true;

  if  ((!s1)  ||  (!s2))
     return  false;

  for (kkuint32  x = 0;  x < len;  ++x)
    if  (toupper (s1[x]) != toupper (s2[x]))
      return false;

  return  true;
}



void  KKStr::StrReplace (char**      dest,  
                         const char* src
                        )
{
  if  (*dest)
     delete [] *dest;

  kkint32  spaceNeeded;
  
  if  (src)
  {
    spaceNeeded = (kkint32)strlen (src) + 1;
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
  
  kkStrUint  newLen = (kkStrUint)strlen (str);
  AllocateStrSpace (newLen + 1);

  STRCOPY (val, allocatedSize, str);

  len = (kkStrUint)newLen;
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
    std::cerr << std::endl 
         << "KKStr::KKStr    ***ERROR***    Missing terminating NULL" << std::endl
         << std::endl;
  }

  kkStrUint  neededSpace = str.len + 1;
  if  (neededSpace > str.allocatedSize)
  {
    KKStr  errMsg = "KKStr::KKStr (const KKStr&  str)   AllocatedSize["  + ::StrFromUint32(str.allocatedSize) + "]  on Source KKStr is to Short";
    std::cerr << endl << errMsg << "  ***ERROR***" << std::endl << std::endl;
    throw new KKException (errMsg);
  }

  AllocateStrSpace (str.allocatedSize);
  if  (!val)  {
    std::cerr << std::endl << "KKStr::KKStr  ***ERROR***   Allocation Failed." << std::endl << std::endl; 
    throw KKException("KKStr::KKStr  ***ERROR***  Allocation Failed.");
  }

  memcpy (val, str.val, str.len);
  len = str.len;
}



 KKStr::KKStr (KKStr&&  str):
    allocatedSize (str.allocatedSize),
    len           (str.len),
    val           (str.val)
{
  str.allocatedSize = 0;
  str.len           = 0;
  str.val           = NULL;
}



/**
 *@brief Creates a String that is populated with 'd' as displayable characters and precision of 'precision'.
 */
KKStr::KKStr (double  d, 
              kkint32 precision
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

  kkStrUint  newLen = (kkStrUint)strlen (buff);
  AllocateStrSpace (newLen + 1);

  STRCOPY (val, allocatedSize, buff);

  len = newLen;
}



/**
 *@brief Creates a KKStr object that has 'size' characters preallocated; and set to empty string.
 */
KKStr::KKStr (kkStrUint  size):
        val (NULL)
{
  KKCheck (size <= MaxStrLen, "KKStr::KKStr  size: " << size << " exceeds max allowed length: " << MaxStrLen)
  AllocateStrSpace ((kkStrUint)size);
  val[0] = 0;
  len = 0;
}



KKStr::KKStr (const std::string&  s):
        allocatedSize (0),
        len (0),
        val (NULL)
{
  AllocateStrSpace ((kkint32)(s.size () + 1));

  len = (kkStrUint)s.size ();
  for  (kkStrUint x = 0;  x < len;  ++x)
    val[x] = s[x];
  val[len] = 0;
}



/** @brief  Constructs a KKStr instance from a sub-string of 'src'.  */
KKStr::KKStr (const char*  src,
              kkuint32     startPos,
              kkuint32     endPos
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

  kkuint32  subStrLen = 1 + endPos - startPos;
  if  (subStrLen > (MaxStrLen - 1))
  {
    cerr << "KKStr::KKStr   ***ERROR***  requested SubStr[" << startPos << ", " << endPos << "]  len[" << subStrLen << "] is greater than MaxStrLen[" << (MaxStrLen - 1) << "]" << std::endl;
    endPos = (startPos + MaxStrLen - 2);
    subStrLen = 1 + endPos - startPos;
  }

  AllocateStrSpace (1 + subStrLen);             // Need one extra byte for NULL terminating character.
  if  (!val)
    throw KKException ("KKStr::KKStr   Allocation Failed");

  memcpy (val, src + startPos, subStrLen);
  len = (kkStrUint)subStrLen;
  val[subStrLen] = 0;
}



void  KKStr::AllocateStrSpace (kkStrUint  size)
{
  if  (size < 1)
    size = 1;

  if  (val)
  {
    KKStr  msg = "KKStr::AllocateStrSpace   ***ERROR***      Previous val was not deleted.";
    cerr << std::endl << msg << std::endl << std::endl;
    throw KKException (msg);
  }

  if  (size >= MaxStrLen)
  {
    //  Can not allocate this much space;  This string has gotten out of control.
    KKStr  errStr (150);
    errStr << "KKStr::AllocateStrSpace   ***ERROR***      Size["  << size << "] is larger than MaxStrLen[" << MaxStrLen << "]";
    cerr << std::endl << errStr << std::endl << std::endl;
    throw KKException (errStr);
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
  allocatedSize = (kkStrUint)size;
  len = 0;
}  /* AllocateStrSpace */




kkMemSize  KKStr::MemoryConsumedEstimated () const  
{
  return  (kkMemSize)(sizeof (char*) + 2 * sizeof (kkStrUint) + allocatedSize);
}



void  KKStr::GrowAllocatedStrSpace (kkStrUint  newAllocatedSize)
{
  if  (newAllocatedSize < allocatedSize)
  {
    KKStr  errMsg (128);
    errMsg << "KKStr::GrowAllocatedStrSpace  ***ERROR***   newAllocatedSize[" << newAllocatedSize << "]  is smaller than allocatedSize[" << allocatedSize << "]";
    cerr  << std::endl << std::endl << errMsg << std::endl << std::endl;
    throw  KKException (errMsg);
  }

  if  (newAllocatedSize >= (MaxStrLen - 5))
  {
    //  Can not allocate this much space;  This string has gotten out of control.
    KKStr  errMsg (128);
    errMsg << "KKStr::GrowAllocatedStrSpace   ***ERROR***   NewAllocatedSize[" << newAllocatedSize << "] is larger than MaxStrLen[" << (MaxStrLen - 5) << "]";
    throw KKException (errMsg);
  }

  newAllocatedSize += 5;  // Lets allocate a little extra space on the hope that we will save a lot of cycles reallocating again this string.
  {
    // Will check to see if reasonable to grow by 25% over current len.
    kkuint32  newAllocatedWithExtraRoomForGrowth = Min ((kkuint32)(len + (len / 4)), (MaxStrLen - 1));
    if  ((newAllocatedWithExtraRoomForGrowth > newAllocatedSize)  &&  (newAllocatedSize < MaxStrLen))
      newAllocatedSize = newAllocatedWithExtraRoomForGrowth;
  }

  if  (val == NULL)
  {
    val = new char[newAllocatedSize];
    if  (val == NULL)
    {
      KKStr  errMsg(128);
      errMsg << "KKStr::GrowAllocatedStrSpace   ***ERROR***   Allocation of NewAllocatedSize[" << newAllocatedSize << "] characters failed.";
      cerr << endl << errMsg << endl <<endl;
      throw KKException(errMsg);
    }
    memset (val, 0, newAllocatedSize);
    allocatedSize = newAllocatedSize;
  }
  else
  {
    char*  newVal = new char[newAllocatedSize];
    if (newVal == NULL)
    {
      KKStr  errMsg(128);
      errMsg << "KKStr::GrowAllocatedStrSpace   ***ERROR***   Allocation of NewAllocatedSize[" << newAllocatedSize << "] characters failed.";
      cerr << endl << errMsg << endl << endl;
      throw KKException(errMsg);
    }

    memset (newVal, 0, newAllocatedSize);
    memcpy (newVal, val, allocatedSize);
    delete[]  val;
    val = newVal;
    allocatedSize = newAllocatedSize;
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



KKStr  KKStr::ToBase64Str (uchar const * buff, kkStrUint buffLen)
{
	return Nyffenegger::base64_encode (buff, buffLen);
}



kkint32  KKStr::Compare (const KKStr&  s2)  const
{
  kkint32  zed = Min (len, s2.len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.val;

  for  (kkint32 x = 0;  x < zed;  x++)
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
 *@brief  Compares with STL string.
 *@param[in]  s2  STL String  std::string that we will compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
kkint32  KKStr::Compare (const std::string&  s2)  const
{
  kkStrUint  s2Len = (kkStrUint)s2.size ();
  kkStrUint  zed   = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (kkStrUint x = 0;  x < zed;  x++)
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
 *@brief  Compares with another KKStr, ignoring case.
 *@param[in]  s2  Other String to compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
kkint32  KKStr::CompareIgnoreCase (const KKStr&  s2)  const
{
  kkint32  zed = Min (len, s2.len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.val;

  for  (kkint32 x = 0;  x < zed;  x++)
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
 *@brief  Compares with ascii-z string ignoring case.
 *@param[in]  s2  Ascii-z string to compare with.
 *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
 */
kkint32  KKStr::CompareIgnoreCase (const char* s2)  const
{
  if  (s2 == NULL)
  {
    if  (len == 0)
      return  0;
    else
      return  1;
  }

  kkuint32  s2Len = 0;
  if  (s2 != NULL)
    s2Len = (kkuint32)strlen (s2);
  kkuint32  zed = Min ((kkuint32)len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2;

  for  (kkStrUint x = 0;  x < zed;  x++)
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



kkint32  KKStr::CompareIgnoreCase (const std::string&  s2)  const
{
  kkStrUint  s2Len = (kkStrUint)s2.size ();
  kkStrUint  zed = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (kkStrUint x = 0;  x < zed;  x++)
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

  kkuint32  len = 0;
  kkint32  x = 0;
  while  (values[x] != NULL)
  {
    len += (kkuint32)strlen (values[x]);
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
  kkuint32 x   = 0;
  kkint32 len  = 0;

  for  (x = 0;  x < values.size ();  x++)
    len += (kkint32)values.size ();

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
  kkuint32 x   = 0;
  kkint32 len  = 0;

  for  (x = 0;  x < values.size ();  x++)
    len += (kkint32)values.size ();

  KKStr  result (len);
  x = 0;
  for  (x = 0;  x < values.size ();  x++)
  {
    result.Append (values[x]);
    x++;
  }

  return  result;
}  /* Concat */



bool  KKStr::Contains (const KKStr& value) const
{
  if  (value.Empty ())
    return true;
  else
    return StrInStr (value);
}



bool  KKStr::Contains (const char*  value) const
{
  if  ((value == NULL)  ||  (*value == 0))
    return true;
  else
    return (Find (value, 0).has_value ());
}



kkint32  KKStr::CountInstancesOf (char  ch)  const
{
  if  (!val)
    return 0;
  kkint32  count = 0;
  for  (kkStrUint x = 0;  x < len;  ++x)
  {
    if  (val[x] == ch)
      ++count;
  }
  return  count;
}



bool  KKStr::StartsWith (const KKStr&  value)  const
{
  return  StartsWith (value, false);
}



bool  KKStr::StartsWith (const char*  value)  const
{
  return  StartsWith (value, false);
}



bool   KKStr::StartsWith (const KKStr& value,   
                          bool  ignoreCase
                         )  const
{
  if  (value.len == 0)
    return true;

  if  (value.len > len)
    return  false;

  if  (ignoreCase)
    return  StrEqualNoCaseN (val, value.val, value.len);
  else
    return  StrEqualN       (val, value.val, value.len);
}



bool   KKStr::StartsWith (const char* value,   
                          bool        ignoreCase
                         )  const
{
  if  (value == NULL)
    return true;

  kkint32  valueLen = (kkint32)strlen (value);

  if  (ignoreCase)
    return  StrEqualNoCaseN (val, value, valueLen);
  else
    return  StrEqualN       (val, value, valueLen);
}



bool   KKStr::EndsWith (const KKStr& value)  const
{
  return  EndsWith (value, false);
}



bool   KKStr::EndsWith (const char* value)  const
{
  return  EndsWith (value, false);
}



bool   KKStr::EndsWith (const KKStr& value,   
                        bool  ignoreCase
                       )  const
{
  if  (value.len == 0)
    return true;

  kkint32  startPos = 1 + len - value.Len ();
  if  (startPos < 0)
    return false;

  if  (ignoreCase)
    return  StrEqualNoCase (val + startPos, value.val);
  else
    return  StrEqual  (val + startPos, value.val);
}



bool   KKStr::EndsWith (const char* value,   
                        bool ignoreCase
                       )  const
{
  if  (value == NULL)
    return true;

  kkint32  valueLen = (kkint32)strlen (value);

  kkint32  startPos = 1 + len - valueLen;
  if  (startPos < 0)
    return false;

  if  (ignoreCase)
    return  StrEqualNoCase (val + startPos, value);
  else
    return  StrEqual (val + startPos, value);
}



bool  KKStr::EqualIgnoreCase (const KKStrConstPtr  s2)  const
{
  return  EqualIgnoreCase  (s2->Str ());
}



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

  w = new wchar_t[len + 1];
  for  (kkStrUint  x = 0; x < len;  x++)
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


KKStr&   KKStr::operator= (KKStr&& src)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  src.ValidateLen ();
  #endif

  delete  val;
  val           = src.val;
  allocatedSize = src.allocatedSize;
  len           = src.len;
  src.val           = NULL;
  src.allocatedSize = 0;
  src.len           = 0;

  return *this;
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

  kkStrUint  spaceNeeded = (src.len + 1U);
  if  ((spaceNeeded > allocatedSize)  ||  (!val))
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
    if  (!val)
      throw KKException ("KKStr::operator=");
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

  kkStrUint  newLen = (kkStrUint)strlen (src);
  kkStrUint  spaceNeeded = (kkStrUint)(newLen + 1U);

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



KKStr&  KKStr::operator= (kkint32  right)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%d", right);

  kkStrUint  newLen = (kkStrUint)strlen (buff);

  kkStrUint  spaceNeeded = newLen + (kkStrUint)1;

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



KKStr&  KKStr::operator= (const std::vector<KKStr>& right)
{
  kkStrUint  spaceNeeded = 2;  /* Start with 2 bytes for overhead. */
  for  (kkStrUint x = 0;  x < right.size ();  x++)
    spaceNeeded += right[x].Len ();

  if  (spaceNeeded > allocatedSize)
  {
    delete  val;
    val = NULL;
    allocatedSize = 0;
    AllocateStrSpace (spaceNeeded);
  }

  if  (!val)
  {
    KKStr  errMsg = "KKStr::operator= (const std::vector<KKStr>& right)   ***ERROR***   Space for string not allocatd!";
    cerr << endl << errMsg << endl <<endl;
    throw KKException (errMsg);
  }

  char*  ptr = val;
  kkStrUint  allocatedSpaceNotUsed = allocatedSize - 1;
  for  (size_t x = 0;  x < right.size ();  x++)
  {
    kkStrUint  rightLen = right[x].Len ();
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



bool  KKStr::operator== (KKStrConstPtr right)  const
{
  if  (!right)
    return false;

  return  (Compare (*right) == 0);
}



bool  KKStr::operator!= (KKStrConstPtr  right)  const
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



void  KKStr::ChopFirstChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)  return;

  if  (len > 0)
  {
    for  (kkStrUint x = 0;  x < len;  ++x)
      val[x] = val[x + 1];
    len--;
    val[len] = 0;
  }
}  /* ChopLastChar */



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



KKStr&  KKStr::Trim (const char* whiteSpaceChars)
{
  TrimRight (whiteSpaceChars);
  TrimLeft(whiteSpaceChars);
  return  *this;  
}  /* Trim */



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

  kkint32  x = len - 1;
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

  kkStrUint  x = 0;

  while  ((strchr (whiteSpaceChars, val[x]))  &&  (val[x] != 0))
    ++x;

  if  (x == 0)
    return;

  kkStrUint  y = 0;

  while  (x < len)
  {
    val[y] = val[x];
    ++x;
    ++y;
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

  kkStrUint  buffLen = (kkStrUint)strlen (buff);
  kkStrUint  newLen = len + buffLen;
  kkStrUint  neededSpace = newLen + 1;

  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= MaxStrLen)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                buffLen[" << buffLen         << "]" << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      throw KKException ("KKStr::Append  Length will exceed what KKStr can managed;  neededSpace: " + StrFromUint32 (neededSpace));
    }
    GrowAllocatedStrSpace (neededSpace);
  }

  kkStrUint  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }
  val[len] = 0;
}  /* Append */



void  KKStr::Append (const char*  buff,
                     kkuint32     buffLen
                   )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (buffLen == 0)  
    return;

  kkStrUint  newLen = len + buffLen;
  kkStrUint  neededSpace = newLen + 1;

  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= MaxStrLen)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                buffLen[" << buffLen         << "]" << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      throw KKException("KKStr::Append  Length will exceed what KKStr can managed;  neededSpace: " + StrFromUint32(neededSpace));
    }
    GrowAllocatedStrSpace (neededSpace);
  }

  kkStrUint  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }

  val[len] = 0;
}  /* Append*/



void  KKStr::Append (char ch)
{
  kkStrUint  neededSpace = len + 2;
  if  (neededSpace > allocatedSize)
  {
    if  (neededSpace >= MaxStrLen)
    {
      cerr << std::endl 
           << "KKStr::Append   ***ERROR***   Size of buffer can not fit into String." << std::endl
           << "                neededSpace[" << neededSpace << "]" << std::endl
           << std::endl;
      throw KKException("KKStr::Append (char ch)  Length will exceed what KKStr can managed;  neededSpace: " + StrFromUint32(neededSpace));
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



void  KKStr::AppendInt32 (kkint32  i)
{
  Append (StrFromInt32 (i));
  return;
}  /* AppendInt32 */



void  KKStr::AppendUInt32 (kkuint32  i)
{
  Append (StrFromUint32 (i));
  return;
}  /* AppendUInt32 */



char  KKStr::FirstChar ()  const
{
  if  ((val == NULL)  ||  (len < 1))
    return 0;

  return  val[0];
}



void  KKStr::FreeUpUnUsedSpace()
{
  kkStrUint unUsedSpace = allocatedSize - (len + 1);
  kkStrUint acceptableWaist = allocatedSize / 5;
  if  (unUsedSpace > acceptableWaist)
  {
    kkuint32 neededSpace = len + 1;
    char* newVal = new char[neededSpace];
    if (newVal == NULL)
    {
      KKStr  errMsg(128);
      errMsg << "KKStr::FreeUpUnUsedSpace   ***ERROR***   Allocation of NeededSpace[" << neededSpace << "] characters failed.";
      cerr << endl << errMsg << endl << endl;
      throw KKException(errMsg);
    }

    memcpy(newVal, val, len);
    newVal[len] = 0;
    delete[]  val;
    val = newVal;
    allocatedSize = neededSpace;
  }
}



char  KKStr::LastChar ()  const
{
  if  ((val == NULL)  ||  (len < 1))
    return 0;
  return val[len - 1];
}  /* LastChar */



OptionUInt32  KKStr::LocateCharacter (char ch)  const
{
  if  (!val)
    return  {};

  kkStrUint idx = 0;
  while  (idx < len)
  {
    if  (val[idx] == ch)
      return  idx;
    idx++;
  }

  return  {};
}  /* LocateCharacter */



kkint32  KKStr::InstancesOfChar (char ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (val == NULL)
    return  0;

  kkint32  count = 0;
  for  (kkStrUint x = 0; x < len;  x++)
  {
    if  (val[x] == ch)
      count++;
  }

  return  count;
}  /* InstancesOfChar */



kkint32  MemCompare (const char* s1,
                     const char* s2,
                     kkuint32    s1Idx,
                     kkuint32    s2Idx,
                     kkuint32    len
                    )
{
  for  (kkuint32 x = 0;  x < len;  x++)
  {
    if  (s1[s1Idx] < s2[s2Idx])
      return -1;
    
    else if  (s1[s1Idx] > s2[s2Idx])
      return 1;

    ++s1Idx;
    ++s2Idx;
  }

  return 0;
}  /* MemCompare */



/**
 *@brief  Returns the position of the last occurrence of the character 'ch'.
 *@details A return of -1 indicates that there is no occurrence of 'ch' in the string.
 */
OptionUInt32  KKStr::LocateLastOccurrence (char  ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val  ||  (len < 1))
    return  {};

  kkStrUint  lastIdx = len;
  while  (lastIdx > 0)
  {
    --lastIdx;
    if  (val[lastIdx] == ch)
      return  lastIdx;
  }

  return {};
}  /* LocateLastOccurrence */



/**
 *@brief  Returns the position of the last occurrence of the string 's'.
 *@details A return of -1 indicates that there is no occurrence of 's' in the string.
 */
OptionUInt32  KKStr::LocateLastOccurrence (const KKStr&  s)  const
{
  kkStrUint  sLen = s.Len ();

  if  ((!val)  ||  (!s.val)  ||  (sLen <= 0)  ||  (sLen > len))
    return {};

  kkStrUint  lastIdx = len - sLen;
  const char* curPos = val + lastIdx;
  const char* sVal = s.val;
  while (true)
  {
    if (strncmp (curPos, sVal, sLen) == 0)
      return lastIdx;

    else if  (lastIdx == 0)
      break;

    --curPos;
    --lastIdx;
  }
  return  {};
}  /* LocateLastOccurrence */



OptionUInt32  KKStr::LocateNthOccurrence (char ch,  kkint32 n)  const
{
  if  (!val)
    return {};

  kkint32 numInstances = 0;
  kkStrUint  x = 0;
  while  ((x < len)  &&  (numInstances < n))
  {
    if  (val[x] == ch)
      ++numInstances;
    ++x;
  }

  if  (numInstances < n)
    return {};
  else
    return (x - 1);
}



/**
 *@brief returns the position of the 1st occurrence of the string 'searchStr'.
 *@details A return of -1 indicates that there is no occurrence of 'searchStr' in the string.
 */
OptionUInt32  KKStr::LocateStr (const KKStr&  searchStr)  const
{
  kkStrUint  searchStrLen = searchStr.Len ();
  if  ((!val)  ||  (!searchStr.val) ||  (searchStrLen > len))
    return {};

  kkStrUint  maxPossibilities = 1 + len - searchStrLen;
  const char* curPos = val;
  for (kkStrUint x = 0; x < maxPossibilities; ++x, ++curPos)
  {
    if (std::memcmp (curPos, searchStr.val, searchStrLen) == 0)
      return x;
  }
  return {};
} /* LocateStr */



KKStr  KKStr::Wide (kkStrUint width,
                    char      dir
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

    if  (width > str.Len ())
    {
      kkStrUint  x = (width - str.Len ()) / 2;
      str = Spaces (x)  + str;
      str.RightPad (width, ' ');
    }
  }

  else
    str.RightPad (width, ' ');

  return str;
}  /* Wide */



void  KKStr::RightPad (kkStrUint  width,
                       char       ch
                      )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
  {
    AllocateStrSpace (width + 1);
    len = 0;
  }

  if  (len > width)
  {
    len = width;
    for  (kkStrUint x = len;  x < allocatedSize;  ++x)
      val[x] = 0;
  }

  else
  {
    kkStrUint  neededSpace = width + 1;

    if  (neededSpace > allocatedSize)
      GrowAllocatedStrSpace (neededSpace); 
   
    while (len < width)
    {
      val[len] = ch;
      len++;
    }

    val[len] = 0;
  }
}  /* RightPad */



void  KKStr::LeftPad (kkStrUint width,
                      uchar     ch
                     )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  kkStrUint  neededSpace = width + 1;

  if  (!val)
  {
    len = 0;
    allocatedSize = 0;
  }

  if  (neededSpace > allocatedSize)
  {
    GrowAllocatedStrSpace(neededSpace);
    KKCheck (val, "KKStr::LeftPad   ***ERROR***   val == NULL; couuld not allocate neededSpace: " << neededSpace)
  }

  if  (len >= width)
  {
    kkStrUint  toIdx    = 0;
    kkStrUint  fromIdx  = len - width;
    while  (fromIdx < len)
    {
      val[toIdx] = val[fromIdx];
      val[fromIdx] = 0;
      ++toIdx;
      ++fromIdx;
    }
    len = width;
    val[len] = 0;
  }
  else
  {
    kkint64  fromIdx = len - 1;
    kkint64  toIdx   = width - 1;
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

    len = width;
    val[len] = 0;
  }
  return;
}  /* LeftPad */



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

  for  (kkStrUint x = 0; x < len; x++)
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

  for  (kkStrUint x = 0; x < len; x++)
    val[x] = (uchar)tolower (val[x]);
}  /* Lower */



KKStr  KKStr::MaxLen (kkStrUint  maxLen)  const
{
  maxLen = Max (0U, maxLen);
  if  (len < maxLen)
    return *this;
  else
    return SubStrSeg (0, maxLen);
}



kkuint32 KKStr::MaxLenSupported ()
{
  return MaxStrLen - 1;
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



KKStrPtr  KKStr::ToKKStrPtr () const
{
  return new KKStr (*this);
}



bool  KKStr::ValidInt (kkint32  &value)
{

  kkint32  sign = 1;

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

       kkint32  digit;
               
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
  kkint32  digit = 0;
  kkint32  sign  = 1;
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
  kkint32 decimalDigits = 0;

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
     value = value * 10.0f + (float)digit;
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

  value = value * (float)sign;
  return  (*ch == 0);
}  //  ValidMoney



bool  KKStr::ValidNum (double&  value)  const
{
  kkint32  digit = 0;
  kkint32  sign  = 1;
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
  kkint32 decimalDigits = 0;

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

  for  (kkStrUint x = 0;  x < len;  x++)
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



KKStr  KKStr::SubStrPart (kkStrUint  firstCharIdx)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (firstCharIdx >= len)
    return "";

  kkStrUint  subStrLen = len - firstCharIdx;
  KKStr  subStr (subStrLen + 1);
  subStr.Append (((char*)&(val[firstCharIdx])), subStrLen);

  return  subStr;
}  /* SubStrPart */



KKStr  KKStr::SubStrPart (kkint32  firstCharIdx)  const
{
  return SubStrPart ((kkStrUint)Max ((kkint32)0, firstCharIdx));
}



KKStr  KKStr::SubStrPart (OptionUInt32  firstCharIdx)  const
{
  return SubStrPart ((kkStrUint)Max (0U, firstCharIdx.value ()));
}



KKStr  KKStr::SubStrPart (kkStrUint  firstCharIdx,
                          kkStrUint  lastCharIdx
                         )  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (len < 1)
    return EmptyStr ();

  KKCheck(lastCharIdx < MaxStrLen, "KKStr::SubStrPart  lastCharIdx: " << lastCharIdx << " Beyond max allowable stlen!")

  if  (lastCharIdx >= len)  lastCharIdx = len - 1;

  if ((firstCharIdx >= len) || (lastCharIdx < firstCharIdx))
    return  EmptyStr ();

  kkStrUint  subStrLen = (kkStrUint)(lastCharIdx - firstCharIdx) + 1;
  KKStr  subStr (subStrLen + 2);

  kkStrUint  x = (kkStrUint)firstCharIdx;
  kkStrUint  y = 0;

  for  (x = firstCharIdx; x <= lastCharIdx;  x++, y++)
    subStr.val[y] = val[x];
  
  subStr.val[y] = 0;
  subStr.len = subStrLen;
  return  subStr;
}  /* SubStrPart */




KKStr  KKStr::SubStrPart (kkStrUint     firstCharIdx,
                          OptionUInt32  lastCharIdx
                         )  const
{
  if  (!lastCharIdx)
    throw bad_optional_access ();
  return SubStrPart (firstCharIdx, lastCharIdx.value ());
}



KKStr  KKStr::SubStrSeg (kkStrUint     firstCharIdx,
                         OptionUInt32  segmentLen
                        )  const
{
  return SubStrSeg (firstCharIdx, segmentLen.value ());
}



KKStr  KKStr::SubStrSeg (kkStrUint  firstCharIdx,
                         kkStrUint  segmentLen
                        )  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (len < 1)
    return EmptyStr ();

  KKCheck(segmentLen < MaxStrLen, "KKStr::SubStrSeg  segmentLen: " << segmentLen<< " larger than allowed max string length!")
  if (firstCharIdx >= len)
    return  EmptyStr ();

  kkuint64 zed = firstCharIdx + segmentLen;
  KKCheck(zed < KKStr::MaxStrLen, "KKStr::SubStrSeg  numerical overflow occured!")
  zed = Min (len, (kkStrUint)zed);

  segmentLen = (kkStrUint)zed - firstCharIdx;
  KKStr  subStr (segmentLen + 2);

  for  (kkStrUint x = firstCharIdx, y = 0;  x < zed;  ++x, ++y)
    subStr.val[y] = val[x];
  
  subStr.val[segmentLen] = 0;
  subStr.len = segmentLen;
  return  subStr;
}



/**
 *@brief  Returns a string consisting of the 'tailLen' characters from the end of the string.
 *@details
 *@code
 *ex:
 *   if  test = "Hello World.";
 *       test.Tail (2) will return "d.".
 *@endcode
 */
KKStr  KKStr::Tail (kkStrUint tailLen)  const
{
  if  (tailLen > len)
    return *this;

  kkStrUint  firstCharIdx = len - tailLen;
  return  SubStrPart (firstCharIdx);
}  /* Tail */



/**
 *@brief Remove characters from the end of the string starting at 'lastCharPos'.
 *@details Removes characters from end of string starting at position 'lastCharPos'.  If 'lastCharPos'
 *         is greater than length of string will do nothing.  If 'lastCharPos' is less than or
 *         equal to '0' will delete all characters from string.
 *@param[in] lastCharPos Will remove all characters starting at 'lastCharPos' from end of string.
 */
void  KKStr::LopOff (kkStrUint lastCharPos)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (lastCharPos >= len)
    return;

  kkStrUint  newLen = (kkStrUint)(lastCharPos + 1);
  while  (len > newLen)
  {
    len--;
    val[len] = 0;
  }
}  /* LoppOff */



KKStr  KKStr::QuotedStr ()  const
{
  if  ((!val)  ||  (len < 1))
    return "\"\"";
  
  KKStr  result (Len () + 5);

  result.Append ('"');

  kkStrUint  idx = 0;

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



KKStr  KKStr::ToXmlStr ()  const
{
  if  ((!val)  ||  (len < 1))
  {
    return KKStr::EmptyStr ();
  }
  
  KKStr  result (Len () + 5);
  
  kkStrUint  idx = 0;
  while  (idx < len)
  {
    switch  (val[idx])
    {
      case  '<' : result.Append ("&lt;");    break;
      case  '>' : result.Append ("&gt;");    break;
      case  '&' : result.Append ("&amp;");   break;
      case  '\"': result.Append ("&quot;");  break;
      case  '\'': result.Append ("&apos;");  break;
      case  '\t': result.Append ("&tab;");   break;
      case  '\n': result.Append ("&nl;");    break;
      case  '\r': result.Append ("&cr;");    break;
      case  '\\': result.Append ("&bs;");    break;
      case     0: result.Append ("&null;");  break;
         
      default:   result.Append (val[idx]);   break;
    }

    idx++;
  }

  return  result;
}  /* ToXmlStr */



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
    delete[] val;
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

    len = (kkStrUint)strlen (tokenNext);
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
    delete[] val;
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

    len = (kkStrUint)strlen (tokenNext);
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
  
  kkStrUint  startCharPos = 0;

  while  (startCharPos < len)
  {
    if  (val[startCharPos] != ' ')
      break;
    startCharPos++;
  }

  if  (startCharPos >= len)
    return  "";

  kkStrUint  lastCharPos = startCharPos;

  while  (lastCharPos < len)
  {
    if  (strchr (delStr, val[lastCharPos]) != NULL)
    {
      // We just found the first delimiter
      return  SubStrSeg (startCharPos, lastCharPos - startCharPos);
    }
    lastCharPos++;
  }

  return  SubStrPart (startCharPos);
}  /* GetNextToken2 */



kkint32  KKStr::ExtractTokenInt (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);
  return  atoi (workStr.Str ());
}



kkuint32  KKStr::ExtractTokenUint (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);
  return workStr.ToUint32 ();
}



KKB::kkuint64  KKStr::ExtractTokenUint64 (const char* delStr)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  KKStr  workStr = ExtractToken2 (delStr);
  return  workStr.ToUint64 ();
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
   
  kkStrUint  newLen = len - (kkStrUint)1;

  for  (kkStrUint  x = 0;  x < newLen; x++)
  {
    val[x] = val[x + 1];
  }

  len = newLen;
  val[len] = 0;

  return  returnChar;
}  /* ExtractChar */



char  KKStr::ExtractLastChar ()
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  ((!val)  ||  (len < 1))
    return 0;

  --len;
  char lastChar = val[len];
  val[len] = 0;

  return lastChar;
}  /* ExtractLastChar */



KKStr  KKStr::DecodeQuotedStr ()  const
{
  if  ((!val)  ||  (len == 0))
    return  EmptyStr ();

  KKStr  result (len + 4);
  kkint32  idx = 0;
   
  kkint32  lastCharPos = len - 1;

  if  ((val[idx] == '"')  &&  (val[len - 1] == '"'))
  {
    ++idx;
    --lastCharPos;
  }

  while  (idx <= lastCharPos)
  {
    if  (val[idx] == '\\')
    {
      ++idx;
      if  (idx <= lastCharPos)
      {
        uchar escapeChar = 0;
        switch (val[idx]) {
          case '0': escapeChar = 0;
          case 'a': escapeChar = '\a';
          case 'b': escapeChar = '\b';
          case 'f': escapeChar = '\f';
          case 'n': escapeChar = '\n';
          case 'r': escapeChar = '\r';
          case 't': escapeChar = '\t';
          case 'v': escapeChar = '\v';
          default:  escapeChar = val[idx];
        }
        result.Append (escapeChar);
        ++idx;
      }
    }
    else
    {
      result.Append (val[idx]);
      ++idx;
    }
  }

  return  result;
}  /* DecodeQuotedStr */



KKStr  KKStr::ExtractQuotedStr (const char*  delChars,
                                bool         decodeEscapeCharacters
                               )
{
  if  ((!val)  ||  (len == 0))
    return  EmptyStr ();

  KKStr  result (len);
  kkStrUint  idx = 0;
   
  bool  lookForTerminatingQuote = false;

  char termQuoteChar = '"';

  if  ((val[idx] == '"')  ||  (val[idx] == '\''))
  {
    lookForTerminatingQuote = true;
    termQuoteChar = val[idx];
    idx++;
  }
  
  if  (idx >= len)
  {
    delete[] val;
    val = NULL;
    AllocateStrSpace (1);
    return  result;
  }

  // Search for matching terminating Quote
  while  (idx < len)
  {
    if  (lookForTerminatingQuote)
    {
      if  (val[idx] == termQuoteChar)
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
         case  'b': result.Append ('\b');     break;
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
    len = (kkStrUint)(len - idx);
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
}  /* Split */



bool  KKStr::ToBool () const
{
  if  (len < 1)  return false;

  if  (STRICMP (val, "false")   == 0)  return false;
  if  (STRICMP (val, "true")    == 0)  return true;
  if  (STRICMP (val, "f")       == 0)  return false;
  if  (STRICMP (val, "t")       == 0)  return true;
  if  (STRICMP (val, "0")       == 0)  return false;
  if  (STRICMP (val, "1")       == 0)  return true;
  if  (STRICMP (val, "no")      == 0)  return false;
  if  (STRICMP (val, "yes")     == 0)  return true;
  if  (STRICMP (val, "n")       == 0)  return false;
  if  (STRICMP (val, "y")       == 0)  return true;

  if  (STRICMP (val, "si")      == 0)  return true;
  if  (STRICMP (val, "cierto")  == 0)  return true;
  if  (STRICMP (val, "falso")   == 0)  return false;

  if  (STRICMP (val, "ya")      == 0)  return true;
  if  (STRICMP (val, "wahr")    == 0)  return true;
  if  (STRICMP (val, "nine")    == 0)  return false;
  if  (STRICMP (val, "falsch")  == 0)  return false;

  if  (STRICMP (val, "oui")     == 0)  return true;
  if  (STRICMP (val, "vrai")    == 0)  return true;
  if  (STRICMP (val, "non")     == 0)  return false;
  if  (STRICMP (val, "faux")    == 0)  return false;

  if  (STRICMP (val, "da")      == 0)  return true;
  if  (STRICMP (val, "pravda")  == 0)  return true;
  if  (STRICMP (val, "nyet")    == 0)  return false;
  if  (STRICMP (val, "lozhnyy") == 0)  return false;

  return false;
}



double  KKStr::ToDouble () const
{
  if  (!val)
    return 0.0;

  double  d = atof (val);
  return d;
}  /* ToDouble */



float  KKStr::ToFloat () const
{
  if  (!val)
    return 0.0f;

  double d = atof (val);
  if  (fabs (d) > FLT_MAX)
  {
    KKStr errMsg;
    errMsg << "KKStr::ToFloat ()  val: " << val << "  exceeds capacity of float: " << FLT_MAX;
    cerr << errMsg << endl;
    throw KKException (errMsg);
  }

  return (float)d;
}   /* ToFloat */



kkint32  KKStr::ToInt () const
{
  if  (!val)
    return 0;

  kkint64 l = atoll (val);
  KKCheck ((l >= INT_MIN) && (l <= INT_MAX), "KKStr::ToInt   val: " << val << " exceeds capacity of 32 bit int.")

  return (kkint32)l;
}  /* ToInt*/



kkint16  KKStr::ToInt16 () const
{
  if  (!val)
    return 0;

  kkint64 ll = atoll (val);
  KKCheck ((ll >= INT16_MIN) && (ll <= INT16_MAX), "KKStr::ToInt16   val: " << val << " exceeds capacity of 16 bit int.")

  return (kkint16)ll;
}  /* ToInt16*/



kkint32  KKStr::ToInt32 () const
{
  if  (!val)
    return 0;

  kkint64 ll = atoll (val);
  KKCheck ((ll >= INT32_MIN) && (ll <= INT32_MAX), "KKStr::ToInt32   val: " << val << " exceeds capacity of 32 bit int.")

  return (kkint32)ll;
}  /* ToInt32*/



KKB::kkint64  KKStr::ToInt64 () const
{
  if  (!val)  return 0;

  #if  defined(__GNUC__)
    return atoll (val);
  #else
    return  (kkint64)_atoi64 (val);
  #endif
}



long  KKStr::ToLong   () const
{
  if  (!val)
    return 0;

  kkint64  ll = atoll (val);
  KKCheck ((ll >= LONG_MIN)  &&  (ll <= LONG_MAX), "KKStr::ToLong  val: " << val << " exceeds capacity of long.")

  return (long)ll;
}  /* ToLong */



float  KKStr::ToPercentage () const
{
  if  (LastChar () == '%')
  {
    KKStr  workStr = this->SubStrSeg (0, len - 1);
    return  workStr.ToFloat ();
  }

  return  100.0f * ToFloat ();
}



uint  KKStr::ToUint () const
{
  if  (!val)  return 0;

  kkint64 ll = atoll(val);
  KKCheck ((ll >= 0)  &&  (ll <= UINT_MAX), "KKStr::ToUint ()    val: " << val << " exceeds capacity of uint.")
  return  (kkuint32)ll;
}



KKB::ulong  KKStr::ToUlong () const
{
  kkint64 ll = atoll(val);
  KKCheck ((ll >= 0)  &&  (ll <= ULONG_MAX), "KKStr::ToUlong ()    val: " << val << " exceeds capacity of ulong.")
  return  (ulong)ll;
}



KKB::kkuint16  KKStr::ToUint16 () const
{
  kkint64 ll = atoll(val);
  KKCheck ((ll >= 0)  &&  (ll <= UINT16_MAX), "KKStr::ToUint16 ()    val: " << val << " exceeds capacity of uint16.")
  return  (kkuint16)ll;
}



KKB::kkuint32  KKStr::ToUint32 () const
{
  kkint64 ll = atoll(val);
  KKCheck ((ll >= 0)  &&  (ll <= UINT32_MAX), "KKStr::ToUint32 ()    val: " << val << " exceeds capacity of uint32.")
  return  (kkuint32)ll;
}



KKB::kkuint64  KKStr::ToUint64 () const
{
  if  (!val)  return 0;
  #if  defined(__GNUC__)
    return  (kkuint64)atoll (val);
  #else
    return  (kkuint64)_atoi64 (val);
  #endif
}


VectorInt32*  KKStr::ToVectorInt32 ()  const
{
  VectorInt32*  results = new VectorInt32 ();

  KKStrParser parser (val);

  KKStr  field = parser.GetNextToken (",\t \n\r");
  while  (!field.Empty ())
  {
    auto  dashPos = field.LocateCharacter ('-');
    if  (!dashPos)
    {
      // This is not a range
      results->push_back (field.ToInt32 ());
    }
    else
    {
      // We are looking at a range
      kkint32  startNum = field.SubStrSeg (0, dashPos).ToInt32 ();
      kkint32  endNum   = field.SubStrPart (dashPos + 1).ToInt32 ();
      for  (kkint32 z = startNum;  z <= endNum;  ++z)
        results->push_back (z);
    }
    field = parser.GetNextToken (",\t \n\r");
  }
  return  results;
}  /* ToVectorint32 */



wchar_t*  KKStr::ToWchar_t () const
{
  size_t returnValue;

  wchar_t* wa = NULL;
  if  (val == NULL)
  {
    wa = new wchar_t[1];
    #if  defined(USE_SECURE_FUNCS)
      mbstowcs_s(&returnValue, wa, 1, "", 1);
    #else
      returnValue = mbstowcs (wa, "", 1);
    #endif
  }
  else
  {
    size_t  wideLen = len + 1;
    wa = new wchar_t[wideLen];
    #if  defined(USE_SECURE_FUNCS)
      mbstowcs_s(&returnValue, wa, wideLen, val, len);
    #else
      returnValue = mbstowcs (wa, val, len);
    #endif
  }

  if  (returnValue == (size_t)-1)
    cerr << "KKStr::ToWchar_t ()   ***WARNING***    Invalid character was encountered." << endl;

  return wa;
}



double  KKStr::ToLatitude ()  const
{
  KKStr latitudeStr (*this);
  latitudeStr.Trim ();

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
  latitudeStr.TrimRight ();

  if  (latitudeStr.FirstChar () == '-')
  {
    latitudeStr.ChopFirstChar ();
    north = !north;
    latitudeStr.TrimLeft ();
  }

  double  degrees = 0.0;
  double  minutes = 0.0;
  double  seconds = 0.0;

  KKStr  degreesStr = "";
  KKStr  minutesStr = "";
  KKStr  secondsStr  = "";

  auto  x = latitudeStr.LocateCharacter (':');
  if  (x.has_value ())
  {
    degreesStr = latitudeStr.SubStrSeg (0, x);
    degreesStr.TrimRight ();
    minutesStr = latitudeStr.SubStrPart (x + 1);
    minutesStr.Trim ();
  }
  else
  {
    x = latitudeStr.LocateCharacter (' ');
    if  (x)
    {
      degreesStr = latitudeStr.SubStrSeg (0, x);
      degreesStr.TrimRight ();
      minutesStr = latitudeStr.SubStrPart (x + 1);
      minutesStr.Trim ();
    }
    else
    {
      degreesStr = latitudeStr;
      minutesStr = "";
    }
  }

  x = minutesStr.LocateCharacter (':');
  if  (x)
  {
    secondsStr = minutesStr.SubStrPart (x + 1);
    minutesStr = minutesStr.SubStrSeg (0, x);
    secondsStr.Trim ();
  }
  else
  {
    x = minutesStr.LocateCharacter (' ');
    if  (x)
    {
      secondsStr = minutesStr.SubStrPart (x + 1);
      minutesStr = minutesStr.SubStrSeg (0, x);
      secondsStr.Trim ();
    }
  }
 
  degrees = degreesStr.ToDouble ();
  minutes = minutesStr.ToDouble ();
  seconds = secondsStr.ToDouble ();

  double  latitude = degrees + (minutes / 60.0) + (seconds / 3600.0);
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

  KKStr  degreesStr = "";
  KKStr  minutesStr = "";
  KKStr  secondsStr  = "";

  auto  x = longitudeStr.LocateCharacter (':');
  if  (x)
  {
    degreesStr = longitudeStr.SubStrSeg (0, x);
    degreesStr.TrimRight ();
    minutesStr = longitudeStr.SubStrPart (x + 1);
    minutesStr.Trim ();
  }
  else
  {
    x = longitudeStr.LocateCharacter (' ');
    if  (x)
    {
      degreesStr = move(longitudeStr.SubStrSeg (0, x));
      degreesStr.TrimRight ();
      minutesStr = longitudeStr.SubStrPart (x + 1);
      minutesStr.Trim ();
    }
    else
    {
      degreesStr = longitudeStr;
      minutesStr = "";
    }
  }

  x = minutesStr.LocateCharacter (':');
  if  (x)
  {
    secondsStr = minutesStr.SubStrPart (x + 1);
    minutesStr = minutesStr.SubStrSeg (0, x);
    secondsStr.Trim ();
  }
  else
  {
    x = minutesStr.LocateCharacter (' ');
    if  (x)
    {
      secondsStr = minutesStr.SubStrPart (x + 1);
      minutesStr = minutesStr.SubStrSeg (0, x);
      secondsStr.Trim ();
    }
  }
 
  degrees = degreesStr.ToDouble ();
  minutes = minutesStr.ToDouble ();
  seconds = secondsStr.ToDouble ();

  double  longitude = degrees + (minutes / 60.0) + (seconds / 3600.0);
  while  (longitude > 180.0)
    longitude = longitude - 360.0;
  if  (!east)
    longitude = 0.0 - longitude;

  return  longitude;
}  /* ToLongitude */



OptionUInt32  SearchStr (const char*   src,
                         kkuint32      srcLen,
                         kkuint32      startPos,
                         const char*   srchStr,
                         kkuint32      srchStrLen
                        )
{
  if  ((!src)  ||  (!srchStr))
    return {};

  kkuint32  zed = (startPos + srchStrLen - 1);
  if  (zed > srcLen)
    return  {};

  kkuint32 numIter = (srcLen - zed);
  const char* startCh = src + startPos;

  for  (kkuint32 x = 0;  x < numIter;  ++x, ++startCh)
  {
    if  (strncmp (startCh, srchStr, srchStrLen) == 0)
      return  startPos + x;
  }
  return {};
}



OptionUInt32  KKStr::Find (const KKStr&  str, kkStrUint pos) const
{
  return  SearchStr (val, len, pos, str.Str (), str.Len ());
}



OptionUInt32  KKStr::Find (const char* s,  kkStrUint pos,  kkStrUint n)  const
{
  return  SearchStr (val, len, pos, s, n);
}



OptionUInt32  KKStr::Find (const char* s, kkStrUint pos) const
{
  return  SearchStr (val, len, pos, s, (kkStrUint)strlen (s));
}



OptionUInt32  KKStr::Find (char c, kkStrUint pos) const
{
  for  (kkStrUint x = pos;  x < len;  x++)
  {
    if  (val[x] == c)
      return  x;
  }
  return {};
}



KKStr  KKB::operator+ (const char    left,
                       const KKStr&  right
                      )
{
  KKStr  result (right.Len () + 3);
  result.Append (left);
  result.Append (right);
  return  result;
}



KKStr  KKB::operator+ (const char*   left,
                       const KKStr&  right
                      )
{
  return  KKStr (left) + right;
}



KKStr  KKStr::operator+ (const char*  right)  const
{
  kkint32  resultStrLen = len + (kkint32)strlen (right);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (right);
  return  result;
}



KKStr  KKStr::operator+ (const KKStr&  right)  const
{
  kkint32  resultStrLen = len + right.len;

  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (right);
  return  result;
}



KKStr  KKStr::operator+ (kkint16  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%-ld", right);
  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (kkuint16  right)  const
{
  char  buff[30];
  SPRINTF (buff, sizeof (buff), "%u", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (kkint32  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%-ld", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (kkuint32  right)  const
{
  char  buff[30];
  SPRINTF (buff, sizeof (buff), "%u", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (kkint64 right)  const
{
  char  buff[70];
  SPRINTF (buff, sizeof (buff), "%-lld", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (kkuint64 right)  const
{
  char  buff[70];
  SPRINTF (buff, sizeof (buff), "%-llu", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (float  right)  const
{
  char  buff[60];

  SPRINTF (buff, sizeof (buff), "%.9g", right);

  kkint32  resultStrLen = len + (kkint32)strlen (buff);
  KKStr  result (resultStrLen + 1);
  result.Append (*this);
  result.Append (buff);
  return  result;
}



KKStr  KKStr::operator+ (double  right)  const
{
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%.17g", right);

  kkint32 resultStrLen = len + (kkint32)strlen (buff);
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



KKStr&  KKStr::operator<< (KKStr&&  right)
{
  if  ((len < 1)  &&  (allocatedSize <= right.allocatedSize))
  {
    delete val;
    val           = right.val;
    len           = right.len;
    allocatedSize = right.allocatedSize;

    right.val           = NULL;
    right.len           = 0;
    right.allocatedSize = 0;
  }
  else
  {
    Append (right.Str ());
  }
  
  return  *this;
}



KKStr&  KKStr::operator<< (kkint16  right)
{
  AppendInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (kkuint16  right)
{
  AppendUInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (kkint32 right)
{
  AppendInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (kkuint32  right)
{
  AppendUInt32 (right);
  return  *this;
}



KKStr&  KKStr::operator<< (kkint64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



KKStr&  KKStr::operator<< (kkuint64  right)
{
  KKStr  s (30);
  s = StrFormatInt64 (right, "0");
  Append (s.Str ());
  return  *this;
}



KKStr&  KKStr::operator<< (float  right)
{  
  char  buff[60];
  SPRINTF (buff, sizeof (buff), "%.9g", right);
  if  (strchr (buff, '.') != NULL)
  {
    // Remove trailing Zeros
    kkint32  buffLen = (kkint32)strlen (buff);
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
    kkint32  buffLen = (kkint32)strlen (buff);
    while  ((buffLen > 1)  &&  (buff[buffLen - 1] == '0')  &&  (buff[buffLen - 2] == '0'))
    {
      buffLen--;
      buff[buffLen] = 0;
    }
  }
  Append (buff);
  return *this;
}



KKStr&  KKStr::operator<< (istream&  right)
{
  bool lastCharCR = false;
  auto ch = right.get();
  while (!right.eof() && (ch != '\n'))
  {
    if (lastCharCR)
    {
      Append('\r');
      lastCharCR = false;
    }
    if (ch == '\r')
    {
      lastCharCR = true;
    }
    else
    {
      Append((char)ch);
    }
    ch = right.get();
  }
  return *this;
}



void  Test2 (ostream& x1, const char* x2)
{
  x1 << x2;
}



#ifdef  WIN32
ostream& __cdecl  KKB::operator<< (      ostream&  os, 
                                   const KKStr&   strng
                                  )
{
  os << (strng.Str ());
  return os;
}



std::istream& __cdecl  KKB::operator>> (std::istream&  is,
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

std::ostream& KKB::operator<< (      std::ostream&  os, 
                               const KKStr&         strng
                              )
{
  Test2 (os, strng.Str ());
  // os << (strng.Str ());
  return os;
}



std::istream&  KKB::operator>> (std::istream&  is,
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



KKStr  KKStr::Spaces (kkStrUint  c)
{
  KKStr s;
  s.RightPad (c);
  return  s;
}



void  KKStr::MemCpy (void*      dest,
                     void*      src,
                     kkStrUint  size
                    )
{
  KKCheck (dest, "KKStr::MemCpy   ***ERROR***    (dest == NULL)")
  KKCheck (src,  "KKStr::MemCpy   ***ERROR***    (src  == NULL)")

  memcpy (dest, src, size);
}



void  KKStr::MemSet (void* dest,  kkuint8  byte, kkStrUint  size)
{
  KKCheck (dest, "KKStr::MemCpy   ***ERROR***    (dest == NULL)")
  memset (dest, byte, size);
}



void  KKStr::StrCapitalize (char*  str)
{
  if  (!str)
     return;
  
  char* ch = str;
  while  (*ch)
  {
    *ch = (char)toupper (*ch);
    ++ch;
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



void  KKStr::WriteXML (const KKStr&  varName,
                       ostream&      o
                      )  const
{
  XmlTag startTag ("KKStr", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Len", len);
  startTag.WriteXML (o);
  XmlContent::WriteXml (val, o);
  XmlTag  endTag ("KKStr", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



void  KKStr::ReadXML (XmlStream&      s,
                      XmlTagConstPtr  tag,
                      VolConstBool&   cancelFlag,
                      RunLog&         log
                     )
{
  kkStrUint  expectedLen = tag->AttributeValueInt32 ("Len");
  delete[] val;
  val = NULL;
  allocatedSize = 0;
  if  (expectedLen > 0)
    AllocateStrSpace (expectedLen);

  delete[]  val;
  val = NULL;
  allocatedSize = 0;
  AllocateStrSpace (expectedLen);

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      Append (*(c->Content ()));
    }
    delete  t;
    if  (cancelFlag)
      t = NULL;
    else
      t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;

  TrimRight (" \r\n");
}  /* ReadXML */



KKStrList::KKStrList ():
  KKQueue<KKStr> (false),
  sorted (false)
{
}



KKStrList::KKStrList (bool  _owner):
  KKQueue<KKStr> (_owner),
  sorted (false)
{
}



KKStrList::KKStrList (const char*  s[]):
    KKQueue<KKStr> (true),
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



kkMemSize  KKStrList::MemoryConsumedEstimated ()  const
{
  kkMemSize  memoryConsumedEstimated = sizeof (KKStrList);
  KKStrList::const_iterator idx;
  for  (idx = this->begin ();  idx != this->end ();  ++idx)
    memoryConsumedEstimated += (*idx)->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



bool  KKStrList::StringInList (KKStr& str)
{
  bool  found = false;
  kkint32 idx;
  kkint32 qSize = QueueSize ();

  for  (idx = 0; ((idx < qSize) && (!found)); idx++)
    found = (str == (*IdxToPtr (idx)));

  return  found;
}



void  KKStrList::ReadXML (XmlStream&      s,
                          XmlTagConstPtr  tag,
                          VolConstBool&   cancelFlag,
                          RunLog&         log
                         )
{
  kkuint32  count = (kkuint32)tag->AttributeValueInt32 ("Count");

  DeleteContents ();

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr  c = dynamic_cast<XmlContentPtr>(t);
      if  ((c != NULL)  &&  (c->Content () != NULL))
      {
        KKStrParser  parser (*(c->Content ()));
        parser.TrimWhiteSpace (" ");
        while  (parser.MoreTokens ())
        {
          KKStr field = parser.GetNextToken ("\t");
          PushOnBack (new KKStr (field));
        }
      }
    }
    delete t;
    if  (cancelFlag)
      t = NULL;
    else
      t = s.GetNextToken (cancelFlag, log);
  }

  if  (count != size ())
  {
    cerr << "KKStrList::ReadXML    ***WARNING***    Expected " << count 
         << " items but " << size () << " were loaded;" 
         << " Tage Name: '" << tag->Name () << "'." 
         << endl;
  }

  delete  t;
  t = NULL;
}  /* ReadXML */



void  KKStrList::WriteXML (const KKStr&  varName,
                           ostream&      o
                         )  const
{
  XmlTag  startTag ("KKStrList", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Count", (kkint32)size ());
  startTag.WriteXML (o);

  const_iterator  idx;
  kkuint32 x = 0;
  for  (idx = begin ();  idx != end ();  ++idx, ++x)
  {
    KKStrPtr  sp = *idx;
    if  (x > 0)  o << "\t";
    XmlContent::WriteXml (sp->QuotedStr (), o);
  }
  XmlTag  endTag ("KKStrList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */



optional<KKStrPtr>  KKStrList::BinarySearch (const KKStr&  searchStr)
{
  if  (!sorted)
  {
    KKStr errMsg = "KKStrList::BinarySearch     **** ERROR ****        KKStr List is Not Sorted";
    cerr << endl << errMsg << endl << endl;
    throw KKException (errMsg);
  }
  
  kkint32  low  = 0;
  kkint32  high = QueueSize () - 1;
  kkint32  mid;

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

  return  {};
}  /* BinarySearch */


  
void   KKStrList::AddString (KKStrPtr  str)
{
  PushOnBack (str);
  sorted = false;
}



KKStrListPtr  KKStrList::ParseDelimitedString (const KKStr&  str,
                                               const char*   delChars
                                              )
{
  KKStrListPtr  parms = new KKStrList (true);

# ifdef  USE_SECURE_FUNCS
    char*  workStr =  _strdup (str.Str ());
# else
    char*  workStr =  strdup (str.Str ());
# endif

  char*  nextChar = workStr;

  while  (*nextChar)
  {
    // Skip Past Leading Blanks
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
kkint32  KKStr::CompareStrings (const KKStr&  s1, 
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
  KKStrListPtr  newList = new KKStrList (true);
  KKStrList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  idx++)
    newList->PushOnBack (new KKStr (*(*idx)));

  return  newList;
}  /* DuplicateListAndContents */



OptionUInt32  LocateLastOccurrence (const char*  str,
                                    char         ch
                                   )
{
  if  (!str)
    return {};

  kkStrUint  idx = (kkStrUint)strlen (str);
  while  (idx > 0)
  {
    --idx;
    if  (str[idx] == ch)
      return idx;
  }

  return {};
}  /* LocateLastOccurrence */



KKStr  KKB::StrFormatDouble (double       val,
                             const char*  mask
                            )
{
  // Get number of decimal Places

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

  kkint32 numOfDecimalPlaces = 0;

  kkStrUint maskLen = (kkStrUint)strlen (mask);
  
  auto decimalPosition = LocateLastOccurrence (mask, '.');

  const char*  maskPtr = mask + maskLen - 1; 

  kkuint64  intPart = (kkuint64)floor (val);

  kkuint32  nextDigit = 0;

  if  (decimalPosition)
  {
    numOfDecimalPlaces = maskLen - decimalPosition.value () - 1;
    printDecimalPoint = true;
    maskPtr = mask + decimalPosition.value () - 1;
    maskLen = decimalPosition.value ();
  }

  if  (printDecimalPoint)
  {
    double  power = pow ((double)10, (double)numOfDecimalPlaces);

    double  frac = val - floor (val);

    kkint32  fracInt = (kkint32)(frac * power + 0.5);

    for  (kkint32 x = 0; x < numOfDecimalPlaces; x++)
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
      //  fracInt will end up equaling 1.000. because of rounding.  
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
           nextDigit = (kkuint32)(intPart % 10);
           intPart = intPart / 10;
           bp--;
           *bp = (uchar)('0' + nextDigit);
           break;

      case  '#':
      case  '9':
           if (intPart > 0)
           {
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
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
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
           }
           break;

      case  '-':
           if  (intPart > 0)
           {
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
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
    nextDigit = (kkuint32)(intPart % 10);
    intPart = intPart / 10;
    bp--;
    *bp = (uchar)('0' + nextDigit);
  }

  if  (!negativePrinted)
  {
    bp--;
    *bp = '-';
  }

  return  KKStr (bp);
}  /* StrFormatDouble */



KKStr  KKB::StrFormatInt (kkint32      val,
                          const char*  mask
                         )
{
  return  KKB::StrFormatDouble ((double)val, mask);
}



KKStr  KKB::StrFormatInt64 (kkint64        val,
                            const char*  mask
                           )
{
  // Get number of decimal Places

  char  buff[128];
  char* bp = buff + 127;
  *bp = 0;

  bool  negativePrinted = true;

  if  (val < 0)
  {
    negativePrinted = false;
    val = 0 - val;
  }

  kkint32 maskLen = (kkint32)strlen (mask);
  const char*  maskPtr = mask + maskLen - 1; 

  kkuint64  intPart = val;

  kkuint32  nextDigit = 0;

  char  formatChar = ' ';
  char  lastFormatChar = ' ';

  while  (maskLen > 0)
  {
    formatChar = (uchar)toupper (*maskPtr);

    switch (formatChar)
    {
      case  '0': 
      case  '@': 
           nextDigit = (kkuint32)(intPart % 10);
           intPart = intPart / 10;
           bp--;
           *bp = (uchar)('0' + nextDigit);
           break;

      case  '#':
      case  '9':
           if (intPart > 0)
           {
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
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
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
           }
           break;

      case  '-':
           if  (intPart > 0)
           {
             nextDigit = (kkuint32)(intPart % 10);
             intPart = intPart / 10;
             bp--;
             *bp = (uchar)('0' + nextDigit);
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
    nextDigit = (kkuint32)(intPart % 10);
    intPart = intPart / 10;
    bp--;
    *bp = (uchar)('0' + nextDigit);
  }

  if  (!negativePrinted)
  {
    bp--;
    *bp = '-';
  }

  return  KKStr (bp);
}  /* StrFormatInt */


KKStr KKB::StrFromBuff (const char* buff, kkuint32 buffLen)
{
  return KKStr (buff, 0, buffLen - 1);
}



KKStr  KKB::StrFromInt16 (kkint16 i)
{
  char  buff[50];

  SPRINTF (buff, sizeof (buff), "%d", i);
  KKStr s (buff);
  return  s;
}  /* StrFromInt16 */



KKStr  KKB::StrFromUint16 (kkuint16 ui)
{
  char  buff[50];

  SPRINTF (buff, sizeof (buff), "%u", ui);
  KKStr s (buff);
  return  s;
}  /* StrFromUint16 */



KKStr  KKB::StrFromInt32 (kkint32 i)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%ld", i);
  KKStr s (buff);
  return  s;
}  /* StrFromInt32 */



KKStr  KKB::StrFromUint32 (kkuint32 ui)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%lu", ui);
  KKStr s (buff);
  return  s;
}  /* StrFromUint32 */



KKStr  KKB::StrFromInt64 (kkint64 i64)
{
  char  buff[50];
  
  SPRINTF (buff, sizeof (buff), "%lld", i64);
  KKStr s (buff);
  return  s;
}  /* StrFromInt64 */



KKStr  KKB::StrFromUint64 (kkuint64 ul)
{
  char  buff[50];
  SPRINTF (buff, sizeof (buff), "%llu", ul);
  KKStr s (buff);
  return  s;
}  /* StrFromUint64 */



KKStr  KKB::StrFromFloat (float f)
{
  char  buff[50];
  SPRINTF (buff, sizeof (buff), "%f", f);
  KKStr s (buff);
  return  s;
}  /* StrFromFloat */



KKStr  KKB::StrFromDouble (double  d)
{
  char  buff[50];
  SPRINTF (buff, sizeof (buff), "%g", d);
  KKStr s (buff);
  return  s;
}  /* StrFromFloat */



KKStr& KKStr::operator<< (std::ostream& (* mf)(std::ostream &))
{
  ostringstream  o;
  mf (o);
  Append (o.str ().c_str ());
  return  *this;
}


const  kkStrUint  KKB::KKStr::MaxStrLen = UINT32_MAX - 2;


VectorKKStr::VectorKKStr ():
  vector<KKStr> ()
{
}



VectorKKStr::VectorKKStr (const VectorKKStr&  v):
  vector<KKStr> (v)
{
}



void  VectorKKStr::ReadXML (XmlStream&      s,
                            XmlTagConstPtr  tag,
                            VolConstBool&   cancelFlag,
                            RunLog&         log
                          )
{
  kkuint32  count = (kkuint32)tag->AttributeValueInt32 ("Count");

  clear ();

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);

  while  (t  &&  (!cancelFlag))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr  c = dynamic_cast<XmlContentPtr>(t);
      if  ((c != NULL)  &&  (c->Content () != NULL))
      {
        KKStrParser  parser (*(c->Content ()));
        parser.TrimWhiteSpace (" ");
        while  (parser.MoreTokens ())
        {
          KKStr field = parser.GetNextToken ("\t");
          push_back (field);
        }
      }
    }

    delete t;
    if  (cancelFlag)
      t = NULL;
    else
      t = s.GetNextToken (cancelFlag, log);
  }

  delete  t;
  t = NULL;

  if  (count != size ())
  {
    cerr << "VectorKKStr::ReadXML    ***WARNING***    Expected " << count 
         << " items but " << size () << " were loaded;" 
         << " Tage Name: '" << tag->Name () << "'." 
         << endl;
  }
}  /* ReadXML */



void  VectorKKStr::WriteXML (const KKStr&  varName,
                             ostream&      o
                            )  const
{
  XmlTag  startTag ("VectorKKStr", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("Count", (kkint32)size ());
  startTag.WriteXML (o);

  const_iterator  idx;
  kkuint32 x = 0;
  for  (idx = begin ();  idx != end ();  ++idx, ++x)
  {
    if  (x > 0)  o << "\t";
    XmlContent::WriteXml (idx->QuotedStr (), o);
  }
  XmlTag  endTag ("VectorKKStr", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}



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



KKStrListIndexed::KKStrListIndexed ():
  comparator              (true),
  indexIndex              (),
  memoryConsumedEstimated (0),
  nextIndex               (0),
  owner                   (true),
  strIndex                (NULL)
{
  strIndex = new StrIndex (comparator);
  memoryConsumedEstimated = sizeof (*this);
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
  DeleteContents ();
  indexIndex.clear ();
  strIndex->clear ();
}



void  KKStrListIndexed::DeleteContents ()
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



size_t  KKStrListIndexed::size ()  const
{
  return  indexIndex.size ();
}



kkMemSize  KKStrListIndexed::MemoryConsumedEstimated ()  const
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



kkint32  KKStrListIndexed::Add (KKStrPtr  s)
{
  StrIndex::iterator  idx;
  idx = strIndex->find (s);
  if  (idx != strIndex->end ())
    return -1;

  kkint32  index = nextIndex;
  ++nextIndex;

  strIndex->insert (StrIndexPair (s, index));
  indexIndex.insert (IndexIndexPair (index, s));

  if  (owner)
    memoryConsumedEstimated += s->MemoryConsumedEstimated ();
  memoryConsumedEstimated += 8;
  return  index;
}  /* Add */



kkint32   KKStrListIndexed::Delete (KKStr&  s)
{
  StrIndex::iterator  idx;
  idx = strIndex->find (&s);
  if  (idx == strIndex->end ())
    return -1;
 
  KKStrPtr  strIndexStr = idx->first;

  kkint32  index = idx->second;
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



kkint32  KKStrListIndexed::LookUp (const KKStr& s)  const
{
  StrIndex::const_iterator  idx;

  KKStr  sNotConst (s);

  idx = strIndex->find (&sNotConst);
  if  (idx == strIndex->end ())
    return -1;
  else
    return idx->second;
}


kkint32  KKStrListIndexed::LookUp (KKStrPtr s)  const
{
  StrIndex::iterator  idx;
  idx = strIndex->find (s);
  if  (idx == strIndex->end ())
    return -1;
  else
    return idx->second;
}



KKStrConstPtr  KKStrListIndexed::LookUp (kkuint32 x)  const
{
  IndexIndex::const_iterator  idx;
  idx = indexIndex.find (x);
  if  (idx == indexIndex.end ())
    return NULL;
  else
    return idx->second;
}



void  KKStrListIndexed::ReadXML (XmlStream&      s,
                                 XmlTagConstPtr  tag,
                                 VolConstBool&   cancelFlag,
                                 RunLog&         log
                                )
{
  DeleteContents ();
  comparator.caseSensitive = tag->AttributeValueByName ("CaseSensative")->ToBool ();

  XmlTokenPtr  t = s.GetNextToken (cancelFlag, log);
  while  (t  &&  (!cancelFlag))
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*c->Content ());

      while  (p.MoreTokens ())
      {
        KKStr  tokStr = p.GetNextToken (",\t\n\r");
        Add (new KKStr (tokStr));
      }
    }
    delete  t;
    if  (cancelFlag)
      t = NULL;
    else
      t = s.GetNextToken (cancelFlag, log);
  }
  delete  t;
  t = NULL;
}  /* ReadXML */



/**@brief Strings will be separated by tab(\t) characters and in order of index. */
KKStr  KKStrListIndexed::ToTabDelString ()  const
{
  KKStr  s ((kkStrUint)indexIndex.size () * 20);
  IndexIndex::const_iterator  idx;
  for  (idx = indexIndex.begin ();  idx != indexIndex.end ();  ++idx)
  {
    if (!s.Empty ())
      s << "\t";
    s << *(idx->second);
  }
  return s;
}



void  KKStrListIndexed::WriteXML (const KKStr& varName,  ostream& o)  const
{
  XmlTag  startTag ("KKStrListIndexed", XmlTag::TagTypes::tagStart);

  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("CaseSensative", caseSensative);
  startTag.WriteXML (o);

  size_t n = size ();
  for  (kkuint32 x = 0;  x < n;  ++x)
  {
    KKStrConstPtr s = LookUp ((kkint32)x);
    if  (x > 0)  o << "\t";
    XmlContent::WriteXml (s->QuotedStr (), o);
  }

  XmlTag endTag ("KKStrListIndexed", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}
