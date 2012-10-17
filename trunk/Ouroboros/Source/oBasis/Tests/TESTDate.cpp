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
#include <oBasis/oDate.h>
#include <oBasis/oMath.h>
#include <oBasisTests/oBasisTests.h>
#include "oBasisTestCommon.h"

struct oNTPDATE_TEST
{
	oDATE Date;
	long long JulianDay;
	long long ModifiedJulianDay;
	long long NTPDate;
	int NTPEra;
	unsigned int NTPTimestamp;
	time_t UnixTime;
	oFILETIME FileTime;
	const char* Epoch;
	const char* TimeFormat;
	const char* ExpectedTimeString;
};

struct oDATE_TEST
{
	oDATE Date;
	int dummy;
};

// http://www.eecis.udel.edu/~mills/database/reports/ntp4/ntp4.pdf
// FILETIMEs from http://www.silisoftware.com/tools/date.php
// Table 2. Interesting Historic NTP Dates
static const oNTPDATE_TEST sNTPTests[] = 
{
	{ oDATE(-4712, oJANUARY, 1), 0, -2400001, -208657814400, -49, 1795583104, 0, 0, "First day Julian Era (start of day, epoch is at noon)", "%c", "Sun Jan 01 00:00:00 4713 BCE" },
	{ oDATE(-1, oJANUARY, 1), 1720695, -679306, -59989766400, -14, 139775744, 0, 0, "2 BCE", "%c", "Mon Jan 01 00:00:00 2 BCE" },
	{ oDATE(0, oJANUARY, 1), 1721060, -678941, -59958230400, -14, 171311744, 0, 0, "1 BCE", "%c", "Sun Jan 01 00:00:00 1 BCE" },
	{ oDATE(1, oJANUARY, 1), 1721426, -678575, -59926608000, -14, 202934144, 0, 0, "1 CE", "%c", "Mon Jan 01 00:00:00 1" },
	{ oDATE(1452, oSEPTEMBER, 30), 2251665, -148327 - 9, -14113958400, -4, 3065910784, 0, 0, "First publishing of Guttenberg Bible", "%c", "Thu Sep 30 00:00:00 1452" }, // the minus 9 is for the same reason as HACK_ADJUSTMENT
	{ oDATE(1492, oAUGUST, 3), 2266217, -133775 - 9, -12856665600, -3, 28236288, 0, 0, "Columbus sailed the ocean blue", "%c", "Wed Aug 03 00:00:00 1492" }, // the minus 9 is for the same reason as HACK_ADJUSTMENT
	{ oDATE_JULIAN_EPOCH_END, 2299150, -100851, -10011254400, -3, 2873647488, 0, 0, "Last day of Julian Calendar", "%c", "Mon Oct 04 00:00:00 1582" },
	{ oDATE_GREGORIAN_EPOCH_START, 2299161, -100840, -10010304000, -3, 2874597888, 0, 0, "First day Gregorian Calendar", "%c", "Fri Oct 15 00:00:00 1582" },
	{ oDATE_FILE_TIME_EPOCH_START, 2305814, -94187, -9435484800, -3, 3449417088, 0, 0, "First day FILETIME", "%c", "Mon Jan 01 00:00:00 1601" },
	{ oDATE(1899, oDECEMBER, 31), 2415020, 15019, -86400, -1, 4294880896, 0, 94353984000000000, "Last day NTP Era -1", "%c", "Sun Dec 31 00:00:00 1899" },
	{ oDATE_NTP_EPOCH_START, 2415021, 15020, 0, 0, 0, 0, 94354848000000000, "First day NTP Era 0", "%c", "Mon Jan 01 00:00:00 1900" },
	{ oDATE_UNIX_TIME_EPOCH_START, 2440588, 40587, 2208988800, 0, 2208988800, 0, 116444736000000000, "First day Unix", "%c", "Thu Jan 01 00:00:00 1970" },
	{ oDATE(1972, oJANUARY, 1), 2441318, 41317, 2272060800, 0, 2272060800, 63072000, 117075456000000000, "First day UTC", "%c", "Sat Jan 01 00:00:00 1972" },
	{ oDATE(1999, oDECEMBER, 31), 2451544, 51543, 3155587200, 0, 3155587200, 946598400, 125910720000000000, "Last day 20th century", "%c", "Fri Dec 31 00:00:00 1999" },
	{ oDATE(2000, oJANUARY, 1), 2451545, 51544, 3155673600, 0, 3155673600, 946684800, 125911584000000000, "First day 21st century", "%c", "Sat Jan 01 00:00:00 2000" },
	{ oDATE(2001, oSEPTEMBER, 9, 1, 46, 40), 2452162, 52161, 3208988800, 0, 3208988800, 1000000000, 126444736000000000, "1 Bil Unix Time", "%c", "Sun Sep 09 01:46:40 2001" },
	{ oDATE(2036, oFEBRUARY, 7), 2464731, 64730, 4294944000, 0, 4294944000, 2085955200, 137304288000000000, "Last day NTP Era 0", "%c", "Thu Feb 07 00:00:00 2036" },
	{ oDATE_NTP_EPOCH_END, 2464731, 64730, 4294967294, 0, 4294967294, 2085978494, 137304520940000000, "Last moment NTP Era 0", "%c", "Thu Feb 07 06:28:14 2036" },
	{ oDATE(2036, oFEBRUARY, 8), 2464732, 64731, 4295030400, 1, 63104, 2086041600, 137305152000000000, "First day NTP Era 1", "%c", "Fri Feb 08 00:00:00 2036" },
	{ oDATE_UNIX_TIME_EPOCH_END_SIGNED32, 2465443, 65442, 4356472447, 1, 61505151, 2147483647, 137919572470000000, "Last day signed 32-bit Unix", "%c", "Tue Jan 19 03:14:07 2038" },
	{ oDATE_UNIX_TIME_EPOCH_END_UNSIGNED32, 2490298, 90297, 6503956095, 1, 2208988799, 4294967296, 159394408950000000, "Last day unsigned 32-bit Unix", "%c", "Sun Feb 07 06:28:15 2106" },
	{ oDATE(2172, oMARCH, 16), 2514442, 114441, 8589974400, 2, 39808, 6380985600, 180254592000000000, "First day NTP Era 2", "%c", "Mon Mar 16 00:00:00 2172" },
	{ oDATE(2500, oJANUARY, 1), 2634167, 234166, 18934214400, 4, 1754345216, 0, 283696992000000000, "2500 CE", "%c", "Fri Jan 01 00:00:00 2500" },
	{ oDATE(3000, oJANUARY, 1), 2816788, 416787, 34712668800, 8, 352930432, 0, 441481536000000000, "3000 CE", "%c", "Wed Jan 01 00:00:00 3000" },
	{ oDATE(2005, oMAY, 24), 2453515, 53514, 3325881600, 0, 3325881600, 1116892800, 127613664000000000, "http://www.mathworks.com/help/toolbox/aerotbx/ug/mjuliandate.html", "%c", "Tue May 24 00:00:00 2005" },
	{ oDATE(2006, oDECEMBER, 19), 2454089, 54088, 3375475200, 0, 3375475200, 1166486400, 128109600000000000, "http://www.mathworks.com/help/toolbox/aerotbx/ug/mjuliandate.html", "%c", "Tue Dec 19 00:00:00 2006" },
	{ oDATE(2004, oOCTOBER, 10, 12, 21), 2453289, 53288, 3306399660, 0, 3306399660, 1097410860, 127418844600000000, "http://www.mathworks.com/help/toolbox/aerotbx/ug/mjuliandate.html", "%c", "Sun Oct 10 12:21:00 2004" },
	{ oDATE(2012, oAUGUST, 15, 15, 12, 58), 2456155, 56154, 3554032378, 0, 3554032378, 1345043578, 129895171780000000, "Day this test was written", "%c", "Wed Aug 15 15:12:58 2012" },
	{ oDATE(2012, oAUGUST, 12, 23, 21, 36), 2456152, 56151, 3553802496, 0, 3553802496, 1344813696, 129892872960000000, "http://en.wikipedia.org/wiki/Unix_time", "%c", "Sun Aug 12 23:21:36 2012" },
};

