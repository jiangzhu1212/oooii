// $(header)
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