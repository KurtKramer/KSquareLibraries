/* DateTime.cpp -- Classes to support Date and Time functionality.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKU.h
 */

/*  Originally developed in ObjectPascal 1989;  converted to c++ in 1995 */

#include  "FirstIncludes.h"

#include  <iostream>
#include  <vector>

#include  "MemoryDebug.h"
using namespace std;


#include  "BasicTypes.h"
#include  "KKStrParser.h"
using namespace KKU;


#include  "DateTime.h"


const char*  DateType::monthlyNames[] = 
{
  "",
  "January",   // 1
  "February",  // 2
  "March",     // 3
  "April",     // 4
  "May",       // 5
  "June",      // 6
  "July",      // 7
  "August",    // 8
  "September", // 9
  "October",   // 10
  "November",  // 11
  "December"   // 12
};


const char*  DateType::monthlyShortNames[] = 
{
  "",
  "Jan",  // 1
  "Feb",  // 2
  "Mar",  // 3
  "Apr",  // 4
  "May",  // 5
  "Jun",  // 6
  "Jul",  // 7
  "Aug",  // 8
  "Sep",  // 9
  "Oct",  // 10
  "Nov",  // 11
  "Dec"   // 12
};


const uchar  DateType::daysInEachMonth[] = 
{
  0, 
  31, // Jan
  28, // Feb
  31, // Mar
  30, // Apr
  31, // MAy
  30, // Jun
  31, // Jul
  31, // Aug
  30, // Sep
  31, // Oct
  30, // Nov
  31  // dec
};


const short  DateType::daysYTDforMonth[] = 
{
  0, 
  31, // Jan
  59, // Feb
  90, // Mar
 120, // Apr
 151, // MAy
 181, // Jun
 212, // Jul
 243, // Aug
 273, // Sep
 304, // Oct
 334, // Nov
 365  // dec
};



uchar  DateType::MonthFromStr (const KKStr&  monthStr)
{
  uchar  month = 0;

  for  (month = 1;  month < 13;  month++)
  {
    if  (monthStr.CompareIgnoreCase (monthlyShortNames[month]) == 0)
      break;

    if  (monthStr.CompareIgnoreCase (monthlyNames[month]) == 0)
      break;
  }

  return  month;
}  /* MonthFromStr */




uchar  DateType::DaysInTheMonth (int32 year, 
                                 uchar month
                                )
{
  if  ((month < 1)  ||  (month > 12))
    return 0;

  if  (month == 2)
  {
    if  ((year % 4) == 0)
      return 29;
  }

  return daysInEachMonth[month];
}



int32  DateType::DaysYTDforMonth (int32 year,
                                  uchar month
                                 )
{
  if  ((month < 1)  ||  (month > 12))
    return 0;

  if  (month < 3)
    return  daysYTDforMonth[month - 1];

  if  ((year % 4) == 0)
  {
    return  daysYTDforMonth[month - 1] + 1;
  }
  else
  {
    return  daysYTDforMonth[month - 1];
  }
}  /* DaysYTDforMonth */






DateType::DateType ():
     year  (0),
     month (0),
     day   (0)
{
}



DateType::DateType (const DateType&  _date):
     year  (_date.year),
     month (_date.month),
     day   (_date.day)
{
}



DateType::DateType (short  _year,
                    uchar  _month,
                    uchar  _day
                   ):
   year  (_year),
   month (_month),
   day   (_day)

{
}


