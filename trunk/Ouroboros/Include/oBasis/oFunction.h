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
// Convenience shorthands for std::bind and std::function
#pragma once
#ifndef oFunction_h
#define oFunction_h

#include <oBasis/oMacros.h>
#include <functional>

// std::function wrapper/syntactic sugar. Use the tr1 form because we must 
// maintain VC90 compatibility.
#define oFUNCTION std::tr1::function
#define oBIND std::tr1::bind
template<typename T> std::tr1::reference_wrapper<T> oBINDREF__(T& _Value) { return std::tr1::reference_wrapper<T>(_Value); }
#define oBINDREF oBINDREF__ // #defined so it's the same color as other oBIND elements for Visual Assist, et al.
#define oBIND1 std::tr1::placeholders::_1
#define oBIND2 std::tr1::placeholders::_2
#define oBIND3 std::tr1::placeholders::_3
#define oBIND4 std::tr1::placeholders::_4
#define oBIND5 std::tr1::placeholders::_5
#define oBIND6 std::tr1::placeholders::_6
#define oBIND7 std::tr1::placeholders::_7
#define oBIND8 std::tr1::placeholders::_8
#define oBIND9 std::tr1::placeholders::_9

// Type definitions for common functions. These are as much for documentation
// for a recommended coding practices as they are functional. Quite often very
// generic code relies on only a very small amount of platform-specific code, so
// by calling out where this has been observed to be a common practices with 
// these functions below, hopefully the reader will consider these and use them
// in more modular API designs. 

// Returns true if the specified file path exists. Check oErrorGetLast() on 
// failure.
typedef oFUNCTION<bool(const char* _Path)> oFUNCTION_PATH_EXISTS;

// Returns true if _pDestination has been filled with the contents of the file
// specified by _Path. Check oErrorGetLast() on failure.
typedef oFUNCTION<bool(void* _pDestination, size_t _SizeofDestination, const char* _Path)> oFUNCTION_LOAD_BUFFER;

// A generic functor that boils down to "just call it"
typedef oFUNCTION<void()> oTASK;

// A parallel-for style task that includes specification of its ordinal position
// in a container.
typedef oFUNCTION<void(size_t _Index)> oINDEXED_TASK;

typedef oFUNCTION<void*(size_t _NumBytes)> oALLOCATE;
typedef oFUNCTION<void(void* _Pointer)> oDEALLOCATE;

#endif
