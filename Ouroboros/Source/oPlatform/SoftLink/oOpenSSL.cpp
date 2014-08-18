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
#include <oBase/assert.h>
#include <oCore/process_heap.h>

using namespace ouro;

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

oOpenSSL::oOpenSSL()
{
	hModule = module::link("ssleay32.dll", sExportedAPIs, (void**)&SSL_library_init, oCOUNTOF(sExportedAPIs));
}

oOpenSSL::~oOpenSSL()
{
	module::close(hModule);
}

oDEFINE_PROCESS_SINGLETON("oOpenSSL", oOpenSSL);

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
	oOpenSSL::singleton().SSL_library_init();
	//oOpenSSL::singleton().SSL_load_error_strings();
	pCtx = oOpenSSL::singleton().SSL_CTX_new (oOpenSSL::singleton().SSLv23_client_method());
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
	
	pSSL = oOpenSSL::singleton().SSL_new (pCtx);   
	if(!pSSL)
		return false;

	oOpenSSL::singleton().SSL_set_fd (pSSL, (int)_hSocket);
    oOpenSSL::singleton().SSL_set_mode(pSSL, SSL_MODE_AUTO_RETRY);

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
			if((res = select((int)_hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
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
		res = oOpenSSL::singleton().SSL_connect(pSSL);
		switch(oOpenSSL::singleton().SSL_get_error(pSSL, res))
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
		oOpenSSL::singleton().SSL_shutdown (pSSL);  /* send SSL/TLS close_notify */
		oOpenSSL::singleton().SSL_free (pSSL);
		pSSL = NULL;
	}
	if(pCtx != NULL)
	{
		oOpenSSL::singleton().SSL_CTX_free (pCtx);	
		pCtx = NULL;
	}
}

int oSocketEncryptor_Impl::Send(SOCKET _hSocket, const void *_pSource, unsigned int _SizeofSource, unsigned int _TimeoutMS)
{
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

		if((res = select((int)_hSocket+1,&fdread,&fdwrite,NULL,&time)) == SOCKET_ERROR)
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

		if(FD_ISSET(_hSocket,&fdwrite) || (bWriteBlockedOnRead && FD_ISSET(_hSocket, &fdread)))
		{
			res = oOpenSSL::singleton().SSL_write(pSSL, _pSource, _SizeofSource);

			switch(oOpenSSL::singleton().SSL_get_error(pSSL,res))
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

		if((res = select((int)_hSocket+1, &fdread, &fdwrite, NULL, &time)) == SOCKET_ERROR)
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

		if(FD_ISSET(_hSocket,&fdread) || (bReadBlockedOnWrite && FD_ISSET(_hSocket,&fdwrite)))
		{
			while(1)
			{
				bReadBlockedOnWrite = false;

				const int buff_len = 1024;
				char buff[buff_len];

				res = oOpenSSL::singleton().SSL_read(pSSL, buff, buff_len);

				int ssl_err = oOpenSSL::singleton().SSL_get_error(pSSL, res);

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
					if(oOpenSSL::singleton().SSL_pending(pSSL))
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
