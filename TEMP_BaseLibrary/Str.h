/* Str.h -- String Management Class
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
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
//*  KKStr class and string manipulation routines.                                  *
//************************************************************************************

#include <map>
#include <string>

#ifdef  WIN32
#else
#define  __cdecl  
#endif

#include "BasicTypes.h"
#include "KKQueue.h"



#define  EnterChar   13
#define  EscapeChar  27



namespace KKU
{
  /**
   *@class KKStr  Str.h
   *@brief A string class providing safe runtime string management.
   *@details  This String class was originally developed back in the early 90's.  It is meant to make 
   *         string management simple and easy.  Strings can dynamically increase in size as needed.  
   *         All methods make sure that there is no accessing outside of the bounds of the allocated
   *         string.
   *@todo Should subclass the class from the stl class 'string'.
   *@author  Kurt Kramer
   */
  class  KKStr 
  {
  public:
    typedef  KKU::int16          int16;
    typedef  KKU::uint16         uint16;
    typedef  KKU::int32          int32;
    typedef  KKU::uint32         uint32;
    typedef  KKU::uint16         KKStrInt;
    typedef  KKStr*              KKStrPtr;
    typedef  KKStr const *       KKStrConstPtr;
    typedef  std::vector<KKStr>  VectorKKStr;

  private:
    static  const  uint32  StrIntMax;

    KKStrInt  allocatedSize;
    KKStrInt  len;
    char*     val;

  public:

    KKStr ();

    ~KKStr ();

    KKStr (const char*  str);

    KKStr (const KKStr&  str);

    KKStr (const KKStrConstPtr&  str);

    KKStr (int32  size);     /**< @brief Creates a KKStr object that preallocates space for 'size' characters. */

    /** Initializes the string with a displayable version of 'd' with 'precision' decimal points. */
    KKStr (double  d,
           int32   precision
          );

    /** Constructs a KKStr instance form a stl::string instance. */
    KKStr (const std::string& s);

    /**  
     *@brief  Constructs an instance from a substring of the ascii-z string 'src'.
     *@param[in]  src       Source string to build new instance from.
     *@param[in]  startPos  First character in 'src' that we want to include in new instance.
     *@param[in]  endPos    Last  character in 'src' that we want to include in new instance.
     */
    KKStr (const char*  src,
           uint32       startPos,
           uint32       endPos
          );

    KKStr&   operator= (const KKStr& src);

    KKStr&   operator= (const char* src);

    KKStr&   operator= (int32  right);

    KKStr&   operator= (const std::vector<KKStr>& right);

    bool     operator== (const KKStr& right)         const;

    bool     operator!= (const KKStr& right)         const;

    bool     operator== (const KKStrConstPtr& right) const;

    bool     operator!= (const KKStrConstPtr& right) const;

    bool     operator== (const char*  rtStr)         const;
 
    bool     operator!= (const char*  rtStr)         const;

    bool     operator== (const std::string right )   const;

    bool     operator!= (const std::string right)    const;

    bool     operator>  (const KKStr& right)         const;

    bool     operator>= (const KKStr& right)         const;

    bool     operator<  (const KKStr& right)         const;

    bool     operator<= (const KKStr& right)         const;

    void     Append (const char* buff);

    void     Append (const char* buff,
                           uint32  buffLen
                    );

    void     Append (char ch);

    void     Append (const KKStr&  str);

    void     Append (const std::string&  str);

    void     AppendInt32 (int32  i);

    void     AppendUInt32 (uint32  i);

    bool     CharInStr (char  ch);  /**<  Determines if 'ch' occurs anywhere in the string. */

    void     ChopFirstChar ();      /**<  Removes the first character from the string. */

    void     ChopLastChar ();       /**<  Removes the last character from the string. */

    int32    Compare (const KKStr&  s2)  const;  /**<  '-1' = less than 's2', '0' = Same as 's2', and '1' = Greater than 's2'.   */

    int32    Compare (const std::string&  s2)  const;  /**<  '-1' = less than 's2', '0' = Same as 's2', and '1' = Greater than 's2'.   */

    int32    CompareTo (const KKStr&  s2)  const  {return  Compare (s2);}


    /**
     *@Brief  Compares with another KKStr, ignoring case.
     *@param[in]  s2  Other String to compare with.
     *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
     */
    int32    CompareIgnoreCase (const KKStr& s2)  const;


    /**
     *@Brief  Compares with STL string ignoring case.
     *@param[in]  s2  STL String  std::string that we will compare with.
     *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
     */
    int32    CompareIgnoreCase (const std::string&  s2)  const;


    /**
     *@Brief  Compares with ascii-z string ignoring case.
     *@param[in]  s2  Ascii-z string to compare with.
     *@return  -1=less, 0=equal, 1=greater, -1, 0, or 1,  indicating if less than, equal, or greater.
     */
    int32    CompareIgnoreCase (const char* s2)  const;


    /**
     *@brief Compares to Strings and returns -1, 0, or 1,  indicating if less than, equal, or greater.
     */
    static  int32  CompareStrings (const KKStr&  s1, 
                                   const KKStr&  s2
                                  );

    /**
     *@brief Concatenates the list of char* strings,  stops at first one that is NULL   
     *details  Iterates through values concatenating each one onto a result string.  Terminates when 'values[x] == NULL'.
     */
    static 
      KKStr  Concat (const char**  values);

    /**
     *@brief Concatenates the list of char* strings,  stops at first one that is NULL
     *@details  Iterates through values Concatenating each one onto a result string.  Terminates when 'values[x] == NULL'.
     */
    static
      KKStr  Concat (const VectorKKStr&  values);

    /**
     *@brief Concatenates the list of 'std::string' strings. 
     *@details  Iterates through values concatenating each one onto a result string.
     */
    static
      KKStr  Concat (const std::vector<std::string>&  values);


    bool     Contains (const KKStr& value);

    bool     Contains (const char*  value);


    bool     Empty () const {return (len <= 0);}


    
    /**  @brief  Static method that returns an Empty String.   */
    static
    const KKStr&  EmptyStr ();

    bool     EndsWith (const KKStr& value);
    bool     EndsWith (const char*  value);
    bool     EndsWith (const KKStr& value,   bool ignoreCase);
    bool     EndsWith (const char*  value,   bool ignoreCase);

    char     EnterStr ();

    bool     EqualIgnoreCase (const KKStr&  s2)  const;
    bool     EqualIgnoreCase (const char*   s2)  const;

    /**
     *@brief  Removes the first character from the string and retuns it to the caller.
     *@details  If the String is already empty it will return 0.
     */
    char     ExtractChar ();



    /**
     *@brief  Extracts the next string token; if the string starts with a quote(") will extract until the terminating quote.
     *@code
     ***************** ExtractQuotedStr ********************
     **     Will extract and decode a quoted string.       *
     **                                                    *
     ** If first character starts with a Quote.            *
     **   a.  Extract all charaters until next Quote.      *
     **                                                    *
     **   b.  Translate                                    *
     **         "\n" -> '\n'                               *
     **         "\t" -> '\t'                               *
     **         "\r" -> '\r'                               *
     **         "\\" -> '\'                                *
     **         "\"" -> '"'                                *
     **         "\0" -> char(0)                            *
     **                                                    *
     **   c.  The start and ending Quotes will not be      *
     **       included in returned string.                 *
     **                                                    *
     **   d.  Following delimiter will be removed.         *
     **       '\n', '\t', '\r'.                            *
     **                                                    *
     ** Else                                               *
     **   a. Extract all characters until delimiter.       *
     **       '\n', '\t', '\r'.                            *
     *******************************************************
     *@endcode
     *@param[in]  delChars  List of acceptable delimiter characters.
     *@param[int32] decodeEscapeCharacters  If true escape sequences will be decoded, that is characters that are preceded by the back slash ('\').  Ex:  '\t' = Tab Character, '\r' = Carrage return, '\\' = '\'.
     *@return Token String
     */
    KKStr  ExtractQuotedStr (const char*  delChars,
                             bool         decodeEscapeCharacters
                            );


    /**
     *@brief  Extract first Token from the string.
     *@details Removes first Token from string and returns it skipping over any leading
     * delimiters. Tokens will be terminated by end of string or the first occurrence of a
     * delimiter character. If no more tokens left will return a Empty KKStr. Note if
     * you do not want to skip over leading delimiter characters the use 'ExtractToken2'.
     *@param[in]  delStr List of delimiting characters.
     *@return  Extracted Token.
     *@see ExtractToken2
     */
    KKStr   ExtractToken  (const char* delStr = "\n\t\r ");  
    

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

    int32    ExtractTokenInt (const char* delStr);

    double   ExtractTokenDouble (const char* delStr);

    uint32   ExtractTokenUint (const char* delStr);

    /**
     *@brief Extract the next token from the string assuming that it is a logical True/False value.
     *@details  This function calls 'ExtractToken2' and then returns true if the string that 
     *it extracted is equal to "Y", "Yes", "True", "T", or "1" otherwise false.
     *@param[in] List of delimiter characters.
     *@return True ot 
     */
    bool     ExtractTokenBool (const char* delStr);

    char     FirstChar () const;                             /**< @brief Returns the first character in the string; if the string is empty returns 0. */

    int32    InstancesOfChar (char ch)  const;               /**< @brief Returns the number of instances of 'ch' in the string. */

    /**
     *@brief Returns a quoted version of string where special characters line Line Feed, Carriage Return,
     * and Tab, are encoded as escape sequences.
     *@details string where 'Line Feed(\n'), Carriage Returns('\r'), Tabs('\t'), and Quotes(") are coded as escape
     * sequences "\n", "\r", "t", or '\".  It is then enclosed in quotes(").
     *@return Quoted String.
     */
    KKStr    QuotedStr ()  const;   

    char     LastChar ()  const;                             /**< @brief Returns the last character in the string but if the string is empty returns 0. */

    /**
     *@brief   pads the string with enough 'ch' characters on the left side until the string is
     *         as long as 'width' characters.
     *@details if 'width' is less than the current length of the string then the string will
     *         have characters removed the beginning until its 'len' equals 'width'.
     */
    void     LeftPad (int32 width,
                      uchar ch = ' '
                     );

    int32    Len ()  const  {return  len;}                   /**< @brief Returns the number of characters in the string.                  */

    int32    LocateCharacter (char  ch) const;               /**< @brief Returns index of 1st occurrence of 'ch' otherwise -1.             */

    int32    LocateLastOccurrence (char  ch)  const;         /**< @brief Returns index of last occurrence of 'ch' otherwise -1.            */
    
    int32    LocateLastOccurrence (const KKStr&  s)  const;  /**< @brief Returns index of last occurrence of 's' otherwise -1.             */

    int32    LocateStr (const KKStr&  searchStr)  const;     /**< @brief Returns index of 1st occurrence of 'searchStr' otherwise -1.      */

    void     LopOff (int32 lastCharPos);                     /**< @brief Trims off all characters after the 'lastCharPos' index; to make an empty string you would have to specify -1. */

    void     Lower ();                                       /**< @brief Make all characters in the String into lower case. */

    KKStr    MaxLen (int32  maxLen)  const;

    int32    MemoryConsumedEstimated () const;

    /** @brief  Will break up the contents of the string into tokens where one of the characters in 'delStr' separates each token. */
    VectorKKStr  Parse (const char* delStr = "\n\r\t, ")  const;

    void     RightPad (int32  width,
                       char ch = ' '
                      );


    /**< @brief Returns a string of spaces 'c' charaters long.
     * @param[in] c Number of space charaters to fill the string with. 
     */
    static 
      KKStr  Spaces (int32  c);


    /**
     *@brief  Breaks up the contents of the string into tokens where the characters in 'delStr' acts as separaters each token. 
     *@param[in] delStr  List of characters that where any one of them can be a delimiter.
     */
    VectorKKStr  Split (const char* delStr = "\n\r\t, ")  const;

    /** @brief  Splits the string up into tokens using 'del' as the separator returning them in a vector. */
    VectorKKStr  Split (char del)  const;


    bool  StartsWith (const KKStr& value)  const;
    bool  StartsWith (const char*  value)  const;
    bool  StartsWith (const KKStr& value,   bool ignoreCase)  const;
    bool  StartsWith (const char*  value,   bool ignoreCase)  const;

    const char*  Str ()  const {return val;}                 /**< @brief Returns a pointer to a ascii string. */

    static
      const char*  Str (const char*  s);


    static
      void  StrCapitalize (char*  str);


    static
      int32  StrCompareIgnoreCase (const char* s1, 
                                   const char* s2
                                  );

    static
      void  StrDelete  (char**  str);


    static
      bool  StrEqual (const char* s1,
                      const char* s2
                     );


    static
      bool  StrEqualNoCase (const char* s1,
                            const char* s2
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
     *@brief returns a SubString consisting of all characters starting at index 'firstChar' until the end of the string.
     *@details  If the index 'firstChar' is past the end of the string a empty string will be returned. 
     *@param[in]  firstChar  First character in string to include in the sub-string.
     *@return  Sub-string.
     */
    KKStr     SubStrPart (int32  firstChar)  const;


    /**
     *@brief returns a SubString consisting of all characters starting at index 'firstChar' and ending at 'lastIndex'
     *@details  If the index 'firstChar' is past the end of the string a empty string will be returned. If 'lastIndex 
     *is past the end of the string then will only include characters until the end of the string.\n
     *The length of the substring will be (lsastChar - firstChar) + 1.
     *@param[in]  firstChar  First character in string to include in the sub-string.
     *@param[in]  lsastChar  Last character in include in the string.
     *@return  Sub-string.
     */
    KKStr     SubStrPart (int32  firstChar,
                          int32  lastChar
                         )  const;

    KKStr     Tail (int32 tailLen)  const;      /**< Return back the last 'tailLen' characters. */


    bool      ToBool       () const;   /**< @brief Returns the bool equivalent of the string,  ex 'Yes' = true, 'No' = false, 'True' = true, etc.  */
    double    ToDouble     () const;
    float     ToFloat      () const;
    int32     ToInt        () const;
    int16     ToInt16      () const;
    int32     ToInt32      () const;
    int64     ToInt64      () const;
    double    ToLatitude   () const;   /**< @brief Processes string as if a standard latitude; ex: "15:32.2S" = -15.53833. */
    double    ToLongitude  () const;   /**< @brief Processes string as if a standard longitude; ex: "95:32.2E" = 95.53833. */
    long      ToLong       () const;
    float     ToPercentage () const;
    uint32    ToUint       () const;
    ulong     ToUlong      () const;
    uint32    ToUint32     () const;
    uint64    ToUint64     () const;

    wchar_t*  ToWchar_t    () const;

    KKStr&    TrimRight (const char* whiteSpaceChars = "\n\r\t ");

    void      TrimRightChar ();

    void      TrimLeft (const char* whiteSpaceChars = "\n\r\t ");

    KKStr     ToLower ()  const;
   
    KKStr     ToUpper ()  const;

    void      Upper ();

    bool      ValidInt (int32&  value); /**< returns true if KKStr is formated as a valid integer 
                                       * otherwise false.
                                       * value = KKStr as interpreted as a integer.
                                       */

    bool      ValidMoney (float&  value)  const;


    bool      ValidNum (double&  value)  const;  /**< Returns true if String is a valid number,  ex 1.0 or -3.123, etc */

    
    //std::string  methods.
    //  These methods are provided for people who are familiar with the stl version of string.
    const char*  c_str ()  {return Str ();}

    const char*  data  ()  {return Str ();}

    /**
     *@todo  Want to implement all the methods that the std::string  class implements.  
     *       this way people who are familiar with the std::string class will find using
     *       this class easier.
    */

    int32  Find (const KKStr&  str, int32 pos = 0)     const;
    int32  Find (const char*   s,   int32 pos, int32 n)  const;
    int32  Find (const char*   s,   int32 pos = 0 )    const;
    int32  Find (char          c,   int32 pos = 0 )    const;
    

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

    int32 compare (const string& str) const;
    int32 compare ( const char* s ) const;
    int32 compare ( size_t pos1, size_t n1, const string& str ) const;
    int32 compare ( size_t pos1, size_t n1, const char* s) const;
    int32 compare ( size_t pos1, size_t n1, const string& str, size_t pos2, size_t n2 ) const;
    int32 compare ( size_t pos1, size_t n1, const char* s, size_t n2) const;
    */


    

    /**
     *@brief  Pads the string with spaces so that it is exactly 'width' characters long.
     *@details  Can pad either left, right, or center as specified by 'dir'.  If KKStr 
     * Already greater than 'width' will truncate new string.
     *
     *@param[in]  width Width of KKStr;  Will pad KKStr with spaces until it is width long.
     *@param[in]  dir  'L' - Pad on the left side, 
     *                 'R' - Pad on the right side,
     *                 'C' - Pad on left and Right so that text is centered.
    */
    KKStr  Wide (int32 width,      
                 char  dir = 'R'
                )  const; 
                                      

    char    operator[] (int16  i) const;   /**< Returns back the character at position 'i', if  i > length of KKStr then returns back 0. */
    char    operator[] (uint16 i) const;   /**< Returns back the character at position 'i', if  i > length of KKStr then returns back 0. */
    char    operator[] (int32  i) const;   /**< Returns back the character at position 'i', if  i > length of KKStr then returns back 0. */
    char    operator[] (uint32 i) const;   /**< Returns back the character at position 'i', if  i > length of KKStr then returns back 0. */


    KKStr  operator+ (const char*   right) const;
    KKStr  operator+ (const KKStr&  right) const;
    KKStr  operator+ (int16   right) const;
    KKStr  operator+ (uint16  right) const;
    KKStr  operator+ (int32   right) const;
    KKStr  operator+ (uint32  right) const;
    KKStr  operator+ (int64   right) const;
    KKStr  operator+ (uint64  right) const;
    KKStr  operator+ (float   right) const;
    KKStr  operator+ (double  right) const;

    KKStr&  operator<< (const char*   right);
    KKStr&  operator<< (const KKStr&  right);
    KKStr&  operator<< (char  right);
    KKStr&  operator<< (int16   right);
    KKStr&  operator<< (uint16  right);
    KKStr&  operator<< (int32   right);
    KKStr&  operator<< (uint32  right);
    KKStr&  operator<< (int64   right);
    KKStr&  operator<< (uint64  right);
    KKStr&  operator<< (float   right);
    KKStr&  operator<< (double  right);

    KKStr&  operator+= (const char*   right)  {return  *this << right;}
    KKStr&  operator+= (const KKStr&  right)  {return  *this << right;}
    KKStr&  operator+= (int16   right)        {return  *this << right;}
    KKStr&  operator+= (uint16  right)        {return  *this << right;}
    KKStr&  operator+= (int32   right)        {return  *this << right;}
    KKStr&  operator+= (uint32  right)        {return  *this << right;}
    KKStr&  operator+= (int64   right)        {return  *this << right;}
    KKStr&  operator+= (uint64  right)        {return  *this << right;}
    KKStr&  operator+= (float   right)        {return  *this << right;}
    KKStr&  operator+= (double  right)        {return  *this << right;}


    //friend  KKU::KKStr& endl (KKStr& _s);
    KKStr&  operator<< (std::ostream& (* mf)(std::ostream &));


  private:
    void  AllocateStrSpace (uint32  size);
    
    void  GrowAllocatedStrSpace (uint32  newAllocatedSize);

    void  ValidateLen ()  const;

  public:
  };   /* KKStr */


  typedef  KKStr::KKStrPtr       KKStrPtr;
  typedef  KKStr::KKStrConstPtr  KKStrConstPtr;
  typedef  pair<KKStr,KKStr>     KKStrPair;


  typedef  KKStr::VectorKKStr  VectorKKStr;


  typedef  KKStr*  KKStrPtr;


  KKStr  operator+ (const char    left,
                    const KKStr&  right
                   );


  KKStr  operator+ (const char*   left,
                    const KKStr&  right
                   );

  #ifdef  WIN32
  std::ostream& __cdecl  operator<< (      std::ostream&  os, 
                                     const KKStr&         str
                                    );

  std::istream& __cdecl  operator>> (std::istream&  is,
                                     KKStr&        str
                                    );

  #else

  std::ostream& operator<< (      std::ostream&  os, 
                            const KKStr&        str
                           );

  std::istream& operator>> (std::istream&  os, 
                            KKStr&        str
                           );
  #endif



  char*  STRCAT (char*        dest,
                 int32        destSize,
                 const char*  src
                );


  char*  STRCOPY (char*        dest, 
                  int32        destSize,
                  const char*  src
                 );

  char*  STRCOPY (char*        dest,
                  uint16       destSize,
                  const char*  src
                 );

  char*  STRDUP (const char* src);


  /**@brief Comapares to string, case insensative, returning (-1 = less)  (0 = equal), and (1 = greater). */
  int32  STRICMP (const char*  left,
                  const char*  right
                 );

  int32  STRNICMP (const char*  left,
                   const char*  right,
                   int32        len
                  );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  int16        right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  uint16       right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  int32        right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  uint32       right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  int64        right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  uint64       right
                 );

  int32  SPRINTF (char*        buff,
                  int32        buffSize,
                  const char*  formatSpec,
                  int32        precision,
                  double       d
                 );

  int32  SPRINTF (char*         buff,
                  int32         buffSize,
                  char  const*  formatSpec,
                  double        d
                 );


  template<typename T>
  KKStr  VectorToDelimitedStr (std::vector<T>  v,
                               char            delimiter
                              )
  {
    KKStr  s (10 * v.size ());
    for  (int32 x = 0;  x < v.size ();  x++)
    {
      if  (x > 0)
        s << ",";
      s << v[x];
    }
    return  s;
  }  /* VectorToDelimitedStr */




  class  KKStrList:  public KKQueue<KKStr>
  {
  public:
    typedef  KKStrList*  KKStrListPtr;


    KKStrList (bool owner    = false,
               int32  initSize = 5
              );

    
    /**
     *@brief  Creates a list from a NULL terminated list of (char*) strings.
     *@details  The last entry in the list has to be NULL.
     *@code
     *  const char* zed[] = {"This", "is", "a", "test", NULL};
     *  KKStrList  wordList (zed);
     *@endcode
     */
    KKStrList (const char*  s[]);

    
    void      AddString (KKStrPtr  str);

    KKStrPtr  BinarySearch (const KKStr&  searchStr);

    KKStrListPtr  DuplicateListAndContents ()  const;

    int32  MemoryConsumedEstimated ()  const;

    void   Sort (bool  _reversedOrder);

    bool   StringInList (KKStr& str);


    static
    KKStrListPtr   ParseDelimitedString (const KKStr&  str,
                                         const char*    delChars = ",\t\n\r"
                                        );

  private:
    bool  sorted;   /**<  true indicates that contents of list are sorted in order. */

    class  StringComparison;
  };  /* KKStrList */

  typedef  KKStrList::KKStrListPtr  KKStrListPtr;
  typedef  KKStrList::KKStrListPtr  StringListPtr; /**<  For comparability with previous version. */





  /**
   *@brief Maintains a list of ordered KKStr instances that can be recalled by eithe string of index.
   */
  class  KKStrListIndexed
  {
  public:
    KKStrListIndexed (bool _owner,
                      bool _caseSensitive
                     );

    KKStrListIndexed (const KKStrListIndexed&  list);

    ~KKStrListIndexed ();

    int32  Add (KKStrPtr  s);

    int32  Add (const KKStr&  s);

    int32  Delete (KKStr&  s);

    int32  LookUp (const KKStr&  s)  const;

    int32  LookUp (KKStrPtr s)  const;

    const KKStrConstPtr  LookUp (int32 x);

    int32  MemoryConsumedEstimated ()  const;

    uint32 size ()  const;

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

    typedef  std::map<KKStrPtr, int32, KKStrPtrComp>  StrIndex;
    typedef  std::pair<KKStrPtr,int32>   StrIndexPair;

    typedef  std::map<int32,  KKStrPtr const>  IndexIndex;
    typedef  std::pair<int32, KKStrPtr const>  IndexIndexPair;

    bool          caseSensative;
    KKStrPtrComp  comparator;
    IndexIndex    indexIndex;
    int32         memoryConsumedEstimated;
    int32         nextIndex;
    bool          owner;
    StrIndex*     strIndex;
  };  /* KKStrListIndexed */



  /**
   *@class  KKStrMatrix
   *@brief  A two dimensional matrix of Strings.
   */
  class  KKStrMatrix
  {
  public:
    typedef  KKStrMatrix*  KKStrMatrixPtr;
    typedef  KKU::uint32   uint32;


    KKStrMatrix (uint32 _numCols):  
        data    (), 
        numCols (_numCols) 
    {
    }

        
    KKStrMatrix (uint32 _numCols,
                 uint32 _numRows
                ):  
        numCols (_numCols) 
    {
      while  ((uint32)data.QueueSize () < _numRows)
        AddRow ();
    }


    ~KKStrMatrix ()  
    {
    }


    uint32  NumRows ()  const  {return data.QueueSize ();}
    uint32  NumCols ()  const  {return numCols;}


    KKStrList&  operator[] (uint32 row);

    KKStrList&  operator() (uint32 row);

    KKStr&  operator() (uint32  row,
                        uint32  col
                       );


    /**
     *@brief  Adds a list of Strings to the end of the Matrix.
     *@details Will add another row to the matrix and populate its contents with the String List being passed in.
     *@param[IN] newRowData Will take ownership of this String list.
    */
    void  AddRow (KKStrListPtr  newRowData);


    /**
     *@brief  Adds a row of empty string to the end of the matix.
    */
    void  AddRow ();


  private:
    KKQueue<KKStrList>   data;
    uint32               numCols;
  };  /* KKStrMatrix */



  typedef  KKStrMatrix::KKStrMatrixPtr  KKStrMatrixPtr;



  KKStr  StrFormatDouble (double       val,
                          const char*  mask
                         );

  KKStr  StrFormatInt (int32        val,
                       const char*  mask
                      );

  KKStr  StrFormatInt64 (int64        val,
                         const char*  mask
                        );


  KKStr StrFromInt16  (int16  i);
  KKStr StrFromUint16 (uint16 ui);
  KKStr StrFromInt32  (int32  i);
  KKStr StrFromUint32 (uint32 ui);
  KKStr StrFromInt64  (int64  i);
  KKStr StrFromUint64 (uint64 ui);
}  /* namespace KKU; */

#endif
