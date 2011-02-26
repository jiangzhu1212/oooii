// $(header)
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
#include <oooii/oProcess.h>
#include <oooii/oSTL.h>

enum SINGLETON_INIT_STATE
{
	UNINITIALIZED,
	DEINITIALIZED,
	DEINITIALIZING,
	DEINITIALIZING_MANAGER,
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
		case DEINITIALIZING_MANAGER: return "DEINITIALIZING_MANAGER";
		case INITIALIZING: return "INITIALIZING";
		default: // special-case Any valid pointer is considered initialized
		case INITIALIZED: return "INITIALIZED";
	}
}
#endif
void ReleaseSystemSMGRTest();

// @oooii-mike: manages thread local singletons so they can be destructed at thread exit
struct oThreadLocalSingletonMgr : public detail::oProcessSingletonTBase<oThreadLocalSingletonMgr>
{
	//~oThreadLocalSingletonMgr()
	//{
	//	oSWAP(ppInstance, (void*)DEINITIALIZING_MANAGER);
	//}

	static oThreadLocalSingletonMgr* Singleton()
	{
		static oTHREADLOCAL oThreadLocalSingletonMgr* sInstance = 0;
		if((size_t)sInstance < INITIALIZED)
			sInstance = 0;
		ResolveStaticInstance((void**)&sInstance, oBIND( &oProcessSingleton<oThreadLocalSingletonMgr>::ConstructStaticInstance, true ));
		return sInstance;
	}

	void AddThreadLocalSingleton(detail::oSingletonBase** ppSingleton)
	{
		threadlocal_singletons_t::iterator it = tlSingletonArray.begin();
		for(; it != tlSingletonArray.end(); ++it)
		{
			if(**it == *ppSingleton)
				break;
		}

		if(it == tlSingletonArray.end())
		{
			assert(tlSingletonArray.size() < tlSingletonArray.max_size() && "Fixed-size oArray for thread-local singletons is too small.");
			tlSingletonArray.push_back(ppSingleton);
		}
	}

	void ReleaseThreadLocalSingletons()
	{
		oFOREACH(detail::oSingletonBase** ppSingleton, tlSingletonArray)
		{
			(*ppSingleton)->Set(1);
			(*ppSingleton)->Release();
			ppSingleton = NULL;
		}

		Set(1);
		Release();
	}

	size_t GetNumSingletons()
	{
		return tlSingletonArray.size();
	}

private:
	typedef oArray<detail::oSingletonBase**, 1024> threadlocal_singletons_t;
	threadlocal_singletons_t tlSingletonArray;
};

void detail::AddThreadLocalSingleton(oSingletonBase** ppSingleton)
{
	oThreadLocalSingletonMgr::Singleton()->AddThreadLocalSingleton(ppSingleton);
}

void detail::ReleaseThreadLocalSingletons()
{
	oThreadLocalSingletonMgr* pTLSMgr = oThreadLocalSingletonMgr::Singleton();
	if(pTLSMgr)
		pTLSMgr->ReleaseThreadLocalSingletons();
}

// @oooii-mike: Manages singletons to ensure system singletons won't be removed until after all other singletons.
struct oSystemSingletonMgr : public detail::oProcessSingletonTBase<oSystemSingletonMgr>
{
	oSystemSingletonMgr()
		: oProcessSingletonTBase<oSystemSingletonMgr>(false,true)
		, releasingSystemSingletons(false)
	{
		Reference();
	}

	~oSystemSingletonMgr()
	{
		oSWAP(ppInstance, (void*)DEINITIALIZING_MANAGER);
	}

	static oSystemSingletonMgr* Singleton()
	{
		static oRef<oSystemSingletonMgr> sInstance = 0;
		ResolveStaticInstance((void**)sInstance.address(), oBIND( &oProcessSingleton<oSystemSingletonMgr>::ConstructStaticInstance, false ));
		return sInstance;
	}

	void Reference() threadsafe
	{
		RefCount.Reference();
	}

	virtual void Release() threadsafe
	{
		tPtrArray& sysArray = thread_cast<tPtrArray&>(sysSingletonArray);

		if( RefCount.Release() )
		{
			Destroy();
		}
		else if(!releasingSystemSingletons)
		{
			size_t numRefs = sysArray.size() + oThreadLocalSingletonMgr::Singleton()->GetNumSingletons() + 2;

			if(RefCount == (int)numRefs) 
			{
				releasingSystemSingletons = true;

				detail::ReleaseThreadLocalSingletons();

				// Release System Singletons
				size_t numSingletons = sysArray.size();
				for(size_t i = 0; i < numSingletons; i++)
				{
					sysArray[i]->Release();
				}

				sysArray.clear();

				Release();
			}
		}
	}

