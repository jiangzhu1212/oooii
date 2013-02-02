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
#pragma once
#ifndef oHTTP_h
#define oHTTP_h
//
// This is our implementation of the Hypertext Transfer Protocol -- HTTP/1.1 (http://www.w3.org/Protocols/rfc2616/rfc2616.html)
//
// Other versions known today:
// HTTP/0.9 was a simple protocol for raw data transfer across the Internet
// HTTP/1.0 as defined by RFC 1945, improved the protocol by allowing messages to be in the format of MIME-like messages, 
//			containing meta information about the data transferred and modifiers on the request/response semantics
// HTTP/1.1 includes more stringent requirements than HTTP/1.0 in order to ensure reliable implementation of its features
// HTTP/1.2 Extension Protocol (PEP) (draft, since 1996?)
//
// Practical information systems require more functionality than simple retrieval, including search, front-end update, and annotation.
// HTTP allows an open-ended set of methods and headers that indicate the purpose of a request. It builds on the discipline of reference provided 
// by the Uniform Resource Identifier (URI), as a location (URL) or name (URN), for indicating the resource to which a method is to be applied.
//
// Messages are passed in a format similar to that used by Internet mail as defined by the Multipurpose Internet Mail Extensions (MIME)
//
// HTTP is also used as a generic protocol for communication between user agents and proxies/gateways to other Internet systems, including those 
// supported by the SMTP, NNTP, FTP, Gopher, and WAIS protocols. In this way, HTTP allows basic hypermedia access to resources available from diverse applications.
//
// Supported:
// - Keep-Alive
// - Connection Timeouts
// - Asynchronous and threadsafe server
//
// Currently NOT supported:
// - Automatic redirection
// - International domains and URLs
// - Connection Pooling
// - Sessions with Cookie persistence
// - Browser style SSL verification
// - Basic/Digest Authentication
// - OAuth Authentication
// - Elegant Key/Value Cookies
// - Automatic Decompression (gzip/deflate)
// - Unicode response bodies
// - Multi-part File Uploads
// - Password manager / .netrc support
// - Asynchronous and threadsafe client
// - Form fields


#include <oBasis/oMIME.h>

enum oHTTP_VERSION
{
	oHTTP_VERSION_UNKNOWN,
	oHTTP_0_9,
	oHTTP_1_0,
	oHTTP_1_1,
	oHTTP_1_2,

	oHTTP_NUM_VERSIONS = 4,
};

enum oHTTP_STATUS_CODE
{
	oHTTP_CONTINUE = 100,
	oHTTP_SWITCHING_PROTOCOLS = 101,
	oHTTP_OK = 200,
	oHTTP_CREATED = 201,
	oHTTP_ACCEPTED = 202,
	oHTTP_NON_AUTHORITATIVE_INFORMATION = 203,
	oHTTP_NO_CONTENT = 204,
	oHTTP_RESET_CONTENT = 205,
	oHTTP_PARTIAL_CONTENT = 206,
	oHTTP_MULTIPLE_CHOICES = 300,
	oHTTP_MOVED_PERMANENTLY = 301,
	oHTTP_FOUND = 302,
	oHTTP_SEE_OTHER = 303,
	oHTTP_NOT_MODIFIED = 304,
	oHTTP_USE_PROXY = 305,
	oHTTP_TEMPORARY_REDIRECT = 307,
	oHTTP_BAD_REQUEST = 400,
	oHTTP_UNAUTHORIZED = 401,
	oHTTP_PAYMENT_REQUIRED = 402,
	oHTTP_FORBIDDEN = 403,
	oHTTP_NOT_FOUND = 404,
	oHTTP_METHOD_NOT_ALLOWED = 405,
	oHTTP_NOT_ACCEPTABLE = 406,
	oHTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
	oHTTP_REQUEST_TIMEOUT = 408,
	oHTTP_CONFLICT = 409,
	oHTTP_GONE = 410,
	oHTTP_LENGTH_REQUIRED = 411,
	oHTTP_PRECONDITION_FAILED = 412,
	oHTTP_REQUEST_ENTITY_TOO_LARGE = 413,
	oHTTP_REQUEST_URI_TOO_LONG = 414,
	oHTTP_UNSUPPORTED_MEDIA_TYPE = 415,
	oHTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
	oHTTP_EXPECTATION_FAILED = 417,
	oHTTP_CLIENT_CLOSED_REQUEST = 499,
	oHTTP_INTERNAL_SERVER_ERROR = 500,
	oHTTP_NOT_IMPLEMENTED = 501,
	oHTTP_BAD_GATEWAY = 502,
	oHTTP_SERVICE_UNAVAILABLE = 503,
	oHTTP_GATEWAY_TIMEOUT = 504,
	oHTTP_VERSION_NOT_SUPPORTED = 505,

	oHTTP_NUM_RESPONSE_CODES = 40,
};

enum oHTTP_METHOD
{
	oHTTP_OPTIONS	= 1<<0,
	oHTTP_GET		= 1<<1,
	oHTTP_HEAD		= 1<<2,
	oHTTP_POST		= 1<<3,
	oHTTP_PUT		= 1<<4,
	oHTTP_DELETE	= 1<<5,
	oHTTP_TRACE		= 1<<6,
	oHTTP_CONNECT	= 1<<7,
	oHTTP_PATCH		= 1<<8,

