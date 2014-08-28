/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
