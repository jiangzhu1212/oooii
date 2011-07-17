/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#ifndef oIOCP_h
#define oIOCP_h

#include <oooii/oInterface.h>

typedef HANDLE oHandle;

struct oIOCP : public oInterface
{
	typedef oFUNCTION<void(oHandle& _Handle, void* _pOverlapped)> callback_t;

	virtual bool RegisterHandle(oHandle& _Handle, callback_t _Callback) = 0;

	// @oooii-mike: There doesn't seem to be a way to disassociate a HANDLE from
	// an IOCP. This shouldn't ever be a problem, however, as UnregisterHandle
	// should only be called when a HANDLE is being destroyed.
	virtual bool UnregisterHandle(oHandle& _Handle) = 0;

	static bool Create(const char* _DebugName, oIOCP** _ppIOCP);
};

#endif // oIOCP_h
