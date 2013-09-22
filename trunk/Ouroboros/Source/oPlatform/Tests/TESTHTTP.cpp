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
#include <oPlatform/oTest.h>
#include <oPlatform/oHTTPClient.h>
#include <oPlatform/oHTTPServer.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oMsgBox.h>
#include <oPlatform/oStream.h>
#include <oBase/finally.h>

#define TEST_FILE "PLATFORM_oHTTP.png"

struct PLATFORM_oHTTP : public oTest
{
	void StartResponse(const oHTTP_REQUEST& _Request, const oNetHost& _Client, oHTTP_RESPONSE* _pResponse)
	{
		if (_Request.RequestLine.Method == oHTTP_POST)
		{
			// Check if the image file came through correctly
			ouro::intrusive_ptr<oImage> image;
			oImageCreate("http test image", _Request.Content.pData, _Request.Content.Length, &image);
			if (TestImage(image))
			{
				const char *indexHtmlPage = "<html><head><title>Received Post</title></head><body><p>Image compare successful</p></body></html>";
				int size = (int)strlen(indexHtmlPage) + 1;
				_pResponse->Content.pData = new char[size];

				strlcpy((char *)_pResponse->Content.pData, indexHtmlPage, size);
				_pResponse->StatusLine.StatusCode = oHTTP_OK;
				_pResponse->Content.Type = oMIME_TEXT_HTML;
				_pResponse->Content.Length = oUInt(strlen(indexHtmlPage));
			}
			else
			{
				const char *indexHtmlPage = "<html><head><title>500 Internal Server Error</title></head><body><p>Internal Server Error, image compare failed.</p></body></html>";
				int size = (int)strlen(indexHtmlPage) + 1;
				_pResponse->Content.pData = new char[size];

				strlcpy((char *)_pResponse->Content.pData, indexHtmlPage, size);
				_pResponse->StatusLine.StatusCode = oHTTP_INTERNAL_SERVER_ERROR;
				_pResponse->Content.Type = oMIME_TEXT_HTML;
				_pResponse->Content.Length = oUInt(strlen(indexHtmlPage));
			}
		}
		else
		{
			//favicon gets first try
			if(strncmp(_Request.RequestLine.RequestURI, "/favicon.ico", 11) == 0)
			{
				extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);

				const char* name = nullptr;
				const void* pBuffer = nullptr;
				size_t size;
				GetDescoooii_ico(&name, &pBuffer, &size);
				_pResponse->Content.Length = oUInt(size);
				_pResponse->StatusLine.StatusCode = oHTTP_OK;
				_pResponse->Content.Type = oMIME_IMAGE_ICO;
				_pResponse->Content.pData = new char[_pResponse->Content.Length];
				memcpy(_pResponse->Content.pData, pBuffer, _pResponse->Content.Length);
			}
			else if (strcmp(_Request.RequestLine.RequestURI, "/ ") == 0 || strcmp(_Request.RequestLine.RequestURI, "/") == 0 || strcmp(_Request.RequestLine.RequestURI, "/index.html") == 0)
			{
				const char *indexHtmlPage = "<html><head><title>Test Page</title></head><body><p>Welcome to OOOii test HTTP server<br/><img src=\"" TEST_FILE "\"/></p></body></html>";
				int size = (int)strlen(indexHtmlPage) + 1;
				_pResponse->Content.pData = new char[size];

				strlcpy((char *)_pResponse->Content.pData, indexHtmlPage, size);
				_pResponse->Content.Type = oMIME_TEXT_HTML;
				_pResponse->StatusLine.StatusCode = oHTTP_OK;
				_pResponse->Content.Length = oUInt(strlen(indexHtmlPage));
			}
			else
				_pResponse->StatusLine.StatusCode = oHTTP_NOT_FOUND;
			if (strstr(_Request.RequestLine.RequestURI, TEST_FILE) != 0)
			{
				ouro::path defaultDataPath = ouro::filesystem::data_path();
				ouro::path_string golden;
				snprintf(golden, "%sGoldenImages%s", defaultDataPath.c_str(), _Request.RequestLine.RequestURI.c_str());

				if (oStreamReaderCreate(golden, &FileReader))
				{
					_pResponse->Content.Type = oMIME_IMAGE_PNG;
					_pResponse->StatusLine.StatusCode = oHTTP_OK;
					oSTREAM_DESC desc;
					FileReader->GetDesc(&desc);
					_pResponse->Content.Length = oUInt(desc.Size);

					_pResponse->Content.pData = new char[(int)desc.Size];
					memset(_pResponse->Content.pData, NULL, oSizeT(desc.Size));
					oSTREAM_READ r;
					r.pData = _pResponse->Content.pData;
					r.Range = desc;
					FileReader->Read(r);
				}
			}
		}
	}

	void FinishResponse(const void *_pMIMEData)
	{
		if (_pMIMEData != 0)
			oTRACE("Finished Sending Message: %p - %s", _pMIMEData, (const char*)_pMIMEData);
		// if we were using dynamic memory, free it here
		delete[] _pMIMEData;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const bool bDevelopmentMode = false;

		ouro::intrusive_ptr<oHTTPServer> Server;
		oHTTPServer::DESC desc;
		desc.Port = 80;
		desc.StartResponse = oBIND(&PLATFORM_oHTTP::StartResponse, this, oBIND1, oBIND2, oBIND3);
		desc.FinishResponse = oBIND(&PLATFORM_oHTTP::FinishResponse, this, oBIND1);

		ouro::intrusive_ptr<oHTTPClient> Client;
		oHTTPClient::DESC clientDesc;
		ouro::from_string(&clientDesc.ServerAddr, "localhost:80");
		if (!oHTTPServerCreate(desc, &Server))
		{
			if (oErrorGetLast() == std::errc::io_error)
			{
				if (strstr(oErrorGetLastString(), "WSAEACCES"))
				{
					snprintf(_StrStatus, _SizeofStrStatus, "Permission denied when trying to create a new HTTP server. This can happen if the current machine is already running an HTTP server such as IIS or Apache.");
					return FAILURE;
				}
			}
		}

		if (!bDevelopmentMode)
		{
			oTESTB0(oHTTPClientCreate(clientDesc, &Client));

			oHTTP_RESPONSE response;

			// Test Head: Head should only return the header for the request.
			Client->Head("/" TEST_FILE, &response);
			oTESTB(response.StatusLine.StatusCode == oHTTP_OK, "Head query for <GoldenImages>/" TEST_FILE " did not return success");
			oTESTB(response.Content.Type == oMIME_IMAGE_PNG, "Head returned incorrect Type for file <GoldenImages>/" TEST_FILE);
			oTESTB(response.Content.Length == 173325, "Head returned incorrect size for image file <GoldenImages>/" TEST_FILE);

			// Create a new buffer based on the header from the head command
			int bufferSize = (int)response.Content.Length;
			char *pBuffer = new char[bufferSize];
			memset(pBuffer, NULL, bufferSize);
			ouro::intrusive_ptr<oBuffer> imageBuffer;
			oBufferCreate("test get buffer", pBuffer, bufferSize, oBuffer::Delete, &imageBuffer);

			// Test Get: Download and compare the image
			Client->Get("/" TEST_FILE, &response, imageBuffer->GetData(), (int)imageBuffer->GetSize());

			ouro::intrusive_ptr<oImage> image;
			oImageCreate("http test image", imageBuffer->GetData(), imageBuffer->GetSize(), &image);
			oTESTB(TestImage(image), "Image compare failed.");

			// Test POST: Sending an image file with POST.  Server will compare the image to the original and returned OK for success and 500 Internal Error for failure
			ouro::path defaultDataPath = ouro::filesystem::data_path();
			ouro::path_string golden;
			snprintf(golden, "%sGoldenImages/" TEST_FILE, defaultDataPath.c_str());
			if (oStreamReaderCreate(golden, &FileReader))
			{
				oSTREAM_DESC fileDesc;
				FileReader->GetDesc(&fileDesc);

				void *pImageFile = new char[(int)fileDesc.Size];
				memset(pImageFile, NULL, oSizeT(fileDesc.Size));
				ouro::intrusive_ptr<oBuffer> postImageBuffer;
				oBufferCreate("test post buffer", pImageFile, oSizeT(fileDesc.Size), oBuffer::Delete, &postImageBuffer);

				oSTREAM_READ r;
				r.pData = pImageFile;
				r.Range = fileDesc; 
				FileReader->Read(r);
				
				char PostBuffer[2048];
				Client->Post("/", oMIME_IMAGE_PNG, postImageBuffer->GetData(), (int)postImageBuffer->GetSize(), &response, &PostBuffer, 2048);
				oTESTB(response.StatusLine.StatusCode == oHTTP_OK, "Server image compare failed.");
			}
		}

		// In development mode run the server forever and server up responses to http requests
		while (bDevelopmentMode)
			oSleep(1000);

		return SUCCESS;
	}

	ouro::intrusive_ptr<threadsafe oStreamReader> FileReader;
};

