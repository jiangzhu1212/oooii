/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oDate.h>
#include <oBasis/oError.h>
#include <oBasis/oInt.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oMath.h>
#include <oBasis/oStdChrono.h>
#include <time.h>
#include "calfaq.h"
// NTP v4

using namespace oStd::chrono;

#define oDATE_OUT_OF_RANGE() do { return oErrorSetLast(oERROR_AT_CAPACITY, "out of range %s -> %s", typeid(_Source).name(), typeid(*_pDestination).name()); } while(false)

static const char* sWeekdayStrings[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char* sMonthStrings[13] = { "invalid", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

const char* oAsString(const oWEEKDAY& _Weekday)
{
	return sWeekdayStrings[_Weekday];
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oWEEKDAY& _Weekday)
{
	oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Weekday));
	return _StrDestination;
}

bool oFromString(oWEEKDAY* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < oCOUNTOF(sWeekdayStrings); i++)
	{
		if (!oStricmp(_StrSource, sWeekdayStrings[i]) || (!_memicmp(_StrSource, sWeekdayStrings[i], 3) && _StrSource[3] == 0))
		{
			*_pValue = (oWEEKDAY)i;
			return true;
		}
	}
	return false;
}

const char* oAsString(const oMONTH& _Month)
{
	return sMonthStrings[_Month];
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oMONTH& _Month)
{
	oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Month));
	return _StrDestination;
}

bool oFromString(oMONTH* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < oCOUNTOF(sMonthStrings); i++)
	{
		if (!oStricmp(_StrSource, sMonthStrings[i]) || (!_memicmp(_StrSource, sMonthStrings[i], 3) && _StrSource[3] == 0))
		{
			*_pValue = (oMONTH)i;
			return true;
		}
	}
	return false;
}

// _____________________________________________________________________________
// Important Dates

static const int kNumSecondsPerDay = 86400;
static const int kNumSecondsPerHour = 3600;

// Documented at http://www.eecis.udel.edu/~mills/database/brief/arch/arch.pdf
// (and it matches calculation from oDateCalcJulianDateNumber)
static const int kNTPEpochJDN = 2415021;
static const int kUnixEpochJDN = 2440588;
static const int kFileTimeEpochJDN = 2305814;
static const int kLastNonGregorianJDN = 2299150;

static const unsigned long long kSecondsFrom1900To1970 = 25567ull * kNumSecondsPerDay;
static const unsigned long long kSecondsFrom1601To1900 = static_cast<unsigned long long>(kNTPEpochJDN - kFileTimeEpochJDN) * kNumSecondsPerDay;

// From NTP reference implementation ntp-dev-4.2.7p295/libntp/ntp_calendar.cpp:
/*
 * Some notes on the terminology:
 *
 * We use the proleptic Gregorian calendar, which is the Gregorian
 * calendar extended in both directions ad infinitum. This totally
 * disregards the fact that this calendar was invented in 1582, and
 * was adopted at various dates over the world; sometimes even after
 * the start of the NTP epoch.
 */
// But this causes the Julian epoch to not be right.

//#define USE_GREGORIAN_ALWAYS

#ifdef USE_GREGORIAN_ALWAYS
	#define IF_SHOULD_USE_GREGORIAN(_oDATE) if (true)
	#define IF_SHOULD_USE_GREGORIAN2(_JDN) if (true)
#else
	//#define IF_SHOULD_USE_GREGORIAN(_oDATE) if (_oDATE > oDATE_JULIAN_EPOCH_END)
	#define IF_SHOULD_USE_GREGORIAN(_oDATE) if (_oDATE > oDATE(-2000, oJANUARY, 1)) // @oooii-tony: I have not justification for this choice, but the EPOCH is simply not calculated using Gregorian, but 2 BCE starts failing using Julian...
	#define IF_SHOULD_USE_GREGORIAN2(_JDN) if (_JDN > 730500) // 2000 * 365 + 2000/4
#endif

// _____________________________________________________________________________
// Julian Date Support

long long oDateCalcJulianDayNumber(const oDATE& _Date)
{
	static const oDATE FloorDate = oDATE_JULIAN_EARLIEST_VALID_DATE;
	if (_Date.Year < FloorDate.Year || (_Date.Year == FloorDate.Year && _Date.Month < FloorDate.Month))
		return oInvalid;

	int style = JULIAN; // 0 = julian, 1 = gregorian
	IF_SHOULD_USE_GREGORIAN(_Date)
		style = GREGORIAN;

	return date_to_jdn(style, _Date.Year, _Date.Month, _Date.Day);
}

