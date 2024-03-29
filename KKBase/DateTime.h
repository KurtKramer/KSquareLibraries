/* DateTime.h -- Classes to support Date and Time functionality.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

/*  Originally developed in ObjectPascal 1989;  converted to c++ in 1995 */

#ifndef  _DATETIME_
#define  _DATETIME_

WarningsLowered
#include <fstream>
#include <iostream>
WarningsRestored

#include "KKStr.h"

/**
 * @file  DateTime.h
 * @brief  Their are three classes in this file meant to represent  Date, Time, and DateTime called
 *         DateType, TimeType, and DateTimeType.
 */


namespace  KKB  
{
  ///<summary> Represents a calendar date consisting of three fields, Year, Month, and Day. </summary>
  ///<remarks> 
  /// This date format will support operations that will allow you to determine the number of days between two 
  /// dates; derive a new date by adding or subtracting Days, Months, or Years from an existing date, etc; and 
  /// convert date into one of several displayable formats.
  ///</remarks>
  class  DateType
  {
  public:
    DateType ();

    DateType (const DateType&  _date);

    DateType (kkint16 _year,
              uchar   _month,
              uchar   _day
             );

    ///<summary> Constructs a date from an integer that represents the total number of days since 1/1/0000 */  </summary>
    DateType (kkint32  days);

    ///<summary> Constructs a date from a displayable string; the year, month, and days fields are detertmie3d from context. </summary>
    ///<remarks>
    /// Strings can be both all numeric or combination of alpha numeric. Each part of the date, (Year, Month, Day) can be separated 
    /// by &quot; &quot;, &quot;/&quot;, tab, line-feed, or carriage-return. Examples of valid strings are &quot;1997/02/25&quot;,
    /// &quot;02/25/1997&quot;, &quot;Feb 02 1997&quot;,  &quot;02/Feb/1997&quot;, &quot;1997 February 02&quot;. The ordering of
    /// the numeric fields will be determined by context with one of three possible formats,  mm/dd/yyyy, yyyy/mm/dd, and dd/mm/yyyy.
    ///
    /// A flaw in this schema is that if the day of the month is in the range of (1 thru 12), an all numeric format is used, and the
    /// ordering is supposed to be &quot;dd/mm/yyyy&quot; this routine will assume that it is looking at &quot;mm/dd/yyyy&quot; with
    /// the result of  swapping the &quot;Month&quot; and &quot;Day&quot; fields. As a rule I try to always use the alpha numeric 
    /// version of the month.
    ///</remarks>
    DateType (KKStr  s);


    ///<summary> Adds a number of days to the date. </summary>
    ///<example>
    ///  Example:   Date d (&quot;April 14 1962&quot;);  //  Sets the instance of &quot;d&quot; to the date &quot;1962-04-14&quot;.
    ///             d.AddDays (12);            //  &quot;d&quot; now contains the date &quot;1962-04-24&quot;.
    ///             d.AddDays (-25);           //  &quot;d&quot; now contains the date &quot;1962-03-30&quot;.
    ///</example>
    void   AddDays   (kkint32 _days);

    void   AddMonths (kkint32 _months);

    void   SubtractDays (kkint32  _days);

    kkint16 Year  () const {return  year;}
    uchar   Month () const {return  month;}
    uchar   Day   () const {return  day;}

    ///<summary> Returns back the number of days since &quot;1/1/0000&quot;. </summary>
    ///<remarks>
    /// This can be used to determine the number of days between two different dates. For example you have two 
    /// instances of &quot;DateType&quot;  &quot;d1&quot; and &quot;d2&quot;. The number of days between the two dates can be determined by
    /// &quot;int deltaDays = d1.Days () - d2.Days ();&quot;.
    ///</remarks>
    kkint32   Days ()  const;

    kkuint64  Seconds () const;

    kkuint32  ToDays    ()  const;
    kkuint32  ToHours   ()  const;
    kkuint64  ToSeconds ()  const;

    KKStr     MM_DD_YY    () const;
    KKStr     MMM_DD_YYYY () const;    /**< @brief Convert into displayable string; ex: May/02/2010.  */
    KKStr     YY_MM_DD    () const;    /**< @brief Convert into displayable string; ex: 11/05/17      */
    KKStr     YYYY_MM_DD  () const;    /**< @brief Convert into displayable string; ex: 20011/05/17   */
    KKStr     YYYY_MMM_DD () const;    /**< @brief Convert into displayable string; ex: 20011/May/17  */
    KKStr     YYYYMMDD    () const;    /**< @brief Convert into displayable string; ex: 20110517      */

