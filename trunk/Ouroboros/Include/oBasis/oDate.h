/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// supports more than one 136 era, so oNTPDate should really be the ultimate
// thing. Support all the legacy formats too: time_t, oFILETIME and a format 
// that's human-workable: oDATE.
// NTP v4: http://tools.ietf.org/html/rfc5905#section-6
#pragma once
#ifndef oDate_h
#define oDate_h

#include <oBasis/oFixedString.h>
#include <oBasis/oOperators.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oUint128.h>
#include <oBasis/oStdChrono.h>
#include <climits>

// A directly-comparable NTP short format timestamp. Use this for small delta 
// time comparisons. (though using a raw oTimer or oTimerMS will be as effective).
typedef unsigned int oNTPShortTime;

// A directly-comparable NTP timestamp. Use this when time_t or FILETIME would 
// be considered. This is basically NTPv1/v2-ish.
typedef unsigned long long oNTPTimestamp;

// A directly-comparable NTP date. Use this when attosecond accuracy is 
// necessary (hope the platform supports it!) or dates far into the past or 
// future are required.
typedef uint128 oNTPDate;

// This is a fixed-point representation of a portion of a second. According
// to NTPv4 docs, oFractionalSecond32 will be in units of about 232 picoseconds 
// and oFractionalSecond64 is in units of 0.05 attoseconds
typedef oStd::chrono::duration<unsigned short, oStd::ratio<1,USHRT_MAX>> oFractionalSecond16;
typedef oStd::chrono::duration<unsigned int, oStd::ratio<1,UINT_MAX>> oFractionalSecond32;
typedef oStd::chrono::duration<unsigned long long, oStd::ratio<1,ULLONG_MAX>> oFractionalSecond64;
typedef oStd::chrono::duration<long long, oStd::pico> oPicoseconds;
typedef oStd::chrono::duration<long long, oStd::atto> oAttoseconds;
typedef oStd::chrono::duration<long long, oStd::ratio<1,10000000>> oFileTime100NanosecondUnits;

enum oDATE_CONVERSION
{
	oDATE_NONE,
	oDATE_TO_LOCAL,
};

// @oooii-tony: Should GMT be hard-coded? or is it locale-adjusted. I can 
// imagine the stamp is in UTC, just should confirm.
#define oDATE_HTTP_FORMAT "%a, %d %b %Y %H:%M:%S GMT" // Fri, 31 Dec 1999 23:59:59 GMT

// Sortable in a text output
#define oDATE_TEXT_SORTABLE_FORMAT "%Y/%m/%d %X" // 1999/12/31 23:59:59
#define oDATE_TEXT_SORTABLE_FORMAT_MS "%Y/%m/%d %X:%0s" // 1999/12/31 23:59:59:325

#define oDATE_SHORT_FORMAT "%m/%d %H:%M" // 12/31 23:59

// The RFC 5424 formats: http://tools.ietf.org/html/rfc5424#section-6.1
#define oDATE_SYSLOG_FORMAT_UTC "%Y-%m-%dT%X.%0sZ" // 2003-10-11T22:14:15.003Z
#define oDATE_SYSLOG_FORMAT_LOCAL "%Y-%m-%dT%X.%0s%o" // 2003-08-24T05:14:15.000003-07:00

class oFILETIME
{
	unsigned int dwLowDateTime;
	unsigned int dwHighDateTime;

public:
	oFILETIME() {}
	oFILETIME(long long _Num100NanosecondIntervals)
		: dwLowDateTime(_Num100NanosecondIntervals & ~0u)
		, dwHighDateTime(_Num100NanosecondIntervals >> 32ll)
	{}
	
	operator long long() const
	{
		unsigned long long ull = (static_cast<unsigned long long>(dwHighDateTime) << 32) | dwLowDateTime;
		return *(long long*)&ull;
	}
};