DateType::DateType (KKStr  s):
   year  (0),
   month (0),
   day   (0)
{
  KKStr  field1 = s.ExtractToken ("\n\r\t/ ");
  KKStr  field2 = s.ExtractToken ("\n\r\t/ ");
  KKStr  field3 = s.ExtractToken ("\n\r\t/ ");

  uchar  m;
  uchar  d;
  short  y;

  m = MonthFromStr (field1);
  if  ((m > 0)  && (m < 13))
  {
    // The first field is a month, assume  mmm/dd/yyyy
    d = (uchar)atoi (field2.Str ());
    y = (short)atoi (field3.Str ());

    if  ((d < 1)  ||  (d > DaysInTheMonth (y, m)))
      d = 1;

    year  = y;
    month = m;
    day   = d;

    return;
  }

  m = MonthFromStr (field2);
  if  ((m > 0)  && (m < 13))
  {
    // Second field is a month field, assuming  yyyy/mmm/dd
    int32 f3 = atoi (field3.Str ());
    int32 f1 = atoi (field1.Str ());
    
    if  (f3 > 1900)
    {
      y = (short)f3;
      d = (uchar)f1;
    }
    else
    {
      y = (short)f1;
      d = (uchar)f3;
    }

    if  (d < 1)
      d = 1;
    else if  (d > DaysInTheMonth (y, m))
      d = DaysInTheMonth (y, m);

    year  = y;
    month = m;
    day   = d;

    AdjustYear ();

    return;
  }

  int32 f1 = atoi (field1.Str ());
  int32 f2 = atoi (field2.Str ());
  int32 f3 = atoi (field3.Str ());

  if  (((f1 >= 1)  &&  (f1 <= 12))  &&
       ((f2 >= 1)  &&  (f2 <= 31))  &&
       ((f3 >= 0)  &&  (f3 <= 9999))
      )
  {
    // mm/dd/yyyy
    year  = short (f3);
    month = uchar (f1);
    day   = uchar (f2);

    AdjustYear ();
    return;
  }

  if  (((f1 >= 0)  &&  (f1 <= 9999))  &&
       ((f2 >= 1)  &&  (f2 <= 12))    &&
       ((f3 >= 1)  &&  (f3 <= 31))
      )
  {
    // yyyy/mm/dd
    year  = short (f1);
    month = uchar (f2);
    day   = uchar (f3);
    AdjustYear ();
    return;
  }

  if  (((f1 >= 0)  &&  (f1 <= 31))  &&
       ((f2 >= 1)  &&  (f2 <= 12))    &&
       ((f3 >= 1)  &&  (f3 <= 9999))
      )
  {
    // dd/mm/yyyy
    year  = short (f3);
    month = uchar (f2);
    day   = uchar (f1);
    AdjustYear ();
    return;
  }

  year = 0;
  month = 1;
  day = 1;
}




int32  DaysInYear (int32 year)
{
  if  ((year % 4) == 0)
    return 366;
  else
    return 365;
}



DateType::DateType (int32  days)
{
  SetFromNumOfDaysInTime (days);
}


uchar  DateType::DaysThisMonth ()  const
{
  return  DaysInTheMonth (year, month);
}


void  DateType::SetFromNumOfDaysInTime (int32  days)
{
  int32  numOf4Years = days / 1461;

  if  (days < 0)
    days--;

  year = (short)(numOf4Years * 4);
  days = days % 1461;

  while  (days < 0)
  {
    days += DaysInYear (year);
    year--;
  }

  if  (days == 0)
  {
    year--;
    days = DaysInYear (year);
  }

  int32  daysInYear = DaysInYear (year);
  while  (days > daysInYear)
  {
    year++;
    days = days - daysInYear;
    daysInYear = DaysInYear (year);
  }

  month = 1;
  int32  daysInMonth = DaysInTheMonth (year, month);
  while  (days > daysInMonth)
  {
    month++;
    days = days - daysInMonth;
    daysInMonth = DaysInTheMonth (year, month);
  }

  day = (uchar)days;
}  /* SetFromNumOfDaysInTime */



void  DateType::AdjustYear ()
{
  if  (year < 100)
  {
    if  (year > 50)
      year += 1900;
    else
      year += 2000;
  }
}  /* AdjustYear */




void   DateType::AddDays (int32  _days)
{
  if  (_days < 0)
  {
    SubtractDays (0 - _days);
    return;
  }

  if  (_days > 365)
  {
    int32  numDays = Days () + _days;
    SetFromNumOfDaysInTime (numDays);
    return;
  }

  int32  newDay = day + _days;
  while  (newDay > DaysThisMonth ())
  {
    newDay = newDay - DaysThisMonth ();
    month++;
    if  (month > 12)
    {
      month = 1;
      year++;
    }
  }

  day = (uchar)newDay;
}  /* AddDays */




void   DateType::SubtractDays (int32  _days)
{
  if  (_days < 0)
  {
    AddDays (0 - _days);
    return;
  }

  if  (_days > 365)
  {
    int32  numDays = Days () - _days;
    SetFromNumOfDaysInTime (numDays);
    return;
  }

  int32  newDay = day - _days;
  while  (newDay < 1)
  {
    month--;
    while  (month < 1)
    {
      month += 12;
      year--;
    }
    newDay = newDay + DaysThisMonth ();
  }

  day = (uchar)newDay;
} /* SubtractDays */



