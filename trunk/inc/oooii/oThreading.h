// $(header)

// Utility code for working in a multi-threaded environment. Though threads, 
// processes, and job system schedulers are platform concepts, parallelism is
// so fundamental to performant code that these APIs are used in what is 
// otherwise an abstract/generic library. Linking to the library will result
// in link errors to these functions because of course any code must be run
// on a specific platform.
#pragma once
#ifndef oThreading_h
#define oThreading_h

#include <oooii/oStddef.h>

// _____________________________________________________________________________
// Thread & Process identification API

// Returns the unique ID for the thread that calls this function
unsigned int oGetCurrentThreadID();

// Returns the unique ID for the main thread that initialized the session for 
// the process.
unsigned int oGetMainThreadID();

// Returns true if the thread calling this function is the main thread
inline bool oCurrentThreadIsMain() { return oGetCurrentThreadID() == oGetMainThreadID(); }

// Return the native handle of the thread that calls this function (HANDLE on 
// Windows)
void* oGetCurrentThreadNativeHandle();

// Returns the unique ID for the process that calls this function
unsigned int oGetCurrentProcessID();

// Returns the native handle of the process that calls this function (HANDLE on
// Windows)
void* oGetProcessNativeHandle(const char* _Name);

// Returns true if the named process is active and running
inline bool oProcessExists(const char* _Name) { return !!oGetProcessNativeHandle(_Name); }

// Returns the native handle of the module that initialized this process 
// (HMODULE on Windows)
void* oGetMainProcessModuleNativeHandle();

// _____________________________________________________________________________
// Thread suspension API

// If you want to sleep a thread just enough to let the CPU process something
// else, use this instead.
void oYield();
void oSleep(unsigned int _Milliseconds);

#define oSPIN_UNTIL(_ThreadsafeExitCriteria) while (!(_ThreadsafeExitCriteria)) oYield()

// _____________________________________________________________________________
// Job system API

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
void oParallelFor(oFUNCTION<void(size_t _Index)> _Function, size_t _Begin, size_t _End);

// For debugging so code can be kept similar to oParallelFor usage, this calls
// the _Function in order.
void oSerialFor(oFUNCTION<void(size_t _Index)> _Function, size_t _Begin, size_t _End);

// oRunParallelTasks runs a list of functions in parallel through the TBB task scheduler
// when it returns all the functions have executed
void oRunParallelTasks(oFUNCTION<void()>* _pFunctions, size_t _NumFunctions);

// For debugging so code can be kept similar to oRunParallelTasks, this calls
// the _pFunctions in order
void oRunSerialTasks(oFUNCTION<void()>* _pFunctions, size_t _NumFunctions);

// oIssueAsyncTask inserts a task to be executed by the TBB task
// scheduler.  Tasks should be non-blocking (no slow file io ) and synchronization 
// should be kept to a minimum.  No dependencies are exposed to keep things simple.
void oIssueAsyncTask(oFUNCTION<void()> _Function);

// Asynchronous job pools/task pools/thread pools are pretty smart. Sometimes 
// they are so smart they appear to be leaking memory when then might be keeping 
// a hot cache of recently used objects. When debugging or testing use this call
// to flush out any kind of smarts the underlying schedule might be trying to do.
void oRecycleScheduler();

#endif
