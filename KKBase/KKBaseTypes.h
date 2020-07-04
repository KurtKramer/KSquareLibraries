/* KKBaseTypes.h -- Fundamental types used throughout the KKB  namespace.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#if  !defined(_KKBASETYPES_)
#define  _KKBASETYPES_


 /*-------------------------------------------------------------------------------------------------------------------
  File        : KKBaseTypes.h
  Description : Contains definitions that are common to all classes in KKBaseLibrery.
  Author      : Kurt Kramer
  Institution : The Kramer Family.
  Contact     : kurtkramer@gmail.com
  Date        : 01/01/1996
  -------------------------------------------------------------------------------------------------------------------*/


/**  
 *@file KKBaseTypes.h
 *@brief  This include file contains common definitions that are used by all classes in KKBaseLibrery.
 *
 *@par Description
 *This is the reference page for the KKBaseLibrery KKBaseTypes.h include file.\n
 *This reference page describes the types and definitions defined in the file \p KKBaseTypes.h.
 *It is not very detailed.
 *
 *@par Author 
 * <a href="http://figment.cse.usf.edu/~kkramer/">Kurt Kramer.</a><br>
 * ( mail : <a href="mailto:kurtkramer@gmail.com">kurtkramer@gmail.com</a> )<br>
 */

//#define WIN32

#if  defined(WIN32) || defined(WIN64)

#include <windows.h>
#include "FLOAT.H"

#else

#include <inttypes.h>
#include "float.h"

#endif

#include <limits>
#include <optional>
#include <sstream>
#include <type_traits>
#include <vector>


#define  PIE            3.14159265358979
#define  TwoPie         6.28318530717959
#define  TwoThirdsPie   2.09439510239319
#define  OneHalfPie     1.57079632679490
#define  OneThirdPie    1.04719755119660
#define  FourThirdsPie  4.18879020478639

#define  RadToDeg      57.29577951308240  /**< @brief Conversion factor from Radians to Degrees */
#define  DegToRad       0.01745329251994  /**< @brief Conversion factor from Degrees to Radians */

#undef min
#undef max


#define scFLOAT(X)  static_cast<float>(##X)
#define scDOUBLE(X)  static_cast<double>(##X)

#define scINT16(X)  static_cast<int16_t>(##X)
#define scUINT16(X)  static_cast<uint16_t>(##X)

#define scINT32(X)  static_cast<int32_t>(##X)
#define scUINT32(X)  static_cast<uint32_t>(##X)

#define scINT64(X)  static_cast<int64_t>(##X)
#define scUINT64(X)  static_cast<uint64_t>(##X)

/*----------------------------------------------------------------------------
  Definition of the namespace "KKB"  which stands for Kurt Kramer Base Library.
  ---------------------------------------------------------------------------*/

/**
 *@namespace KKB  Kurt Kramer Utilities
 *This namespace is intended to gather BaseBibrary Classes together.
 *@par Author 
 *<a href="http://figment.cse.usf.edu/~kkramer/">Kurt Kramer.</a><br>
 *( mail : <a href="mailto:kurtkramer@gmail.com">kurtkramer@gmail.com</a> )<br>
 */
namespace KKB
{
  typedef  unsigned  char   uchar;    /**< @brief  Unsigned character. */
  typedef  unsigned  int    uint;     /**< @brief  Unsigned integer.   */
  typedef  unsigned  short  ushort;   /**< @brief  Unsigned short.     */
  typedef  unsigned  long   ulong;    /**< @brief  Unsigned long       */

