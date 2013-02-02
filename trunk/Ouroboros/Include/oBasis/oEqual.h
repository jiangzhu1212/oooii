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
// Approximate equality for float/double types. This uses absolute bit 
// differences rather than epsilon or some very small float value because eps
// usage isn't valid across the entire range of floating precision. With this
// oEqual API it can be guaranteed that the statement "equal within N minimal 
// bits of difference" will be true.
#pragma once
#ifndef oEqual_h
#define oEqual_h

// ulps = "units of last place". Number of float-point steps of error. At various
// sizes, 1 bit of difference in the floating point number might mean large or
// small deltas, so just doing an epsilon difference is not valid across the
// entire spectrum of floating point representations. With ULPS, you specify the
// maximum number of floating-point steps, not absolute (fixed) value that some-
// thing should differ by, so it scales across all of float's range.
#define oDEFAULT_ULPS 5

template<typename T> inline bool oEqual(const T& A, const T& B, int maxUlps = oDEFAULT_ULPS) { return A == B; }

bool oEqual(const float& A, const float& B, int maxUlps = oDEFAULT_ULPS);
bool oEqual(const double& A, const double& B, int maxUlps = oDEFAULT_ULPS);

#endif
