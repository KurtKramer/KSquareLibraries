/* DateTime.h -- Classes to support Date and Time functionality.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

/*  Originally developed in ObjectPascal 1989;  converted to c++ in 1995 */

#ifndef  _DATETIME_
#define  _DATETIME_

#include  "Str.h"

/**
 * @file  DateTime.h
 * @brief  Their are three classes in this file meant to represent Date, Time, and DateTime called
 *         DateType, TimeType, and DateTimeType.
 */




namespace  KKU  
{
  /**
   *@brief  Represents a calendar date consisting of three fields, Year, Month, and Day.
   *@details This date format will support operations that will allow you to determine the number
   * of days between two dates; derive a new date by adding or subtracting Days, Months, or Years
   * from an existing date, etc; and convert date into one of several displayable formats.
   */
  class  DateType
  {
  public:
    DateType ();

    DateType (const DateType&  _date);

    DateType (short  _year,
              uchar  _month,
              uchar  _day
             );

    /** @brief Constructs a date from an integer that represents the total number of days since 1/1/0000 */
    DateType (int32  days);

    /**
     * @brief Constructs a date from a displayable string.
     * @details Strings can be both all numeric or combination of alpha numeric. Each part of the date, (Year,
     *         Month, Day) can be separated by ' ', '/', '\t', '\n', or '\r'. Examples of valid strings are
     *         "1997/02/25", "02/25/1997", "Feb 02 1997",  "02/Feb/1997", "1997 February 02".  The ordering of
     *         the numeric fields will be determined by context with one of three possible formats, mm/dd/yyyy,
     *         yyyy/mm/dd, and dd/mm/yyyy.
     *
     *         A flaw in this schema is that if the day of the month is in the range of (1<->12), an all numeric
     *         format is used, and the ordering is supposed to be "dd/mm/yyyy" this routine will assume that it
     *         is looking at "mm/dd/yyyy" with the result of swapping the "Month" and "Day" fields.  As a rule
     *         I try to always use the alpha numeric version of the month.
     */
    DateType (KKStr  s);


    /**
     *@brief  Adds a number of days to the date.
     *@details
     *@code
     *   Example:   Date d ("April 14 1962");  //  Sets the instance of 'd' to the date '1962/04/14'.
     *              d.AddDays (12);            //  'd' now contains the date '1962/04/24'.
     *              d.AddDays (-25);           //  'd' now contains the date '1962/03/30'.
     *@endcode
     */
    void   AddDays   (int32 _days);

    void   AddMonths (int32 _months);

    void   SubtractDays (int32  _days);

    short  Year  () const {return  year;}
    uchar  Month () const {return  month;}
    uchar  Day   () const {return  day;}

    /**
     *@brief Returns back the number of days since "1/1/0000".
     *@details  This can be used to determine the number of days between two different dates.  For example
     *          you have two instances of 'DateType'  'd1' and 'd2'.  The number of days between the two
     *          dates can be determined by  "int deltaDays = d1.Days () - d2.Days ();".
     */
    int32   Days ()  const;

    uint64  Seconds () const;

    uint32  ToDays    ()  const;
    uint32  ToHours   ()  const;
    uint64  ToSeconds ()  const;

    KKStr   MM_DD_YY    () const;
    KKStr   MMM_DD_YYYY () const;    /**< @brief Convert into displayable string; ex: May/02/2010.  */
    KKStr   YY_MM_DD    () const;    /**< @brief Convert into displayable string; ex: 11/05/17      */
    KKStr   YYYY_MM_DD  () const;    /**< @brief Convert into displayable string; ex: 20011/05/17   */
    KKStr   YYYY_MMM_DD () const;    /**< @brief Convert into displayable string; ex: 20011/May/17  */
    KKStr   YYYYMMDD    () const;    /**< @brief Convert into displayable string; ex: 20110517      */

    DateType&  operator=  (const DateType&  right);
    bool       operator== (const DateType&  right)  const;
    bool       operator!= (const DateType&  right)  const;
    bool       operator>  (const DateType&  right)  const;
    bool       operator>= (const DateType&  right)  const;
    bool       operator<  (const DateType&  right)  const;
    bool       operator<= (const DateType&  right)  const;
    DateType   operator+  (const DateType&  right)  const;
    DateType   operator+  (int32  right)  const;
    DateType   operator-  (int32  right)  const;
    DateType   operator-  (const DateType&  right)  const;
    DateType&  operator++ ();

    uchar  DaysThisMonth ()  const;  /**<  @brief returns the number of days in 'month' and if leapYear and February returns 29. */
    
    static  uchar  MonthFromStr (const KKStr&  monthStr);

    static  uchar  DaysInTheMonth (int32 year, 
                                   uchar month
                                  );

    static  int32  DaysYTDforMonth (int32 year,
                                    uchar month
                                   );

  private:
    void  AdjustYear ();
    void  SetFromNumOfDaysInTime (int32  days);

