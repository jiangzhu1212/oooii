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
#include <oPlatform/oHTTP.h>
#include <set>
#include "oHTTPInternal.h"

const char* oAsString(oHTTP_VERSION _Code)
{
	switch (_Code)
	{
	case oHTTP_VERSION_UNKNOWN: return "HTTP/?.?";
	case oHTTP_0_9: return "HTTP/0.9";
	case oHTTP_1_0: return "HTTP/1.0";
	case oHTTP_1_1: return "HTTP/1.1";
	case oHTTP_1_2: return "HTTP/1.2";
		oNODEFAULT;
	}
}

bool oFromString(oHTTP_VERSION* _pRequest, const char* _StrSource)
{
	return oEnumFromString<oHTTP_NUM_VERSIONS>(_pRequest, _StrSource);
}

const char* oAsString(oHTTP_STATUS_CODE _Code)
{
	switch (_Code)
	{
		case oHTTP_CONTINUE: return "Continue";
		case oHTTP_SWITCHING_PROTOCOLS: return "Switching Protocols";
		case oHTTP_OK: return "OK";
		case oHTTP_CREATED: return "Created";
		case oHTTP_ACCEPTED: return "Accepted";
		case oHTTP_NON_AUTHORITATIVE_INFORMATION: return "Non-Authoritative Information";
		case oHTTP_NO_CONTENT: return "No Content";
		case oHTTP_RESET_CONTENT: return "Reset Content";
		case oHTTP_PARTIAL_CONTENT: return "Partial Content";
		case oHTTP_MULTIPLE_CHOICES: return "Multiple Choices";
		case oHTTP_MOVED_PERMANENTLY: return "Moved Permanently";
		case oHTTP_FOUND: return "Found";
		case oHTTP_SEE_OTHER: return "See Other";
		case oHTTP_NOT_MODIFIED: return "Not Modified";
		case oHTTP_USE_PROXY: return "Use Proxy";
		case oHTTP_TEMPORARY_REDIRECT: return "Temporary Redirect";
		case oHTTP_BAD_REQUEST: return "Bad Request";
		case oHTTP_UNAUTHORIZED: return "Unauthorized";
		case oHTTP_PAYMENT_REQUIRED: return "Payment Required";
		case oHTTP_FORBIDDEN: return "Forbidden";
		case oHTTP_NOT_FOUND: return "Not Found";
		case oHTTP_METHOD_NOT_ALLOWED: return "Method Not Allowed";
		case oHTTP_NOT_ACCEPTABLE: return "Not Acceptable";
		case oHTTP_PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
		case oHTTP_REQUEST_TIMEOUT: return "Request Timeout";
		case oHTTP_CONFLICT: return "Conflict";
		case oHTTP_GONE: return "Gone";
		case oHTTP_LENGTH_REQUIRED: return "Length Required";
		case oHTTP_PRECONDITION_FAILED: return "Precondition Failed";
		case oHTTP_REQUEST_ENTITY_TOO_LARGE: return "Request Entity Too Large";
		case oHTTP_REQUEST_URI_TOO_LONG: return "Request-URI Too Long";
		case oHTTP_UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
		case oHTTP_REQUESTED_RANGE_NOT_SATISFIABLE: return "Requested Range Not Satisfiable";
		case oHTTP_EXPECTATION_FAILED: return "Expectation Failed";
		case oHTTP_CLIENT_CLOSED_REQUEST: return "Client Closed Request";
		case oHTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
		case oHTTP_NOT_IMPLEMENTED: return "Not Implemented";
		case oHTTP_BAD_GATEWAY: return "Bad Gateway";
		case oHTTP_SERVICE_UNAVAILABLE: return "Service Unavailable";
		case oHTTP_GATEWAY_TIMEOUT: return "Gateway Timeout";
		case oHTTP_VERSION_NOT_SUPPORTED: return "Version Not Supported";
		default: return "Unrecognized http error";
	}
}

static const char* HttpMethodAsString[] = 
{
	"OPTIONS",
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"CONNECT",
	"PATCH"
};

const char* oAsString(oHTTP_METHOD _Method)
{
	int index = firstbithigh(_Method);
	oASSERT(1==countbits((unsigned long)_Method) && index < oHTTP_NUM_METHODS, "Illegal or unsupported Method");
	return HttpMethodAsString[index];
}

bool oFromString(oHTTP_METHOD* _pMethod, const char* _StrSource)
{
	int TmpMethod;
	if (!oEnumFromString<oHTTP_NUM_METHODS>(&TmpMethod, _StrSource, HttpMethodAsString))
		return false;
	*_pMethod = (oHTTP_METHOD)(1 << TmpMethod);
	return true;
}

