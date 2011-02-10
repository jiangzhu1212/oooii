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
#include <oooii/oEvent.h>
#include <oooii/oCommandQueue.h>

void SetLocation( size_t index, size_t start, int* array )
{
	int startValue = array[start - 1];
	array[index] = startValue + (int)(index + 1 - start );
}

void FillArray( int* array, size_t start, size_t end )
{
	oParallelFor( oBIND( &SetLocation, oBIND1, start, array ), start, end );
}

void FireEvent( oEvent* event )
{
	event->Set();
}

void CheckTest( int* array, size_t size, bool* result )
{
	*result = false;
	for( size_t i = 0; i < size; ++i )
	{
		if( array[i] != (int)i)
			return;
	}
	*result = true;
}

struct TESTCommandQueue : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCommandQueue queue;

		static const size_t TestSize = 4096;
		int TestArray[TestSize];

		for( int i = 0; i < 2; ++i )
		{
			bool bResult = false;
			oEvent FinishedEvent;

			memset( TestArray, -1, TestSize * sizeof(int ) );
			TestArray[0] = 0;

			queue.AddCommand( oBIND(&FillArray, TestArray, 1, 1024 ) );
			queue.AddCommand( oBIND(&FillArray, TestArray, 1024, 2048 ) );
			queue.AddCommand( oBIND(&FillArray, TestArray, 2048, TestSize ) );
			queue.AddCommand( oBIND(&CheckTest, TestArray, TestSize, &bResult ) );
			queue.AddCommand( oBIND(&FireEvent, &FinishedEvent ) );
			FinishedEvent.Wait();
			oTESTB( bResult, "oCommandQueue failed to preserver order!");
		}

		oRecycleScheduler();

		return SUCCESS;
	}
};

TESTCommandQueue TESTCommandQueueTest;