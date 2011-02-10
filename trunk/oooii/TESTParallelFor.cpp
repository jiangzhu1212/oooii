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
#include "pch.h"
#include <oooii/oTest.h>
#include <oooii/oThreading.h>

void ParallelTestA( size_t index, int* array )
{
	array[index] = (int)(index);
}

void ParallelTestAB( size_t index, int* array, int loop )
{
	int final = (int)index;
	for( int i = 0; i < loop; ++i )
	{
		final *= final; 
	}
	array[index] = final;
}

void ParallelTestABC( size_t index, int* array, int loop, int test )
{
	int final = (int)index;
	for( int i = 0; i < loop; ++i )
	{
		final *= final; 
	}

	array[index] = index % test ? 42 : final;
}

void Meh()
{
	while(1)
	{
		oSleep( 10 );
	}
}

void SetIndex( int* array, int index )
{
	array[index] = index * index;
}

struct TESTParallelFor : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		// Test single parameter
		for( size_t i = 0; i < mArraySize; ++i )
			ParallelTestA( i, mTestArrayA );

		oParallelFor(oBIND(&ParallelTestA, oBIND1, mTestArrayB), 0, mArraySize);
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor single param failed to compute singlethreaded result" );

		// Test two parameters
		for( size_t i = 0; i < mArraySize; ++i )
			ParallelTestAB( i, mTestArrayA, 2 );

		oParallelFor(oBIND(&ParallelTestAB, oBIND1, &mTestArrayB[0], 2), 0, mArraySize);
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor failed to compute singlethreaded result" );

		// Test three parameters
		for( size_t i = 0; i < mArraySize; ++i )
			ParallelTestABC( i, mTestArrayA, 2, 7 );

		oParallelFor(oBIND(&ParallelTestABC, oBIND1, &mTestArrayB[0], 2, 7),  0, mArraySize);
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor failed to compute singlethreaded result" );

		oParallelFor(oBIND(&ParallelTestAB, oBIND1, &mTestArrayB[0], 2),  0, mArraySize);


		// Test parallel do
		static const int funcCount = 32;
		int TestArray[funcCount];
		oFUNCTION<void()> funcArray[funcCount];

		for( int i = 0; i < funcCount; ++i )
		{
			funcArray[i] = oBIND( &SetIndex, TestArray, i );
		}
		oRunParallelTasks( funcArray, funcCount );
		for( int i = 0; i < funcCount; ++i )
		{
			if( TestArray[i] != i * i )
			{
				oTESTB( false, "oIssueParallelTask failed to compute answer!" );
			}
		}

		oRecycleScheduler();
		return SUCCESS;
	}

	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];
};

TESTParallelFor TestParallelFor;
