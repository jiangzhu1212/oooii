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

class TBBTaskObserver : public task_scheduler_observer
{
public:
	TBBTaskObserver()
	{
		observe();
	}

	virtual void on_scheduler_exit( bool is_worker )
	{
		if(is_worker)
			detail::ReleaseThreadLocalSingletons();
	}
};

class TBBInit : public oProcessSingleton<TBBInit>
{
	task_scheduler_init* init;
	TBBTaskObserver* observer;

#ifdef _DEBUG
	// Allows us to break execution when an access violation occurs
	static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionInfo)
	{
		PEXCEPTION_RECORD& pRecord = _pExceptionInfo->ExceptionRecord;
		if (pRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			// @oooii-mike switched to a plain C assert because it was causing
			// problems with exceptions occurring during oASSERTs.

			//static const char* ReadError = "Read";
			//static const char* WriteError = "Write";
			//const char* err = ( 0 == pRecord->ExceptionInformation[0] ) ? ReadError : WriteError;

			//oASSERT( false, "%s access violation at 0x%p", err, pRecord->ExceptionInformation[1] );
			assert(false && "Access violation");
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
#endif


public:
	struct Run
	{
		Run() { TBBInit::Singleton(); }
	};

	~TBBInit()
	{
		//oASSERT(oThread::CurrentThreadIsMain(), "TBB must be deinitialized from Main Thread");
		delete init;
		delete observer;
	}


	TBBInit()
	{
		//oASSERT(oThread::CurrentThreadIsMain(), "TBB must be initialized from Main Thread");
		observer = new TBBTaskObserver;
		init = new task_scheduler_init;
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

	void Recycle()
	{
		// @oooii-kevin: This strange bit of code is based on discussions found here
		// http://origin-software.intel.com/en-us/forums/showthread.php?t=66757
		// basically to make certain TBB cleans itself up inside of Recycle we
		// have to tear down the initialization object and wait for some arbitrary
		// amount of time before starting it up again, as TBB asynchronously destroys
		// threads.  If we still see leaking threads, this wait may need to be increased.
		delete init;
		oSleep(1000);
		init = new task_scheduler_init;
	}

};

// @oooii-mike: static initialization of TBBInit from a spawned process that loaded
// oooiilib from a DLL was being called in a thread for some unknown reason, which
// caused TBB to incorrectly allocate thread local resources and assert on exit.
// static TBBInit::Run sTBBInit; // @oooii-tony: ok static, we just need it to run the singleton before any use code kicks in, and oSingleton guarantees it'll run once per process.

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
