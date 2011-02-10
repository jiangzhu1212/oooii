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
#pragma once
#ifndef oThreading_h
#define oThreading_h

#include <oooii/oInterface.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

// @oooii-tony: Avoid including <windows.h> and keep the convenience of inline
// construction (no factory pattern), declare these non-virtual and use 
// something very generic to ensure class size.

#ifdef _WIN64
	#define oMUTEX_FOOTPRINT() unsigned __int64 Footprint[5] // RTL_CRITICAL_SECTION
#elif defined(_WIN32)
	#define oMUTEX_FOOTPRINT() unsigned int Footprint[6]
#else
	#error Unsupported platform (oThreading)
#endif

#ifdef _DEBUG
	// Debug info for tagging which thread we're on to allow for protection against
	// attempts to recursively lock.
	#define oRWMUTEX_FOOTPRINT() void* Footprint; mutable size_t ThreadID
#else
	#define oRWMUTEX_FOOTPRINT() void* Footprint
#endif

class oMutex : oNoncopyable
{
	// NOTE: This mutex is recursive.

	oMUTEX_FOOTPRINT();
public:
	oMutex();
	~oMutex();
	void* GetNativeHandle() threadsafe;
	void Lock() threadsafe;
	bool TryLock() threadsafe;
	void Unlock() threadsafe;

	class ScopedLock : oNoncopyable
	{
		threadsafe oMutex& m;
	public:
		//@oooii-Andrew: casting away const because mutex locks are similar to ref counting
		// and does not affect the class memory layout in a meaningful way - Kevin
		inline ScopedLock(const threadsafe oMutex& in) : m(const_cast<threadsafe oMutex&>(in)) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};
};

class oRWMutex : oNoncopyable
{
	// NOTE: This mutex is not recursive.

	oRWMUTEX_FOOTPRINT();
public:
	oRWMutex();
	~oRWMutex();

	void* GetNativeHandle() threadsafe;
	void Lock() threadsafe; // Locks exclusively, no other lock attempt can go through
	void LockRead() const threadsafe; // Locks shared, other shared locks can go through, but not exclusives
	void Unlock() threadsafe;
	void UnlockRead() const threadsafe;

	class ScopedLock : oNoncopyable
	{
		threadsafe oRWMutex& m;
	public:
		inline ScopedLock(threadsafe oRWMutex& in) : m(in) { m.Lock(); }
		inline ~ScopedLock() { m.Unlock(); }
	};

	class ScopedLockRead : oNoncopyable
	{
		const threadsafe oRWMutex& m;
	public:
		inline ScopedLockRead(const threadsafe oRWMutex& in) : m(in) { m.LockRead(); }
		inline ~ScopedLockRead() { m.UnlockRead(); }
	};
};

// Asserts if the specified mutex is locked. This is a useful call from a dtor
// that uses a mutex to ensure it's lock isn't left dangling.
void oCheckUnlocked(threadsafe oMutex& _Mutex);
void oCheckUnlocked(threadsafe oRWMutex& _RWMutex);

// oParallelFor is a wrapper for TBB parallel for that takes a single threaded
// for loop and turns it into a loop that can be executed on several threads
// when it returns the loop has completed executing
// For example, given the following function:
//
// void foo( size_t index, ... );
// 
// Parallel for can be executed by calling:
//
// oParallelFor(oBIND(foo, 2, ...), 0, 1024);
//
// Which is equivalent to calling:
//
// for( size_t i = 0; i < 1024; i += 2 )
// {
//	    foo( i, ... );
// }
// Use oBIND() to reduce any function signature to this one, a function of the
// form void MyFunc(size_t index, ...) where ... can be any number of other 
// parameters. See oBIND/std::tr1::bind()/boost::bind() for more details.
void oParallelFor(oFUNCTION<void(size_t index)> _Function, size_t _Begin, size_t _End);

// oRunParallelTasks runs a list of functions in parallel through the TBB task scheduler
// when it returns all the functions have executed
void oRunParallelTasks(oFUNCTION<void()>* _pFunctions, int _FunctionCount);

// oIssueAsyncTask inserts a task to be executed by the TBB task
// scheduler.  Tasks should be non-blocking (no slow file io ) and synchronization 
// should be kept to a minimum.  No dependencies are exposed to keep things simple.
void oIssueAsyncTask(oFUNCTION<void()> _Function);

// Asynchronous job pools/task pools/thread pools are pretty smart. Sometimes 
// they are so smart they appear to be leaking memory when then might be keeping 
// a hot cache of recently used objects. When debugging or testing use this call
// to flush out any kind of smarts the underlying schedule might be trying to do.
void oRecycleScheduler();

// If you want to sleep a thread just enough to let the CPU process something
// else, use this instead.
void oYield();
void oSleep(unsigned int _Milliseconds);

#define oSPIN_UNTIL(_ThreadsafeExitCriteria) while (!(_ThreadsafeExitCriteria)) oYield()

class oEvent;
namespace oAsyncFileIO
{
	struct RESULT
	{
		void *pData; // buffer containing the total contents of the file described by Path in the DESC
		size_t Size; // size of the data read
		errno_t Result; // Result of the read
	};

	struct DESC
	{
		const char* Path; // Full file path
		void* (*Allocate)(size_t _Size); // allocation routine to be called on the IO thread
		threadsafe oEvent* pCompletionEvent; // (optional) Event to be fired when load is complete
		void (*Continuation)(RESULT* _pResult, void* _pUserData); // (optional) function to call on the IO thread once the file is done
		void* pUserData; // (optional) user data for the continuation function (can be null)
	};

	bool InitializeIOThread(size_t _MaxNumQueuedRequests, bool _ExecuteOnMainThread = false);
	void DeinitializeIOThread();

	// Schedules a call to oLoadBuffer() on another thread.
	void ScheduleFileRead(const DESC* _pDesc, RESULT* _pResult);
};

#endif
