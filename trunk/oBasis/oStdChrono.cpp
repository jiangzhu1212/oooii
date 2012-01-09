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
#include <oBasis/oStdChrono.h>
#include <time.h>
#include "oWinHeaders.h"

oStd::chrono::high_resolution_clock::time_point oStd::chrono::high_resolution_clock::now()
{
	LARGE_INTEGER ticks, freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&ticks);
	oStd::chrono::high_resolution_clock::time_point pt;
	*(double*)&pt = ticks.QuadPart / static_cast<double>(freq.QuadPart);
	return pt;
}

oStd::chrono::system_clock::time_point oStd::chrono::system_clock::now()
{
	time_t t = time(nullptr);
	return from_time_t(t);
}

time_t oStd::chrono::system_clock::to_time_t(const time_point& _TimePoint)
{
	return *(time_t*)&_TimePoint;
}

oStd::chrono::system_clock::time_point oStd::chrono::system_clock::from_time_t(time_t _TimePoint)
{
	oStd::chrono::system_clock::time_point pt;
	*(time_t*)&pt = _TimePoint;
	return pt;
}
