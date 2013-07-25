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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oStd_all_h
#define oStd_all_h
#include <oStd/algorithm.h>
#include <oStd/assert.h>
#include <oStd/atof.h>
#include <oStd/byte.h>
#include <oStd/callable.h>
#include <oStd/color.h>
#include <oStd/date.h>
#include <oStd/djb2.h>
#include <oStd/endian.h>
#include <oStd/equal.h>
#include <oStd/finally.h>
#include <oStd/fixed_string.h>
#include <oStd/fixed_vector.h>
#include <oStd/fnv1a.h>
#include <oStd/fourcc.h>
#include <oStd/function.h>
#include <oStd/guid.h>
#include <oStd/ini.h>
#include <oStd/intrinsics.h>
#include <oStd/macros.h>
#include <oStd/murmur3.h>
#include <oStd/operators.h>
#include <oStd/oFor.h>
#include <oStd/oStdAtomic.h>
#include <oStd/oStdChrono.h>
#include <oStd/oStdConditionVariable.h>
#include <oStd/oStdFuture.h>
#include <oStd/oStdMakeUnique.h>
#include <oStd/oStdMutex.h>
#include <oStd/oStdRatio.h>
#include <oStd/oStdThread.h>
#include <oStd/path.h>
#include <oStd/path_traits.h>
#include <oStd/string.h>
#include <oStd/string_traits.h>
#include <oStd/text_document.h>
#include <oStd/throw.h>
#include <oStd/timer.h>
#include <oStd/uint128.h>
#include <oStd/unordered_map.h>
#include <oStd/uri.h>
#include <oStd/xml.h>
#endif