enum oWEEKDAY
{
	oSUNDAY,
	oMONDAY,
	oTUESDAY,
	oWEDNESDAY,
	oTHURSDAY,
	oFRIDAY,
	oSATURDAY,
	oUNKOWN_WEEKDAY,
};

enum oMONTH
{
	oJANUARY = 1,
	oFEBRUARY,
	oMARCH,
	oAPRIL,
	oMAY,
	oJUNE,
	oJULY,
	oAUGUST,
	oSEPTEMBER,
	oOCTOBER,
	oNOVEMBER,
	oDECEMBER,
};

oAPI const char* oAsString(const oWEEKDAY& _Weekday);
oAPI const char* oAsString(const oMONTH& _Month);

// This calculates dates using the proleptic Gregorian calendar method - i.e.
// where all dates, even those before the Gregorian calendar was invented, are 
// calculated using the Gregorian method. This is consistent with how NTP 
// reports time.
struct oDATE : oCompareable<oDATE>
{
	oDATE(int _Year = 0, oMONTH _Month = oJANUARY, int _Day = 0, int _Hour = 0, int _Minute = 0, int _Second = 0, int _Millisecond = 0)
		: Year(_Year)
		, Month(_Month)
		, DayOfWeek(oUNKOWN_WEEKDAY)
		, Day(_Day)
		, Hour(_Hour)
		, Minute(_Minute)
		, Second(_Second)
		, Millisecond(_Millisecond)
	{}

	int Year; // The full astronomical year. (1 == 1 CE, 1 BCE == 0, 2 BCE = -1, 4713 BCE = -4712)
	oMONTH Month;
	oWEEKDAY DayOfWeek; // this is ignored when oDATE is a source for oDateConvert
	int Day; // [1,31]
	int Hour; // [0,23]
	int Minute; // [0,59]
	int Second; // [0,59] We don't support leap seconds because the standards don't
	int Millisecond; // [0,999] @oooii-tony: NTPv4: Attoseconds, Picoseconds, really? I'll update the code when I can measure those kind of numbers.

	inline bool operator==(const oDATE& _That) const { return Year == _That.Year && Month == _That.Month && Day == _That.Day && Hour == _That.Hour && Minute == _That.Minute && Second == _That.Second && Millisecond == _That.Millisecond; }
	inline bool operator<(const oDATE& _That) const
	{
		return Year < _That.Year
			|| (Year == _That.Year && Month < _That.Month)
			|| (Year == _That.Year && Month == _That.Month && Day < _That.Day)
			|| (Year == _That.Year && Month == _That.Month && Day == _That.Day && Hour < _That.Hour)
			|| (Year == _That.Year && Month == _That.Month && Day == _That.Day && Hour == _That.Hour && Minute < _That.Minute)
			|| (Year == _That.Year && Month == _That.Month && Day == _That.Day && Hour == _That.Hour && Minute == _That.Minute && Second < _That.Second)
			|| (Year == _That.Year && Month == _That.Month && Day == _That.Day && Hour == _That.Hour && Minute == _That.Minute && Second == _That.Second && Millisecond < _That.Millisecond);
	}
};

#define oDATE_JULIAN_EARLIEST_VALID_DATE oDATE(-4800, oMARCH, 1, 12) 
#define oDATE_JULIAN_EPOCH_START oDATE(-4712, oJANUARY, 1, 12)
#define oDATE_JULIAN_EPOCH_END oDATE(1582, oOCTOBER, 4)
#define oDATE_GREGORIAN_EPOCH_START oDATE(1582, oOCTOBER, 15)
#define oDATE_GREGORIAN_EPOCH_START_ENGLAND_SWEDEN oDATE(1752, oSEPTEMBER, 2)
#define oDATE_NTP_EPOCH_START oDATE(1900, oJANUARY, 1)
#define oDATE_NTP_EPOCH_END oDATE(2036, oFEBRUARY, 7, 6, 28, 14)
#define oDATE_UNIX_TIME_EPOCH_START oDATE(1970, oJANUARY, 1)
#define oDATE_UNIX_TIME_EPOCH_END_SIGNED32 oDATE(2038, oJANUARY, 19, 3, 14, 7)
#define oDATE_UNIX_TIME_EPOCH_END_UNSIGNED32 oDATE(2106, oFEBRUARY, 7, 6, 28, 15)
#define oDATE_FILE_TIME_EPOCH_START oDATE(1601, oJANUARY, 1)
//#define oDATE_FILE_TIME_EPOCH_END ?

