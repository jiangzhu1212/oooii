/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// Support Date/Time/Wall Clock functionality in a way that should really be the 
// final statement on the matter. NTPv4 has attosecond accuracy and inherently
// supports more than one 136 era, so ntp_date should really be the ultimate 
// thing. Support all the legacy formats too: time_t, file_time_t and a format 
// that is human-workable: date.
// NTP v4: http://tools.ietf.org/html/rfc5905#section-6
#pragma once
#ifndef oDate_h
#define oDate_h

#include <oStd/fixed_string.h>
#include <oStd/operators.h>
#include <oStd/chrono.h>
#include <oStd/uint128.h>
#include <climits>

namespace oStd {

// A directly-comparable NTP short format timestamp. Use this for small delta 
// time comparisons. (though using a raw oTimer or oTimerMS will be as effective).
typedef unsigned int ntp_time;

// A directly-comparable NTP timestamp. Use this when time_t or FILETIME would 
// be considered. This is basically NTPv1/v2-ish.
typedef unsigned long long ntp_timestamp;

// A directly-comparable NTP date. Use this when attosecond accuracy is 
// necessary (hope the platform supports it!) or dates far into the past or 
// future are required.
struct ntp_date : uint128 {};

// This is a fixed-point representation of a portion of a second. According
// to NTPv4 docs, oFractionalSecond32 will be in units of about 232 picoseconds 
// and oFractionalSecond64 is in units of 0.05 attoseconds
typedef chrono::duration<unsigned short, ratio<1,USHRT_MAX>> fractional_second16;
typedef chrono::duration<unsigned int, ratio<1,UINT_MAX>> fractional_second32;
typedef chrono::duration<unsigned long long, ratio<1,ULLONG_MAX>> fractional_second64;
typedef chrono::duration<long long, pico> picoseconds;
typedef chrono::duration<long long, atto> attoseconds;

// this is in 100 nanosecond units
typedef chrono::duration<long long, ratio<1,10000000>> file_time;

class file_time_t // FILETIME on windows
{
public:
	file_time_t() {}
	file_time_t(long long _Num100NanosecondIntervals)
		: dwLowDateTime(_Num100NanosecondIntervals & ~0u)
		, dwHighDateTime(_Num100NanosecondIntervals >> 32ll)
	{}

