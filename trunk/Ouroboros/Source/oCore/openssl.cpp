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
#include <oCore/module.h>
#include <oCore/process_heap.h>
#include <openssl/ssl.h>

namespace ouro { namespace net { namespace openssl {

class context
{
public:
	static context& singleton();

	SSL* open(int socket_fd, unsigned int timeout_ms);
	void close(SSL* ssl);

	int send(SSL* ssl, int socket_fd, const void* src, size_t src_size, unsigned int timeout_ms);
	int receive(SSL* ssl, int socket_fd, void* dst, size_t dst_size, unsigned int timeout_ms);

private:
	module::id module_id;
	SSL_CTX* ctx;

	context();
	~context();

	void deinitialize();

	// context exports
	int (*SSL_library_init)(void);
	void (*SSL_load_error_strings)(void);
	SSL_CTX* (*SSL_CTX_new)(const SSL_METHOD *meth);
	SSL_METHOD* (*SSLv23_client_method)(void);
	SSL* (*SSL_new)(SSL_CTX *ctx);
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
};

oDEFINE_PROCESS_SINGLETON("ouro::net::context", context);

context::context()
	: ctx(nullptr)
{
	static const char* sExported[] = 
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

	try
	{
		module_id = module::link("ssleay32.dll", sExported, (void**)&SSL_library_init, oCOUNTOF(sExported));
		ctx = SSL_CTX_new(SSLv23_client_method());
	}

	catch (std::exception&)
	{
		auto e = std::current_exception();
		deinitialize();
		std::rethrow_exception(e);
	}
}

context::~context()
{
	deinitialize();
}

void context::deinitialize()
{
	if (ctx)
	{
		SSL_CTX_free(ctx);
		ctx = nullptr;
	}

	if (module_id)
	{
		module::close(module_id);
		module_id = module::id();
	}
}

SSL* context::open(int socket_fd, unsigned int timeout_ms)
{
	if (!ctx)
		return nullptr;
	
	SSL* ssl = SSL_new(ctx);   
	if (!ssl)
		return false;

	SSL_set_fd(ssl, socket_fd);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	int res = 0;
	fd_set fdwrite;
	fd_set fdread;
	bool wblocked = false;
	bool rblocked = false;

	timeval time;
	time.tv_sec = timeout_ms / 1000;
	time.tv_usec = 0;

	while (1)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		if (wblocked)
			FD_SET(socket_fd, &fdwrite);
		if (rblocked)
			FD_SET(socket_fd, &fdread);

		if (wblocked || rblocked)
		{
			wblocked = false;
			rblocked = false;
			if ((res = select(socket_fd+1, &fdread, &fdwrite, nullptr, &time)) == SOCKET_ERROR)
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return nullptr;
			}
			
			if (!res) // timeout
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return nullptr;
			}
		}

		res = SSL_connect(ssl);
		
		switch (SSL_get_error(ssl, res))
		{
			case SSL_ERROR_NONE:
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return ssl;
				break;     
			case SSL_ERROR_WANT_WRITE:
				wblocked = true;
				break;
			case SSL_ERROR_WANT_READ:
				rblocked = true;
				break;         
			default:	      
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				return nullptr;
		}
	}
}

void context::close(SSL* ssl)
{
	if (ssl)
	{
		SSL_shutdown(ssl);
		SSL_free(ssl);
	}
}

int context::send(SSL* ssl, int socket_fd, const void* src, size_t src_size, unsigned int timeout_ms)
{
	fd_set fdwrite;
	fd_set fdread;
	
	int res;
	int offset = 0;
	int remaining = (int)src_size;

	bool bWriteBlockedOnRead = false;

	timeval time;
	time.tv_sec = timeout_ms / 1000;
	time.tv_usec = 0;
	
	while (remaining > 0)
	{
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);
		FD_SET(socket_fd, &fdwrite);
		
		if (bWriteBlockedOnRead)
			FD_SET(socket_fd, &fdread);

		if ((res = select(socket_fd+1, &fdread, &fdwrite, nullptr, &time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return 0;
		}

		if (!res) // timeout
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return 0;
		}

		if (FD_ISSET(socket_fd, &fdwrite) || (bWriteBlockedOnRead && FD_ISSET(socket_fd, &fdread)))
		{
			oASSERT(size_t((int)src_size) == src_size, "src too big");

			res = SSL_write(ssl, src, (int)src_size);

			switch(SSL_get_error(ssl, res))
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

int context::receive(SSL* ssl, int socket_fd, void* dst, size_t dst_size, unsigned int timeout_ms)
{
	int res = 0, offset = 0;
	fd_set fdread;
	fd_set fdwrite;
	
	timeval time;
	time.tv_sec = timeout_ms / 1000;
	time.tv_usec = 0;

	bool bReadBlockedOnWrite = false;
	bool finished = false;

	while (!finished)
	{
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(socket_fd, &fdread);

		if (bReadBlockedOnWrite)
			FD_SET(socket_fd, &fdwrite);

		if ((res = select(socket_fd+1, &fdread, &fdwrite, nullptr, &time)) == SOCKET_ERROR)
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			return 0;
		}

		if (!res) // timeout
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			return 0;
		}

		if (FD_ISSET(socket_fd, &fdread) || (bReadBlockedOnWrite && FD_ISSET(socket_fd, &fdwrite)))
		{
			while(1)
			{
				bReadBlockedOnWrite = false;

				const int buf_size = 1024;
				char buf[buf_size];

				res = SSL_read(ssl, buf, buf_size);

				int ssl_err = SSL_get_error(ssl, res);
				if (ssl_err == SSL_ERROR_NONE)
				{
					if (offset + res > (int)dst_size - 1)
					{
						FD_ZERO(&fdread);
						FD_ZERO(&fdwrite);
						return 0;
					}

					memcpy((uint8_t*)dst + offset, buf, res);
					offset += res;
					if (SSL_pending(ssl))
						continue;
					else
					{
						finished = true;
						break;
					}
				}

				else if (ssl_err == SSL_ERROR_ZERO_RETURN)
				{
					finished = true;
					break;
				}

				else if (ssl_err == SSL_ERROR_WANT_READ)
					break;

				else if (ssl_err == SSL_ERROR_WANT_WRITE)
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
	((uint8_t*)dst)[offset] = 0;
	return offset;
}

void ensure_initialized()
{
	context::singleton();
}

void* open(void* socket, unsigned int timeout_ms)
{
	return context::singleton().open((int)socket, timeout_ms);
}

void* open(void* socket)
{
	return open(socket, (unsigned int)-1);
}

void close(void* ssl)
{
	context::singleton().close((SSL*)ssl);
}

size_t send(void* ssl, void* socket, const void* src, size_t src_size, unsigned int timeout_ms)
{
	return context::singleton().send((SSL*)ssl, (int)socket, src, src_size, timeout_ms);
}

size_t send(void* ssl, void* socket, const void* src, size_t src_size)
{
	return send(ssl, socket, src, src_size, (unsigned int)-1);
}

size_t receive(void* ssl, void* socket, void* dst, size_t dst_size, unsigned int timeout_ms)
{
	return context::singleton().receive((SSL*)ssl, (int)socket, dst, dst_size, timeout_ms);
}

size_t receive(void* ssl, void* socket, void* dst, size_t dst_size)
{
	return receive(ssl, socket, dst, dst_size, (unsigned int)-1);
}

}}}
