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
#include <oBasis/oNTPTime.h>
#include <oBasis/oError.h>

static const unsigned long long SEC_FROM_1900_TO_1970 = (365ull * 70 + 17) * 24 * 60 * 60;

bool oNTPTimeFromPosix(time_t _PosixTime, oNTPTime* _pNTPTime)
{
	// Posix time is # seconds since Jan 1, 1970 NTP time is # seconds since 
	// Jan 1, 1900 so take the difference. (17 leap years)
	unsigned long long t = _PosixTime + SEC_FROM_1900_TO_1970;
	if (t < static_cast<unsigned long long>(_PosixTime))
		return oErrorSetLast(oERROR_AT_CAPACITY, "Time rollover occurred when converting from Posix to NTP time");
	*_pNTPTime = t;
	return true;
}

bool oNTPTimeToPosix(oNTPTime _NTPTime, time_t* _pPosixTime)
{
	if (_NTPTime >= SEC_FROM_1900_TO_1970)
	{
		*_pPosixTime = _NTPTime - SEC_FROM_1900_TO_1970;
		return true;
	}

	return oErrorSetLast(oERROR_AT_CAPACITY, "The specified NTPTime is before the Posix (time_t) epoch, so thus the returned time would be invalid");
}
