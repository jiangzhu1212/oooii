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
#pragma once
#ifndef oOPENSSL_h
#define oOPENSSL_h

#include <oPlatform/oModuleUtil.h>

#include "oWinsock.h"
#undef interface
#undef INTERFACE_DEFINED
#include <openssl/ssl.h>

oDECLARE_DLL_SINGLETON_BEGIN(oOpenSSL)
	int (*SSL_library_init)(void);
	void (*SSL_load_error_strings)(void);
	SSL_CTX * (*SSL_CTX_new)(const SSL_METHOD *meth);
	SSL_METHOD * (*SSLv23_client_method)(void);
	SSL * (*SSL_new)(SSL_CTX *ctx);
	int (*SSL_set_fd)(SSL *s, int fd);
	long (*SSL_ctrl)(SSL *ssl,int cmd, long larg, void *parg);
	int (*SSL_shutdown)(SSL *s);
	void (*SSL_free)(SSL *ssl);
	void (*SSL_CTX_free)(SSL_CTX *);
	int (*SSL_write)(SSL *ssl,const void *buf,int num);
	int (*SSL_get_error)(const SSL *s,int ret_code);
	int (*SSL_connect)(SSL *ssl);
	int (*SSL_read)(SSL *ssl,void *buf,int num);
	int (*SSL_pending)(const SSL *s);
oDECLARE_DLL_SINGLETON_END()

#define interface struct __declspec(novtable)
interface oSocketEncryptor : oInterface
{
	static bool Create(oSocketEncryptor** _ppEncryptor);

	virtual bool OpenSSLConnection(SOCKET _hSocket, unsigned int _TimeoutMS) = 0;
	virtual void CleanupOpenSSL() = 0;
	virtual int Send(SOCKET _hSocket, const void *_pSource, unsigned int _SizeofSource, unsigned int _TimeoutMS) = 0;
	virtual int Receive(SOCKET _hSocket, char *_pData, unsigned int  _BufferSize, unsigned int _TimeoutMS) = 0;
};

#endif
