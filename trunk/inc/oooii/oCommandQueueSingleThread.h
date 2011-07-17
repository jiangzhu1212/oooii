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
// Enqueues commands such that they are executed in the order they were enqueued,
// A single thread will be created, and all commands will execute on this new thread.
// Should almost always use oCommandQueue instead of this class. Use this class
// only if you have a really good reason to want commands to always be executed
// from the same thread.
#pragma once
#ifndef oCommandQueueSingleThread_h
#define oCommandQueueSingleThread_h

#include <oooii/oCommandQueue.h>
#include <oooii/oThread.h>
#include <oooii/oRef.h>
#include <oooii/oEvent.h>

class oCommandQueueProc;

struct oCommandQueueSingleThread : public oCommandQueue
{
	oCommandQueueSingleThread();
	~oCommandQueueSingleThread();

private:
	void IssueCommand() threadsafe override;
	oRef<oCommandQueueProc> ThreadProc;
	oRef<threadsafe oThread> Thread;
};

#endif
