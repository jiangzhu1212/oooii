/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Interface for working with the task/job system. oTaskSerial functions are 
// convenience functions provided so that parallel code can be more easily 
// debugged during development.

// NOTE: Tradeoff rational. At this time (C++11 just got approved) there is no
// generic/truly-cross-platform async/job system. (TBB isn't available for Wii
// or PS3), so really oTask should be a platform header and thus not included
// in oBasis, which should comprise of generic-only code. However there is code
// and more to come that is purely abstract/mathematical, but uses parallelism
// for performance. Why make algorithmic code part of a platform library when
// it may indeed be only a call to oParallelFor that keeps it from being generic?
// So make the call here: The API is declared in oBasis and thus can be used by
// generic code, but client code will get link errors. If oTaskParallelFor is
// implemented merely by wrapping oTaskSerialFor with it, then that is a 
// platform decision, but the algorithms are still valid. Port it to concrt, A 
// threadpool implementation, or TBB, or Sony's Spurs or whatever, the generic 
// code will run as efficiently as possible. (Well OK, maybe not on segmented 
// systems like Spurs/PS3.) The key thinking here in the tradeoff is it's ok to 
// have link errors if it better enables guarantees that oBasis code is 
// developed in a scalable manner.

#pragma once
#ifndef oTask_h
#define oTask_h

#include <oBasis/oFunction.h>
#include <oBasis/oInterface.h>

// For debugging so code can be kept similar to oParallelFor usage, this calls
// the _Function in order.
inline void oTaskSerialFor(size_t _Begin, size_t _End, oINDEXED_TASK _Function)
{
	for (size_t i = _Begin; i < _End; i++)
		_Function(i);
}

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
void oTaskParallelFor(size_t _Begin, size_t _End, oINDEXED_TASK&& _Function);

// Initialize the global scheduler and its memory.  This is optional, but
// should be called at init as it allocates memory which may be detected as a 
// leak.
void oTaskInitScheduler();

// When threads outside of the underlying task/job system call into the task 
// system they need a certain amount of memory for the thread to function 
// properly. Calling this ensures the memory is allocated at call time. Also the 
// specified string will be set to the debugger's list of threads.
void oTaskRegisterThisThread(const char* _DebuggerName);




// oIssueAsyncTask inserts a task to be executed by the TBB task
// scheduler.  Tasks should be non-blocking (no slow file io ) and synchronization 
// should be kept to a minimum.  No dependencies are exposed to keep things simple.
void DEPRECATED_oTaskIssueAsync(oTASK _Task);

// Runs the specified task (a drop-in debug replacement for oTaskIssueAsync)
inline void DEPRECATED_oTaskIssueSerial(oTASK _Task) { _Task(); }

#endif
