// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/lzma.h>
#include <oMemory/byte.h>
#include <oBase/macros.h>
#include <oBase/throw.h>
#include <Lzma/C/LzmaLib.h>

namespace ouro {

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

static size_t lzma_estimate_compressed_size(size_t src_size)
{
	// http://sourceforge.net/projects/sevenzip/forums/forum/45797/topic/3420786
	// a post by ipavlov...
	return static_cast<size_t>(1.1f * src_size + 0.5f) + oKB(16);
}

static const char* as_string_lzma_error(int _Error)
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

size_t lzma_compress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT src, size_t src_size)
{
	size_t CompressedSize = 0;
	if (dst)
	{
		const size_t EstSize = lzma_compress(nullptr, 0, nullptr, src_size);
		if (dst && dst_size < EstSize)
			oTHROW0(no_buffer_space);

		((HDR*)dst)->UncompressedSize = src_size;
		CompressedSize = dst_size;
		size_t outPropsSize = LZMA_PROPS_SIZE;
		unsigned char outProps[LZMA_PROPS_SIZE];
		int LZMAError = LzmaCompress(
			static_cast<unsigned char*>(byte_add(dst, sizeof(HDR)))
			, &CompressedSize
			, static_cast<const unsigned char*>(src)
			, src_size
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
			oTHROW(protocol_error, "compression failed: %s", as_string_lzma_error(LZMAError));
	}

	else
		CompressedSize = sizeof(HDR) + lzma_estimate_compressed_size(src_size);

	return CompressedSize;
}

size_t lzma_decompress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT src, size_t src_size)
{
	size_t UncompressedSize = ((const HDR*)src)->UncompressedSize;
		if (dst && dst_size < UncompressedSize)
			oTHROW0(no_buffer_space);

	size_t destLen = dst_size;
	size_t srcLen = src_size;
	int LZMAError = LzmaUncompress(
		static_cast<unsigned char*>(dst)
		, &destLen
		, static_cast<const unsigned char*>(byte_add(src, sizeof(HDR)))
		, &srcLen
		, LZMADEFAULT_Props
		, LZMA_PROPS_SIZE);

	if (LZMAError)
		oTHROW(protocol_error, "decompression failed: %s", as_string_lzma_error(LZMAError));

	return UncompressedSize;
}

}