double oDateCalcJulian(const oDATE& _Date)
{
	long long JDN = oDateCalcJulianDayNumber(_Date);
	if (JDN == oInvalid)
		return std::numeric_limits<double>::quiet_NaN();
	return JDN + (_Date.Hour-12)/24.0 + _Date.Minute/1440.0 + _Date.Second/86400.0;
}

double oDateCalcModifiedJulian(const oDATE& _Date)
{
	double JD = oDateCalcJulian(_Date);
	if (isnan(JD))
		return JD;
	return JD - 2400000.5;
}

void oDateCalcFromJulianDayNumber(long long _JDN, oDATE* _pDate)
{
	int style = JULIAN;
	IF_SHOULD_USE_GREGORIAN2(_JDN)
		style = GREGORIAN;
	jdn_to_date(style, oInt(_JDN), &_pDate->Year, (int*)&_pDate->Month, &_pDate->Day);
	_pDate->DayOfWeek = static_cast<oWEEKDAY>(day_of_week(style, abs(_pDate->Year), _pDate->Month, _pDate->Day));
	_pDate->Hour = _pDate->Minute = _pDate->Second = _pDate->Millisecond = 0;
}

// _____________________________________________________________________________
// oNTPDate support

long long oDateGetNTPDate(const oNTPDate& _Date)
{
	return *(long long*)&_Date.DataMS;
}

int oDateGetNTPEra(const oNTPDate& _Date)
{
	unsigned int i = _Date.DataMS >> 32;
	return oInt(*(int*)&i);
}

unsigned int oDateGetNTPTimestamp(const oNTPDate& _Date)
{
	return oUInt(_Date.DataMS & ~0u);
}

double oDateGetNTPSecondFraction(const oNTPDate& _Date)
{
	return _Date.DataLS / (double)ULLONG_MAX;
}

long long oDateGetNTPSeconds(const oNTPDate& _Date)
{
	long long Era = oDateGetNTPEra(_Date);
	return (Era << 32) + oDateGetNTPTimestamp(_Date);
}

unsigned int oDateGetNTPTimestamp(const oNTPTimestamp& _Timestamp)
{
	return _Timestamp >> 32;
}

double oDateGetNTPSecondFraction(const oNTPTimestamp& _Timestamp)
{
	return (_Timestamp & ~0u) / (double)UINT_MAX;
}

// ignores year/month/day and returns the number of seconds for _NumDays + hour + minute + second
long long oDateCalcNumSeconds(long long _NumDays, const oDATE& _Date)
{
	return _NumDays * kNumSecondsPerDay + _Date.Hour*kNumSecondsPerHour + _Date.Minute*60 + _Date.Second; 
}

void oDateCalcHMS(long long _NumSeconds, oDATE* _pDate)
{
	long long s = abs(_NumSeconds);
	_pDate->Hour = (s / 3600) % 24;
	_pDate->Minute = (s / 60) % 60;
	_pDate->Second = s % 60;
}

// _____________________________________________________________________________
// From oDATE

static bool oDateConvert(const oDATE& _Source, int* _pNTPEra, unsigned int* _pNTPTimestamp)
{
	long long JDN = oDateCalcJulianDayNumber(_Source);
	if (JDN == oInvalid)
		return false;
	long long DaysIntoEpoch = JDN - kNTPEpochJDN;
	long long SecondsIntoEpoch = oDateCalcNumSeconds(DaysIntoEpoch, _Source);
	// As documented in RFC5905, section 7.1
	*_pNTPEra = oInt(SecondsIntoEpoch >> 32ll);
	*_pNTPTimestamp = oUInt((SecondsIntoEpoch - (static_cast<long long>(*_pNTPEra) << 32ll)) & 0xffffffff);
	return true;
}

bool oDateConvert(const oDATE& _Source, oNTPTimestamp* _pDestination)
{
	int Era = 0;
	unsigned int Timestamp = 0;
	if (!oDateConvert(_Source, &Era, &Timestamp) || Era != 0)
		oDATE_OUT_OF_RANGE();
	*_pDestination = (static_cast<unsigned long long>(Timestamp) << 32) | oFractionalSecond32(duration_cast<oFractionalSecond32>(milliseconds(_Source.Millisecond))).count();
	return true;
}