// Returns the Julian Day Number (JDN), (i.e. Days since noon on the first day 
// of recorded history: Jan 1, 4713 BCE. This function interprets on or after 
// Oct 15, 1582 as a Gregorian Calendar date and dates prior as a Julian 
// Calendar Date. This will not return correct values for dates before 
// March 1, 4801 BCE and will instead return oInvalid.
oAPI long long oDateCalcJulianDayNumber(const oDATE& _Date);

// Returns the Julian Date (JD) from the specified date. This begins with 
// calculation of the Julian Day Number, so if that is invalid, this will return
// NaN.
oAPI double oDateCalcJulian(const oDATE& _Date);

// Returns the Modified Julian Date (MJD) from the specified date. This begins
// with calculation of the Julian Day Number, so if that is invalid, this will 
// return NaN.
oAPI double oDateCalcModifiedJulian(const oDATE& _Date);

// Returns the component of an oNTPDate according to NTPv4.
oAPI long long oDateGetNTPDate(const oNTPDate& _Date);
oAPI int oDateGetNTPEra(const oNTPDate& _Date);
oAPI unsigned int oDateGetNTPTimestamp(const oNTPDate& _Date);
oAPI double oDateGetNTPSecondFraction(const oNTPDate& _Date);
oAPI long long oDateGetNTPSeconds(const oNTPDate& _Date);

oAPI unsigned int oDateGetNTPTimestamp(const oNTPTimestamp& _Timestamp);
oAPI double oDateGetNTPSecondFraction(const oNTPTimestamp& _Timestamp);

// If this returns 0, then there was an overflow, i.e. the specified time was 
// outside NTP's epoch/upper limit.

// This expects any combination of: oNTPShortTime, oNTPTimestamp, oNTPDate, 
// time_t, double (seconds), oFILETIME and oDATE (on supported platforms, 
// FILETIME should be expected to work directly).
template<typename DATE1_T, typename DATE2_T> bool oDateConvert(const DATE1_T& _Source, DATE2_T* _pDestination);

// Same syntax and usage as strftime, with the added formatters:
// %o: +-H:M offset from UTC time.
// %s: milliseconds. or %0s, meaning pad to 3 digits (i.e. 003)
// %u: microseconds. or %0u, meaning pad to 6 digits (i.e. 000014)
// %P: picoseconds.
// %t: attoseconds.
oAPI size_t oDateStrftime(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, const oNTPDate& _Date, oDATE_CONVERSION _Conversion = oDATE_NONE);

template<typename DATE_T> size_t oDateStrftime(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, const DATE_T& _Date, oDATE_CONVERSION _Conversion = oDATE_NONE)
{
	oNTPDate d;
	if (!oDateConvert(_Date, &d))
		return 0;
	return oDateStrftime(_StrDestination, _SizeofStrDestination, _Format, d, _Conversion);
}

template<typename DATE_T, size_t size> size_t oDateStrftime(char (&_StrDestination)[size], const char* _Format, const DATE_T& _Date, oDATE_CONVERSION _Conversion = oDATE_NONE) { return oDateStrftime(_StrDestination, size, _Format, _Date, _Conversion); }
template<typename DATE_T, size_t capacity> size_t oDateStrftime(oFixedString<char, capacity>& _StrDestination, const char* _Format, const DATE_T& _Date, oDATE_CONVERSION _Conversion = oDATE_NONE) { return oDateStrftime(_StrDestination, _StrDestination.capacity(), _Format, _Date, _Conversion); }

#endif
