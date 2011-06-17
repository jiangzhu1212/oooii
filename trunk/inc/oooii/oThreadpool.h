// $(header)
// NOTE: Prefer oParallelFor to this threadpool. The code is simpler, performance
// is currently 3x in debug, 4x in release over oThreadpool, and oThreadpool is
// still in active development, or rather is used primarily as a testbed for 
// various concurrent queue implementations.
#pragma once
#ifndef oThreadpool_h
#define oThreadpool_h

#include <oooii/oInterface.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oEvent;
interface oThreadpool : oInterface
{
	enum IMPLEMENTATION
	{
		WINDOWS_THREAD_POOL,
		OOOII,
	};

	struct DESC
	{
		DESC()
			: Implementation(WINDOWS_THREAD_POOL)
			, NumThreads(~0u)
		{}

		IMPLEMENTATION Implementation;
		unsigned int NumThreads; // specify ~0u for same number as HW threads. 0 or 1 means single-threaded.
	};

	typedef void (*TaskProc)(void* _pData);

	static bool Create(const DESC* _pDesc, threadsafe oThreadpool** _ppThreadpool);

	// This replaces any prior event
	virtual void RegisterShutdownEvent(oEvent* _pShutdownEvent) threadsafe = 0;
	virtual void ScheduleTask(TaskProc _Task, void* _pData) threadsafe = 0;
};

#endif
