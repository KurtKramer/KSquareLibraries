/* Str.cpp -- String Management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <string.h>

#include "MemoryDebug.h"
using namespace std;

#include "KKQueue.h"

#include "KKStr.h"
#include "KKException.h"
#include "KKStrParser.h"
#include "RunLog.h"
#include "XmlStream.h"
using namespace KKB;



char*  KKB::STRCOPY (char*        dest,
                     kkuint16     destSize,
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



char*  KKB::STRCOPY (char*        dest,
                     kkint32      destSize,
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
    strcat (dest, src);
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
                        kkint32        len
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
    return sprintf (buff, formatSpec, right);
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
    return sprintf (buff, formatSpec, right);
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
    return sprintf (buff, formatSpec, right);
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
    return sprintf (buff, formatSpec, right);
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
    return sprintf (buff, formatSpec, right);
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
    return sprintf (buff, formatSpec, right);
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
  return  sprintf (buff, formatSpec, precision, d);
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



KKStr::LessCaseInsensitiveOperator::LessCaseInsensitiveOperator ()
{}



bool  KKStr::LessCaseInsensitiveOperator::operator ()  (const KKStr&  s1,  
                                                        const KKStr&  s2
                                                       )
{
  char* s1Ptr = (char*)s1.Str ();
  char* s2Ptr = (char*)s2.Str ();

  if  (s1Ptr == NULL)
    return (s2Ptr != NULL);

  if  (s2Ptr == NULL)
    return false;

  while  ((*s1Ptr != 0)  &&  (tolower (*s1Ptr) == tolower (*s2Ptr)))
  {
    ++s1Ptr;
    ++s2Ptr;
  }

  return  (*s1Ptr) < (*s2Ptr);
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
  
  kkuint32  newLen = (kkuint32)strlen (str);
  AllocateStrSpace (newLen + 1);

  STRCOPY (val, (kkuint16)allocatedSize, str);

  len = (kkuint16)newLen;
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

  kkuint16  neededSpace = str.len + 1;
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
 @brief  Constructs a new KKStr from a pointer to a KKStr.
 */
KKStr::KKStr (KKStrConstPtr  str): 
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
  
  kkuint16  neededSpace = str->len + 1;

  if  (neededSpace > str->allocatedSize)
  {
    cerr << std::endl;
    cerr << "KKStr::KKStr (const KKStr&  str)   **** ERROR ****" << std::endl;
    cerr << "        AllocatedSize["  << str->allocatedSize << "]  on Source KKStr is to Short" << std::endl;
    cerr << "        for Str[" << str->val << "]." << std::endl;
    throw "KKStr  Constructor (const KKStrPtr&  str)   'AllocatedSize' on source is too small.";
  }

  AllocateStrSpace (str->allocatedSize);

  STRCOPY (val, allocatedSize, str->val);

  len = str->len;
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

  kkuint16  newLen = (kkuint16)strlen (buff); 
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
 *@brief Creates a KKStr object that has 'size' characters preallocated; and set to empty string.
 */
KKStr::KKStr (kkint32  size):
        val (NULL)
{
  if  (size <= 0)
    size = 1;

  else if  ((kkuint32)size >= StrIntMax)
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
  AllocateStrSpace ((kkint32)(s.size () + 1));

  len = (kkuint16)s.size ();
  for  (kkint32 x = 0;  x < len;  ++x)
    val[x] = s[x];
  val[len] = 0;
}



/** @brief  Constructs a KKStr instance from a substr of 'src'.  */
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
  if  (subStrLen > (StrIntMax - 1))
  {
    cerr << "KKStr::KKStr   ***ERROR***  requested SubStr[" << startPos << ", " << endPos << "]  len[" << subStrLen << "] is greater than StrIntMax[" << (StrIntMax - 1) << "]" << std::endl;
    endPos = (startPos + StrIntMax - 2);
    subStrLen = 1 + endPos - startPos;
  }

  AllocateStrSpace (1 + subStrLen);             // Need one extra byte for NULL terminating character.

  memcpy (val, src + startPos, subStrLen);
  len = (kkuint16)subStrLen;
  val[subStrLen] = 0;
}



void  KKStr::AllocateStrSpace (kkuint32  size)
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
  allocatedSize = (kkuint16)size;
  len = 0;
}  /* AllocateStrSpace */



kkint32  KKStr::MemoryConsumedEstimated () const  
{
  return sizeof (char*) + 2 * sizeof (kkuint16) + allocatedSize;
}



