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
#include <oPlatform/oSingleton.h>
#include <oBasis/oBasisRequirements.h>
#include <oConcurrency/mutex.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSystem.h>
#include <oStd/backoff.h>

using namespace oConcurrency;

// @oooii-tony: Singletons are useful concepts and really cross-platform. It is
// unfortunate that on at least Windows it requires important platform API calls
// to enforce true singleness across DLL boundaries. Because it would be really
// nice to promote this to oBasis, encapsulate all platform calls here in this
// namespace

//#define oENABLE_SINGLETON_TRACE

static void* untracked_malloc(size_t _Size) { return oProcessHeapAllocate(_Size); }
static void untracked_free(void* _Pointer) { oProcessHeapDeallocate(_Pointer); }

namespace oSingletonPlatform
{
#ifdef oENABLE_SINGLETON_TRACE
	// use our own trace because assert and debugger are based on singletons, so 
	// they might be gone or themselves being destructed when we want to print out
	// debug info.
	static void Trace(const char* _TypeinfoName, const char* _File, int _Line, const char* _Format, ...)
	{
		char modname[_MAX_PATH];
		oVERIFY(oModuleGetName(modname, oModuleGetCurrent()));
		char syspath[_MAX_PATH];
		char msg[oKB(4)];
		int offset = oPrintf(msg, "%s(%d): {%s} %s %s ", _File, _Line, oGetFilebase(modname), oSystemGetExecutionPath(syspath), oGetTypename(_TypeinfoName));
		va_list args;
		va_start(args, _Format);
		oVPrintf(msg + offset, oCOUNTOF(msg) - offset, _Format, args);
		va_end(args);
		oStrcat(msg, "\n");
		oDebuggerPrint(msg);
	}
#endif

	oHMODULE GetCurrentModule() { return oModuleGetCurrent(); }

} // namespace oSingletonPlatform

#ifdef oENABLE_SINGLETON_TRACE
	#define oSINGLETON_TRACE(_Name, format, ...) do { oSingletonPlatform::Trace(_Name, __FILE__, __LINE__, format, ## __VA_ARGS__); } while(false)
#else
	#define oSINGLETON_TRACE(_Name, format, ...) __noop
#endif

bool oConstructOnceV(void* volatile* _pPointer, void* (*_New)())
{
	static void* CONSTRUCTING = (void*)0x1;
	bool constructed = false;
	if (*_pPointer <= CONSTRUCTING)
	{
		if (oStd::atomic_compare_exchange(_pPointer, CONSTRUCTING, (void*)nullptr))
		{
			void* p = _New();
			oStd::atomic_exchange(_pPointer, p);
			constructed = true;
		}

		else
		{
			oStd::backoff bo;
			while (*_pPointer <= CONSTRUCTING)
				bo.pause();
		}
	}

	return constructed;
}
	
// {7F15EF8E-3AA2-43D8-B802-06A3E195E21C}
static const oGUID IID_oSingletonCtors = { 0x7f15ef8e, 0x3aa2, 0x43d8, { 0xb8, 0x2, 0x6, 0xa3, 0xe1, 0x95, 0xe2, 0x1c } };
typedef std::unordered_map<oGUID, oStd::type_info_default_constructor, std::hash<oGUID>, std::equal_to<oGUID>, oProcessHeapAllocator<std::pair<oGUID, oStd::type_info_default_constructor>>> oSingletonCtors;
void oPlacementNewSingletonCtor(void* _Pointer) { new(_Pointer) oSingletonCtors(); }

void* oSingletonCtorRegistryCreate()
{
	void* p = nullptr;
	oProcessHeapFindOrAllocate(IID_oSingletonCtors, false, false, sizeof(oSingletonCtors), oPlacementNewSingletonCtor, "oSingletonCtors", &p);
	return p;
}

oSingletonRegister::oSingletonRegister(const char* _SingletonName, const oGUID& _SingletonGUID, oStd::type_info_default_constructor _PlacementNew)
{
	oSingletonCtors& ctors = *(oSingletonCtors*)oSingletonCtorRegistryCreate();
	if (ctors.find(_SingletonGUID) == ctors.end())
	{
		ctors[_SingletonGUID] = _PlacementNew;
		oSINGLETON_TRACE(_SingletonName, "registered");
	}
}

