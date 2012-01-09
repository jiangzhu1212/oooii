// $(header)
#include <oPlatform/oSingleton.h>
#include <oBasis/oThread.h>
#include <oPlatform/oDebugger.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oProcessHeap.h>
#include <oPlatform/oSystem.h>

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
		int offset = sprintf_s(msg, "%s(%d): {%s} %s %s ", _File, _Line, oGetFilebase(modname), oSystemGetPath(syspath, oSYSPATH_EXECUTION), oGetTypename(_TypeinfoName));
		va_list args;
		va_start(args, _Format);
		vsprintf_s(msg + offset, oCOUNTOF(msg) - offset, _Format, args);
		va_end(args);
		strcat_s(msg, "\n");
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
			oBackoff bo;
			while (*_pPointer <= CONSTRUCTING)
				bo.Pause();
		}
	}

	return constructed;
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
		oProcessHeapFindOrAllocate(GUID, false, true, sizeof(oThreadlocalRegistry), ctor, "oThreadlocalRegistry", (void**)&p);
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
	void RegisterAtExit(oFUNCTION<void()> _AtExit);
	void EndThread();

protected:
	static const oGUID GUID;
	oRecursiveMutex Mutex;

	typedef oArray<oSingletonBase*, 32> thread_singletons_t;
	typedef std::unordered_map<unsigned int, thread_singletons_t, std::hash<unsigned int>, std::equal_to<unsigned int>, oStdUserCallbackAllocator<std::pair<const unsigned int, thread_singletons_t>>> singletons_t;
	singletons_t Singletons;

	typedef oArray<oFUNCTION<void()>, 32> atexitlist_t;
	typedef std::unordered_map<unsigned int, atexitlist_t, std::hash<unsigned int>, std::equal_to<unsigned int>, oStdUserCallbackAllocator<std::pair<const unsigned int, atexitlist_t>>> atexits_t;
	atexits_t AtExits;

	static void ctor(void* _Pointer) { new (_Pointer) oThreadlocalRegistry(); }
};

// {CBC5C6D4-7C46-4D05-9143-A043418C0B3A}
const oGUID oThreadlocalRegistry::GUID = { 0xcbc5c6d4, 0x7c46, 0x4d05, { 0x91, 0x43, 0xa0, 0x43, 0x41, 0x8c, 0xb, 0x3a } };

oThreadlocalRegistry::oThreadlocalRegistry()
	: Singletons(0, singletons_t::hasher(), singletons_t::key_equal(), oStdUserCallbackAllocator<singletons_t::value_type>(untracked_malloc, untracked_free))
	, AtExits(0, atexits_t::hasher(), atexits_t::key_equal(), oStdUserCallbackAllocator<atexits_t::value_type>(untracked_malloc, untracked_free))
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
	oSINGLETON_TRACE(typeid(*this).name(), "referenced (%d -> %d)", r-1, r);
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

void* oSingletonBase::NewV(const char* _TypeInfoName, size_t _Size, void (*_Ctor)(void*), const oGUID& _GUID, bool _IsThreadLocal)
{
	oSingletonBase* p = nullptr;
	if (oProcessHeapFindOrAllocate(_GUID, _IsThreadLocal, true, _Size, _Ctor, oGetTypename(_TypeInfoName), (void**)&p))
	{
		p->Name = oGetTypename(_TypeInfoName);
		oSINGLETON_TRACE(_TypeInfoName, "%ssingleton initialized", _IsThreadLocal ? "threadlocal " : "");

		if (_IsThreadLocal)
		{
			oThreadlocalRegistry::Singleton()->RegisterThreadlocalSingleton(p);
			p->Release();
		}
	}

	else
		p->Reference();

	return p;
}

unsigned int GetTID()
{
	oStd::thread::id id = oStd::this_thread::get_id();
	return *(unsigned int*)&id;
}

void oThreadlocalRegistry::RegisterThreadlocalSingleton(oSingletonBase* _pSingleton) threadsafe
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	thread_singletons_t& ts = thread_cast<singletons_t&>(Singletons)[GetTID()]; // protected by mutex above
	ts.push_back(_pSingleton);
	_pSingleton->Reference();
}

void oThreadlocalRegistry::RegisterAtExit(oFUNCTION<void()> _AtExit)
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	oStd::thread::id id = oStd::this_thread::get_id();
	atexitlist_t& list = AtExits[*(unsigned int*)&id];
	list.push_back(_AtExit);
}

void oThreadlocalRegistry::EndThread()
{
	oLockGuard<oRecursiveMutex> Lock(Mutex);
	oThreadlocalRegistry* pThis = thread_cast<oThreadlocalRegistry*>(this); // protected by mutex above

	// Call all atexit functions while threadlocal singletons are still up
	{
		atexits_t::iterator it = pThis->AtExits.find(oAsUint(oStd::this_thread::get_id()));
		if (it != pThis->AtExits.end())
		{
			oFOR(oFUNCTION<void()>& AtExit, it->second)
				AtExit();

			it->second.clear();
		}
	}

	// Now tear down the singletons
	{
		singletons_t::iterator it = pThis->Singletons.find(GetTID());
		if (it != pThis->Singletons.end())
		{
			oFOR(oSingletonBase* s, it->second)
				s->Release();

			it->second.clear();
		}
	}
}

void* oThreadlocalMalloc(const oGUID& _GUID, size_t _Size)
{
	void* p = nullptr;

	// @oooii-tony: Because this can be called from system threads, driver threads,
	// and 3rd-party libs that don't care about your application's reporting (TBB)
	// just punt on reporting these at leaks.
	if (oProcessHeapFindOrAllocate(_GUID, true, false, _Size, nullptr, "oThreadlocalMalloc", &p))
		oThreadAtExit(oBIND(oProcessHeapDeallocate, p));
	return p;
}

void oThreadAtExit(oFUNCTION<void()> _AtExit)
{
	oThreadlocalRegistry::Singleton()->RegisterAtExit(_AtExit);
}

void oEndThread()
{
	oThreadlocalRegistry::Singleton()->EndThread();
}
