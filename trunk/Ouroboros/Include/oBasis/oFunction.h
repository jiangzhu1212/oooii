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
// Convenience shorthands for std::bind and std::function, but also serves as an
// abstraction between std::tr1, std::, and boost::
#pragma once
#ifndef oFunction_h
#define oFunction_h

#include <functional>

template<typename T> std::reference_wrapper<T> oBINDREF__(T& _Value) { return std::reference_wrapper<T>(_Value); }

#define oFUNCTION std::function
#define oBIND std::bind
#define oBINDREF oBINDREF__ // #defined so it's the same color as other oBIND elements for Visual Assist, et al.
#define oBIND1 std::placeholders::_1
#define oBIND2 std::placeholders::_2
#define oBIND3 std::placeholders::_3
#define oBIND4 std::placeholders::_4
#define oBIND5 std::placeholders::_5
#define oBIND6 std::placeholders::_6
#define oBIND7 std::placeholders::_7
#define oBIND8 std::placeholders::_8
#define oBIND9 std::placeholders::_9

#endif