const char* oAsString(oHTTP_HEADER_FIELD _Field)
{
	switch (_Field)
	{
	case oHTTP_HEADER_ACCEPT: return "Accept";
	case oHTTP_HEADER_ACCEPT_CHARSET: return "Accept-Charset";
	case oHTTP_HEADER_ACCEPT_ENCODING: return "Accept-Encoding";
	case oHTTP_HEADER_ACCEPT_LANGUAGE: return "Accept-Language";
	case oHTTP_HEADER_ACCEPT_RANGES: return "Accept-Ranges";
	case oHTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN: return "Access-Control-Allow-Origin";
	case oHTTP_HEADER_AGE: return "Age";
	case oHTTP_HEADER_ALLOW: return "Allow";
	case oHTTP_HEADER_AUTHORIZATION: return "Authorization";
	case oHTTP_HEADER_CACHE_CONTROL: return "Cache-Control";
	case oHTTP_HEADER_CONNECTION: return "Connection";
	case oHTTP_HEADER_CONTENT_ENCODING: return "Content-Encoding";
	case oHTTP_HEADER_CONTENT_LANGUAGE: return "Content-Language";
	case oHTTP_HEADER_CONTENT_LENGTH: return "Content-Length";
	case oHTTP_HEADER_CONTENT_LOCATION: return "Content-Location";
	case oHTTP_HEADER_CONTENT_MD5: return "Content-MD5";
	case oHTTP_HEADER_CONTENT_RANGE: return "Content-Range";
	case oHTTP_HEADER_CONTENT_TYPE: return "Content-Type";
	case oHTTP_HEADER_DATE: return "Date";
	case oHTTP_HEADER_ETAG: return "ETag";
	case oHTTP_HEADER_EXPECT: return "Expect";
	case oHTTP_HEADER_EXPIRES: return "Expires";
	case oHTTP_HEADER_FROM: return "From";
	case oHTTP_HEADER_HOST: return "Host";
	case oHTTP_HEADER_IF_MATCH: return "If-Match";
	case oHTTP_HEADER_IF_MODIFIED_SINCE: return "If-Modified-Since";
	case oHTTP_HEADER_IF_NONE_MATCH: return "If-None-Match";
	case oHTTP_HEADER_IF_RANGE: return "If-Range";
	case oHTTP_HEADER_IF_UNMODIFIED_SINCE: return "If-Unmodified-Since";
	case oHTTP_HEADER_LAST_MODIFIED: return "Last-Modified";
	case oHTTP_HEADER_LOCATION: return "Location";
	case oHTTP_HEADER_MAX_FORWARDS: return "Max-Forwards";
	case oHTTP_HEADER_PRAGMA: return "Pragma";
	case oHTTP_HEADER_PROXY_AUTHENTICATE: return "Proxy-Authenticate";
	case oHTTP_HEADER_PROXY_AUTHORIZATION: return "Proxy-Authorization";
	case oHTTP_HEADER_RANGE: return "Range";
	case oHTTP_HEADER_REFERER: return "Referer";
	case oHTTP_HEADER_RETRY_AFTER: return "Retry-After";
	case oHTTP_HEADER_SERVER: return "Server";
	case oHTTP_HEADER_TE: return "TE";
	case oHTTP_HEADER_TRAILER: return "Trailer";
	case oHTTP_HEADER_TRANSFER_ENCODING: return "Transfer-Encoding";
	case oHTTP_HEADER_UPGRADE: return "Upgrade";
	case oHTTP_HEADER_USER_AGENT: return "User-Agent";
	case oHTTP_HEADER_VARY: return "Vary";
	case oHTTP_HEADER_VIA: return "Via";
	case oHTTP_HEADER_WARNING: return "Warning";
	case oHTTP_HEADER_WWW_AUTHENTICATE: return "WWW-Authenticate";
	default: return "Unrecognized HTTP header field";
	}
}

bool oFromString(oHTTP_HEADER_FIELD* _pField, const char* _StrSource)
{
	return oEnumFromString<oHTTP_NUM_HEADER_FIELDS>(_pField, _StrSource);
}


