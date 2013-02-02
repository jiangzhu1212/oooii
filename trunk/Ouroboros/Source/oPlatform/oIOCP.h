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
#pragma once
#ifndef oIOCP_h
#define oIOCP_h

#include <oBasis/oStdChrono.h>
#include <oBasis/oInterface.h>
#include <oPlatform/Windows/oWindows.h>

typedef HANDLE oHandle;

struct oIOCPOp : public WSAOVERLAPPED
{
public:
	oIOCPOp()
	{
		Reset();
	}
	void Reset()
	{
		memset(this, 0, sizeof(WSAOVERLAPPED));
	}

	// Each oIOCP needs a certain amount of private data depending upon the operation type (socket/fileIO...)
	// this allows the IOCP access, it is always the exact size of PrivateDataSize on the oIOCP
	template<typename T>
	void GetPrivateData(T** _ppPrivateData) { *_ppPrivateData = reinterpret_cast<T*>(pPrivateData); }
	template<typename T>
	void ConstructPrivateData(T** _ppPrivateData) { GetPrivateData(_ppPrivateData); new(*_ppPrivateData)T(); }
	template<typename T>
	void DestructPrivateData() { ((T*)pPrivateData)->~T(); }
private:
	friend struct oIOCPContext;
	struct oIOCPContext* pContext;
	void* pPrivateData;
};

struct oIOCP : public oInterface
{
	typedef oFUNCTION<void(oIOCPOp* _pSocketOp)> io_completion_routine_t;
	typedef oTASK io_shutdown_t;

	struct DESC
	{
		DESC()
			: MaxOperations(128)
			, PrivateDataSize(0)
			, Handle(INVALID_HANDLE_VALUE) //if not changed, only DispatchIOTask will function since there will be no other way to register a completion
		{}
		unsigned int MaxOperations; // The maximum number of operations in flight
		size_t PrivateDataSize;  // The size of the private data on the oIOCP
		oHandle Handle; // Underling IO object (file/socket) this IOCP is using
		io_completion_routine_t IOCompletionRoutine; // Routine called every time an IOCP returns
	};

	// There are a limited number of ops available (MaxOperations)
	// so these should be acquired and returned as soon as possible
	virtual oIOCPOp* AcquireSocketOp() = 0;
	virtual void ReturnOp(oIOCPOp* _pIOCPOp) = 0;
	virtual void DispatchIOTask(oTASK&& _task) = 0;
	//Will send off the op to the iocp threads for processing. Useful for some windows functions
	//	that can operate either synchronously or asynchronously. Use this so you can only have one
	//	code path to respond to completed operations. If it completes synchronously just pass it here.
	virtual void DispatchManualCompletion(oHandle _handle, oIOCPOp* _pIOCPOp) = 0;
};

// Due to how windows manages IOCP the IOCP object must outlive the parent object.  To handle this
// the IOCP system has a lazy destruction system.  When using oIOCP the parent object should not
// do refcounting but instead rely on the IOCP system to tell it when to destroy itself.  This
// is handled via the ParentDestructionTask.  All refcouting should be routed to the IOCP object.
oAPI bool oIOCPCreate(const oIOCP::DESC& _Desc, oTASK _ParentDestructionTask, oIOCP** _ppIOCP);

//Useful to avoid over subscription issues. Can use to decide the best number of iocp operations to have in flight at once.
oAPI int oIOCPThreadCount();

#endif // oIOCP_h
