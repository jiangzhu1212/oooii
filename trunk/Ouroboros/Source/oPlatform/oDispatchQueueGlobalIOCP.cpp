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
#include "oDispatchQueueGlobalIOCP.h"
#include <oBasis/oDispatchQueueGlobalT.h>
#include "oIOCP.h"

struct oDispatchQueueGlobalIOCP_Impl
{
	oDispatchQueueGlobalIOCP_Impl()
		: IOCP(nullptr)
		, Valid(true)
	{ 
	}

	bool Init(oInterface* _pSelf)
	{
		oIOCP::DESC IOCPDesc;
		IOCPDesc.MaxOperations = 16;

		if(!oIOCPCreate(IOCPDesc, [&, _pSelf](){ Valid = false; _pSelf->Release(); }, &IOCP))
		{
			oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP.");
			return false;
		}
		return true;
	}

	int ReferenceImpl() threadsafe
	{ 
		return IOCP->Reference();
	}  

	bool ReleaseImpl() threadsafe
	{ 
		if(!Valid)
			return true;

		IOCP->Release();
		return false;
	} 

	void Issue(oTASK&& _Task) threadsafe
	{
		IOCP->DispatchIOTask(std::move(_Task));
	}

private:
	oIOCP* IOCP;
	bool Valid;
};

bool oDispatchQueueCreateGlobalIOCP(const char* _DebugName, size_t _InitialTaskCapacity, threadsafe oDispatchQueueGlobal** _ppDispatchQueue)
{
	bool success = false;
	oCONSTRUCT(_ppDispatchQueue, oDispatchQueueGlobalT<oDispatchQueueGlobalIOCP_Impl>(_DebugName, _InitialTaskCapacity, &success));
	return success;
}
