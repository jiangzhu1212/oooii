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
// Functions to help in making sure files written are the same in debug and release builds. In particular unwritten parts of descriptions can have fixed values in debug, and random values in release.
#pragma once
#ifndef oSerialization_h
#define oSerialization_h

namespace oSerialization
{
	static const int MEMORY_INIT_VALUE = 0xBB; //so we have a different code other than the microsoft codes
		
	inline void oInitForSerialization(void *_arg, size_t _size)
	{
		memset(_arg, MEMORY_INIT_VALUE, _size);		
	}

	template<typename T>
	void oInitForSerialization(T &_arg)
	{
		oInitForSerialization((void*)(&_arg), sizeof(_arg));
		new(&_arg) T;
	}
	
	class DisableCRTMemoryInit //doesn't do anything in release builds. otherwise, it will keep secure crt functions from filling buffers with 0xFE while alive.
	{
	public:
		DisableCRTMemoryInit();
		~DisableCRTMemoryInit();
	};
}

#endif