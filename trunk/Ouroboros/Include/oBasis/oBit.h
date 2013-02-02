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

// Utility functions helpful when dealing with bitfields. This is written to use
// the same API as HLSL Shader Model 5.0.
#pragma once
#ifndef oBit_h
#define oBit_h

#include <oBasis/oHLSLBit.h>

// Returns the next power of 2 from the current value.
// So if _Value is 512 this will return 1024
inline unsigned long oNextPow2(unsigned long _Value)
{
	unsigned long pow2;
	_BitScanReverse(&pow2, _Value);
	++pow2;
	return 1 << pow2;
}

// Returns whether the value is between the _Start and the _End (inclusive of _Start and _End) taking into account integer rollover
template<typename T>
inline bool oBetweenInclusive( T _Start, T _End, T _Value )
{
	// @oooii-kevin: This cast is necessary because the MS compiler is implicitly converting the intermediate computation to 32-bit (which is probably a compiler bug)
	return (T)(_Value - _Start)  <= (T)(_End - _Start);
}

#endif
