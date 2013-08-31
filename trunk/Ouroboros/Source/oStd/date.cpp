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
#include <oStd/date.h>
#include <oStd/assert.h>
#include <oStd/string.h>
#include <oStd/throw.h>
#include <ctime>
#include "calfaq.h"
// NTP v4

using namespace oStd::chrono;

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

namespace oStd {

#define oDATE_OUT_OF_RANGE(_ToType) do { throw std::domain_error(oStd::formatf("date_cast<%s>(const %s&) out of range", typeid(_Date).name(), typeid(_ToType).name())); } while(false)

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
	//#define IF_SHOULD_USE_GREGORIAN(_Date) if (_oDATE > oDATE_JULIAN_EPOCH_END)
	#define IF_SHOULD_USE_GREGORIAN(_oDATE) if (_oDATE > date(-2000, month::January, 1)) // @oooii-tony: I have no justification for this choice, but the EPOCH is simply not calculated using Gregorian, but 2 BCE starts failing using Julian...
	#define IF_SHOULD_USE_GREGORIAN2(_JDN) if (_JDN > 730500) // 2000 * 365 + 2000/4
#endif

// _____________________________________________________________________________
// Julian Date Support

long long oStd::julian_day_number(const date& _Date)
{
	if (_Date.year < min_julian_valid_date.year || (_Date.year == min_julian_valid_date.year && _Date.month < min_julian_valid_date.month))
		return -1;

	int style = JULIAN; // 0 = julian, 1 = gregorian
	IF_SHOULD_USE_GREGORIAN(_Date)
		style = GREGORIAN;

	return date_to_jdn(style, _Date.year, _Date.month, _Date.day);
}

double oStd::julian_date(const date& _Date)
{
	long long JDN = julian_day_number(_Date);
	if (JDN == -1)
		return std::numeric_limits<double>::quiet_NaN();
	return JDN + (_Date.hour-12)/24.0 + _Date.minute/1440.0 + _Date.second/86400.0;
}

double oStd::modified_julian_date(const date& _Date)
{
	double JD = julian_date(_Date);
	if (JD != JD) // where's my std::isnan()?
		return JD;
	return JD - 2400000.5;
}

weekday::value day_of_week(const date& _Date)
{
	int style = JULIAN; // 0 = julian, 1 = gregorian
	IF_SHOULD_USE_GREGORIAN(_Date)
		style = GREGORIAN;
	return static_cast<oStd::weekday::value>(::day_of_week(style, abs(_Date.year), _Date.month, _Date.day));
}

static void oDateCalcFromJulianDayNumber(long long _JDN, date* _pDate)
{
	int style = JULIAN;
	IF_SHOULD_USE_GREGORIAN2(_JDN)
		style = GREGORIAN;
	jdn_to_date(style, static_cast<int>(_JDN), &_pDate->year, (int*)&_pDate->month, &_pDate->day);
	_pDate->day_of_week = static_cast<weekday::value>(::day_of_week(style, abs(_pDate->year), _pDate->month, _pDate->day));
	_pDate->hour = _pDate->minute = _pDate->second = _pDate->millisecond = 0;
}

// ignores year/month/day and returns the number of seconds for _NumDays + hour + minute + second
long long oDateCalcNumSeconds(long long _NumDays, const date& _Date)
{
	return _NumDays * kNumSecondsPerDay + _Date.hour*kNumSecondsPerHour + _Date.minute*60 + _Date.second; 
}

void oDateCalcHMS(long long _NumSeconds, date* _pDate)
{
	long long s = abs(_NumSeconds);
	_pDate->hour = (s / 3600) % 24;
	_pDate->minute = (s / 60) % 60;
	_pDate->second = s % 60;
}

// _____________________________________________________________________________
// From date

static bool date_cast(const date& _Source, int* _pNTPEra, unsigned int* _pNTPTimestamp)
{
	long long JDN = julian_day_number(_Source);
	if (JDN == -1)
		return false;
	long long DaysIntoEpoch = JDN - kNTPEpochJDN;
	long long SecondsIntoEpoch = oDateCalcNumSeconds(DaysIntoEpoch, _Source);
	// As documented in RFC5905, section 7.1
	*_pNTPEra = static_cast<int>(SecondsIntoEpoch >> 32ll);
	*_pNTPTimestamp = static_cast<unsigned int>((SecondsIntoEpoch - (static_cast<long long>(*_pNTPEra) << 32ll)) & 0xffffffff);
	return true;
}

template<> ntp_timestamp oStd::date_cast<ntp_timestamp>(const date& _Date)
{
	int Era = 0;
	unsigned int Timestamp = 0;
	if (!date_cast(_Date, &Era, &Timestamp) || Era != 0)
		oDATE_OUT_OF_RANGE(ntp_timestamp);
	return (static_cast<unsigned long long>(Timestamp) << 32) | fractional_second32(duration_cast<fractional_second32>(milliseconds(_Date.millisecond))).count();
}

template<> ntp_date date_cast<ntp_date>(const date& _Date)
{
	int Era = 0;
	unsigned int Timestamp = 0;
	if (!date_cast(_Date, &Era, &Timestamp))
		oDATE_OUT_OF_RANGE(ntp_date);
	ntp_date d;
	d.DataMS = (static_cast<unsigned long long>(*(unsigned int*)&Era) << 32) | Timestamp;
	d.DataLS = fractional_second64(duration_cast<fractional_second64>(milliseconds(_Date.millisecond))).count();
	return std::move(d);
}

template<> time_t date_cast<time_t>(const date& _Date)
{
	long long JDN = julian_day_number(_Date);
	if (JDN < kUnixEpochJDN) // should this cap at the upper end? signed? unsigned? 64-bit?
		oDATE_OUT_OF_RANGE(time_t);
	long long UnixDays = JDN - kUnixEpochJDN;
	long long s = oDateCalcNumSeconds(UnixDays, _Date);
	if (s > INT_MAX)
		oDATE_OUT_OF_RANGE(time_t);
	return static_cast<time_t>(s);
}

template<> file_time_t date_cast<file_time_t>(const date& _Date)
{
	long long JDN = julian_day_number(_Date);
	if (JDN < kFileTimeEpochJDN) // should this cap at the upper end? signed? unsigned? 64-bit?
		oDATE_OUT_OF_RANGE(file_time_t);
	seconds s(oDateCalcNumSeconds(JDN - kFileTimeEpochJDN, _Date));
	file_time whole = duration_cast<file_time>(s);
	file_time fractional = duration_cast<file_time>(milliseconds(_Date.millisecond));
	return std::move(file_time_t((whole + fractional).count()));
}

// _____________________________________________________________________________
// From ntp_date

template<> ntp_timestamp date_cast<ntp_timestamp>(const ntp_date& _Date)
{
	unsigned int Era = _Date.DataMS >> 32;
	if (Era != 0)
		oDATE_OUT_OF_RANGE(ntp_timestamp);
	return (_Date.DataMS << 32) | fractional_second32(duration_cast<fractional_second32>(fractional_second64(_Date.DataLS))).count();
}

template<> ntp_date date_cast<ntp_date>(const ntp_date& _Date)
{
	return _Date;
}

template<> time_t date_cast<time_t>(const ntp_date& _Date)
{
	int Era = oStd::get_ntp_era(_Date);
	if (Era != 0)
		oDATE_OUT_OF_RANGE(time_t);
	long long s = oStd::get_ntp_seconds(_Date);
	if (s < kSecondsFrom1900To1970)
		oDATE_OUT_OF_RANGE(time_t);
	return s - kSecondsFrom1900To1970;
}

template<> file_time_t date_cast<file_time_t>(const ntp_date& _Date)
{
	int Era = oStd::get_ntp_era(_Date);
	if (Era < -3) // the era when 1601 is
		oDATE_OUT_OF_RANGE(file_time_t);
	seconds s(oStd::get_ntp_seconds(_Date) + kSecondsFrom1601To1900);
	if (s.count() < 0)
		oDATE_OUT_OF_RANGE(file_time_t);
	file_time whole = duration_cast<file_time>(s);
	file_time fractional = duration_cast<file_time>(fractional_second64(_Date.DataLS));
	return std::move(file_time_t((whole + fractional).count()));
}

template<> date date_cast<date>(const ntp_date& _Date)
{
	long long s = get_ntp_seconds(_Date);
	long long JDN = (s / kNumSecondsPerDay) + kNTPEpochJDN;
	date d;
	oDateCalcFromJulianDayNumber(JDN, &d);
	oDateCalcHMS(s, &d);
	d.millisecond = static_cast<int>(duration_cast<milliseconds>(fractional_second64(_Date.DataLS)).count());
	return std::move(d);
}

template<> date date_cast<date>(const date& _Date)
{
	// force a cleanup of the data if they are out-of-range
	ntp_date d = date_cast<ntp_date>(_Date);
	return date_cast<date>(d);
}

// _____________________________________________________________________________
// From ntp_timestamp

template<> ntp_timestamp date_cast<ntp_timestamp>(const ntp_timestamp& _Date)
{
	return _Date;
}

template<> ntp_date date_cast<ntp_date>(const ntp_timestamp& _Date)
{
	ntp_date d;
	d.DataMS = static_cast<unsigned int>(_Date >> 32ull);
	d.DataLS = duration_cast<fractional_second64>(fractional_second32(_Date & ~0u)).count();
	return std::move(d);
}

template<> time_t date_cast<time_t>(const ntp_timestamp& _Date)
{
	unsigned int s = _Date >> 32ull;
	if (s < kSecondsFrom1900To1970)
		oDATE_OUT_OF_RANGE(time_t);
	return _Date - kSecondsFrom1900To1970;
}

template<> file_time_t date_cast<file_time_t>(const ntp_timestamp& _Date)
{
	file_time whole = duration_cast<file_time>(seconds((_Date >> 32ull) + kSecondsFrom1601To1900));
	file_time fractional = duration_cast<file_time>(fractional_second32(_Date & ~0u));
	return file_time_t((whole + fractional).count());
}

template<> date date_cast<date>(const ntp_timestamp& _Date)
{
	unsigned int s = oStd::get_ntp_timestamp(_Date);
	long long JDN = (s / kNumSecondsPerDay) + kNTPEpochJDN;
	date d;
	oDateCalcFromJulianDayNumber(JDN, &d);
	oDateCalcHMS(s, &d);
	d.millisecond = static_cast<int>(duration_cast<milliseconds>(fractional_second32(_Date & ~0u)).count());
	return std::move(d);
}

// _____________________________________________________________________________
// From ntp_time

template<> ntp_timestamp date_cast<ntp_timestamp>(const ntp_time& _Date)
{
	unsigned long long sec = _Date >> 16;
	return (sec << 32) | duration_cast<fractional_second32>(fractional_second16(_Date & 0xffff)).count();
}

template<> ntp_date date_cast<ntp_date>(const ntp_time& _Date)
{
	ntp_date d;
	d.DataMS = _Date >> 16;
	d.DataLS = duration_cast<fractional_second64>(fractional_second16(_Date & 0xffff)).count();
	return std::move(d);
}

template<> time_t date_cast<time_t>(const ntp_time& _Date)
{
	return _Date >> 16;
}

template<> file_time_t date_cast<file_time_t>(const ntp_time& _Date)
{
	file_time whole = duration_cast<file_time>(seconds(_Date >> 16));
	file_time fractional = duration_cast<file_time>(fractional_second16(_Date & 0xffff));
	whole += fractional;
	if (whole.count() < 0)
		oDATE_OUT_OF_RANGE(file_time_t);
	return whole.count();
}

template<> date date_cast<date>(const ntp_time& _Date)
{
	// ntp prime epoch
	date d;
	d.year = 1900;
	d.month = month::January;
	d.day = 1;
	oDateCalcHMS((_Date >> 16), &d);
	d.millisecond = static_cast<int>(duration_cast<milliseconds>(fractional_second16(_Date & 0xffff)).count());
	return std::move(d);
}

// _____________________________________________________________________________
// From time_t

template<> ntp_timestamp date_cast<ntp_timestamp>(const time_t& _Date)
{
	long long s = static_cast<unsigned int>(_Date + kSecondsFrom1900To1970);
	return s << 32;
}

template<> ntp_date date_cast<ntp_date>(const time_t& _Date)
{
	ntp_date d;
	d.DataMS = _Date + kSecondsFrom1900To1970;
	d.DataLS = 0;
	return std::move(d);
}

template<> time_t date_cast<time_t>(const time_t& _Date)
{
	return _Date;
}

template<> file_time_t date_cast<file_time_t>(const time_t& _Date)
{
	return std::move(file_time_t(duration_cast<file_time>(seconds(_Date + kSecondsFrom1601To1900 + kSecondsFrom1900To1970)).count()));
}

template<> date date_cast<date>(const time_t& _Date)
{
	long long JDN = (_Date / kNumSecondsPerDay) + kUnixEpochJDN;
	date d;
	oDateCalcFromJulianDayNumber(JDN, &d);
	oDateCalcHMS(_Date, &d);
	d.millisecond = 0;
	return std::move(d);
}

// _____________________________________________________________________________
// From file_time_t

template<> ntp_timestamp date_cast<ntp_timestamp>(const file_time_t& _Date)
{
	long long usec = (long long)_Date / 10;
	fractional_second32 fractional = duration_cast<fractional_second32>(microseconds(usec % micro::den));
	long long s = (usec / micro::den) - kSecondsFrom1601To1900;
	if (s < 0 || ((s & ~0u) != s))
		oDATE_OUT_OF_RANGE(ntp_timestamp);
	return (s << 32) | fractional.count();
}

template<> ntp_date date_cast<ntp_date>(const file_time_t& _Date)
{
	long long usec = (long long)_Date / 10;
	ntp_date d;
	d.DataMS = (usec / micro::den) - kSecondsFrom1601To1900;
	d.DataLS = duration_cast<fractional_second64>(file_time((long long)_Date % file_time::period::den)).count();
	return std::move(d);
}

template<> time_t date_cast<time_t>(const file_time_t& _Date)
{
	long long s = duration_cast<seconds>(microseconds((long long)_Date / 10)).count() - kSecondsFrom1601To1900 - kSecondsFrom1900To1970;
	if (s < 0 || s > INT_MAX)
		oDATE_OUT_OF_RANGE(time_t);
	return static_cast<int>(s);
}

template<> file_time_t date_cast<file_time_t>(const file_time_t& _Date)
{
	return _Date;
}

template<> date date_cast<date>(const file_time_t& _Date)
{
	ntp_date d = date_cast<ntp_date>(_Date);
	return date_cast<date>(d);
}

// _____________________________________________________________________________
// to FILETIME
#ifdef _MSC_VER
	template<> ntp_timestamp date_cast<ntp_timestamp>(const FILETIME& _Date) { return date_cast<ntp_timestamp>((const file_time_t&)_Date); }
	template<> ntp_date date_cast<ntp_date>(const FILETIME& _Date) { return date_cast<ntp_date>((const file_time_t&)_Date); }
	template<> time_t date_cast<time_t>(const FILETIME& _Date) { return date_cast<time_t>((const file_time_t&)_Date); }
	template<> file_time_t date_cast<file_time_t>(const FILETIME& _Date) { return date_cast<file_time_t>((const file_time_t&)_Date); }
	template<> date date_cast<date>(const FILETIME& _Date) { return date_cast<date>((const file_time_t&)_Date); }

