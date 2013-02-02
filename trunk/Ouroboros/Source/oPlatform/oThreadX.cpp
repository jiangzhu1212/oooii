/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oThreadX.h>
#include <oBasis/oError.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/Windows/oWindows.h>
#include "SoftLink/oWinPSAPI.h"

static bool FindOldestAndThusMainThread(unsigned int _ThreadID, unsigned int _ProcessID, ULONGLONG* _pMinCreateTime, unsigned int* _pOutMainThreadID)
{
	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, TRUE, _ThreadID);
	if (hThread)
	{
		FILETIME tCreate, tExit, tKernel, tUser;

		if (GetThreadTimes(hThread, &tCreate, &tExit, &tKernel, &tUser))
		{
			ULONGLONG createTime = ((ULONGLONG)tCreate.dwHighDateTime << 32) | tCreate.dwLowDateTime;
			if (createTime && createTime < *_pMinCreateTime)
			{
				*_pMinCreateTime = createTime;
				*_pOutMainThreadID = _ThreadID;
			}
		}

		oVB(CloseHandle(hThread));
	}

	return true;
}

oThread::id oThreadGetMainID()
{
	oThread::id mainThreadID;
	ULONGLONG minCreateTime = MAXULONGLONG;
	oEnumProcessThreads(::GetCurrentProcessId(), oBIND(FindOldestAndThusMainThread, oBIND1, oBIND2, &minCreateTime, (unsigned int*)&mainThreadID));
	return mainThreadID;
}

oThread::native_handle_type oThreadGetCurrentNativeHandle()
{
	return ::GetCurrentThread();
}

bool oThreadSetAffinity(oThread& _Thread, size_t _AffinityMask)
{
	DWORD_PTR PriorAffinity = SetThreadAffinityMask((HANDLE)_Thread.native_handle(), _AffinityMask);
	if (!PriorAffinity)
		return oWinSetLastError();
	return true;
}

bool oThreadSetPriority(threadsafe oThread& _Thread, oTHREAD_PRIORITY _Priority)
{
	int WindowsPriority = _Priority - 3;
	if (0 == SetThreadPriority((HANDLE)_Thread.native_handle(), WindowsPriority))
		return oWinSetLastError();
	return true;
}

oTHREAD_PRIORITY oThreadGetPriority(oThread& _Thread)
{
	int WindowsPriority = GetThreadPriority((HANDLE)_Thread.native_handle());
	if (THREAD_PRIORITY_ERROR_RETURN == WindowsPriority)
		return oTHREAD_PRIORITY_NOT_RUNNING;
	return oTHREAD_PRIORITY(WindowsPriority + 3);
}
