// $(header)
#ifndef OOOII_NO_TBB
#include <oPlatform/oCRTHeap.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oReporting.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oStddef.h>
#include <oPlatform/oWindows.h>
#include <tbb/tbb.h>
#include <tbb/task_scheduler_init.h>
#include <oPlatform/oProcessHeap.h>

void RegisterNOP()
{
}

void oTaskRegisterThisThread(const char* _DebuggerName)
{
	oDebuggerSetThreadName(_DebuggerName);
	// Issuing a NOP task causes TBB to allocate this thread's memory
	oTaskIssueAsync(&RegisterNOP);
}

using namespace tbb;

class TBBTaskObserver : public task_scheduler_observer
{
public:
	TBBTaskObserver()
	{
		observe();
	}

	virtual void on_scheduler_exit(bool is_worker)
	{
		if (is_worker)
			oEndThread();
	}
};


#ifdef _DEBUG
class TBBExceptionHandler : public oProcessSingleton<TBBExceptionHandler>
{
public:
	static const oGUID GUID;

	TBBExceptionHandler()
	{
		// Since we use TBBs task system directly we need to handle exceptions
		// that may be thrown on task threads.  Typically TBB does this by
		// wrapping the tasks in try/catch blocks, however our usage model
		// directly manipulates the task system spawning asynchronous tasks.
		// Since we're only interested in access violations we register an
		// exception handler and place it last to capture any access violations
		AddVectoredExceptionHandler(0, &TBBExceptionHandler::HandleAccessViolation);
	}

	~TBBExceptionHandler()
	{
		RemoveVectoredExceptionHandler(&TBBExceptionHandler::HandleAccessViolation);
	}

	// Allows us to break execution when an access violation occurs
	static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionInfo)
	{
		PEXCEPTION_RECORD& pRecord = _pExceptionInfo->ExceptionRecord;
		if (pRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		{
			static const char* ReadError = "Read";
			static const char* WriteError = "Write";
			const char* err = (0 == pRecord->ExceptionInformation[0]) ? ReadError : WriteError;

			oASSERT(false, "%s access violation at 0x%p", err, pRecord->ExceptionInformation[1]);
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
};

// {9840E986-9ADE-4D11-AFCE-AB2D8AC530C0}
const oGUID TBBExceptionHandler::GUID = { 0x9840e986, 0x9ade, 0x4d11, { 0xaf, 0xce, 0xab, 0x2d, 0x8a, 0xc5, 0x30, 0xc0 } };

struct TBBExceptionHandlerInstaller
{
	TBBExceptionHandlerInstaller()
	{
		oProcessHeapLockGuard Tickle; // ensure the process heap is instantiated before the Singleton below so it is tracked
		TBBExceptionHandler::Singleton();
	}
};

// @oooii-kevin: OK Static, we need to make sure the exception handler is installed very early 
static TBBExceptionHandlerInstaller GInstallHandler;
#endif

class TBBTaskSchedulerInit : public oProcessSingleton<TBBTaskSchedulerInit>
{
	task_scheduler_init* init;
	TBBTaskObserver* observer;

public:
	static const oGUID GUID;

	TBBTaskSchedulerInit()
	{
		observer = new TBBTaskObserver;
		init = new task_scheduler_init;
	}
	~TBBTaskSchedulerInit()
	{
		delete init;
		delete observer;
	}
};

// {CFEBF25E-97EA-4BAB-AC50-D53474D3C758}
const oGUID TBBTaskSchedulerInit::GUID = { 0xcfebf25e, 0x97ea, 0x4bab, { 0xac, 0x50, 0xd5, 0x34, 0x74, 0xd3, 0xc7, 0x58 } };

void oTaskInitScheduler()
{
	// Instantiate the singleton
	TBBTaskSchedulerInit::Singleton();
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

void oTaskParallelFor(size_t _Begin, size_t _End, oFUNCTION<void(size_t _Index)> _Function)
{
	parallel_for(blocked_range<size_t>(_Begin, _End), TBBParallelFor(_Function));
}


class TBBTask : public task
{
public:
	TBBTask(oTASK _Task) :
	  Task(_Task)
	{}
	task* execute() 
	{
		Task();
		return nullptr;
	}
private:
	oTASK Task;
};

#include "oCRTLeakTracker.h"

void oTaskIssueAsync(oTASK _Task)
{
	// @oooii-kevin: For tasks with no dependency we use task::enqueue this ensures the main thread never has to participate 
	// in TBB threading and prioritizes tasks that are issued without dependency as these tend to be tasks that 
	// are longer running and behave more like raw threads
	//
	// task::enqueue vs task::spawn
	// from http://software.intel.com/en-us/blogs/2010/05/04/tbb-30-new-today-version-of-intel-threading-building-blocks/
	// The TBB 3.0 schedule supports task::enqueue, which is effectively a “run me after the other things already pending” request. 
	// Although similar to spawning a task, an enqueued task is scheduled in a different manner.  Enqueued tasks are valuable when 
	// approximately first-in first-out behavior is important, such as in situations where latency of response is more important than 
	// efficient throughput.

	// @oooii-tony: This can report a false-positive leak with the Microsoft CRT 
	// leak reporter if task::allocate_root() is called in the middle of a memory
	// state check block. (working with _CrtMemState elsewhere). See oBug_1856
	// for more information.
	// NOTE: I tried to wrap this in a disable-tracking, reenable-after block, but
	// that can cause deadlocks as all of CRT it seems shares the same mutex. Also
	// just setting the state allows for any number of threads to have their 
	// allocs ignored during the disabled period. I tried having 

	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking(false);
	task& taskToSpawn = *new(task::allocate_root()) TBBTask(_Task);
	oCRTLeakTracker::Singleton()->EnableThreadlocalTracking();
	task::enqueue(taskToSpawn);
}

class TBBTaskGroup : public task
{
public:
	TBBTaskGroup(oTASK* _pFunctions, size_t _NumFunctions) :
	   pFunctions(_pFunctions)
	  , NumFunctions(_NumFunctions)
	  {}
	  task* execute() 
	  {
		  // Set the refcount to the number of children plus 1 for the wait
		  set_ref_count(static_cast<int>(NumFunctions) + 1);

		  // Spawn all children except the last one
		  for (size_t i = 0; i < NumFunctions - 1; i++)
				spawn(*new(allocate_child())TBBTask(pFunctions[i]));

		  // Spawn the last task and wait
		  spawn_and_wait_for_all(*new(allocate_child())TBBTask(pFunctions[NumFunctions - 1]));
		  return nullptr;
	  }
private:
	oTASK* pFunctions;
	size_t NumFunctions;
};

void oTaskRunParallel(oTASK* _pFunctions, size_t _NumFunctions)
{
	task& taskToSpawn = *new(task::allocate_root()) TBBTaskGroup(_pFunctions, _NumFunctions);
	task::spawn_root_and_wait(taskToSpawn);
}


#endif //OOOII_NO_TBB