bool oDateConvert(const oDATE& _Source, oNTPDate* _pDestination)
{
	int Era = 0;
	unsigned int Timestamp = 0;
	if (!oDateConvert(_Source, &Era, &Timestamp))
		oDATE_OUT_OF_RANGE();
	_pDestination->DataMS = (static_cast<unsigned long long>(*(unsigned int*)&Era) << 32) | Timestamp;
	_pDestination->DataLS = oFractionalSecond64(duration_cast<oFractionalSecond64>(milliseconds(_Source.Millisecond))).count();
	return true;
}

bool oDateConvert(const oDATE& _Source, time_t* _pDestination)
{
	long long JDN = oDateCalcJulianDayNumber(_Source);
	if (JDN < kUnixEpochJDN) // should this cap at the upper end? signed? unsigned? 64-bit?
		oDATE_OUT_OF_RANGE();
	long long UnixDays = JDN - kUnixEpochJDN;
	long long s = oDateCalcNumSeconds(UnixDays, _Source);
	if (s > INT_MAX)
		oDATE_OUT_OF_RANGE();
	*_pDestination = oUInt(s);
	return true;
}

bool oDateConvert(const oDATE& _Source, oFILETIME* _pDestination)
{
	long long JDN = oDateCalcJulianDayNumber(_Source);
	if (JDN < kFileTimeEpochJDN) // should this cap at the upper end? signed? unsigned? 64-bit?
		oDATE_OUT_OF_RANGE();
	seconds s(oDateCalcNumSeconds(JDN - kFileTimeEpochJDN, _Source));
	oFileTime100NanosecondUnits whole = duration_cast<oFileTime100NanosecondUnits>(s);
	oFileTime100NanosecondUnits fractional = duration_cast<oFileTime100NanosecondUnits>(milliseconds(_Source.Millisecond));
	*_pDestination = oFILETIME((whole + fractional).count());
	return true;
}

bool oDateConvert(const oDATE& _Source, oDATE* _pDestination)
{
	// force a cleanup of the data if they are out-of-range
	oNTPDate d;
	if (!oDateConvert(_Source, &d))
		return false; // pass through error
	return oDateConvert(d, _pDestination);
}

// _____________________________________________________________________________
// From oNTPDate

bool oDateConvert(const oNTPDate& _Source, oNTPTimestamp* _pDestination)
{
	unsigned int Era = _Source.DataMS >> 32;
	if (Era != 0)
		oDATE_OUT_OF_RANGE();
	*_pDestination = (_Source.DataMS << 32) | oFractionalSecond32(duration_cast<oFractionalSecond32>(oFractionalSecond64(_Source.DataLS))).count();
	return true;
}

bool oDateConvert(const oNTPDate& _Source, oNTPDate* _pDestination)
{
	*_pDestination = _Source;
	return true;
}

bool oDateConvert(const oNTPDate& _Source, time_t* _pDestination)
{
	int Era = oDateGetNTPEra(_Source);
	if (Era != 0)
		oDATE_OUT_OF_RANGE();
	long long s = oDateGetNTPSeconds(_Source);
	if (s < kSecondsFrom1900To1970)
		oDATE_OUT_OF_RANGE();
	*_pDestination = s - kSecondsFrom1900To1970;
	return true;
}

bool oDateConvert(const oNTPDate& _Source, oFILETIME* _pDestination)
{
	int Era = oDateGetNTPEra(_Source);
	if (Era < -3) // the era when 1601 is
		oDATE_OUT_OF_RANGE();
	seconds s(oDateGetNTPSeconds(_Source) + kSecondsFrom1601To1900);
	if (s.count() < 0)
		oDATE_OUT_OF_RANGE();
	oFileTime100NanosecondUnits whole = duration_cast<oFileTime100NanosecondUnits>(s);
	oFileTime100NanosecondUnits fractional = duration_cast<oFileTime100NanosecondUnits>(oFractionalSecond64(_Source.DataLS));
	*_pDestination = oFILETIME((whole + fractional).count());
	return true;
}

bool oDateConvert(const oNTPDate& _Source, oDATE* _pDestination)
{
	long long s = oDateGetNTPSeconds(_Source);
	long long JDN = (s / kNumSecondsPerDay) + kNTPEpochJDN;
	oDateCalcFromJulianDayNumber(JDN, _pDestination);
	oDateCalcHMS(s, _pDestination);
	_pDestination->Millisecond = oInt(duration_cast<milliseconds>(oFractionalSecond64(_Source.DataLS)).count());
	return true;
}

