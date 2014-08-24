/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oPlatform/oEMail.h>
#include <oBase/string.h>

using namespace ouro;

#define oSMTPCOMMAND(expr) do { if (!(expr)) { Disconnect(); return false; } } while(false)
#define SMTP_CONNECTION_OK 220
#define SMTP_OK 250
#define SMTP_ENCRYPTION_OK 334
#define SMTP_ACCEPTED 235
#define SMTP_GO_AHEAD 354

const char *BOUNDARY = "----=_Boundary_40364E2B-D16B-4BDD-9EDE-A839E5AFD136";

static char EncodingTable[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

void Base64Encode(const char *data, char *_Output, int _InputLength, int _OutputLength) 
{
	oASSERT((_OutputLength >=  4 * (_InputLength / 3.0f)), "Base64Encode: OutputLength must be 4/3rds the size of InputLength");
	oASSERT(_Output != NULL, "Base64Encode: Output must not be NULL");

	_OutputLength = (int)(4 * (_InputLength / 3.0f));
	int i = 0, j = 0;
	unsigned char charArray3[3], charArray4[4];
	int outputIndex = 0;
	while (_InputLength--)
	{
		charArray3[i++] = *(data++);
		if (i == 3) 
		{
			charArray4[0] = (charArray3[0] & 0xfc) >> 2;
			charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
			charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
			charArray4[3] = charArray3[2] & 0x3f;

			for(i = 0; (i <4) ; ++i)
			{
				_Output[outputIndex++] = EncodingTable[charArray4[i]];
			}
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; ++j)
			charArray3[j] = '\0';

		charArray4[0] = (charArray3[0] & 0xfc) >> 2;
		charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
		charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
		charArray4[3] = charArray3[2] & 0x3f;

		for (j = 0; (j < i + 1); ++j)
			_Output[outputIndex++] = EncodingTable[charArray4[j]]; 

		while((i++ < 3))
			_Output[outputIndex++] = '=';
	}

	_Output[outputIndex++] = 0;
}

class oEMail_Impl : public oEMail
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oEMail_Impl(Encryption _EncryptionType, oNetAddr _ServerAddress, bool* _pSuccess);
	bool Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, bool _bPasswordEncoded/*= false*/, bool _AsHTML/*= false*/) override;
	bool Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, const char *_pAttachmentName, const void *_pAttachmentData, size_t _SizeOfAttachmentData, bool _bPasswordEncoded/*= false*/, bool _AsHTML/*= false*/) override;
private:
	bool EstablishConnection(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, bool _bPasswordEncoded);
	bool SendMessage(const char *_pSubject, const char *_pMessage, bool _AsHTML);
	bool SendAttachment(const char *_pAttachmentName, const void *_pAttachment, size_t _SizeOfAttachment);
	bool SendSocket(const char *_pMessage);
	bool SendSocket(const void *_pData, size_t _SizeOfData);
	bool SendHeader(const char *_pSubject);
	bool SendEndData();
	bool SendSetDataMode();

	int ReceiveSocket(char *_pBuffer, int _BufferSize);
	void Disconnect();

	oRefCount Refcount;
	intrusive_ptr<threadsafe oSocketEncrypted> Socket;

	char ServerAddress[_MAX_PATH];
	Encryption EncryptionType;

	bool bUseEncryption;
};

oEMail_Impl::oEMail_Impl(Encryption _EncryptionType, oNetAddr _ServerAddress, bool* _pSuccess)
	: EncryptionType(_EncryptionType)
	, bUseEncryption(false)
{
	oSocket::DESC Desc;
	Desc.Addr = _ServerAddress;
	Desc.Protocol = oSocket::TCP;
	Desc.Style = oSocket::BLOCKING;

	if( !oSocketEncryptedCreate( "Server Connection", Desc, &Socket ) )
		*_pSuccess = false;
	else
		*_pSuccess = true;
}

int GrabResponseCode(const char *_Response)
{
	bool bSuccessful = false;
	char retCode[4];
	strlcpy(retCode, _Response, 3);
	return atoi(retCode);
}

bool oEMail_Impl::SendSocket(const char *_pMessage)
{
	return bUseEncryption ? Socket->SendEncrypted(_pMessage, (oSocket::size_t)strlen(_pMessage)) 
		: Socket->Send(_pMessage, (oSocket::size_t)strlen(_pMessage));
}

