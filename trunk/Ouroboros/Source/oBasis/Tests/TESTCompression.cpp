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
#include <oBasis/tests/oBasisTests.h>
#include <oBasis/oGZip.h>
#include <oBasis/oLZMA.h>
#include <oBasis/oPath.h>
#include <oBasis/oSnappy.h>
#include <oBasis/oString.h>
#include <oBasis/oTimer.h>
#include <oStd/finally.h>
#include <oStd/timer.h>
#include "oBasisTestCommon.h"

//#include <oBasis/oError.h>
//#include <oStd/fixed_string.h>

static bool TestCompress(const void* _pSourceBuffer, size_t _SizeofSourceBuffer, oCompressFn _Compress, oDecompressFn _Decompress, size_t* _pCompressedSize)
{
	size_t maxCompressedSize = _Compress(nullptr, 0, _pSourceBuffer, _SizeofSourceBuffer);
	void* compressed = new char[maxCompressedSize];
	oStd::finally OSEFreeCompressed([&] { if (compressed) delete [] compressed; });

	*_pCompressedSize = _Compress(compressed, maxCompressedSize, _pSourceBuffer, _SizeofSourceBuffer);
	oTESTB0(*_pCompressedSize != 0);

	size_t uncompressedSize = _Decompress(nullptr, 0, compressed, *_pCompressedSize);
	if (_SizeofSourceBuffer != uncompressedSize)
		return oErrorSetLast(std::errc::protocol_error, "loaded and uncompressed sizes don't match");

	void* uncompressed = malloc(uncompressedSize);
	oStd::finally OSEFreeUncompressed([&] { if (uncompressed) free(uncompressed); });

	oTESTB0(0 != _Decompress(uncompressed, uncompressedSize, compressed, *_pCompressedSize));
	oTESTB(!memcmp(_pSourceBuffer, uncompressed, uncompressedSize), "memcmp failed between uncompressed and loaded buffers");
	return true;
}

bool oBasisTest_oCompression(const oBasisTestServices& _Services)
{
	static const char* BenchmarkFilename = "Test/Geometry/buddha.obj";

	oStd::path_string path;
	oTESTB(_Services.ResolvePath(path.c_str(), path.capacity(), BenchmarkFilename, true), "not found: %s", BenchmarkFilename);
	oTESTB(oCleanPath(path.c_str(), path.capacity(), path), "Failed to clean path on \"%s\"", path);

	char* pOBJBuffer = nullptr;
	size_t Size = 0;
	oTESTB(_Services.AllocateAndLoadBuffer((void**)&pOBJBuffer, &Size, path, true), "Failed to load file \"%s\"", path);
	oStd::finally FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pOBJBuffer); });

	double timeSnappy, timeLZMA, timeGZip;
	size_t CompressedSize0, CompressedSize1, CompressedSize2;

	oStd::timer t;
	oTESTB0(TestCompress(pOBJBuffer, Size, oSnappyCompress, oSnappyDecompress, &CompressedSize0));
	timeSnappy = t.seconds();
	t.reset();
	oTESTB0(TestCompress(pOBJBuffer, Size, oLZMACompress, oLZMADecompress, &CompressedSize1));
	timeLZMA = t.seconds();
	t.reset();
	oTESTB0(TestCompress(pOBJBuffer, Size, oGZipCompress, oGZipDecompress, &CompressedSize2));
	timeGZip = t.seconds();
	t.reset();

	oStd::sstring strUncompressed, strSnappy, strLZMA, strGZip, strSnappyTime, strLZMATime, strGZipTime;
	oStd::format_bytes(strUncompressed, Size, 2);
	oStd::format_bytes(strSnappyTime, CompressedSize0, 2);
	oStd::format_bytes(strLZMATime, CompressedSize1, 2);
	oStd::format_bytes(strGZip, CompressedSize2, 2);
	
	oStd::format_duration(strSnappyTime, timeSnappy, true);
	oStd::format_duration(strLZMATime, timeLZMA, true);
	oStd::format_duration(strGZipTime, timeGZip, true);

	oErrorSetLast(0, "Compressed %s from %s to Snappy: %s in %s, LZMA: %s in %s, GZip: %s in %s"
		, path.c_str()
		, strUncompressed.c_str()
		, strSnappy.c_str()
		, strSnappyTime.c_str()
		, strLZMA.c_str()
		, strLZMATime.c_str()
		, strGZip.c_str()
		, strGZipTime.c_str());
	return true;
}
