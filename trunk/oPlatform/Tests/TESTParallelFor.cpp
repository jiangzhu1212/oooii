// $(header)
#include <oPlatform/oTest.h>
#include <oBasis/oTask.h>

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

		oTaskParallelFor(0, mArraySize, oBIND(&ParallelTestA, oBIND1, mTestArrayB));
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor single param failed to compute singlethreaded result" );

		// Test two parameters
		for( size_t i = 0; i < mArraySize; ++i )
			ParallelTestAB( i, mTestArrayA, 2 );

		oTaskParallelFor(0, mArraySize, oBIND(&ParallelTestAB, oBIND1, &mTestArrayB[0], 2));
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor failed to compute singlethreaded result" );

		// Test three parameters
		for( size_t i = 0; i < mArraySize; ++i )
			ParallelTestABC( i, mTestArrayA, 2, 7 );

		oTaskParallelFor(0, mArraySize, oBIND(&ParallelTestABC, oBIND1, &mTestArrayB[0], 2, 7));
		oTESTB( memcmp( mTestArrayA, mTestArrayB, mArraySize * sizeof(int) ) == 0, "oParallelFor failed to compute singlethreaded result" );

		oTaskParallelFor(0, mArraySize, oBIND(&ParallelTestAB, oBIND1, &mTestArrayB[0], 2));


		// Test parallel do
		static const int funcCount = 32;
		int TestArray[funcCount];
		oTASK funcArray[funcCount];

		for( int i = 0; i < funcCount; ++i )
		{
			funcArray[i] = oBIND( &SetIndex, TestArray, i );
		}
		oTaskRunParallel( funcArray, funcCount );
		for( int i = 0; i < funcCount; ++i )
		{
			if( TestArray[i] != i * i )
			{
				oTESTB( false, "oIssueParallelTask failed to compute answer!" );
			}
		}
		return SUCCESS;
	}

	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];
};

oTEST_REGISTER(TESTParallelFor);
