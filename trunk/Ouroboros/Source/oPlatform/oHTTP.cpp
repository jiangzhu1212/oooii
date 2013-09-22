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
#include "oHTTPInternal.h"
#include <oHLSL/oHLSLBit.h>
#include <oPlatform/oHTTP.h>
#include <set>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oHTTP_VERSION)
	oRTTI_ENUM_BEGIN_VALUES(oHTTP_VERSION)
		oRTTI_VALUE_CUSTOM(oHTTP_VERSION_UNKNOWN, "HTTP/?.?")
		oRTTI_VALUE_CUSTOM(oHTTP_0_9, "HTTP/0.9")
		oRTTI_VALUE_CUSTOM(oHTTP_1_0, "HTTP/1.0")
		oRTTI_VALUE_CUSTOM(oHTTP_1_1, "HTTP/1.1")
		oRTTI_VALUE_CUSTOM(oHTTP_1_2, "HTTP/1.2")
	oRTTI_ENUM_END_VALUES(oHTTP_VERSION)
oRTTI_ENUM_END_DESCRIPTION(oHTTP_VERSION)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oHTTP_STATUS_CODE)
	oRTTI_ENUM_BEGIN_VALUES(oHTTP_STATUS_CODE)
		oRTTI_VALUE_CUSTOM(oHTTP_CONTINUE, "Continue")
		oRTTI_VALUE_CUSTOM(oHTTP_SWITCHING_PROTOCOLS, "Switching Protocols")
		oRTTI_VALUE_CUSTOM(oHTTP_OK, "OK")
		oRTTI_VALUE_CUSTOM(oHTTP_CREATED, "Created")
		oRTTI_VALUE_CUSTOM(oHTTP_ACCEPTED, "Accepted")
		oRTTI_VALUE_CUSTOM(oHTTP_NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information")
		oRTTI_VALUE_CUSTOM(oHTTP_NO_CONTENT, "No Content")
		oRTTI_VALUE_CUSTOM(oHTTP_RESET_CONTENT, "Reset Content")
		oRTTI_VALUE_CUSTOM(oHTTP_PARTIAL_CONTENT, "Partial Content")
		oRTTI_VALUE_CUSTOM(oHTTP_MULTIPLE_CHOICES, "Multiple Choices")
		oRTTI_VALUE_CUSTOM(oHTTP_MOVED_PERMANENTLY, "Moved Permanently")
		oRTTI_VALUE_CUSTOM(oHTTP_FOUND, "Found")
		oRTTI_VALUE_CUSTOM(oHTTP_SEE_OTHER, "See Other")
		oRTTI_VALUE_CUSTOM(oHTTP_NOT_MODIFIED, "Not Modified")
		oRTTI_VALUE_CUSTOM(oHTTP_USE_PROXY, "Use Proxy")
		oRTTI_VALUE_CUSTOM(oHTTP_TEMPORARY_REDIRECT, "Temporary Redirect")
		oRTTI_VALUE_CUSTOM(oHTTP_BAD_REQUEST, "Bad Request")
		oRTTI_VALUE_CUSTOM(oHTTP_UNAUTHORIZED, "Unauthorized")
		oRTTI_VALUE_CUSTOM(oHTTP_PAYMENT_REQUIRED, "Payment Required")
		oRTTI_VALUE_CUSTOM(oHTTP_FORBIDDEN, "Forbidden")
		oRTTI_VALUE_CUSTOM(oHTTP_NOT_FOUND, "Not Found")
		oRTTI_VALUE_CUSTOM(oHTTP_METHOD_NOT_ALLOWED, "Method Not Allowed")
		oRTTI_VALUE_CUSTOM(oHTTP_NOT_ACCEPTABLE, "Not Acceptable")
		oRTTI_VALUE_CUSTOM(oHTTP_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required")
		oRTTI_VALUE_CUSTOM(oHTTP_REQUEST_TIMEOUT, "Request Timeout")
		oRTTI_VALUE_CUSTOM(oHTTP_CONFLICT, "Conflict")
		oRTTI_VALUE_CUSTOM(oHTTP_GONE, "Gone")
		oRTTI_VALUE_CUSTOM(oHTTP_LENGTH_REQUIRED, "Length Required")
		oRTTI_VALUE_CUSTOM(oHTTP_PRECONDITION_FAILED, "Precondition Failed")
		oRTTI_VALUE_CUSTOM(oHTTP_REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large")
		oRTTI_VALUE_CUSTOM(oHTTP_REQUEST_URI_TOO_LONG, "Request-URI Too Long")
		oRTTI_VALUE_CUSTOM(oHTTP_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type")
		oRTTI_VALUE_CUSTOM(oHTTP_REQUESTED_RANGE_NOT_SATISFIABLE, "Requested Range Not Satisfiable")
		oRTTI_VALUE_CUSTOM(oHTTP_EXPECTATION_FAILED, "Expectation Failed")
		oRTTI_VALUE_CUSTOM(oHTTP_CLIENT_CLOSED_REQUEST, "Client Closed Request")
		oRTTI_VALUE_CUSTOM(oHTTP_INTERNAL_SERVER_ERROR, "Internal Server Error")
		oRTTI_VALUE_CUSTOM(oHTTP_NOT_IMPLEMENTED, "Not Implemented")
		oRTTI_VALUE_CUSTOM(oHTTP_BAD_GATEWAY, "Bad Gateway")
		oRTTI_VALUE_CUSTOM(oHTTP_SERVICE_UNAVAILABLE, "Service Unavailable")
		oRTTI_VALUE_CUSTOM(oHTTP_GATEWAY_TIMEOUT, "Gateway Timeout")
		oRTTI_VALUE_CUSTOM(oHTTP_VERSION_NOT_SUPPORTED, "Version Not Supported")
	oRTTI_ENUM_END_VALUES(oHTTP_STATUS_CODE)
oRTTI_ENUM_END_DESCRIPTION(oHTTP_STATUS_CODE)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oHTTP_METHOD)
	oRTTI_ENUM_BEGIN_VALUES(oHTTP_METHOD)
		oRTTI_VALUE_CUSTOM(oHTTP_OPTIONS, "OPTIONS")
		oRTTI_VALUE_CUSTOM(oHTTP_GET, "GET")
		oRTTI_VALUE_CUSTOM(oHTTP_HEAD, "HEAD")
		oRTTI_VALUE_CUSTOM(oHTTP_POST, "POST")
		oRTTI_VALUE_CUSTOM(oHTTP_PUT, "PUT")
		oRTTI_VALUE_CUSTOM(oHTTP_DELETE, "DELETE")
		oRTTI_VALUE_CUSTOM(oHTTP_TRACE, "TRACE")
		oRTTI_VALUE_CUSTOM(oHTTP_CONNECT, "CONNECT")
		oRTTI_VALUE_CUSTOM(oHTTP_PATCH, "PATCH")
	oRTTI_ENUM_END_VALUES(oHTTP_METHOD)
oRTTI_ENUM_END_DESCRIPTION(oHTTP_METHOD)

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oHTTP_HEADER_FIELD)
	oRTTI_ENUM_BEGIN_VALUES(oHTTP_HEADER_FIELD)
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCEPT, "Accept")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCEPT_CHARSET, "Accept-Charset")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCEPT_ENCODING, "Accept-Encoding")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCEPT_LANGUAGE, "Accept-Language")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCEPT_RANGES, "Accept-Ranges")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "Access-Control-Allow-Origin")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_AGE, "Age")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ALLOW, "Allow")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_AUTHORIZATION, "Authorization")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CACHE_CONTROL, "Cache-Control")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONNECTION, "Connection")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_ENCODING, "Content-Encoding")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_LANGUAGE, "Content-Language")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_LENGTH, "Content-Length")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_LOCATION, "Content-Location")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_MD5, "Content-MD5")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_RANGE, "Content-Range")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_CONTENT_TYPE, "Content-Type")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_DATE, "Date")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_ETAG, "ETag")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_EXPECT, "Expect")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_EXPIRES, "Expires")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_FROM, "From")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_HOST, "Host")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_IF_MATCH, "If-Match")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_IF_MODIFIED_SINCE, "If-Modified-Since")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_IF_NONE_MATCH, "If-None-Match")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_IF_RANGE, "If-Range")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_IF_UNMODIFIED_SINCE, "If-Unmodified-Since")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_LAST_MODIFIED, "Last-Modified")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_LOCATION, "Location")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_MAX_FORWARDS, "Max-Forwards")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_PRAGMA, "Pragma")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_PROXY_AUTHENTICATE, "Proxy-Authenticate")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_PROXY_AUTHORIZATION, "Proxy-Authorization")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_RANGE, "Range")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_REFERER, "Referer")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_RETRY_AFTER, "Retry-After")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_SERVER, "Server")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_TE, "TE")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_TRAILER, "Trailer")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_TRANSFER_ENCODING, "Transfer-Encoding")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_UPGRADE, "Upgrade")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_USER_AGENT, "User-Agent")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_VARY, "Vary")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_VIA, "Via")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_WARNING, "Warning")
		oRTTI_VALUE_CUSTOM(oHTTP_HEADER_WWW_AUTHENTICATE, "WWW-Authenticate")
	oRTTI_ENUM_END_VALUES(oHTTP_HEADER_FIELD)
