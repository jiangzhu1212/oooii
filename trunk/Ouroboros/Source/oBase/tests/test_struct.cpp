/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include "test_struct.h"
#include <oString/string.h>
#include <time.h>

void init_test_struct(test_struct* t, const void* buffer1, int buffer1_size, const void* buffer2, int buffer2_size)
{
	t->c1 = 'c';
	strlcpy(t->String, "String");
	t->int1 = 1;
	t->long1 = 11111111;
	t->floatArray[0] = 1.00f;
	t->floatArray[1] = 1.01f;
	t->floatArray[2] = 1.02f;
	t->int2 = 2;
	t->blA = false;
	t->blB = true;
	t->blC = false;
	t->blD = true;
	t->p1 = "p1 a string larger than a pointer size (len=48)";
	strlcpy(t->TestString, "TestString");
	t->float1 = 1.111f;
	// Expected padding
	t->p2 = "p2";
	t->int3 = 3;
	t->c2 = 'K';
	t->b1size = buffer1_size;
	t->b1 = buffer1;
	t->NullString = nullptr;
	t->int4 = 4;
	t->intArray[0] = 100;
	t->intArray[1] = 101;
	t->intArray[2] = 102;
	t->intArray[3] = 103;
	t->double1 = 7.77777;
	t->float2 = 2.222f;
	t->b2size = buffer2_size;
	strlcpy(t->FinalTestString, "FinalTestString");
	t->b2 = buffer2;
	time_t tm = time(nullptr);
	t->time = ouro::date_cast<ouro::ntp_timestamp>(tm);
}

bool operator==(const test_struct& x, const test_struct& y)
{
	return x.c1 == y.c1
		&& !strcmp(x.String, y.String)
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
