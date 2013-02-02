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
#ifndef oStaticMutex_h
#define oStaticMutex_h

#include <oBasis/oMutex.h>
#include <oPlatform/oProcessHeap.h>

template<typename MutexT, typename UniqueType>
class oStaticMutex : oNoncopyable
{
public:
	inline void lock() threadsafe { mutex()->lock(); }
	inline bool try_lock() threadsafe { return mutex()->try_lock(); }
	inline void unlock() threadsafe { mutex()->unlock(); }
	inline typename MutexT::native_handle_type native_handle() { mutex()->native_handle(); }

protected:
	MutexT* mutex() threadsafe
	{
		if (!pInternal)
		{
			if (oProcessHeapFindOrAllocate(GUID, false, true, sizeof(MutexT), NewMutex, oGetTypename(typeid(*this).name()), (void**)&pInternal))
				atexit(DeleteMutex);
		}

		return pInternal;
	}

	static void NewMutex(void* _pInstance) { new (_pInstance) MutexT(); }
	static void DeleteMutex()
	{
		MutexT* pTemp = pInternal;
		oStd::atomic_exchange(&pInternal, nullptr);
		pTemp->~MutexT();
		oProcessHeapDeallocate(pTemp);
	}

	static const oGUID GUID;
	static MutexT* pInternal;
};

#define oDEFINE_STATIC_MUTEX(_MutexT, _UniqueName, _GUID) \
	oDECLARE_HANDLE(_UniqueName##_t); \
	static oStaticMutex<_MutexT, _UniqueName##_t> _UniqueName; \
	const oGUID oStaticMutex<_MutexT, _UniqueName##_t>::GUID = _GUID; \
	_MutexT* oStaticMutex<_MutexT, _UniqueName##_t>::pInternal = nullptr

#endif
