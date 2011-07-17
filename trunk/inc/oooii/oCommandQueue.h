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
// but a command could execute at any time on any thread.
#pragma once
#ifndef oCommandQueue_h
#define oCommandQueue_h

#include <oooii/oMutex.h>
#include <list>

struct oCommandQueue
{
	oCommandQueue();
	~oCommandQueue();

	// Enqueue a command to be executed
	void Enqueue(oFUNCTION<void()> _Command) threadsafe;

	// Block until all commands enqueued are executed. If _AllowEnqueues is true,
	// then Enqueue() calls will append to the command list and thus this Flush
	// might never return. If _AllowEnqueues is false, then Enqueue calls are 
	// ignored.
	void Flush(bool _AllowEnqueues = false) threadsafe;
protected:
	void ExecuteNext() threadsafe;
	bool Empty();
	void Disable() {Enabled = false;}
private:
	virtual void IssueCommand() threadsafe;
	std::list<oFUNCTION<void()>> Commands;
	oRWMutex ConsumerLock;
	volatile bool Enabled;
};

#endif
