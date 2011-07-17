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

// Basic bit stream helper for network serialization.
//
// Note: To push a single byte, don't use char or it will serialize as a string. Use BYTE.

#pragma once
#ifndef oBitStream_h
#define oBitStream_h

#include <oooii/oAssert.h>

typedef unsigned char BYTE;

struct oBitStream
{
	oBitStream();
	oBitStream(BYTE* _buffer, size_t _size);

	void PushRaw(void* _pSrc, size_t _size);
	void PopRaw(void* _pDest, size_t _size);

	template<typename T> void Push(T* data);
	template<typename T> void Push(T data);
	template<typename T> void Pop(T* data);

	BYTE* Buffer();
	size_t MaxSize();

	size_t CurPos();
	void CurPos(size_t _newPos);

private:
	BYTE* buffer;
	size_t maxSize;
	size_t curPos;
};



inline oBitStream::oBitStream()
	: buffer(NULL)
	, maxSize(0)
	, curPos(0)
{ }

inline oBitStream::oBitStream(BYTE* _buffer, size_t _size)
	: buffer(_buffer)
	, maxSize(_size)
	, curPos(0)
{ }

inline void oBitStream::PushRaw(void* _pSrc, size_t _size)
{
	oASSERT(curPos + _size <= maxSize, "Writing past end of buffer.");

	memcpy((void*)(buffer + curPos), _pSrc, _size);
	curPos += _size;
}

inline void oBitStream::PopRaw(void* _pDest, size_t _size)
{
	oASSERT(curPos + _size <= maxSize, "Reading past end of buffer.");

	memcpy(_pDest, (void*)(buffer + curPos), _size);
	curPos += _size;
}

template<typename T>
inline void oBitStream::Push(T* data)
{
	oASSERT(false, "Pushing unknown data type to oBitStream.");
}

template<typename T>
inline void oBitStream::Push(T data)
{
	oASSERT(false, "Pushing unknown data type to oBitStream.");
}

template<typename T>
inline void oBitStream::Pop(T* data)
{
	oASSERT(false, "Popping unknown data type to oBitStream.");
}

#define SPECIALIZE_BASIC_TYPE(type) \
	template<> \
	inline void oBitStream::Push<type>(type data) \
	{ \
		oASSERT(curPos + sizeof(type) <= maxSize, "oBitStream::Push exceeds capacity."); \
		*((type*)(buffer + curPos)) = data; \
		curPos += sizeof(type); \
	} \
	template<> \
	inline void oBitStream::Pop<type>(type* data) \
	{ \
		oASSERT(curPos + sizeof(type) <= maxSize, "oBitStream::Pop attempt to pop past end of buffer."); \
		*data = *reinterpret_cast<type*>(buffer + curPos); \
		curPos += sizeof(type); \
	}

SPECIALIZE_BASIC_TYPE(bool)
SPECIALIZE_BASIC_TYPE(BYTE)
SPECIALIZE_BASIC_TYPE(int)
SPECIALIZE_BASIC_TYPE(unsigned int)
SPECIALIZE_BASIC_TYPE(short)
SPECIALIZE_BASIC_TYPE(unsigned short)
SPECIALIZE_BASIC_TYPE(long)
SPECIALIZE_BASIC_TYPE(unsigned long)
SPECIALIZE_BASIC_TYPE(float)
SPECIALIZE_BASIC_TYPE(double)

template<>
inline void oBitStream::Push<const char>(const char* data)
{
	short len = (short)strlen(data);
	Push(len);
	PushRaw((void*)data, len);
}

template<>
inline void oBitStream::Pop<char>(char* data)
{
	short len;
	Pop(&len);
	PopRaw(data, len);
	data[len] = 0; // @oooii-mike: terminating zero is not part of the packed data.
}

inline BYTE* oBitStream::Buffer()
{
	return buffer;
}

inline size_t oBitStream::MaxSize()
{
	return maxSize;
}

inline size_t oBitStream::CurPos()
{
	return curPos;
}

inline void oBitStream::CurPos(size_t _newPos)
{
	curPos = _newPos;
}

#endif // oBitStream_h
