// $(header)
// Thin wrapper for Std::oConditionVariable that provides threadsafe 
// specification to reduce thread_cast throughout the code. Prefer using 
// these wrappers over using std::oConditionVariable directly.
#pragma once
#ifndef oConditionVariable_h
#define oConditionVariable_h

#include <oBasis/oStdConditionVariable.h>
#include <oBasis/oMutex.h>
#include <oBasis/oThreadsafe.h>

class oConditionVariable
{
	oStd::condition_variable ConditionVariable;
	oStd::condition_variable& CV() threadsafe { return thread_cast<oStd::condition_variable&>(ConditionVariable); }
	oStd::unique_lock<oStd::mutex>& L(oUniqueLock<oMutex>& _Lock) threadsafe
	{
		return *(oStd::unique_lock<oStd::mutex>*)&_Lock;
	}

	oConditionVariable(oConditionVariable const&); /* = delete */
	oConditionVariable& operator=(oConditionVariable const&); /* = delete */

	oStd::cv_status wait_for(oUniqueLock<oMutex>& _Lock, unsigned int _TimeoutMS);

public:
	oConditionVariable() {}
	~oConditionVariable() {}

	void notify_one() threadsafe { CV().notify_one(); }
	void notify_all() threadsafe { CV().notify_all(); }

	void wait(oUniqueLock<oMutex>& _Lock) threadsafe { CV().wait(L(_Lock)); }

	template <typename Predicate>
	void wait(oUniqueLock<oMutex>& _Lock, Predicate _Predicate) threadsafe
	{
		CV().wait(L(_Lock), _Predicate);
	}

	template <typename Clock, typename Duration>
	oStd::cv_status wait_until(oUniqueLock<oMutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime) threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime);
	}

	template <typename Clock, typename Duration, typename Predicate>
	bool wait_until(oUniqueLock<oMutex>& _Lock, const oStd::chrono::time_point<Clock, Duration>& _AbsoluteTime, Predicate _Predicate) threadsafe
	{
		return CV().wait_until(L(_Lock), _AbsoluteTime, _Predicate);
	}

	template <typename Rep, typename Period>
	oStd::cv_status wait_for(oUniqueLock<oMutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime) threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime);
	}

	template <typename Rep, typename Period, typename Predicate>
	bool wait_for(oUniqueLock<oMutex>& _Lock, const oStd::chrono::duration<Rep, Period>& _RelativeTime, Predicate _Predicate) threadsafe
	{
		return CV().wait_for(L(_Lock), _RelativeTime, _Predicate);
	}

	typedef oStd::condition_variable::native_handle_type native_handle_type;
	native_handle_type nativfe_handle() threadsafe { return CV().native_handle(); }
};

#endif