bool oEMail_Impl::SendSocket(const void *_pData, size_t _SizeOfData)
{
	return bUseEncryption ? Socket->SendEncrypted(_pData, (oSocket::size_t)_SizeOfData) : Socket->Send(_pData, (oSocket::size_t)_SizeOfData);
}

int oEMail_Impl::ReceiveSocket(char *_pBuffer, int _BufferSize)
{
	oSocket::size_t size = bUseEncryption ? Socket->RecvEncrypted(_pBuffer, _BufferSize) : Socket->Recv(_pBuffer, _BufferSize);
	if (size <= 0)
		return 0;

	return GrabResponseCode(_pBuffer);
}

void oEMail_Impl::Disconnect()
{
	// Say good-bye... if we have already timed out, don't bother the server is probably gone
	char buffer[_MAX_PATH];
	// Send Quit
	SendSocket("QUIT\r\n");
	// Receive bye
	ReceiveSocket(buffer, _MAX_PATH);
}

bool oEMail_Impl::EstablishConnection(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, bool _bPasswordEncoded)
{
	bUseEncryption = false;

	char hostname[_MAX_PATH];
	char ipAddress[_MAX_PATH];
	char port[_MAX_PATH];
	char sendBuffer[_MAX_PATH];
	char recvBuffer[_MAX_PATH];

	// Read initial data from connection
	oSMTPCOMMAND(SMTP_CONNECTION_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

	// Send EHLO in the clear
	Socket->GetHostname(hostname, _MAX_PATH, ipAddress, _MAX_PATH, port, _MAX_PATH);
	snprintf(sendBuffer, "EHLO %s\r\n", ipAddress);
	oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

	if (EncryptionType == USE_TLS)
	{
		// Respond with STARTTLS to indicate desire for TLS encrypted connection
		oSMTPCOMMAND(SendSocket("STARTTLS\r\n") && SMTP_CONNECTION_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

		// Next send will establish SSL encryption on this socket.
		bUseEncryption = true;

		// Send EHLO encrypted and decrypt the response
		oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

		// Request login
		snprintf(sendBuffer, "AUTH LOGIN\r\n");
		oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_ENCRYPTION_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

		// Send base64 encrypted username
		char encLogin[_MAX_PATH];
		Base64Encode(_pFromAddress, encLogin, (int)strlen(_pFromAddress), _MAX_PATH);
		snprintf(sendBuffer, "%s\r\n", encLogin);
		oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_ENCRYPTION_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

		// Send base64 encrypted password
		char encPass[_MAX_PATH];
		if (_bPasswordEncoded)
			strlcpy(encPass, _pPassword);
		else
			Base64Encode(_pPassword, encPass, (int)strlen(_pPassword), _MAX_PATH);
		snprintf(sendBuffer, "%s\r\n", encPass);
		oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_ACCEPTED == ReceiveSocket(recvBuffer, _MAX_PATH));
	}

	// Send from email address (this will actually be ignored by google, the account name will be used instead)
	snprintf(sendBuffer, "MAIL FROM:<%s>\r\n", _pFromAddress);
	oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

	// Send to address
	snprintf(sendBuffer, "RCPT TO:<%s>\r\n", _pToAddress);
	oSMTPCOMMAND(SendSocket(sendBuffer) && SMTP_OK == ReceiveSocket(recvBuffer, _MAX_PATH));

	return true;
}

bool oEMail_Impl::SendMessage(const char *_pSubject, const char *_pMessage, bool _AsHTML)
{
	char sendBuffer[_MAX_PATH];
	// Send Subject
	snprintf(sendBuffer, "Subject: %s\n", _pSubject); 
	oSMTPCOMMAND(SendSocket(sendBuffer));

		// MIME Version
	oSMTPCOMMAND(SendSocket("MIME-Version: 1.0\r\n"));
	oSMTPCOMMAND(SendSocket("Content-Type: multipart/mixed;\r\n"));
	snprintf(sendBuffer, "    boundary=\"%s\"\r\n", BOUNDARY);
	oSMTPCOMMAND(SendSocket(sendBuffer));
	
	oSMTPCOMMAND(SendSocket("\r\n"));
	oSMTPCOMMAND(SendSocket("\r\n"));
	snprintf(sendBuffer, "--%s\r\n", BOUNDARY);
	oSMTPCOMMAND(SendSocket(sendBuffer));

	if(_AsHTML)
		oSMTPCOMMAND(SendSocket("Content-Type: text/html; charset=US-ASCII\r\n"));
	else
		oSMTPCOMMAND(SendSocket("Content-Type: text/plain; charset=US-ASCII\r\n"));

	oSMTPCOMMAND(SendSocket("\r\n"));

	oSMTPCOMMAND(SendSocket(_pMessage));
	oSMTPCOMMAND(SendSocket("\r\n\r\n"));

	return true;
}

bool oEMail_Impl::SendSetDataMode()
{
	char recvBuffer[_MAX_PATH];
	// Set DATA mode
	if (!(SendSocket("DATA\r\n") && SMTP_GO_AHEAD == ReceiveSocket(recvBuffer, _MAX_PATH)))
		return false; // If something goes bad and we are in data mode, any commands will be ignored so just hang up.
	return true;
}

bool oEMail_Impl::SendAttachment(const char *_pAttachmentName, const void *_pAttachment, size_t _SizeOfAttachment)
{
	// once data mode has been started the server will not send back any responses until we send "\r\n.\r\n"
	char sendBuffer[_MAX_PATH];

	snprintf(sendBuffer, "--%s\r\n", BOUNDARY);
	oSMTPCOMMAND(SendSocket(sendBuffer));

	oSMTPCOMMAND(SendSocket("Content-Type: application/octet-stream\r\n"));
	oSMTPCOMMAND(SendSocket("Content-Transfer-Encoding: base64\r\n"));
	oSMTPCOMMAND(SendSocket("Content-Disposition: attachment;"));
	snprintf(sendBuffer, " filename=\"%s\"\r\n", _pAttachmentName);
	oSMTPCOMMAND(SendSocket(sendBuffer));
	oSMTPCOMMAND(SendSocket("\r\n"));
	
	// encode attachment to base64
	int size = (int)(4 * (_SizeOfAttachment / 3.f + 1));
	char *pBase64Attachment = new char[size];
	Base64Encode((const char*)_pAttachment, pBase64Attachment, static_cast<int>(_SizeOfAttachment), size);
	
	// now send attachment
	oSMTPCOMMAND(SendSocket( pBase64Attachment, size));
	oSMTPCOMMAND(SendSocket("\r\n"));
	oSMTPCOMMAND(SendSocket("\r\n"));

	delete[] pBase64Attachment;
	return true;
}

bool oEMail_Impl::SendEndData()
{
	char sendBuffer[_MAX_PATH];
	char recvBuffer[_MAX_PATH];

	snprintf(sendBuffer, "--%s--\r\n", BOUNDARY);
	oSMTPCOMMAND(SendSocket(sendBuffer));
	oSMTPCOMMAND(SendSocket("\r\n"));
	// Send Body, must end with \r\n.\r\n  the . indicates the end of data mode
	oSMTPCOMMAND(SendSocket("\r\n.\r\n") && SMTP_OK == ReceiveSocket(recvBuffer, _MAX_PATH));
	return true;
}

bool oEMail_Impl::Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, bool _bPasswordEncoded/*= false*/, bool _AsHTML/*= false*/)
{
	if (!EstablishConnection(_pToAddress, _pFromAddress, _pPassword, _bPasswordEncoded))
		return false;

	if (!SendSetDataMode())
		return false;

	if (!SendMessage(_pSubject, _pMessage, _AsHTML))
		return false;

	if (!SendEndData())
		return false;

	// We're done, say good-bye
	Disconnect();
	return true;
}

bool oEMail_Impl::Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, const char *_pAttachmentName, 
	const void *_pAttachmentData, size_t _SizeOfAttachmentData, bool _bPasswordEncoded/*= false*/, bool _AsHTML/*= false*/)
{
	if (!EstablishConnection(_pToAddress, _pFromAddress, _pPassword, _bPasswordEncoded))
		return false;

	if (!SendSetDataMode())
		return false;
	
	if (!SendMessage(_pSubject, _pMessage, _AsHTML))
		return false;

	if (!SendAttachment(_pAttachmentName, _pAttachmentData, _SizeOfAttachmentData))
		return false;

	if (!SendEndData())
		return false;
	// We're done, say good-bye
	Disconnect();
	return true;
}

oAPI bool oEMailCreate(oEMail::Encryption _EncryptionType, oNetAddr _ServerAddress, oEMail** _ppEMail) 
{
	bool success = false;
	oCONSTRUCT( _ppEMail, oEMail_Impl(_EncryptionType, _ServerAddress, &success) );
	return success;
}