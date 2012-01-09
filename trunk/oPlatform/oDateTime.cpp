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
#include <oPlatform/oDateTime.h>
#include <oBasis/oError.h>
#include <oPlatform/oWindows.h>

int oCompareDateTime(const oDateTime& _DateTime1, const oDateTime& _DateTime2)
{
	time_t time1 = oConvertDateTime(_DateTime1);
	time_t time2 = oConvertDateTime(_DateTime2);
	if (time1 == time2)
		return _DateTime1.Milliseconds > _DateTime2.Milliseconds ? 1 : (_DateTime1.Milliseconds < _DateTime2.Milliseconds) ? -1 : 0;
	else return time1 > time2 ? 1 : -1;
}

bool oGetDateTime(oDateTime* _pDateTime)
{
	if (!_pDateTime)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "A valid address to receive an oDateTime must be specified.");
	
	SYSTEMTIME st;
	GetSystemTime(&st);

	static_assert(sizeof(SYSTEMTIME) == sizeof(oDateTime), "");
	memcpy(_pDateTime, &st, sizeof(st));
	return true;
}

bool oGetLocalDateTime(const oDateTime& _UTCTime, oDateTime* _pLocalTime)
{
	if (!_pLocalTime)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "A valid address to receive an oDateTime must be specified.");

	if (!SystemTimeToTzSpecificLocalTime(nullptr, (const LPSYSTEMTIME)&_UTCTime, (LPSYSTEMTIME)_pLocalTime))
		return oWinSetLastError();

	return true;
}

time_t oConvertDateTime(const oDateTime& _DateTime)
{
	return oSystemTimeToUnixTime((const SYSTEMTIME*)&_DateTime);
}

void oConvertDateTime(oDateTime* _DateTime, time_t _Time)
{
	SYSTEMTIME st;
	oUnixTimeToSystemTime(_Time, &st);
	oV(SystemTimeToTzSpecificLocalTime(0, &st, (SYSTEMTIME*)_DateTime));
}

static const char* TIME_STRING_FORMAT = "%04hu/%02hu/%02hu %02hu:%02hu:%02hu:%03hu";

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oDateTime& _Value)
{
	return -1 == sprintf_s(_StrDestination, _SizeofStrDestination, TIME_STRING_FORMAT, _Value.Year, _Value.Month, _Value.Day, _Value.Hour, _Value.Minute, _Value.Second, _Value.Milliseconds) ? _StrDestination : nullptr;
}

bool oFromString(oDateTime* _pValue, const char* _StrSource)
{
	return 7 == sscanf_s(_StrSource, TIME_STRING_FORMAT, &_pValue->Year, &_pValue->Month, &_pValue->Day, &_pValue->Hour, &_pValue->Minute, &_pValue->Second, &_pValue->Milliseconds);
}
