// $(header)
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
