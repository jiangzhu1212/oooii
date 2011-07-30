// $(header)
#include <oooii/oTest.h>
#include <oooii/oEvent.h>
#include <oooii/oCommandQueue.h>
#include <oooii/oThreading.h>

struct TESTCommandQueue : public oTest
{
	static void SetLocation( size_t index, size_t start, int* array )
	{
		int startValue = array[start - 1];
		array[index] = startValue + (int)(index + 1 - start );
	}

	static void FillArray( int* array, size_t start, size_t end )
	{
		oParallelFor( oBIND( &SetLocation, oBIND1, start, array ), start, end );
	}

	static void FireEvent( oEvent* event )
	{
		event->Set();
	}

	static void CheckTest( int* array, size_t size, bool* result )
	{
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
		oCommandQueue queue;

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
			oTESTB( bResult, "oCommandQueue failed to preserver order!");
		}

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTCommandQueue);
