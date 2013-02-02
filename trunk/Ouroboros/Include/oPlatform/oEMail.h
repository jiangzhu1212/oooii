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
// Send/Receive email via SMTP and IMAP
#pragma once
#ifndef oEMail_h
#define oEMail_h

#include <oBasis/oInterface.h>
#include <oPlatform/oSocket.h>

interface oEMail : oInterface
{
	enum Encryption
	{
		USE_TLS = 0,
		Unencrypted,
	};
	virtual bool Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, bool _bPasswordEncoded = false, bool _AsHTML = false) = 0;
	virtual bool Send(const char *_pToAddress, const char *_pFromAddress, const char *_pPassword, const char *_pSubject, const char *_pMessage, const char *_pAttachmentName, const void *_pAttachmentData, size_t _SizeOfAttachmentData, bool _bPasswordEncoded = false, bool _AsHTML = false) = 0;
};

oAPI bool oEMailCreate(oEMail::Encryption _EncryptionType, oNetAddr _ServerAddress, oEMail** _ppEMail);
#endif