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
#include "pch.h"
#include <oooii/oTest.h>
#include <oooii/oThreading.h>
#include "oWinsock.h"

struct TESTSingletonInteraction : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		#if defined(_WIN32) || defined(_WIN64)
			// @oooii-kevin: This is to verify singletons aren't leaking 
			// obug-843, if you disable the AsyncFileIO calls no leak occurs
			// or if you disable the winsock call, no leak occurs.  It's some
			// kind of interaction between the two singletons
			oAsyncFileIO::InitializeIOThread(20);
			oWinsock::Singleton()->WSAGetLastError();
			oAsyncFileIO::DeinitializeIOThread();
			return SUCCESS;
		#else
			sprintf_s(_StrStatus, _SizeofStrStatus, "This test is Windows-specific.");
			return SKIPPED;
		#endif
	}

};

TESTSingletonInteraction TestSingletonInteraction;