void  DateType::AddMonths (int32 _months)
{
  int32  newMonth = month + _months;
  year = year + (short)(newMonth / 12);
  newMonth = newMonth % 12;
  if  (newMonth == 0)
  {
    year--;
    newMonth += 12;
  }
}  /* AddMonths */





DateType&  DateType::operator= (const DateType&  right)
{
  year  = right.year;
  month = right.month;
  day   = right.day;

  return *this;
}





int32  DateType::Days ()  const
{
  if  ((month < 1)  ||  (month > 12))
    return 0;

  
  int32  totDays = 0;

  if  (year < 0)
  {
    int32  x = 0 - year;
    totDays = totDays - ((x * 365) + (x / 4));
  }

  else if  (year > 0)
  {  
    totDays += (year * 365) + ((year - 1) / 4) + 1;  // The "+ 1" is for year 0 which is a leap year
  }

  totDays += day + DaysYTDforMonth (year, month);

  return  totDays;
}  /* Days */




KKU::uint64   DateType::Seconds ()  const
{
  uint64  numDays = (uint64)Days ();
  uint64  numSecs = numDays * (uint64)86400;
  return  numSecs;
}


KKU::uint32  DateType::ToDays    ()  const  {return  Days ();}
KKU::uint32  DateType::ToHours   ()  const  {return  Days () * 24;}
KKU::uint64  DateType::ToSeconds ()  const  {return  (uint64)(Days ()) * (uint64)86400;}




int32  DateType::Compare (const DateType&  right)  const
{
  if  (year > right.year)
    return  1;

  else if  (year < right.year)
    return  -1;

  else if  (month > right.month)
    return  1;

  else if  (month < right.month)
    return  -1;

  else if  (day > right.day)
    return  1;

  else if  (day < right.day)
    return -1;

  else
    return 0;
}  /* Compare */


bool  DateType::operator== (const DateType&  right)  const
{
  return (Compare (right) == 0);
}


bool  DateType::operator!= (const DateType&  right)  const
{
  return (Compare (right) != 0);
}



bool  DateType::operator>  (const DateType&  right)  const
{
  return (Compare (right) > 0);
}



bool  DateType::operator>= (const DateType&  right)  const
{
  return (Compare (right) >= 0);
}



bool  DateType::operator< (const DateType&  right)  const
{
  return (Compare (right) < 0);
}




bool  DateType::operator<= (const DateType&  right)  const
{
  return (Compare (right) <= 0);
}



void  DateType::AdjustDateFields (int32&  years,
                                  int32&  months,
                                  int32&  days
                                 )
{
  years  += (months / 12);
  months =  (months % 12);

  int32  daysInTheMonth = DaysInTheMonth (years, (uchar)months);

  while  ((days < 1)  &&  (days > daysInTheMonth))
  {
    if  (days < 1)
    {
      if  (months == 1)
        months = 12;
      else
        months--;
    
      daysInTheMonth = DaysInTheMonth (years, (uchar)months);
      days = days + daysInTheMonth;
    }
    else
    {
      if  (months >= 12)
        months = 1;
      else
        months++;
      days = days - daysInTheMonth;

      daysInTheMonth = DaysInTheMonth (years, (uchar)months);
    }
  }
}  /* AdjustDateFields */



DateType  DateType::operator+  (const DateType&  right)  const
{
  return  DateType (Days () + right.Days ());
}



DateType  DateType::operator+  (int32  right)  const
{
  return  DateType (Days () + right);
}


DateType  DateType::operator-  (int32  right)  const
{
  return  DateType (Days () - right);
}



DateType  DateType::operator-  (const DateType&  right)  const
{
  return  DateType (Days () - right.Days ());
}


DateType& DateType::operator++ () 
{
  day++;
  if  (day > DaysInTheMonth (year, month))
  {
    day = 1;
    month++;
    if  (month > 12)
    {
      month = 1;
      year++;
    }
  }

  return  *this;
}


KKStr   DateType::MM_DD_YY ()  const
{
  KKStr  s (9);
  s << StrFormatInt (Month (),       "00")  << "/"
    << StrFormatInt (Day   (),       "00")  << "/"
    << StrFormatInt (Year  () % 100, "00");
  return  s;
}  /* MM_DD_YY */