    int32  Compare (const DateType&  right)  const;



    static  void  AdjustDateFields (int32&  years,
                                    int32&  months,
                                    int32&  days
                                   );

    static const uchar  daysInEachMonth[];
    static const short  daysYTDforMonth[];
    static const char*  monthlyShortNames[];
    static const char*  monthlyNames[];

    friend class  DateTime;

    short  year;
    uchar  month;
    uchar  day;
  };  /* DateType */



  /**
   *@brief  Represents a Time, consisting of three fields, Hour, Minute, and Second.
   */
  class  TimeType 
  {
  public:
    TimeType ();

    TimeType (const TimeType&  time);

    TimeType (uchar  _hour,
              uchar  _minute,
              uchar  _second
             );

    TimeType (int32  seconds);

    TimeType (KKStr  s);

    uchar  Hour   () const {return hour;}
    uchar  Minute () const {return minute;}
    uchar  Second () const {return second;}

    uint32 Seconds   ()  const;
    double ToHours   ()  const;
    double ToMinutes ()  const;

    void  Hour   (uchar _hour)   {hour   = _hour;}
    void  Minute (uchar _minute) {minute = _minute;}
    void  Second (uchar _second) {second = _second;}

    KKStr  HH_MM_SS ()  const;
    KKStr  HHMMSS   ()  const;


    TimeType&  operator=  (const TimeType&  right);
    bool       operator== (const TimeType&  right)  const;
    bool       operator!= (const TimeType&  right)  const;
    bool       operator>  (const TimeType&  right)  const;
    bool       operator>= (const TimeType&  right)  const;
    bool       operator<  (const TimeType&  right)  const;
    bool       operator<= (const TimeType&  right)  const;
    TimeType   operator+  (const TimeType&  right)  const;  /**< Add one day      */
    TimeType   operator-  (const TimeType&  right)  const;  /**< Subtract one day */

  private:
    friend class  DateTime;

    int32  Compare (const TimeType&  right)  const;

    uchar  hour;
    uchar  minute;
    uchar  second;
  };



  /**
   *@brief represents Date and Time, consists of two member classes DateType & TimeType.
   *@sa osGetFileDateTime
   *@sa osGetLocalDateTime
   */
  class  DateTime
  {
  public:
    DateTime ();

    DateTime (const DateTime&  dateTime);

    DateTime (const DateType&  _date,
              const TimeType&  _time
             );


    DateTime (short  _year,
              uchar  _month,
              uchar  _day,
              uchar  _hour,
              uchar  _minute,
              uchar  _second
             );

    DateTime (const KKStr&  s);

    void                  AddDays     (int32  _days);
    void                  AddHours    (int32  _hours);
    void                  AddMinutes  (int32  _mins);
    void                  AddSeconds  (long _secs);

    const KKU::DateType&  Date () const  {return  date;}

    const TimeType&       Time () const  {return  time;}

    uint64                Seconds () const;

    uint32                ToDays    ()  const;
    double                ToHours   ()  const;
    uint64                ToSeconds ()  const;

    short                 Year  () const  {return  date.Year  ();}
    uchar                 Month () const  {return  date.Month ();}
    uchar                 Day   () const  {return  date.Day   ();}

    void                  HoursAdd    (int32  hours);    /**< @brief  Add _hours to DateTime, will adjust date to accommodate 24 hour clock. */
 
    void                  MinutesAdd  (int32 _mins);

    void                  SecondsAdd  (long  _secs);

    KKStr                 YYYYMMDDHHMMSS ()  const;

    KKStr                 YYYY_MM_DD_HH_MM_SS () const;

    KKStr                 HH_MM_SS () const;            /**< The date part will be converted into hours. */

    DateTime&       operator=  (const DateTime&  right);
    bool            operator== (const DateTime&  right)  const;
    bool            operator!= (const DateTime&  right)  const;
    bool            operator>  (const DateTime&  right)  const;
    bool            operator>= (const DateTime&  right)  const;
    bool            operator<  (const DateTime&  right)  const;
    bool            operator<= (const DateTime&  right)  const;
    DateTime        operator+  (const DateTime&  right)  const;
    DateTime        operator-  (const DateTime&  right)  const;

  private:
    int32  Compare (const DateTime&  right)  const;

    DateType  date;
    TimeType  time;
  };  /* DateTime */


  KKStr&  operator<< (      KKU::KKStr&     left,
                      const KKU::DateType&  right
                     );


  std::ostream& operator<< (      std::ostream&   os, 
                            const KKU::DateType&  right
                           );


  KKStr&  operator<< (       KKStr&     left,
                       const TimeType&  right
                      );


  std::ostream& operator<< (      std::ostream&  os, 
                            const TimeType&      right
                           );


  KKStr&  operator<< (      KKU::KKStr&  left,
                      const DateTime&    right
                     );


  std::ostream& operator<< (      std::ostream&   os, 
                            const DateTime&       right
                           ); 
}  /* KKU */
#endif
