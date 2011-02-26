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
#include "pch.h"
#include <oooii/oAssert.h>
#include <oooii/oNonCopyable.h>
#include <oooii/oStddef.h>
#include <oooii/oString.h>
#include <oooii/oTempString.h>
#include <oooii/oSTL.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <list>

// #define REUSE_TEMP_STRINGS

struct oStringBuffer : oNoncopyable
{
	oStringBuffer(size_t _size)
		: size(_size)
		, refCount(0)
	{
		str = new char[size];
		*str = 0;
	}

	~oStringBuffer()
	{
		delete[] str;
	}

	char*		str;
	size_t		size;
	oRefCount	refCount;
};

#ifdef REUSE_TEMP_STRINGS
struct oStringBufferMgr : public oProcessThreadlocalSingleton<oStringBufferMgr>
{
	~oStringBufferMgr()
	{
		for(tStrBufList::iterator it = strBufList.begin(); it != strBufList.end(); ++it)
			delete *it;

		strBufList.clear();
	}

	virtual void Reference() threadsafe
	{
		oProcessThreadlocalSingleton<oStringBufferMgr>::Reference();
	}

	virtual void Release() threadsafe
	{
		oProcessThreadlocalSingleton<oStringBufferMgr>::Release();
	}

	oStringBuffer* GetStringBuffer(size_t _size)
	{
		oStringBuffer* pStrBuf = NULL;

		// Find the smallest available buffer
		for(tStrBufList::iterator it = strBufList.begin(); it != strBufList.end(); ++it)
		{
			oStringBuffer* strBuf = *it;
			if(!strBuf->refCount.Valid() && strBuf->size >= _size)
			{
				pStrBuf = strBuf;
				if(strBuf->size == _size)
					break;
			}
		}

		// Allocate a new oStringBuffer
		if(!pStrBuf)
		{
			pStrBuf = new oStringBuffer(_size);
			strBufList.push_back(pStrBuf);
		}

		return pStrBuf;
	}

private:
	typedef std::list<oStringBuffer*> tStrBufList;
	tStrBufList strBufList;
};
#endif // REUSE_TEMP_STRINGS

oTempString::oTempString(size_t _size)
	: pStrBuffer(NULL)
	, offset(0)
{
	oASSERT(_size, "oTempString can not have zero size.");

#ifdef REUSE_TEMP_STRINGS
	pStrBuffer = oStringBufferMgr::Singleton()->GetStringBuffer(_size);
#else
	pStrBuffer = new oStringBuffer(_size);
#endif // REUSE_TEMP_STRINGS
	pStrBuffer->refCount.Set(1);
	*pStrBuffer->str = 0;
}

oTempString::oTempString(oStringBuffer* _pStrBuffer, size_t _offset)
	: pStrBuffer(_pStrBuffer)
	, offset(_offset)
{
	oASSERT(_pStrBuffer, "NULL pStrBuffer in oTempString");
	oASSERT(offset <= _pStrBuffer->size, "Offset exceeds size of oTempString"); // Not valid if offset == size, but useful for end pointers.

	pStrBuffer->refCount.Reference();
}

oTempString::~oTempString()
{
	if(pStrBuffer->refCount.Release())
	{
#ifndef REUSE_TEMP_STRINGS
		delete pStrBuffer;
#endif // REUSE_TEMP_STRINGS
	}
	pStrBuffer = NULL;
}

oTempString& oTempString::operator=(const oTempString& rhs)
{
	pStrBuffer = rhs.pStrBuffer;
	offset = rhs.offset;

	pStrBuffer->refCount.Reference();

	return *this;
}

oTempString::operator char*()
{
	return pStrBuffer->str + offset;
}

char* oTempString::c_str()
{
	return pStrBuffer->str + offset;
}

size_t oTempString::size() const
{
	return pStrBuffer->size - offset;
}
