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
#include <oooii/oCommandQueue.h>
#include <oooii/oAtomic.h>
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>

oCommandQueue::oCommandQueue()
	: Enabled(true)
{
}

oCommandQueue::~oCommandQueue()
{
	Flush();
}

void oCommandQueue::Flush(bool _AllowEnqueues) threadsafe
{
	Enabled = _AllowEnqueues;		 

	oCommandQueue* pThis = thread_cast<oCommandQueue*>(this); // safe because this is a read-only spinlock
	while (!pThis->Commands.empty())
		oYield();

	// Ensures Execute is fully finished unlocking the mutex after the queue has
	// been emptied.
	ConsumerLock.Lock();
	ConsumerLock.Unlock();

	Enabled = true;
}

void oCommandQueue::Enqueue(oFUNCTION<void()> _Command) threadsafe
{
	if (Enabled) // Queue can be disabled during a drain event
	{
		// Add this command to the queue
		ConsumerLock.Lock();
		oCommandQueue* pThis = thread_cast<oCommandQueue*>(this); // safe because of lock above
		pThis->Commands.push_back(_Command);
		size_t CurrentCommandCount = pThis->Commands.size();
		ConsumerLock.Unlock();

		// If this command is the only one in the queue kick of the execution
		if (1 == CurrentCommandCount)
			IssueCommand();
	}
}

void oCommandQueue::ExecuteNext() threadsafe
{
	// Threadcast is safe because we're just reading the front to get the command
	// to execute.  After executing threadsafety is ensured by the ConsumerLock.
	oCommandQueue* pThis = thread_cast<oCommandQueue*>(this);
	pThis->Commands.front()(); // Execute actual command

	// Push the command off the list and execute the next one if available
	ConsumerLock.Lock();
	
	pThis->Commands.pop_front();

	if (!pThis->Commands.empty())
		IssueCommand();

	ConsumerLock.Unlock();
}

void oCommandQueue::IssueCommand() threadsafe
{
	oIssueAsyncTask(oBIND(&oCommandQueue::ExecuteNext, this));
}

bool oCommandQueue::Empty()
{
	oRWMutex::ScopedLockRead lock(ConsumerLock);
	return Commands.empty();
}