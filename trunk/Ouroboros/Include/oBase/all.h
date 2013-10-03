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
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oBase_all_h
#define oBase_all_h
#include <oBase/algorithm.h>
#include <oBase/assert.h>
#include <oBase/atof.h>
#include <oBase/byte.h>
#include <oBase/color.h>
#include <oBase/date.h>
#include <oBase/djb2.h>
#include <oBase/endian.h>
#include <oBase/equal.h>
#include <oBase/event.h>
#include <oBase/finally.h>
#include <oBase/fixed_string.h>
#include <oBase/fixed_vector.h>
#include <oBase/fnv1a.h>
#include <oBase/fourcc.h>
#include <oBase/guid.h>
#include <oBase/ini.h>
#include <oBase/intrusive_ptr.h>
#include <oBase/macros.h>
#include <oBase/memory.h>
#include <oBase/murmur3.h>
#include <oBase/operators.h>
#include <oBase/opttok.h>
#include <oBase/path.h>
#include <oBase/path_traits.h>
#include <oBase/scc.h>
#include <oBase/string.h>
#include <oBase/string_traits.h>
#include <oBase/text_document.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <oBase/type_info.h>
#include <oBase/types.h>
#include <oBase/uint128.h>
#include <oBase/unordered_map.h>
#include <oBase/uri.h>
#include <oBase/xml.h>
#endif
