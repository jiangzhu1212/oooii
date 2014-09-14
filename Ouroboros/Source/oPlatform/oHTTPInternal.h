// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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

}

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
		for (auto i = 0; i < oCOUNTOF(guard); i++)
		{
			oASSERT(guard[i] == 0x0011f350, "Guardband corrupted!");
		}
	}
private:
	int guard[size];
};
#endif

#endif // oHTTPInternal_h