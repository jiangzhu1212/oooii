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
#include <oBasis/oGZip.h>
#include <oBasis/oError.h>
#include <oBasis/oInt.h>
#include <zlib/zlib.h>

//#include <oStaging/oByteStreamHelpers.h>

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
static const unsigned char GZipID1 = 0x1f;
static const unsigned char GZipID2 = 0x8b;
static const unsigned char GZipCM = 0x08;

static const unsigned char GZipFlgFExtra = 0x04;
static const unsigned char GZipFlgFName = 0x08;
static const unsigned char GZipFlgFComment = 0x10;
static const unsigned char GZipFlgFHCrc = 0x02;

#pragma pack(1)
struct GZIP_HDR
{
	unsigned char ID1;
	unsigned char ID2;
	unsigned char CM;
	unsigned char FLG;
	unsigned int MTIME;
	unsigned char XFL;
	unsigned char OS;
};

size_t oGZipCompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	size_t CompressedSize = 0;

	if (_pDestination)
	{
		const size_t EstSize = oGZipCompress(nullptr, 0, nullptr, _SizeofSource);
		oCOMPRESSION_CHECK_DEST(EstSize);

		GZIP_HDR h;
		h.ID1 = GZipID1;
		h.ID2 = GZipID2;
		h.CM = GZipCM;
		h.FLG = 0x00;
		h.MTIME = 0;
		h.XFL = 0x02;
		h.OS = 0xff;
		unsigned int ISIZE = oUInt(_SizeofSource);

		memcpy(_pDestination, &h, sizeof(h));
		_SizeofDestination -= sizeof(h);
		_pDestination = oStd::byte_add(_pDestination, sizeof(h));
	
		uLongf bytesWritten = oUInt(_SizeofDestination);
		int result = compress2(static_cast<Bytef*>(_pDestination), &bytesWritten, static_cast<const Bytef*>(_pSource), oUInt(_SizeofSource), 9);
		if (result != Z_OK)
		{
			oErrorSetLast(std::errc::protocol_error, "compression failed");
			return 0;
		}

		_SizeofDestination -= bytesWritten;
		_pDestination = oStd::byte_add(_pDestination, bytesWritten);

		if (_SizeofDestination < GZipFooterSize)
		{
			oErrorSetLast(std::errc::no_buffer_space);
			return 0;
		}

		unsigned int& CRC32 = *(unsigned int*)_pDestination;
		CRC32 = crc32(0, Z_NULL, 0);
		CRC32 = crc32(CRC32, static_cast<const Bytef*>(_pSource), oUInt(_SizeofSource));

		_SizeofDestination -= sizeof(CRC32);
		_pDestination = oStd::byte_add(_pDestination, sizeof(CRC32));
		*(unsigned int*)_pDestination = oUInt(_SizeofSource);

		CompressedSize = sizeof(h) + bytesWritten + GZipFooterSize;
	}

	else
		CompressedSize = compressBound(oUInt(_SizeofSource)) + sizeof(GZIP_HDR) + GZipFooterSize;
	
	return CompressedSize;
}

size_t oGZipDecompress(void* oRESTRICT _pDestination, size_t _SizeofDestination, const void* oRESTRICT _pSource, size_t _SizeofSource)
{
	const void* src = _pSource;
	const GZIP_HDR& h = *(const GZIP_HDR*)src;

	if (h.ID1 != GZipID1 || h.ID2 != GZipID2 || h.CM != GZipCM)
	{
		oErrorSetLast(std::errc::protocol_error, "Not a valid GZip stream");
		return 0;
	}

	src = oStd::byte_add(src, sizeof(GZIP_HDR));

	// we don't generate optional fields but we need to skip over them if someone 
	// else did
	if (h.FLG && GZipFlgFExtra)
	{
		unsigned short extraSize = *(unsigned short*)src;
		src = oStd::byte_add(src, sizeof(unsigned short) + extraSize);
	}

	if (h.FLG && GZipFlgFName)
	{
		while (*static_cast<const char*>(src))
			src = oStd::byte_add(src, 1);
	}

	if (h.FLG && GZipFlgFComment)
	{
		while (*static_cast<const char*>(src))
			src = oStd::byte_add(src, 1);
	}

	if (h.FLG && GZipFlgFHCrc)
		src = oStd::byte_add(src, 2);

	unsigned int compressedSize = oUInt(_SizeofSource - oStd::byte_diff(src, _pSource) - GZipFooterSize);
	unsigned int UncompressedSize = *(unsigned int*)oStd::byte_add(_pSource, sizeof(h) + compressedSize + sizeof(unsigned int));

	if (_pDestination)
	{
		oCOMPRESSION_CHECK_DEST(UncompressedSize);

		uLongf bytesWritten = oUInt(_SizeofDestination);
		if (Z_OK != uncompress(static_cast<Bytef*>(_pDestination), &bytesWritten, static_cast<const Bytef*>(src), compressedSize))
		{
			oErrorSetLast(std::errc::protocol_error, "decompression error");
			return 0;
		}

		unsigned int expectedCRC32 = crc32(0, Z_NULL, 0);
		expectedCRC32 = crc32(expectedCRC32, static_cast<const Bytef*>(_pDestination), UncompressedSize);

		unsigned int CRC32 = *(unsigned int*)oStd::byte_add(_pSource, sizeof(h) + compressedSize);
		if (expectedCRC32 != CRC32)
		{
			oErrorSetLast(std::errc::protocol_error, "CRC mismatch in GZip stream");
			UncompressedSize = 0;
		}
	}

	return UncompressedSize;
}
