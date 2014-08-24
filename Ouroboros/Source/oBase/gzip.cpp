/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oMemory/byte.h>
#include <oBase/throw.h>
#include <oBase/types.h>
#include <zlib/zlib.h>

namespace ouro {

// GZip format
// 1 byte ID1 - 0x1f
// 1 byte ID2 - 0x8b
// 1 byte CM - 0x08  // means use deflate for compression which is the only allowed "option"
// 1 byte FLG - 0x00 // series of 8 flags, all should be 0 for our case. most are outdated, or not useful, or reserved
// 4 bytes MTIME - 0x00 // Modification time. last time file was modified, use 0 for http since its not really relevant in that case. even in regular file case 0 is fine
// 1 byte XFL - 0x02 // means use max compression. 0x04 is the only other option which means use fast compression. no direct translation to deflate values, just informational, not used for anything
// 1 byte OS - 0xff // there is a list of os's that you can specify. list created at time GZip was created. so not really relevant today. could use 11 for NTFS I suppose. 0xff means unknown
// many bytes - then place the deflate data
// 4 bytes CRC32 generate crc32 and place here
// 4 bytes ISIZE size of the data uncompressed

//Note that GZip is little endian

static const int GZipFooterSize = 8;
static const uint8_t GZipID1 = 0x1f;
static const uint8_t GZipID2 = 0x8b;
static const uint8_t GZipCM = 0x08;

static const uint8_t GZipFlgFExtra = 0x04;
static const uint8_t GZipFlgFName = 0x08;
static const uint8_t GZipFlgFComment = 0x10;
static const uint8_t GZipFlgFHCrc = 0x02;

#pragma pack(1)
struct GZIP_HDR
{
	uint8_t ID1;
	uint8_t ID2;
	uint8_t CM;
	uint8_t FLG;
	uint32_t MTIME;
	uint8_t XFL;
	uint8_t OS;
};

size_t gzip_compress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT src, size_t src_size)
{
	size_t CompressedSize = 0;

	if (dst)
	{
		const size_t EstSize = gzip_compress(nullptr, 0, nullptr, src_size);
		if (dst && dst_size < EstSize)
			oTHROW0(no_buffer_space);

		GZIP_HDR h;
		h.ID1 = GZipID1;
		h.ID2 = GZipID2;
		h.CM = GZipCM;
		h.FLG = 0x00;
		h.MTIME = 0;
		h.XFL = 0x02;
		h.OS = 0xff;
		uint32_t ISIZE = as_uint(src_size);

		memcpy(dst, &h, sizeof(h));
		dst_size -= sizeof(h);
		dst = byte_add(dst, sizeof(h));
	
		uLongf bytesWritten = as_ulong(dst_size);
		int result = compress2(static_cast<Bytef*>(dst), &bytesWritten, static_cast<const Bytef*>(src), static_cast<uint32_t>(src_size), 9);
		if (result != Z_OK)
			oTHROW(protocol_error, "compression failed");

		dst_size -= bytesWritten;
		dst = byte_add(dst, bytesWritten);

		if (dst_size < GZipFooterSize)
			oTHROW0(no_buffer_space);

		uint32_t& CRC32 = *(uint32_t*)dst;
		CRC32 = crc32(0, Z_NULL, 0);
		CRC32 = crc32(CRC32, static_cast<const Bytef*>(src), static_cast<uint32_t>(src_size));

		dst_size -= sizeof(CRC32);
		dst = byte_add(dst, sizeof(CRC32));
		*(uint32_t*)dst = static_cast<uint32_t>(src_size);

		CompressedSize = sizeof(h) + bytesWritten + GZipFooterSize;
	}

	else
		CompressedSize = compressBound(static_cast<uint32_t>(src_size)) + sizeof(GZIP_HDR) + GZipFooterSize;
	
	return CompressedSize;
}

size_t gzip_decompress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT in_src, size_t src_size)
{
	const void* src = in_src;
	const GZIP_HDR& h = *(const GZIP_HDR*)src;

	if (h.ID1 != GZipID1 || h.ID2 != GZipID2 || h.CM != GZipCM)
		oTHROW(protocol_error, "Not a valid GZip stream");

	src = byte_add(src, sizeof(GZIP_HDR));

	// we don't generate optional fields but we need to skip over them if someone 
	// else did
	if (h.FLG && GZipFlgFExtra)
	{
		uint16_t extraSize = *(uint16_t*)src;
		src = byte_add(src, sizeof(uint16_t) + extraSize);
	}

	if (h.FLG && GZipFlgFName)
	{
		while (*static_cast<const char*>(src))
			src = byte_add(src, 1);
	}

	if (h.FLG && GZipFlgFComment)
	{
		while (*static_cast<const char*>(src))
			src = byte_add(src, 1);
	}

	if (h.FLG && GZipFlgFHCrc)
		src = byte_add(src, 2);

	size_t sz = src_size - byte_diff(src, in_src) - GZipFooterSize;
	uint32_t compressedSize = as_uint(sz);
	uint32_t UncompressedSize = *(uint32_t*)byte_add(in_src, sizeof(h) + compressedSize + sizeof(uint32_t));

	if (dst)
	{
		if (dst && dst_size < UncompressedSize)
			oTHROW0(no_buffer_space);

		uLongf bytesWritten = as_ulong(dst_size);
		if (Z_OK != uncompress(static_cast<Bytef*>(dst), &bytesWritten, static_cast<const Bytef*>(src), compressedSize))
			oTHROW(protocol_error, "decompression error");

		uint32_t expectedCRC32 = crc32(0, Z_NULL, 0);
		expectedCRC32 = crc32(expectedCRC32, static_cast<const Bytef*>(dst), UncompressedSize);

		uint32_t CRC32 = *(uint32_t*)byte_add(in_src, sizeof(h) + compressedSize);
		if (expectedCRC32 != CRC32)
			oTHROW(protocol_error, "CRC mismatch in GZip stream");
	}

	return UncompressedSize;
}

} // namespace ouro
