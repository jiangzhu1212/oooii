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

// Abstraction for a platform process. All processes are started in a 
// suspended state so a debugger can be attached, so Start() must
// be called after an oProcess is created. Also if this interface is
// destroyed, the process itself may persist. Use Wait(), Kill() 
// and/or GetExitCode() to control the process's actual lifetime.
#pragma once
#ifndef oProcess_h
#define oProcess_h

#include <oooii/oInterface.h>

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
		// oGetLastError() set to EPIPE.
		size_t StdHandleBufferSize;
		bool SetFocus:1;
		bool StartMinimized:1;
	};

	static bool Create(const DESC* _pDesc, threadsafe oProcess** _ppProcess);

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

#endif
