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
#include "oBasisTestStruct.h"
#include <oBasis/oError.h>
#include <time.h>

void oBasisTestStructInit(oBASIS_TEST_STRUCT* _pTest, const void* _pBuffer1, int _SizeofBuffer1, const void* _pBuffer2, int _SizeofBuffer2)
{
	_pTest->c1 = 'c';
	strlcpy(_pTest->OOOiiString, "OOOiiString");
	_pTest->int1 = 1;
	_pTest->long1 = 11111111;
	_pTest->floatArray[0] = 1.00f;
	_pTest->floatArray[1] = 1.01f;
	_pTest->floatArray[2] = 1.02f;
	_pTest->int2 = 2;
	_pTest->blA = false;
	_pTest->blB = true;
	_pTest->blC = false;
	_pTest->blD = true;
	_pTest->p1 = "p1 a string larger than a pointer size (len=48)";
	strlcpy(_pTest->TestString, "TestString");
	_pTest->float1 = 1.111f;
	// Expected padding
	_pTest->p2 = "p2";
	_pTest->int3 = 3;
	_pTest->c2 = 'K';
	_pTest->b1size = _SizeofBuffer1;
	_pTest->b1 = _pBuffer1;
	_pTest->NullString = nullptr;
	_pTest->int4 = 4;
	_pTest->intArray[0] = 100;
	_pTest->intArray[1] = 101;
	_pTest->intArray[2] = 102;
	_pTest->intArray[3] = 103;
	_pTest->double1 = 7.77777;
	_pTest->float2 = 2.222f;
	_pTest->b2size = _SizeofBuffer2;
	strlcpy(_pTest->FinalTestString, "FinalTestString");
	_pTest->b2 = _pBuffer2;
	time_t t = time(nullptr);
	_pTest->time = oStd::date_cast<oStd::ntp_timestamp>(t);
}

bool operator==(const oBASIS_TEST_STRUCT& x, const oBASIS_TEST_STRUCT& y)
{
	return x.c1 == y.c1
		&& !strcmp(x.OOOiiString, y.OOOiiString)
		&& x.int1 == y.int1
		&& x.long1 == y.long1
		&& x.floatArray[0] == y.floatArray[0]
		&& x.floatArray[1] == y.floatArray[1]
		&& x.floatArray[2] == y.floatArray[2]
		&& x.int2 == y.int2
		&& x.blA == y.blA
		&& x.blB == y.blB
		&& x.blC == y.blC
		&& x.blD == y.blD
		&& !strcmp(x.p1, y.p1)
		&& !strcmp(x.TestString, y.TestString)
		&& x.float1 == y.float1
		&& !strcmp(x.p2, y.p2)
		&& x.int3 == y.int3
		&& x.c2 == y.c2
		&& x.b1size == y.b1size
		&& !memcmp(x.b1, y.b1, x.b1size)
		&& x.NullString == y.NullString
		&& x.int4 == y.int4
		&& x.intArray[0] == y.intArray[0]
		&& x.intArray[1] == y.intArray[1]
		&& x.intArray[2] == y.intArray[2]
		&& x.intArray[3] == y.intArray[3]
		&& x.double1 == y.double1
		&& x.float2 == y.float2
		&& x.b2size == y.b2size
		&& !strcmp(x.FinalTestString, y.FinalTestString)
		&& !memcmp(x.b2, y.b2, x.b2size)
		&& x.time == y.time
		;
}
