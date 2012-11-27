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
#include <oBasis/oMIME.h>
#include <oBasis/oStringize.h>
#include <unordered_map>
#include <oBasis/oAlgorithm.h>

const char* oAsString(oMIME_TYPE _Type)
{
	switch (_Type)
	{
		case oMIME_APPLICATION_ATOMXML: return "application/atom+xml";
		case oMIME_APPLICATION_ECMASCRIPT: return "application/ecmascript";
		case oMIME_APPLICATION_EDIX12: return "application/EDI-X12";
		case oMIME_APPLICATION_EDIFACT: return "application/EDITFACT";
		case oMIME_APPLICATION_EXE : return "application/exe";
		case oMIME_APPLICATION_JSON: return "application/json";
		case oMIME_APPLICATION_JAVASCRIPT: return "application/javascript";
		case oMIME_APPLICATION_OCTETSTREAM: return "application/octet-stream";
		case oMIME_APPLICATION_OGG: return "application/ogg";
		case oMIME_APPLICATION_PDF: return "application/pdf";
		case oMIME_APPLICATION_POSTSCRIPT: return "application/postscript";
		case oMIME_APPLICATION_RDFXML: return "application/rdf+xml";
		case oMIME_APPLICATION_RSSXML: return "application/rss+xml";
		case oMIME_APPLICATION_SOAPXML: return "application/soap+xml";
		case oMIME_APPLICATION_FONTWOFF: return "application/font-woff";
		case oMIME_APPLICATION_XDMP: return "application/x-dmp";
		case oMIME_APPLICATION_XHTMLXML: return "application/xhtml+xml";
		case oMIME_APPLICATION_XMLDTD: return "application/xml-dtd";
		case oMIME_APPLICATION_XOPXML: return "application/xop+xml";
		case oMIME_APPLICATION_ZIP: return "application/zip";
		case oMIME_APPLICATION_GZIP: return "application/x-gzip";
		case oMIME_APPLICATION_7Z: return "application/x-7z-compressed";
		case oMIME_AUDIO_BASIC: return "audio/basic";
		case oMIME_AUDIO_L24: return "audio/l24";
		case oMIME_AUDIO_MP4: return "audio/mp4";
		case oMIME_AUDIO_MPEG: return "audio/mpeg";
		case oMIME_AUDIO_OGG: return "audio/ogg";
		case oMIME_AUDIO_VORBIS: return "audio/vorbis";
		case oMIME_AUDIO_WMA: return "audio/x-ms-wma";
		case oMIME_AUDIO_WAX: return "audio/x-ms-wax";
		case oMIME_AUDIO_REALAUDIO: return "audio/vnd.rn-realaudio";
		case oMIME_AUDIO_WAV: return "audio/vnd.wave";
		case oMIME_AUDIO_WEBM: return "audio/webm";
		case oMIME_IMAGE_GIF: return "image/gif";
		case oMIME_IMAGE_JPEG: return "image/jpeg";
		case oMIME_IMAGE_PJPEG: return "image/pjpeg";
		case oMIME_IMAGE_PNG: return "image/png";
		case oMIME_IMAGE_SVGXML: return "imagesvg+xml";
		case oMIME_IMAGE_TIFF: return "image/tiff";
		case oMIME_IMAGE_ICO: return "image/vnd.microsoft.icon";
		case oMIME_MESSAGE_HTTP: return "message/http";
		case oMIME_MESSAGE_IMDNXML: return "message/imdn+xml";
		case oMIME_MESSAGE_PARTIAL: return "message/partial";
		case oMIME_MESSAGE_RFC822: return "message/rfc822";
		case oMIME_MODEL_EXAMPLE: return "model/example";
		case oMIME_MODEL_IGES: return "model/iges";
		case oMIME_MODEL_MESH: return "model/mesh";
		case oMIME_MODEL_VRML: return "model/vrml";
		case oMIME_MODEL_X3DBINARY: return "model/x3d+binary";
		case oMIME_MODEL_X3DVRML: return "model/x3d+vrml";
		case oMIME_MODEL_X3DXML: return "model/x3d+xml";
		case oMIME_MULTIPART_MIXED: return "multipart/mixed";
		case oMIME_MULTIPART_ALTERNATIVE: return "multipart/alternative";
		case oMIME_MULTIPART_RELATED: return "multipart/related";
		case oMIME_MULTIPART_FORMDATA: return "multipart/form-data";
		case oMIME_MULTIPART_SIGNED: return "multipart/signed";
		case oMIME_MULTIPART_ENCRYPTED: return "multipart/encrypted";
		case oMIME_TEXT_CMD: return "text/cmd";
		case oMIME_TEXT_CSS: return "text/css";
		case oMIME_TEXT_CSV: return "text/csv";
		case oMIME_TEXT_HTML: return "text/html";
		case oMIME_TEXT_JAVASCRIPT: return "text/javascript";
		case oMIME_TEXT_PLAIN: return "text/plain";
		case oMIME_TEXT_VCARD: return "text/vcard";
		case oMIME_TEXT_XML: return "text/xml";
		case oMIME_VIDEO_MPEG: return "video/mpeg";
		case oMIME_VIDEO_MP4: return "video/mp4";
		case oMIME_VIDEO_OGG: return "video/ogg";
		case oMIME_VIDEO_QUICKTIME: return "video/quicktime";
		case oMIME_VIDEO_WEBM: return "video/webm";
		case oMIME_VIDEO_MATROSKA: return "video/x-matroska";
		case oMIME_VIDEO_WMV: return "video/x-ms-wmv";
		case oMIME_VIDEO_FLASH_VIDEO: return "video/x-flv";
		default: return "Unrecognized MIME type";
	}
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oMIME_TYPE& _Type)
{
	oStrcpy(_StrDestination, _SizeofStrDestination, oAsString(_Type));
	return _StrDestination;
}

bool oFromString(oMIME_TYPE* _pValue, const char* _StrSource)
{
	return oEnumFromString<oMIME_NUM_TYPES>(_pValue, _StrSource);
}

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