// _____________________________________________________________________________
// From oNTPTimestamp

bool oDateConvert(const oNTPTimestamp& _Source, oNTPTimestamp* _pDestination)
{
	*_pDestination = _Source;
	return true;
}

bool oDateConvert(const oNTPTimestamp& _Source, oNTPDate* _pDestination)
{
	_pDestination->DataMS = oUInt(_Source >> 32ull);
	_pDestination->DataLS = duration_cast<oFractionalSecond64>(oFractionalSecond32(_Source & ~0u)).count();
	return true;
}

bool oDateConvert(const oNTPTimestamp& _Source, time_t* _pDestination)
{
	unsigned int s = oUInt(_Source >> 32ull);
	if (s < kSecondsFrom1900To1970)
		oDATE_OUT_OF_RANGE();
	*_pDestination = _Source - kSecondsFrom1900To1970;
	return true;
}

bool oDateConvert(const oNTPTimestamp& _Source, oFILETIME* _pDestination)
{
	oFileTime100NanosecondUnits whole = duration_cast<oFileTime100NanosecondUnits>(seconds((_Source >> 32ull) + kSecondsFrom1601To1900));
	oFileTime100NanosecondUnits fractional = duration_cast<oFileTime100NanosecondUnits>(oFractionalSecond32(_Source & ~0u));
	*_pDestination = oFILETIME((whole + fractional).count());
	return true;
}

bool oDateConvert(const oNTPTimestamp& _Source, oDATE* _pDestination)
{
	unsigned int s = oDateGetNTPTimestamp(_Source);
	long long JDN = (s / kNumSecondsPerDay) + kNTPEpochJDN;
	oDateCalcFromJulianDayNumber(JDN, _pDestination);
	oDateCalcHMS(s, _pDestination);
	_pDestination->Millisecond = oInt(duration_cast<milliseconds>(oFractionalSecond32(_Source & ~0u)).count());
	return true;
}

// _____________________________________________________________________________
// From oNTPShortTime

bool oDateConvert(const oNTPShortTime& _Source, oNTPTimestamp* _pDestination)
{
	unsigned long long sec = _Source >> 16;
	*_pDestination = (sec << 32) | duration_cast<oFractionalSecond32>(oFractionalSecond16(_Source & 0xffff)).count();
	return true;
}

bool oDateConvert(const oNTPShortTime& _Source, oNTPDate* _pDestination)
{
	_pDestination->DataMS = _Source >> 16;
	_pDestination->DataLS = duration_cast<oFractionalSecond64>(oFractionalSecond16(_Source & 0xffff)).count();
	return true;
}

bool oDateConvert(const oNTPShortTime& _Source, time_t* _pDestination)
{
	*_pDestination = (_Source >> 16);
	return true;
}

bool oDateConvert(const oNTPShortTime& _Source, oFILETIME* _pDestination)
{
	oFileTime100NanosecondUnits whole = duration_cast<oFileTime100NanosecondUnits>(seconds(_Source >> 16));
	oFileTime100NanosecondUnits fractional = duration_cast<oFileTime100NanosecondUnits>(oFractionalSecond16(_Source & 0xffff));
	whole += fractional;
	if (whole.count() < 0)
		oDATE_OUT_OF_RANGE();
	*_pDestination = whole.count();
	return true;
}

bool oDateConvert(const oNTPShortTime& _Source, oDATE* _pDestination)
{
	// ntp prime epoch
	_pDestination->Year = 1900;
	_pDestination->Month = oJANUARY;
	_pDestination->Day = 1;
	oDateCalcHMS((_Source >> 16), _pDestination);
	_pDestination->Millisecond = oInt(duration_cast<milliseconds>(oFractionalSecond16(_Source & 0xffff)).count());
	return true;
}

// _____________________________________________________________________________
// From time_t

bool oDateConvert(const time_t& _Source, oNTPTimestamp* _pDestination)
{
	long long s = oUInt(_Source + kSecondsFrom1900To1970);
	*_pDestination = s << 32;
	return true;
}

bool oDateConvert(const time_t& _Source, oNTPDate* _pDestination)
{
	_pDestination->DataMS = _Source + kSecondsFrom1900To1970;
	_pDestination->DataLS = 0;
	return true;
}

