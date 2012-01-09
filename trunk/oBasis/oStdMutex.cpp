// $(header)
#include <oBasis/oStdMutex.h>
#include <oBasis/oBackoff.h>
#include "oWinHeaders.h"
#include <crtdbg.h>

// Use low-level assertions in here because
// A. it protects against any complexities in user implementations of oASSERT
// B. When this is replaced with code from the compiler vendor we won't have control anyway

#ifdef _DEBUG
	#define oCRTASSERT(expr, msg, ...) if (!(expr)) { if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, "OOOii Debug Library", #expr "\n\n" msg, ## __VA_ARGS__)) oDEBUGBREAK(); }
#else
	#define oCRTASSERT(expr, msg, ...) __noop
#endif

static_assert(sizeof(oStd::recursive_mutex) == sizeof(CRITICAL_SECTION), "");
#ifdef _DEBUG
	static_assert(sizeof(oStd::mutex) == sizeof(SRWLOCK) + sizeof(size_t), "");
#else
	static_assert(sizeof(oStd::mutex) == sizeof(SRWLOCK), "");
#endif

oStd::mutex::mutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
}

oStd::mutex::~mutex()
{
	#ifdef oHAS_SHMUTEX_TRYLOCK
		if (!try_lock())
			oCRTASSERT(false, "mutex is locked on destruction: this could result in a deadlock or race condition.");
	#endif
}

oStd::mutex::native_handle_type oStd::mutex::native_handle()
{
	return (PSRWLOCK)&Footprint;
}

void oStd::mutex::lock()
{
	// @oooii-tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != oStd::this_thread::get_id(), "mutex is non-recursive and already locked on this thread. This could result in a deadlock.");
	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = oStd::this_thread::get_id();
	#endif
}

bool oStd::mutex::try_lock()
{
	#ifdef oHAS_SHMUTEX_TRYLOCK
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void oStd::mutex::unlock()
{
	#ifdef _DEBUG
		ThreadID = oStd::thread::id();
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

oStd::recursive_mutex::recursive_mutex()
{
	InitializeCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oStd::recursive_mutex::~recursive_mutex()
{
	if (!try_lock())
	{
		oCRTASSERT(false, "recursive_mutex is locked on destruction: this could result in a deadlock or race condition.");
		unlock();
	}

	DeleteCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oStd::recursive_mutex::native_handle_type oStd::recursive_mutex::native_handle()
{
	return (LPCRITICAL_SECTION)Footprint;
}

void oStd::recursive_mutex::lock()
{
	EnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

bool oStd::recursive_mutex::try_lock()
{
	return !!TryEnterCriticalSection((LPCRITICAL_SECTION)Footprint);
}

void oStd::recursive_mutex::unlock()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)Footprint);
}

oStd::shared_mutex::shared_mutex()
{
	InitializeSRWLock((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = oStd::thread::id();
	#endif
}

oStd::shared_mutex::~shared_mutex()
{
	#ifdef oHAS_SHMUTEX_TRYLOCK
		if (!try_lock())
			oCRTASSERT(false, "shared_mutex is locked on destruction: this could result in a deadlock or race condition.");
	#endif
}

oStd::shared_mutex::native_handle_type oStd::shared_mutex::native_handle()
{
	return (PSRWLOCK)&Footprint;
}

void oStd::shared_mutex::lock()
{
	// @oooii-tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != oStd::this_thread::get_id(), "shared_mutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");
	AcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = oStd::this_thread::get_id();
	#endif
}

bool oStd::shared_mutex::try_lock()
{
	#ifdef oHAS_SHMUTEX_TRYLOCK
		return !!TryAcquireSRWLockExclusive((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void oStd::shared_mutex::unlock()
{
	#ifdef _DEBUG
		ThreadID = oStd::thread::id();
	#endif
	ReleaseSRWLockExclusive((PSRWLOCK)&Footprint);
}

void oStd::shared_mutex::lock_shared()
{
	// @oooii-tony: Based on what I've observed, the low bit of the word that is
	// the slim RW lock is 1 when locked and 0 when not locked, so test for that...
	oCRTASSERT(!Footprint || ThreadID != oStd::this_thread::get_id(), "shared_mutex is non-recursive and already read/shared locked on this thread. This could result in a deadlock.");
	AcquireSRWLockShared((PSRWLOCK)&Footprint);
	#ifdef _DEBUG
		ThreadID = oStd::this_thread::get_id();
	#endif
}

bool oStd::shared_mutex::try_lock_shared()
{
	#ifdef oHAS_SHMUTEX_TRYLOCK
		return !!TryAcquireSRWLockShared((PSRWLOCK)&Footprint);
	#else
		return false;
	#endif
}

void oStd::shared_mutex::unlock_shared()
{
	#ifdef _DEBUG
		ThreadID = oStd::thread::id();
	#endif
	ReleaseSRWLockShared((PSRWLOCK)&Footprint);
}

bool oStd::timed_mutex::try_lock_for(unsigned int _TimeoutMS)
{
	// Based on:
	// http://software.intel.com/en-us/blogs/2008/09/17/pondering-timed-mutex/

	oBackoff bo;

	do 
	{
		if (Mutex.try_lock())
			return true;

		if (!bo.TryPause())
		{
			Sleep(1);
			_TimeoutMS--;
			bo.Reset();
		}

	} while (_TimeoutMS > 0);

	return false;
}

oStd::once_flag::once_flag()
	: Footprint(0)
{
	InitOnceInitialize((PINIT_ONCE)Footprint);
}

BOOL CALLBACK InitOnceCallback(PINIT_ONCE InitOnce, PVOID Parameter, PVOID* Context)
{
	std::tr1::function<void()>* pFN = (std::tr1::function<void()>*)Parameter;
	(*pFN)();
	return TRUE;
}

void oStd::call_once(oStd::once_flag& _Flag, std::tr1::function<void()> _Function)
{
	InitOnceExecuteOnce(*(PINIT_ONCE*)&_Flag, InitOnceCallback, &_Function, nullptr);
}
