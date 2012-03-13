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
// Abstraction for a platform process and additional API for working with the 
// current and other system processes. All processes are started in a suspended 
// state so a debugger can be attached, so Start() must be called after an 
// oProcess is created. Also if this interface is destroyed, the process itself 
// may persist. Use Wait(), Kill() and/or GetExitCode() to control the process's 
// actual lifetime.
#pragma once
#ifndef oProcess_h
#define oProcess_h

#include <oBasis/oInterface.h>

interface oProcess : oInterface
{
	struct DESC
	{
		DESC()
			: CommandLine(0)
			, EnvironmentString(0)
			, StdHandleBufferSize(0)
			, SetFocus(true)
			, StartMinimized(false)
		{}

		const char* CommandLine;
		const char* EnvironmentString;
		
		// If 0, then a unique instance is created, no shared pipes, 
		// and thus WriteToStdin and ReadFromStdout will return 0 with
		// oErrorGetLast() set to EPIPE.
		size_t StdHandleBufferSize;
		bool SetFocus:1;
		bool StartMinimized:1;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Processes are created in a suspended state to give a developer an 
	// opportunity to set breakpoints and perform various other debugging 
	// activities. Call this to resume the process.
	virtual void Start() threadsafe = 0;

	// Stops the process as immediately as it can
	virtual bool Kill(int _ExitCode) threadsafe = 0;
	
	// Blocks until the process has exited or the timeout time has reached. 
	// Returns false if timed out.
	virtual bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
	
	// Returns the platform ID of this process
	virtual unsigned int GetProcessID() const threadsafe = 0;

	// Returns the platform ID of the main thread associated with this process
	virtual unsigned int GetThreadID() const threadsafe = 0;

	// Retrieves effectively the return value from main from this process. If it
	// cannot be retrieved, this function will return false.
	virtual bool GetExitCode(int* _pExitCode) const threadsafe = 0;

	virtual size_t WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe = 0;
	virtual size_t ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe = 0;
};

// Creates an instance of oProcess according to the specified desc.
oAPI bool oProcessCreate(const oProcess::DESC& _Desc, threadsafe oProcess** _ppProcess);

// Returns the ID of the calling process
oAPI unsigned int oProcessGetCurrentID();

// Call the specified function for each of the child processes of the current
// process. The function should return true to keep enumerating, or false to
// exit early. This function returns false if there is a failure, check 
// oErrorGetLast() for more information. The error can be oERROR_NOT_FOUND 
// if there are no child processes.
oAPI bool oProcessEnum(oFUNCTION<bool(unsigned int _ProcessID, unsigned int _ParentProcessID, const char* _ProcessExePath)> _Function);

// Wait for a process to exit/finish
oAPI bool oProcessWaitExit(unsigned int _ProcessID, unsigned int _TimeoutMS = oINFINITE_WAIT);

// Returns the id of specified process, or 0 if none found
oAPI unsigned int oProcessGetID(const char* _Name);

// Returns true if the specified process exists, or false if it does not
inline bool oProcessExists(const char* _Name) { return 0 != oProcessGetID(_Name); }

// Returns true if the specified process has a debugger, remote or otherwise,
// attached.
oAPI bool oProcessHasDebuggerAttached(unsigned int _ProcessID);

// Unceremoniously ends the specified process
oAPI bool oProcessTerminate(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive = true);

// Unceremoniously end all processes that are children of the current process.
oAPI void oProcessTerminateChildren(unsigned int _ProcessID, unsigned int _ExitCode, bool _Recursive = true);

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
oAPI bool oProcessGetMemoryStats(unsigned int _ProcessID, oPROCESS_MEMORY_STATS* _pStats);

struct oPROCESS_TIME_STATS
{
	time_t StartTime;
	time_t ExitTime;
	time_t KernelTime;
	time_t UserTime;
};

oAPI bool oProcessGetTimeStats(unsigned int _ProcessID, oPROCESS_TIME_STATS* _pStats);

// Returns overall system CPU percentage [0,100] usage. This requires keeping
// values around from a previous run. These should be initialized to 0 to 
// indicate there's no prior value and from then on this function should be 
// called regularly with those addresses passed in to retain the previous 
// sample's values. The values should be unique per process queried.
oAPI double oProcessCalculateCPUUsage(unsigned int _ProcessID, unsigned long long* _pPreviousSystemTime, unsigned long long* _pPreviousProcessTime);

struct oPROCESS_ALLOC_DESC
{
	enum TYPE
	{
		LIBC,
		PROCESS,
		EXTERNAL,
	};

	void* BaseAddress;
	unsigned long long Size;
	unsigned long long Overhead;
	TYPE Type;
	bool Used;
};

// Walks all allocations in all heaps associated with the current process
oAPI void oProcessEnumHeapAllocations(oFUNCTION<void(oPROCESS_ALLOC_DESC& _Desc)> _Walker);

#endif
