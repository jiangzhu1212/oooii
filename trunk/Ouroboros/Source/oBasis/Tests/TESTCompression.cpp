/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasisTests/oBasisTests.h>
#include <oBasis/oCompression.h>
#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oOnScopeExit.h>
#include <oBasis/oPath.h>
#include <oBasis/oString.h>
#include <oBasis/oTimer.h>
#include "oBasisTestCommon.h"

static bool TestCompress(const void* _pSourceBuffer, size_t _SizeofSourceBuffer, oCOMPRESSION_TRADEOFF _Tradeoff, size_t* _pCompressedSize)
{
	size_t maxCompressedSize = oCalcMaxCompressedSize(_SizeofSourceBuffer, _Tradeoff);
	void* compressed = new char[maxCompressedSize];
	oOnScopeExit OSEFreeCompressed([&] { if (compressed) delete [] compressed; });

	oTESTB0(oCompress(compressed, maxCompressedSize, _pSourceBuffer, _SizeofSourceBuffer, _Tradeoff, _pCompressedSize));

	size_t uncompressedSize = oGetUncompressedSize(compressed);
	if (_SizeofSourceBuffer != uncompressedSize)
		return oErrorSetLast(oERROR_GENERIC, "loaded and uncompressed sizes don't match");

	void* uncompressed = malloc(uncompressedSize);
	oOnScopeExit OSEFreeUncompressed([&] { if (uncompressed) free(uncompressed); });

	oTESTB0(oUncompress(uncompressed, uncompressedSize, compressed, *_pCompressedSize));
	oTESTB(!memcmp(_pSourceBuffer, uncompressed, uncompressedSize), "memcmp failed between uncompressed and loaded buffers");
	return true;
}

bool oBasisTest_oCompression(const oBasisTestServices& _Services)
{
	static const char* BenchmarkFilename = "Test/Geometry/buddha.obj";

	oStringPath path;
	oTESTB(_Services.ResolvePath(path.c_str(), path.capacity(), BenchmarkFilename, true), "not found: %s", BenchmarkFilename);
	oTESTB(oCleanPath(path.c_str(), path.capacity(), path), "Failed to clean path on \"%s\"", path);

	char* pOBJBuffer = nullptr;
	size_t Size = 0;
	oTESTB(_Services.AllocateAndLoadBuffer((void**)&pOBJBuffer, &Size, path, true), "Failed to load file \"%s\"", path);
	oOnScopeExit FreeBuffer([&] { _Services.DeallocateLoadedBuffer(pOBJBuffer); });

	double start, timeSpeedy, timeMinimal;
	size_t CompressedSize0, CompressedSize1;

	start = oTimer();
	oTESTB0(TestCompress(pOBJBuffer, Size, oFAVOR_DECOMPRESSION_SPEED, &CompressedSize0));
	timeSpeedy = oTimer() - start;
	start = oTimer();
	oTESTB0(TestCompress(pOBJBuffer, Size, oFAVOR_MINIMAL_SIZE, &CompressedSize1));
	timeMinimal = oTimer() - start;

	oStringS strUncompressed, strSpeedy, strMinimal, strSpeedyTime, strMinimalTime;
	oErrorSetLast(oERROR_NONE, "Compressed %s from %s to Speedy: %s in %s, Minimally: %s in %s"
		, path.c_str()
		, oFormatMemorySize(strUncompressed, Size, 2)
		, oFormatMemorySize(strSpeedy, CompressedSize0, 2)
		, oFormatTimeSize(strSpeedyTime, timeSpeedy, true)
		, oFormatMemorySize(strMinimal, CompressedSize1, 2)
		, oFormatTimeSize(strMinimalTime, timeMinimal, true));
	return true;
}
