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

	oEvent(const char* _DebugName = 0);
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

	// Returns TIMED_OUT if timed out, else the index of the event that broke the wait
	static size_t WaitMultiple(threadsafe oEvent** _ppEvents, size_t _NumEvents, bool _WaitAll = true, unsigned int _TimeoutMS = oINFINITE_WAIT);
};

#endif
