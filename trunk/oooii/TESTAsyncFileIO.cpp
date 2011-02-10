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
#include <oooii/oEvent.h>
#include <oooii/oThreading.h>

void TESTAsyncFileIO_Continuation(oAsyncFileIO::RESULT* _pResult, void* _pUserData)
{
	*(int*)_pUserData = 0x13371337;
}

struct TESTAsyncFileIO : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oAsyncFileIO::InitializeIOThread(20);

		char testFilePath[_MAX_PATH];
		oTESTB(oTestManager::Singleton()->FindFullPath(testFilePath, "oooii.ico"), "Failed to find %s", "oooii.ico");

		threadsafe oEvent ReadComplete;
		int continuationResult = 0;

		oAsyncFileIO::DESC desc;
		desc.Path = testFilePath;
		desc.Allocate = malloc;
		desc.pCompletionEvent = &ReadComplete;
		desc.Continuation = TESTAsyncFileIO_Continuation;
		desc.pUserData = &continuationResult;

		oAsyncFileIO::RESULT result;
		memset(&result, 0, sizeof(result));

		oAsyncFileIO::ScheduleFileRead(&desc, &result);

		oTESTB(ReadComplete.Wait(5000), "ASyncFileIO read timed out");
		oTESTB(continuationResult == 0x13371337, "Continuation not called");
		oTESTB(result.Result == 0, "Errors loading file %s", testFilePath);
		oTESTB(result.Size == 12221, "Invalid file size");
		free(result.pData);

		oAsyncFileIO::DeinitializeIOThread();

		return SUCCESS;
	}
};

TESTAsyncFileIO TestAsyncFileIO;
