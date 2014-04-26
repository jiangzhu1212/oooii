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
// Define some very basic information about memory access for this platform.
#pragma once
#ifndef oBase_memcfg_h
#define oBase_memcfg_h

#ifdef _WIN64
	#define o64BIT 1
#else
	#define o32BIT 1
#endif

#define oCACHE_LINE_SIZE 64

#if o64BIT == 1
	#define oDEFAULT_MEMORY_ALIGNMENT 16
#elif o32BIT == 1
	#define oDEFAULT_MEMORY_ALIGNMENT 4
#else
#error undefined platform
#endif

// helper for implementing move operators where the eviscerated value is typically zero
#define oMOVE0(field) do { field = _That.field; _That.field = 0; } while (false)
#define oMOVE_ATOMIC0(field) do { field = _That.field.exchange(0); } while (false)

namespace ouro {

static const int cache_line_size = oCACHE_LINE_SIZE;
static const int default_alignment = oDEFAULT_MEMORY_ALIGNMENT;

} // namespace ouro

#endif