  #if  defined(WIN32)  ||  defined(WIN64)
    typedef  __int8            kkint8;
    typedef  unsigned __int8   kkuint8;
    typedef  __int16           kkint16;  /**<@brief  16 bit signed integer. */
    typedef  unsigned __int16  kkuint16; /**<@brief  16 bit unsigned integer. */
    //typedef  wchar_t          WCHAR;
    typedef  __int32           kkint32;
    typedef  unsigned __int32  kkuint32;
    typedef  __int64           kkint64;
    typedef  unsigned __int64  kkuint64;
  #else
  #if  defined(__GNUG__)
    typedef  __INT8_TYPE__     kkint8;
    typedef  __UINT8_TYPE__    kkuint8;
    typedef  __INT16_TYPE__    kkint16;  /**<@brief  16 bit signed integer. */
    typedef  __UINT16_TYPE__   kkuint16; /**<@brief  16 bit unsigned integer. */
    typedef  unsigned short    WCHAR;
    typedef  __INT32_TYPE__    kkint32;
    typedef  __UINT32_TYPE__   kkuint32;
    typedef  __INT64_TYPE__    kkint64;
    typedef  __UINT64_TYPE__   kkuint64;
  #else
    typedef  int8_t            kkint8;   /**<@brief  8  bit signed integer.   */
    typedef  uint8_t           kkuint8;  /**<@brief  8  bit unsigned integer. */
    typedef  int16_t           kkint16;  /**<@brief 16 bit signed integer.    */
    typedef  uint16_t          kkuint16; /**<@brief 16 bit unsigned integer.  */
    typedef  unsigned short    WCHAR;
    typedef  int32_t           kkint32;
    typedef  uint32_t          kkuint32;
    typedef  int64_t           kkint64;
    typedef  uint64_t          kkuint64;
  #endif
  #endif

  #define int16_max    (kkint16)32767
  #define int16_min    (kkint16)-32,768
  #define uint16_max   (kkuint16)65535
  #define int32_max    (kkint32)2147483647
  #define int32_min    (kkint32)-214748367
  #define uint32_max   (kkuint32)4294967295
  #define int64_max    (kkint64)9223372036854775807
  #define int64_min    (kkint64)-9223372036854775807
  #define uint64_max   (kkuint64)18446744073709551615

 
  extern  float   FloatMax;
  extern  float   FloatMin;
  extern  double  DoubleMax;
  extern  double  DoubleMin;

  float  FloatAbs (float f);


  typedef  std::vector<bool>     VectorBool;
  typedef  std::vector<char>     VectorChar;
  typedef  std::vector<uchar>    VectorUchar;
  typedef  std::vector<int>      VectorInt;
  typedef  std::vector<uint>     VectorUint;
  typedef  std::vector<kkint16>  VectorInt16;   /**< @brief  Vector of signed 16 bit integers.   */
  typedef  std::vector<kkuint16> VectorUint16;  /**< @brief  Vector of unsigned 16 bit integers. */
  typedef  std::vector<short>    VectorShort;
  typedef  std::vector<ulong>    VectorUlong;
  typedef  std::vector<kkint32>  VectorInt32;   /**< @brief  Vector of signed 32 bit integers.   */
  typedef  std::vector<kkuint32> VectorUint32;  /**< @brief  Vector of unsigned 32 bit integers. */
  typedef  std::vector<kkint64>  VectorInt64;
  typedef  std::vector<kkuint64> VectorUint64;
  typedef  std::vector<double>   VectorDouble;  /**< @brief  Vector of doubles.                 */
  typedef  std::vector<float>    VectorFloat;

  typedef  VectorChar*           VectorCharPtr;
  typedef  VectorUchar*          VectorUcharPtr;
  typedef  VectorInt*            VectorIntPtr;
  typedef  VectorInt32*          VectorInt32Ptr;
  typedef  VectorInt64*          VectorInt64Ptr;
  typedef  VectorUlong*          VectorUlongPtr;
  typedef  VectorFloat*          VectorFloatPtr;
  typedef  VectorDouble*         VectorDoubletPtr;
  typedef  VectorShort*          VectorShortPtr;

  typedef  char const * const  * const ConstCharStarArray;

  typedef  volatile const bool  VolConstBool;  /**< Commonly used for passing Cancel Flag in multi threaded environment.  */

  typedef kkuint64  kkMemSize;

  typedef std::optional<kkuint16> OptionUInt16;

  typedef std::optional<kkint32>  OptionInt32;

  typedef std::optional<kkuint32> OptionUInt32;

  typedef std::vector<OptionUInt32>  VectorOptionUInt32;



  template<typename T>
  T  Max (T x, T y) { return x > y ? x : y; }
    
  template<typename T, typename... Args>
  T  Max (T x, T y, Args... args) { T zed = Max (x, y);  return Max (zed, args...); }



  template<typename T>
  T  Min (T x, T y) { return x < y ? x : y; }
    
  template<typename T, typename... Args>
  T  Min (T x, T y, Args... args) { T zed = Min (x, y);  return Min (zed, args...); }



