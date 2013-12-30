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
#include <oBase/gzip.h>
#include <oBase/lzma.h>
#include <oBase/snappy.h>
#include <oBase/finally.h>
#include <oBase/path.h>
#include <oBase/throw.h>
#include <oBase/timer.h>
#include <oCore/filesystem.h>

#include "../../test_services.h"

namespace ouro {
	namespace tests {

static void TestCompress(const void* _pSourceBuffer, size_t _SizeofSourceBuffer, compress_fn _Compress, decompress_fn _Decompress, size_t* _pCompressedSize)
{
	size_t maxCompressedSize = _Compress(nullptr, 0, _pSourceBuffer, _SizeofSourceBuffer);
	void* compressed = new char[maxCompressedSize];
	finally OSEFreeCompressed([&] { if (compressed) delete [] compressed; });

	*_pCompressedSize = _Compress(compressed, maxCompressedSize, _pSourceBuffer, _SizeofSourceBuffer);
	oCHECK0(*_pCompressedSize != 0);

	size_t uncompressedSize = _Decompress(nullptr, 0, compressed, *_pCompressedSize);
	if (_SizeofSourceBuffer != uncompressedSize)
		oTHROW(protocol_error, "loaded and uncompressed sizes don't match");

	void* uncompressed = malloc(uncompressedSize);
	finally OSEFreeUncompressed([&] { if (uncompressed) free(uncompressed); });

	oCHECK0(0 != _Decompress(uncompressed, uncompressedSize, compressed, *_pCompressedSize));
	oCHECK(!memcmp(_pSourceBuffer, uncompressed, uncompressedSize), "memcmp failed between uncompressed and loaded buffers");
}

void TESTcompression(test_services& _Services)
{
	static const char* TestPath = "Test/Geometry/buddha.obj";

	size_t Size = 0;
	std::shared_ptr<char> pOBJBuffer = _Services.load_buffer(TestPath, &Size);

	double timeSnappy, timeLZMA, timeGZip;
	size_t CompressedSize0, CompressedSize1, CompressedSize2;

	timer t;
	TestCompress(pOBJBuffer.get(), Size, snappy_compress, snappy_decompress, &CompressedSize0);
	timeSnappy = t.seconds();
	t.reset();
	TestCompress(pOBJBuffer.get(), Size, lzma_compress, lzma_decompress, &CompressedSize1);
	timeLZMA = t.seconds();
	t.reset();
	TestCompress(pOBJBuffer.get(), Size, gzip_compress, gzip_decompress, &CompressedSize2);
	timeGZip = t.seconds();
	t.reset();

	sstring strUncompressed, strSnappy, strLZMA, strGZip, strSnappyTime, strLZMATime, strGZipTime;
	format_bytes(strUncompressed, Size, 2);
	format_bytes(strSnappy, CompressedSize0, 2);
	format_bytes(strLZMA, CompressedSize1, 2);
	format_bytes(strGZip, CompressedSize2, 2);
	
	format_duration(strSnappyTime, timeSnappy, true);
	format_duration(strLZMATime, timeLZMA, true);
	format_duration(strGZipTime, timeGZip, true);

	_Services.report("Compressed %s from %s to Snappy: %s in %s, LZMA: %s in %s, GZip: %s in %s"
		, TestPath
		, strUncompressed.c_str()
		, strSnappy.c_str()
		, strSnappyTime.c_str()
		, strLZMA.c_str()
		, strLZMATime.c_str()
		, strGZip.c_str()
		, strGZipTime.c_str());
}

	} // namespace tests
} // namespace ouro