class oThreadlocalRegistry
{
public:

	oThreadlocalRegistry();

	~oThreadlocalRegistry()
	{
		EndThread(); // for main thread case
	}

	static oThreadlocalRegistry* Singleton()
	{
		oThreadlocalRegistry* p = nullptr;
		oProcessHeapFindOrAllocate(GUID, false, true, sizeof(oThreadlocalRegistry), oStd::type_info<oThreadlocalRegistry>::default_construct, "oThreadlocalRegistry", (void**)&p);
		return p;
	}

	static void Destroy()
	{
		oThreadlocalRegistry* p = nullptr;
		if (oProcessHeapFind(GUID, false, (void**)&p))
		{
			p->~oThreadlocalRegistry();
			oProcessHeapDeallocate(p);
		}
	}

	void RegisterThreadlocalSingleton(oSingletonBase* _pSingleton) threadsafe;
	void RegisterAtExit(const oTASK& _AtExit);
	void EndThread();

protected:
	static const oGUID GUID;
	oConcurrency::recursive_mutex Mutex;

	typedef oStd::fixed_vector<oSingletonBase*, 32> thread_singletons_t;
	typedef std::unordered_map<oStd::thread::id, thread_singletons_t, std::hash<oStd::thread::id>, std::equal_to<oStd::thread::id>, oStdUserAllocator<std::pair<const oStd::thread::id, thread_singletons_t>>> singletons_t;
	singletons_t Singletons;

	typedef oStd::fixed_vector<oFUNCTION<void()>, 32> atexitlist_t;
	typedef std::unordered_map<oStd::thread::id, atexitlist_t, std::hash<oStd::thread::id>, std::equal_to<oStd::thread::id>, oStdUserAllocator<std::pair<const oStd::thread::id, atexitlist_t>>> atexits_t;
	atexits_t AtExits;
};

// {CBC5C6D4-7C46-4D05-9143-A043418C0B3A}
const oGUID oThreadlocalRegistry::GUID = { 0xcbc5c6d4, 0x7c46, 0x4d05, { 0x91, 0x43, 0xa0, 0x43, 0x41, 0x8c, 0xb, 0x3a } };

oThreadlocalRegistry::oThreadlocalRegistry()
	: Singletons(0, singletons_t::hasher(), singletons_t::key_equal(), oStdUserAllocator<singletons_t::value_type>(untracked_malloc, untracked_free))
	, AtExits(0, atexits_t::hasher(), atexits_t::key_equal(), oStdUserAllocator<atexits_t::value_type>(untracked_malloc, untracked_free))
{}

// @oooii-tony: External declared elsewhere for some lifetime timing hackin'
void oThreadlocalRegistryCreate()
{
	oThreadlocalRegistry::Singleton();
}

void oThreadlocalRegistryDestroy()
{
	oThreadlocalRegistry::Destroy();
}

oSingletonBase::oSingletonBase(int _InitialRefCount)
	: hModule(oSingletonPlatform::GetCurrentModule())
	, Name("")
	, RefCount(_InitialRefCount)
{}

oSingletonBase::~oSingletonBase()
{
	oSINGLETON_TRACE(Name, "deinitialized");
}

int oSingletonBase::Reference() threadsafe
{
	int r = RefCount.Reference();
	oSINGLETON_TRACE(typeid(*this).name(), "referenced %d -> %d", r-1, r);
	return r;
}

void oSingletonBase::Release() threadsafe
{
	int r = RefCount;
	oSINGLETON_TRACE(typeid(*this).name(), "released %d -> %d", r, r-1);
	if (RefCount.Release())
	{
		oASSERT(hModule == oSingletonPlatform::GetCurrentModule(), "Singleton being freed by a module different than the one creating it.");
		this->~oSingletonBase();
		oProcessHeapDeallocate(thread_cast<oSingletonBase*>(this)); // safe because we're not using this anymore
	}
}
	
