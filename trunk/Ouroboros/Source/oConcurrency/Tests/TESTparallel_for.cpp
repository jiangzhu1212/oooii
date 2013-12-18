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
#include <oConcurrency/oConcurrency.h>
#include <oBase/macros.h>
#include <oBase/throw.h>

namespace oConcurrency {
	namespace tests {

static void test_a(size_t _Index, int* _pArray)
{
	_pArray[_Index] = (int)(_Index);
}

static void test_ab(size_t _Index, int* _pArray, int _Loop)
{
	int final = (int)_Index;
	for (int i = 0; i < _Loop; i++)
		final *= final; 
	_pArray[_Index] = final;
}

static void test_abc(size_t _Index, int* _pArray, int _Loop, int _Test)
{
	int final = (int)_Index;
	for(int i = 0; i < _Loop; i++)
		final *= final; 
	_pArray[_Index] = _Index % _Test ? 42 : final;
}

void TESTparallel_for()
{
	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];

	// Test single parameter
	oFORI(i, mTestArrayA)
		test_a(i, mTestArrayA);

	oConcurrency::parallel_for(0, mArraySize, std::bind(test_a, oBIND1, mTestArrayB));
	oCHECK(memcmp(mTestArrayA, mTestArrayB, mArraySize * sizeof(int)) == 0, 
		"oConcurrency::parallel_for single param failed to compute singlethreaded result");

	// Test two parameters
	oFORI(i, mTestArrayA)
		test_ab(i, mTestArrayA, 2);

	oConcurrency::parallel_for(0, mArraySize, std::bind(test_ab, oBIND1, &mTestArrayB[0], 2));
	oCHECK(memcmp(mTestArrayA, mTestArrayB, mArraySize * sizeof(int)) == 0
		, "oConcurrency::parallel_for failed to compute singlethreaded result");

	// Test three parameters
	oFORI(i, mTestArrayA)
		test_abc(i, mTestArrayA, 2, 7);

	oConcurrency::parallel_for(0, mArraySize, std::bind(test_abc, oBIND1, &mTestArrayB[0], 2, 7));
	oCHECK(memcmp(mTestArrayA, mTestArrayB, mArraySize * sizeof(int)) == 0
		, "oConcurrency::parallel_for failed to compute singlethreaded result");

	oConcurrency::parallel_for(0, mArraySize, std::bind(test_ab, oBIND1, &mTestArrayB[0], 2));
};

	} // namespace tests
} // namespace oConcurrency
