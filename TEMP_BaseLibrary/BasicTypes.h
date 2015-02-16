/* BasicTypes.h -- Fundemental types used throught the KKU  namespace.  
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _BASICTYPES_
#define  _BASICTYPES_

 /*-------------------------------------------------------------------------------------------------------------------
  File        : BasicTypes.h
  Description : Contains definitions that are common to all classes in BaseLibrary.
  Author      : Kurt Kramer
  Institution : The Kramer Family.
  Contact     : kurtkramer@gmail.com
  Date        : 01/01/1996
  -------------------------------------------------------------------------------------------------------------------*/


/**  
 *@file BasicTypes.h
 *@brief  This include file contains common definitions that are used by all classes in BaseLibrary.
 *
 *@par Description
 *This is the reference page for the BaseLibrary BasicTypes.h include file.\n
 *This reference page describes the types and definitions defined in the file \p BasicTypes.h.
 *It is not very detailed.
 *
 *@par Author 
 * <a href="http://figment.cse.usf.edu/~kkramer/">Kurt Kramer.</a><br>
 * ( mail : <a href="mailto:kurtkramer@gmail.com">kurtkramer@gmail.com</a> )<br>
 */


#ifdef WIN32
#include "FLOAT.H"
#else
#include <stdint.h>
#include <inttypes.h>
#include "float.h"
#endif

#include  <vector>

#define  Fabs   (float)fabs
#define  Fsqrt  (float)sqrt
#define  SQRT   (float)sqrt

#define  PIE            3.14159265358979
#define  TwoPie         6.28318530717959
#define  TwoThirdsPie   2.09439510239319
#define  OneHalfPie     1.57079632679490
#define  OneThirdPie    1.04719755119660
#define  FourThirdsPie  4.18879020478639

#define  RadToDeg      57.29577951308240  /**< @brief Conversion factor from Radians to Degrees */
#define  DegToRad       0.01745329251994  /**< @brief Conversion factor from Degrees to Radians */

#define  FFLOAT_MIN  FLT_MIN
#define  FFLOAT_MAX  FLT_MAX

#define  ENCLOSE_IN_QUOTES(x) #x

/*---------------------------------------------------------------------------
  Definition of the namespace "KKU"  which stands for Kurt Kramer Utilities.
  ---------------------------------------------------------------------------*/

/**
 *@namespace KKU  Kurt Kramer Utilities
 *This namespace is intended to gather BaseLibrary Classes together.
 *@par Author 
 *<a href="http://figment.cse.usf.edu/~kkramer/">Kurt Kramer.</a><br>
 *( mail : <a href="mailto:kurtkramer@gmail.com">kurtkramer@gmail.com</a> )<br>
 */
namespace KKU
{
  typedef  unsigned  char   uchar;    /**< @brief  Unsigned character. */
  typedef  unsigned  int    uint;     /**< @brief  Unsigned integer.   */
  typedef  unsigned  short  ushort;   /**< @brief  Unsigned short.     */
  typedef  unsigned  long   ulong;    /**< @brief  Unsigned long       */

  #if  defined(WIN32) 
    typedef  __int8            int8;
    typedef  __int8            kkint8;
    typedef  unsigned __int8   uint8;
    typedef  unsigned __int8   kkuint8;
    typedef  __int16           int16;     /**<@brief  16 bit signed integer. */
    typedef  unsigned __int16  uint16;    /**<@brief  16 bit unsigned integer. */
    typedef  unsigned __int16  kkuint16;  /**<@brief  16 bit unsigned integer. */
    //typedef  wchar_t          WCHAR;
    typedef  __int32           int32;
    typedef  __int32           kkint32;
    typedef  unsigned __int32  uint32;
    typedef  unsigned __int32  kkuint32;
    typedef  __int64           int64;
    typedef  __int64           kkint64;
    typedef  unsigned __int64  uint64;
    typedef  unsigned __int64  kkuint64;
  #else
  #if  defined(__GNUG__)
    typedef  int8_t            int8;     /**<@brief  8  bit signed integer.   */
    typedef  int8_t            kkint8;   /**<@brief  8  bit signed integer.   */
    typedef  uint8_t           uint8;    /**<@brief  8  bit unsigned integer. */
    typedef  uint8_t           kkuint8;  /**<@brief  8  bit unsigned integer. */
    typedef  int16_t           int16;    /**<@brief  16 bit signed integer. */
    typedef  uint16_t          uint16;   /**<@brief  16 bit unsigned integer. */
    typedef  uint16_t          kkuint16;
    typedef  unsigned short    WCHAR;
    typedef  int32_t           int32;
    typedef  int32_t           kkint32;
    typedef  uint32_t          uint32;
    typedef  uint32_t          kkuint32;
    typedef  int64_t           int64;
    typedef  int64_t           kkint64;
    typedef  uint64_t          uint64;
    typedef  uint64_t          kkuint64;
#else
    typedef  __INT8_TYPE__     int8;
    typedef  __INT8_TYPE__     kkint8;
    typedef  __UINT8_TYPE__    uint8;
    typedef  __UINT8_TYPE__    kkuint8;
    typedef  __INT16_TYPE__    int16;    /**<@brief  16 bit signed integer. */
    typedef  __UINT16_TYPE__   uint16;   /**<@brief  16 bit unsigned integer. */
    typedef  __UINT16_TYPE__   kkuint16;
    typedef  unsigned short    WCHAR;
    typedef  __INT32_TYPE__    int32;
    typedef  __INT32_TYPE__    kkint32;
    typedef  __UINT32_TYPE__   uint32;
    typedef  __UINT32_TYPE__   kkuint32;
    typedef  __INT64_TYPE__    int64;
    typedef  __UINT64_TYPE__   uint64;
  #endif
  #endif

