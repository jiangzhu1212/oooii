// $(header)
// Approximation of the upcoming C++11 condition_variable interface.
// NOTE: According to the C++11 spec, spurious wakeups are allowable. The
// Windows implementation documents that spurious wakeups indeed occur. Use
// the Guarded Suspension pattern (Google it) to address this issue.

// !  !  ! ! !!!!!!  NOTE  !!!!!! ! !  !  !
// This is currently just a type-up of my reading of the spec and some example
// code on the web. duration_cast and time_point_cast are still suspect, so if
// you're using this for the first time, take care and assume none of this 
// works, especially the time casts.

#pragma once
#ifndef oStdConditionVariable_h
#define oStdConditionVariable_h

#include <oBasis/oStdChrono.h>
#include <oBasis/oStdMutex.h>

// To keep the main classes neat, collect all the platform-specific forward
// declaration here. This is done in this vague manner to avoid including 
// platform headers in this file.
#if defined(_WIN32) || defined(_WIN64)
	#define oCONDITION_VARIABLE_FOOTPRINT() void* Footprint;
#else
	#error Unsupported platform (oCONDITION_VARIABLE_FOOTPRINT)
#endif

namespace oStd {

enum /*class*/ cv_status { no_timeout, timeout };

class condition_variable
{
	oCONDITION_VARIABLE_FOOTPRINT();

	condition_variable(condition_variable const&); /* = delete */
	condition_variable& operator=(condition_variable const&); /* = delete */

	cv_status wait_for(unique_lock<mutex>& _Lock, unsigned int _TimeoutMS);

public:
	condition_variable();
	~condition_variable();

	void notify_one();
	void notify_all();

	void wait(unique_lock<mutex>& _Lock);

	template <typename Predicate>
	void wait(unique_lock<mutex>& _Lock, Predicate _Predicate)
	{
		while (!_Predicate())
			wait(_Lock);
	}

	template <typename Clock, typename Duration>
	cv_status wait_until(unique_lock<mutex>& _Lock, const chrono::time_point<Clock, Duration>& _AbsoluteTime)
	{
		chrono::high_resolution_clock::duration duration = time_point_cast<chrono::high_resolution_clock::time_point>(_AbsoluteTime) - chrono::high_resolution_clock::now();
		return wait_for(_Lock, duration);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool wait_until(unique_lock<mutex>& _Lock, const chrono::time_point<Clock, Duration>& _AbsoluteTime, Predicate _Predicate)
	{
		oASSERT(_Lock.owns_lock(), "Lock must own the mutex lock");
		while (!_Predicate())
		{
			if (wait_until(_Lock, _AbsoluteTime) == timeout)
				return _Predicate();
		}

		return true;
	}

	template <typename Rep, typename Period>
	cv_status wait_for(unique_lock<mutex>& _Lock, const chrono::duration<Rep, Period>& _RelativeTime)
	{
		chrono::milliseconds ms = chrono::duration_cast<chrono::milliseconds>(_RelativeTime);
		return wait_for(_Lock, static_cast<unsigned int>(ms.count()));
	}

	template <typename Rep, typename Period, typename Predicate>
	bool wait_for(unique_lock<mutex>& _Lock, const chrono::duration<Rep, Period>& _RelativeTime, Predicate _Predicate)
	{
		while (!_Predicate())
		{
			cv_status status = wait_for(_Lock, _RelativeTime);
			if (status == timeout)
				return _Predicate();
		}
		return true;
	}

	typedef void* native_handle_type;
	native_handle_type native_handle() { return Footprint; }
};

void notify_all_at_thread_exit(condition_variable& _ConditionVariable, unique_lock<mutex> _Lock);

} // namespace oStd

#endif
