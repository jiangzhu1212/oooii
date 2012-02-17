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
#pragma once
#ifndef oBasisTestCommon_h
#define oBasisTestCommon_h

#include <oBasis/oError.h>

#ifdef o32BIT
	// @oooii-tony: I think tbb is registering as leaks out of this test because
	// they're not reported when a serial execution (no exercise of tbb) occurs and
	// this allocator just allocates one big arena from malloc, so it shouldn't be
	// several smaller allocs, whereas TBB might behave that way.
	#define oBug_1938
	#define oBug_1938_EXIT() do { if (1) {return oErrorSetLast(oERROR_LEAKS, "oBug_1938: Concurrency test is done serially to avoid reports of a leak that is probably from TBB. Thus this does not confirm concurrency works."); } } while(0)
#else
	#define oBug_1938_EXIT() do {} while(0)
#endif

#define oTESTB0(test) do { if (!(test)) return false; } while(false) // pass through error
#define oTESTB(test, msg, ...) do { if (!(test)) return oErrorSetLast(oERROR_GENERIC, msg, ## __VA_ARGS__); } while(false)

#endif
