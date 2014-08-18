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
