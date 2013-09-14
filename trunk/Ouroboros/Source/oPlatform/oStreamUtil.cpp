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
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/oStream.h>
#include <oBasis/oBuffer.h>

bool oStreamLoad(void** _ppOutBuffer, size_t* _pOutSize, const oFUNCTION<void*(size_t _NumBytes)>& _Allocate, const oFUNCTION<void(void* _Pointer)>& _Deallocate, const char* _URIReference, bool _AsString)
{
	oStd::intrusive_ptr<threadsafe oStreamReader> Reader;
	if (!oStreamReaderCreate(_URIReference, &Reader))
		return false; // pass through error

	oSTREAM_DESC sd;
	Reader->GetDesc(&sd);
	unsigned long long actualSize = sd.Size + (_AsString ? 4 : 0);
	oSTREAM_READ r;
	r.Range.Size = sd.Size; // only read what's available
	r.pData = _Allocate(oSizeT(actualSize)); // 4 for support of UTF16/UTF32's null

	if (!r.pData)
	{
		oStd::sstring fileSize;
		oStd::format_bytes(fileSize, actualSize, 2);
		return oErrorSetLast(std::errc::no_buffer_space, "Out of memory allocating %s", fileSize.c_str());
	}

	// Add nul terminator (readied for UTF supprot
	if (_AsString)
	{
		((char*)r.pData)[sd.Size] = 0;
		((char*)r.pData)[sd.Size+1] = 0;
		((char*)r.pData)[sd.Size+2] = 0;
		((char*)r.pData)[sd.Size+3] = 0;
	}

	if (!Reader->Read(r))
	{
		_Deallocate(r.pData);
		return false; // pass through error
	}

	// Some apps (FXC!!) get confused if you report a size that has extra 
	// characters beyond the nul terminator (WHY!?). So fudge the number here 
	// based on content and keep it all as internalized to file I/O as possible.
	*_pOutSize = oSizeT(sd.Size);
	if (_AsString)
	{
		oStd::utf_type::value type = oStd::utfcmp(r.pData, oSizeT(__min(sd.Size, 512ull)));
		switch (type)
		{
			case oStd::utf_type::utf32be:
			case oStd::utf_type::utf32le:
				*_pOutSize += 4;
				break;
			case oStd::utf_type::utf16be:
			case oStd::utf_type::utf16le:
				*_pOutSize += 2;
				break;
			case oStd::utf_type::ascii: 
				(*_pOutSize)++;
				break;
		}
	}

	*_ppOutBuffer = r.pData;
	return true;
}

bool oStreamLoadPartial(void* _pBuffer, size_t _SizeofBuffer, const char* _URIReference)
{
	oStd::intrusive_ptr<threadsafe oStreamReader> Reader;
	if (!oStreamReaderCreate(_URIReference, &Reader))
		return false;

	oSTREAM_DESC sd;
	Reader->GetDesc(&sd);

	if (sd.Size < _SizeofBuffer)
		return false;

	oSTREAM_READ r;
	r.pData = _pBuffer;
	r.Range.Size = _SizeofBuffer;

	if (!Reader->Read(r))
		return false;

	return true;
}

bool oBufferLoad(const char* _URIReference, oBuffer** _ppBuffer, bool _AsString)
{
	void* b = nullptr;
	size_t size = 0;
	bool success = oStreamLoad(&b, &size, malloc, free, _URIReference, _AsString);
	return success ? oBufferCreate(_URIReference, b, size, free, _ppBuffer) : success;
}

static void FreeString(const char* string) 
{ 
	free((void*)string);
}

bool oOBJLoad(const char* _URIReference, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ)
{
	void* pBuffer = nullptr;
	size_t Size = 0;

	if (!oStreamLoad(&pBuffer, &Size, malloc, free, _URIReference, true))
		return false; // pass through error

	oStd::finally FreeBuffer([&] { if (pBuffer) free(pBuffer); });

	if (!oOBJCreate(_URIReference, (const char*)pBuffer, _Init, _ppOBJ))
		return false; // pass through error

	return true;
}

bool oMTLLoad(const char* _URIReference, threadsafe oMTL** _ppMTL)
{
	void* pBuffer = nullptr;
	size_t Size = 0;

	if (!oStreamLoad(&pBuffer, &Size, malloc, free, _URIReference, true))
		return false; // pass through error

	oStd::finally FreeBuffer([&] { if (pBuffer) free(pBuffer); });

	if (!oMTLCreate(_URIReference, (const char*)pBuffer, _ppMTL))
		return false; // pass through error

	return true;
}

bool oOBJLoad(const char* _URIReference, const oOBJ_INIT& _Init, threadsafe oOBJ** _ppOBJ, threadsafe oMTL** _ppMTL)
{
	if (!oOBJLoad(_URIReference, _Init, _ppOBJ))
		return false; // pass through error

	oOBJ_DESC d;
	(*_ppOBJ)->GetDesc(&d);

	if (_ppMTL)
	{
		if (oSTRVALID(d.MTLPath))
		{
			oStd::path_string mtlPath;
			if (oIsFullPath(d.MTLPath))
				mtlPath = d.MTLPath;
			else
			{
				mtlPath = _URIReference;
				oReplaceFilename(mtlPath, d.MTLPath);
			}

			if (!oMTLLoad(mtlPath, _ppMTL))
				return false; // pass through error
		}

		else
			*_ppMTL = nullptr;
	}

	return true;
}

#define LOAD_BUFFER \
	void* pBuffer = nullptr; \
	size_t Size = 0; \
	if (!oStreamLoad(&pBuffer, &Size, malloc, free, _URIReference, true)) \
		return false; // pass through error

std::shared_ptr<oStd::csv> oCSVLoad(const char* _URIReference)
{
	LOAD_BUFFER
	try { return std::move(std::make_shared<oStd::csv>(_URIReference, (char*)pBuffer, FreeString)); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		return nullptr;
	}
}

std::shared_ptr<oStd::ini> oINILoad(const char* _URIReference)
{
	LOAD_BUFFER
	try { return std::move(std::make_shared<oStd::ini>(_URIReference, (char*)pBuffer, FreeString)); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		return nullptr;
	}
}

std::shared_ptr<oStd::xml> oXMLLoad(const char* _URIReference)
{
	LOAD_BUFFER
	try { return std::move(std::make_shared<oStd::xml>(_URIReference, (char*)pBuffer, FreeString)); }
	catch (std::exception& e)
	{
		oErrorSetLast(e);
		return nullptr;
	}
}