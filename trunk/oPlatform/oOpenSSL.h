// $(header)
#pragma once
#ifndef oOPENSSL_h
#define oOPENSSL_h

#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <psapi.h>
#include <openssl/ssl.h>

struct oOpenSSL : oModuleSingleton<oOpenSSL>
{
	oOpenSSL();
	~oOpenSSL();

public:

	int				(*SSL_library_init)(void);
	void			(*SSL_load_error_strings)(void);
	SSL_CTX *		(*SSL_CTX_new)(const SSL_METHOD *meth);
	SSL_METHOD *	(*SSLv23_client_method)(void);
	SSL *			(*SSL_new)(SSL_CTX *ctx);
	int				(*SSL_set_fd)(SSL *s, int fd);
	long			(*SSL_ctrl)(SSL *ssl,int cmd, long larg, void *parg);
	int				(*SSL_shutdown)(SSL *s);
	void			(*SSL_free)(SSL *ssl);
	void			(*SSL_CTX_free)(SSL_CTX *);
	int 			(*SSL_write)(SSL *ssl,const void *buf,int num);
	int				(*SSL_get_error)(const SSL *s,int ret_code);
	int 			(*SSL_connect)(SSL *ssl);
	int 			(*SSL_read)(SSL *ssl,void *buf,int num);
	int				(*SSL_pending)(const SSL *s);

protected:
	oHMODULE hOpenSSL;
};

interface oSocketEncryptor : oInterface
{
	static bool Create(oSocketEncryptor** _ppEncryptor);

	virtual bool OpenSSLConnection(SOCKET _hSocket, unsigned int _TimeoutMS) = 0;
	virtual void CleanupOpenSSL() = 0;
	virtual int Send(SOCKET _hSocket, const void *_pSource, unsigned int _SizeofSource, unsigned int _TimeoutMS) = 0;
	virtual int Receive(SOCKET _hSocket, char *_pData, unsigned int  _BufferSize, unsigned int _TimeoutMS) = 0;
};


#endif
