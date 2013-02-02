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
#include "oHTTPInternal.h"
#include "oHTTPProtocol.h"
#include <oPlatform/oSystem.h>

const char *pDefaultPage = "<html><head><title>%u %s</title></head><body><p>%u %s</p></body></html>";

bool oHTTPProtocol::ProcessSocketReceive(void* _pData, unsigned int _SizeData, interface oSocket* _pSocket)
{
	// Process all bytes until done
	size_t SzDataTaken = 0;
	while (true)
	{
#ifdef SUSPECTED_BUGS_IN_HTTP
		HeaderGuard.Check();
		BodyGuard.Check();
#endif
		switch (State)
		{
		case oSTATE_WAIT_FOR_REQUEST:
			TheHeaderPos = 0;

			// Reset response
			TheResponse.Reset();
			TheResponse.StatusLine.Version = oHTTP_1_1;
			bCallFinishResponse = false;

			State = oSTATE_RECEIVE_HEADER;
			break;

		case oSTATE_RECEIVE_HEADER:
			// If we got called with 0 bytes, it could mean the connection is broken/closed/tearing down, check for that
			if (!_SizeData && !_pSocket->IsConnected())
			{
				bPrepareToCloseSocket = true;
				State = oSTATE_WAIT_FOR_CLOSE;
				break;
			}
			// Try to (continue to) extract an HTTP header
			if (!oExtractHTTPHeader(oByteAdd((const char*)_pData, SzDataTaken), (_SizeData - SzDataTaken), TheHeader.c_str(), &TheHeaderPos, TheHeader.capacity(), &SzDataTaken))
			{
				// If the HTTP header is bigger than our capacity, then send an error back
				if (TheHeaderPos >= TheHeader.capacity())
				{
					TheResponse.StatusLine.StatusCode = oHTTP_REQUEST_ENTITY_TOO_LARGE;
					bPrepareToCloseSocket = true;
					State = oSTATE_PROCESS_RESPONSE;
					break;
				}
				// oExtractHTTPHeader doesn't take any bytes if it can't extract a HTTP header (0 terminator before header end marker for example)
				if (_SizeData && !SzDataTaken)
				{
					TheResponse.StatusLine.StatusCode = oHTTP_BAD_REQUEST;
					bPrepareToCloseSocket = true;
					State = oSTATE_PROCESS_RESPONSE;
					break;
				}

				// Otherwise request more data
				oASSERT(SzDataTaken==_SizeData, "Assumimg that we took all data, before requesting more");
				return true;
			}
			State = oSTATE_PARSE_HEADER;
			break;

		case oSTATE_PARSE_HEADER:
			// We received the header in full, now parse it
			TheRequest.Reset();
			if (!oFromString(&TheRequest, TheHeader.c_str()))
			{
				TheResponse.StatusLine.StatusCode = oHTTP_BAD_REQUEST;
				bPrepareToCloseSocket = true;
				State = oSTATE_PROCESS_RESPONSE;
				break;
			}
			State = oSTATE_PROCESS_HEADER;
			break;

		case oSTATE_PROCESS_HEADER:
			{
				const char* pValue;
				unsigned int ValueUInt;
				// Get Host header field (mandatory in HTTP/1.1)
				if (!oHTTPFindHeader(TheRequest.HeaderFields, oHTTP_HEADER_HOST, &pValue))
				{
					TheResponse.StatusLine.StatusCode = oHTTP_BAD_REQUEST;
					bPrepareToCloseSocket = true;
					State = oSTATE_PROCESS_RESPONSE;
					break;
				}
				//oTrim(HttpRequest.Host.c_str(), _MAX_PATH, pValue); // TODO Should we parse Host: or leave it to the user code

				// Process optional headers
				
				// Connection
				if (oHTTPFindHeader(TheRequest.HeaderFields, oHTTP_HEADER_CONNECTION, &pValue))
				{
					char* ctx;
					char* pElement = oStrTok(pValue, ",", &ctx);
					while (pElement)
					{
						// Optimization so we can compare with lower case string matching, the actual header stays unchanged though
						oToLower(pElement);
						
						// keep-alive / close
						if (strstr(pElement, "close"))
							bPrepareToCloseSocket = true;

						pElement = oStrTok(nullptr, ",", &ctx);
					}
				}

				// Content-Type
				if (oHTTPFindHeader(TheRequest.HeaderFields, oHTTP_HEADER_CONTENT_TYPE, &pValue))
					oFromString(&TheRequest.Content.Type, pValue);

				// Content-Length
				if (oHTTPFindHeader(TheRequest.HeaderFields, oHTTP_HEADER_CONTENT_LENGTH, &ValueUInt))
					TheRequest.Content.Length = ValueUInt;

				// Allocate memory for receiving a content body if needed and go to the next state
				oASSERT(TheBody==nullptr, "There really shouldn't be a body allocated at this time");
				if (TheRequest.Content.Length)
				{
					// Alloc Body
					TheBody = new char[TheRequest.Content.Length];
					TheBodyPos = 0;

					State = oSTATE_RECEIVE_CONTENT_BODY;
				}
				else
				{
					State = oSTATE_START_RESPONSE;
				}
			}
			break;

		case oSTATE_RECEIVE_CONTENT_BODY:
			// If we got called with 0 bytes, it could mean the connection is broken/closed/tearing down, check for that
			if (!_SizeData && !_pSocket->IsConnected())
			{
				// Free Body (if any)
				if (TheBody)
				{
					delete[] TheBody;
					TheBody = nullptr;
				}
				bPrepareToCloseSocket = true;
				State = oSTATE_WAIT_FOR_CLOSE;
				break;
			}
			if (!oExtractContent(oByteAdd(_pData, SzDataTaken), (_SizeData - SzDataTaken), TheBody, &TheBodyPos, TheRequest.Content.Length, &SzDataTaken))
			{
				// Request more data
				oASSERT(SzDataTaken==_SizeData, "Assuming that we took all data, before requesting more");
				return true;
			}
			TheRequest.Content.pData = TheBody;
			State = oSTATE_START_RESPONSE;
			break;

		case oSTATE_START_RESPONSE:
			// Set a default response status code
			TheResponse.StatusLine.StatusCode = oHTTP_NOT_IMPLEMENTED;

			// Request fully received, now dispatch to the callback, if it supports the HTTP method
			if ((Desc.SupportedMethods & TheRequest.RequestLine.Method) != 0)
			{
				Desc.StartResponse(TheRequest, &TheResponse);
				bCallFinishResponse = (TheResponse.Content.pData != nullptr);
			}
			
			// Free Body (if any)
			if (TheBody)
			{
				delete[] TheBody;
				TheBody = nullptr;
			}

			// Handle the method if the Callback doesn't support it
			if ((Desc.SupportedMethods & TheRequest.RequestLine.Method) == 0)
			{
				oASSERT(oHTTP_NOT_IMPLEMENTED==TheResponse.StatusLine.StatusCode, "Default status code was still expected to be oHTTP_NOT_IMPLEMENTED");
				State = oSTATE_PROCESS_RESPONSE;
				break;
			}

			State = oSTATE_PROCESS_RESPONSE;
			break;

		case oSTATE_PROCESS_RESPONSE:
			{
				// Add Date header field
				oDATE date;
				oSystemGetDate(&date);
				oStringS dateString;
				oDateStrftime(dateString, oDATE_HTTP_FORMAT, date);
				oHTTPAddHeader(TheResponse.HeaderFields, oHTTP_HEADER_DATE, dateString);

				// Add Content header fields

				// If the StartResponse callback says OK, or filled out a body, we'll go with that. Otherwise we'll send a standard error body.
				if (TheResponse.StatusLine.StatusCode == oHTTP_OK || (TheResponse.Content.pData && TheResponse.Content.Length != 0))
				{
					if (TheResponse.Content.Type != oMIME_UNKNOWN)
						oHTTPAddHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONTENT_TYPE, oAsString(TheResponse.Content.Type));
				}
				else
				{
					// If we are going to close the connection, let the client know
					if (bPrepareToCloseSocket)
						oHTTPAddHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONNECTION, "close");

					// Generate a default response (HTML) with the response code and response code as string in the title/body of the HTML
					oHTTPAddHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONTENT_TYPE, oAsString(oMIME_TEXT_HTML));

					// TODO: Possible race condition on DefaultResponseBody? Could a previous send of DefaultResponseBody still be in progress at this time?
					oPrintf(DefaultResponseBody, pDefaultPage, TheResponse.StatusLine.StatusCode, oAsString(TheResponse.StatusLine.StatusCode), TheResponse.StatusLine.StatusCode, oAsString(TheResponse.StatusLine.StatusCode));
					TheResponse.Content.Length = oUInt(DefaultResponseBody.size());

					// If MIMEData was set by StartResponse, finish it early because something went wrong (Response.Size==0)
					if (bCallFinishResponse)
					{
						Desc.FinishResponse(TheResponse.Content.pData);
						TheResponse.Content.pData = nullptr;
						bCallFinishResponse = false;
					}
					
					// We're sending MIMEData that's from a static buffer, so doesn't need to be freed after send
					oASSERT(!TheResponse.Content.pData && !bCallFinishResponse, "Assuming we can specify the Content Body");
					TheResponse.Content.pData = DefaultResponseBody.c_str();
				}

				// TODO: While it's ok to set a content length header field of 0, for multipart/byteranges it may not be allowed or desired to set this header field..
				oHTTPAddHeader(TheResponse.HeaderFields, oHTTP_HEADER_CONTENT_LENGTH, oUInt(TheResponse.Content.Length));

				// With the HEAD method we don't actually send the body
				if (TheRequest.RequestLine.Method == oHTTP_HEAD)
				{
					// The StartResponse handler shouldn't have given it's data for a HEAD request, but let's deal with it if it did
					if (bCallFinishResponse)
					{
						Desc.FinishResponse(TheResponse.Content.pData);
						TheResponse.Content.pData = nullptr;
						bCallFinishResponse = false;
					}

					// There could be a default response here if there's an error.
					oASSERT(TheResponse.StatusLine.StatusCode != oHTTP_OK || !TheResponse.Content.pData, "HEAD requests should not have a Content Body");
				}

				// Set ReasonPhrase
				TheResponse.StatusLine.ReasonPhrase = oAsString(TheResponse.StatusLine.StatusCode);

				// oHTTPResponse -> String
				// We can re-use the header buffer that was used for parsing the initial request
				oToString(TheHeader, TheResponse);
			}

			State = oSTATE_SEND_RESPONSE;
			break;

		case oSTATE_SEND_RESPONSE:
			{
				// Allocate a buffer to which we copy all data to be sent, so we can add a send context at the end
				// TODO: Come up with a more efficient way (without copying) / change Socket callback to pass both header and body pointers/sizes
				size_t SendBufferTransferSize = TheHeader.size();
				
				size_t SendBufferSize = SendBufferTransferSize;
				char* SendBuffer = new char[SendBufferSize];

				size_t SizeAdded = 0;
				oInsertContent(TheHeader.c_str(), TheHeader.size(), SendBuffer, &SizeAdded, SendBufferSize, nullptr);
				oASSERT(SizeAdded==SendBufferSize, "Expected SendBuffer to be filled exactly");

				if (_pSocket->Send(SendBuffer, oUInt(SendBufferTransferSize), TheResponse.Content.pData, TheResponse.Content.pData ? oUInt(TheResponse.Content.Length) : 0))
				{
					++SendsInProgress;
				}
				else
				{
					if (bCallFinishResponse)
					{
						Desc.FinishResponse(TheResponse.Content.pData);
						TheResponse.Content.pData = nullptr;
						bCallFinishResponse = false;
					}
					delete[] SendBuffer;
					bPrepareToCloseSocket = true;
				}

				if (bPrepareToCloseSocket)
					State = oSTATE_WAIT_FOR_CLOSE;
				else
					State = oSTATE_WAIT_FOR_REQUEST;
			}
			break;

		case oSTATE_WAIT_FOR_CLOSE:
			oASSERT(bPrepareToCloseSocket, "Expected to get to this state because we're going to close");
			if (SendsInProgress == 0)
				return false;

			// FIXME: Actually we don't want to request more data, we're just waiting on sends for graceful disconnect
			return true;
		}
	}
}

bool oHTTPProtocol::ProcessSocketSend(void* _pHeader, void* _pBody, unsigned int _SizeData, interface oSocket* _pSocket)
{
#ifdef SUSPECTED_BUGS_IN_HTTP
	BodyGuard.Check();
#endif
	// Call finish response if we have a body and it's not our own
	if (_pBody && _pBody != DefaultResponseBody.c_str())
		Desc.FinishResponse(_pBody);
		
	delete[] _pHeader;

	if (--SendsInProgress == 0 && bPrepareToCloseSocket)
		return false;

	return true;
}

