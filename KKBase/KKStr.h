/* KKStr.h -- String Management Class
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */   
#ifndef  _KKSTR_
#define  _KKSTR_
//************************************************************************************
//*                                                                                  *
//*  Developed By:  Kurt A. Kramer                                                   *
//*                                                                                  *
//*  Date:          Early 90's                                                       *
//*                                                                                  *
//************************************************************************************
//*  KKStr class and string manipulation routines.
//************************************************************************************

#include <istream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#ifdef  WIN32
#else
#if !defined(__cdecl)
#define  __cdecl
#endif
#endif

#include "KKBaseTypes.h"
#include "KKQueue.h"

#define  EnterChar   13
#define  EscapeChar  27


namespace  KKB
{
#if  !defined(_RunLog_Defined_)
  class RunLog;
#endif

#if  !defined(_XmlStream_Defined_)
  class  XmlStream;
#endif

#if  !defined(_XmlTag_Defined_)
  class  XmlTag;
  typedef  XmlTag  const *  XmlTagConstPtr;
#endif

  typedef  kkuint32  kkStrUint;

  class  KKStr;

  class  VectorKKStr:  public std::vector<KKStr>
  {
  public:
    VectorKKStr ();

    VectorKKStr (const VectorKKStr&  v);

    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );

    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;
  };


  ///<summary>
  /// A string class providing safe runtime management; strings can be used as stream and StringBuilder objects. Simple
  /// token parsing is supported as well as various translations to other formats such as 'int', 'double', and others.
  /// Allocation is done dynamically increasing as needs warrant. All methods ensure that there is no accessing outside 
  /// the bounds of the allocated string.  
  ///</summary>
  ///<todo> Should subclass the class from the stl class 'string'. </todo>
  class  KKStr 
  {
  public:
    typedef  KKStr*                 KKStrPtr;
    typedef  const KKStr*           KKStrConstPtr;
    typedef  std::optional<KKStr>   OptionKKStr;

  private:
    static  const  kkStrUint  MaxStrLen;

    kkStrUint  allocatedSize;
    kkStrUint  len;
    char*      val;

  public:

    KKStr ();

    ~KKStr ();

    KKStr (const char*  str);

    KKStr (const KKB::KKStr&  str);

    KKStr (KKB::KKStr  &&str);  /**< Move Constructor */

    KKStr (kkStrUint  size);     /**< @brief Creates a KKStr object that pre-allocates space for 'size' characters. */

    ///<summary> Initializes the string with a displayable version of <paramref name='d'/> with <paramref name='precision'/> decimal points. </summary>
    KKStr (double  d,  kkint32 precision);

    ///<summary>Constructs a KKStr instance form a stl::string instance.</summary>
    KKStr (const std::string& s);

    ///<summary> Constructs an instance from a substring of the ascii-z string <paramref name="src"/>. </summary>
    ///<param name='src'> Source string to build new instance from. </param>
    ///<param name = 'startPos'> Index of first character that we want to include in new instance.</param>
    ///<param name='endPos'> Index of last  character that we want to include in new instance.</param>
    KKStr (const char*  src,  kkuint32 startPos,  kkuint32 endPos);

	static
		KKStr  ToBase64Str (uchar const * buff, kkStrUint buffLen);

    //KKStr&   operator= (const KKStrConstPtr  src);

    KKStr&   operator= (const KKStr& src);

    KKStr&   operator= (KKStr&& src);

    KKStr&   operator= (const char* src);

    KKStr&   operator= (kkint32  right);

    KKStr&   operator= (const std::vector<KKStr>& right);

    bool     operator== (const KKStr& right)      const;

    bool     operator!= (const KKStr& right)      const;

    bool     operator== (KKStrConstPtr right)     const;

    bool     operator!= (KKStrConstPtr right)     const;

    bool     operator== (const char*  rtStr)      const;

    bool     operator!= (const char*  rtStr)      const;

    bool     operator== (const std::string right) const;

    bool     operator!= (const std::string right) const;

    bool     operator>  (const KKStr& right)      const;

    bool     operator>= (const KKStr& right)      const;

    bool     operator<  (const KKStr& right)      const;

    bool     operator<= (const KKStr& right)      const;

    void     Append (const char* buff);

    void     Append (const char* buff,
                     kkuint32    buffLen
                    );

    void     Append (char ch);

    void     Append (const KKStr&  str);

    void     Append (const std::string&  str);

    void     AppendInt32 (kkint32  i);

    void     AppendUInt32 (kkuint32  i);

    bool     CharInStr (char  ch);  /**<  Determines if 'ch' occurs anywhere in the string. */

    void     ChopFirstChar ();      /**<  Removes the first character from the string. */

    void     ChopLastChar ();       /**<  Removes the last character from the string. */

    kkint32  Compare (const KKStr&  s2)  const;  /**<  '-1' = less than 's2', '0' = Same as 's2', and '1' = Greater than 's2'.   */

    kkint32  Compare (const std::string&  s2)  const;  /**<  '-1' = less than 's2', '0' = Same as 's2', and '1' = Greater than 's2'.   */

    kkint32  CompareTo (const KKStr&  s2)  const  {return  Compare (s2);}


    ///<summary>Compares with another KKStr, ignoring case.</summary>
    ///<param name='s2'>  Other String to compare with.</param>
    ///<returns> -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.</returns>
    kkint32  CompareIgnoreCase (const KKStr& s2)  const;


    ///<summary>Compares with STL string ignoring case.  </summary>
    /// <param name='s2'> STL String  std::string that we will compare with. </param>
    ///<returns> -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater. </returns>
    kkint32  CompareIgnoreCase (const std::string&  s2)  const;


    ///<summary>Compares with ascii-z string ignoring case. </summary>
    ///<param name='s2'> Ascii-z string to compare with. </param>
    ///<returns> -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater. </returns>
    kkint32  CompareIgnoreCase (const char* s2)  const;


    ///<summary>Compares to Strings and returns -1, 0, or 1,  indicating if less than, equal, or greater. </summary>
    static  kkint32  CompareStrings (const KKStr&  s1,  const KKStr&  s2);

    ///<summary> 
    /// Concatenates the list of char* strings, stopping at first NULL. Each of these NULL terminated strings are concatenated 
    /// onto the result string; terminates when 'values[x] == NULL'.
    ///</summary>
    static  KKStr  Concat (const char**  values);

    ///<summary> 
    /// Concatenates the list of char* strings, stops at first NULL  Each of these NULL terminated strings are concatenated 
    /// onto the result string; terminates when 'values[x] == NULL'.
    ///</summary>
    static
      KKStr  Concat (const VectorKKStr&  values);

    /**
     *@brief Concatenates the list of 'std::string' strings. 
     *@details  Iterates through values concatenating each one onto a result string.
     */
    static
      KKStr  Concat (const std::vector<std::string>&  values);


    bool     Contains (const KKStr& value) const;

    bool     Contains (const char*  value) const;

    kkint32  CountInstancesOf (char  ch)  const;

    /** @brief  Trees this KKSr instance as a QuotedStr; decodes escape sequences such as '\\', '\r', '\n',  '\t', and '\0' into original characters. */
    KKStr    DecodeQuotedStr ()  const;

    bool     Empty () const {return (len <= 0);}

    
    /**  @brief  Static method that returns an Empty String.   */
    static
    const KKStr&  EmptyStr ();

    bool     EndsWith (const KKStr& value) const;
    bool     EndsWith (const char*  value) const;
    bool     EndsWith (const KKStr& value,   bool ignoreCase) const;
    bool     EndsWith (const char*  value,   bool ignoreCase) const;

    bool     EqualIgnoreCase (const KKStr&         s2)  const;
    bool     EqualIgnoreCase (const KKStrConstPtr  s2)  const;
    bool     EqualIgnoreCase (const char*          s2)  const;

    /**
     *@brief  Removes the first character from the string and returns it to the caller.
     *@details  If the String is already empty it will return 0.
     */
    char     ExtractChar ();


    /**
     *@brief  Removes the last character from the string and returns it to the caller.
     *@details  If the String is already empty it will return 0.
     */
    char     ExtractLastChar ();

    ///<summary>
    /// Extracts the next string token; if the string starts with a quote(&quot;) will extract until the terminating quote. Special
    /// control characters that are encoded with back-slashes such as carriage-return, line-feed, tab, quotes, etc will be decoded.
    /// It is the inverse of the QuotedStr or ToQuotedStr methods.
    ///</summary>
    ///<param nqame='delChars'> List of acceptable delimiter characters. </param>
    ///<param name='decodeEscapeCharacters'> Indicates if escape sequences should be decoded back to their original code.</param>
    ///<returns>Token String </returns>
    KKStr  ExtractQuotedStr (const char*  delChars,
                             bool         decodeEscapeCharacters
                            );


    ///<summary> Extract first Token from the string. </summary>
    ///<remarks>
    /// Removes first Token from string and returns it skipping over any leading delimiters. Tokens will be terminated 
    /// by end of string or the first occurrence of a delimiter character. If no more tokens left will return a Empty
    /// KKStr. Note if you do not want to skip over leading delimiter characters the use 'ExtractToken2'.
    ///</remarks>
    ///<param name='delStr'> List of delimiting characters. </param>
    ///<returns>  Extracted Token. </returns>
    ///<seealso cref='ExtractToken2'/>
    KKStr  ExtractToken  (const char* delStr = "\n\t\r ");  
    

    /**
     *@brief  Extract first Token from the string.
     *@details Removes first Token from string and returns it. Unlike 'ExtractToken' it will not
     * skip over leading delimiters.  If the first character is a delimiter it will return a
     * empty string.  Tokens will be terminated by end of string or the first occurrence of a
     * delimiter character. If no more tokens left will return a Empty KKStr.
     *@param[in] delStr List of delimiting characters.
     *@return  Extracted Token.
     *@see ExtractToken
     *@see ExtractTokenInt
     *@see ExtractTokenDouble
     *@see ExtractTokenUint
     *@see ExtractTokenBool
     */
    KKStr   ExtractToken2 (const char* delStr = "\n\t\r ");

    /**
     *@brief  Retrieves the first token in the string without removing any characters.
     *@details Similar to 'ExtractToken2' except it does not remove characters from the string.
     *@return  The first token in the string.
     */
    KKStr    GetNextToken2 (const char* delStr = "\n\t\r ") const;

    kkint32  ExtractTokenInt (const char* delStr);

    double   ExtractTokenDouble (const char* delStr);

    kkuint32 ExtractTokenUint (const char* delStr);

    kkuint64 ExtractTokenUint64 (const char* delStr);

    /**
     *@brief Extract the next token from the string assuming that it is a logical True/False value.
     *@details  This function calls 'ExtractToken2' and then returns true if the string that 
     *it extracted is equal to "Y", "Yes", "True", "T", or "1" otherwise false.
     *@param[in]  delStr  List of delimiter characters.
     *@returns 'true' or 'false'.
     */
    bool     ExtractTokenBool (const char* delStr);

    char     FirstChar () const;                             /**< Returns the first character in the string; if the string is empty returns 0. */

    void     FreeUpUnUsedSpace ();                           /**< Alloocated space s significanly larger than length of string will reallocate to free up unused space. */

    kkint32  InstancesOfChar (char ch)  const;               /**< Returns number of instances of 'ch' in the string. */

    /**
     *@brief Returns a quoted version of string where special characters Line-Feed, Carriage Return,
     * and Tab, are encoded as escape sequences.
     *@details string where 'Line Feed(\\n'), Carriage Returns('\\r'), Tabs('\\t'), and Quotes(") are coded as escape
     * sequences "\\n", "\\r", "t", or "\\".  It is then enclosed in quotes(").
     *@return Quoted String.
     */
    KKStr    QuotedStr ()  const;   

    char     LastChar ()  const;                             /**< Returns the last character in the string but if the string is empty returns 0. */

    /**
     *@brief   pads the string with enough 'ch' characters on the left side until the string is
     *         as long as 'width' characters.
     *@details if 'width' is less than the current length of the string then the string will
     *         have characters removed the beginning until its 'len' equals 'width'.
     */
    void     LeftPad (kkStrUint width,
                      uchar     ch = ' '
                     );

    kkStrUint Len ()  const  {return  len;}                       /**< @brief Returns the number of characters in the string.                  */
     
    OptionUInt32  LocateCharacter (char ch) const;                /**< @brief Returns index of 1st occurrence of 'ch' otherwise -1.             */

    OptionUInt32  LocateLastOccurrence (char ch)  const;          /**< @brief Returns index of last occurrence of 'ch' otherwise -1.            */
    
    OptionUInt32  LocateLastOccurrence (const KKStr&  s)  const;  /**< @brief Returns index of last occurrence of 's' otherwise -1.             */

    OptionUInt32  LocateNthOccurrence (char ch, kkint32 x)  const;

    OptionUInt32  LocateStr (const KKStr&  searchStr)  const;     /**< @brief Returns index of 1st occurrence of 'searchStr' otherwise -1.      */

    void     LopOff (kkStrUint  lastCharPos);                       /**< @brief Trims off all trailing characters starting at index 'lastCharPos'. */

    void     Lower ();                                            /**< @brief Make all characters in the String into lower case. */

    KKStr    MaxLen (kkStrUint  maxLen)  const;                   /**< Returns a string that will not be longer that 'maxLen'; any chracters beyond that length will be chopped off. */

    static
    kkStrUint  MaxLenSupported ();                                /**< Returns the maximum String Length that this string can support. */

    kkMemSize  MemoryConsumedEstimated () const;

    /** @brief  Will break up the contents of the string into tokens where one of the characters in 'delStr' separates each token. */
    VectorKKStr  Parse (const char* delStr = "\n\r\t, ")  const;

    /**
     *@brief  Pads string on the right side with specified character so that the string will be of specified length.
     *@param[in] width  Width that string will need to be; if less than current length then the string will be truncated to 'len'.
     *@param[in] ch   Character to pad with;  if not specified will default to space (' ').
     */
    void  RightPad (kkStrUint  width,
                    char       ch = ' '
                   );


    /**@brief Returns a string of spaces 'c' characters long.
     *@param[in] c Number of space characters to fill the string with.
     */
    static 
      KKStr  Spaces (kkStrUint  c);

    /**
     *@brief  Breaks up the contents of the string into tokens where the characters in 'delStr' acts as separates each token.
     *@param[in] delStr  List of characters that where any one of them can be a delimiter.
     */
    VectorKKStr  Split (const char* delStr = "\n\r\t, ")  const;

    /** @brief  Splits the string up into tokens using 'del' as the separator returning them in a vector. */
    VectorKKStr  Split (char del)  const;

    bool  StartsWith (const KKStr&  value)  const;
    bool  StartsWith (const char*   value)  const;
    bool  StartsWith (const KKStr&  value,  bool ignoreCase)  const;
    bool  StartsWith (const char*   value,  bool ignoreCase)  const;

    const char*  Str ()  const {return val;}      /**< @brief Returns a pointer to a ascii string. */

    static
      void  MemCpy (void* dest,  void*  src,  kkStrUint  size);

    static
      void  MemSet (void* dest,  kkuint8  byte, kkStrUint  size);


    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );

    static
      const char*  Str (const char*  s);

    
    static
      void  StrCapitalize (char*  str);


    static
      const char*  StrChr (const char*  str, int ch);


    static
      kkint32  StrCompareIgnoreCase (const char* s1, 
                                   const char* s2
                                  );

    static
      void  StrDelete  (char**  str);


    static
      bool  StrEqual (const char* s1,
                      const char* s2
                     );

    static
      bool  StrEqualN (const char* s1,
                       const char* s2,
                       kkuint32    len
                      );

    static
      bool  StrEqualNoCase (const char* s1,
                            const char* s2
                           );


    static
      bool  StrEqualNoCaseN (const char* s1,
                             const char* s2,
                             kkuint32    len
                            );


    static
      bool  StrInStr (const char*  target,
                      const char*  searchStr
                     );


    /**
     *@brief  Replaces the contents of *dest with *src.
     *@details  First deletes current *dest then allocates new a new  string to **dest so no memory is lost.
    */
    void  StrReplace (char**      dest, 
                      const char* src
                     );


    //WCHAR*       StrWide ()  const;    /**<  Returns a Wide Character version of the string.  The caller will be responsible for deleting this string. */
    wchar_t*     StrWide ()  const;      /**<  Returns a Wide Character version of the string.  The caller will be responsible for deleting this string. */

    bool         StrInStr (const KKStr&  searchField)  const;


    /**
     *@brief returns a SubString consisting of all characters starting at index [firstCharIdx] until the end of the string.
     *@details  If the index [firstCharIdx] is past the end of the string a empty string will be returned. 
     *@param[in]  firstChar  First character in string to include in the sub-string.
     *@return  Sub-string.
     */
    KKStr     SubStrPart (kkStrUint     firstCharIdx)  const;

    KKStr     SubStrPart (kkint32       firstCharIdx)  const;

    KKStr     SubStrPart (OptionUInt32  firstCharIdx)  const;


    /**
     *@brief returns a SubString consisting of all characters starting at index 'firstChar' and ending at 'lastIndex'
     *@details  If the index 'firstChar' is past the end of the string a empty string will be returned. If 'lastIndex 
     *is past the end of the string then will only include characters until the end of the string.\n
     *The length of the substring will be (lastChar - firstChar) + 1.
     *@param[in]  firstChar  First character in string to include in the sub-string.
     *@param[in]  lastChar   Last character in include in the string.
     *@return  Sub-string.
     */
    KKStr     SubStrPart (kkStrUint  firstCharIdx,
                          kkStrUint  lastCharIdx
                         )  const;

    KKStr     SubStrPart (kkStrUint     firstCharIdx,
                          OptionUInt32  lastCharIdx
                         )  const;

    KKStr     SubStrSeg (kkStrUint  firstCharIdx,
                         kkStrUint  segmentLen
                        )  const;

    KKStr     SubStrSeg (kkStrUint     firstCharIdx,
                         OptionUInt32  segmentLen
                        )  const;

    KKStr     Tail (kkStrUint tailLen)  const;      /**< Return back the last 'tailLen' characters. */


    bool      ToBool       () const;   /**< @brief Returns the bool equivalent of the string,  ex 'Yes' = true, 'No' = false, 'True' = true, etc.  */
    double    ToDouble     () const;
    float     ToFloat      () const;
    kkint32   ToInt        () const;
    kkint16   ToInt16      () const;
    kkint32   ToInt32      () const;
    kkint64   ToInt64      () const;
    double    ToLatitude   () const;   /**< @brief Processes string as if a standard latitude; ex: "15:32.2S" = -15.53833. */
    double    ToLongitude  () const;   /**< @brief Processes string as if a standard longitude; ex: "95:32.2E" = 95.53833. */
    long      ToLong       () const;
    float     ToPercentage () const;
    KKStr     ToQuotedStr  () const  {return QuotedStr ();}
    uint      ToUint       () const;
    ulong     ToUlong      () const;
    kkuint16  ToUint16     () const;
    kkuint32  ToUint32     () const;
    kkuint64  ToUint64     () const;

    VectorInt32*  ToVectorInt32 ()  const;

    wchar_t*  ToWchar_t    () const;

    KKStr&    Trim (const char* whiteSpaceChars = "\n\r\t ");

    void      TrimLeft (const char* whiteSpaceChars = "\n\r\t ");

    KKStr&    TrimRight (const char* whiteSpaceChars = "\n\r\t ");

    void      TrimRightChar ();

    KKStr     ToLower ()  const;
   
    KKStrPtr  ToKKStrPtr () const;    /**< Instatiates a new instance of KKStr with Allocated optimized for current string length.  */

    KKStr     ToUpper ()  const;

    KKStr     ToXmlStr ()  const;

    void      Upper ();

    bool      ValidInt (kkint32&  value); /**< returns true if KKStr is formated as a valid integer otherwise false.
                                           *@param[out] value of string as interpreted as a integer.
                                           */

    bool      ValidMoney (float&  value)  const;


    bool      ValidNum (double&  value)  const;  /**< Returns true if String is a valid number,  ex 1.0 or -3.123, etc */


    void      WriteXML (const KKStr&   varName,
                        std::ostream&  o
                       )  const;
    

    //std::string  methods.
    //  These methods are provided for people who are familiar with the stl version of string.
    const char*  c_str ()  {return Str ();}

    const char*  data  ()  {return Str ();}

    /**@todo  Want to implement all the methods that the std::string  class implements  
     *this way people who are familiar with the std::string class will find using
     *this class easier.
     */

    ///<summary> Will return the position where the 1st instance of 'str' after 'pos' occurs or -1 if not found. </summary>
    ///<param name='str'> String that you are searching for. </param>
    ///<param name='pos'> The starting position to start the search from. </param>
    ///<returns> The index where 'str' first occurs at or after 'pos' otherwise -1 if not found. </returns>
    OptionUInt32  Find (const KKStr&  str, kkStrUint pos = 0)     const;

    OptionUInt32  Find (const char*   s,   kkStrUint pos, kkStrUint n)  const;
    OptionUInt32  Find (const char*   s,   kkStrUint pos = 0 )    const;
    OptionUInt32  Find (char          c,   kkStrUint pos = 0 )    const;
    

    /*
    size_t  Find_First_Of (const string& str, size_t pos = 0 ) const;
    size_t  Find_First_Of (const char* s, size_t pos, size_t n ) const;
    size_t  Find_First_Of (const char* s, size_t pos = 0 ) const;
    size_t  Find_First_Of (char c, size_t pos = 0 ) const;
    
    size_t  find_last_of (const string& str, size_t pos = npos ) const;
    size_t  find_last_of (const char* s, size_t pos, size_t n ) const;
    size_t  find_last_of (const char* s, size_t pos = npos ) const;
    size_t  find_last_of (char c, size_t pos = npos ) const;


    size_t  find_first_not_of (const string& str, size_t pos = 0 ) const;
    size_t  find_first_not_of (const char* s, size_t pos, size_t n ) const;
    size_t  find_first_not_of (const char* s, size_t pos = 0 ) const;
    size_t  find_first_not_of (char c, size_t pos = 0 ) const;


    size_t  find_last_not_of (const string& str, size_t pos = npos ) const;
    size_t  find_last_not_of (const char* s, size_t pos, size_t n ) const;
    size_t  find_last_not_of (const char* s, size_t pos = npos ) const;
    size_t  find_last_not_of (char c, size_t pos = npos ) const;

    string substr (size_t pos = 0, size_t n = npos ) const;

    kkint32 compare (const string& str) const;
    kkint32 compare ( const char* s ) const;
    kkint32 compare ( size_t pos1, size_t n1, const string& str ) const;
    kkint32 compare ( size_t pos1, size_t n1, const char* s) const;
    kkint32 compare ( size_t pos1, size_t n1, const string& str, size_t pos2, size_t n2 ) const;
    kkint32 compare ( size_t pos1, size_t n1, const char* s, size_t n2) const;
    */
    

    ///<summary>
    /// Pads the string with spaces so that it is exactly 'width' characters long.
    /// Can pad either left, right, or center as specified by 'dir'. If KKStr Already greater than 'width' will truncate new string.
    ///</summary>
    ///<param name='width'> Width of KKStr; will pad KKStr with spaces until it is width long. </param>
    ///<param name='dir'>Direction to pad from  'L' - Pad on the left side,  'R' - Pad on the right side, and 
    /// 'C' - Pad on left and Right so that text is centered.</param>
    KKStr  Wide (kkStrUint width,  char  dir = 'R')  const; 
                                      

    template<typename T=int>
    char operator[] (T  i) const
    {
#ifdef  KKDEBUG
      ValidateLen ();
#endif
      if ((!val) || (i < 0) || ((kkStrUint)i >= len))  
        return 0;
      else  
        return val[i];
    }

    KKStr  operator+ (const char*   right) const;
    KKStr  operator+ (const KKStr&  right) const;
    KKStr  operator+ (kkint16       right) const;    
    KKStr  operator+ (kkuint16      right) const;
    KKStr  operator+ (kkint32       right) const;
    KKStr  operator+ (kkuint32      right) const;
    KKStr  operator+ (kkint64       right) const;
    KKStr  operator+ (kkuint64      right) const;
    KKStr  operator+ (float         right) const;
    KKStr  operator+ (double        right) const;

    KKStr&  operator<< (const char*   right);
    KKStr&  operator<< (const KKStr&  right);
    KKStr&  operator<< (KKStr&&       right);
    KKStr&  operator<< (char          right);
    KKStr&  operator<< (kkint16       right);
    KKStr&  operator<< (kkuint16      right);
    KKStr&  operator<< (kkint32       right);
    KKStr&  operator<< (kkuint32      right);
    KKStr&  operator<< (kkint64       right);
    KKStr&  operator<< (kkuint64      right);
    KKStr&  operator<< (float         right);
    KKStr&  operator<< (double        right);
    KKStr&  operator<< (std::istream& right);

    KKStr&  operator+= (const char*   right)  {return  *this << right;}
    KKStr&  operator+= (const KKStr&  right)  {return  *this << right;}
    KKStr&  operator+= (kkint16   right)      {return  *this << right;}
    KKStr&  operator+= (kkuint16  right)      {return  *this << right;}
    KKStr&  operator+= (kkint32   right)      {return  *this << right;}
    KKStr&  operator+= (kkuint32  right)      {return  *this << right;}
    KKStr&  operator+= (kkint64   right)      {return  *this << right;}
    KKStr&  operator+= (kkuint64  right)      {return  *this << right;}
    KKStr&  operator+= (float     right)      {return  *this << right;}
    KKStr&  operator+= (double    right)      {return  *this << right;}

    //friend  KKB::KKStr& endl (KKStr& _s);
    KKStr&  operator<< (std::ostream& (* mf)(std::ostream &));

  private:
    void  AllocateStrSpace (kkStrUint  size);
    
    void  GrowAllocatedStrSpace (kkStrUint  newAllocatedSize);

    void  ValidateLen ()  const;


  };   /* KKStr */

  typedef  KKStr::KKStrPtr         KKStrPtr;
  typedef  KKStr::KKStrConstPtr    KKStrConstPtr;
  typedef  std::pair<KKStr,KKStr>  KKStrPair;
  typedef  KKStr::OptionKKStr      OptionKKStr;