oRTTI_ENUM_END_DESCRIPTION(oHTTP_HEADER_FIELD)

namespace ouro {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderField& _Fields) 
{
	if (-1 == snprintf(_StrDestination, _SizeofStrDestination, "%s:%s" oNEWLINE, _Fields.Key.c_str(), _Fields.Value.c_str()))
		return nullptr;

	return _StrDestination;
}

bool from_string(oHTTPHeaderField* _pValue, const char* _StrSource)
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
// 	if (!ouro::from_string(&_pValue->Field, Value))
// 		_pValue->Field = oHTTP_HEADER_NON_STANDARD;
	oStrTokClose(&ctx);
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPRequestInternal& _Request) 
{ 
	if (!ouro::to_string(_StrDestination, _SizeofStrDestination, _Request.RequestLine))
		return nullptr;

	char* StrDest = ouro::byte_add(_StrDestination, strlen(_StrDestination));
	size_t SizeRemaining = _SizeofStrDestination - (StrDest - _StrDestination);

	if (!ouro::to_string(StrDest, SizeRemaining, _Request.HeaderFieldsInternal))
		return nullptr;

	if (ouro::sncatf(_StrDestination, _SizeofStrDestination, oNEWLINE))
		return nullptr;

	return _StrDestination;
}

bool from_string(oHTTPRequestInternal* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* RequestLine = oStrTok(_StrSource, oNEWLINE, &ctx);
	if (!RequestLine) return false;
	ouro::finally free_ctx([&]() { oStrTokClose(&ctx); });
	if (!ouro::from_string(&_pValue->RequestLine, RequestLine))
		return false;
	if (!ouro::from_string(&_pValue->HeaderFieldsInternal, ctx))
		return false;
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTP_RESPONSE::oHTTP_STATUS_LINE& _StatusLine) 
{ 
	// From RFC 2616 - HTTP/1.1: 
	// CR             = <US-ASCII CR, carriage return (13)>
	// LF             = <US-ASCII LF, linefeed (10)>
	// SP             = <US-ASCII SP, space (32)>
	// Status-Line	  = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%s %u %s" oNEWLINE, ouro::as_string(_StatusLine.Version), _StatusLine.StatusCode, _StatusLine.ReasonPhrase.c_str()) ? _StrDestination : nullptr; 
}