KKStr   DateType::MMM_DD_YYYY ()  const
{
  KKStr  s (12);

  if  ((month < 1)  ||  (month > 12))
    s << "***";
  else
    s << monthlyShortNames[month];

  s << StrFormatInt (Day   (),  "00")  << "/"
    << StrFormatInt (Year  (),  "0000");
  return  s;
}  /* MMM_DD_YYYY */


KKStr   DateType::YY_MM_DD ()  const
{
  KKStr  s (9);
  s << StrFormatInt (year % 100, "00")  << "/"
    << StrFormatInt (month,      "00")  << "/"
    << StrFormatInt (day,        "00");
  return  s;
}  /* YY_MM_DD */


KKStr   DateType::YYYY_MM_DD ()  const
{
  KKStr  s (11);
  s << StrFormatInt (year,   "0000")  << "/"
    << StrFormatInt (month,  "00")    << "/"
    << StrFormatInt (day,    "00");
  return  s;
}  /* YYYY_MM_DD */



KKStr   DateType::YYYY_MMM_DD ()  const
{
  const char*  monthStr = "***";
  if  ((month >= 1)  &&  (month <= 12))
    monthStr = monthlyShortNames[month];

  KKStr  s (12);
  s << StrFormatInt (year,   "0000")  << "/"
    << monthStr                       << "/"
    << StrFormatInt (day,    "00");
  return  s;
}  /* YYYY_MM_DD */



KKStr   DateType::YYYYMMDD () const
{
  KKStr  s (8);
  s << StrFormatInt (year,   "0000")
    << StrFormatInt (month,  "00")
    << StrFormatInt (day,    "00");
  return  s;
}


KKStr&  KKU::operator<< (      KKStr&    left,
                         const DateType&  right
                        )
{
  left << right.YYYY_MMM_DD ();
  return  left;
}



ostream&  KKU::operator<< (      ostream&  os, 
                           const DateType& right
                          )
{
  os << right.YYYY_MMM_DD ();
  return  os;
}



TimeType::TimeType ():
   hour   (0),
   minute (0),
   second (0)
{}




TimeType::TimeType (const TimeType&  _time):
   hour   (_time.hour),
   minute (_time.minute),
   second (_time.second)
{}



TimeType::TimeType (uchar  _hour,
                    uchar  _minute,
                    uchar  _second
                   ):
   hour   (_hour),
   minute (_minute),
   second (_second)
{}



TimeType::TimeType (int32  seconds)
{
  int32 hours   = seconds / 3600;
      seconds = seconds % 3600;

  int32 minutes = seconds / 60;
      seconds = seconds % 60;

  hour   = (uchar)hours;
  minute = (uchar)minutes;
  second = (uchar)seconds;
}





TimeType::TimeType (KKStr  s):
   hour   (0),
   minute (0),
   second (0)

{
  KKStr  field1, field2, field3;

  if  (s.Len () == 6)
  {
    field1 = s.SubStrPart (0, 1);
    field2 = s.SubStrPart (2, 3);
    field3 = s.SubStrPart (4, 5);
  }
  else
  {
    field1 = s.ExtractToken ("\n\r\t: ");
    field2 = s.ExtractToken ("\n\r\t: ");
    field3 = s.ExtractToken ("\n\r\t: ");
  }

  int32  f1 = atoi (field1.Str ());
  int32  f2 = atoi (field2.Str ());
  int32  f3 = atoi (field3.Str ());
 

  if  (((f1 >= 0)  &&  (f1 <= 24))  &&
       ((f2 >= 0)  &&  (f2 <= 59))  &&
       ((f3 >= 0)  &&  (f3 <= 59))
      )
  {
    hour    = (uchar)f1;
    minute  = (uchar)f2;
    second  = (uchar)f3;
  }
}



 KKStr  TimeType::HH_MM_SS ()  const
 {
   return  StrFormatInt (hour, "00") + ":" + StrFormatInt (minute, "00") + ":" + StrFormatInt (second, "00");
 }


KKStr  TimeType::HHMMSS   ()  const
{
  return  StrFormatInt (hour, "00") + StrFormatInt (minute, "00") + StrFormatInt (second, "00");
}


