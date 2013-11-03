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
#include <oCore/process_heap.h>
#include <oCore/debugger.h>
#include <oCore/module.h>
#include <oCore/process.h>
#include <oCore/system.h>
#include <oBase/guid.h>
#include <oBase/fnv1a.h>
#include <oStd/mutex.h>

namespace ouro {
	namespace process_heap {

inline size_t hash(const char* _Name, process_heap::scope _Scope)
{
	size_t h = fnv1a<size_t>(_Name);
	if (_Scope == process_heap::per_thread)
	{
		oStd::thread::id id = oStd::this_thread::get_id();
		h = fnv1a<size_t>(&id, sizeof(oStd::thread::id), h);
	}
	return h;
}

class context
{
public:
	static context& singleton();

	context();

	void reference();
	void release();

	void* allocate(size_t _Size);
	void deallocate(void* _Pointer);

	bool find_or_allocate(size_t _Size
		, const char* _Name
		, scope _Scope
		, tracking _Tracking
		, const std::function<void(void* _Pointer)>& _PlacementConstructor
		, const std::function<void(void* _Pointer)>& _Destructor
		, void** _pPointer);

	bool find(const char* _Name, scope _Scope, void** _pPointer);

	void exit_thread();

	void report();

private:
	static const size_t max_stack_depth = 32;
	static const size_t std_bind_internal_offset = 5; // number of symbols internal to std::bind to skip
	static bool valid;
	static context* sAtExitInstance;

	HANDLE hHeap;
	oStd::recursive_mutex Mutex;
	int RefCount;

	struct mapped_file
	{
		context* instance;
		guid guid;
		process::id pid;
	};
	
	struct entry
	{
		entry()
			: pointer(nullptr)
		{
			InitializeSRWLock(&mutex);
		}

		// The allocated pointer
		void* pointer;

		// thread id of pointer initialization and where deinitialization will 
		// probably take place
		oStd::thread::id init;
		sstring name;
		enum scope scope;
		enum tracking tracking;
		debugger::symbol stack[max_stack_depth];
		size_t num_stack_entries;
		SRWLOCK mutex; // std::mutex can't support copy or move, so need to use platform type directly

		void lock() { AcquireSRWLockExclusive(&mutex); }
		void unlock() { ReleaseSRWLockExclusive(&mutex); }
		void lock_shared() { AcquireSRWLockShared(&mutex); }
		void unlock_shared() { ReleaseSRWLockShared(&mutex); }
	};

	struct matches
	{
		matches(void* _Pointer) : Pointer(_Pointer) {}
		bool operator()(const std::pair<size_t, entry>& _Entry) { return _Entry.second.pointer == Pointer; }
		void* Pointer;
	};

	// Main container of all pointers allocated from the process heap
	typedef std::map<size_t, entry, std::less<size_t>, process_heap::std_allocator<std::pair<size_t, entry>>> container_t;
	container_t Pointers;

	static void report_footer(size_t _NumLeaks);
	static void at_exit();
};

context::context()
	: hHeap(GetProcessHeap())
	, RefCount(1)
{
	// From oGSReport.cpp: This touches the CPP file ensuring our __report_gsfailure is installed
	extern void oGSReportInstaller();
	oGSReportInstaller();

	sAtExitInstance = this;
	atexit(at_exit);
	valid = true;

	path modulePath = this_module::path();
	lstring buf;
	mstring exec;
	snprintf(buf, "%s(%d): {%s} %s process_heap initialized at 0x%p\n", __FILE__, __LINE__, modulePath.c_str(), system::exec_path(exec), this);
	debugger::print(buf);
}

bool context::valid = false;
context* context::sAtExitInstance = nullptr;

void context::report_footer(size_t _NumLeaks)
{
	lstring buf;
	mstring exec;
	snprintf(buf, "========== Process Heap Leak Report: %u Leaks %s ==========\n", _NumLeaks, system::exec_path(exec));
	debugger::print(buf);
}

void context::at_exit()
{
	context::singleton().exit_thread();

	if (valid)
		context::singleton().report();
	else
		report_footer(0);
}

void context::reference()
{
	oStd::atomic_increment(&RefCount);
}

void context::release()
{
	if (0 == oStd::atomic_decrement(&RefCount))
	{
		valid = false;
		this->~context();
		VirtualFreeEx(GetCurrentProcess(), this, 0, MEM_RELEASE);
	}
}

void* context::allocate(size_t _Size)
{
	return HeapAlloc(hHeap, 0, _Size);
}

void context::deallocate(void* _Pointer)
{
	bool DoRelease = false;
	{
		oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);

		if (valid)
		{
			auto it = std::find_if(Pointers.begin(), Pointers.end(), matches(_Pointer));
			if (it == Pointers.end())
 				HeapFree(hHeap, 0, _Pointer);
			else
			{
				// The order here is critical, we need to tell the heap to deallocate 
				// first, remove it from the list, exit the mutex and then release. If 
				// we release first we risk destroying the heap and then still needing 
				// to access it.
				HeapFree(hHeap, 0, it->second.pointer);
				Pointers.erase(it);
				DoRelease = true;
			}
		}

		else
		{
			// shutting down so this allocation must be from pPointers itself, so just 
			// free it
			HeapFree(hHeap, 0, _Pointer);
		}
	}