bool from_string(oHTTP_RESPONSE::oHTTP_STATUS_LINE* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* Version = oStrTok(_StrSource, " ", &ctx);
	if (!Version) return false;
	ouro::finally free_ctx([&]() { oStrTokClose(&ctx); });
	if (!ouro::from_string(&_pValue->Version, Version))
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

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPResponseInternal& _Response) 
{ 
	if (!ouro::to_string(_StrDestination, _SizeofStrDestination, _Response.StatusLine))
		return nullptr;

	char* StrDest = ouro::byte_add(_StrDestination, strlen(_StrDestination));
	size_t SizeRemaining = _SizeofStrDestination - (StrDest - _StrDestination);

	if (!ouro::to_string(StrDest, SizeRemaining, _Response.HeaderFieldsInternal))
		return nullptr;

	if (ouro::sncatf(_StrDestination, _SizeofStrDestination, oNEWLINE))
		return nullptr;

	return _StrDestination;
}

bool from_string(oHTTPResponseInternal* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* RequestLine = oStrTok(_StrSource, oNEWLINE, &ctx);
	if (!RequestLine) return false;
	ouro::finally free_ctx([&]() { oStrTokClose(&ctx); });
	if (!ouro::from_string(&_pValue->StatusLine, RequestLine))
		return false;
	if (!ouro::from_string(&_pValue->HeaderFieldsInternal, ctx))
		return false;
	return true;
}

} // namespace ouro

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

