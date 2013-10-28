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
#include <oBase/assert.h>
#include <oBase/finally.h>

using namespace oStd;
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

static void TESTfilesystem_map()
{
	path TestPath = data_path() / "Test/Textures/lena_1.png";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	unsigned long long size = file_size(TestPath);
	
	void* mapped = map(TestPath, true, 0, size);
	if (!mapped)
		oTHROW(protocol_error, "map failed");
	finally Unmap([&] { if (mapped) unmap(mapped); }); // safety unmap if we fail for some non-mapping reason

	size_t Size = 0;
	std::shared_ptr<char> loaded = load(TestPath, load_option::binary_read, &Size);

	if (Size != size)
		oTHROW(io_error, "mismatch: mapped and loaded file sizes");

	if (memcmp(loaded.get(), mapped, Size))
		oTHROW(protocol_error, "memcmp failed between mapped and loaded files");
		
	unmap(mapped);
	mapped = nullptr; // signal Unmap to noop
}

void TESTfilesystem()
{
	TESTfilesystem_paths();
	TESTfilesystem_map();
}

	} // namespace tests
} // namespace ouro