struct PLATFORM_oHTTPLarge : public oTest
{
	uint TestBufferSize;
	std::vector<char> TestBuffer;

	void StartResponse(const oHTTP_REQUEST& _Request, const oNetHost& _Client, oHTTP_RESPONSE* _pResponse)
	{
		_pResponse->StatusLine.StatusCode = oHTTP_INTERNAL_SERVER_ERROR;
		if (_Request.RequestLine.Method == oHTTP_POST)
		{
			if(_Request.Content.Length != TestBufferSize)
				return;
			if(_Request.Content.Type != oMIME_APPLICATION_XDMP)
				return;

			bool bufferCorrect = true;
			char* httpBuffer = (char*)_Request.Content.pData;
			for (uint i = 0;i < TestBufferSize; ++i)
			{
				if(TestBuffer[i] != httpBuffer[i])
				{
					bufferCorrect = false;
				}
			}

			if(!bufferCorrect)
				_pResponse->StatusLine.StatusCode = oHTTP_INTERNAL_SERVER_ERROR;
			else
				_pResponse->StatusLine.StatusCode = oHTTP_OK;
		}
	}

	void FinishResponse(const void *_pMIMEData)
	{
		// if we were using dynamic memory, free it here
		delete[] _pMIMEData;
	}

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const bool bDevelopmentMode = false;

