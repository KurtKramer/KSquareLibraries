/* BitString.h -- Bit String management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#ifndef  _BITSTRING_
#define  _BITSTRING_

#include "Atom.h"
#include "KKBaseTypes.h"
#include "KKStr.h"
#include "XmlStream.h"

namespace  KKB
{
  /**
  *@class  BitString
  *@brief Allows you to manage very long bit strings.
  *@author Kurt Kramer
  *@details  Useful when you need to deal with very large yes/no decisions.  For example performing Feature Selection 
  * on a DNA dataset where you can have 50,000+ features.  You need to keep a list of which feature combinations have 
  * been tried.  You can use a BitString to do this where a particular bit indicates a particular feature.  In the
  * feature selection case you may want to track several thousand feature combinations.  If you did this using arrays
  * you would require very large amount of memory to accomplish this.  With BitString's the memory requirement is
  * reduced to 1/8'the allowing for more efficient use of memory.
  *
  * This class will manage Bit-Strings up to UINT_MAX in length. Logical operations such as bitwise AND, OR, and NOT
  * are supported plus others.  An example of where this class is used is in KKMLL::FeatureNumList.
  */

  class  BitString: public Atom  // : public Atom
  {
  public:
    /** @brief  Instantiates a empty bit-string of length 0; needed for ReadXML  */
    BitString ();


    /**
     *@brief Construct a bit string of length _binLen with all bits set to '0'.
     *@param[in]  _bitLen Length of bit string to allocate.
     */
    BitString (kkuint32  _bitLen);
  

    /** @brief Copy constructor */
    BitString (const BitString&  b);
  

    /**
     *@brief Construct a BitString of length _bitLen with bits indicated by '_bitNums' set to '1'.
     *@param[in]  _binLen      Length of bit string.
     *@param[in]  _bitNums     List if bit positions to set to '1'.
     *@param[in]  _bitNumsLen  Size of '_bitNums' array.
     */
    BitString (kkuint32   _bitLen,
               kkuint16*  _bitNums,
               kkuint32   _bitNumsLen
              );


    ~BitString ();

    /**@brief Returns the length of the bit-string */
    kkuint32  BitLen ()  const  {return bitLen;}

    const char*  ClassName () const  {return "BitString";}

    virtual
    BitString*   Duplicate ()  const;

    /**
     *@brief  Create a bit-string from a Hex String.
     *@details Will convert the Hex String stored in the parameter 'hexStr' and create a bit string from it.\n
     * ex: "A189" will be converted into "1010000110001001"
     *@param[in]  hexStr  String containing hex characters '0' - '9', 'A' - 'F'
     *@param[out] validHexStr  returns 'TRUE' if no invalid characters.
     *@returns  BitString with the appropriate bits set to  represent the hex number in 'hexStr'.
     */
    static  
      BitString  FromHexStr (const KKStr&  hexStr,
                             bool&         validHexStr
                            );

    /**@brief  Returns number of bits set to '1'. */
    kkuint32  Count  ()  const;


    ///<summary> Returns true if bit indicated by <paramref name='bitNum'/> is set to 1. </summary>
    bool  Test (kkuint32  bitNum)  const;


    ///<summary>
    /// Get Bit positions that are set to 1; The parameter <paramref name='setBits'/> will be populated with the list of bits that are set 
    /// to 1 for bit strings that are up to 2^16-1 bits long.
    ///</summary>
    ///<code>
    /// ex: Bit String &quot;001200110011&quot; will produce a vector &lt;2, 3, 6, 7, 10, 11&gt;
    ///</code>
    ///<param name='setBits'> Will be populated with all bits that are set to '1', will be cleared first.</param>
    void  ListOfSetBits16 (VectorUint16&  setBits)  const;  

    
    ///<summary>
    /// Get Bit positions that are set to &quot;1&quot;.  The parameter <paramref name="setBits"/> will be populated with the list of 
    /// bits that are set to &quot;1&quot; for bit strings that are up to 2^32-1 bits long.
    ///</summary>
    ///<example>
    /// ex: Bit String &quot;001200110011&quot; will produce a vector &lt;2, 3, 6, 7, 10, 11&gt;
    ///</example>
    ///<param in="setBits"> Will be populated with all bits that are set to &quot;1&quot;, will be cleared first. </param>
    void  ListOfSetBits32 (VectorUint32&  setBits)  const;  

    /**
     *@brief Populates a boolean vector where each element reflects whether the corresponding bit is set.
     *@param[out] boolVector  Vector to be populated reflecting which bits are set to '1'.
     */
    //void  PopulateVectorBool (VectorBool&  boolVector)  const;

    void  ReSet ();                 /**< @brief Set all bits to '0'.                      */
    void  ReSet (kkuint32 bitNum);  /**< @brief Set the bit indicated by 'bitNum' to '0'. */
    void  Set   ();                 /**< @brief Set all bits to '1'.                      */
    void  Set   (kkuint32 bitNum);   /**< @brief Set the bit indicated by 'bitNum' to '1'. */

    virtual
    void     ReadXML (XmlStream&      s,
                      XmlTagConstPtr  tag,
                      VolConstBool&   cancelFlag,
                      RunLog&         log
                     );

    virtual
    void     WriteXML (const KKStr&   varName,
                       std::ostream&  o
                      )  const;


    /**
     *@brief Returns a Hex-String representation.
     *@details  ex:  "1110 0000 0101 1011"  would return "E09B".
     */
    KKStr  HexStr ()  const;

    BitString&  operator=  (const BitString&  right);
    BitString&  operator|= (const BitString&  right);  /**< @brief Performs a bitwise OR against the left operand.  */
    BitString&  operator+= (const BitString&  right);  /**< @brief Performs a bitwise OR against the left operand.  */
    BitString&  operator&= (const BitString&  right);  /**< @brief Performs a bitwise AND against the left operand. */
    BitString&  operator*= (const BitString&  right);  /**< @brief Performs a bitwise AND against the left operand. */
    BitString&  operator^= (const BitString&  right);  /**< @brief Performs a bitwise XOR against the left operand. */
    BitString   operator^  (const BitString&  right);  /**< @brief Performs a bitwise XOR between two operands returning a new BitString. */

    bool  operator== (const BitString&  right)  const;
    bool  operator!= (const BitString&  right)  const;
    bool  operator>= (const BitString&  right)  const;
    bool  operator>  (const BitString&  right)  const;
    bool  operator<= (const BitString&  right)  const;
    bool  operator<  (const BitString&  right)  const;

  private:
    inline
    void  CalcByteAndBitOffsets (kkuint32  bitNum,
                                 kkint32&  byteOffset,
                                 uchar&  bitOffset
                                )  
                                const;

    kkint32  Compare (const BitString&  right)  const;

    static uchar   bitMasks [8]; 
    static uchar   bitMasksRev [8]; 
    static uchar*  bitCounts;

    /**
     *@brief  Initializes static array 'bitCounts' which maintains the number of bits in the binary representation of 0 - 255.
     *@details  The purpose of 'bitCounts' is to make the computation of total number of bits in a bit string stored in 'str' as fast as possible.
     */
    static  void   BuildBitCounts ();

    static  kkint32  HexCharToInt (uchar hexChar);

    kkuint32 bitLen;     /**< Number of bits to manage;  (0 .. bitLen - )                   */
    kkuint32 byteLen;    /**< Number of bytes required to manage 'bitLen' bits.             */
    uchar*   str;        /**< Where the bits will be stored; will be 'byteLen' bytes long.  
                          *   str[0] will contain bits 0 - 7.
                          */
  };  /* BitString */

  typedef  BitString*  BitStringPtr;

  typedef  XmlElementTemplate<BitString>  XmlElementBitString;
} /* namespace  KKB */

#endif

