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
#include <oooii/oSingleton.h>
#include <oooii/oAllocator.h>
#include <oooii/oAssert.h>
#include <oooii/oAtomic.h>
#include <oooii/oStdAlloc.h>
#include <oooii/oString.h>
#include <oooii/oThread.h>
#include <oooii/oThreading.h>

enum SINGLETON_INIT_STATE
{
	UNINITIALIZED,
	DEINITIALIZED,
	DEINITIALIZING,
	INITIALIZING,
	INITIALIZED,
};

#ifdef _DEBUG
template<> static const char* oAsString(const SINGLETON_INIT_STATE& _State)
{
	switch (_State)
	{
		case UNINITIALIZED: return "UNINITIALIZED";
		case DEINITIALIZED: return "DEINITIALIZED";
		case DEINITIALIZING: return "DEINITIALIZING";
		case INITIALIZING: return "INITIALIZING";
		default: // special-case Any valid pointer is considered initialized
		case INITIALIZED: return "INITIALIZED";
	}
}
#endif

detail::oSingletonBase::oSingletonBase()
	: ModuleId(GetModuleHandle(0))
	, ppInstance(0)
{
}

detail::oSingletonBase::~oSingletonBase()
{
}

void detail::oSingletonBase::Reference() threadsafe
{
	RefCount.Reference();
}

void detail::oSingletonBase::Release() threadsafe
{
	if (RefCount.Release())
	{
		oASSERT((HMODULE)ModuleId == GetModuleHandle(0), "Singleton being freed by a module different than the one creating it.");
		oSWAP(ppInstance, (void*)DEINITIALIZED);
		this->~oSingletonBase();
		oHeap::StaticDeallocateShared((void*)this);
	}
}

void detail::oSingletonBase::ResolveStaticInstance(void** _ppInstance, oFUNCTION<void*()> _ConstructStaticInstance)
{
	// use the pointer as a flag to ensure no one else tries to double-init
	// the singleton
	if (UNINITIALIZED == oCAS((size_t*)_ppInstance, INITIALIZING, UNINITIALIZED))
	{
		void* p = _ConstructStaticInstance();
		static_cast<detail::oSingletonBase*>(p)->ppInstance = _ppInstance;
		oSWAP(_ppInstance, p); // OK we're done
	}

	else if ((size_t)*_ppInstance == DEINITIALIZED)
		assert(0 && "Accessing deinitialized Singleton() (static teardown ordering issue)");

	else
		oSPIN_UNTIL((size_t)*_ppInstance >= INITIALIZED);

	#ifdef _DEBUG
		size_t s = (size_t)*_ppInstance;
		oASSERT(s >= INITIALIZED, "Singleton failed to initialize (is %s)", oAsString((SINGLETON_INIT_STATE)s));
	#endif
}

const char* detail::oSingletonBase::GetUniqueName(const type_info& _TypeInfo)
{
	// it's ok to have some extra static copies of this, around, just so we can do 
	// a quick return of the string for temp use.
	static oTHREADLOCAL char name[64];
	sprintf_s(name, "OOOii.%u.%s", oThread::GetCurrentThreadID(), oGetSimpleTypename(_TypeInfo.name()));
	return name;
}
