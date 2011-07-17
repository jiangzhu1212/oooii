// $(header)
#include <oooii/oSingleton.h>
#include <oooii/oDebugger.h>
#include <oooii/oModule.h>
#include <oooii/oProcessHeap.h>
#include <oooii/oString.h>
#include <oooii/oThread.h>
#include <oooii/oThreading.h>
#include <oooii/oSTL.h>

#ifdef _DEBUG
	#define oSINGLETON_TRACE(format, ...) detail::Trace(format, ## __VA_ARGS__)
#else
	#define oSINGLETON_TRACE(format, ...) __noop
#endif

namespace detail {

enum SINGLETON_INIT_STATE
{
	UNINITIALIZED,
	DEINITIALIZED,
	DEINITIALIZING,
	INITIALIZING,
	INITIALIZED,
};

#ifdef _DEBUG
static const char* oAsString(const SINGLETON_INIT_STATE& _State)
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

// use our own trace because assert and debugger are based on singletons, so 
// they might be gone or themselves being destructed when we want to print out
// debug info.
static void Trace(const char* _Format, ...)
{
	char msg[oKB(4)];
	va_list args;
	va_start(args, _Format);
	vsprintf_s(msg, _Format, args);
	va_end(args);
	strcat_s(msg, "\n");
	oDebugger::Print(msg);
}

#endif

static void GetUniqueName(char* _StrDestination, size_t _SizeofStrDestination, const char* _TypeInfoName, bool _MakeThreadUnique)
{
	sprintf_s(_StrDestination, _SizeofStrDestination, "OOOii.%s", oGetSimpleTypename(_TypeInfoName));
	if (_MakeThreadUnique)
		oStrAppend(_StrDestination, _SizeofStrDestination, ".%u", ::GetCurrentThreadId());
}

template<size_t size> static inline void GetUniqueName(char (&_StrDestination)[size], const char* _TypeInfoName, bool _MakeThreadUnique) { GetUniqueName(_StrDestination, size, _TypeInfoName, _MakeThreadUnique); }

struct oProcessThreadlocalSingletonRegistry : public oProcessThreadlocalSingleton<oProcessThreadlocalSingletonRegistry>
{
	oProcessThreadlocalSingletonRegistry()
	{
		atexit(AtExit);
	}

	void Register(oSingletonBase* _pSingleton)
	{
		Singletons.push_back(_pSingleton);
	}

	static void AtExit();

