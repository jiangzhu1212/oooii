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
#include <oooii/oTest.h>
#include <oooii/oEvent.h>
#include <oooii/oCommandQueueSingleThread.h>
#include <oooii/oThreading.h>

struct TESTCommandQueueSingleThread : public oTest
{
	static unsigned int CommandThreadID; //assumption that we will never get a thread id of 0;
	static bool WrongThreadError;

	static void SetLocation( size_t index, size_t start, int* array )
	{
		int startValue = array[start - 1];
		array[index] = startValue + (int)(index + 1 - start );
	}

	static void FillArray( int* array, size_t start, size_t end )
	{
		if( CommandThreadID == 0) //this command should execute before any others.
			CommandThreadID = oGetCurrentThreadID();

		if(CommandThreadID != oGetCurrentThreadID())
			WrongThreadError = true;

		oParallelFor( oBIND( &SetLocation, oBIND1, start, array ), start, end );
	}

	static void FireEvent( oEvent* event )
	{
		if(CommandThreadID != oGetCurrentThreadID())
			WrongThreadError = true;

		event->Set();
	}

	static void CheckTest( int* array, size_t size, bool* result )
	{
		if(CommandThreadID != oGetCurrentThreadID())
			WrongThreadError = true;

		*result = false;
		for( size_t i = 0; i < size; ++i )
		{
			if( array[i] != (int)i)
				return;
		}
		*result = true;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oCommandQueueSingleThread queue;

		static const size_t TestSize = 4096;
		int TestArray[TestSize];

		for( int i = 0; i < 2; ++i )
		{
			bool bResult = false;
			oEvent FinishedEvent;

			memset( TestArray, -1, TestSize * sizeof(int ) );
			TestArray[0] = 0;

			queue.Enqueue( oBIND(&FillArray, TestArray, 1, 1024 ) );
			queue.Enqueue( oBIND(&FillArray, TestArray, 1024, 2048 ) );
			queue.Enqueue( oBIND(&FillArray, TestArray, 2048, TestSize ) );
			queue.Enqueue( oBIND(&CheckTest, TestArray, TestSize, &bResult ) );
			queue.Enqueue( oBIND(&FireEvent, &FinishedEvent ) );
			FinishedEvent.Wait();
			oTESTB( bResult, "oCommandQueueSingleThread failed to preserver order!");
			oTESTB(!WrongThreadError, "oCommandQueueSingleThread command was not executing on the correct thread.");
		}

		return SUCCESS;
	}
};

unsigned int TESTCommandQueueSingleThread::CommandThreadID = 0;
bool TESTCommandQueueSingleThread::WrongThreadError = false;

oTEST_REGISTER(TESTCommandQueueSingleThread);
