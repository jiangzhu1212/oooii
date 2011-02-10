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
#ifndef oProcess_h
#define oProcess_h

#include <oooii/oInterface.h>

interface oProcess : public oInterface
{
	struct DESC
	{
		DESC()
			: CommandLine(0)
			, EnvironmentString(0)
			, StdHandleBufferSize(0)
		{}

		const char* CommandLine;
		const char* EnvironmentString;
		
		// If 0, then a unique instance is created, no shared pipes, 
		// and thus WriteToStdin and ReadFromStdout will return 0 with
		// oGetLastError() set to EPIPE.
		size_t StdHandleBufferSize;
	};

	// Creates a process that has not yet started, but exists so any debugging
	// has a chance to put in a breakpoint in the new process.
	static bool Create(const DESC* _pDesc, threadsafe oProcess** _ppProcess);

	static size_t GetCurrentProcessID();
	static size_t GetProcessHandle(const char* _pName);

	virtual void Start() threadsafe = 0;
	virtual void Kill(int _ExitCode) threadsafe = 0;
	virtual bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe = 0;
	
	virtual bool GetExitCode(int* _pExitCode) const threadsafe = 0;

	virtual size_t WriteToStdin(const void* _pSource, size_t _SizeofWrite) threadsafe = 0;
	virtual size_t ReadFromStdout(void* _pDestination, size_t _SizeofRead) threadsafe = 0;
};

#endif
