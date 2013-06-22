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
// std::chrono is often too obtuse to provide a significant ROI to used directly 
// so here is oTimer, some syntactic sugar on the 99% usage case we seem to have
// in the codebase.
#pragma once
#ifndef oTimer_h
#define oTimer_h

#include <oStd/assert.h>
#include <oStd/oStdChrono.h>
#include <oBasis/oInvalid.h>


// returns system ticks as seconds since the std (Unix) epoch.
inline double oTimer() { return oStd::chrono::high_resolution_clock::now().time_since_epoch().count(); }

// oTimer, but in in milliseconds (double, float and int)
inline double oTimerMSD() { return oTimer()*1000.0; }
inline float oTimerMSF() { return (float)oTimer()*1000.0f; }
inline unsigned int oTimerMS() { return static_cast<unsigned int>(oTimerMSF()); }

// _____________________________________________________________________________
// Generic inline utilities for making common timing problems more 
// self-documenting. These use the above API.

class oLocalTimeout
{
	// For those times a loop should only continue for a limited time, favor using
	// this class as it is more self-documenting and more centralized than just
	// doing the math directly.
	double End;
public:
	oLocalTimeout(double _Timeout) { Reset(_Timeout); }
	oLocalTimeout(unsigned int _TimeoutMS) { Reset(_TimeoutMS / 1000.0); }
	inline void Reset(double _Timeout) { End = oTimer() + _Timeout; }
	inline void Reset(unsigned int _TimeoutMS) { Reset(_TimeoutMS / 1000.0); }
	inline bool TimedOut() const { return oTimer() >= End; }
};

class oScopedPartialTimeout
{
	// Sometimes it is necessary to call a system function that takes a timeout
	// value in a loop. So from interface/user space we'd like the calling function
	// to respect the timeout we gave, even if the lower-level code does something
	// less elegant. To encapsulate this common pattern, here is a scoped timeout
	// object that decrements a timeout value as code within it's scope executes.
	// Often usage of this object implies careful application of {} braces, which 
	// is still cleaner code than maintaining timer calls.
	// NOTE: If your loop may not take much time to execute, consider creating the
	// instance of this class outside the loop and calling UpdateTimeout.

public:
	// Pointer to a timeout value to update. The value should be initialized to 
	// the user-specified total timeout initially and then allowed to be updated 
	// in a loop using UpdateTimeout(). If the value of the timeout is 
	// oInfiniteWait, then this class doesn't update the value thus allowing the 
	// oInfiniteWait value to be propagated.
	oScopedPartialTimeout(unsigned int* _pTimeoutMSCountdown);
	~oScopedPartialTimeout();

	// Call this in a loop that is doing other work to decrement the value pointed
	// to by pTimeoutMSCountdown.
	void UpdateTimeout();

protected:
	unsigned int* pTimeoutMSCountdown;
	unsigned int Start; //needs to be in ms or you can have some serious precision issues causing the timer to never advance. i.e. if each iteration of the loop is less than 0.5ms
};

#endif
