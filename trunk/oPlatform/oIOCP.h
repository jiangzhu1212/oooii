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

#include <oBasis/oStdChrono.h>
#include <oBasis/oInterface.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oEvent.h>

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
		{}
		unsigned int MaxOperations; // The maximum number of operations in flight
		size_t PrivateDataSize;  // The size of the private data on the oIOCP
		oHandle Handle; // Underling IO object (file/socket) this IOCP is using
		io_completion_routine_t IOCompletionRoutine; // Routine called every time an IOCP returns
	};

	// There are a limited number of ops available (MaxOperations)
	// so these should be aquired and returned as soon as possible
	virtual oIOCPOp* AcquireSocketOp() = 0;
	virtual void ReturnOp(oIOCPOp* _pIOCPOp) = 0;
};

// Waits for the event in an active state, meaning the thread will work on any 
// IOCP tasks that may queue up.  This is useful when IOCP work spawns additional
// IOCP work that is needed immediately.
oAPI void oIOCPActiveWait(oEvent* _pEvent, unsigned int _TimeoutMS = oINFINITE_WAIT);

// The Parent IO object (socket, fileIO...) which is creating the IOCP must provide
// a valid pointer to its interface as the IOCP must manage its lifetime.  Due to how
// windows manages IOCP the IOCP object must outlive the parent object.  In addition
// the parent must use oDEFINE_REFCOUNT_IOCP_INTERFACE to get proper refcounting. The
// parent object must only hold a raw pointer to the IOCP (no reference)
oAPI bool oIOCPCreate(const oIOCP::DESC& _Desc, threadsafe oInterface* _pParent, oIOCP** _ppIOCP);

#define oDEFINE_REFCOUNT_IOCP_INTERFACE(Refcount, pIOCP) int Reference() threadsafe override { if( pIOCP ) pIOCP->Reference(); return (Refcount).Reference() - 1;}  void Release() threadsafe override { if ((Refcount).Release()) delete this; else if(pIOCP) pIOCP->Release(); } 

#endif // oIOCP_h
