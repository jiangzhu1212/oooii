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
#include <oCore/windows/win_iocp.h>
#include <oBase/backoff.h>
#include <oBase/concurrent_object_pool.h>
#include <oBase/invalid.h>
#include <oCore/debugger.h>
#include <oCore/process_heap.h>
#include <oCore/reporting.h>
#include <oCore/windows/win_error.h>
#include <oStd/thread.h>
#include <vector>

#define oSHUTDOWN 1
#define oCOMPLETION 2

using namespace oStd;

namespace ouro {
	namespace windows {

struct iocp_overlapped : public OVERLAPPED
{
	iocp_overlapped() { clear(); }

	inline void clear() { memset(this, 0, sizeof(OVERLAPPED)); hFile = nullptr; Task = nullptr; }

	HANDLE hFile;
	std::function<void(size_t _NumBytes)> Task;
};

class iocp_threadpool
{
public:
	static unsigned int concurrency();

	static iocp_threadpool& singleton();
	static void* find_instance();

	// Waits for all work to be completed
	void wait() { wait_for(infinite); }
	bool wait_for(unsigned int _TimeoutMS);
	void join();

	OVERLAPPED* associate(HANDLE _Handle, const std::function<void(size_t _NumBytes)>& _OnCompletion);
	void disassociate(OVERLAPPED* _pOverlapped);

private:
	iocp_threadpool() : hIoPort(nullptr), NumRunningThreads(0), NumAssociations(0) {}
	iocp_threadpool(size_t _OverlappedCapacity, size_t _NumWorkers = 0);
	~iocp_threadpool();
	iocp_threadpool(iocp_threadpool&& _That) { operator=(std::move(_That)); }
	iocp_threadpool& operator=(iocp_threadpool&& _That);

	void work();

	HANDLE hIoPort;
	std::vector<oStd::thread> Workers;
	std::atomic<size_t> NumRunningThreads;
	size_t NumAssociations;

	concurrent_object_pool<iocp_overlapped> pool;

	iocp_threadpool(const iocp_threadpool&); /* = delete; */
	const iocp_threadpool& operator=(const iocp_threadpool&); /* = delete; */
};

unsigned int iocp_threadpool::concurrency()
{
	return oStd::thread::hardware_concurrency();
}

void iocp_threadpool::work()
{
	debugger::thread_name("iocp worker");
	NumRunningThreads++;
	while (true)
	{
		DWORD nBytes = 0;
		ULONG_PTR key = 0;
		iocp_overlapped* ol = nullptr;
		if (GetQueuedCompletionStatus(hIoPort, &nBytes, &key, (OVERLAPPED**)&ol, INFINITE))
		{
			if (oSHUTDOWN == key)
				break;
			else if (oCOMPLETION == key)
			{
				if (ol->Task)
					ol->Task(nBytes);
			}

			else
				oTHROW(operation_not_supported, "CompletionKey %p not supported", key);
		}
		else if (ol)
		{
			if (ol->Task)
				ol->Task(0);
		}
	}
	NumRunningThreads--;
}

iocp_threadpool& iocp_threadpool::singleton()
{
	static iocp_threadpool* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"iocp"
			, process_heap::per_process
			, process_heap::leak_tracked
			, [=](void* _pMemory) { new (_pMemory) iocp_threadpool(oKB(512)); }
			, [=](void* _pMemory) { ((iocp_threadpool*)_pMemory)->~iocp_threadpool(); }
			, &sInstance);
	}
	return *sInstance;
}

void* iocp_threadpool::find_instance()
{
	void* pInstance = nullptr;
	process_heap::find("iocp", process_heap::per_process, &pInstance);
	return pInstance;
}