bool oDateConvert(const time_t& _Source, time_t* _pDestination)
{
	*_pDestination = _Source;
	return true;
}

bool oDateConvert(const time_t& _Source, oFILETIME* _pDestination)
{
	*_pDestination = oFILETIME(duration_cast<oFileTime100NanosecondUnits>(seconds(_Source + kSecondsFrom1601To1900 + kSecondsFrom1900To1970)).count());
	return true;
}

bool oDateConvert(const time_t& _Source, oDATE* _pDestination)
{
	long long JDN = (_Source / kNumSecondsPerDay) + kUnixEpochJDN;
	oDateCalcFromJulianDayNumber(JDN, _pDestination);
	oDateCalcHMS(_Source, _pDestination);
	_pDestination->Millisecond = 0;
	return true;
}

// _____________________________________________________________________________
// From oFILETIME

bool oDateConvert(const oFILETIME& _Source, oNTPTimestamp* _pDestination)
{
	long long usec = (long long)_Source / 10;
	oFractionalSecond32 fractional = duration_cast<oFractionalSecond32>(microseconds(usec % oStd::micro::den));
	long long s = (usec / oStd::micro::den) - kSecondsFrom1601To1900;
	if (s < 0 || ((s & ~0u) != s))
		oDATE_OUT_OF_RANGE();
	*_pDestination = (s << 32) | fractional.count();
	return true;
}

bool oDateConvert(const oFILETIME& _Source, oNTPDate* _pDestination)
{
	long long usec = (long long)_Source / 10;
	_pDestination->DataMS = (usec / oStd::micro::den) - kSecondsFrom1601To1900;
	_pDestination->DataLS = duration_cast<oFractionalSecond64>(oFileTime100NanosecondUnits((long long)_Source % oFileTime100NanosecondUnits::period::den)).count();
	return true;
}

bool oDateConvert(const oFILETIME& _Source, time_t* _pDestination)
{
	long long s = duration_cast<seconds>(microseconds((long long)_Source / 10)).count() - kSecondsFrom1601To1900 - kSecondsFrom1900To1970;
	if (s < 0 || s > INT_MAX)
		oDATE_OUT_OF_RANGE();
	*_pDestination = oInt(s);
	return true;
}

bool oDateConvert(const oFILETIME& _Source, oFILETIME* _pDestination)
{
	*_pDestination = _Source;
	return true;
}

bool oDateConvert(const oFILETIME& _Source, oDATE* _pDestination)
{
	oNTPDate d;
	if (!oDateConvert(_Source, &d))
		return false; // pass through error
	return oDateConvert(d, _pDestination);
}

// _____________________________________________________________________________
// formatting

// Returns the number of seconds to add to a UTC time to get the time in the 
// current locale's timezone.
static int oDateGetTimezoneOffset()
{
	time_t now = time(nullptr);
	tm utc;
	gmtime_s(&utc, &now);
	// get whether we're locally in daylight savings time
	tm local;
	localtime_s(&local, &now);
	bool IsDaylightSavings = !!local.tm_isdst;
	time_t utctimeInterpretedAsLocaltime = mktime(&utc);
	return oInt(now - utctimeInterpretedAsLocaltime) + (IsDaylightSavings ? kNumSecondsPerHour : 0);
}