void* oSingletonBase::NewV(const char* _TypeInfoName, size_t _Size, oStd::type_info_default_constructor _Ctor, const oGUID& _GUID, bool _IsThreadLocal)
{
	oStd::type_info_default_constructor PlacementNew = _Ctor;

	oSingletonCtors* ctors = nullptr;
	if (oProcessHeapFind(IID_oSingletonCtors, _IsThreadLocal, (void**)&ctors))
	{
		auto it = ctors->find(_GUID);
		oASSERT(it != ctors->end(), "%s ctor not found (did you use forget oSINGLETON_REGISTER?)", _TypeInfoName);
		PlacementNew = it->second;
	}

	oSingletonBase* p = nullptr;
	if (oProcessHeapFindOrAllocate(_GUID, _IsThreadLocal, true, _Size, PlacementNew, oGetTypename(_TypeInfoName), (void**)&p))
	{
		p->Name = oGetTypename(_TypeInfoName);
		oSINGLETON_TRACE(_TypeInfoName, "%ssingleton initialized using %s ctor", _IsThreadLocal ? "threadlocal " : "", !ctors ? "module-local" : "prime-module");

		if (_IsThreadLocal)
		{
			oThreadlocalRegistry::Singleton()->RegisterThreadlocalSingleton(p);
			p->Release();
		}
	}

	else if (!_IsThreadLocal)
		p->Reference();

	return p;
}

void oThreadlocalRegistry::RegisterThreadlocalSingleton(oSingletonBase* _pSingleton) threadsafe
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	thread_singletons_t& ts = thread_cast<singletons_t&>(Singletons)[oStd::this_thread::get_id()]; // protected by mutex above
	ts.push_back(_pSingleton);
	_pSingleton->Reference();
}

void oThreadlocalRegistry::RegisterAtExit(const oTASK& _AtExit)
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	atexitlist_t& list = AtExits[oStd::this_thread::get_id()];
	list.push_back(_AtExit);
}

void oThreadlocalRegistry::EndThread()
{
	oConcurrency::lock_guard<oConcurrency::recursive_mutex> Lock(Mutex);
	oThreadlocalRegistry* pThis = thread_cast<oThreadlocalRegistry*>(this); // protected by mutex above

	// Call all atexit functions while threadlocal singletons are still up
	{
		atexits_t::iterator it = pThis->AtExits.find(oStd::this_thread::get_id());
		if (it != pThis->AtExits.end())
		{
			oFOR(oFUNCTION<void()>& AtExit, it->second)
				AtExit();

			it->second.clear();
		}
	}

	// Now tear down the singletons
	{
		singletons_t::iterator it = pThis->Singletons.find(oStd::this_thread::get_id());
		if (it != pThis->Singletons.end())
		{
			oFOR(oSingletonBase* s, it->second)
				s->Release();

			it->second.clear();
		}
	}
}

void oThreadlocalMalloc(const oGUID& _GUID, const oLIFETIME_TASK& _Create, const oLIFETIME_TASK& _Destroy, size_t _Size, void** _ppAllocation)
{
	// @oooii-tony: Because this can be called from system threads, driver threads,
	// and 3rd-party libs that don't care about your application's reporting (TBB)
	// just punt on reporting these at leaks.
	if (oProcessHeapFindOrAllocate(_GUID, true, false, _Size, nullptr, "oThreadlocalMalloc", _ppAllocation))
	{
		if (_Create)
			_Create(*_ppAllocation);
		
		if (_Destroy)
			oConcurrency::thread_at_exit(oBIND(_Destroy, *_ppAllocation));
		
		oConcurrency::thread_at_exit(oProcessHeapDeallocate, *_ppAllocation);
	}
}

void oConcurrency::thread_at_exit(const std::function<void()>& _AtExit)
{
	oThreadlocalRegistry::Singleton()->RegisterAtExit(_AtExit);
}

void oConcurrency::end_thread()
{
	oThreadlocalRegistry::Singleton()->EndThread();
}
