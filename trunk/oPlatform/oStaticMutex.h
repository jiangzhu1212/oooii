// $(header)
#pragma once
#ifndef oStaticMutex_h
#define oStaticMutex_h

#include <oBasis/oMutex.h>
#include <oPlatform/oProcessHeap.h>

template<typename UniqueType>
class oStaticMutex : oNoncopyable
{
public:
	inline void lock() threadsafe { mutex()->lock(); }
	inline bool try_lock() threadsafe { return mutex()->try_lock(); }
	inline void unlock() threadsafe { mutex()->unlock(); }
	inline oMutex::native_handle_type native_handle() { mutex()->native_handle(); }

protected:
	oMutex* mutex() threadsafe
	{
		if (!pInternal)
		{
			if (oProcessHeapFindOrAllocate(GUID, false, true, sizeof(oMutex), NewMutex, oGetTypename(typeid(*this).name()), (void**)&pInternal))
				atexit(DeleteMutex);
		}

		return pInternal;
	}

	static void NewMutex(void* _pInstance) { new (_pInstance) oMutex(); }
	static void DeleteMutex()
	{
		oMutex* pTemp = pInternal;
		oStd::atomic_exchange(&pInternal, nullptr);
		pTemp->~oMutex();
		oProcessHeapDeallocate(pTemp);
	}

	static const oGUID GUID;
	static oMutex* pInternal;
};

#define oDEFINE_STATIC_MUTEX(_UniqueName, _GUID) \
	oDECLARE_HANDLE(_UniqueName##_t); \
	static oStaticMutex<_UniqueName##_t> _UniqueName; \
	const oGUID oStaticMutex<_UniqueName##_t>::GUID = _GUID; \
	oMutex* oStaticMutex<_UniqueName##_t>::pInternal = nullptr

#endif
