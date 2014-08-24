/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// A simple timer for down-and-dirty code timing.
#pragma once
#ifndef oBase_timer_h
#define oBase_timer_h

#include <oBase/assert.h>
#include <chrono>

namespace ouro {

class timer
{
	// Encapsulates marking a starting point and using time since that 
	// starting point. Statics can be used to wrap lengthy std api.
public:
	template<typename T, typename Ratio> static T nowT() { return std::chrono::duration_cast<std::chrono::duration<T, Ratio>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }
	template<typename T> static T nowT() { return std::chrono::duration_cast<std::chrono::duration<T, std::ratio<1>>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }

	static inline long long nowus() { return nowT<long long, std::micro>(); }
	static inline double now() { return nowT<double>(); }
	static inline float nowf() { return nowT<float>(); }
	static inline double nowms() { return nowT<double, std::milli>(); }
	static inline float nowmsf() { return nowT<float, std::milli>(); }
	static inline unsigned int nowmsi() { return nowT<unsigned int, std::milli>(); }

	timer() { reset(); }
	inline void reset() { Start = now(); }
	inline double seconds() const { return now() - Start; }
	inline double milliseconds() const { return (now() - Start) * 1000.0; }
private:
	double Start;
};

class local_timeout
{
	// For those times a loop should only continue for a limited time, favor using
	// this class as it is more self-documenting and more centralized than just
	// doing the math directly.
	double End;
public:
	local_timeout(double _Timeout) { reset(_Timeout); }
	local_timeout(unsigned int _TimeoutMS) { reset(_TimeoutMS / 1000.0); }
	inline void reset(double _Timeout) { End = timer::now() + _Timeout; }
	inline void reset(unsigned int _TimeoutMS) { reset(_TimeoutMS / 1000.0); }
	inline bool timed_out() const { return timer::now() >= End; }
};

class scoped_timer : public timer
{
	// A super-simple benchmarking tool to report to stdout the time since this 
	// timer was last reset. This traces in the dtor, so scoping can be used to 
	// group the benchmarking.

public:
	scoped_timer(const char* _StringLiteralName) : Name(_StringLiteralName) {}
	~scoped_timer() { trace(); }
	
	inline void trace() { oTRACEA("%s took %.03f sec", Name ? Name : "(null)", seconds()); }

private:
	const char* Name;
};

}

#endif
