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
#include <oBasis/oCompression.h>
#include <oBasis/oByte.h>
#include <oBasis/oError.h>
#include <oBasis/oMacros.h>
#include <snappy/snappy.h>
#include <Lzma/C/LzmaLib.h>

// Note: oFAVOR_DECOMPRESSION_SPEED implies snappy 'snap'
//       oFAVOR_MINIMAL_SIZE implies lzma 'lzma'

enum oALGORITHM_VERSION
{
	oALGO_SNAPPY_1_0,
	oALGO_LZMA,
};

// From LzmaLib.h documentation:
static const int LZMADEFAULT_level = 5;
static const unsigned int LZMADEFAULT_dictSize = 1 << 24;
static const int LZMADEFAULT_lc = 3;
static const int LZMADEFAULT_lp = 0;
static const int LZMADEFAULT_pb = 2;
static const int LZMADEFAULT_fb = 32;
static const int LZMADEFAULT_numThreads = 2;

static const unsigned char LZMADEFAULT_Props[] = { 93, 0, 0, 0, 1 };

const char* oAsStringLZMA_ERROR(int _Error)
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
		default: return "Unrecognized LZMA error code";
	}
}

#pragma pack(1)
struct oCOMPRESS_HEADER
{
	unsigned int UncompressedSize;
	unsigned char AlgorithmVersion;
};

static_assert(sizeof(oCOMPRESS_HEADER) == 5, "padding is bloating a compressed buffer header");

inline oCOMPRESS_HEADER& hdr(void* _pCompressedBuffer) { return *(oCOMPRESS_HEADER*)_pCompressedBuffer; }
inline const oCOMPRESS_HEADER& hdr(const void* _pCompressedBuffer) { return *(oCOMPRESS_HEADER*)_pCompressedBuffer; }

bool oCompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource, oCOMPRESSION_TRADEOFF _Tradeoff, size_t* _pActualCompressedSize)
{
	oCOMPRESS_HEADER& header = hdr(_pDestination);
	header.UncompressedSize = oUInt(_SizeofSource);
	
	switch (_Tradeoff)
	{
		case oFAVOR_DECOMPRESSION_SPEED:
			if (_SizeofDestination < oCalcMaxCompressedSize(_SizeofSource, _Tradeoff))
				return oErrorSetLast(oERROR_AT_CAPACITY, "destination buffer is too small to receive the compressed result");
			header.AlgorithmVersion = oALGO_SNAPPY_1_0;
			snappy::RawCompress(static_cast<const char*>(_pSource), _SizeofSource, static_cast<char*>(oByteAdd(_pDestination, sizeof(oCOMPRESS_HEADER))), _pActualCompressedSize);
			return true;
		case oFAVOR_MINIMAL_SIZE:
		{
			header.AlgorithmVersion = oALGO_LZMA;
			*_pActualCompressedSize = _SizeofDestination;
			size_t outPropsSize = LZMA_PROPS_SIZE;
			unsigned char outProps[LZMA_PROPS_SIZE];
			int LZMAError = LzmaCompress(
				static_cast<unsigned char*>(oByteAdd(_pDestination, sizeof(oCOMPRESS_HEADER)))
				, _pActualCompressedSize
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
				return oErrorSetLast(oERROR_GENERIC, "%s", oAsStringLZMA_ERROR(LZMAError));
			return true;
		}
		oNODEFAULT;
	}
}

bool oUncompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	const oCOMPRESS_HEADER& header = hdr(_pSource);
	if (_SizeofDestination < header.UncompressedSize)
		return oErrorSetLast(oERROR_AT_CAPACITY, "destination buffer is too small to receive the uncompressed result");
	
	switch (header.AlgorithmVersion)
	{
		case oALGO_SNAPPY_1_0:
			if (!snappy::RawUncompress(static_cast<const char*>(oByteAdd(_pSource, sizeof(oCOMPRESS_HEADER))), _SizeofSource, static_cast<char*>(_pDestination)))
				return oErrorSetLast(oERROR_CORRUPT, "decompression failed");
			break;

		case oALGO_LZMA:
		{
			size_t destLen = _SizeofDestination;
			size_t srcLen = _SizeofSource;
			int LZMAError = LzmaUncompress(static_cast<unsigned char*>(_pDestination), &destLen, static_cast<const unsigned char*>(oByteAdd(_pSource, sizeof(oCOMPRESS_HEADER))), &srcLen, LZMADEFAULT_Props, LZMA_PROPS_SIZE);
			if (LZMAError)
				return oErrorSetLast(oERROR_CORRUPT, "decompression failed: %s", oAsStringLZMA_ERROR(LZMAError));
			break;
		}
		default:
			return oErrorSetLast(oERROR_CORRUPT, "Unrecognized codec");
	}

	return true;
}

static size_t LZMAEstimateCompressedSize(size_t _SizeofSource)
{
	// http://sourceforge.net/projects/sevenzip/forums/forum/45797/topic/3420786
	// a post by ipavlov...
	return static_cast<size_t>(1.1f * _SizeofSource + 0.5f) + oKB(16);
}

size_t oCalcMaxCompressedSize(size_t _SizeofSource, oCOMPRESSION_TRADEOFF _Tradeoff)
{
	switch (_Tradeoff)
	{
		case oFAVOR_DECOMPRESSION_SPEED: return sizeof(oCOMPRESS_HEADER) + snappy::MaxCompressedLength(_SizeofSource);
		case oFAVOR_MINIMAL_SIZE: return sizeof(oCOMPRESS_HEADER) + LZMAEstimateCompressedSize(_SizeofSource);
		oNODEFAULT;
	}
}

size_t oGetUncompressedSize(const void* _pCompressedBuffer)
{
	return hdr(_pCompressedBuffer).UncompressedSize;
}
