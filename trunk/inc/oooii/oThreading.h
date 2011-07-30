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
unsigned int oThreadGetCurrentID();

// Returns the unique ID for the main thread that initialized the session for 
// the process.
unsigned int oThreadGetMainID();

// Return the native handle of the thread that calls this function (HANDLE on 
// Windows)
void* oThreadGetCurrentNativeHandle();

// Returns the ID of the calling process
unsigned int oProcessGetCurrentID();

// Call the specified function for each of the child processes of the current
// process. The function should return true to keep enumerating, or false to
// exit early. This function returns false if there is a failure, check 
// oGetLastError() for more information. The error can be ECHILD if there are no 
// child processes.
bool oProcessEnum(oFUNCTION<bool(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath)> _Function);

// Wait for a process to exit/finish
bool oProcessWaitExit(unsigned int _ProcessID, unsigned int _TimeoutMS = oINFINITE_WAIT);

// Returns true if the named process is active and running
bool oProcessExists(const char* _Name);

// Returns true if the specified process has a debugger, remote or otherwise,
// attached. If the _ProcessID is 0, then this returns results for the current
// process.
bool oProcessHasDebuggerAttached(unsigned int _ProcessID = 0);

// Unceremoniously ends the specified process
bool oProcessTerminate(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive = true);

// Unceremoniously end all processes that are children of the current process.
void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive = true);

struct oPROCESS_MEMORY_STATS
{
	unsigned long long WorkingSet;
	unsigned long long WorkingSetPeak;
	unsigned long long NonSharedUsage;
	unsigned long long PageFileUsage;
	unsigned long long PageFileUsagePeak;
	unsigned int NumPageFaults;
};

// Fills the specified stats struct with summary memory usage information about
// the specified process.
bool oProcessGetMemoryStats(unsigned int _ProcessID, oPROCESS_MEMORY_STATS* _pStats);

struct oPROCESS_TIME_STATS
{
	time_t StartTime;
	time_t ExitTime;
	time_t KernelTime;
	time_t UserTime;
};

bool oProcessGetTimeStats(unsigned int _ProcessID, oPROCESS_TIME_STATS* _pStats);
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