char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderField& _Fields) 
{
	if (-1 == oPrintf(_StrDestination, _SizeofStrDestination, "%s:%s" oNEWLINE, _Fields.Key.c_str(), _Fields.Value.c_str()))
		return nullptr;

	return _StrDestination;
}
bool oFromString(oHTTPHeaderField* _pValue, const char* _StrSource)
{
	char* ctx;
	char* Key = oStrTok(_StrSource, ":", &ctx);
	if (!Key) return false;
	char* Value = oStrTok(nullptr, oNEWLINE, &ctx);
	if (!Value) return false;
	while(*Value == ' ')
		++Value;
	_pValue->Key = Key;
	_pValue->Value = Value;
// 	if (!oFromString(&_pValue->Field, Value))
// 		_pValue->Field = oHTTP_HEADER_NON_STANDARD;
	oStrTokClose(&ctx);
	return true;
}


bool oHTTPHeaderFields::AddHeader(const char* _Header, const char* _Value)
{
	oHTTPHeaderField NewHeader;
	NewHeader.Key = _Header;
	NewHeader.Value = _Value;
	Vector.push_back(NewHeader);
	return true;
}
bool oHTTPHeaderFields::FindHeader(const char* _Header, const char** _Value)
{
	for (unsigned int i=0; i<Vector.size(); ++i)
	{
		char* typeStr = strstr(Vector[i].Key, _Header);
		if (typeStr)
		{
			*_Value = Vector[i].Value;
			return true;
		}
	}
	return false;
}

bool oHTTPHeaderFields::RemoveHeader(const char* _Header)
{
	return false;
}

void oHTTPHeaderFields::ClearHeaders()
{
	Vector.clear();
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderFields& _Fields) 
{
	char* NextLine = _StrDestination;
	size_t SzRemaining = _SizeofStrDestination;
	for (unsigned int i=0; i<_Fields.Vector.size(); ++i)
	{
		if (!oToString(NextLine, SzRemaining, _Fields.Vector[i]))
			return nullptr;

		size_t LineLength = strlen(NextLine);
		NextLine += LineLength;
		SzRemaining -= LineLength;
	}
	return _StrDestination;
}
bool oFromString(oHTTPHeaderFields* _pValue, const char* _StrSource)
{
	// FIXME: oStrTok will probably not return the empty line and skip right over the two newlines in a row...
	char* ctx;
	char* Header = oStrTok(_StrSource, oNEWLINE, &ctx);
	oOnScopeExit free_ctx([&]() { oStrTokClose(&ctx); });
	while (Header)
	{
		oHTTPHeaderField Field;
		if (!oFromString(&Field, Header))
			return false;
		_pValue->Vector.push_back(Field);

		// Try to get next header line
		Header = oStrTok(nullptr, oNEWLINE, &ctx);
	}

	return true;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTP_REQUEST::oHTTP_REQUEST_LINE& _RequestLine) 
{ 
	// From RFC 2616 - HTTP/1.1: 
	// CR             = <US-ASCII CR, carriage return (13)>
	// LF             = <US-ASCII LF, linefeed (10)>
	// SP             = <US-ASCII SP, space (32)>
	// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
	return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%s %s %s" oNEWLINE, oAsString(_RequestLine.Method), _RequestLine.RequestURI.c_str(), oAsString(_RequestLine.Version)) ? _StrDestination : nullptr; 
}
bool oFromString(oHTTP_REQUEST::oHTTP_REQUEST_LINE* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* Value = oStrTok(_StrSource, " ", &ctx);
	oOnScopeExit free_ctx([&]() { oStrTokClose(&ctx); });
	if (!Value) return false;
	if (!oFromString(&_pValue->Method, Value))
		return false;

	Value = oStrTok(nullptr, " ", &ctx);
	if (!Value) return false;
	_pValue->RequestURI = Value;

	Value = oStrTok(nullptr, " " oNEWLINE, &ctx);
	if (!Value) return false;
	if (!oFromString(&_pValue->Version, Value))
		return false;

	return true;
}

void oHTTPRequestInternal::Reset()
{
	RequestLine.Method = (oHTTP_METHOD)0;
	RequestLine.RequestURI = "";
	RequestLine.Version = oHTTP_VERSION_UNKNOWN;
	HeaderFields = &HeaderFieldsInternal;
	HeaderFieldsInternal.ClearHeaders();
	Content.Type = oMIME_UNKNOWN;
	Content.pData = nullptr;
	Content.Length = 0;
}

