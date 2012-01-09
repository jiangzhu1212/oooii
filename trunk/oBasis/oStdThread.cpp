// $(header)
#include <oBasis/oStdThread.h>
#include <oBasis/oStdAtomic.h>
#include <process.h>
#include "oWinHeaders.h"

typedef std::tr1::function<void()> oPROC;

oStd::thread::id oStd::this_thread::get_id()
{
	oStd::thread::id ID;
	*(unsigned int*)&ID = ::GetCurrentThreadId();
	return ID;
}

void oStd::this_thread::yield()
{
	#if defined(_WIN32) || defined(_WIN64)
		::SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void oStd::this_thread::__sleep_for(unsigned int _Milliseconds)
{
	::Sleep(_Milliseconds);
}

oStd::thread::id::id()
	: ID(UINT_MAX)
{}

bool std::operator==(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x == *(unsigned int*)&y; }
bool std::operator!=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x != *(unsigned int*)&y; }
bool std::operator<(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x < *(unsigned int*)&y; }
bool std::operator<=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x <= *(unsigned int*)&y; }
bool std::operator>(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x > *(unsigned int*)&y; }
bool std::operator>=(oStd::thread::id x, oStd::thread::id y) { return *(unsigned int*)&x >= *(unsigned int*)&y; }

unsigned int oStd::thread::hardware_concurrency()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

oStd::thread::thread()
	: hThread(nullptr)
{}

static void __cdecl ostd_thread_proc(void* lpdwThreadParam)
{
	oPROC* pFN = (oPROC*)lpdwThreadParam;
	(*pFN)();
	pFN->~oPROC();
	_free_dbg(pFN, _CRT_BLOCK);
	_endthread();
}

void oStd::thread::initialize(std::tr1::function<void()> _ThreadProc)
{
	// Because a move operator may null out or change ThreadProc, we
	// need to make a separate, persistent copy. Also allocate this in a way that
	// doesn't show up as a false positive on leak reports. Since this is 
	// std::thread, flag the alloc as a CRT allocation because I imagine when
	// MSVC implements this it will be.
	oPROC* pProc = new (_malloc_dbg(sizeof(oPROC), _CRT_BLOCK, __FILE__, __LINE__)) std::tr1::function<void()>();
	*pProc = _ThreadProc;
	hThread = (void*)_beginthread(ostd_thread_proc, 64*1024, pProc);
}

#ifdef oHAS_MOVE_CTOR
	oStd::thread::thread(thread&& _That)
	{
		*this = std::move(_That);
	}

	oStd::thread& oStd::thread::operator=(thread&& _That)
	{
		return move_operator_eq(_That);
	}
#endif

void oStd::thread::move_ctor(thread& _This, thread& _That)
{
	if (&_This != &_That)
	{
		_This.hThread = _That.hThread;
		_That.hThread = nullptr;
	}
}

oStd::thread& oStd::thread::move_operator_eq(thread& _That)
{
	move_ctor(*this, _That);
	return *this;
}

oStd::thread::~thread()
{
	if (joinable())
	{
		oASSERT(false, "Calling std::terminate because a joinable thread was destroyed");
		std::terminate();
	}
}

void std::swap(oStd::thread& _this, oStd::thread& _That)
{
	_this.swap(_That);
}

void oStd::thread::swap(thread& _That)
{
	std::swap(hThread, _That.hThread);
}

void oStd::thread::detach()
{
	oStd::atomic_exchange(&hThread, 0);
}

void oStd::thread::join()
{
	if (joinable())
		::WaitForSingleObject(hThread, INFINITE);
}

bool oStd::thread::joinable() const
{
	DWORD exitCode = 0;
	if (!GetExitCodeThread(hThread, &exitCode))
		return false;
	return exitCode == STILL_ACTIVE;
}

oStd::thread::id oStd::thread::get_id() const
{
	oStd::thread::id ID;
	unsigned int intID = ::GetThreadId(hThread);
	if (intID)
		*(unsigned int*)&ID = intID;
	return ID;
}

oStd::thread::native_handle_type oStd::thread::native_handle()
{
	return hThread;
}
