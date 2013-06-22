/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Internet media type:
// http://en.wikipedia.org/wiki/Mime_type
#pragma once
#ifndef oMIME_h
#define oMIME_h

#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oRTTI.h>

enum oMIME_TYPE
{
	// There is no unknown type in the standard, in a such a case a type name is 
	// not supposed to be sent. For example in the http protocol when an unknown 
	// MIME type is encountered just don't send the Content-Type header.
	oMIME_UNKNOWN,

	oMIME_APPLICATION_ATOMXML,
	oMIME_APPLICATION_ECMASCRIPT,
	oMIME_APPLICATION_EDIX12,
	oMIME_APPLICATION_EDIFACT,
	oMIME_APPLICATION_EXE,
	oMIME_APPLICATION_JSON,
	oMIME_APPLICATION_JAVASCRIPT,
	oMIME_APPLICATION_OCTETSTREAM,
	oMIME_APPLICATION_OGG,
	oMIME_APPLICATION_PDF,
	oMIME_APPLICATION_POSTSCRIPT,
	oMIME_APPLICATION_RDFXML,
	oMIME_APPLICATION_RSSXML,
	oMIME_APPLICATION_SOAPXML,
	oMIME_APPLICATION_FONTWOFF,
	oMIME_APPLICATION_XDMP,
	oMIME_APPLICATION_XHTMLXML,
	oMIME_APPLICATION_XMLDTD,
	oMIME_APPLICATION_XOPXML,
	oMIME_APPLICATION_ZIP,
	oMIME_APPLICATION_GZIP,
	oMIME_APPLICATION_7Z,

	oMIME_AUDIO_BASIC,
	oMIME_AUDIO_L24,
	oMIME_AUDIO_MP4,
	oMIME_AUDIO_MPEG,
	oMIME_AUDIO_OGG,
	oMIME_AUDIO_VORBIS,
	oMIME_AUDIO_WMA,
	oMIME_AUDIO_WAX,
	oMIME_AUDIO_REALAUDIO,
	oMIME_AUDIO_WAV,
	oMIME_AUDIO_WEBM,

	oMIME_IMAGE_GIF,
	oMIME_IMAGE_JPEG,
	oMIME_IMAGE_PJPEG,
	oMIME_IMAGE_PNG,
	oMIME_IMAGE_SVGXML,
	oMIME_IMAGE_TIFF,
	oMIME_IMAGE_ICO,

	oMIME_MESSAGE_HTTP,
	oMIME_MESSAGE_IMDNXML,
	oMIME_MESSAGE_PARTIAL,
	oMIME_MESSAGE_RFC822,

	oMIME_MODEL_EXAMPLE,
	oMIME_MODEL_IGES,
	oMIME_MODEL_MESH,
	oMIME_MODEL_VRML,
	oMIME_MODEL_X3DBINARY,
	oMIME_MODEL_X3DVRML,
	oMIME_MODEL_X3DXML,

	oMIME_MULTIPART_MIXED,
	oMIME_MULTIPART_ALTERNATIVE,
	oMIME_MULTIPART_RELATED,
	oMIME_MULTIPART_FORMDATA,
	oMIME_MULTIPART_SIGNED,
	oMIME_MULTIPART_ENCRYPTED,

	oMIME_TEXT_CMD,
	oMIME_TEXT_CSS,
	oMIME_TEXT_CSV,
	oMIME_TEXT_HTML,
	oMIME_TEXT_JAVASCRIPT,
	oMIME_TEXT_PLAIN,
	oMIME_TEXT_VCARD,
	oMIME_TEXT_XML,

	oMIME_VIDEO_MPEG,
	oMIME_VIDEO_MP4,
	oMIME_VIDEO_OGG,
	oMIME_VIDEO_QUICKTIME,
	oMIME_VIDEO_WEBM,
	oMIME_VIDEO_MATROSKA,
	oMIME_VIDEO_WMV,
	oMIME_VIDEO_FLASH_VIDEO,

	oMIME_TYPE_COUNT,
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oMIME_TYPE)

//only common extensions supported currently. if returns false, _pType will also be set to oMIME_UNKNOWN
oAPI bool oMIMEFromExtension(oMIME_TYPE* _pType, const char* _Extension);

#endif