  #define int32_max    2147483647
  #define int32_min   -2147483648
  #define uint32_max   4294967295
  #define int64_max    9223372036854775807
  #define int64_min   -9223372036854775808
  #define uint64_max   18446744073709551615

  typedef  volatile const bool  VolConstBool;  /**< Commonly used for passing Cancel Flag in multi threaded environment.  */

  typedef  char const * const  * const ConstCharStarArray;

  typedef  std::vector<bool>   VectorBool;
  typedef  std::vector<char>   VectorChar;
  typedef  std::vector<uchar>  VectorUchar;
  typedef  std::vector<int>    VectorInt;
  typedef  std::vector<uint>   VectorUint;
  typedef  std::vector<int16>  VectorInt16;   /**< @brief  Vector of signed 16 bit integers.   */
  typedef  std::vector<uint16> VectorUint16;  /**< @brief  Vector of unsigned 16 bit integers. */
  typedef  std::vector<short>  VectorShort;
  typedef  std::vector<ulong>  VectorUlong;
  typedef  std::vector<int32>  VectorInt32;   /**< @brief  Vector of signed 16 bit integers.   */
  typedef  std::vector<uint32> VectorUint32;  /**< @brief  Vector of unsigned 16 bit integers. */
  typedef  std::vector<double> VectorDouble;  /**< @brief  Vector of doubles.                 */
  typedef  std::vector<float>  VectorFloat;

  typedef  VectorChar*    VectorCharPtr;
  typedef  VectorUchar*   VectorUcharPtr;
  typedef  VectorInt*     VectorIntPtr;
  typedef  VectorUlong*   VectorUlongPtr;
  typedef  VectorFloat*   VectorFloatPtr;
  typedef  VectorDouble*  VectorDoubletPtr;
  typedef  VectorShort*   VectorShortPtr;

  typedef  float  FFLOAT;

  /** @brief Generic Min function,  Both parameters must be of the same type.  */
  template <class T> T  Min (T  a, 
                             T  b
                            )
  {
    if  (a <= b)
      return  a;
    else
      return  b;
  }


  /** @brief Generic Min function,  Both parameters must be of the same type.  */
  template <class T> T  Min (T  a, 
                             T  b,
                             T  c
                            )
  {
    if  ((a <= b) && (a < c))
      return  a;
    else if  (b <= c)
      return  b;
    else
      return  c;
  }




  /** @brief generic Max function,  Both parameters must be of the same type. */
  template <class T> T  Max (T  a, 
                             T  b
                            )
  {
    if  (a >= b)
      return  a;
    else
      return  b;
  }




  /** @brief Generic Max function, that takes three parameters.  All three parameters must be of the same type. */
  template <class T> T  Max (T  a, 
                             T  b,
                             T  c
                            )
  {
    if  ((a >= b)  &&  (a >= c))
      return  a;

    if  (b >= c)
      return b;
    else
      return c;
  }




  /** @brief Generic Max function, that takes four parameters.  All four parameters must be of the same type. */
  template <class T> T  Max (T  a, 
                             T  b,
                             T  c,
                             T  d
                            )
  {
    if  ((a >= b)  &&  (a >= c)  &&  (a >= d))
      return a;

    if  ((b >= c)  &&  (b >= d))
      return b;

    if  (c >= d)
      return c;
    else
      return d;
  }


  template <class T>  T  Swap (T&  x,
                               T&  y
                              )
  {
    T z = x;
    x = y;
    y = z;
  }  /* Swap */



  //************************ lrand48 *****************************
  //*                                                            *
  //* The following two functions were written by Daniel J Garcia*
  //* on or near July 11, 2006. These two functions allow the use*
  //* of the Unix functions lrand48 and srand48 in a windows     *
  //* environment.                                               *
  //**************************************************************
  /** @brief Returns a long int32 random value. Can be called without first calling the SRand48 function, however, this is not recommended. */
  int32  LRand48 ();

  /** @brief Seeds the lrand48 functions with the parameters passed to it. */
  void  SRand48 (int64 _seed);

  bool  IsNaN (const float&  f);

  bool  IsNaN (const double&  d);

  bool  IsFinite (const float&  f);

  bool  IsFinite (const double&  f);


  #pragma pack(push,1)
  /**
   *@brief  Structure used to break 32 bit  word into different formats;
   */
  union  WordFormat32Bits
  {
    WordFormat32Bits ();
    WordFormat32Bits (uint32  d);
    WordFormat32Bits (int32   d);
    WordFormat32Bits (uchar b0, uchar b1,  uchar b2,  uchar b3);

    struct
    {
      uchar   byte0;
      uchar   byte1;
      uchar   byte2;
      uchar   byte3;
    }  fourBytes;

    uint32  unsigned32BitInt;
    uint32  signed32BitInt;
  };
  #pragma pack(pop)

}  /* namespace KKU */


#endif