static bool oTESTDateFail(const char* _DateName, const char* _DateType, long long _ExpectedValue, long long _CalculatedValue)
{
	return oErrorSetLast(oERROR_GENERIC, "%s off by %+lld for %s", oSAFESTRN(_DateType), _ExpectedValue-_CalculatedValue, oSAFESTRN(_DateName));
}

static bool oTESTDateFail(const char* _DateName, const char* _DateType, const oDATE& _ExpectedValue, const oDATE& _CalculatedValue)
{
	return oErrorSetLast(oERROR_GENERIC, "Conversion back to oDATE failed for %s", oSAFESTRN(_DateName));
}

#define oTESTDATE(_Test, _LLExpected, _LLCalculated) do \
	{	if (_Test._LLExpected != _LLCalculated) \
		return oTESTDateFail(_Test.ExpectedTimeString, #_LLExpected, _Test._LLExpected, _LLCalculated); \
	} while(false)

#define oTESTDATECONVERT(_Test, src, pdst) do \
	{	if (!oDateConvert(src, pdst)) \
		return oErrorSetLast(oERROR_GENERIC, "oDateConvert(%s, %s) failed for %s", #src, #pdst, oSAFESTRN(_Test.ExpectedTimeString)); \
	} while(false)

#define oTESTDATENOCONVERT(_Test, src, pdst) do \
{	if (oDateConvert(src, pdst)) \
		return oErrorSetLast(oERROR_GENERIC, "oDateConvert(%s, %s) succeeded for invalid date %s", #src, #pdst, oSAFESTRN(_Test.ExpectedTimeString)); \
} while(false)

#define oTESTDATEMILLI(_Test, _IExpected, _ICalculated) do \
{	if (_IExpected != _ICalculated) \
		return oErrorSetLast(oERROR_GENERIC, "Fractional sec should have been %dms for %s", _IExpected, oSAFESTRN(_Test.ExpectedTimeString)); \
} while(false)

bool oBasisTest_oDate_Julian()
{
	for (size_t i = 0; i < oCOUNTOF(sNTPTests); i++)
	{
		const oNTPDATE_TEST& T = sNTPTests[i];
		
		long long JDN = oDateCalcJulianDayNumber(T.Date);
		oTESTDATE(T, JulianDay, JDN);

		long long MJD = static_cast<long long>(floor(oDateCalcModifiedJulian(T.Date)));
		oTESTDATE(T, ModifiedJulianDay, MJD);
	}

	return true;
}

bool oBasisTest_oDate_UnixTime()
{
	for (size_t i = 0; i < oCOUNTOF(sNTPTests); i++)
	{
		const oNTPDATE_TEST& T = sNTPTests[i];
		if (T.Date >= oDATE_UNIX_TIME_EPOCH_START && T.Date <= oDATE_UNIX_TIME_EPOCH_END_SIGNED32)
		{
			oNTPDate NTPDate;
			oTESTDATECONVERT(T, T.UnixTime, &NTPDate);

			int era = oDateGetNTPEra(NTPDate);
			oTESTDATE(T, NTPEra, era);

			unsigned int timestamp = oDateGetNTPTimestamp(NTPDate);
			oTESTDATE(T, NTPTimestamp, timestamp);

			if (T.NTPEra == 0)
			{
				oNTPTimestamp NTPTimestamp;
				oTESTDATECONVERT(T, T.UnixTime, &NTPTimestamp);
				oTESTDATE(T, NTPTimestamp, oDateGetNTPTimestamp(NTPTimestamp));
			}

			oFILETIME ft;
			oTESTDATECONVERT(T, T.UnixTime, &ft);
			oTESTDATE(T, FileTime, ft);

			oDATE Date;
			oTESTDATECONVERT(T, T.Date, &Date);
			oTESTDATE(T, Date, Date);
		}
	}

	return true;
}

bool oBasisTest_oDate_FileTime()
{
	for (size_t i = 0; i < oCOUNTOF(sNTPTests); i++)
	{
		const oNTPDATE_TEST& T = sNTPTests[i];
		if (T.Date >= oDATE_FILE_TIME_EPOCH_START)
		{
			oNTPDate NTPDate;
			oTESTDATECONVERT(T, T.FileTime, &NTPDate);

			int era = oDateGetNTPEra(NTPDate);
			oTESTDATE(T, NTPEra, era);

			unsigned int timestamp = oDateGetNTPTimestamp(NTPDate);
			oTESTDATE(T, NTPTimestamp, timestamp);

			if (T.NTPEra == 0)
			{
				oNTPTimestamp NTPTimestamp;
				oTESTDATECONVERT(T, T.FileTime, &NTPTimestamp);
				oTESTDATE(T, NTPTimestamp, oDateGetNTPTimestamp(NTPTimestamp));
			}

			if (T.Date >= oDATE_UNIX_TIME_EPOCH_START && T.Date <= oDATE_UNIX_TIME_EPOCH_END_SIGNED32)
			{
				time_t t;
				oTESTDATECONVERT(T, T.FileTime, &t);
				oTESTDATE(T, UnixTime, t);
			}

			oDATE Date;
			oTESTDATECONVERT(T, T.Date, &Date);
			oTESTDATE(T, Date, Date);
		}
	}

	return true;
}

static bool oTestNTPTimestamp(const oNTPDATE_TEST& _Test, int _ExpectedMilliseconds, const oNTPTimestamp& _NTPTimestamp)
{
	unsigned int timestamp = oDateGetNTPTimestamp(_NTPTimestamp);
	oTESTDATE(_Test, NTPTimestamp, timestamp);
	
	int MS = static_cast<int>(round(oDateGetNTPSecondFraction(_NTPTimestamp) * 1000));
	oTESTDATEMILLI(_Test, _ExpectedMilliseconds, MS);
	return true;
}

static bool oBasisTest_oDate_NTP(const oBasisTestServices& _Services)
{
	for (size_t i = 0; i < oCOUNTOF(sNTPTests); i++)
	{
		const oNTPDATE_TEST& T = sNTPTests[i];
		oDATE d = T.Date;
		d.Millisecond = _Services.Rand() % 1000;
		
		oNTPDate NTPDateNoMS;
		oTESTDATECONVERT(T, T.Date, &NTPDateNoMS);

		oNTPDate NTPDate;
		oTESTDATECONVERT(T, d, &NTPDate);

		int era = oDateGetNTPEra(NTPDateNoMS);
		oTESTDATE(T, NTPEra, era);
		 
		unsigned int timestamp = oDateGetNTPTimestamp(NTPDateNoMS);
		oTESTDATE(T, NTPTimestamp, timestamp);
		
		int MS = static_cast<int>(round(oDateGetNTPSecondFraction(NTPDate) * 1000));
		oTESTDATEMILLI(T, d.Millisecond, MS);

		long long NTPDatePart = oDateGetNTPDate(NTPDateNoMS);
		oTESTDATE(T, NTPDate, NTPDatePart);

		oNTPTimestamp NTPTimestamp = 0;
		if (T.NTPEra == 0)
		{
			oTESTDATECONVERT(T, d, &NTPTimestamp);
			if (!oTestNTPTimestamp(T, d.Millisecond, NTPTimestamp))
				return false; // pass through error
		}
		else 
			oTESTDATENOCONVERT(T, d, &NTPTimestamp);

		NTPTimestamp = 0;
		if (T.NTPEra == 0)
		{
			oTESTDATECONVERT(T, NTPDate, &NTPTimestamp);
			if (!oTestNTPTimestamp(T, d.Millisecond, NTPTimestamp))
				return false; // pass through error
		}
		else 
			oTESTDATENOCONVERT(T, d, &NTPTimestamp);

		oDATE TestDate;
		oTESTDATECONVERT(T, NTPDateNoMS, &TestDate);
		oTESTDATE(T, Date, TestDate);

		if (T.NTPEra == 0)
		{
			memset(&TestDate, 0, sizeof(TestDate));
			oTESTDATECONVERT(T, T.Date, &NTPTimestamp);
			oTESTDATECONVERT(T, NTPTimestamp, &TestDate);
			oTESTDATE(T, Date, TestDate);

			oFILETIME ft;
			oTESTDATECONVERT(T, NTPTimestamp, &ft);
			oTESTDATE(T, FileTime, ft);
		}

		time_t t = 0;
		if (T.Date >= oDATE_UNIX_TIME_EPOCH_START && T.Date <= oDATE_UNIX_TIME_EPOCH_END_SIGNED32)
		{
			oTESTDATECONVERT(T, T.Date, &t);
			oTESTDATE(T, UnixTime, t);
		}
		else
			oTESTDATENOCONVERT(T, T.Date, &t);

		oFILETIME ft;
		if (T.Date >= oDATE_FILE_TIME_EPOCH_START)
		{
			oTESTDATECONVERT(T, T.Date, &ft);
			oTESTDATE(T, FileTime, (long long)ft);
			oTESTDATECONVERT(T, NTPDateNoMS, &ft);
			oTESTDATE(T, FileTime, (long long)ft);
		}
		else
		{
			oTESTDATENOCONVERT(T, T.Date, &ft);
			oTESTDATENOCONVERT(T, NTPDateNoMS, &ft);
		}

		oStringM StrDate;
		oDateStrftime(StrDate, T.TimeFormat, NTPDateNoMS);
		
		if (oStrcmp(StrDate, T.ExpectedTimeString))
			return oErrorSetLast(oERROR_GENERIC, "mal-formatted time for %s", oSAFESTRN(T.ExpectedTimeString));
	}

	return true;
}

bool oBasisTest_oDate(const oBasisTestServices& _Services)
{
	oTESTB0(oBasisTest_oDate_Julian());
	oTESTB0(oBasisTest_oDate_NTP(_Services));
	oTESTB0(oBasisTest_oDate_UnixTime());
	oTESTB0(oBasisTest_oDate_FileTime());
	oErrorSetLast(oERROR_NONE, "");
	return true;
}