	void AddSystemSingleton(oProcessSingletonBase* pSingleton)
	{
		sysSingletonArray.push_back(pSingleton);
		pSingleton->Reference();
	}

private:
	typedef oArray<oProcessSingletonBase*, 64> tPtrArray;
	tPtrArray sysSingletonArray;

	bool releasingSystemSingletons;
#ifdef _DEBUG
	//tPtrArray trackedSingletonArray;
	void TrackSingleton(oProcessSingletonBase* pSingleton)
	{
		//trackedSingletonArray.push_back(pSingleton);
	}
#endif

	friend struct detail::oProcessSingletonBase;
};

inline void intrusive_ptr_release(threadsafe oSystemSingletonMgr* _pSingleton) { _pSingleton->Release(); }
inline void intrusive_ptr_add_ref(threadsafe oSystemSingletonMgr* _pSingleton) { _pSingleton->Reference(); }

detail::oProcessSingletonBase::oProcessSingletonBase(bool systemSingleton, bool singletonMgr)
	: ModuleId( oProcess::GetCurrentModuleHandle() )
{
	if(systemSingleton)
		oSystemSingletonMgr::Singleton()->AddSystemSingleton(this);

	if(!singletonMgr)
	{
		intrusive_ptr_add_ref(oSystemSingletonMgr::Singleton());
#ifdef _DEBUG
		//oSystemSingletonMgr::Singleton()->TrackSingleton(this);
#endif
	}
}

detail::oProcessSingletonBase::~oProcessSingletonBase()
{
	if((size_t)*ppInstance != DEINITIALIZING_MANAGER)
		intrusive_ptr_release(oSystemSingletonMgr::Singleton());
}

void detail::oSingletonBase::Reference() threadsafe
{
	RefCount.Reference();
}

void detail::oSingletonBase::Release() threadsafe
{
	if (RefCount.Release())
	{
		Destroy();
	}
}

detail::oSingletonBase::oSingletonBase()
: ppInstance(0)
{

}


bool detail::oSingletonBase::ResolveStaticInstance(void** _ppInstance, oFUNCTION<void*()> _ConstructStaticInstance)
{
	bool constructed = false;
	// use the pointer as a flag to ensure no one else tries to double-init
	// the singleton
	if (UNINITIALIZED == oCAS((size_t*)_ppInstance, INITIALIZING, UNINITIALIZED))
	{
		void* p = _ConstructStaticInstance();
		static_cast<detail::oSingletonBase*>(p)->ppInstance = _ppInstance;
		oSWAP(_ppInstance, p); // OK we're done

		constructed = true;
	}

	else if ((size_t)*_ppInstance == DEINITIALIZED)
		assert(0 && "Accessing deinitialized Singleton() (static teardown ordering issue)");

	else
		oSPIN_UNTIL((size_t)*_ppInstance >= INITIALIZED);

#ifdef _DEBUG
	size_t s = (size_t)*_ppInstance;
	oASSERT(s >= INITIALIZED, "Singleton failed to initialize (is %s)", oAsString((SINGLETON_INIT_STATE)s));
#endif
	return constructed;
}

void detail::oSingletonBase::Destroy() threadsafe
{
	// At base level we just call the destructor directly
	this->~oSingletonBase();
}

void detail::oProcessSingletonBase::Destroy() threadsafe
{
	// Process singleton's need to handle the statically allocated memory
	// and call the destructor unlike oSingletonBase
	oASSERT(ModuleId == oProcess::GetCurrentModuleHandle() || ModuleId == oProcess::GetMainProcessModuleHandle(), "Singleton being freed by a module different than the one creating it.");
	oSWAP(ppInstance, (void*)DEINITIALIZED);
	this->~oProcessSingletonBase();
	oHeap::StaticDeallocateShared((void*)this);
};

const char* detail::oProcessSingletonBase::GetUniqueName(const type_info& _TypeInfo, bool _ThreadUnique)
{
	// it's ok to have some extra static copies of this, around, just so we can do 
	// a quick return of the string for temp use.
	static oTHREADLOCAL char name[64];
	sprintf_s(name, "OOOii.%s", oGetSimpleTypename(_TypeInfo.name()));
	if( _ThreadUnique )
		oStrAppend( name, oCOUNTOF(name), ".%u", oThread::GetCurrentThreadID() );

	return name;
}

void detail::oModuleSingletonBase::Destroy() threadsafe
{
	this->~oModuleSingletonBase();
	oHeap::StaticDeallocateShared((void*)this);
}
