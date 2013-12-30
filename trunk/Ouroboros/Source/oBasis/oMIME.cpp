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
#include <oBasis/oMIME.h>
#include <unordered_map>
#include <oBase/algorithm.h>
#include <oBasis/oInvalid.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oMIME_TYPE)
	oRTTI_ENUM_BEGIN_VALUES(oMIME_TYPE)
		oRTTI_VALUE_CUSTOM(oMIME_UNKNOWN, "unknown")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_ATOMXML, "application/atom+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_ECMASCRIPT, "application/ecmascript")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_EDIX12, "application/EDI-X12")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_EDIFACT, "application/EDITFACT")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_EXE , "application/exe")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_JSON, "application/json")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_JAVASCRIPT, "application/javascript")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_OCTETSTREAM, "application/octet-stream")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_OGG, "application/ogg")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_PDF, "application/pdf")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_POSTSCRIPT, "application/postscript")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_RDFXML, "application/rdf+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_RSSXML, "application/rss+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_SOAPXML, "application/soap+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_FONTWOFF, "application/font-woff")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_XDMP, "application/x-dmp")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_XHTMLXML, "application/xhtml+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_XMLDTD, "application/xml-dtd")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_XOPXML, "application/xop+xml")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_ZIP, "application/zip")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_GZIP, "application/x-gzip")
		oRTTI_VALUE_CUSTOM(oMIME_APPLICATION_7Z, "application/x-7z-compressed")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_BASIC, "audio/basic")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_L24, "audio/l24")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_MP4, "audio/mp4")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_MPEG, "audio/mpeg")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_OGG, "audio/ogg")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_VORBIS, "audio/vorbis")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_WMA, "audio/x-ms-wma")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_WAX, "audio/x-ms-wax")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_REALAUDIO, "audio/vnd.rn-realaudio")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_WAV, "audio/vnd.wave")
		oRTTI_VALUE_CUSTOM(oMIME_AUDIO_WEBM, "audio/webm")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_GIF, "image/gif")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_JPEG, "image/jpeg")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_PJPEG, "image/pjpeg")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_PNG, "image/png")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_SVGXML, "imagesvg+xml")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_TIFF, "image/tiff")
		oRTTI_VALUE_CUSTOM(oMIME_IMAGE_ICO, "image/vnd.microsoft.icon")
		oRTTI_VALUE_CUSTOM(oMIME_MESSAGE_HTTP, "message/http")
		oRTTI_VALUE_CUSTOM(oMIME_MESSAGE_IMDNXML, "message/imdn+xml")
		oRTTI_VALUE_CUSTOM(oMIME_MESSAGE_PARTIAL, "message/partial")
		oRTTI_VALUE_CUSTOM(oMIME_MESSAGE_RFC822, "message/rfc822")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_EXAMPLE, "model/example")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_IGES, "model/iges")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_MESH, "model/mesh")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_VRML, "model/vrml")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_X3DBINARY, "model/x3d+binary")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_X3DVRML, "model/x3d+vrml")
		oRTTI_VALUE_CUSTOM(oMIME_MODEL_X3DXML, "model/x3d+xml")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_MIXED, "multipart/mixed")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_ALTERNATIVE, "multipart/alternative")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_RELATED, "multipart/related")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_FORMDATA, "multipart/form-data")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_SIGNED, "multipart/signed")
		oRTTI_VALUE_CUSTOM(oMIME_MULTIPART_ENCRYPTED, "multipart/encrypted")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_CMD, "text/cmd")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_CSS, "text/css")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_CSV, "text/csv")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_HTML, "text/html")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_JAVASCRIPT, "text/javascript")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_PLAIN, "text/plain")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_VCARD, "text/vcard")
		oRTTI_VALUE_CUSTOM(oMIME_TEXT_XML, "text/xml")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_MPEG, "video/mpeg")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_MP4, "video/mp4")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_OGG, "video/ogg")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_QUICKTIME, "video/quicktime")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_WEBM, "video/webm")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_MATROSKA, "video/x-matroska")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_WMV, "video/x-ms-wmv")
		oRTTI_VALUE_CUSTOM(oMIME_VIDEO_FLASH_VIDEO, "video/x-flv")
	oRTTI_ENUM_END_VALUES(oMIME_TYPE)
	oRTTI_ENUM_VALIDATE_COUNT(oMIME_TYPE, oMIME_TYPE_COUNT)
oRTTI_ENUM_END_DESCRIPTION(oMIME_TYPE)

namespace 
{
	class oExtensionToMime
	{
	public:
		oExtensionToMime()
		{
			//html 
			LookupTable[std::string(".htm")] = oMIME_TEXT_HTML;
			LookupTable[std::string(".html")] = oMIME_TEXT_HTML;
			LookupTable[std::string(".css")] = oMIME_TEXT_CSS;
			LookupTable[std::string(".jss")] = oMIME_TEXT_JAVASCRIPT;
			LookupTable[std::string(".js")] = oMIME_TEXT_JAVASCRIPT;
			LookupTable[std::string(".txt")] = oMIME_TEXT_PLAIN;

			//images
			LookupTable[std::string(".jpg")] = oMIME_IMAGE_JPEG;
			LookupTable[std::string(".jpeg")] = oMIME_IMAGE_JPEG;
			LookupTable[std::string(".png")] = oMIME_IMAGE_PNG;
			LookupTable[std::string(".giv")] = oMIME_IMAGE_GIF;
			LookupTable[std::string(".ico")] = oMIME_IMAGE_ICO;
			LookupTable[std::string(".gif")] = oMIME_IMAGE_GIF;

			//audio
			LookupTable[std::string(".mp3")] = oMIME_AUDIO_MPEG;
			LookupTable[std::string(".wav")] = oMIME_AUDIO_WAV;
			LookupTable[std::string(".wma")] = oMIME_AUDIO_WMA;
			LookupTable[std::string(".ogg")] = oMIME_VIDEO_OGG;

			//video
			LookupTable[std::string(".mpeg")] = oMIME_VIDEO_MPEG;
			LookupTable[std::string(".mov")] = oMIME_VIDEO_QUICKTIME;
			LookupTable[std::string(".webm")] = oMIME_VIDEO_WEBM;

			//other
			LookupTable[std::string(".pdf")] = oMIME_APPLICATION_PDF;

		}
		oMIME_TYPE operator()(const char* _Extension)
		{
			auto mimeIter = LookupTable.find(std::string(_Extension));
			if(mimeIter != cend(LookupTable))
			{
				return mimeIter->second;
			}
			return oMIME_UNKNOWN;
		}
	private:
		//note that extensions are short, we will always take advandtage of the small string optimization, so no heap allocs for the std::strings
		std::unordered_map<std::string, oMIME_TYPE> LookupTable; 
	};

	oExtensionToMime ExtensionToMime;
}

bool oMIMEFromExtension(oMIME_TYPE* _pType, const char* _Extension)
{
	*_pType = ExtensionToMime(_Extension);

	return *_pType != oMIME_UNKNOWN;
}