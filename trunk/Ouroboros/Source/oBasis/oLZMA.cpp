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
#include <oBasis/oLZMA.h>
#include <oBasis/oError.h>
//#include "oWinHeaders.h"
#include <Lzma/C/LzmaLib.h>

// From LzmaLib.h documentation:
static const int LZMADEFAULT_level = 5;
static const unsigned int LZMADEFAULT_dictSize = 1 << 24;
static const int LZMADEFAULT_lc = 3;
static const int LZMADEFAULT_lp = 0;
static const int LZMADEFAULT_pb = 2;
static const int LZMADEFAULT_fb = 32;
static const int LZMADEFAULT_numThreads = 2;

static const unsigned char LZMADEFAULT_Props[] = { 93, 0, 0, 0, 1 };

#pragma pack(1)
struct HDR
{
	size_t UncompressedSize;
};

static size_t LZMAEstimateCompressedSize(size_t _SizeofSource)
{
	// http://sourceforge.net/projects/sevenzip/forums/forum/45797/topic/3420786
	// a post by ipavlov...
	return static_cast<size_t>(1.1f * _SizeofSource + 0.5f) + oKB(16);
}

static const char* oAsStringLZMA_ERROR(int _Error)
{
	switch (_Error)
	{
		case SZ_OK: return "SZ_OK";
		case SZ_ERROR_DATA: return "SZ_ERROR_DATA";
		case SZ_ERROR_MEM: return "SZ_ERROR_MEM";
		case SZ_ERROR_CRC: return "SZ_ERROR_CRC";
		case SZ_ERROR_UNSUPPORTED: return "SZ_ERROR_UNSUPPORTED";
		case SZ_ERROR_PARAM: return "SZ_ERROR_PARAM";
		case SZ_ERROR_INPUT_EOF: return "SZ_ERROR_INPUT_EOF";
		case SZ_ERROR_OUTPUT_EOF: return "SZ_ERROR_OUTPUT_EOF";
		case SZ_ERROR_READ: return "SZ_ERROR_READ";
		case SZ_ERROR_WRITE: return "SZ_ERROR_WRITE";
		case SZ_ERROR_PROGRESS: return "SZ_ERROR_PROGRESS";
		case SZ_ERROR_FAIL: return "SZ_ERROR_FAIL";
		case SZ_ERROR_THREAD: return "SZ_ERROR_THREAD";
		case SZ_ERROR_ARCHIVE: return "SZ_ERROR_ARCHIVE";
		case SZ_ERROR_NO_ARCHIVE: return "SZ_ERROR_NO_ARCHIVE";
		default: break;
	}
	return "Unrecognized LZMA error code";
}

size_t oLZMACompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	size_t CompressedSize = 0;
	if (_pDestination)
	{
		const size_t EstSize = oLZMACompress(nullptr, 0, nullptr, _SizeofSource);
		oCOMPRESSION_CHECK_DEST(EstSize);

		((HDR*)_pDestination)->UncompressedSize = _SizeofSource;
		CompressedSize = _SizeofDestination;
		size_t outPropsSize = LZMA_PROPS_SIZE;
		unsigned char outProps[LZMA_PROPS_SIZE];
		int LZMAError = LzmaCompress(
			static_cast<unsigned char*>(oStd::byte_add(_pDestination, sizeof(HDR)))
			, &CompressedSize
			, static_cast<const unsigned char*>(_pSource)
			, _SizeofSource
			, outProps
			, &outPropsSize
			, LZMADEFAULT_level
			, LZMADEFAULT_dictSize
			, LZMADEFAULT_lc
			, LZMADEFAULT_lp
			, LZMADEFAULT_pb
			, LZMADEFAULT_fb
			, LZMADEFAULT_numThreads);

		if (LZMAError)
		{
			oErrorSetLast(std::errc::protocol_error, "compression failed: %s", oAsStringLZMA_ERROR(LZMAError));
			CompressedSize = 0;
		}
	}

	else
		CompressedSize = sizeof(HDR) + LZMAEstimateCompressedSize(_SizeofSource);

	return CompressedSize;
}

size_t oLZMADecompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	size_t UncompressedSize = ((const HDR*)_pSource)->UncompressedSize;
	oCOMPRESSION_CHECK_DEST(UncompressedSize);

	size_t destLen = _SizeofDestination;
	size_t srcLen = _SizeofSource;
	int LZMAError = LzmaUncompress(
		static_cast<unsigned char*>(_pDestination)
		, &destLen
		, static_cast<const unsigned char*>(oStd::byte_add(_pSource, sizeof(HDR)))
		, &srcLen
		, LZMADEFAULT_Props
		, LZMA_PROPS_SIZE);

	if (LZMAError)
	{
		oErrorSetLast(std::errc::protocol_error, "decompression failed: %s", oAsStringLZMA_ERROR(LZMAError));
		UncompressedSize = 0;
	}

	return UncompressedSize;
}