void  KKStr::GrowAllocatedStrSpace (kkuint32  newAllocatedSize)
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
    //  Can not allocate this much space;  This string has gotten out of control.
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
    allocatedSize = (kkuint16)newAllocatedSize;
  }
  else
  {
    char*  newVal = new char[newAllocatedSize];
    memset (newVal, 0, newAllocatedSize);
    memcpy (newVal, val, allocatedSize);
    delete  val;
    val = newVal;
    allocatedSize = (kkuint16)newAllocatedSize;
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
  kkuint16  s2Len = (kkuint16)s2.size ();
  kkint32  zed = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (kkint32 x = 0;  x < zed;  x++)
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

  for  (kkuint16 x = 0;  x < zed;  x++)
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
  kkuint16  s2Len = (kkuint16)s2.size ();
  kkint32  zed = Min (len, s2Len);

  const char*  s1Ptr = val;
  const char*  s2Ptr = s2.c_str ();

  for  (kkint32 x = 0;  x < zed;  x++)
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



kkint32  KKStr::CountInstancesOf (char  ch)  const
{
  if  (!val)
    return 0;
  kkint32  count = 0;
  for  (kkint32 x = 0;  x < len;  ++x)
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



bool  KKStr::StartsWith (const char*   value)  const
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
                          bool ignoreCase
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







bool   KKStr::EndsWith (const KKStr& value)
{
  return  EndsWith (value, false);
}



bool   KKStr::EndsWith (const char* value)
{
  return  EndsWith (value, false);
}



bool   KKStr::EndsWith (const KKStr& value,   
                        bool  ignoreCase
                       )
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
                       )
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

  kkint32  x;
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


KKStr&  KKStr::operator= (const KKStrConstPtr src)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  src.ValidateLen ();
  #endif

  if  (src == this)
  {
    // We are assigning our selves to our selves;  there is nothing to do.
    return *this;
  }

  kkuint16  spaceNeeded = src->len + 1;
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

  if  (src->val)
    memcpy (val, src->val, src->len);

  len = src->len;
  val[len] = 0;

  return *this;
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

  kkuint16  spaceNeeded = src.len + 1;
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

  kkuint16  newLen = (kkuint16)strlen (src);
  kkuint16  spaceNeeded = newLen + 1;

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

  kkuint16  newLen = (kkuint16)strlen (buff);

  kkuint16  spaceNeeded = newLen + 1;

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
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  kkint32  spaceNeeded = 2;  /* Start with 2 bytes for overhead. */
  kkuint32  x = 0;
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
  kkint32  allocatedSpaceNotUsed = allocatedSize - 1;
  for  (x = 0;  x < right.size ();  x++)
  {
    kkint32  rightLen = right[x].Len ();
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
    for  (int x = 0;  x < len;  ++x)
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

  kkuint16  x = 0;

  while  ((strchr (whiteSpaceChars, val[x]))  &&  (val[x] != 0))
    x++;

  if  (x == 0)
    return;

  kkuint16  y = 0;

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

  kkuint32  buffLen = (kkuint32)strlen (buff);
  kkuint32  newLen = len + buffLen;
  kkuint32  neededSpace = newLen + 1;

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

  kkuint32  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }
  val[len] = 0;
}  /* Append */



void  KKStr::Append (const char*   buff,
                           kkuint32  buffLen
                    )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (buffLen == 0)  
    return;

  kkuint32  newLen = len + buffLen;
  kkuint32  neededSpace = newLen + 1;

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

  kkuint32  x = 0;
  for  (x = 0;  x < buffLen;  x++)
  {
    val[len] = buff[x];
    len++;
  }

  val[len] = 0;
}  /* Append*/






