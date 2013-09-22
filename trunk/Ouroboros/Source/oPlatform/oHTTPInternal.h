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
#pragma once
#ifndef oHTTPInternal_h
#define oHTTPInternal_h

#include <oPlatform/oHTTP.h>

static const unsigned int HTTPRequestTimeoutMS = 10000;
static const unsigned int HTTPRequestSize = 4096;

struct oHTTPHeaderField
{
	ouro::sstring Key;
	ouro::lstring Value;
};

struct oHTTPHeaderFields
{
	bool AddHeader(const char* _Header, const char* _Value);
	bool FindHeader(const char* _Header, const char** _Value);
	bool RemoveHeader(const char* _Header);
	void ClearHeaders();

	std::vector<oHTTPHeaderField> Vector;
};

struct oHTTPRequestInternal : public oHTTP_REQUEST
{
	void Reset();
	void Reset(oHTTP_METHOD _Method, const char* _RequestURI, oHTTP_VERSION _Version);
	oHTTPHeaderFields HeaderFieldsInternal;
};

struct oHTTPResponseInternal : public oHTTP_RESPONSE
{
	void Reset();
	oHTTPHeaderFields HeaderFieldsInternal;
};

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderField& _Fields);
bool from_string(oHTTPHeaderField* _pValue, const char* _StrSource);

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderFields& _Fields);
bool from_string(oHTTPHeaderFields* _pValue, const char* _StrSource);

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTP_REQUEST::oHTTP_REQUEST_LINE& _RequestLine);
bool from_string(oHTTP_REQUEST::oHTTP_REQUEST_LINE* _pValue, const char* _StrSource);

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPRequestInternal& _Request);
bool from_string(oHTTPRequestInternal* _pValue, const char* _StrSource);

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPResponseInternal& _Response);
bool from_string(oHTTPResponseInternal* _pValue, const char* _StrSource);

} // namespace ouro

bool oExtractHTTPHeader(const char* _pData, size_t _SizeofData, char* _pHeader, size_t* _pSizeofHeader, size_t _MaxHeaderSize, size_t* _pSizeofDataTaken);
bool oExtractContent(const void* _pData, size_t _SizeofData, void* _pContent, size_t* _pSizeofContent, size_t _TotalContentSize, size_t* _pSizeofDataTaken);
bool oInsertContent(const void* _pContent, size_t _SizeofContent, void* _pBuffer, size_t* _pSizeofBuffer, size_t _TotalBufferSize, size_t* _pSizeofContentTaken);

// @oooii-kevin FIXME: Prior to Hong Kong we started seeing infrequent heap 
// corruption. I suspect HTTP, follow this define to see where.
//#define SUSPECTED_BUGS_IN_HTTP

#ifdef SUSPECTED_BUGS_IN_HTTP
template<size_t size>
struct oGuardBand
{
public:
	oGuardBand()
	{
		ouro::memset4(guard, 0x0011f350, 4 * size);
	}

	void Check() threadsafe
	{
		oFORI(i, guard)
		{
			oASSERT(guard[i] == 0x0011f350, "Guardband corrupted!");
		}
	}
private:
	int guard[size];
};
#endif

#endif // oHTTPInternal_h