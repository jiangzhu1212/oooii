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
// A simple timer for down-and-dirty code timing.
#pragma once
#ifndef oStd_timer_h
#define oStd_timer_h

#include <oStd/assert.h>
#include <oStd/oStdChrono.h>

namespace oStd {

class timer
{
	// Encapsulates marking a starting point and using time since that 
	// starting point.
public:
	timer() { reset(); }
	inline void reset() { Start = now(); }
	inline double seconds() const { return now() - Start; }
	inline double milliseconds() const { return (now() - Start) * 1000.0; }
private:
	double Start;
	double now() const { return oStd::chrono::high_resolution_clock::now().time_since_epoch().count(); }
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

} // namespace oStd

#endif