int32  TimeType::Compare (const TimeType&  right)  const
{
  if  (hour > right.hour)
    return  1;

  else if  (hour < right.hour)
    return  -1;

  else if  (minute > right.minute)
    return  1;

  else if  (minute < right.minute)
    return  -1;

  else if  (second > right.second)
    return  1;

  else if  (second < right.second)
    return  -1;

  else
    return 0;
}



TimeType&  TimeType::operator= (const TimeType&  right)
{
  hour   = right.hour;
  minute = right.minute;
  second = right.second;
  return *this;
}




bool  TimeType::operator== (const TimeType&  right)  const
{
  return  (Compare (right) == 0);
}



bool  TimeType::operator!= (const TimeType&  right)  const
{
  return  (Compare (right) == 0);
}




bool  TimeType::operator>  (const TimeType&  right)  const
{
  return  (Compare (right) > 0);
}



bool  TimeType::operator>= (const TimeType&  right)  const
{
  return  (Compare (right) >= 0);
}



bool  TimeType::operator<  (const TimeType&  right)  const
{
  return  (Compare (right) < 0);
}


KKU::uint32  TimeType::Seconds ()  const
{
  return  hour * 3600 + minute * 60 + second;
}


double  TimeType::ToHours ()  const
{
  return  (double)hour +  (double)(minute * 60 + second) / 3600.0;
}


double  TimeType::ToMinutes ()  const
{
  return  (double)hour +  (double)(minute * 60 + second) / 60.0;
}





TimeType  TimeType::operator+ (const TimeType&  right)  const
{
  int32  totSeconds = Seconds () + right.Seconds ();
  return  TimeType (totSeconds);
}


TimeType  TimeType::operator- (const TimeType&  right)  const
{
  int32  totSeconds = Seconds () - right.Seconds ();
  return  TimeType (totSeconds);
}



bool  TimeType::operator<= (const TimeType&  right)  const
{
  return  (Compare (right) <= 0);
}




KKStr&  KKU::operator<< (      KKStr&    left,
                         const TimeType&  right
                        )
{
  left << StrFormatInt (right.Hour   (), "00") << ":"
       << StrFormatInt (right.Minute (), "00") << ":"
       << StrFormatInt (right.Second (), "00");

  return  left;
}



ostream&  KKU::operator<< (      ostream&  os, 
                           const TimeType& right
                          )
{
  os << StrFormatInt (right.Hour   (), "00") << ":"
     << StrFormatInt (right.Minute (), "00") << ":"
     << StrFormatInt (right.Second (), "00");

  return  os;
}





DateTime::DateTime ():  
      date(),
      time ()
{}





DateTime::DateTime (const DateTime&  dateTime):
       date (dateTime.date),
       time (dateTime.time)
{}






DateTime::DateTime (const DateType&  _date,
                    const TimeType&  _time
                   ):
       date (_date),
       time (_time)
{}






DateTime::DateTime (short  _year,
                    uchar  _month,
                    uchar  _day,
                    uchar  _hour,
                    uchar  _minute,
                    uchar  _second
                   ):
      date (_year, _month, _day),
      time (_hour, _minute, _second)
{}





DateTime::DateTime (const KKStr&  s):
      date(),
      time ()

{
  if  (s.Empty ())
    return;

  KKStrParser parser (s);
  KKStr  field1 = parser.GetNextToken ("- ");
  KKStr  field2 = parser.GetNextToken ("- ");

  date = DateType (field1);
  time = TimeType (field2);
}






int32 DateTime::Compare (const DateTime&  right)  const
{
  if  (date < right.date)
    return -1;
 
  else if  (date > right.date)
    return  1;

  return  time.Compare (right.time);
}



KKU::uint64  DateTime::Seconds () const
{
  uint64  secsInDate = (uint64)date.Seconds ();
  uint64  secsInTime = (uint64)time.Seconds ();
  return  secsInDate + secsInTime;
}


KKU::uint32  DateTime::ToDays    ()  const  {return  date.Days ();}
double       DateTime::ToHours   ()  const  {return  date.Days () * 24 + time.ToHours ();}
KKU::uint64  DateTime::ToSeconds ()  const  {return  (uint64)(date.Days ()) * (uint64)86400 + time.Seconds ();};


void  DateTime::AddDays (int32  _days)
{
  date.AddDays (_days);
}


