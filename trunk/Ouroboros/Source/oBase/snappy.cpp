// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/lzma.h>
#include <oMemory/byte.h>
#include <oBase/throw.h>
#include <snappy/snappy.h>

namespace ouro {

size_t snappy_compress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT src, size_t _SizeofSource)
{
	size_t CompressedSize = snappy::MaxCompressedLength(_SizeofSource);
	if (dst)
	{
		if (dst && dst_size < CompressedSize)
			oTHROW0(no_buffer_space);
		snappy::RawCompress(static_cast<const char*>(src), _SizeofSource, static_cast<char*>(dst), &CompressedSize);
	}
	return CompressedSize;
}

size_t snappy_decompress(void* oRESTRICT dst, size_t dst_size, const void* oRESTRICT src, size_t _SizeofSource)
{
	size_t UncompressedSize = 0;
	snappy::GetUncompressedLength(static_cast<const char*>(src), _SizeofSource, &UncompressedSize);
		if (dst && dst_size < UncompressedSize)
			oTHROW0(no_buffer_space);
	if (dst && !snappy::RawUncompress(static_cast<const char*>(src), _SizeofSource, static_cast<char*>(dst)))
		oTHROW0(protocol_error);
	return UncompressedSize;
}

}
