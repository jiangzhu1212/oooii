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

// Interface for a typical multi-threaded sync event. This also has supported
// for multi-process synchronization through the use of named events.
#pragma once
#ifndef oEvent_h
#define oEvent_h

#include <oooii/oNoncopyable.h>
#include <oooii/oStddef.h>

class oEvent : oNoncopyable
{
	void* hEvent;
	#ifdef _DEBUG
		char DebugName[64];
	#endif
public:

	// It is advised to almost always assign a debug name so that triggered events 
	// can be more well understood while debugging many-threaded applications.
	// Specify _InterProcessName to create a system-global event that different
	// processes can attach to by each creating an event of the same name.

	// Two separate ctors are required because compiler can decide to convert a 
	// char pointer to a bool causing unexpected behavior
	oEvent(bool _AutoReset, const char* _DebugName = 0, const char* _InterProcessName = 0);
	oEvent(const char* _DebugName = 0, const char* _InterProcessName = 0);
	~oEvent();
	inline void* GetNativeHandle() threadsafe { return hEvent; }
	inline const char* GetDebugName() const threadsafe
	{
		#ifdef _DEBUG
			return thread_cast<const char*>(DebugName);
		#else
			return "";
		#endif
	}

	void Set() threadsafe;
	void Reset() threadsafe;

	// Returns false if timed out
	bool Wait(unsigned int _TimeoutMS = oINFINITE_WAIT) threadsafe;

	// Returns false if timed out. If _pWaitBreakingEventIndex is null, this waits
	// for all events to be set. If not null, then the pointer is filled with the
	// index of the first object to break the wait.
	static bool WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, size_t* _pWaitBreakingEventIndex = nullptr, unsigned int _TimeoutMS = oINFINITE_WAIT);
private:
	void CommonConstructor(bool _AutoReset, const char* _DebugName, const char* _InterProcessName);
};

#endif
