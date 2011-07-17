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
#ifndef oAsyncFileIO_h
#define oAsyncFileIO_h

#include <oooii/oStddef.h>

namespace oAsyncFileIO 
{
	enum COMMAND_TYPE
	{
		READ,
		WRITE,
		WRITE_APPEND
	};

	struct COMMAND
	{	
		// Input
		COMMAND_TYPE CommandType; 
		bool DataIsText; 
		const char* PathToFile;
		union
		{
			void* pDataOut;
			const void* pDataIn;
		};
		size_t SizeOfDataBlob;

		// Output
		bool Success;

		// Function to call on the IO thread once the file is done. 
		// Success will indicate what happened, a call to oGetLastError()
		// will return more info.
		oFUNCTION< void(oAsyncFileIO::COMMAND* _pCommand) > Continuation;
	};

	void ScheduleIO(COMMAND* _pCommand);
};

#endif
