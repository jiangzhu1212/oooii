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
#include <oBase/string.h>
#include <stdexcept>
#include <cmath>

#include <oBase/assert.h>

static inline const char* plural(unsigned int n) { return n == 1 ? "" : "s"; }

static int cat_part(char* _StrDestination, size_t _SizeofStrDestination, unsigned int _Amount, const char* _Label, const char* _AbbreviatedLabel, bool _First, bool _Abbreviated)
{
	return _Amount
		? ouro::sncatf(_StrDestination, _SizeofStrDestination, "%s%u%s%s%s"
			, _First ? "" : " "
			, _Amount, _Abbreviated ? "" : " "
			, _Abbreviated ? _AbbreviatedLabel : _Label
			, !_Abbreviated ? plural(_Amount) : "")

		: static_cast<int>(strlen(_StrDestination ? _StrDestination : ""));
}

int ouro::format_duration(char* _StrDestination, size_t _SizeofStrDestination, double _TimeInSeconds, bool _Abbreviated, bool _IncludeMS)
{
	if (_TimeInSeconds != _TimeInSeconds) // check for denorm/inf
		_TimeInSeconds = 0.0;

	if (_TimeInSeconds < 0.0)
		throw std::range_error("negative time (did you do start - end instead of end - start?)");

	int result = 0;

	const static double ONE_MINUTE = 60.0;
	const static double ONE_HOUR = 60.0 * ONE_MINUTE;
	const static double ONE_DAY = 24.0 * ONE_HOUR;
	const static double ONE_WEEK = 7 * ONE_DAY;
	const static double ONE_MONTH = 30.0 * ONE_DAY;
	const static double ONE_YEAR = 365 * ONE_DAY;

	#define SHAVE(_Var, _Amt) \
		unsigned int _Var = static_cast<unsigned int>(_TimeInSeconds / (_Amt)); \
		if (_Var) _TimeInSeconds -= ((_Amt) * (_Var));

	SHAVE(year, ONE_YEAR);
	SHAVE(month, ONE_MONTH);
	SHAVE(week, ONE_WEEK);
	SHAVE(day, ONE_DAY);
	SHAVE(hour, ONE_HOUR);
	SHAVE(minute, ONE_MINUTE);
	SHAVE(second, 1);
	unsigned int millisecond = static_cast<unsigned int>((_TimeInSeconds - floor(_TimeInSeconds)) * 1000.0);

 	int len = 0;
	char* s = _StrDestination;
	size_t n = _SizeofStrDestination;
	*s = 0;

	#define CAT(_Var, _Abbrev) do \
	{	len = cat_part(s, n, _Var, #_Var, _Abbrev, len == 0, _Abbreviated); \
		if (static_cast<size_t>(len) >= _SizeofStrDestination) \
		{	s = nullptr; \
			n = 0; \
		} \
	} while (false)

	CAT(year, "y");
	CAT(month, "mo");
	CAT(week, "w");
	CAT(day, "d");
	CAT(hour, "h");
	CAT(minute, "m");
	CAT(second, "s");
	if (_IncludeMS)
		CAT(millisecond, "ms");

	if (!len)
		len = sncatf(_StrDestination, _SizeofStrDestination, "0%s", _Abbreviated ? "s" : " seconds");

	return len;
}
