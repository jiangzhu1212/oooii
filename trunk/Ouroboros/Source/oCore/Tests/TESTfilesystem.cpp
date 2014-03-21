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
#include <oCore/filesystem.h>
#include <oCore/windows/win_iocp.h>
#include <oBase/assert.h>
#include <oBase/event.h>
#include <oBase/finally.h>
#include <oBase/timer.h>

#include "../../test_services.h"

using namespace ouro::filesystem;

namespace ouro {
	namespace tests {

static void TESTfilesystem_paths()
{
	path path = current_path();
	oTRACE("CWD: %s", path.c_str());
	path = app_path();
	oTRACE("APP: %s", path.c_str());
	path = temp_path();
	oTRACE("SYSTMP: %s", path.c_str());
	path = system_path();
	oTRACE("SYS: %s", path.c_str());
	path = os_path();
	oTRACE("OS: %s", path.c_str());
	path = dev_path();
	oTRACE("DEV: %s", path.c_str());
	path = desktop_path();
	oTRACE("DESKTOP: %s", path.c_str());
	path = data_path();
	oTRACE("DATA: %s", path.c_str());
}

static void TESTfilesystem_map(test_services& _Services)
{
	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "Test/Textures/lena_1.png";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	unsigned long long size = file_size(TestPath);
	
	void* mapped = map(TestPath, map_option::binary_read, 0, size);
	if (!mapped)
		oTHROW(protocol_error, "map failed");
	finally Unmap([&] { if (mapped) unmap(mapped); }); // safety unmap if we fail for some non-mapping reason

	size_t Size = 0;
	std::shared_ptr<char> loaded = load(TestPath, &Size);

	if (Size != size)
		oTHROW(io_error, "mismatch: mapped and loaded file sizes");

	if (memcmp(loaded.get(), mapped, Size))
		oTHROW(protocol_error, "memcmp failed between mapped and loaded files");
		
	unmap(mapped);
	mapped = nullptr; // signal Unmap to noop
}

void TESTfilesystem_async(test_services& _Services)
{
	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "Test/Geometry/buddha.obj";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	event e;
	timer t;
	for(int i = 0; i < 32; i++)
	{
		async_load(TestPath, [&,i](const path& _Path, void* _pBuffer, size_t _Size)->async_finally::value
		{
			size_t s = _Size;
			char* p = (char*)_pBuffer;
			e.set(1<<i);
			return async_finally::free_buffer;
		});
	}

	double after = t.milliseconds();
	oCHECK(e.wait_for(std::chrono::seconds(20), ~0u), "timed out");
	double done = t.milliseconds();

	_Services.report("all dispatches: %.02fms | all completed: %.02fms", after, done);
}

void TESTfilesystem(test_services& _Services)
{
	TESTfilesystem_paths();
	TESTfilesystem_map(_Services);
	TESTfilesystem_async(_Services);
}

	} // namespace tests
} // namespace ouro