	if (DoRelease)
		release();
}

void context::exit_thread()
{
	fixed_vector<void*, 32> allocs;
	{
		oStd::thread::id tid = oStd::this_thread::get_id();
		oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);
		for (container_t::const_iterator it = Pointers.begin(); it != Pointers.end(); ++it)
			if (it->second.scope == process_heap::per_thread && tid == it->second.init)
				allocs.push_back(it->second.pointer);
	}

	oFOR(void* p, allocs)
		context::deallocate(p);
}

bool context::find_or_allocate(size_t _Size
	, const char* _Name
	, scope _Scope
	, tracking _Tracking
	, const std::function<void(void* _Pointer)>& _PlacementConstructor
	, const std::function<void(void* _Pointer)>& _Destructor
	, void** _pPointer)
{
	if (!_Size || !_pPointer)
		throw std::invalid_argument("invalid argument");
	size_t h = hash(_Name, _Scope);
	bool Allocated = false;
	bool CallCtor = false;

	// the constructor for this new object could call back into FindOrAllocate 
	// when creating its members. This can cause a deadlock so release the primary 
	// lock before constructing the new object and lock a shared mutex just for 
	// that entry.
	{
		oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);
		auto it = Pointers.find(h);
		if (it == Pointers.end())
		{
			entry& e = Pointers[h];
			*_pPointer = e.pointer = allocate(_Size);
			e.init = oStd::this_thread::get_id();
			e.name = _Name;
			e.scope = _Scope;
			e.tracking = _Tracking;
			e.num_stack_entries = debugger::callstack(e.stack, 3);
			reference();

			Allocated = true;
			if (_PlacementConstructor)
			{
				CallCtor = true;
				AcquireSRWLockExclusive(&e.mutex);
			}
		}
		else
		{
			AcquireSRWLockShared(&it->second.mutex); // may exist but not be constructed yet
			*_pPointer = it->second.pointer;
			ReleaseSRWLockShared(&it->second.mutex);
		}
	}

	if (CallCtor)
	{
		// Entry is already locked
		_PlacementConstructor(*_pPointer);

		// unlock the entry - how else can I do this!? I can't keep a pointer to the
		// entry because it can be moved around, so I need to safely reevaluate it.
		oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);
		auto it = Pointers.find(h);
		ReleaseSRWLockExclusive(&it->second.mutex);
	}
	
	return Allocated;
}

bool context::find(const char* _Name, scope _Scope, void** _pPointer)
{
	if (!_Name || !_pPointer)
		throw std::invalid_argument("invalid argument");
	*_pPointer = nullptr;
	size_t h = hash(_Name, _Scope);
	oStd::lock_guard<oStd::recursive_mutex> lock(Mutex);
	auto it = Pointers.find(h);
	if (it != Pointers.end())
		*_pPointer = it->second.pointer;
	return !!*_pPointer;
}