	oHTTP_NUM_METHODS = 9,
};

enum oHTTP_HEADER_FIELD
{
	oHTTP_HEADER_ACCEPT,
	oHTTP_HEADER_ACCEPT_CHARSET,
	oHTTP_HEADER_ACCEPT_ENCODING,
	oHTTP_HEADER_ACCEPT_LANGUAGE,
	oHTTP_HEADER_ACCEPT_RANGES,
	oHTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN,
	oHTTP_HEADER_AGE,
	oHTTP_HEADER_ALLOW,
	oHTTP_HEADER_AUTHORIZATION,
	oHTTP_HEADER_CACHE_CONTROL,
	oHTTP_HEADER_CONNECTION,
	oHTTP_HEADER_CONTENT_ENCODING,
	oHTTP_HEADER_CONTENT_LANGUAGE,
	oHTTP_HEADER_CONTENT_LENGTH,
	oHTTP_HEADER_CONTENT_LOCATION,
	oHTTP_HEADER_CONTENT_MD5,
	oHTTP_HEADER_CONTENT_RANGE,
	oHTTP_HEADER_CONTENT_TYPE,
	oHTTP_HEADER_DATE,
	oHTTP_HEADER_ETAG,
	oHTTP_HEADER_EXPECT,
	oHTTP_HEADER_EXPIRES,
	oHTTP_HEADER_FROM,
	oHTTP_HEADER_HOST,
	oHTTP_HEADER_IF_MATCH,
	oHTTP_HEADER_IF_MODIFIED_SINCE,
	oHTTP_HEADER_IF_NONE_MATCH,
	oHTTP_HEADER_IF_RANGE,
	oHTTP_HEADER_IF_UNMODIFIED_SINCE,
	oHTTP_HEADER_LAST_MODIFIED,
	oHTTP_HEADER_LOCATION,
	oHTTP_HEADER_MAX_FORWARDS,
	oHTTP_HEADER_PRAGMA,
	oHTTP_HEADER_PROXY_AUTHENTICATE,
	oHTTP_HEADER_PROXY_AUTHORIZATION,
	oHTTP_HEADER_RANGE,
	oHTTP_HEADER_REFERER,
	oHTTP_HEADER_RETRY_AFTER,
	oHTTP_HEADER_SERVER,
	oHTTP_HEADER_TE,
	oHTTP_HEADER_TRAILER,
	oHTTP_HEADER_TRANSFER_ENCODING,
	oHTTP_HEADER_UPGRADE,
	oHTTP_HEADER_USER_AGENT,
	oHTTP_HEADER_VARY,
	oHTTP_HEADER_VIA,
	oHTTP_HEADER_WARNING,
	oHTTP_HEADER_WWW_AUTHENTICATE,

	oHTTP_HEADER_NON_STANDARD,

	oHTTP_NUM_HEADER_FIELDS,
};

oAPI const char* oAsString(oHTTP_STATUS_CODE _Code);
oAPI bool oFromString(oHTTP_STATUS_CODE* _pValue, const char* _StrSource);
oAPI const char* oAsString(oHTTP_METHOD _Request);
oAPI bool oFromString(oHTTP_METHOD* _pRequest, const char* _StrSource);
oAPI const char* oAsString(oHTTP_HEADER_FIELD _Request);
oAPI bool oFromString(oHTTP_HEADER_FIELD* _pRequest, const char* _StrSource);

typedef struct oHTTPHeaderFields* oHTTP_HEADER_FIELDS;

struct oHTTP_CONTENT_BODY
{
	oMIME_TYPE Type;
	void* pData;
	unsigned int Length;
};

struct oHTTP_REQUEST
{
	struct oHTTP_REQUEST_LINE
	{
		oHTTP_METHOD Method;
		oStringXXL RequestURI;
		oHTTP_VERSION Version;
	};
	oHTTP_REQUEST_LINE RequestLine;
	oHTTP_HEADER_FIELDS HeaderFields;
	oHTTP_CONTENT_BODY Content;
};

struct oHTTP_RESPONSE
{
	struct oHTTP_STATUS_LINE
	{
		oHTTP_VERSION Version;
		oHTTP_STATUS_CODE StatusCode;
		oStringS ReasonPhrase;
	};
	oHTTP_STATUS_LINE StatusLine;
	oHTTP_HEADER_FIELDS HeaderFields;
	oHTTP_CONTENT_BODY Content;
};

oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char* _Value);
oAPI bool oHTTPAddHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int _Value);
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, const char** _Value);
oAPI bool oHTTPFindHeader(const oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field, unsigned int* _Value);
oAPI bool oHTTPRemoveHeader(oHTTP_HEADER_FIELDS _HeaderFields, oHTTP_HEADER_FIELD _Field);
oAPI void oHTTPClearHeaders(oHTTP_HEADER_FIELDS _HeaderFields);

#endif // oHTTP_h