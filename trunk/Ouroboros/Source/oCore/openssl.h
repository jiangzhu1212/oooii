// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oCore_openssl_h
#define oCore_openssl_h

#include <oCore/module.h>

namespace ouro { namespace net { namespace openssl {

void ensure_initialized();

// returns an ssl handle that can be passed to close, send, receive
// This requires a valid blocking/synchronous socket.
void* open(void* socket);

// same as above with a timeout
void* open(void* socket, unsigned int timeout_ms);

// all opened handles should be closed when finished
void close(void* ssl);

// sends/receives an encrypted message
// all return how much was sent/received
size_t send(void* ssl, void* socket, const void* src, size_t src_size);
size_t send(void* ssl, void* socket, const void* src, size_t src_size, unsigned int timeout_ms);
size_t receive(void* ssl, void* socket, void* dst, size_t dst_size);
size_t receive(void* ssl, void* socket, void* dst, size_t dst_size, unsigned int timeout_ms);

}}}

#endif
