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
#include "oOpenSSL.h"
#include <oStd/assert.h>
#include "oWinsock.h"

static const char* sExportedAPIs[] = 
{
	"SSL_library_init",
	"SSL_load_error_strings",
	"SSL_CTX_new",
	"SSLv23_client_method",
	"SSL_new",
	"SSL_set_fd",
	"SSL_ctrl",
	"SSL_shutdown",
	"SSL_free",
	"SSL_CTX_free",
	"SSL_write",
	"SSL_get_error",
	"SSL_connect",
	"SSL_read",
	"SSL_pending",
};

oDEFINE_DLL_SINGLETON_CTOR(oOpenSSL, "ssleay32.dll", SSL_library_init)

// {3F8F7F52-E9F9-4704-BE7D-6E7F678E12C0}
const oGUID oOpenSSL::GUID = { 0x3f8f7f52, 0xe9f9, 0x4704, { 0xbe, 0x7d, 0x6e, 0x7f, 0x67, 0x8e, 0x12, 0xc0 } };

class oSocketEncryptor_Impl : public oSocketEncryptor
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oSocketEncryptor_Impl(bool *_pSuccess);
	~oSocketEncryptor_Impl();
	
	bool OpenSSLConnection(SOCKET _hSocket, unsigned int _TimeoutMS) override;
	void CleanupOpenSSL() override;
	int Send(SOCKET _hSocket, const void *_pSource, unsigned int _SizeofSource, unsigned int _TimeoutMS) override;
	int Receive(SOCKET _hSocket, char *_pData, unsigned int _BufferSize, unsigned int _TimeoutMS) override;
private:
	SSL_CTX *pCtx;
	SSL *pSSL;

	oRefCount Refcount;
};

bool oSocketEncryptor::Create(oSocketEncryptor** _ppEncryptor)
{
	bool success = false;
	oCONSTRUCT(_ppEncryptor, oSocketEncryptor_Impl(&success));
	return success;
}
oSocketEncryptor_Impl::oSocketEncryptor_Impl(bool *_pSuccess)
{
	pCtx = NULL;
	oOpenSSL::Singleton()->SSL_library_init();
	//oOpenSSL::Singleton()->SSL_load_error_strings();
	pCtx = oOpenSSL::Singleton()->SSL_CTX_new (oOpenSSL::Singleton()->SSLv23_client_method());
	*_pSuccess = (pCtx != NULL);
}

oSocketEncryptor_Impl::~oSocketEncryptor_Impl()
{
	CleanupOpenSSL();
}