void oHTTPRequestInternal::Reset(oHTTP_METHOD _Method, const char* _RequestURI, oHTTP_VERSION _Version)
{
	RequestLine.Method = _Method;
	RequestLine.RequestURI = _RequestURI;
	RequestLine.Version = _Version;
	HeaderFields = &HeaderFieldsInternal;
	HeaderFieldsInternal.ClearHeaders();
	Content.Type = oMIME_UNKNOWN;
	Content.pData = nullptr;
	Content.Length = 0;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPRequestInternal& _Request) 
{ 
	if (!oToString(_StrDestination, _SizeofStrDestination, _Request.RequestLine))
		return nullptr;

	char* StrDest = oByteAdd(_StrDestination, strlen(_StrDestination));
	size_t SzLeft = _SizeofStrDestination - (StrDest - _StrDestination);

	if (!oToString(StrDest, SzLeft, _Request.HeaderFieldsInternal))
		return nullptr;

	if (oStrAppendf(_StrDestination, _SizeofStrDestination, oNEWLINE))
		return nullptr;

	return _StrDestination;
}
bool oFromString(oHTTPRequestInternal* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* RequestLine = oStrTok(_StrSource, oNEWLINE, &ctx);
	if (!RequestLine) return false;
	oOnScopeExit free_ctx([&]() { oStrTokClose(&ctx); });
	if (!oFromString(&_pValue->RequestLine, RequestLine))
		return false;
	if (!oFromString(&_pValue->HeaderFieldsInternal, ctx))
		return false;
	return true;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTP_RESPONSE::oHTTP_STATUS_LINE& _StatusLine) 
{ 
	// From RFC 2616 - HTTP/1.1: 
	// CR             = <US-ASCII CR, carriage return (13)>
	// LF             = <US-ASCII LF, linefeed (10)>
	// SP             = <US-ASCII SP, space (32)>
	// Status-Line	  = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
	return -1 != oPrintf(_StrDestination, _SizeofStrDestination, "%s %u %s" oNEWLINE, oAsString(_StatusLine.Version), _StatusLine.StatusCode, _StatusLine.ReasonPhrase.c_str()) ? _StrDestination : nullptr; 
}
bool oFromString(oHTTP_RESPONSE::oHTTP_STATUS_LINE* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* Version = oStrTok(_StrSource, " ", &ctx);
	if (!Version) return false;
	oOnScopeExit free_ctx([&]() { oStrTokClose(&ctx); });
	if (!oFromString(&_pValue->Version, Version))
		return false;

	char* StatusCode = oStrTok(nullptr, " ", &ctx);
	if (!StatusCode) 
		return false;
	_pValue->StatusCode = (oHTTP_STATUS_CODE)atoi(StatusCode);

	char* ReasonPhrase = oStrTok(nullptr, oNEWLINE, &ctx);
	if (!ReasonPhrase) 
		return false;
	_pValue->ReasonPhrase = ReasonPhrase;
	return true;
}

void oHTTPResponseInternal::Reset()
{
	StatusLine.StatusCode = (oHTTP_STATUS_CODE)0;
	StatusLine.ReasonPhrase = "";
	StatusLine.Version = oHTTP_VERSION_UNKNOWN;
	HeaderFields = &HeaderFieldsInternal;
	HeaderFieldsInternal.ClearHeaders();
	Content.Type = oMIME_UNKNOWN;
	Content.pData = nullptr;
	Content.Length = 0;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPResponseInternal& _Response) 
{ 
	if (!oToString(_StrDestination, _SizeofStrDestination, _Response.StatusLine))
		return nullptr;

	char* StrDest = oByteAdd(_StrDestination, strlen(_StrDestination));
	size_t SzLeft = _SizeofStrDestination - (StrDest - _StrDestination);

	if (!oToString(StrDest, SzLeft, _Response.HeaderFieldsInternal))
		return nullptr;

	if (oStrAppendf(_StrDestination, _SizeofStrDestination, oNEWLINE))
		return nullptr;

	return _StrDestination;
}
bool oFromString(oHTTPResponseInternal* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* RequestLine = oStrTok(_StrSource, oNEWLINE, &ctx);
	if (!RequestLine) return false;
	oOnScopeExit free_ctx([&]() { oStrTokClose(&ctx); });
	if (!oFromString(&_pValue->StatusLine, RequestLine))
		return false;
	if (!oFromString(&_pValue->HeaderFieldsInternal, ctx))
		return false;
	return true;
}

oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char* _Value)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->AddHeader(oAsString(_Field), _Value);
}
oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int _Value)
{
	if(!_HeaderFields)
		return false;
	oStringS StrValue;
	oPrintf(StrValue, "%u", _Value);
	return _HeaderFields->AddHeader(oAsString(_Field), StrValue.c_str());
}
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char** _Value)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->FindHeader(oAsString(_Field), _Value);
}
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int* _Value)
{
	if(!_HeaderFields)
		return false;
	const char* Value;
	if (!_HeaderFields->FindHeader(oAsString(_Field), &Value))
		return false;
	return oFromString(_Value, Value);
}
oAPI bool oHTTPRemoveHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->RemoveHeader(oAsString(_Field));
}
oAPI void oHTTPClearHeaders(oHTTP_HEADER_FIELDS _HeaderFields)
{
	if(!_HeaderFields)
		return;
	_HeaderFields->ClearHeaders();
}

const char* GetEndHeaders(const char* _pBuffer)
{
	const char *pEnd = strstr(_pBuffer, "\n\r\n");
	if (pEnd)
		return pEnd+3;

	pEnd = strstr(_pBuffer, "\n\n");
	if (pEnd)
		return pEnd+2;
	else
		return 0;
}

// Take bytes from _pData until we have a full HTTP header and return true, return false when needing another callback with more data or when the _MaxHeaderSize has been reached
bool oExtractHTTPHeader(const char* _pData, size_t _SzData, char* _pHeader, size_t* _pSzHeader, size_t _MaxHeaderSize, size_t* _pSzDataTaken)
{
	// If the current header size is bigger than the maximum, we're done
	if (*_pSzHeader >= _MaxHeaderSize)
		return false;

	// First copy the new data to the header buffer, since the end-marker (two oNEWLINEs in a row) might be split into multiple data packets
	memcpy_s(oByteAdd(_pHeader, *_pSzHeader), (_MaxHeaderSize - *_pSzHeader), _pData, _SzData);

	// Put a zero terminator at the end of what we just copied (or at the end of the buffer in case of it filling up)
	_pHeader[__min(*_pSzHeader + _SzData, _MaxHeaderSize - 1)] = 0;

	char* HeaderEnd = (char*)GetEndHeaders(_pHeader);
	if (!HeaderEnd)
	{
		// The end marker hasn't been found yet, so if there's a zero terminator in the just copied data, we will never find an end marker thus error out
		if (memchr(_pData, 0, _SzData))
			return false;

		*_pSzDataTaken += _SzData;
		*_pSzHeader += _SzData; // This can make the current header size bigger than the maximum, but this should be handled properly
		return false;
	}

	// Zero terminate, oFromString() needs to know when to stop parsing header fields
	HeaderEnd[0] = 0;

	*_pSzDataTaken += ((HeaderEnd - _pHeader) - *_pSzHeader);
	*_pSzHeader = HeaderEnd - _pHeader;
	return true;
}

// Take bytes from _pData until we have the full content (_ContentSize) and return true, return false when needing another callback with more data
bool oExtractContent(const void* _pData, size_t _SzData, void* _pContent, size_t* _pSzContent, size_t _TotalContentSize, size_t* _pSzDataTaken)
{
	size_t NewContentSize = *_pSzContent + _SzData;
	if (NewContentSize > _TotalContentSize)
		NewContentSize = _TotalContentSize;

	size_t SizeToCopy = NewContentSize - *_pSzContent;
	if (SizeToCopy)
		memcpy_s(oByteAdd(_pContent, *_pSzContent), (_TotalContentSize - *_pSzContent), _pData, SizeToCopy);

	*_pSzDataTaken += NewContentSize - *_pSzContent;
	*_pSzContent = NewContentSize;
	return (NewContentSize == _TotalContentSize);
}

// Insert content into a buffer until it's full (_TotalBufferSize) and return true, return false when needing another callback with more content
// If _pSzContentTaken is not set, we'll fail on buffer overflow
bool oInsertContent(const void* _pContent, size_t _SzContent, void* _pBuffer, size_t* _pSzBuffer, size_t _TotalBufferSize, size_t* _pSzContentTaken)
{
	size_t NewBufferSize = _SzContent + *_pSzBuffer;
	if (NewBufferSize > _TotalBufferSize)
		NewBufferSize = _TotalBufferSize;

	size_t SizeToCopy = NewBufferSize - *_pSzBuffer;
	if (SizeToCopy)
		memcpy_s(oByteAdd(_pBuffer, *_pSzBuffer), _TotalBufferSize - *_pSzBuffer, _pContent, SizeToCopy);

	if (_pSzContentTaken)
	{
		*_pSzContentTaken = NewBufferSize - *_pSzBuffer;
	}
	else
	{
		*_pSzBuffer += _SzContent;
		return _TotalBufferSize == *_pSzBuffer;
	}

	*_pSzBuffer = NewBufferSize;
	return _TotalBufferSize == NewBufferSize;
}