void  DateTime::AddHours (int32  _hours)
{
  if  (_hours < 0)
  {
    int32  newHour = time.Hour () + _hours;
    while  (newHour < 0)
    {
      date = date - 1;
      newHour += 24;
    }
    time.Hour ((uchar)newHour);
  }
  else
  {
    int32  newHour = time.Hour () + _hours;
    while  (newHour > 23)
    {
      date = date + 1;
      newHour -= 24;
    }
    time.Hour ((uchar)newHour);
  }
}  /* AddHours */


void  DateTime::AddMinutes  (int32  _mins)
{
  int32  newMins = time.Minute () + _mins;

  while  (newMins < 0)
  {
    HoursAdd (-1);
    newMins += 60;
  }

  while  (newMins >= 60)
  {
    HoursAdd (1);
    newMins -= 60;
  }

  time.Minute ((uchar)newMins);
}  /* AddMinutes */


void  DateTime::AddSeconds  (long _secs)
{
  long  newSecs = time.Second () + _secs;

  long  minsToAdd = newSecs / 60;
  newSecs = newSecs - (minsToAdd * 60);
  if  (minsToAdd != 0)
    MinutesAdd (minsToAdd);

  while  (newSecs < 0)
  {
    MinutesAdd (-1);
    newSecs += 60;
  }

  while  (newSecs >= 60)
  {
    MinutesAdd (1);
    newSecs -= 60;
  }

  time.Second ((uchar)newSecs);
}  /* AddSeconds */




void  DateTime::HoursAdd (int32  _hours)
{
  AddHours (_hours);
}  /* HoursAdd */




void  DateTime::MinutesAdd  (int32 _mins)
{
  AddMinutes (_mins);
}  /* MinutesAdd */




void  DateTime::SecondsAdd  (long  _secs)
{
  AddSeconds (_secs);
}   /* SecondsAdd */



KKStr  DateTime::YYYYMMDDHHMMSS ()  const
{
  return  date.YYYYMMDD () + time.HHMMSS ();
}



KKStr  DateTime::YYYY_MM_DD_HH_MM_SS () const
{
  return  date.YYYY_MM_DD () + "-" + time.HH_MM_SS ();
}


KKStr  DateTime::HH_MM_SS () const
{
  return  StrFormatInt ((date.ToHours () + time.Hour ()), "###00") + ":" + 
          StrFormatInt (time.Minute (), "00")                      + ":" +
          StrFormatInt (time.Second (), "00");
}



DateTime&  DateTime::operator= (const DateTime&  right)
{
  date = right.date;
  time = right.time;
  return *this;
}




bool  DateTime::operator== (const DateTime&  right)  const
{
  return  Compare (right) == 0;
}



bool  DateTime::operator!= (const  DateTime&  right)  const
{
  return  Compare (right) != 0;
}



bool  DateTime::operator>  (const DateTime&  right)  const
{
  return  Compare (right) > 0;
}



bool  DateTime::operator>= (const DateTime&  right)  const
{
  return  Compare (right) >= 0;
}


bool  DateTime::operator<  (const DateTime&  right)  const
{
  return  Compare (right) < 0;
}


bool  DateTime::operator<= (const DateTime&  right)  const
{
  return  Compare (right) <= 0;
}



DateTime   DateTime::operator+ (const DateTime& right)  const
{
  int32 netDays = date.Days ()    + right.date.Days ();
  int32 netSecs = time.Seconds () + right.time.Seconds (); 
  netSecs = netSecs % 86400;

  return  DateTime (DateType (netDays), TimeType (netSecs));
}




DateTime   DateTime::operator- (const DateTime& right)  const
{
  int32 netDays = date.Days () - right.date.Days ();

  int32 leftSecs  = time.Seconds ();
  int32 rightSecs = right.time.Seconds ();

  while  (rightSecs > leftSecs)
  {
    netDays--;
    leftSecs = leftSecs + 86400; 
  }

  int32 netSecs = leftSecs - rightSecs;

  int32 numOfDaysInSecs = netSecs / 86400;
  netSecs = netSecs % 86400;

  return  DateTime (DateType (netDays + numOfDaysInSecs), TimeType (netSecs));
}






KKStr&  KKU::operator<< (      KKStr&     left,
                         const DateTime&  right
                        )
{
  left << right.Date () << "-" << right.Time ();
  return left;
}




ostream&  KKU::operator<< (      ostream&  os, 
                           const DateTime& right
                          )
{
  os << right.Date () << "-" << right.Time ();
  return  os;
}



