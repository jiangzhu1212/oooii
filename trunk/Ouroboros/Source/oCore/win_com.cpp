// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/windows/win_com.h>
#include <oCore/windows/win_error.h>
#include <oCore/process_heap.h>
#include <oCompiler.h>

#define WIN32_LEAN_AND_MEAN
#include <ObjBase.h>

namespace ouro {
	namespace windows {
		namespace com {

class context
{
public:
	context()
		: CallUninit(false)
	{
		CallUninit = SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED|COINIT_SPEED_OVER_MEMORY));
		if (!CallUninit)
			throw error();
	}

	~context() { if (CallUninit) CoUninitialize(); }

private:
	bool CallUninit;
};
			
void ensure_initialized()
{
	static oTHREAD_LOCAL context* sInstance = nullptr;
	if (!sInstance)
	{
		process_heap::find_or_allocate(
			"com::context"
			, process_heap::per_thread
			, process_heap::garbage_collected
			, [=](void* _pMemory) { new (_pMemory) context(); }
			, nullptr
			, &sInstance);
	}
}
		
		} // namespace com
	} // namespace windows
} // namespace ouro