	oArray<oRef<oSingletonBase>, 128> Singletons;
};

void oProcessThreadlocalSingletonRegistry::AtExit()
{
	if (::GetCurrentThreadId() == oWinGetMainThreadID())
		oReleaseAllProcessThreadlocalSingletons();
}

oSingletonBase::oSingletonBase()
	: ppInstance(0)
	, hModule(oModule::GetCurrent())
{
}

oSingletonBase::~oSingletonBase()
{
	char moduleName[_MAX_PATH];
	oVERIFY(oModule::GetModuleName(moduleName, (oHMODULE)hModule));
	oSINGLETON_TRACE("--- %s deinitialized in module %s, thread %u ---", TypeInfoName, moduleName, oGetCurrentThreadID());
}

int oSingletonBase::Reference() threadsafe
{
	return RefCount.Reference();
}

void oSingletonBase::Release() threadsafe
{
	if (RefCount.Release())
	{
		oASSERT(hModule == oModule::GetCurrent(), "Singleton being freed by a module different than the one creating it.");
		oSWAP(ppInstance, (void*)DEINITIALIZED);
		this->~oSingletonBase();
		oProcessHeap::Deallocate(thread_cast<oSingletonBase*>(this)); // safe because we're not using this anymore
	}
}

void oSingletonBase::CallCtor(void* _Memory, oFUNCTION<void (void*)> _Ctor, void** _ppInstance, const char* _TypeInfoName, bool _IsThreadUnique)
{
	_Ctor(_Memory);
	oSingletonBase* pSingleton = reinterpret_cast<oSingletonBase*>(_Memory);
	pSingleton->ppInstance = _ppInstance;
	pSingleton->TypeInfoName = _TypeInfoName;

	if (_IsThreadUnique)
	{
		if (strcmp(_TypeInfoName, "detail::oProcessThreadlocalSingletonRegistry")) // protect against infinite loop
		{
			oProcessThreadlocalSingletonRegistry::Singleton()->Register(pSingleton);
			pSingleton->Release();
		}
	}
}

void oSingletonBase::FindOrCreate(void** _ppInstance, oFUNCTION<void (void*)> _Ctor, const char* _TypeInfoName, size_t _TypeSize, bool _IsProcessUnique, bool _IsThreadUnique)
{
	// use the pointer as a flag to ensure no one else tries to double-init the 
	// singleton
	if (UNINITIALIZED == oCAS((uintptr_t*)_ppInstance, INITIALIZING, UNINITIALIZED))
	{
		const char* InternalTypeInfoName = oGetTypeName(_TypeInfoName);

		char uniqueName[128];

		oSingletonBase* pSingleton = 0;
		detail::GetUniqueName(uniqueName, InternalTypeInfoName, _IsThreadUnique);

		if (_IsProcessUnique)
		{
			#ifdef _DEBUG
				char moduleName[_MAX_PATH];
				oVERIFY(oModule::GetModuleName(moduleName, oModule::GetCurrent()));
			#endif

			if (oProcessHeap::FindOrAllocate(uniqueName, _TypeSize, oBIND(&oSingletonBase::CallCtor, oBIND1, _Ctor, _ppInstance, InternalTypeInfoName, _IsThreadUnique), (void**)&pSingleton))
			{
				oSINGLETON_TRACE("--- %ssingleton %s initialized from module %s, thread %u ---", _IsProcessUnique ? (_IsThreadUnique ? "Process-wide threadlocal " : "Process-wide ") : "", InternalTypeInfoName, moduleName, oGetCurrentThreadID());
			}

			else
			{
				pSingleton->Reference();
				oSINGLETON_TRACE("--- %ssingleton %s referenced from module %s, thread %u ---", _IsProcessUnique ? (_IsThreadUnique ? "Process-wide threadlocal " : "Process-wide ") : "", InternalTypeInfoName, moduleName, oGetCurrentThreadID());
			}
		}

		else
		{
			// Use static heap to avoid spurious memory leak reports
			pSingleton = (oSingletonBase*)oProcessHeap::Allocate(_TypeSize);
		}

		// we're done, so replace module-specific flagged value with actual pointer
		oSWAP(_ppInstance, pSingleton);
	}

	else if ((size_t)*_ppInstance == DEINITIALIZED)
	{
		oSINGLETON_TRACE("oSingleton: Accessing deinitialized %s (static teardown ordering issue)", oGetTypeName(_TypeInfoName));
		oCRTASSERT(false, "oSingleton: Accessing deinitialized oSingleton (static teardown ordering issue)");
	}

	else
		oSPIN_UNTIL((size_t)*_ppInstance >= INITIALIZED);

	#ifdef _DEBUG
		size_t s = (size_t)*_ppInstance;
		if (s <INITIALIZED)
			oSINGLETON_TRACE("oSingleton: Initialization for %s failed (state is %s)", oGetTypeName(_TypeInfoName), oAsString((SINGLETON_INIT_STATE)s));
		oCRTASSERT(s >= INITIALIZED, "oSingleton: Singleton failed to initialize. Check debug output for current state.");
	#endif
}

} // namespace detail

void oReleaseAllProcessThreadlocalSingletons()
{
	detail::oProcessThreadlocalSingletonRegistry* s = detail::oProcessThreadlocalSingletonRegistry::Singleton(false);
	if (oIsValidSingletonPointer(s))
		s->Release();
}

bool oIsValidSingletonPointer(void* _pSingleton)
{
	return (size_t)_pSingleton > detail::INITIALIZED;
}
