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
// This is an abstraction for the implementations required to run unit tests.
// Libraries in Ouroboros are broken up into dependencies on each other and on 
// system resources. For example oStd is dependent on the absence of C++11 
// support, oBase is dependent on C++11/compiler features and oCore is dependent
// on non-standard operating system API. To be able to test a library sometimes
// requires extra features not available directly to the library, so to keep 
// each library isolated to the point it can be used in a different library 
// without other higher-level Ouroboros libraries, expose an abstract interface
// for enabling the unit tests - it would be up to whatever other suite to 
// implement these interfaces.
#pragma once
#ifndef Ouroboros_test_services_h
#define Ouroboros_test_services_h

#include <cstdarg>
#include <memory>

namespace ouro {

namespace surface { class buffer; }

class test_services
{
public:

	// Generate a random number. The seed is often configurable from the test
	// infrastructure so behavior can be reproduced.
	virtual int rand() = 0;
	
	// Write to the test infrastructure's TTY
	virtual void vreport(const char* _Format, va_list _Args) = 0;
	inline void report(const char* _Format, ...) { va_list a; va_start(a, _Format); vreport(_Format, a); va_end(a); }

	// Load the entire contents of the specified file into a newly allocated 
	// buffer.
	virtual std::shared_ptr<char> load_buffer(const char* _Path, size_t* _pSize = nullptr) = 0;

	virtual bool is_debugger_attached() const = 0;

	virtual size_t total_physical_memory() const = 0;

	// Returns the average and peek percent usage of the CPU by this process
	virtual void get_cpu_utilization(float* _pAverage, float* _pPeek) = 0;

	// Resets the frame of recording average and peek percentage CPU utilization
	virtual void reset_cpu_utilization() = 0;
	
	// This function compares the specified surface to a golden image named after
	// the test's name suffixed with _NthTest. If _NthTest is 0 then the golden 
	// image should not have a suffix. If _MaxRMSError is negative a default 
	// should be used. If the surfaces are not similar this throws an exception.
	virtual void check(const surface::buffer* _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f) = 0;

	inline void check(std::shared_ptr<surface::buffer>& _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{ check(_pBuffer.get(), _NthTest, _MaxRMSError); }

	inline void check(std::shared_ptr<const surface::buffer>& _pBuffer
		, int _NthTest = 0
		, float _MaxRMSError = -1.0f)
	{ check(_pBuffer.get(), _NthTest, _MaxRMSError); }
};

} // namespace ouro

#endif