char* ouro::to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTPHeaderFields& _Fields) 
{
	char* NextLine = _StrDestination;
	size_t SizeRemaining = _SizeofStrDestination;
	for (unsigned int i=0; i<_Fields.Vector.size(); ++i)
	{
		if (!ouro::to_string(NextLine, SizeRemaining, _Fields.Vector[i]))
			return nullptr;

		size_t LineLength = strlen(NextLine);
		NextLine += LineLength;
		SizeRemaining -= LineLength;
	}
	return _StrDestination;
}
bool ouro::from_string(oHTTPHeaderFields* _pValue, const char* _StrSource)
{
	// FIXME: oStrTok will probably not return the empty line and skip right over the two newlines in a row...
	char* ctx;
	char* Header = oStrTok(_StrSource, oNEWLINE, &ctx);
	ouro::finally free_ctx([&]() { oStrTokClose(&ctx); });
	while (Header)
	{
		oHTTPHeaderField Field;
		if (!ouro::from_string(&Field, Header))
			return false;
		_pValue->Vector.push_back(Field);

		// Try to get next header line
		Header = oStrTok(nullptr, oNEWLINE, &ctx);
	}

	return true;
}

char* ouro::to_string(char* _StrDestination, size_t _SizeofStrDestination, const oHTTP_REQUEST::oHTTP_REQUEST_LINE& _RequestLine) 
{ 
	// From RFC 2616 - HTTP/1.1: 
	// CR             = <US-ASCII CR, carriage return (13)>
	// LF             = <US-ASCII LF, linefeed (10)>
	// SP             = <US-ASCII SP, space (32)>
	// Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%s %s %s" oNEWLINE, ouro::as_string(_RequestLine.Method), _RequestLine.RequestURI.c_str(), ouro::as_string(_RequestLine.Version)) ? _StrDestination : nullptr; 
}
bool ouro::from_string(oHTTP_REQUEST::oHTTP_REQUEST_LINE* _pValue, const char* _StrSource)
{
	char* ctx = nullptr;
	char* Value = oStrTok(_StrSource, " ", &ctx);
	ouro::finally free_ctx([&]() { oStrTokClose(&ctx); });
	if (!Value) return false;
	if (!ouro::from_string(&_pValue->Method, Value))
		return false;

	Value = oStrTok(nullptr, " ", &ctx);
	if (!Value) return false;
	_pValue->RequestURI = Value;

	Value = oStrTok(nullptr, " " oNEWLINE, &ctx);
	if (!Value) return false;
	if (!ouro::from_string(&_pValue->Version, Value))
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

oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char* _Value)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->AddHeader(ouro::as_string(_Field), _Value);
}
oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int _Value)
{
	if(!_HeaderFields)
		return false;
	ouro::sstring StrValue;
	snprintf(StrValue, "%u", _Value);
	return _HeaderFields->AddHeader(ouro::as_string(_Field), StrValue.c_str());
}
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char** _Value)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->FindHeader(ouro::as_string(_Field), _Value);
}
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int* _Value)
{
	if(!_HeaderFields)
		return false;
	const char* Value;
	if (!_HeaderFields->FindHeader(ouro::as_string(_Field), &Value))
		return false;
	return ouro::from_string(_Value, Value);
}
oAPI bool oHTTPRemoveHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field)
{
	if(!_HeaderFields)
		return false;
	return _HeaderFields->RemoveHeader(ouro::as_string(_Field));
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
bool oExtractHTTPHeader(const char* _pData, size_t _SizeofData, char* _pHeader, size_t* _pSizeofHeader, size_t _MaxHeaderSize, size_t* _pSizeofDataTaken)
{
	// If the current header size is bigger than the maximum, we're done
	if (*_pSizeofHeader >= _MaxHeaderSize)
		return false;

	// First copy the new data to the header buffer, since the end-marker (two oNEWLINEs in a row) might be split into multiple data packets
	memcpy_s(ouro::byte_add(_pHeader, *_pSizeofHeader), (_MaxHeaderSize - *_pSizeofHeader), _pData, _SizeofData);

	// Put a zero terminator at the end of what we just copied (or at the end of the buffer in case of it filling up)
	_pHeader[__min(*_pSizeofHeader + _SizeofData, _MaxHeaderSize - 1)] = 0;

	char* HeaderEnd = (char*)GetEndHeaders(_pHeader);
	if (!HeaderEnd)
	{
		// The end marker hasn't been found yet, so if there's a zero terminator in the just copied data, we will never find an end marker thus error out
		if (memchr(_pData, 0, _SizeofData))
			return false;

		*_pSizeofDataTaken += _SizeofData;
		*_pSizeofHeader += _SizeofData; // This can make the current header size bigger than the maximum, but this should be handled properly
		return false;
	}

	// Zero terminate, ouro::from_string() needs to know when to stop parsing header fields
	HeaderEnd[0] = 0;

	*_pSizeofDataTaken += ((HeaderEnd - _pHeader) - *_pSizeofHeader);
	*_pSizeofHeader = HeaderEnd - _pHeader;
	return true;
}

// Take bytes from _pData until we have the full content (_ContentSize) and return true, return false when needing another callback with more data
bool oExtractContent(const void* _pData, size_t _SizeofData, void* _pContent, size_t* _pSizeofContent, size_t _TotalContentSize, size_t* _pSizeofDataTaken)
{
	size_t NewContentSize = *_pSizeofContent + _SizeofData;
	if (NewContentSize > _TotalContentSize)
		NewContentSize = _TotalContentSize;

	size_t SizeToCopy = NewContentSize - *_pSizeofContent;
	if (SizeToCopy)
		memcpy_s(ouro::byte_add(_pContent, *_pSizeofContent), (_TotalContentSize - *_pSizeofContent), _pData, SizeToCopy);

	*_pSizeofDataTaken += NewContentSize - *_pSizeofContent;
	*_pSizeofContent = NewContentSize;
	return (NewContentSize == _TotalContentSize);
}

// Insert content into a buffer until it's full (_TotalBufferSize) and return true, return false when needing another callback with more content
// If _pSizeofContentTaken is not set, we'll fail on buffer overflow
bool oInsertContent(const void* _pContent, size_t _SizeofContent, void* _pBuffer, size_t* _pSizeofBuffer, size_t _TotalBufferSize, size_t* _pSizeofContentTaken)
{
	size_t NewBufferSize = _SizeofContent + *_pSizeofBuffer;
	if (NewBufferSize > _TotalBufferSize)
		NewBufferSize = _TotalBufferSize;

	size_t SizeToCopy = NewBufferSize - *_pSizeofBuffer;
	if (SizeToCopy)
		memcpy_s(ouro::byte_add(_pBuffer, *_pSizeofBuffer), _TotalBufferSize - *_pSizeofBuffer, _pContent, SizeToCopy);

	if (_pSizeofContentTaken)
	{
		*_pSizeofContentTaken = NewBufferSize - *_pSizeofBuffer;
	}
	else
	{
		*_pSizeofBuffer += _SizeofContent;
		return _TotalBufferSize == *_pSizeofBuffer;
	}

	*_pSizeofBuffer = NewBufferSize;
	return _TotalBufferSize == NewBufferSize;
}