	template<> FILETIME date_cast<FILETIME>(const date& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
	template<> FILETIME date_cast<FILETIME>(const ntp_date& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
	template<> FILETIME date_cast<FILETIME>(const ntp_timestamp& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
	template<> FILETIME date_cast<FILETIME>(const ntp_time& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
	template<> FILETIME date_cast<FILETIME>(const time_t& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
	template<> FILETIME date_cast<FILETIME>(const file_time_t& _Date) { file_time_t t = date_cast<file_time_t>(_Date); return *(FILETIME*)&t; }
#endif

// Returns the number of seconds to add to a UTC time to get the time in the 
// current locale's timezone.
static int timezone_offset()
{
	time_t now = time(nullptr);
	tm utc;
	gmtime_s(&utc, &now);
	// get whether we're locally in daylight savings time
	tm local;
	localtime_s(&local, &now);
	bool IsDaylightSavings = !!local.tm_isdst;
	time_t utctimeInterpretedAsLocaltime = mktime(&utc);
	return static_cast<int>(now - utctimeInterpretedAsLocaltime) + (IsDaylightSavings ? kNumSecondsPerHour : 0);
}

size_t strftime(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, const ntp_date& _Date, date_conversion::value _Conversion)
{
	ntp_date DateCopy(_Date);

	int TimezoneOffsetSeconds = timezone_offset();

	if (_Conversion == date_conversion::to_local)
		DateCopy.DataMS += TimezoneOffsetSeconds;

	date d = date_cast<date>(DateCopy); // clean up any overrun values
	fractional_second64 fractional(_Date.DataLS);

	*_StrDestination = 0;
	char* w = _StrDestination;
	char* wend = _StrDestination + _SizeofStrDestination;
	const char* f = _Format;
	oStd::mstring s;
	size_t len = 0;

#define ADD_CHECK() do \
	{	if (!len) \
	return 0; \
	w += len; \
	} while(false)

#define ADDS() do \
	{	len = snprintf(w, std::distance(w, wend), "%s", s.c_str()); \
	ADD_CHECK(); \
	} while(false)

#define ADDSTR(_String) do \
	{	len = snprintf(w, std::distance(w, wend), (_String)); \
	ADD_CHECK(); \
	} while(false)

#define ADDDIG(n) do \
	{	len = snprintf(w, std::distance(w, wend), "%u", (n)); \
	ADD_CHECK(); \
	} while(false)

#define ADD2DIG(n) do \
	{	len = snprintf(w, std::distance(w, wend), "%02u", (n)); \
	ADD_CHECK(); \
	} while(false)

#define ADDDATE(_Format) do \
	{	len = strftime(w, std::distance(w, wend), _Format, _Date, _Conversion); \
	ADD_CHECK(); \
	} while(false)

#define ADDDUR(durationT) do \
	{ durationT d = duration_cast<durationT>(fractional); \
	len = snprintf(w, std::distance(w, wend), "%llu", d.count()); \
	ADD_CHECK(); \
	} while(false)

#define ADDDUR0(pad, durationT) do \
	{ durationT d = duration_cast<durationT>(fractional); \
	len = snprintf(w, std::distance(w, wend), "%0" #pad "llu", d.count()); \
	ADD_CHECK(); \
	} while(false)

	while (*f)
	{
		if (*f == '%')
		{
			f++;
			switch (*f)
			{
			case 'a': s = as_string(d.day_of_week); s[3] = 0; ADDS(); break;
			case 'A': ADDSTR(as_string(d.day_of_week)); break;
			case 'b': s = as_string(d.month); s[3] = 0; ADDS(); break;
			case 'B': ADDSTR(as_string(d.month)); break;
			case 'c': ADDDATE("%a %b %d %H:%M:%S %Y"); break;
			case 'd': ADD2DIG(d.day); break;
			case 'H': ADD2DIG(d.hour); break;
			case 'I': ADD2DIG(d.hour % 12); break;
			case 'j': oASSERT(false, "Day of year not yet supported"); break;
			case 'm': ADD2DIG(d.month); break;
			case 'M': ADD2DIG(d.minute); break;
			case 'o': 
				{
					int Hours = TimezoneOffsetSeconds / 3600;
					int Minutes = (abs(TimezoneOffsetSeconds) / 60) - abs(Hours * 60);
					len = snprintf(w, std::distance(w, wend), "%+d:%02d", Hours, Minutes);
					ADD_CHECK();
					break;
				}

			case 'p': ADDSTR(d.hour < 12 ? "AM" : "PM"); break;
			case 'S': ADD2DIG(d.second); break;
			case 'U': oASSERT(false, "Week of year not yet supported"); break;
			case 'w': ADDDIG(d.day_of_week); break;
			case 'W': oASSERT(false, "Week of year not yet supported"); break;
			case 'x': ADDDATE("%m/%d/%y"); break;
			case 'X': ADDDATE("%H:%M:%S"); break;
			case 'y': ADD2DIG(d.year % 100); break;
			case 'Y':
				{
					if (d.year >= 1)
						ADDDIG(d.year);
					else
					{
						ADDDIG(abs(--d.year));
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
					len = ::strftime(_StrDestination, std::distance(w, wend), "%Z", &TM);
					ADD_CHECK();
					break;
				}

			case '%': *w++ = *f++; break;
				// Non-standard fractional seconds
			case 's': ADDDUR(milliseconds); break;
			case 'u': ADDDUR(microseconds); break;
			case 'P': ADDDUR(picoseconds); break;
			case 't': ADDDUR(attoseconds); break;

			case '0':
				{
					f++;
					switch (*f)
					{
						case 's': ADDDUR0(3, milliseconds); break;
						case 'u': ADDDUR0(6, microseconds); break;
						default:
							oTHROW(protocol_error, "not supported: formatting token %%0%c", *f);
					}

					break;
				}

			default:
				oTHROW(protocol_error, "not supported: formatting token %%%c", *f);
			}

			f++;
		}

		else
			*w++ = *f++;
	}

	*w = 0;
	return std::distance(_StrDestination, w);
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const date& _Date)
{
	return strftime(_StrDestination, _SizeofStrDestination, sortable_date_format, date_cast<ntp_date>(_Date)) ? _StrDestination : nullptr;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const weekday::value& _Weekday)
{
	return strlcpy(_StrDestination, as_string(_Weekday), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const month::value& _Month)
{
	return strlcpy(_StrDestination, as_string(_Month), _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
}

bool from_string(weekday::value* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < 7; i++)
	{
		const char* s = as_string(weekday::value(i));
		if (!_stricmp(_StrSource, s) || (!_memicmp(_StrSource, s, 3) && _StrSource[3] == 0))
		{
			*_pValue = weekday::value(i);
			return true;
		}
	}
	return false;
}

bool from_string(month::value* _pValue, const char* _StrSource)
{
	for (size_t i = 0; i < 12; i++)
	{
		const char* s = as_string(month::value(i));
		if (!_stricmp(_StrSource, s) || (!_memicmp(_StrSource, s, 3) && _StrSource[3] == 0))
		{
			*_pValue = month::value(i);
			return true;
		}
	}
	return false;
}

} // namespace oStd
