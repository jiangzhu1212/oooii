/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
// Sometimes it's useful to create temporary strings without having to declare
// stack buffers. For example, if there's a multi-threaded message pump where
// a network thread receives a string and it wants to be passed onto another
// thread through a queue, this can be used if the buffer size is large enough
// that strings won't be overwritten by the time the consumer thread has processed
// the string.

#pragma once
#ifndef oStringRingBuffer_h
#define oStringRingBuffer_h

#include <stdarg.h>

template<typename charT> class oStringRingBuffer
{
	charT* pWrite;
	charT* pEnd;
	charT* pRingBuffer;

	size_t templated_strlen(const charT* _String);
	int templated_strcpy_s(charT* _StrDestination, size_t _SizeofStrDestination, const charT* _StrSource);
	int templated_vsprintf_s(charT* _StrDestination, size_t _SizeofStrDestination, const charT* _Format, va_list _Args);

public:
	oStringRingBuffer(charT* _pBuffer, size_t _SizeofBuffer)
		: pRingBuffer(_pBuffer)
		, pWrite(_pBuffer)
		, pEnd(_pBuffer + _SizeofBuffer)
	{}

	template<size_t size> oStringRingBuffer(charT (&_pBuffer)[size])
		: pRingBuffer(_pBuffer)
		, pWrite(_pBuffer)
		, pEnd(_pBuffer + size)
	{}

	// All copy/print API return the resulting string in the ring buffer
	const charT* vprintf(const charT* _Format, va_list _Args)
	{
		int n = templated_vsprintf_s(pWrite, std::distance(pWrite, pEnd), _Format, _Args);
		if (n < 0)
		{
			// must be at end of buffer, so start over
			pWrite = pRingBuffer;
			n = templated_vsprintf_s(pWrite, std::distance(pWrite, pEnd), _Format, _Args);
			if (n < 0)
				return 0; // too big for the entire buffer
		}

		const charT* read = pWrite;
		pWrite += n + 1; // +1 for nul terminator
		return read;
	}

	const charT* sprintf(const charT* _Format, ...)
	{
		va_list args;
		va_start(args, _Format);
		const charT* read = this->vprintf(_Format, args);
		va_end(args);
		return read;
	}

	const charT* strcpy(const charT* _Source)
	{
		size_t len = templated_strlen(_Source);
		if ((size_t)std::distance(pRingBuffer, pEnd) < (len - 1))
			return 0;

		if ((pWrite + len + 1) > pEnd)
			pWrite = pRingBuffer;

		templated_strcpy_s(pWrite, std::distance(pWrite, pEnd), _Source);
		const charT* read = pWrite;
		pWrite += len + 1;
		return read;
	}
};

#endif
