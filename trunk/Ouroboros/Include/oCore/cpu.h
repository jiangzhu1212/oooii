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
// Utility for querying the Central Processing Unit hardware of the current 
// computer.
#pragma once
#ifndef oCore_cpu_h
#define oCore_cpu_h

#include <oBase/fixed_string.h>
#include <functional>

namespace ouro {
	namespace cpu { 

/* enum class */ namespace type
{	enum value {

	unknown,
	x86,
	x64,
	ia64,
	arm,

};}

/* enum class */ namespace support
{	enum value {

	none, // the feature does not exist
	not_found, // the feature is not exposed
	hardware_only, // no platform support/API exposure
	full, // both the current platform and HW support the feature

};}

struct cache_info
{
	unsigned int size;
	unsigned int line_size;
	unsigned int associativity;
};

struct info
{
	type::value type;
	int processor_count;
	int processor_package_count;
	int hardware_thread_count;
	cache_info data_cache[3];
	cache_info instruction_cache[3];
	ouro::sstring string;
	ouro::sstring brand_string;
};

// Returns a description of the current CPU
info get_info();

// Enumerates all cpu features. Return true to continue enumeration, false to 
// exit early.
void enumerate_features(const std::function<bool(const char* _FeatureName, const support::value& _Support)>& _Enumerator);

	} // namespace cpu
} // namespace ouro

#endif
