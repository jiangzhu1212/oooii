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
#include <oCore/windows/win_skeleton.h>
#include <oCore/mutex.h>
#include <oCore/process_heap.h>

namespace ouro {
	namespace windows {
		namespace skeleton {
			
class context
{
public:
	static context& singleton();

	void register_source(handle _hSkeleton, const std::function<void(bone_info* _pSkeleton)>& _Get)
	{
		lock_guard<shared_mutex> lock(Mutex);
		if (Sources.find(_hSkeleton) != Sources.end())
			throw std::exception("redundant registration");
		Sources[_hSkeleton] = _Get;
	}

	void unregister_source(handle _hSkeleton)
	{
		lock_guard<shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it != Sources.end())
			Sources.erase(it);
	}

	bool get_info(handle _hSkeleton, bone_info* _pSkeleton)
	{
		shared_lock<shared_mutex> lock(Mutex);
		auto it = Sources.find(_hSkeleton);
		if (it == Sources.end())
			oTHROW_INVARG0();
		it->second(_pSkeleton);
		return true;
	}

private:
	shared_mutex Mutex;
	std::map<handle
		, std::function<void(bone_info* _pSkeleton)>
		, std::less<handle>
		, process_heap::std_allocator<std::pair<handle, std::function<void(bone_info* _pSkeleton)>>>>
	Sources;

	context() {}
	~context()
	{
		oASSERT(Sources.empty(), "");
	}
};

oDEFINE_PROCESS_SINGLETON("ouro::windows::win_skeleton", context);

void register_source(handle _handle, const std::function<void(bone_info* _pSkeleton)>& _GetSkeleton) { context::singleton().register_source(_handle, _GetSkeleton); }
void unregister_source(handle _handle) { context::singleton().unregister_source(_handle); }
bool get_info(handle _handle, bone_info* _pSkeleton) { return context::singleton().get_info(_handle, _pSkeleton); }

		} // namespace skeleton
	} // namespace windows
} // namespace ouro
