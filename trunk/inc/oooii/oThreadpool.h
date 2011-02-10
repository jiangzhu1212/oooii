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
interface oThreadpool : public oInterface
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