iocp_threadpool::iocp_threadpool(size_t _OverlappedCapacity, size_t _NumWorkers)
	: hIoPort(nullptr)
	, NumRunningThreads(0)
	, NumAssociations(0)
	, pool(_OverlappedCapacity)
{
	reporting::ensure_initialized();

	const size_t NumWorkers = _NumWorkers ? _NumWorkers : thread::hardware_concurrency();
	hIoPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(NumWorkers));
	oVB(hIoPort);

	Workers.resize(NumWorkers);
	auto worker = std::bind(&iocp_threadpool::work, this);
	NumRunningThreads = 0;
	oFOR(auto& w, Workers)
		w = std::move(thread(worker));

	backoff bo;
	while (NumRunningThreads != Workers.size())
		bo.pause();
}

iocp_threadpool::~iocp_threadpool()
{
	if (!wait_for(20000))
		throw std::runtime_error("timed out waiting for iocp completion");

	oFOR(auto& w, Workers)
		PostQueuedCompletionStatus(hIoPort, 0, oSHUTDOWN, nullptr);

	oFOR(auto& w, Workers)
		w.join();

	if (INVALID_HANDLE_VALUE != hIoPort)
	{
		CloseHandle(hIoPort);
	}
}

iocp_threadpool& iocp_threadpool::operator=(iocp_threadpool&& _That)
{
	if (this != &_That)
	{
		hIoPort = _That.hIoPort; _That.hIoPort = INVALID_HANDLE_VALUE;
		Workers = std::move(_That.Workers);
		NumRunningThreads.store(_That.NumRunningThreads); _That.NumRunningThreads = 0;
		NumAssociations = _That.NumAssociations; _That.NumAssociations = 0;
		pool = std::move(_That.pool);
	}
	return *this;
}

bool iocp_threadpool::wait_for(unsigned int _TimeoutMS)
{
	backoff bo;

	unsigned int start = timer::now_ms();

	#ifdef _DEBUG
		local_timeout to(5.0);
	#endif

	while (NumAssociations > 0)
	{ 
		if (_TimeoutMS != infinite && timer::now_ms() >= (start + _TimeoutMS))
			return false;

		bo.pause();

		#ifdef _DEBUG
			if (to.timed_out())
			{
				oTRACE("Waiting for %u outstanding iocp associations to finish...", NumAssociations);
				to.reset(5.0);
			}
		#endif
	}

	return true;
}

void iocp_threadpool::join()
{
	void* pInstance = find_instance();
	if (pInstance)
		process_heap::deallocate(pInstance);
}

OVERLAPPED* iocp_threadpool::associate(HANDLE _Handle, const std::function<void(size_t _NumBytes)>& _OnCompletion)
{
	NumAssociations++;
	iocp_overlapped* ol = pool.allocate();
	if (ol)
	{
		if (hIoPort != CreateIoCompletionPort(_Handle, hIoPort, oCOMPLETION, static_cast<DWORD>(Workers.size())))
		{
			disassociate(ol);
			oVB(false);
		}

		ol->hFile = _Handle;
		ol->Task = _OnCompletion;
	}

	else
		NumAssociations--;

	return ol;
}

void iocp_threadpool::disassociate(OVERLAPPED* _pOverlapped)
{
	iocp_overlapped* ol = static_cast<iocp_overlapped*>(_pOverlapped);
	ol->clear();
	pool.deallocate(ol);
	NumAssociations--;
}

namespace iocp {

unsigned int concurrency()
{
	return iocp_threadpool::concurrency();
}

void ensure_initialized()
{
	iocp_threadpool::singleton();
}

OVERLAPPED* associate(HANDLE _Handle, const std::function<void(size_t _NumBytes)>& _OnCompletion)
{
	return iocp_threadpool::singleton().associate(_Handle, _OnCompletion);
}

void disassociate(OVERLAPPED* _pOverlapped)
{
	iocp_threadpool::singleton().disassociate(_pOverlapped);
}

void wait()
{
	iocp_threadpool::singleton().wait();
}

bool wait_for(unsigned int _TimeoutMS)
{
	return iocp_threadpool::singleton().wait_for(_TimeoutMS);
}

bool joinable()
{
	return !!iocp_threadpool::find_instance();
}

void join()
{
	return iocp_threadpool::singleton().join();
}

		} // namespace iocp
	} // namespace windows
} // namespace ouro
