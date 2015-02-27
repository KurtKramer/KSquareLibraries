/* BitString.h -- Bit String management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

#ifndef  _BITSTRING_
#define  _BITSTRING_

#include  "BasicTypes.h"
#include  "Str.h"

namespace  KKU
{
  /**
  *@class  BitString
  *@brief Allows you to manage very long bit strings.
  *@author Kurt Kramer
  *@details  Useful when you need to deal with very large yes/no decisions. For example performing Feature Selection
  * on a DNA dataset where you can have 50,000+ features.  You need to keep a list of which feature combinations have 
  * been tried.  You can use a BitString to do this where a particular bit indicates a particular feature.  In the
  * feature selection case you may want to track several thousand feature combinations.  If you did this using arrays
  * you would require very large amount of memory to accomplish this. With BitString's the memory requirement is
  * reduced to 1/8'th allowing for more efficient use of memory.
  *
  * This class will manage Bit-Strings up to UINT_MAX in length. Logical operations such as bitwise AND, OR, and NOT
  * are supported plus others.  An example of where this class is used is in PicesLibrary MLL::FeatureNumList.
  */

  class  BitString
  {
  public:

    /**
     *@brief Construct a bit string of length _binLen with all bits set to '0'.
     *@param[in]  _bitLen Length of bit string to allocate.
     */
    BitString (uint32  _bitLen);
  

    /** @brief Copy constructor */
    BitString (const BitString&  b);
  

    /**
     *@brief Construct a BitString of length _bitLen with bits indicated by '_bitNums' set to '1'.
     *@param[in]  _binLen      Length of bit string.
     *@param[in]  _bitNums     List if bit positions to set to '1'.
     *@param[in]  _bitNumsLen  Size of '_bitNums' array.
     */
    BitString (uint32   _bitLen,
               uint16*  _bitNums,
               uint32   _bitNumsLen
              );


    ~BitString ();

    /**@brief Returns the length of the bit-string */
    uint32  BitLen ()  const  {return bitLen;}

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
    uint32  Count  ()  const;


    /**@brief Returns true if bit indicated by 'bitNum' is set to '1'. */
    bool  Test (uint32  bitNum)  const;


    /**
     *@brief Get Bit positions that are set to '1'.
     *@details  The parameter 'setBits' will be populated with the list of bits that are set to '1' for bit strings that are up to 2^16-1 bits long.
     *@code
     * ex: Bit String "001200110011" will produce a vector <2, 3, 6, 7, 10, 11>
     *@endcode
     *@param[out] Will be populated with all bits that are set to '1', will be cleared first.
     */
    void  ListOfSetBits16 (VectorUint16&  setBits)  const;  

    /**
     *@brief Get Bit positions that are set to '1'.
     *@details  The parameter 'setBits' will be populated with the list of bits that are set to '1' for bit strings that are up to 2^32-1 bits long.
     *@code
     * ex: Bit String "001200110011" will produce a vector <2, 3, 6, 7, 10, 11>
     *@endcode
     *@param[out] Will be populated with all bits that are set to '1', will be cleared first.
     */
    void  ListOfSetBits32 (VectorUint32&  setBits)  const;  

    /**
     *@brief Populates a boolean vector where each element reflects whether the corresponding bit is set.
     *@param[out] boolVector  Vector to be populated reflecting which bits are set to '1'.
     */
    void  PopulateVectorBool (VectorBool&  boolVector)  const;

    void  ReSet ();                 /**< @brief Set all bits to '0'.                      */
    void  ReSet (uint32 bitNum);    /**< @brief Set the bit indicated by 'bitNum' to '0'. */
    void  Set   ();                 /**< @brief Set all bits to '1'.                      */
    void  Set   (uint32 bitNum);    /**< @brief Set the bit indicated by 'bitNum' to '1'. */


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
    void  CalcByteAndBitOffsets (uint32  bitNum,
                                 int32&  byteOffset,
                                 uchar&  bitOffset
                                )  
                                const;

    int32  Compare (const BitString&  right)  const;

    static uchar   bitMasks [8]; 
    static uchar   bitMasksRev [8]; 
    static uchar*  bitCounts;

    static  void   BuildBitCounts ();
    static  int32  HexCharToInt (uchar hexChar);

    uint32   bitLen;     /**< Number of bits to manage;  (0 .. bitLen - )                   */
    uint32   byteLen;    /**< Number of bytes required to manage 'bitLen' bits.             */
    uchar*   str;        /**< Where the bits will be stored; will be 'byteLen' bytes long.  
                              str[0] will contain bits 0 - 7.
                          */
  };  /* BitString */

  typedef  BitString*  BitStringPtr;

} /* namespace  KKU */

#endif

