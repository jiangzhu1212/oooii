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
// This is a complex struct useful for reflection/serialization tests because it
// exacerbates internal alignment and requires non-trivial consideration of 
// type usage.
#pragma once
#ifndef oBasisTestStruct_h
#define oBasisTestStruct_h

#include <oStd/date.h>
#include <oBasis/oPlatformFeatures.h>

struct oBASIS_TEST_STRUCT
{
	bool blA;
	char c1;
	char OOOiiString[64 * 3];
	bool blB;
	int int1;
	long long long1;
	float floatArray[3];
	int int2;
	bool blC;
	bool blD;
	const char* p1;
	char TestString[64 * 2];
	float float1;
	char FinalTestString[64];
	const char* p2;
	int int3;
	char c2;
	// Expected padding
	int b1size;
	const void* b1;
	const char* NullString;
	int int4;
	int intArray[4];
	double double1;
	float float2;
	int b2size;
	const void* b2;
	oStd::ntp_timestamp time;
};

#define oBASIS_TEST_STRUCT_MEMBERS(_Macro) \
	_Macro(blA) \
	_Macro(c1) \
	_Macro(OOOiiString) \
	_Macro(blB) \
	_Macro(int1) \
	_Macro(long1) \
	_Macro(floatArray) \
	_Macro(int2) \
	_Macro(blC) \
	_Macro(blD) \
	_Macro(p1) \
	_Macro(TestString) \
	_Macro(float1) \
	_Macro(FinalTestString) \
	_Macro(p2) \
	_Macro(int3) \
	_Macro(c2) \
	_Macro(b1size) \
	_Macro(b1) \
	_Macro(NullString) \
	_Macro(int4) \
	_Macro(intArray) \
	_Macro(double1) \
	_Macro(float2) \
	_Macro(b2size) \
	_Macro(b2) \
	_Macro(time)

oAPI bool operator==(const oBASIS_TEST_STRUCT& x, const oBASIS_TEST_STRUCT& y);
oAPI void oBasisTestStructInit(oBASIS_TEST_STRUCT* _pTest, const void* _pBuffer1, int _SizeofBuffer1, const void* _pBuffer2, int _SizeofBuffer2);
inline bool operator!=(const oBASIS_TEST_STRUCT& x, const oBASIS_TEST_STRUCT& y) { return !(x == y); }

#endif