	operator long long() const
	{
		unsigned long long ull = (static_cast<unsigned long long>(dwHighDateTime) << 32) | dwLowDateTime;
		return *(long long*)&ull;
	}

private:
	unsigned int dwLowDateTime;
	unsigned int dwHighDateTime;
};

namespace weekday { enum value { Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Unknown, };}
namespace month { enum value { January = 1, February, March, April, May, June, July, August, September, October, November, December, };}

inline const char* as_string(const weekday::value& _Weekday)
{
	static const char* sWeekdayStrings[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
	return sWeekdayStrings[_Weekday];
}

inline const char* as_string(const month::value& _Month)
{
	static const char* sMonthStrings[13] = { "invalid", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	return sMonthStrings[_Month];
}

// This calculates dates using the proleptic Gregorian calendar method - i.e.
// where all dates, even those before the Gregorian calendar was invented, are 
// calculated using the Gregorian method. This is consistent with how NTP 
// reports time.
struct date : oComparable<date>
{
	date(int _Year = 0, month::value _Month = month::January, int _Day = 0, int _Hour = 0, int _Minute = 0, int _Second = 0, int _Millisecond = 0)
		: year(_Year)
		, month(_Month)
		, day_of_week(weekday::Unknown)
		, day(_Day)
		, hour(_Hour)
		, minute(_Minute)
		, second(_Second)
		, millisecond(_Millisecond)
	{}

	int year; // The full astronomical year. (1 == 1 CE, 1 BCE == 0, 2 BCE = -1, 4713 BCE = -4712)
	month::value month;
	weekday::value day_of_week; // this is ignored when date is a source for date_cast
	int day; // [1,31]
	int hour; // [0,23]
	int minute; // [0,59]
	int second; // [0,59] We don't support leap seconds because the standards don't
	int millisecond; // [0,999] @oooii-tony: NTPv4: Attoseconds, Picoseconds, really? I'll update the code when I can measure those kind of numbers.

	inline bool operator==(const date& _That) const { return year == _That.year && month == _That.month && day == _That.day && hour == _That.hour && minute == _That.minute && second == _That.second && millisecond == _That.millisecond; }
	inline bool operator<(const date& _That) const
	{
		return year < _That.year
			|| (year == _That.year && month < _That.month)
			|| (year == _That.year && month == _That.month && day < _That.day)
			|| (year == _That.year && month == _That.month && day == _That.day && hour < _That.hour)
			|| (year == _That.year && month == _That.month && day == _That.day && hour == _That.hour && minute < _That.minute)
			|| (year == _That.year && month == _That.month && day == _That.day && hour == _That.hour && minute == _That.minute && second < _That.second)
			|| (year == _That.year && month == _That.month && day == _That.day && hour == _That.hour && minute == _That.minute && second == _That.second && millisecond < _That.millisecond);
	}
};

static const date min_julian_valid_date(-4800, month::March, 1, 12);
static const date julian_epoch_start(-4712, month::January, 1, 12);
static const date julian_epoch_end(1582, month::October, 4);
static const date gregorian_epoch_start(1582, month::October, 15);
static const date gregorian_epoch_start_england_sweden(1752, month::September, 2);
static const date ntp_epoch_start(1900, month::January, 1);
static const date ntp_epoch_end(2036, month::February, 7, 6, 28, 14);
static const date unix_time_epoch_start(1970, month::January, 1);
static const date unix_time_epoch_end_signed32(2038, month::January, 19, 3, 14, 7);
static const date unix_time_epoch_end_unsigned32(2106, month::February, 7, 6, 28, 15);
static const date file_time_epoch_start(1601, month::January, 1);
//static const date file_time_epoch_end(?);

// Returns the Julian Day Number (JDN), (i.e. Days since noon on the first day 
// of recorded history: Jan 1, 4713 BCE. This function interprets on or after 
// Oct 15, 1582 as a Gregorian Calendar date and dates prior as a Julian 
// Calendar Date. This will not return correct values for dates before 
// March 1, 4801 BCE and will instead return oInvalid.
long long julian_day_number(const date& _Date);

// Returns the Julian Date (JD) from the specified date. This begins with 
// calculation of the Julian Day Number, so if that is invalid, this will return
// NaN.
double julian_date(const date& _Date);

// Returns the Modified Julian Date (MJD) from the specified date. This begins
// with calculation of the Julian Day Number, so if that is invalid, this will 
// return NaN.
double modified_julian_date(const date& _Date);

// Returns the day of the week for the specified date
weekday::value day_of_week(const date& _Date);

// Returns the component of an ntp_date according to NTPv4.
inline long long get_ntp_date(const ntp_date& _Date) { return *(long long*)&_Date.DataMS; }
inline int get_ntp_era(const ntp_date& _Date) { unsigned int i = _Date.DataMS >> 32; return *(int*)&i; }
inline unsigned int get_ntp_timestamp(const ntp_date& _Date) { return _Date.DataMS & ~0u; }
inline double get_ntp_fractional_seconds(const ntp_date& _Date) { return _Date.DataLS / (double)ULLONG_MAX; }
inline long long get_ntp_seconds(const ntp_date& _Date) { long long Era = get_ntp_era(_Date); return (Era << 32) + get_ntp_timestamp(_Date); }
inline unsigned int get_ntp_timestamp(const ntp_timestamp& _Timestamp) { return _Timestamp >> 32; }
inline double get_ntp_fractional_seconds(const ntp_timestamp& _Timestamp) { return (_Timestamp & ~0u) / (double)std::numeric_limits<unsigned int>::max(); }

// cast from one date type to another
template<typename DateT1, typename DateT2> DateT1 date_cast(const DateT2& _Date);

// Common date formats for use with oStd::strftime

static const char* http_date_format = "%a, %d %b %Y %H:%M:%S GMT"; // Fri, 31 Dec 1999 23:59:59 GMT

// Sortable in a text output
static const char* sortable_date_format = "%Y/%m/%d %X"; // 1999/12/31 23:59:59
static const char* sortable_date_ms_format = "%Y/%m/%d %X:%0s"; // 1999/12/31 23:59:59:325
static const char* short_date_format = "%m/%d %H:%M"; // 12/31 23:59

// The RFC 5424 formats: http://tools.ietf.org/html/rfc5424#section-6.1
static const char* syslog_utc_date_format = "%Y-%m-%dT%X.%0sZ"; // 2003-10-11T22:14:15.003Z
static const char* syslog_local_date_format = "%Y-%m-%dT%X.%0s%o"; // 2003-08-24T05:14:15.000003-07:00

namespace date_conversion
{	enum value {
	none,
	to_local,
};}

// Same syntax and usage as strftime, with the added formatters:
// %o: +-H:M offset from UTC time.
// %s: milliseconds. or %0s, meaning pad to 3 digits (i.e. 003)
// %u: microseconds. or %0u, meaning pad to 6 digits (i.e. 000014)
// %P: picoseconds.
// %t: attoseconds.
size_t strftime(char* _StrDestination
	, size_t _SizeofStrDestination
	, const char* _Format
	, const oStd::ntp_date& _Date
	, date_conversion::value _Conversion = date_conversion::none);

template<typename DATE_T>
size_t strftime(char* _StrDestination
	, size_t _SizeofStrDestination
	, const char* _Format
	, const DATE_T& _Date
	, date_conversion::value _Conversion = date_conversion::none)
{
	return strftime(_StrDestination
		, _SizeofStrDestination
		, _Format
		, date_cast<oStd::ntp_date>(_Date)
		, _Conversion);
}

template<typename DATE_T, size_t size>
size_t strftime(char (&_StrDestination)[size]
	, const char* _Format
	, const DATE_T& _Date
	, date_conversion::value _Conversion = date_conversion::none)
{
	return strftime(_StrDestination, size, _Format, _Date, _Conversion);
}

template<typename DATE_T, size_t capacity> size_t strftime(fixed_string<char, capacity>& _StrDestination, const char* _Format, const DATE_T& _Date, date_conversion::value _Conversion = date_conversion::none) { return oStd::strftime(_StrDestination, _StrDestination.capacity(), _Format, _Date, _Conversion); }

} // namespace oStd

namespace std {

inline std::string to_string(const oStd::weekday::value& _Weekday) { return oStd::as_string(_Weekday); }
inline std::string to_string(const oStd::month::value& _Month) { return oStd::as_string(_Month); }

} // namespace std

#endif
