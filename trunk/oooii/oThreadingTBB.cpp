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
#ifndef OOOII_NO_TBB
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>
#include <oooii/oSingleton.h>
#include <oooii/oStddef.h>
#include <oooii/oWindows.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
using namespace tbb;

class TBBInit : public oSingleton<TBBInit>
{
	// @oooii-tony: TBB's static init can result in false-positives in the on-exit
	// leak detector. To work around that, control TBB's lifetime. Also, because
	// TBB doesn't block on its own cleanup of worker threads, go through a whole 
	// bunch of headache code to do what TBB should have, wait on all worker 
	// threads to be finished before exiting.

	// can't wait at once on more than 64 in one WaitForSingleObject() call, so
	// assume we won't have more than this for a while.
	static const size_t MAX_EXPECTED_THREADS = 64;

	task_scheduler_init* pInit;
	DWORD TBBThreadIDs[MAX_EXPECTED_THREADS];
	unsigned int NumTBBThreads;

	static void Noop(size_t _Index) {}

#ifdef _DEBUG
	// Allows us to break execution when an access violation occurs
	static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionInfo)
	{
		PEXCEPTION_RECORD& pRecord = _pExceptionInfo->ExceptionRecord;
		if (pRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			static const char* ReadError = "Read";
			static const char* WriteError = "Write";
			const char* err = ( 0 == pRecord->ExceptionInformation[0] ) ? ReadError : WriteError;

			oASSERT( false, "%s access violation at 0x%p", err, pRecord->ExceptionInformation[1] );
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
#endif

public:
	struct Run
	{
		Run() { TBBInit::Singleton(); }
	};

	TBBInit() { Create(); }
	~TBBInit() { Destroy(); }

	void Recycle() { Destroy(); Create(); }

private:

	void Create()
	{
		NumTBBThreads = 0;
		oASSERT(task_scheduler_init::default_num_threads() <= MAX_EXPECTED_THREADS, "");

		DWORD before[MAX_EXPECTED_THREADS];
		unsigned int nBefore = oGetProcessThreads(before);

		pInit = new tbb::task_scheduler_init();

		// TBB doesn't create worker threads until first use, so get that first use
		// out of the way here.
		oParallelFor(&Noop, 0, MAX_EXPECTED_THREADS);

		DWORD after[MAX_EXPECTED_THREADS];
		unsigned int nAfter = oGetProcessThreads(after);

		// Cull list into just what is different in threads
		for (size_t i = 0; i < nAfter; i++)
		{
			size_t j = 0;
			for (; j < nBefore; j++)
				if (after[i] == before[j])
					break;
			if (j == nBefore) // not found in before list, so this is a tbb thread
				TBBThreadIDs[NumTBBThreads++] = after[i];
		}

#ifdef _DEBUG
		// Since we use TBBs task system directly we need to handle exceptions
		// that may be thrown on task threads.  Typically TBB does this by
		// wrapping the tasks in try/catch blocks, however our usage model
		// directly manipulates the task system spawning asynchronous tasks.
		// Since we're only interested in access violations we register an
		// exception handler and place it last to capture any access violations
		AddVectoredExceptionHandler(0, &TBBInit::HandleAccessViolation );
#endif
	}

	void Destroy()
	{
		// Grab handles to threads before TBB starts destroying them
		HANDLE hThreads[64]; // max of windows anyway
		for (size_t i = 0; i < NumTBBThreads; i++)
			hThreads[i] = OpenThread(SYNCHRONIZE, FALSE, TBBThreadIDs[i]);

		delete pInit;

		// Finally! We can wait for TBB to be completely clean before moving onto
		// other static cleanup, such as detecting memory leaks.
		bool timedout = !oWaitMultiple(hThreads, NumTBBThreads, true, 5000);

		for (size_t i = 0; i < NumTBBThreads; i++)
			oVB(CloseHandle(hThreads[i]));

		if (timedout)
			oTRACE("Timeout waiting for TBB threads to exit.");
	}
};

static TBBInit::Run sTBBInit; // @oooii-tony: ok static, we just need it to run the singleton before any use code kicks in, and oSingleton guarantees it'll run once per process.

void oRecycleScheduler()
{
	TBBInit::Singleton()->Recycle();
}

class TBBParallelFor 
{
	oFUNCTION<void(size_t index)> Function;
public:
	TBBParallelFor(oFUNCTION<void(size_t index)> _Function) : Function(_Function) {}

	void operator()(const blocked_range<size_t>& _Range) const
	{
		for (size_t i = _Range.begin(); i != _Range.end(); ++i)
			Function(i);
	}
};

void oParallelFor(oFUNCTION<void(size_t index)> _Function, size_t _Begin, size_t _End)
{
	parallel_for(blocked_range<size_t>(_Begin, _End), TBBParallelFor(_Function));
}


class TBBTask : public task
{
public:
	TBBTask(oFUNCTION<void()> _Task) :
	  Task(_Task)
	{}
	task* execute( ) 
	{
		Task();
		return NULL;
	}
private:
	oFUNCTION<void()> Task;
};
void oIssueAsyncTask(oFUNCTION<void()> _Function )
{
	// For tasks with no dependency we use task::enqueue this ensures the main thread never has to participate 
	// in TBB threading and prioritizes tasks that are issued without dependency as these tend to be tasks that 
	// are longer running and behave more like raw threads
	//
	// task::enqueue vs task::spawn
	// from http://software.intel.com/en-us/blogs/2010/05/04/tbb-30-new-today-version-of-intel-threading-building-blocks/
	// The TBB 3.0 schedule supports task::enqueue, which is effectively a “run me after the other things already pending” request. 
	// Although similar to spawning a task, an enqueued task is scheduled in a different manner.  Enqueued tasks are valuable when 
	// approximately first-in first-out behavior is important, such as in situations where latency of response is more important than 
	// efficient throughput.

	task& taskToSpawn = *new(task::allocate_root()) TBBTask(_Function);
	task::enqueue( taskToSpawn );
}

class TBBTaskGroup : public task
{
public:
	TBBTaskGroup(oFUNCTION<void()>* _pFunctions, int _FunctionCount) :
	   pFunctions(_pFunctions)
	  ,FunctionCount(_FunctionCount)
	  {}
	  task* execute( ) 
	  {
		  // Set the refcount to the number of children plus 1 for the wait
		  set_ref_count( FunctionCount + 1 );

		  // Spawn all children except the last one
		  for( int i = 0; i < FunctionCount - 1; ++i )
		  {
				spawn( *new( allocate_child() )TBBTask( pFunctions[i] ) );
		  }

		  // Spawn the last task and wait
		  spawn_and_wait_for_all( *new( allocate_child() )TBBTask( pFunctions[FunctionCount- 1] ) );
		  return NULL;
	  }
private:
	oFUNCTION<void()>* pFunctions;
	int FunctionCount;
};

void oRunParallelTasks( oFUNCTION<void()>* _pFunctions, int _FunctionCount )
{
	task& taskToSpawn = *new(task::allocate_root()) TBBTaskGroup(_pFunctions, _FunctionCount );
	task::spawn_root_and_wait( taskToSpawn );
}


#endif //OOOII_NO_TBB
