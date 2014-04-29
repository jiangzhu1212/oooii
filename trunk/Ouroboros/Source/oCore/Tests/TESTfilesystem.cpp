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
#include <oBase/byte.h>
#include <oBase/event.h>
#include <oBase/finally.h>
#include <oBase/guid.h>
#include <oBase/murmur3.h>
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

static void TESTfilesystem_read(test_services& _Services)
{
	static const unsigned int kNumReads = 5;

	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "oooii.ico";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	scoped_file ReadFile(TestPath, open_option::binary_read);
	const auto FileSize = file_size(ReadFile);

	size_t BytesPerRead = static_cast<size_t>(FileSize) / kNumReads;
	int ActualReadCount = kNumReads + (((FileSize % kNumReads) > 0) ? 1 : 0);

	char TempFileBlob[1024 * 32];
	void* pHead = TempFileBlob;

	size_t r = 0;
	long long Offset = 0;
	for (int i = 0; i < kNumReads; ++i, r += BytesPerRead)
	{
		seek(ReadFile, Offset, seek_origin::set);
		auto BytesRead = read(ReadFile, pHead, BytesPerRead, BytesPerRead);
		oCHECK(BytesRead == BytesPerRead, "failed to read correctly");
		pHead = ouro::byte_add(pHead, BytesPerRead);
		Offset += BytesPerRead;
	}

	auto RemainingBytes = FileSize - r;
	if (RemainingBytes > 0)
		read(ReadFile, pHead, byte_diff(pHead, TempFileBlob), RemainingBytes);

	static const uint128 ExpectedFileHash(13254728276562583748ull, 8059648572410507760ull);
	oCHECK(murmur3(TempFileBlob, static_cast<unsigned int>(FileSize)) == ExpectedFileHash, "Test failed to compute correct hash");
}

static void TESTfilesystem_write()
{
	path TempFilePath = temp_path() / "TESTfilesystem_write.bin";
	remove(TempFilePath);
	oCHECK(!exists(TempFilePath), "remove failed: %s", TempFilePath.c_str());

	static const guid TestGUID = { 0x9aab7fc7, 0x6ad8, 0x4260, { 0x98, 0xef, 0xfd, 0x93, 0xda, 0x8e, 0xdc, 0x3c } };

	{
		scoped_file WriteFile(TempFilePath, open_option::binary_write);

		auto BytesWritten = write(WriteFile, &TestGUID, sizeof(TestGUID));
		oCHECK(BytesWritten == sizeof(TestGUID), "write failed");
	}

	scoped_allocation LoadedGUID = load(TempFilePath);
	oCHECK(*(guid*)LoadedGUID == TestGUID, "Write failed to write correct guid");
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

	scoped_allocation loaded = load(TestPath);

	if (loaded.size() != size)
		oTHROW(io_error, "mismatch: mapped and loaded file sizes");

	if (memcmp(loaded, mapped, loaded.size()))
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
	for (int i = 0; i < 32; i++)
	{
		load_async(TestPath, [&,i](const path& _Path, scoped_allocation& _Buffer, size_t _Size, const std::system_error* _pError)
		{
			if (_pError)
				return;
			size_t s = _Size;
			char* p = (char*)_Buffer;
			e.set(1<<i);
		});
	}

	double after = t.milliseconds();
	oCHECK(e.wait_for (std::chrono::seconds(20), ~0u), "timed out");
	double done = t.milliseconds();

	_Services.report("all dispatches: %.02fms | all completed: %.02fms", after, done);
}

void TESTfilesystem(test_services& _Services)
{
	TESTfilesystem_paths();
	TESTfilesystem_read(_Services);
	TESTfilesystem_write();
	TESTfilesystem_map(_Services);
	TESTfilesystem_async(_Services);
}

	} // namespace tests
} // namespace ouro