size_t oDateStrftime(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, const oNTPDate& _Date, oDATE_CONVERSION _Conversion)
{
	oNTPDate DateCopy(_Date);

	int TimezoneOffsetSeconds = oDateGetTimezoneOffset();

	if (_Conversion == oDATE_TO_LOCAL)
		DateCopy.DataMS += TimezoneOffsetSeconds;

	oDATE d;
	if (!oDateConvert(DateCopy, &d))
		return 0; // pass through error

	oFractionalSecond64 fractional(_Date.DataLS);

	*_StrDestination = 0;
	char* w = _StrDestination;
	char* wend = _StrDestination + _SizeofStrDestination;
	const char* f = _Format;
	oStringM s;
	size_t len = 0;

	#define ADD_CHECK() do \
	{	if (!len) \
			return 0; \
			w += len; \
	} while(false)

	#define ADDS() do \
	{	len = oPrintf(w, std::distance(w, wend), "%s", s.c_str()); \
		ADD_CHECK(); \
	} while(false)

	#define ADDSTR(_String) do \
	{	len = oPrintf(w, std::distance(w, wend), (_String)); \
		ADD_CHECK(); \
	} while(false)

	#define ADDDIG(n) do \
	{	len = oPrintf(w, std::distance(w, wend), "%u", (n)); \
	ADD_CHECK(); \
	} while(false)

	#define ADD2DIG(n) do \
	{	len = oPrintf(w, std::distance(w, wend), "%02u", (n)); \
		ADD_CHECK(); \
	} while(false)

	#define ADDDATE(_Format) do \
	{	len = oDateStrftime(w, std::distance(w, wend), _Format, _Date, _Conversion); \
		ADD_CHECK(); \
	} while(false)

	#define ADDDUR(durationT) do \
	{ durationT d = duration_cast<durationT>(fractional); \
		len = oPrintf(w, std::distance(w, wend), "%llu", d.count()); \
		ADD_CHECK(); \
	} while(false)

	#define ADDDUR0(pad, durationT) do \
	{ durationT d = duration_cast<durationT>(fractional); \
		len = oPrintf(w, std::distance(w, wend), "%0" #pad "llu", d.count()); \
		ADD_CHECK(); \
	} while(false)

	while (*f)
	{
		if (*f == '%')
		{
			f++;
			switch (*f)
			{
				case 'a': s = oAsString(d.DayOfWeek); s[3] = 0; ADDS(); break;
				case 'A': ADDSTR(oAsString(d.DayOfWeek)); break;
				case 'b': s = oAsString(d.Month); s[3] = 0; ADDS(); break;
				case 'B': ADDSTR(oAsString(d.Month)); break;
				case 'c': ADDDATE("%a %b %d %H:%M:%S %Y"); break;
				case 'd': ADD2DIG(d.Day); break;
				case 'H': ADD2DIG(d.Hour); break;
				case 'I': ADD2DIG(d.Hour % 12); break;
				case 'j': oASSERT(false, "Day of year not yet supported"); break;
				case 'm': ADD2DIG(d.Month); break;
				case 'M': ADD2DIG(d.Minute); break;
				case 'o': 
				{
					int Hours = TimezoneOffsetSeconds / 3600;
					int Minutes = (abs(TimezoneOffsetSeconds) / 60) - abs(Hours * 60);
					len = oPrintf(w, std::distance(w, wend), "%+d:%02d", Hours, Minutes);
					ADD_CHECK();
					break;
				}

				case 'p': ADDSTR(d.Hour < 12 ? "AM" : "PM"); break;
				case 'S': ADD2DIG(d.Second); break;
				case 'U': oASSERT(false, "Week of year not yet supported"); break;
				case 'w': ADDDIG(d.DayOfWeek); break;
				case 'W': oASSERT(false, "Week of year not yet supported"); break;
				case 'x': ADDDATE("%m/%d/%y"); break;
				case 'X': ADDDATE("%H:%M:%S"); break;
				case 'y': ADD2DIG(d.Year % 100); break;
				case 'Y':
				{
					if (d.Year >= 1)
						ADDDIG(d.Year);
					else
					{
						ADDDIG(abs(--d.Year));
						ADDSTR(" BCE");
					}
					break;
				}

				case 'Z':
				{
					time_t t;
					time(&t);
					tm TM;
					localtime_s(&TM, &t);
					len = strftime(_StrDestination, std::distance(w, wend), "%Z", &TM);
					ADD_CHECK();
					break;
				}

				case '%': *w++ = *f++; break;
				// Non-standard fractional seconds
				case 's': ADDDUR(milliseconds); break;
				case 'u': ADDDUR(microseconds); break;
				case 'P': ADDDUR(oPicoseconds); break;
				case 't': ADDDUR(oAttoseconds); break;

				case '0':
				{
					f++;
					switch (*f)
					{
						case 's': ADDDUR0(3, milliseconds); break;
						case 'u': ADDDUR0(6, microseconds); break;
						default:
							oErrorSetLast(oERROR_NOT_FOUND, "not supported: formatting token %%0%c", *f);
							return 0;
					}
					
					break;
				}
				
				default:
					oErrorSetLast(oERROR_NOT_FOUND, "not supported: formatting token %%%c", *f);
					return 0;
			}

			f++;
		}

		else
			*w++ = *f++;
	}

	*w = 0;
	return std::distance(_StrDestination, w);
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oDATE& _Date)
{
	return oDateStrftime(_StrDestination, _SizeofStrDestination, oDATE_TEXT_SORTABLE_FORMAT, _Date) ? _StrDestination : nullptr;
}
