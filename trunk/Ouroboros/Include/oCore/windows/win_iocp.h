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
#ifndef oCore_win_iocp_h
#define oCore_win_iocp_h

#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {
	namespace windows {
		namespace iocp {

unsigned int concurrency();

void ensure_initialized();

// Retrieve an OVERLAPPED structure configured for an async call using IO 
// completion ports (IOCP). This should be used for all async operations on the
// handle until the handle is closed. The specified completion function and 
// anything it references will live as long as the association does. One the 
// file is closed call disassociate to free the OVERLAPPED object.
OVERLAPPED* associate(HANDLE _Handle, const std::function<void(size_t _NumBytes)>& _OnCompletion);
void disassociate(OVERLAPPED* _pOverlapped);

// Waits until all associated IO operations have completed
void wait();
bool wait_for(unsigned int _TimeoutMS);

// returns if IO threads are running
bool joinable();

// waits for all associated IO operations to complete then joins all IO threads.
void join();

		} // namespace iocp
	} // namespace windows
} // namespace ouro

#endif
