// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/mutex.h>
#include <oBase/backoff.h>
#include <stdexcept>

#if NTDDI_VERSION >= NTDDI_WIN7
	#define oHAS_SLIM_TRY_LOCK
#endif

static_assert(sizeof(ouro::recursive_mutex) == sizeof(CRITICAL_SECTION), "size mismatch");
#ifdef _DEBUG
	static_assert(sizeof(ouro::mutex) == sizeof(SRWLOCK) + sizeof(std::thread::id), "size mismatch");
#else
	static_assert(sizeof(ouro::mutex) == sizeof(SRWLOCK), "size mismatch");
#endif

#ifdef _DEBUG
	#define ASSIGN_TID() ThreadID = std::this_thread::get_id()
	#define ASSIGN_TID_CHECKED() do { if (Footprint && ThreadID == std::this_thread::get_id()) { throw std::logic_error("non-recursive already locked on this thread"); } ThreadID = std::this_thread::get_id(); } while(false)
	#define CLEAR_TID()	ThreadID = std::thread::id()
	#ifdef oHAS_SLIM_TRY_LOCK
		#define CHECK_UNLOCKED() if (!try_lock()) throw std::logic_error("mutex locked on destruction")
	#else
		#define CHECK_UNLOCKED()
	#endif
#else
	#define ASSIGN_TID()
	#define ASSIGN_TID_CHECKED()
	#define CLEAR_TID()
	#define CHECK_UNLOCKED()
#endif

namespace ouro {

	const adopt_lock_t adopt_lock;
	const defer_lock_t defer_lock;
	const try_to_lock_t try_to_lock;

mutex::mutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
}

mutex::~mutex()
{
	CHECK_UNLOCKED();
}

mutex::native_handle_type mutex::native_handle()
{
	return (PSRWLOCK)&Footprint;
}

void mutex::lock()
{
	ASSIGN_TID_CHECKED();
	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);
}

bool mutex::try_lock()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void mutex::unlock()
{
	CLEAR_TID();
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

void shared_mutex::lock_shared()
{
	ASSIGN_TID_CHECKED();
	AcquireSRWLockShared((PSRWLOCK)&Footprint);
}

bool shared_mutex::try_lock_shared()
{
	#ifdef oHAS_SLIM_TRY_LOCK
		return !!TryAcquireSRWLockShared((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void shared_mutex::unlock_shared()
{
	CLEAR_TID();
	ReleaseSRWLockShared((PSRWLOCK)&Footprint);
}

recursive_mutex::recursive_mutex()
{
	InitializeCriticalSection((LPCRITICAL_SECTION)Footprint);
}

recursive_mutex::~recursive_mutex()
{
	CHECK_UNLOCKED();
	DeleteCriticalSection((LPCRITICAL_SECTION)Footprint);
}

recursive_mutex::native_handle_type recursive_mutex::native_handle()
{
	return (LPCRITICAL_SECTION)Footprint;
}

void recursive_mutex::lock()
{
	EnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool recursive_mutex::try_lock()
{
	return !!TryEnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

void recursive_mutex::unlock()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool timed_mutex::try_lock_for(unsigned int _TimeoutMS)
{
	// Based on:
	// http://software.intel.com/en-us/blogs/2008/09/17/pondering-timed-mutex/

	ouro::backoff bo;

	do 
	{
		if (try_lock())
			return true;

		if (!bo.try_pause())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			_TimeoutMS -= 10;
			bo.reset();
		}

	} while ((int)_TimeoutMS > 0);

	return false;
}

once_flag::once_flag()
	: Footprint(0)
{
	InitOnceInitialize((PINIT_ONCE)Footprint);
}

BOOL CALLBACK InitOnceCallback(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context)
{
	std::function<void()>* pFN = (std::function<void()>*)Parameter;
	(*pFN)();
	return TRUE;
}

void call_once(once_flag& _Flag, const std::function<void()>& _Function)
{
	InitOnceExecuteOnce(*(PINIT_ONCE*)&_Flag, InitOnceCallback, (PVOID)&_Function, nullptr);
}

} // namespace ouro