void  KKStr::Append (char ch)
{
  kkuint32  neededSpace = len + 2;
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



void  KKStr::AppendInt32 (kkint32  i)
{
  if  (i == 0)
  {
    Append ('0');
    return;
  }

  char  buff[20];
  kkint16  bi = sizeof (buff) - 1;
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
    kkint16  digit = i % 10;
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



void  KKStr::AppendUInt32 (kkuint32  i)
{
  if  (i == 0)
  {
    Append ('0');
    return;
  }

  char  buff[20];
  kkint16  bi = sizeof (buff) - 1;
  buff[bi] = 0;

  while  (i > 0)
  {
    --bi;
    kkint16  digit = i % 10;
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




kkint32  KKStr::LocateCharacter (char  ch)  const
{
  if  (!val)
    return  -1;

  kkint32  idx    = 0;

  while  (idx < len)
  {
    if  (val[idx] == ch)
      return  idx;
    idx++;
  }

  return  -1;
}  /* LocateCharacter */




kkint32  KKStr::InstancesOfChar (char ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (val == NULL)
    return  0;

  kkint32  count = 0;

  for  (kkuint16 x = 0; x < len;  x++)
  {
    if  (val[x] == ch)
      count++;
  }

  return  count;
}  /* InstancesOfChar */



kkint32  MemCompare (const char* s1,
                   const char* s2,
                   kkint32     s1Idx,
                   kkint32     s2Idx,
                   kkint32     len
                )
{
  for  (kkint32 x = 0;  x < len;  x++)
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
kkint32  KKStr::LocateStr (const KKStr&  searchStr)  const
{
  if  ((!val)  ||  (!(searchStr.val)))
    return  -1;

  kkint32  idx = 0;

  kkint32  lastIdx = len - searchStr.len;

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
kkint32  KKStr::LocateLastOccurrence (char  ch)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return -1;

  bool   found  = false;
  kkint32  idx    = len - 1;

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
kkint32  KKStr::LocateLastOccurrence (const KKStr&  s)  const
{
  kkint32  sLen = s.Len ();

  if  ((!val)  ||  (!s.val)  ||  (sLen <= 0)  ||  (sLen > len))
    return -1;

  bool  found  = false;
  kkint32 idx    = len - sLen;

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



kkint32  KKStr::LocateNthOccurrence (char ch,  kkint32 n)  const
{
  if  (!val)
    return -1;

  kkint32 numInstances = 0;
  kkint32  x = 0;
  while  ((x < len)  &&  (numInstances < n))
  {
    if  (val[x] == ch)
      ++numInstances;
    ++x;
  }

  if  (numInstances < n)
    return -1;
  else
    return (x - 1);
}





KKStr  KKStr::Wide (kkint32 width,
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

    kkint32  x = (width - str.Len ()) / 2;

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




void  KKStr::RightPad (kkint32  width,
                       char ch
                      )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (width < 0)
  {
    cerr << std::endl;
    cerr << "KKStr::RightPad (kkint32  width,  char ch)    **** ERROR ****" << std::endl;
    cerr << "                width[" << width << "]  invalid." << std::endl;
    cerr << std::endl;
    exit (-1);
  }

  if  (!val)
  {
    AllocateStrSpace (width + 1);
    len = 0;
  }

 
  if  (len > (kkuint16)width)
  {
    len = (kkuint16)width;
    for  (kkint32 x = len;  x < allocatedSize;  x++)
      val[x] = 0;
  }

  else
  {
    kkuint32  neededSpace = width + 1;

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
   
    while (len < (kkuint16)width)
    {
      val[len] = ch;
      len++;
    }

    val[len] = 0;
  }
}  /* RightPad */




void  KKStr::LeftPad (kkint32 width, 
                      uchar ch
                     )
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (width < 0)
  {
    cerr << std::endl;
    cerr << "KKStr::LeftPad (kkint32  width,  char ch)    **** ERROR ****" << std::endl;
    cerr << "                width[" << width << "]  invalid." << std::endl;
    cerr << std::endl;
    width = 0;
  }

  if  (!val)
  {
    len = 0;
    allocatedSize = 0;
  }

  kkuint32  neededSpace = (kkuint32)width + 1;
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

  if  (len >= (kkuint16)width)
  {
    // 2010-04-20
    // This code has never been debugged.  So the first time we run it
    // we want to make sure that it is doing what I say it is doing.
    /** @todo  Need to properly debug through 'KKStr::LeftPad; */
    kkuint16  toIdx    = 0;
    kkuint16  fromIdx  = len - (kkuint16)width;
    while  (fromIdx < len)
    {
      val[toIdx] = val[fromIdx];
      val[fromIdx] = 0;
      ++toIdx;
      ++fromIdx;
    }
    len = (kkuint16)width;
    val[len] = 0;
  }
  else
  {
    kkint32  fromIdx = len - 1;
    kkint32  toIdx   = width - 1;
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

    len = (kkuint16)width;
    val[len] = 0;
  }
  return;
}  /* LeftPad */





char  KKStr::EnterStr ()
{
  kkint32 bp = 0;
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
          // We have a control character.
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

  kkuint16  newLen = (kkuint16)strlen (buff);

  kkuint16  neededSpace = newLen + 1;

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

  kkuint32  x;

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

  kkuint32  x;

  for  (x = 0; x < len; x++)
    val[x] = (uchar)tolower (val[x]);
}  /* Lower */



KKStr  KKStr::MaxLen (kkint32  maxLen)  const
{
  maxLen = Max ((kkint32)0, maxLen);
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

  for  (kkint32 x = 0;  x < len;  x++)
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
KKStr  KKStr::SubStr (kkint32  firstChar,
                      kkint32  subStrLen
                     )
{
  kkuint32  lastChar;
  KKStr  subStr;
  kkuint32  x;
  kkuint32  y;


  if  ((subStrLen < 1)  ||  (!val))
  {
    subStr = "";
    return  subStr;
  }
   

  lastChar = firstChar + subStrLen - 1;

  if  (lastChar >= len)
     lastChar = len - 1;   

  kkint32  neededSpace = 1 + subStrLen;

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



KKStr  KKStr::SubStrPart (kkint32  firstChar)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  ((kkuint16)firstChar >= len)
    return "";

  if  (firstChar < 0)
    firstChar = 0;

  kkuint16  subStrLen = len - (kkuint16)firstChar;
  KKStr  subStr (subStrLen + 1);
  subStr.Append (((char*)&(val[firstChar])), subStrLen);

  return  subStr;
}  /* SubStrPart */




KKStr  KKStr::SubStrPart (kkint32  firstChar,
                          kkint32  lastChar
                         )  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (((kkuint16)firstChar >= len)  ||  (lastChar < firstChar))
    return  "";

  if  (firstChar < 0)
    firstChar = 0;

  if  (lastChar >= len)
    lastChar = len - 1;


  kkuint16  subStrLen = ((kkuint16)lastChar - (kkuint16)firstChar) + 1;
  KKStr  subStr (subStrLen + 2);

  kkuint16  x = (kkuint16)firstChar;
  kkuint16  y = 0;

  for  (x = (kkuint16)firstChar; x <= (kkuint16)lastChar;  x++, y++)
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
KKStr  KKStr::Tail (kkint32 tailLen)  const   // Return back the last 'len' characters.
{
  if  (tailLen <= 0)
    return "";

  kkuint16  firstChar = Max ((kkuint16)(len - (kkuint16)tailLen), (kkuint16)0);
  return  SubStrPart (firstChar);
}  /* Tail */




/**
 *@brief Remove characters from the end of the string.
 *@details Removes characters from end of string starting at position 'lastCharPos'.  If 'lastCharPos'
 *         is greater than length of string will do nothing.  If 'lastCharPos' is less than or
 *         equal to '0' will delete all characters from string.
 *@param[in] lastCharPos Will remove all characters starting at 'lastCharPos' from end of string.
 */
void  KKStr::LopOff (kkint32 lastCharPos)
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (lastCharPos >= len)
    return;

  if  (lastCharPos < -1)
    lastCharPos = -1;

  kkuint16  newLen = (kkuint16)(lastCharPos + 1);
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

  kkint32  idx = 0;

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
  
  kkint32  idx = 0;
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

    len = (kkuint16)strlen (tokenNext);
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

    len = (kkuint16)strlen (tokenNext);
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
  
  kkint32  startCharPos = 0;

  while  (startCharPos < len)
  {
    if  (val[startCharPos] != ' ')
      break;
    startCharPos++;
  }

  if  (startCharPos >= len)
    return  "";

  kkint32  lastCharPos = startCharPos;

  while  (lastCharPos < len)
  {
    if  (strchr (delStr, val[lastCharPos]) != NULL)
    {
      // We just found the first delimiter
      lastCharPos--;
      return  SubStrPart (startCharPos, lastCharPos);
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
  return  (kkuint32)atol (workStr.Str ());
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
   
  kkuint16  newLen = len - 1;

  for  (kkuint16  x = 0;  x < newLen; x++)
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

  return val[len];
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
        result.Append (val[idx]);
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
  kkint32  idx = 0;
   
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
    len = (kkuint16)(len - idx);
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



char  KKStr::operator[] (kkint16 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  ((i < 0)  ||  ((kkuint16)i >= len))
    return 0;
  else
    return val[i];
}



char  KKStr::operator[] (kkuint16 i)  const
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



char  KKStr::operator[] (kkint32 i)  const
{
  #ifdef  KKDEBUG
  ValidateLen ();
  #endif

  if  (!val)
    return 0;

  if  ((i < 0)  ||  ((kkuint16)i >= len))
    return 0;
  else
    return val[i];
}



char  KKStr::operator[] (kkuint32 i)  const
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




kkint32  KKStr::ToInt () const
{
  if  (!val)
    return 0;

  kkint32  i = atoi (val);
  return i;
}  /* ToInt*/



kkint16  KKStr::ToInt16 () const
{
  if  (!val)
    return 0;

  kkint16  i = (kkint16)atoi (val);
  return i;
}  /* ToInt32*/



kkint32  KKStr::ToInt32 () const
{
  if  (!val)
    return 0;

  kkint32  i = atoi (val);
  return i;
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



kkuint32 KKStr::ToUint () const
{
  if  (!val)  return 0;
  return  (kkuint32)atol (val);
}



KKB::ulong  KKStr::ToUlong () const
{
  if  (!val)  return 0;
  return  (ulong)atol (val);
}



KKB::kkuint32  KKStr::ToUint32 () const
{
  if  (!val)  return 0;
  return  (kkuint32)atol (val);
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
    kkint32 dashPos = field.LocateCharacter ('-');
    if  (dashPos < 0)
    {
      // This is not a range
      results->push_back (field.ToInt32 ());
    }
    else
    {
      // We are looking at a range
      kkint32  startNum = field.SubStrPart (0, dashPos - 1).ToInt32 ();
      kkint32  endNum   = field.SubStrPart (dashPos + 1).ToInt32 ();
      for  (kkint32 z = startNum;   z <= endNum;  ++z)
        results->push_back (z);
    }
    field = parser.GetNextToken (",\t \n\r");
  }
  return  results;
}  /* ToVectorint32 */





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

  kkint32  x = latitudeStr.LocateCharacter (':');
  if  (x >= 0)
  {
    degreesStr = latitudeStr.SubStrPart (0, x - 1);
    degreesStr.TrimRight ();
    minutesStr = latitudeStr.SubStrPart (x + 1);
    minutesStr.Trim ();
  }
  else
  {
    x = latitudeStr.LocateCharacter (' ');
    if  (x >= 0)
    {
      degreesStr = latitudeStr.SubStrPart (0, x - 1);
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
  if  (x >= 0)
  {
    secondsStr = minutesStr.SubStrPart (x + 1);
    minutesStr = minutesStr.SubStrPart (0, x - 1);
    secondsStr.Trim ();
  }
  else
  {
    x = minutesStr.LocateCharacter (' ');
    if  (x >= 0)
    {
      secondsStr = minutesStr.SubStrPart (x + 1);
      minutesStr = minutesStr.SubStrPart (0, x - 1);
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

  kkint32  x = longitudeStr.LocateCharacter (':');
  if  (x >= 0)
  {
    degreesStr = longitudeStr.SubStrPart (0, x - 1);
    degreesStr.TrimRight ();
    minutesStr = longitudeStr.SubStrPart (x + 1);
    minutesStr.Trim ();
  }
  else
  {
    x = longitudeStr.LocateCharacter (' ');
    if  (x >= 0)
    {
      degreesStr = longitudeStr.SubStrPart (0, x - 1);
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
  if  (x >= 0)
  {
    secondsStr = minutesStr.SubStrPart (x + 1);
    minutesStr = minutesStr.SubStrPart (0, x - 1);
    secondsStr.Trim ();
  }
  else
  {
    x = minutesStr.LocateCharacter (' ');
    if  (x >= 0)
    {
      secondsStr = minutesStr.SubStrPart (x + 1);
      minutesStr = minutesStr.SubStrPart (0, x - 1);
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



kkint32  SearchStr (const char*  src,
                  kkint32      srcLen,
                  kkint32      startPos,
                  const char*  srchStr,
                  kkint32      srchStrLen
                 )
{
  if  ((!src)  ||  (!srchStr))
    return -1;

  kkint32 numIter = (srcLen - (startPos + srchStrLen - 1));
  const char* startCh = src + startPos;

  kkint32  x;
  for  (x = 0;  x < numIter;  ++x, ++startCh)
  {
    if  (strncmp (startCh, srchStr, srchStrLen) == 0)
      return  startPos + x;
  }
  return -1;
}



kkint32  KKStr::Find (const KKStr&  str, kkint32 pos) const
{
  return  SearchStr (val, len, pos, str.Str (), str.Len ());
}



kkint32  KKStr::Find (const char*   s,   kkint32 pos, kkint32 n)  const
{
  return  SearchStr (val, len, pos, s, n);
}



kkint32  KKStr::Find (const char* s,  kkint32 pos) const
{
  return  SearchStr (val, len, pos, s, (kkint32)strlen (s));
}



kkint32  KKStr::Find (char c, kkint32 pos) const
{
  for  (kkint32 x = pos;  x < len;  x++)
  {
    if  (val[x] == c)
      return  x;
  }
  return -1;
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




KKStr  KKStr::Spaces (kkint32  c)
{
  KKStr s;
  s.RightPad (c);
  return  s;
}



void  KKStr::MemCpy (void*   dest,
                     void*   src,
                     kkuint32  size
                    )
{

  if  (dest == NULL)
  {
    cerr << endl << "KKStr::MemCpy   ***ERROR***    (dest == NULL)" << endl << endl;
  }

  else if  (src == NULL)
  {
    cerr << endl << "KKStr::MemCpy   ***ERROR***    (src == NULL)" << endl << endl;
  }

  else
  {
    memcpy (dest, src, size);
  }
}



void  KKStr::MemSet (void* dest,  kkuint8  byte, kkuint32  size)
{
  if  (dest == NULL)
  {
    cerr << "KKStr::MemSet  ***ERROR***    (dest == NULL)" << endl;
    return;
  }

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
                      RunLog&         log
                     )
{
  kkuint16  expectedLen = tag->AttributeValueInt32 ("Len");
  if  (expectedLen > 0)
    AllocateStrSpace (expectedLen);

  delete  val;
  val = NULL;
  allocatedSize = 0;
  AllocateStrSpace (expectedLen);

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      Append (*(c->Content ()));
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}  /* ReadXML */



KKStrList::KKStrList ():
  KKQueue<KKStr> (false),
  sorted (false)
{
}


KKStrList::KKStrList (bool   owner):
  KKQueue<KKStr> (owner),
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




kkint32  KKStrList::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (KKStrList);
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
                          RunLog&         log
                         )
{
  kkuint32  count = (kkuint32)tag->AttributeValueInt32 ("Count");

  DeleteContents ();

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
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
    t = s.GetNextToken (log);
  }
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



kkint32  LocateLastOccurrence (const char*   str,
                               char          ch
                              )
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
    return  (kkint32)idx;
  else
    return  -1;

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

  kkint32 maskLen = (kkint32)strlen (mask);
  
  kkint32 decimalPosition = LocateLastOccurrence (mask, '.');

  const char*  maskPtr = mask + maskLen - 1; 

  long  intPart = (long)floor (val);

  kkint32  nextDigit = 0;

  kkint32  x;

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

    kkint32  fracInt = (kkint32)(frac * power + 0.5);

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

  kkint64  intPart = val;

  kkint32  nextDigit = 0;

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



KKStr& KKStr::operator<< (std::ostream& (* mf)(std::ostream &))
{
  ostringstream  o;
  mf (o);
  Append (o.str ().c_str ());
  return  *this;
}

const  kkuint32  KKB::KKStr::StrIntMax = USHRT_MAX;








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
                            RunLog&         log
                          )
{
  kkuint32  count = (kkuint32)tag->AttributeValueInt32 ("Count");

  clear ();

  XmlTokenPtr  t = s.GetNextToken (log);

  while  (t)
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
    t = s.GetNextToken (log);
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



kkuint32  KKStrListIndexed::size ()  const
{
  return  indexIndex.size ();
}



kkint32  KKStrListIndexed::MemoryConsumedEstimated ()  const
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
                                 RunLog&         log
                                )
{
  DeleteContents ();
  bool  caseSensative = tag->AttributeValueByName ("CaseSensative")->ToBool ();
  comparator.caseSensitive = caseSensative;

  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr c = dynamic_cast<XmlContentPtr> (t);
      KKStrParser p (*c->Content ());

      while  (p.MoreTokens ())
      {
        KKStr  s = p.GetNextToken (",\t\n\r");
        Add (new KKStr (s));
      }
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}  /* ReadXML */



KKStr  KKStrListIndexed::ToTabDelString ()  const
{
  KKStr  s (indexIndex.size () * 20);
  IndexIndex::const_iterator  idx;
  for  (idx = indexIndex.begin ();  idx != indexIndex.end ();  ++idx)
  {
    if (!s.Empty ())
      s << "\t";
    s << *(idx->second);
  }
  return s;
}


void  KKStrListIndexed::WriteXML (const KKStr&            varName,
                                  ostream&                o
                                 )  const
{
  XmlTag  startTag ("KKStrListIndexed", XmlTag::TagTypes::tagStart);

  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.AddAtribute ("CaseSensative", caseSensative);
  startTag.WriteXML (o);

  kkuint32 n = size ();
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