#define  _KKStr_Defined_

  KKStr  operator+ (const char   left,  const KKStr&  right);
  KKStr  operator+ (const char*  left,  const KKStr&  right);
  

  #ifdef  WIN32
  std::ostream& __cdecl  operator<< (      std::ostream&  os, 
                                     const KKStr&         str
                                    );

  std::istream& __cdecl  operator>> (std::istream&  is,
                                     KKStr&         str
                                    );

  #else

  std::ostream& operator<< (      std::ostream&  os, 
                            const KKStr&         str
                           );

  std::istream& operator>> (std::istream&  os, 
                            KKStr&         str
                           );
  #endif


  char*  STRCAT  (char*  dest,  kkint32   destSize, const char*  src);
  char*  STRCOPY (char*  dest,  kkint32   destSize, const char*  src);
  char*  STRCOPY (char*  dest,  kkStrUint destSize, const char*  src);

  char*  STRDUP (const char* src);

  kkint32  STRICMP  (const char*  left, const char*  right);
  kkint32  STRNICMP (const char*  left, const char*  right,  kkint32  len);

  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkint16      right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkuint16     right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkint32      right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkuint32     right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkint64      right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkuint64     right);
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  const char*  formatSpec,  kkint32      precision,  double       d );
  kkint32  SPRINTF (char*  buff,  kkint32 buffSize,  char const*  formatSpec,  double       d);



  class  KKStrList:  public KKQueue<KKStr>
  {
  public:
    typedef  KKStrList*  KKStrListPtr;

    KKStrList ();

    KKStrList (bool _owner);

    
    ///<summary> 
    /// Creates a list from a array of NULL terminated strings; the last entry in the list has to be NULL.
    ///   <code>
    ///  Example:
    ///     const char* zed[] = {"This", "is", "a", "test", NULL};
    ///     KKStrList  wordList (zed);
    ///   </code>
    ///</summary>
    KKStrList (const char*  s[]);

    
    void      AddString (KKStrPtr  str);

    std::optional<KKStrPtr>  BinarySearch (const KKStr&  searchStr);

    KKStrListPtr  DuplicateListAndContents ()  const;

    kkMemSize  MemoryConsumedEstimated ()  const;

    void   Sort (bool  _reversedOrder);

    bool   StringInList (KKStr& str);


    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );


    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;

    static
    KKStrListPtr   ParseDelimitedString (const KKStr&  str,
                                         const char*   delChars = ",\t\n\r"
                                        );

  private:
    bool  sorted;   /**<  true indicates that contents of list are sorted in order. */

    class  StringComparison;
  };  /* KKStrList */

  typedef  KKStrList::KKStrListPtr  KKStrListPtr;
  typedef  KKStrList::KKStrListPtr  StringListPtr; /**<  For comparability with previous version. */



  ///<summary> Maintains a list of ordered KKStr instances that can be recalled by either string of index. </summary>
  class  KKStrListIndexed
  {
  public:
    KKStrListIndexed ();

    KKStrListIndexed (bool _owner,
                      bool _caseSensitive
                     );

    KKStrListIndexed (const KKStrListIndexed&  list);

    ~KKStrListIndexed ();

    ///<summary>Note that if 'owner' flag s set to true will take ownership of the strings added to index.</summary>
    ///<param name='s'>  Will add 's' into the u=index unless another string with the same value already exists. </param>
    ///<returns> The index assigned to 's' or -1 if 's' is a duplicate of one already in the list. </returns>
    kkint32  Add (KKStrPtr  s);

    // /**
    //  *@param[in]  s  Will add a new instance of 's' to the index unless one with the same value already exists.
    //  *@returns The index assigned to 's' or -1 if 's' is a duplicate of one already in the list.
    //  */
    //  kkint32 Add (const KKStr&  s);

    bool  CaseSensative ()  const  {return caseSensative;}

    kkint32 Delete (KKStr&  s);

    void  DeleteContents ();

    kkint32 LookUp (const KKStr&  s)  const;

    kkint32 LookUp (KKStrPtr s)  const;

    KKStrConstPtr  LookUp (kkuint32 x)  const;

    kkMemSize MemoryConsumedEstimated ()  const;

    void  ReadXML (XmlStream&      s,
                   XmlTagConstPtr  tag,
                   VolConstBool&   cancelFlag,
                   RunLog&         log
                  );

    size_t size ()  const;

    KKStr  ToTabDelString ()  const;  /**< Strings will be separated by tab(\t) characters and in order of index. */

    void  WriteXML (const KKStr&   varName,
                    std::ostream&  o
                   )  const;

    bool  operator== (const KKStrListIndexed&  right);

    bool  operator!= (const KKStrListIndexed&  right);

  private:
    class  KKStrPtrComp
    {
    public:
      KKStrPtrComp (bool  _caseSensitive);
      KKStrPtrComp (const KKStrPtrComp&  comparator);
      bool operator() (const KKStrConstPtr& lhs, const KKStrConstPtr& rhs)  const;
      bool  caseSensitive;
    };

    typedef  std::map<KKStrPtr, kkint32, KKStrPtrComp>  StrIndex;
    typedef  std::pair<KKStrPtr,kkint32>   StrIndexPair;

    typedef  std::map<kkint32,  KKStrPtr const>  IndexIndex;
    typedef  std::pair<kkint32, KKStrPtr const>  IndexIndexPair;

    bool          caseSensative;
    KKStrPtrComp  comparator;
    IndexIndex    indexIndex;
    kkMemSize     memoryConsumedEstimated;
    kkint32       nextIndex;
    bool          owner;
    StrIndex*     strIndex;
  };  /* KKStrListIndexed */


  KKStr  StrFormatDouble (double       val,
                          const char*  mask
                         );

  KKStr  StrFormatInt (kkint32      val,
                       const char*  mask
                      );

  KKStr  StrFormatInt64 (kkint64      val,
                         const char*  mask
                        );
  
  KKStr StrFromBuff   (const char* buff, kkuint32 buffLen);
  KKStr StrFromInt16  (kkint16  i);
  KKStr StrFromUint16 (kkuint16 ui);
  KKStr StrFromInt32  (kkint32  i);
  KKStr StrFromUint32 (kkuint32 ui);
  KKStr StrFromInt64  (kkint64  i);
  KKStr StrFromUint64 (kkuint64 ui);
  KKStr StrFromFloat  (float    f);
  KKStr StrFromDouble (double   d);

}  /* namespace KKB; */

#endif