void context::report()
{
	// freeing of singletons is done with atexit(). So ignore leaks that were 
	// created on this thread because they will potentially be freed after this
	// report. The traces for singleton lifetimes should indicate thread id of 
	// freeing so if there are any after this report that don't match the thread
	// if of this report, that would be bad.

	// do a pre-scan to see if it's worth printing anything to the log

	unsigned int nLeaks = 0;
	unsigned int nIgnoredLeaks = 0;
	for (container_t::const_iterator it = Pointers.begin(); it != Pointers.end(); ++it)
	{
		if (it->second.tracking == process_heap::leak_tracked)
			nLeaks++;
		else
			nIgnoredLeaks++;
	}

	path moduleName = this_module::path();
	
	xlstring buf;
	
	if (nLeaks)
	{
		mstring exec;
		snprintf(buf, "========== Process Heap Leak Report %s (Module %s) ==========\n", system::exec_path(exec), moduleName.c_str());
		debugger::print(buf);
		for (container_t::const_iterator it = Pointers.begin(); it != Pointers.end(); ++it)
		{
			if (it->second.tracking == process_heap::leak_tracked)
			{
				const entry& e = it->second;

				mstring TLBuf;
				if (e.scope == process_heap::per_thread)
				{
					snprintf(TLBuf, " (thread_local in thread 0x%x%s)"
						, *(unsigned int*)&e.init
						, this_process::get_main_thread_id() == e.init ? " (main)" : "");
				}

				snprintf(buf, "%s%s\n", e.name.c_str(), e.scope == process_heap::per_thread ? TLBuf.c_str() : "");
				debugger::print(buf);

				bool IsStdBind = false;
				for (size_t i = 0; i < e.num_stack_entries; i++)
				{
					bool WasStdBind = IsStdBind;
					debugger::format(buf, e.stack[i], "  ", &IsStdBind);
					if (!WasStdBind && IsStdBind) // skip a number of the internal wrappers
						i += std_bind_internal_offset;
					debugger::print(buf);
				}
			}
		}
	}

	report_footer(nLeaks);

	// For ignored leaks, release those refs on the process heap
	for (unsigned int i = 0; i < nIgnoredLeaks; i++)
		release();
}

context& context::singleton()
{
	static context* sInstance = nullptr;
	if (!sInstance)
	{
		static const guid GUID_ProcessHeap = { 0x7c5be6d1, 0xc5c2, 0x470e, { 0x85, 0x4a, 0x2b, 0x98, 0x48, 0xf8, 0x8b, 0xa9 } }; // {7C5BE6D1-C5C2-470e-854A-2B9848F88BA9}

		// Filename is "<GUID><CurrentProcessID>"
		path_string MappedFilename;
		to_string(MappedFilename, GUID_ProcessHeap);
		sncatf(MappedFilename, ".%u", GetCurrentProcessId());

		// Create a memory-mapped File to store the instance location so it's 
		// accessible by all code modules (EXE, DLLs)
		SetLastError(ERROR_SUCCESS);
		HANDLE hMappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0
			, PAGE_READWRITE, 0, sizeof(context::mapped_file), MappedFilename);

		if (!hMappedFile)
			oTHROW(io_error, "could not create process_heap memory mapped file '%s'.", MappedFilename.c_str());
		context::mapped_file* file = 
			(context::mapped_file*)MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0, 0, 0);

		if (hMappedFile && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			// File already exists, loop until it has been initialized by the 
			// creating thread.
			while (file->pid != this_process::get_id() || file->guid != GUID_ProcessHeap)
			{
				UnmapViewOfFile(file);
				::Sleep(0);
				file = (context::mapped_file*)MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0, 0, 0);
			}
			sInstance = file->instance;
		}

		else if (hMappedFile) // this is the creating thread
		{
			// Allocate memory at the highest possible address then store that value 
			// in the memory mapped file for other DLLs to access.
			*(&sInstance) = static_cast<context*>(
				VirtualAllocEx(GetCurrentProcess(), 0, sizeof(context)
				, MEM_COMMIT|MEM_RESERVE|MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE));
			
			if (!sInstance)
				oTHROW(no_buffer_space, "process_heap VirtualAllocEx failed");

			new (sInstance) context();

			file->instance = sInstance;
			file->guid = GUID_ProcessHeap;
			file->pid = this_process::get_id();
		}

		UnmapViewOfFile(file);
	}

	return *sInstance;
}

void ensure_initialized()
{
	context::singleton();
}

void* allocate(size_t _Size)
{
	return context::singleton().allocate(_Size);
}

void deallocate(void* _Pointer)
{
	context::singleton().deallocate(_Pointer);
}

void exit_thread()
{
	context::singleton().exit_thread();
}

bool find_or_allocate(size_t _Size
	, const char* _Name
	, scope _Scope
	, tracking _Tracking
	, const std::function<void(void* _Pointer)>& _PlacementConstructor
	, const std::function<void(void* _Pointer)>& _Destructor
	, void** _pPointer)
{
	return context::singleton().find_or_allocate(_Size, _Name, _Scope, _Tracking
		, _PlacementConstructor, _Destructor, _pPointer);
}

bool find(const char* _Name, scope _Scope, void** _pPointer)
{
	return context::singleton().find(_Name, _Scope, _pPointer);
}

	} // namespace process_heap
} // namespace ouro