    DateType&  operator=  (const DateType&  right);
    bool       operator== (const DateType&  right)  const;
    bool       operator!= (const DateType&  right)  const;
    bool       operator>  (const DateType&  right)  const;
    bool       operator>= (const DateType&  right)  const;
    bool       operator<  (const DateType&  right)  const;
    bool       operator<= (const DateType&  right)  const;
    DateType   operator+  (const DateType&  right)  const;
    DateType   operator+  (kkint32  right)  const;
    DateType   operator-  (kkint32  right)  const;
    DateType   operator-  (const DateType&  right)  const;
    DateType&  operator++ ();
    DateType   operator++ (int);
    DateType&  operator-- ();
    DateType   operator-- (int);

    uchar  DaysThisMonth ()  const;  /**<  @brief returns the number of days in 'month' and if leapYear and February returns 29. */
    
    static  uchar  MonthFromStr (const KKStr&  monthStr);

    static  uchar  DaysInTheMonth (kkint32 year, 
                                   uchar month
                                  );

    static  kkint32  DaysYTDforMonth (kkint32 year,
                                    uchar month
                                   );

  private:
    void  AdjustYear ();
    void  SetFromNumOfDaysInTime (kkint32  days);

    kkint32  Compare (const DateType&  right)  const;



    static  void  AdjustDateFields (kkint32&  years,
                                    kkint32&  months,
                                    kkint32&  days
                                   );

    static const kkuint8  daysInEachMonth[];
    static const kkint16  daysYTDforMonth[];
    static const char*    monthlyShortNames[];
    static const char*    monthlyNames[];

    friend class  DateTime;

    kkint16  year;
    kkuint8  month;
    kkuint8  day;
  };  /* DateType */


  
  ///<summary> Represents a Time, consisting of three fields, Hour, Minute, and Second. </summary>
  class  TimeType 
  {
  public:
    TimeType ();

    TimeType (const TimeType&  time);

    TimeType (uchar  _hour,
              uchar  _minute,
              uchar  _second
             );

    TimeType (kkint32  seconds);

    TimeType (KKStr  s);

    uchar    Hour   () const {return hour;}
    uchar    Minute () const {return minute;}
    uchar    Second () const {return second;}

    kkuint32 Seconds   ()  const;
    double   ToHours   ()  const;
    double   ToMinutes ()  const;

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

    kkint32  Compare (const TimeType&  right)  const;

    uchar  hour;
    uchar  minute;
    uchar  second;
  };



  ///<summary>Represents Date and Time, consists of two member classes DateType and TimeType.</summary>
  class  DateTime
  {
  public:
    DateTime ();

    DateTime (const DateTime&  dateTime);

    DateTime (const DateType&  _date,
              const TimeType&  _time
             );

    DateTime (kkint16 _year,
              uchar  _month,
              uchar  _day,
              uchar  _hour,
              uchar  _minute,
              uchar  _second
             );

    DateTime (const KKStr&  s);

    void                  AddDays     (kkint32  _days);
    void                  AddHours    (kkint32  _hours);
    void                  AddMinutes  (kkint32  _mins);
    void                  AddSeconds  (kkint64  _secs);

    const KKB::DateType&  Date () const  {return  date;}

    const TimeType&       Time () const  {return  time;}

    kkuint64              Seconds () const;
    
    kkint16               Year  () const  {return  date.Year  ();}
    uchar                 Month () const  {return  date.Month ();}
    uchar                 Day   () const  {return  date.Day   ();}

    kkuint32              ToDays    ()  const;
    double                ToHours   ()  const;
    kkuint64              ToSeconds ()  const;             /**< Number seconds since "0000/01/01 00:00:00"  */

    void                  HoursAdd    (kkint32  hours);    /**< @brief  Add _hours to DateTime, will adjust date to accommodate 24 hour clock. */
    void                  MinutesAdd  (kkint32 _mins);
    void                  SecondsAdd  (long    _secs);

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
    kkint32  Compare (const DateTime&  right)  const;

    DateType  date;
    TimeType  time;
  };  /* DateTime */


  KKStr&  operator<< (KKB::KKStr&  left,  const DateType&  right);
  KKStr&  operator<< (KKB::KKStr&  left,  const TimeType&  right);
  KKStr&  operator<< (KKB::KKStr&  left,  const DateTime&  right);

  std::ostream&  operator<< (std::ostream&  os,  const DateType&  right);
  std::ostream&  operator<< (std::ostream&  os,  const TimeType&  right);
  std::ostream&  operator<< (std::ostream&  os,  const DateTime&  right); 

}  /* KKB */
#endif
