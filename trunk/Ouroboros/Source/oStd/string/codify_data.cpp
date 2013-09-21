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
#include <oStd/macros.h>
#include <oStd/string.h>

int snprintf(char* _StrDestination, size_t _SizeofStrDestination, const char* _Format, ...);

using namespace std;

namespace oStd {

errno_t replace(char* oRESTRICT _StrResult, size_t _SizeofStrResult, const char* oRESTRICT _StrSource, const char* _StrFind, const char* _StrReplace);

template<typename T> struct StaticArrayTraits {};
template<> struct StaticArrayTraits<unsigned char>
{
	static const size_t WORDS_PER_LINE = 20;
	static inline const char* GetFormat() { return "0x%02x,"; }
	static inline const char* GetType() { return "unsigned char"; }
};
template<> struct StaticArrayTraits<unsigned short>
{
	static const size_t WORDS_PER_LINE = 16;
	static inline const char* GetFormat() { return "0x%04x,"; }
	static inline const char* GetType() { return "unsigned short"; }
};
template<> struct StaticArrayTraits<unsigned int>
{
	static const size_t WORDS_PER_LINE = 10;
	static inline const char* GetFormat() { return "0x%08x,"; }
	static inline const char* GetType() { return "unsigned int"; }
};
template<> struct StaticArrayTraits<unsigned long long>
{
	static const size_t WORDS_PER_LINE = 10;
	static inline const char* GetFormat() { return "0x%016llx,"; }
	static inline const char* GetType() { return "unsigned long long"; }
};

static const char* file_base(const char* _Path)
{
	const char* p = _Path + strlen(_Path) - 1;
	while (p >= _Path)
	{
		if (*p == '/' || *p == '\\')
			return p + 1;
		p--;
	}
	return nullptr;
}

static char* codify_buffer_name(char* _StrDestination, size_t _SizeofStrDestination, const char* _Path)
{
	if (replace(_StrDestination, _SizeofStrDestination, file_base(_Path), ".", "_"))
		return nullptr;
	return _StrDestination;
}

template<size_t size> inline char* codify_buffer_name(char (&_StrDestination)[size], const char* _Path) { return codify_buffer_name(_StrDestination, size, _Path); }

template<typename T>
static size_t codify_data(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const T* _pBuffer, size_t _SizeofBuffer)
{
	const T* words = static_cast<const T*>(_pBuffer);
	char* str = _StrDestination;
	char* end = str + _SizeofStrDestination - 1; // -1 for terminator
	const size_t nWords = _SizeofBuffer / sizeof(T);

	str += snprintf(str, _SizeofStrDestination, "const %s sBuffer[] = \n{ // *** AUTO-GENERATED BUFFER, DO NOT EDIT ***", StaticArrayTraits<T>::GetType());
	for (size_t i = 0; i < nWords; i++)
	{
		size_t numberOfElementsLeft = std::distance(str, end);
		if ((i % StaticArrayTraits<T>::WORDS_PER_LINE) == 0 && numberOfElementsLeft > 2)
		{
			*str++ = '\n';
			*str++ = '\t';
			numberOfElementsLeft -= 2;
		}

		str += snprintf(str, numberOfElementsLeft, StaticArrayTraits<T>::GetFormat(), *words++);
	}

	// handle any remaining bytes
	const size_t nExtraBytes = _SizeofBuffer % sizeof(T);
	if (nExtraBytes)
	{
		unsigned long long tmp = 0;
		memcpy(&tmp, &reinterpret_cast<const unsigned char*>(_pBuffer)[sizeof(T) * nWords], nExtraBytes);
		str += snprintf(str, std::distance(str, end), StaticArrayTraits<T>::GetFormat(), static_cast<T>(tmp));
	}

	str += snprintf(str, std::distance(str, end), "\n};\n");

	// add accessor function

	char bufferId[_MAX_PATH];
	codify_buffer_name(bufferId, _BufferName);

	unsigned long long sz = _SizeofBuffer; // explicitly size this out so printf formatting below can remain the same between 32- and 64-bit
	str += snprintf(str, std::distance(str, end), "void get_%s(const char** ppBufferName, const void** ppBuffer, size_t* pSize) { *ppBufferName = \"%s\"; *ppBuffer = sBuffer; *pSize = %llu; }\n", bufferId, file_base(_BufferName), sz);

	if (str < end)
		*str++ = 0;

	return std::distance(_StrDestination, str);
}

size_t codify_data(char* _StrDestination, size_t _SizeofStrDestination, const char* _BufferName, const void* _pBuffer, size_t _SizeofBuffer, size_t _WordSize)
{
	switch (_WordSize)
	{
		case sizeof(unsigned char): return codify_data(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned char*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned short): return codify_data(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned short*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned int): return codify_data(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned int*>(_pBuffer), _SizeofBuffer);
		case sizeof(unsigned long long): return codify_data(_StrDestination, _SizeofStrDestination, _BufferName, static_cast<const unsigned long long*>(_pBuffer), _SizeofBuffer);
		default: break;
	}

	return 0;
}

} // namespace oStd
