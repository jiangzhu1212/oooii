// $(header)
#include <oBasis/oDispatchQueuePrivate.h>
#include <oBasis/oError.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oRef.h>
#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oStdMutex.h>
#include <oBasis/oTask.h>
#include "oBasisTestCommon.h"

static void SetLocation(size_t _Index, size_t _Start, int* _Array)
{
	int startValue = _Array[_Start - 1];
	_Array[_Index] = startValue + static_cast<int>(_Index + 1 - _Start);
}

static void FillArray(int* _Array, size_t _Start, size_t _End, oStd::thread::id* _pExecutionThreadID, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID == oStd::thread::id()) // this command should execute before any others.
		*_pExecutionThreadID = oStd::this_thread::get_id();

	if (*_pExecutionThreadID != oStd::this_thread::get_id())
		*_pWrongThreadError = true;

	oTaskParallelFor(_Start, _End, oBIND(&SetLocation, oBIND1, _Start, _Array));
}

static void CheckTest(int* _Array, size_t _Size, bool* _pResult, oStd::thread::id* _pExecutionThreadID, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID != oStd::this_thread::get_id())
		*_pWrongThreadError = true;

	*_pResult = false;
	for (size_t i = 0; i < _Size; i++)
		if (_Array[i] != static_cast<int>(i))
			return;
	*_pResult = true;
}

static void NotifyAll(oStd::condition_variable& _ConditionVariable, oStd::thread::id* _pExecutionThreadID, bool* _pWrongThreadError)
{
	if (*_pExecutionThreadID != oStd::this_thread::get_id())
		*_pWrongThreadError = true;

	_ConditionVariable.notify_all();
}

bool oBasisTest_oDispatchQueuePrivate()
{
	oRef<threadsafe oDispatchQueue> q;
	oTESTB(oDispatchQueueCreatePrivate("TESTDispatchQueuePrivate", 100, &q), "Failed to create private dispatch queue");
	oOnScopeExit JoinQueue([&] { q->Join(); });

	static const size_t TestSize = 4096;
	int TestArray[TestSize];

	for(int i = 0; i < 2; ++i)
	{
		bool bResult = false;

		oStd::condition_variable Finished;
		oStd::mutex FinishedMutex;

		memset(TestArray, -1, TestSize * sizeof(int));
		TestArray[0] = 0;

		oStd::thread::id ExecutionThreadID;
		bool WrongThread = false;

		q->Dispatch(oBIND(&FillArray, TestArray, 1, 1024, &ExecutionThreadID, &WrongThread));
		q->Dispatch(oBIND(&FillArray, TestArray, 1024, 2048, &ExecutionThreadID, &WrongThread));
		q->Dispatch(oBIND(&FillArray, TestArray, 2048, TestSize, &ExecutionThreadID, &WrongThread));
		q->Dispatch(oBIND(&CheckTest, TestArray, TestSize, &bResult, &ExecutionThreadID, &WrongThread));
		q->Dispatch(oBIND(&NotifyAll, oBINDREF(Finished), &ExecutionThreadID, &WrongThread));

		oStd::unique_lock<oStd::mutex> FinishedLock(FinishedMutex);
		Finished.wait(FinishedLock);

		oTESTB(bResult, "oDispatchQueuePrivate failed to preserve order!");
		oTESTB(!WrongThread, "oDispatchQueuePrivate command was not executing on the correct thread.");
	}

	oErrorSetLast(oERROR_NONE);
	return true;
}
