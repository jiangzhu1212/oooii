// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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