		oTestManager::DESC testDesc;
		oTestManager::Singleton()->GetDesc(&testDesc);
		if(testDesc.Exhaustive)
		{
			oTRACEA("This test does not play well with AVG. Be sure that is not running, or is temporarily disabled.");
			TestBufferSize = oGB(2)-oKB(1); //can't go over 2GB.
		}
		else
		{
			snprintf(_StrStatus, _SizeofStrStatus, "Large buffer test reduced in size. Use -x for exhaustive mode.");
			TestBufferSize = oMB(2);
		}

		ouro::intrusive_ptr<oHTTPServer> Server;
		oHTTPServer::DESC desc;
		desc.Port = 80;
		desc.StartResponse = oBIND(&PLATFORM_oHTTPLarge::StartResponse, this, oBIND1, oBIND2, oBIND3);
		desc.FinishResponse = oBIND(&PLATFORM_oHTTPLarge::FinishResponse, this, oBIND1);

		ouro::intrusive_ptr<oHTTPClient> Client;
		oHTTPClient::DESC clientDesc;
		ouro::from_string(&clientDesc.ServerAddr, "localhost:80");
		if (!oHTTPServerCreate(desc, &Server))
		{
			if (oErrorGetLast() == std::errc::io_error)
			{
				if (strstr(oErrorGetLastString(), "WSAEACCES"))
				{
					snprintf(_StrStatus, _SizeofStrStatus, "Permission denied when trying to create a new HTTP server. This can happen if the current machine is already running an HTTP server such as IIS or Apache.");
					return FAILURE;
				}
			}
		}

		if (!bDevelopmentMode)
		{
			oTESTB0(oHTTPClientCreate(clientDesc, &Client));

			oHTTP_RESPONSE response;

			TestBuffer.resize(TestBufferSize);
			std::generate(begin(TestBuffer), end(TestBuffer), []() { return rand();});

			// Test Head: Head should only return the header for the request.
			Client->Post("/test", oMIME_APPLICATION_XDMP, TestBuffer.data(), oInt(TestBuffer.size()), &response, nullptr, 0);
			oTESTB(response.StatusLine.StatusCode == oHTTP_OK, "Server did not accept the large buffer");
		}

		// In development mode run the server forever and server up responses to http requests
		while (bDevelopmentMode)
			oSleep(1000);

		return SUCCESS;
	}

	ouro::intrusive_ptr<threadsafe oStreamReader> FileReader;
};

oTEST_REGISTER(PLATFORM_oHTTP);
oTEST_REGISTER(PLATFORM_oHTTPLarge);