  template<class T> 
  std::optional<T> Max (const std::optional<T>& a, const std::optional<T>& b)
  {
    if  (!a)  return b; else if  (!b) return a; else return (a.value () > b.value ()) ? a : b;
  }



  template <class T>  T  Swap (T& x, T& y)  { T z = x;  x = y;  y = z; }
  


  /** @brief  A implementations of the Unix version of rand48 returning a 32 bit integer. */
  kkint32  LRand48 ();

  /** @brief Seeds the Lrand48 functions with the parameters passed to it. */
  void  SRand48 (kkint64 _seed);

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
    WordFormat32Bits (kkuint32  d);
    WordFormat32Bits (kkint32 d);
    WordFormat32Bits (uchar b0, uchar b1,  uchar b2,  uchar b3);

    struct
    {
      uchar   byte0;
      uchar   byte1;
      uchar   byte2;
      uchar   byte3;
    }  fourBytes;

    kkuint32  unsigned32BitInt;
    kkuint32  signed32BitInt;
  };
  #pragma pack(pop)



  template<typename T>
  T*  kkReallocateArray (T*       oldA,
                         kkint32  oldSize,
                         kkint32  newSize
                        )
  {
    T*  newA = new T[newSize];
    kkint32 x = 0;
    while (x < Min(oldSize, newSize))  {
      newA[x] = oldA[x];
      ++x;
    }
    while  (x < Max(oldSize, newSize))  {
      newA[x] = static_cast<T>(0);
      ++x;
    }
    delete[]  oldA;
    oldA = NULL;
    return newA;
  }  /* namespace KKB */
}



template<typename T>
unsigned char  touchar_t (T x)
{
  if (std::is_signed<T>::value)
  {
    if (x < 0)
    {
      std::stringstream errMsg;
      errMsg << "touchar_t  ***ERROR***  x: " << x << " is negative number!";
      std::cerr << '\n' << errMsg.str () << "\n\n";
      throw std::range_error (errMsg.str ());
    }
  }

  if (x > std::numeric_limits<unsigned char>::max ())
  {
    std::stringstream errMsg;
    errMsg << "touchar_t  ***ERROR***  x: " << x << " exceeds max range of uchar: " << std::numeric_limits<unsigned char>::max () << '!';
    std::cerr << '\n' << errMsg.str () << "\n\n";
    throw std::range_error (errMsg.str ());
  }

  return static_cast<unsigned char> (x);
}



template<typename T>
int32_t  toint32_t (T x)
{
  if  constexpr (std::is_floating_point<T>::value)
  {
  }
  else
  {
    static_assert(std::is_integral<T>::value, "Integral required.");

    if  constexpr(std::is_signed<T>::value)
    {
      if (x < std::numeric_limits<int32_t>::min ())
      {
        std::stringstream errMsg;
        errMsg << "toint32_t  ***ERROR***  x: " << x << " exceeds min range of int32_t: " << std::numeric_limits<int32_t>::min () << '!';
        std::cerr << '\n' << errMsg.str () << "\n\n";
        throw std::range_error (errMsg.str ());
      }
    }
  }
  return static_cast<int32_t> (x);
}



template<typename T>
uint32_t  touint32_t (T x)
{
  if (std::is_signed<T>::value)
  {
    if (x < 0)
    {
      std::stringstream errMsg;
      errMsg << "touint32_t  ***ERROR***  x: " << x << " is negative number!";
      std::cerr << '\n' << errMsg.str () << "\n\n";
      throw std::range_error (errMsg.str ());
    }
  }
  return static_cast<uint32_t> (x);
}



template<typename T>
size_t tosize_t(T x)
{
  if  (std::is_signed<T>::value)
  {
    if (x < 0)
    {
      std::stringstream errMsg;
      errMsg << "tosize_t  ***ERROR***  x: " << x << " is negative number!";
      std::cerr << '\n' << errMsg.str () << "\n\n";
      throw std::range_error (errMsg.str ());
    }
  }
  return static_cast<size_t> (x);
}



template<typename T>
float tofloat (T x)
{
  return static_cast<float> (x);
}



template<typename T>
double todouble (T x)
{
  return static_cast<double> (x);
}

#endif