bool oSocketEncryptor_Impl::OpenSSLConnection(SOCKET _hSocket, unsigned int _TimeoutMS)
{
	if (!pCtx)
		return false;
	
	pSSL = oOpenSSL::Singleton()->SSL_new (pCtx);   
	if(!pSSL)
		return false;

	oOpenSSL::Singleton()->SSL_set_fd (pSSL, (int)_hSocket);
    oOpenSSL::Singleton()->SSL_set_mode(pSSL, SSL_MODE_AUTO_RETRY);

	int res = 0;
	fd_set fdwrite;
	fd_set fdread;
	bool bWriteBlocked = false;
	bool bReadBlocked = false;

	timeval time;
	time.tv_sec = _TimeoutMS / 1000;
	time.tv_usec = 0;

	while(1)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		if(bWriteBlocked)
			FD_SET(_hSocket, &fdwrite);
		if(bReadBlocked)
			FD_SET(_hSocket, &fdread);

		if(bWriteBlocked || bReadBlocked)
		{
			bWriteBlocked = false;
			bReadBlocked = false;
			if((res = oWinsock::Singleton()->select((int)_hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return false;
			}
			if(!res)
			{
				//timeout
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return false;
			}
		}
		res = oOpenSSL::Singleton()->SSL_connect(pSSL);
		switch(oOpenSSL::Singleton()->SSL_get_error(pSSL, res))
		{
		  case SSL_ERROR_NONE:
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return true;
			break;     
		  case SSL_ERROR_WANT_WRITE:
			bWriteBlocked = true;
			break;
		  case SSL_ERROR_WANT_READ:
			bReadBlocked = true;
			break;         
		  default:	      
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return false;
		}
	}
}

void oSocketEncryptor_Impl::CleanupOpenSSL()
{
	if(pSSL != NULL)
	{
		oOpenSSL::Singleton()->SSL_shutdown (pSSL);  /* send SSL/TLS close_notify */
		oOpenSSL::Singleton()->SSL_free (pSSL);
		pSSL = NULL;
	}
	if(pCtx != NULL)
	{
		oOpenSSL::Singleton()->SSL_CTX_free (pCtx);	
		pCtx = NULL;
	}
}

int oSocketEncryptor_Impl::Send(SOCKET _hSocket, const void *_pSource, unsigned int _SizeofSource, unsigned int _TimeoutMS)
{
	oWinsock* ws = oWinsock::Singleton();

	fd_set fdwrite;
	fd_set fdread;
	
	int res;
	int offset = 0;
	int remaining = _SizeofSource;

	bool bWriteBlockedOnRead = false;

	timeval time;
	time.tv_sec = _TimeoutMS / 1000;
	time.tv_usec = 0;
	
	while (remaining > 0)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		FD_SET(_hSocket, &fdwrite);

		
		if(bWriteBlockedOnRead)
		{
			FD_SET(_hSocket, &fdread);
		}

		if((res = ws->select((int)_hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return 0;
		}

		if(!res)
		{
			//timeout
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return 0;
		}

		if(ws->FD_ISSET(_hSocket,&fdwrite) || (bWriteBlockedOnRead && ws->FD_ISSET(_hSocket, &fdread)))
		{
			res = oOpenSSL::Singleton()->SSL_write(pSSL, _pSource, _SizeofSource);

			switch(oOpenSSL::Singleton()->SSL_get_error(pSSL,res))
			{
			  case SSL_ERROR_NONE:
				remaining -= res;
				offset += res;
				break;
			  case SSL_ERROR_WANT_WRITE:
				break;
			  case SSL_ERROR_WANT_READ:
				bWriteBlockedOnRead = true;
				break;
			  default:	      
				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				return 0;
			}
		}
	}

	FD_ZERO(&fdwrite);
	FD_ZERO(&fdread);

	return offset;
}

int oSocketEncryptor_Impl::Receive(SOCKET _hSocket, char *_pData, unsigned int _BufferSize, unsigned int _TimeoutMS)
{
	oWinsock *ws = oWinsock::Singleton();
	int res = 0, offset = 0;
	fd_set fdread;
	fd_set fdwrite;
	
	timeval time;
	time.tv_sec = _TimeoutMS / 1000;
	time.tv_usec = 0;

	bool bReadBlockedOnWrite = false;
	bool bFinish = false;

	while(!bFinish)
	{
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(_hSocket,&fdread);

		if(bReadBlockedOnWrite)
			FD_SET(_hSocket, &fdwrite);

		if((res = ws->select((int)_hSocket+1, &fdread, &fdwrite, NULL, &time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			return 0;
		}

		if(!res)
		{
			//timeout
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			return 0;
		}

		if(ws->FD_ISSET(_hSocket,&fdread) || (bReadBlockedOnWrite && ws->FD_ISSET(_hSocket,&fdwrite)))
		{
			while(1)
			{
				bReadBlockedOnWrite = false;

				const int buff_len = 1024;
				char buff[buff_len];

				res = oOpenSSL::Singleton()->SSL_read(pSSL, buff, buff_len);

				int ssl_err = oOpenSSL::Singleton()->SSL_get_error(pSSL, res);

				if(ssl_err == SSL_ERROR_NONE)
				{
					if(offset + res > (int)_BufferSize - 1)
					{
						FD_ZERO(&fdread);
						FD_ZERO(&fdwrite);
						return 0;
					}
					memcpy(_pData + offset, buff, res);
					offset += res;
					if(oOpenSSL::Singleton()->SSL_pending(pSSL))
					{
						continue;
					}
					else
					{
						bFinish = true;
						break;
					}
				}
				else if(ssl_err == SSL_ERROR_ZERO_RETURN)
				{
					bFinish = true;
					break;
				}
				else if(ssl_err == SSL_ERROR_WANT_READ)
				{
					break;
				}
				else if(ssl_err == SSL_ERROR_WANT_WRITE)
				{
					bReadBlockedOnWrite = true;
					break;
				}
				else
				{
					FD_ZERO(&fdread);
					FD_ZERO(&fdwrite);
					return 0;
				}
			}
		}
	}
	FD_ZERO(&fdread);
	FD_ZERO(&fdwrite);
	_pData[offset] = 0;

	return offset;
}