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
// Retains the string form of a hashed string in Debug
#pragma once
#ifndef oHashString_h
#define oHashString_h

#include <oooii/oAssert.h>
#include <oooii/oHash.h>

#ifdef _DEBUG
#define DEBUG_HASH_STRING
#endif

struct oHashString
{
	typedef unsigned int tHash;

	static tHash Hash(const char* _str);

	oHashString();
	oHashString(const oHashString& _hashStr);
	oHashString(const char* _str);
	oHashString(tHash _hash);

	void Set(const char* _str);
	void Set(tHash _hash);

	bool operator==(const oHashString& _other) const;
	bool operator!=(const oHashString& _other) const;
	bool operator<(const oHashString& _other) const;
	bool operator>(const oHashString& _other) const;

	tHash GetHash() const;
	const char* GetStr(); // Not threadsafe

private:
#ifdef DEBUG_HASH_STRING
	static const int MaxStringLength = 64 - sizeof(unsigned int);
	char str[MaxStringLength];
#endif // DEBUG_HASH_STRING
	tHash hash;
};



inline oHashString::tHash oHashString::Hash(const char* _str)
{
	return oHash_djb2i(_str);
}

inline oHashString::oHashString()
	: hash(0)
{
#ifdef DEBUG_HASH_STRING
	strcpy_s(str, MaxStringLength, "NULL");
#endif // DEBUG_HASH_STRING
}

inline oHashString::oHashString(const oHashString& _hashStr)
{
	*this = _hashStr;
}

inline oHashString::oHashString(const char* _str)
{
	Set(_str);
}

inline oHashString::oHashString(oHashString::tHash _hash)
{
	Set(_hash);
}

inline void oHashString::Set(const char* _str)
{
	oASSERT(_str, "NULL oHashString");
#ifdef DEBUG_HASH_STRING
	strncpy_s(str, MaxStringLength, _str, MaxStringLength-1);
#endif // DEBUG_HASH_STRING
	hash = Hash(_str);
}

inline void oHashString::Set(oHashString::tHash _hash)
{
	hash = _hash;
#ifdef DEBUG_HASH_STRING
	str[0] = 0;
#endif // DEBUG_HASH_STRING
}

inline bool oHashString::operator==(const oHashString& _other) const
{
	return(hash == _other.hash);
}

inline bool oHashString::operator!=(const oHashString& _other) const
{
	return(hash != _other.hash);
}

inline bool oHashString::operator<(const oHashString& _other) const
{
	return(hash < _other.hash);
}

inline bool oHashString::operator>(const oHashString& _other) const
{
	return(hash > _other.hash);
}

inline oHashString::tHash oHashString::GetHash() const
{
	return hash;
}

inline const char* oHashString::GetStr() // Not threadsafe
{
#ifdef DEBUG_HASH_STRING
	return str;
#else
	static char buf[32];
	sprintf_s(buf, 32, "%u", hash);
	return buf;
#endif // DEBUG_HASH_STRING
}

#endif // oHashString_h
