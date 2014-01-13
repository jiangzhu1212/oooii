/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// Synchronization object often described as a reverse semaphore. This object
// gets initialized with a count and gives the system API to decrement the 
// count. When the count reaches 0, this object becomes unblocked. This object
// must be manually reset to a new count in order to be reused.
#pragma once
#ifndef oBase_countdown_latch_h
#define oBase_countdown_latch_h

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdexcept>

namespace ouro {

class countdown_latch
{
public:
	// In either construction or reset(), setting _InitialCount to a negative 
	// number will immediately set the underlying event. Setting _InitialCount
	// to zero or above will reset the event and allow functionality to proceed,
	// so setting the event to zero will require a reference then a release to 
	// really make sense.
	countdown_latch(int _InitialCount);

	// Returns the count of outstanding references on this latch. This should only 
	// be used for a user-facing update/UI or debug spew. This should not be used 
	// in control logic, use wait() instead.
	int outstanding() const;
	
	// Sets _InitialCount as it was done during the constructor. This is 
	// in the sense that it will change the current state of the latch,
	// but client code calling this while a wait() is just waking up means that if
	// the intent was to prevent that wake, then there's a race condition in the
	// client code. Use this only when it is know there aren't concurrent calls to 
	// reference or release or wait going on.
	void reset(int _InitialCount);
	
	// For systems that cannot know an initial count, this will make the latch 
	// require one more release() call before unblocking. This is not a preferred
	// practice because there is an ABA race condition where between the decision 
	// to go from 1 to 0 to thus trigger the event and actually triggering the 
	// event, another reference may sneak in. This means the system isn't well-
	// behaved and should be programmed to not accept any new references (new 
	// work) while a thread is waiting an any queued tasks to flush. One way to do 
	// this is to keep a scheduling reference separate from any of the work 
	// references and only release the scheduling reference at a time known not to 
	// allow additional work references to be added, such as a frame-to-frame 
	// application.
	void reference();
	
	// Call this to reduce the number of references keeping the latch locked. The
	// common use case should be to set an initial count of expected work items,
	// then schedule that work asynchronously where each item calls release on the 
	// latch when done. Then in some flush code, wait on the latch and thus when
	// all work is done, all counts on the latch are released and the wait is 
	// unblocked.
	void release();

	// Block the calling thread indefinitely until the number of outstanding items 
	// reaches zero.
	void wait();

	// Block the calling thread for a time or until the number of outstanding 
	// items reaches zero.
	template<typename Rep, typename Period>
	std::cv_status::cv_status wait_for(const std::chrono::duration<Rep, Period>& _RelativeTime);

private:
	std::condition_variable ZeroReferences;
	std::mutex Mutex;
	int NumOutstanding;

	countdown_latch(const countdown_latch&); /* = delete */
	const countdown_latch& operator=(const countdown_latch&); /* = delete */
};

inline countdown_latch::countdown_latch(int _InitialCount)
	: NumOutstanding(_InitialCount)
{
	// to centralize code, have all operations on NumOutStanding go through 
	// release, so add one first so this all ends up being a noop
	NumOutstanding++;
	release();
}

inline int countdown_latch::outstanding() const
{
	return NumOutstanding;
}

inline void countdown_latch::reset(int _InitialCount)
{
	Mutex.lock();
	NumOutstanding = _InitialCount + 1;
	Mutex.unlock();
	release();
}

inline void countdown_latch::reference()
{
	std::lock_guard<std::mutex> Lock(Mutex);
	if (NumOutstanding <= 0)
		throw std::runtime_error(
		"countdown_latch::reference() called too late to keep any waiting threads "
		"blocked. This is a race condition in client code because references are "
		"being added after the countdown had finished. If possible, use reset or the "
		"ctor to set an initial count beforehand and do not use reference() at all. "
		"A classic semaphore does not have API to reference after initialization for "
		"this very reason, but there are cases where careful coding can allow for "
		"reference() calls to be appropriate.");
	NumOutstanding++;
}

inline void countdown_latch::release()
{
	std::lock_guard<std::mutex> Lock(Mutex);
	if (--NumOutstanding <= 0)
		ZeroReferences.notify_all();
}

inline void countdown_latch::wait()
{
	std::unique_lock<std::mutex> Lock(Mutex);
	while (NumOutstanding > 0) // Guarded Suspension
		ZeroReferences.wait(Lock);
}

template<typename Rep, typename Period>
std::cv_status::cv_status countdown_latch::wait_for(const std::chrono::duration<Rep, Period>& _RelativeTime)
{
	std::cv_status::cv_status status = std::cv_status::no_timeout;
	std::unique_lock<std::mutex> Lock(Mutex);
	while (std::cv_status::no_timeout == status && NumOutstanding > 0)
		status = ZeroReferences.wait_for(Lock, _RelativeTime);
	return status;
}

} // namespace ouro

#endif
