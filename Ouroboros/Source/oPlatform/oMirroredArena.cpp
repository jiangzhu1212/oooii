/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oPlatform/oMirroredArena.h>
#include <oHLSL/oHLSLBit.h>
#include <oBase/assert.h>
#include <oBasis/oBuffer.h>
#include <oMemory/byte.h>
#include <oBasis/oError.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oRefCount.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <oCore/windows/win_error.h>

using namespace ouro;
using namespace ouro::page_allocator;
using namespace std;

// @tony: Note on future/potential cross-platform support. I admit it, I 
// don't know linux other than it does have virtual memory and it can do some
// interesting things. What I don't see from my 10 minutes of Googling, is if
// Linux has the concept of a GUARD page like Windows does. I think I'd have 
// used Guard pages in this implementation rather than manually marking read-
// only pages r/w myself, but when I was about to change it, I thought again and 
// decided to leave it because A. it works and B. I don't see that what I'd 
// change it to is supported by Linux. If someone knows, let me know!

uintptr_t oBitShiftLeft(unsigned int _BitIndex)
{
	// @tony: 1 << 63 == 1 << 31 :(  1LL << 63 is what you want.
	return uintptr_t(1) << _BitIndex;
}

namespace detail {

	static const size_t NUM_WORD_BITS = 8 * sizeof(void*);
	static const size_t PAGE_SIZE = page_size();

	// @tony: We want to efficiently tell what arena we're in from the global 
	// exception handler. To facilitate this, we'll assume some things about the
	// arena's structure. Those assumptions are:
	// 1. All oMirroredArenas must be aligned to some power of two. This way 
	//    determination of the arena's base address can be done with a bitmask.
	// 2. All oMirroredArenas have a max size of their alignment, so the bitmask   
	//    works. (what if it were 2x size of alignment? the masked bits might not be 
	//    the base)
	// 3. All bookkeeping is a fixed, known offset from the base, so it's easy to find

	static const size_t REQUIRED_ALIGNMENT = 1 << 27; // 128 MB
	static const size_t MAX_SIZE = REQUIRED_ALIGNMENT;
	static const size_t BOOKKEEPING_INCREMENT = 1 << 29;

	struct BOOKKEEPING_HEADER
	{
		unsigned int MagicNumber; // must be 'OOii'
		std::atomic<unsigned int> NumDirtyPages;
	};

	static void* GetBasePointer(void* _pUserPointer)
	{
		return byte_align_down(_pUserPointer, REQUIRED_ALIGNMENT);
	}

	static unsigned int GetPageNumber(void* _pUserPointer)
	{
		void* pBase = GetBasePointer(_pUserPointer);
		return static_cast<unsigned int>(byte_diff(_pUserPointer, pBase) / PAGE_SIZE);
	}

	static void* GetPageBasePointer(void* _pBasePointer, unsigned int _PageNumber)
	{
		return byte_add(_pBasePointer, _PageNumber * PAGE_SIZE);
	}

	static void* GetPageBasePointer(void* _pUserPointer)
	{
		return byte_align_down(_pUserPointer, PAGE_SIZE);
	}

	static void* GetBookkeepingBasePointer(void* _pUserPointer)
	{
		void* pBase = GetBasePointer(_pUserPointer);
		return (void*)((uintptr_t)pBase | BOOKKEEPING_INCREMENT);
	}

	void GetBookkeepingPointers(void* _pUserPointer, BOOKKEEPING_HEADER** _ppBookkeepingHeader, void** _ppDirtyBits)
	{
		*_ppBookkeepingHeader = (BOOKKEEPING_HEADER*)(GetBookkeepingBasePointer(_pUserPointer));
		*_ppDirtyBits = (*_ppBookkeepingHeader) + 1;
		oASSERT((*_ppBookkeepingHeader)->MagicNumber == 'OOii', "Invalid magic number for oMirroredArena bookkeeping");
	}

	static size_t GetNumPages(size_t _UserSize)
	{
		return _UserSize / PAGE_SIZE;
	}

	static size_t GetNumBookkeepingBytes(size_t _UserSize)
	{
		size_t nPages = GetNumPages(_UserSize);
		return 1 + (nPages / 8); // 8 bits per byte and round up
	}

	static void ResetBookkeeping(void* _pBasePointer)
	{
		BOOKKEEPING_HEADER* pBookkeepingHeader = (BOOKKEEPING_HEADER*)GetBookkeepingBasePointer(_pBasePointer);
		memset(pBookkeepingHeader, 0, PAGE_SIZE);
		pBookkeepingHeader->MagicNumber = 'OOii';
	}

	static void MarkPageDirty(void* _pUserPointer)
	{
		unsigned int pageNumber = GetPageNumber(_pUserPointer);
		unsigned int pageWord = pageNumber / NUM_WORD_BITS;
		unsigned int pageBit = pageNumber % NUM_WORD_BITS;

		BOOKKEEPING_HEADER* pBookkeepingHeader = 0;
		void* pDirtyBits = 0;
		uintptr_t pageMask = oBitShiftLeft(pageBit);

		GetBookkeepingPointers(_pUserPointer, &pBookkeepingHeader, &pDirtyBits);

		// @tony: 
		// !!!!!!!!!!!!!!!!!!!!!!!! IF YOU GET A CRASH HERE !!!!!!!!!!!!!!!!!!!!!!!!
		// oMirroredArena uses advanced features of VirtualAlloc to use access 
		// violations to tell what pages of memory have been touched. This means the
		// system hooks unhandled access violations and for speed assumes it is an
		// intended access violation to be marked here. HOWEVER, if it is an 
		// unintended access violation, then the code a bit higher in the callstack 
		// gets it wrong. In release that's ok since any unhandled exception is 
		// fatal. In debug, there is code to catch this condition. So before looking
		// directly here for the error, see if an exception can be reproduced in a 
		// debug build. If not, then find all READ_WRITE_DIFF usages of 
		// oMirroredArena and change them to READ_WRITE while debugger to use the
		// non-exception-handling path.

		#ifdef _WIN64
				LONGLONG orig = InterlockedOr64((LONGLONG*)pDirtyBits + pageWord, pageMask);
		#elif defined(_WIN32)
				LONG orig = _InterlockedOr((LONG*)pDirtyBits + pageWord, pageMask);
		#else
			#error Unsupported Platform (InterlockedOr)
		#endif

		if ((orig & pageMask) == 0) // if the bit wasn't set before, increment count
			pBookkeepingHeader->NumDirtyPages++;
	}

	static void UnlockPage(void* _pUserPointer)
	{
		void* pUserPage = GetPageBasePointer(_pUserPointer);
		set_access(pUserPage, PAGE_SIZE, access::read_write);
	}

	static void* GetUserPointer(PEXCEPTION_POINTERS _pExceptionInfo)
	{
		// http://msdn.microsoft.com/en-us/library/aa363082(v=VS.85).aspx
		return (void*)_pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
	}

	struct oAccessViolationHandler : oInterface
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);
		oDEFINE_NOOP_QUERYINTERFACE();

		oAccessViolationHandler()
		{
			AddVectoredExceptionHandler(1, HandleAccessViolation);
		}

		~oAccessViolationHandler()
		{
			RemoveVectoredExceptionHandler(HandleAccessViolation);
		}

		static inline threadsafe oAccessViolationHandler* Singleton()
		{
			static oAccessViolationHandler sHandler;
			return &sHandler;
		}

		bool RegisterArena(void* _pBase, size_t _Size) threadsafe
		{
			lock_t lock(thread_cast<shared_mutex&>(Mutex));
			
			threadsafe ARENA* pFreeArena = 0;
			oFORI(i, RegisteredArenas)
			{
				if (!RegisteredArenas[i].pBase)
					pFreeArena = &RegisteredArenas[i];

				if (RegisteredArenas[i].pBase == _pBase && _Size != RegisteredArenas[i].Size)
				{
					oASSERT(false, "Double registration of an arena with a mismatched size.");
					return false;
				}
			}

			if (!pFreeArena)
			{
				oASSERT(false, "Failed to find a free slot for arena registration. This means the maximum allowable oMirroredArena count of %u has been exceeded.", MAX_NUM_ARENAS);
				return false;
			}

			pFreeArena->pBase = _pBase;
			pFreeArena->Size = _Size;

			return true;
		}

		void UnregisterArena(void* _pBase) threadsafe
		{
			lock_t lock(thread_cast<shared_mutex&>(Mutex));
			oFORI(i, RegisteredArenas)
				if (_pBase == RegisteredArenas[i].pBase)
					RegisteredArenas[i].pBase = 0;
		}

		size_t GetArenaSize(void* _pUserPointer) const
		{
			void* pBase = GetBasePointer(_pUserPointer);
			oFORI(i, RegisteredArenas)
				if (pBase == RegisteredArenas[i].pBase)
					return RegisteredArenas[i].Size;
			return 0;
		}

		inline void Lock() threadsafe { thread_cast<shared_mutex&>(Mutex).lock(); }
		inline void Unlock() threadsafe { thread_cast<shared_mutex&>(Mutex).unlock(); }
		inline void LockRead() const threadsafe { thread_cast<shared_mutex&>(Mutex).lock_shared(); }
		inline void UnlockRead() const threadsafe { thread_cast<shared_mutex&>(Mutex).unlock_shared(); }

		static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionInfo)
		{
			if (_pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				void* pUserPointer = GetUserPointer(_pExceptionInfo);

				// We lock not only for GetArenaSize() below, but also lock here to lock
				// out a RetrieveChanges() or ApplyChanges().
				oLockedPointer<oAccessViolationHandler> pLockedHandler(Singleton());

				#ifdef _DEBUG
					// @tony: In release, any unhandled exception is fatal, so favor 
					// performance over safety, but in debug make unexpected exceptions 
					// behave well.

					size_t arenaSize = pLockedHandler->GetArenaSize(pUserPointer);
					if (!arenaSize || (pUserPointer >= byte_add(GetBasePointer(pUserPointer), arenaSize)))
						return EXCEPTION_CONTINUE_SEARCH;
				#endif

				MarkPageDirty(pUserPointer);
				UnlockPage(pUserPointer);
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

	protected:

		static const size_t MAX_NUM_ARENAS = 16;

		struct ARENA
		{
			void* pBase;
			size_t Size;
		};

		ARENA RegisteredArenas[MAX_NUM_ARENAS];

		oRefCount RefCount;
		typedef ouro::shared_mutex mutex_t;
		typedef ouro::lock_guard<mutex_t> lock_t;
		typedef ouro::shared_lock<mutex_t> lock_shared_t;
		mutable mutex_t Mutex;
	};

} // namespace detail

inline void intrusive_ptr_lock(threadsafe ::detail::oAccessViolationHandler* p) { p->Lock(); }
inline void intrusive_ptr_unlock(threadsafe ::detail::oAccessViolationHandler* p) { p->Unlock(); }
inline void intrusive_ptr_lock_shared(threadsafe ::detail::oAccessViolationHandler* p) { p->LockRead(); }
inline void intrusive_ptr_unlock_shared(threadsafe ::detail::oAccessViolationHandler* p) { p->UnlockRead(); }

struct oMirroredArena_Impl : public oMirroredArena
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oMirroredArena);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	oMirroredArena_Impl(const DESC& _Desc, bool* _pSuccess);
	~oMirroredArena_Impl();

	size_t GetNumDirtyPages() const threadsafe override;

	// Pay close attention to order of operations and responsibility in these
	// Apply/Retrieve calls. The main calls handle memory protection and exception
	// interaction in a highly controlled and ordered way. COPY and DIFF and other
	// future methods process the contents of the protected space according to the 
	// specifics of their algorithm. So COPY and DIFF are not necessarily safe, 
	// but are called from the parent Apply/Retrieve, which does guarantee their
	// safety. This also means be careful of orders when adding a new algorithm,
	// because it may break assumptions when there were only two algos to execute.

	bool RetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved) threadsafe override;
	bool ApplyChanges(const void* _pChangeBuffer) override;
	bool IsInChanges(const void* _pAddress, size_t _Size, const void* _pChangeBuffer) const threadsafe override;

	bool COPYRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved);
	bool DIFFRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved);
	bool DINERetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved);

	bool COPYApplyChanges(const void* _pChangeBuffer);
	bool DIFFApplyChanges(const void* _pChangeBuffer);

	DESC Desc;
	oRefCount RefCount;

	// This is only needed for READ_WRITE_DIFF_NO_EXCEPTION
	void** ppDirtyPages;
	size_t DirtyPagesCapacity; // number of void*'s, not the size in bytes

	struct DIFF_HEADER
	{
		unsigned int PageNumber;
	};

	struct CHANGE_HEADER
	{
		unsigned long long Size; // Size of data after CHANGE_HEADER
		unsigned int Type; // currently either 'COPY' or 'DIFF'
	};
};

size_t oMirroredArena::GetRequiredAlignment()
{
	return ::detail::REQUIRED_ALIGNMENT;
}

size_t oMirroredArena::GetMaxSize()
{
	return ::detail::MAX_SIZE;
}

size_t oMirroredArena::GetChangeBufferSize(const void* _pChangeBuffer)
{
	// @tony: Hmm, this makes me think that the change buffer format, which
	// is a cross-platform concept, should be further isolated from the platform-
	// specific nature of VirtualAlloc().
	const oMirroredArena_Impl::CHANGE_HEADER* pChangeHeader = reinterpret_cast<const oMirroredArena_Impl::CHANGE_HEADER*>(_pChangeBuffer);
	return sizeof(oMirroredArena_Impl::CHANGE_HEADER) + (size_t)pChangeHeader->Size;
}

bool oMirroredArenaCreate(const oMirroredArena::DESC& _Desc, oMirroredArena** _ppMirroredArena)
{
	if (!_ppMirroredArena)
		return oErrorSetLast(std::errc::invalid_argument);
	bool success = false;
	oCONSTRUCT(_ppMirroredArena, oMirroredArena_Impl(_Desc, &success));
	return success;
}

size_t oMirroredArena::GetHeaderSize()
{
	return sizeof(oMirroredArena_Impl::CHANGE_HEADER);
}

oMirroredArena_Impl::oMirroredArena_Impl(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, ppDirtyPages(0)
	, DirtyPagesCapacity(0)
{
	*_pSuccess = false;

	oASSERT(page_size() == ::detail::PAGE_SIZE, "Page size is not what's assumed in oMirroredArena");

	if (_Desc.BaseAddress == nullptr)
	{
		oErrorSetLast(std::errc::invalid_argument, "Base address must not be nullptr");
		goto error;
	}

	if (!byte_aligned(_Desc.BaseAddress, oMirroredArena::GetRequiredAlignment()))
	{
		oErrorSetLast(std::errc::invalid_argument, "Base address alignment not correct. It must be aligned to 0x%p", oMirroredArena::GetRequiredAlignment());
		goto error;
	}

	if (_Desc.Size > oMirroredArena::GetMaxSize())
	{
		oErrorSetLast(std::errc::invalid_argument, "Size larger than the maximum %u(user) > %u(max)", _Desc.Size, oMirroredArena::GetMaxSize());
		goto error;
	}

	if (Desc.Usage == READ_WRITE_DIFF)
	{
		try { set_access(Desc.BaseAddress, Desc.Size, access::read_only); }
		catch (std::exception&) { goto error; }

		void* pDesiredBookkeeping = ::detail::GetBookkeepingBasePointer(Desc.BaseAddress);
		void* pBookkeeping = reserve_and_commit(pDesiredBookkeeping, ::detail::PAGE_SIZE, true);
		if (!pBookkeeping || pBookkeeping != pDesiredBookkeeping)
			goto error;

		::detail::ResetBookkeeping(Desc.BaseAddress);

		// Ensure access violation handler is instantiated
		::detail::oAccessViolationHandler::Singleton();

#ifdef _DEBUG
		// Register with a double-checker
		{
			::detail::oAccessViolationHandler::Singleton()->RegisterArena(Desc.BaseAddress, Desc.Size);
		}
#endif
	}

	else if (Desc.Usage == READ_WRITE_DIFF_NO_EXCEPTIONS)
	{
		size_t pageSize = page_size();
		DirtyPagesCapacity = byte_align(Desc.Size, pageSize) / pageSize;
		ppDirtyPages = new void*[DirtyPagesCapacity];
	}

	*_pSuccess = true;
	return;

error:
	Desc.BaseAddress = nullptr;
}

oMirroredArena_Impl::~oMirroredArena_Impl()
{
	// If we failed to allocate the base address just return
	if (!Desc.BaseAddress)
		return;

	if (ppDirtyPages)
		delete [] ppDirtyPages;

	#ifdef _DEBUG
		// Unregister with a double-checker
		{
			::detail::oAccessViolationHandler::Singleton()->UnregisterArena(Desc.BaseAddress);
		}
	#endif

	if (Desc.Usage == READ_WRITE_DIFF)
		unreserve(::detail::GetBookkeepingBasePointer(Desc.BaseAddress));
}

size_t oMirroredArena_Impl::GetNumDirtyPages() const threadsafe
{
	size_t nDirtyPages = 0;

	switch (Desc.Usage)
	{
		case READ_WRITE_DIFF:
		{
			::detail::BOOKKEEPING_HEADER* pBookkeepingHeader = 0;
			void* pVoidDirtyBits = 0;
			::detail::GetBookkeepingPointers(Desc.BaseAddress, &pBookkeepingHeader, &pVoidDirtyBits);
			oASSERT(pBookkeepingHeader->MagicNumber == 'OOii', "Arena bookkeeping corrupt");
			nDirtyPages = pBookkeepingHeader->NumDirtyPages;
			break;
		}

		case READ_WRITE_DIFF_NO_EXCEPTIONS:
			oASSERT(false, "Not yet implemented.");
			break;

		case READ_WRITE:
			// No way to know, assume worst-case.
			nDirtyPages = Desc.Size / ::detail::PAGE_SIZE;
			break;

		case READ: // no dirty pages in read-only memory
		default:
			break;
	}

	return nDirtyPages;
}

bool oMirroredArena_Impl::COPYRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved)
{
	const size_t requiredSize = sizeof(CHANGE_HEADER) + Desc.Size;

	if (_pSizeRetrieved)
		*_pSizeRetrieved = requiredSize;

	if (!_pChangeBuffer)
	{
		if (_pSizeRetrieved)
			*_pSizeRetrieved = requiredSize;
		return true;
	}

	if (_SizeofChangeBuffer < requiredSize)
	{
		oErrorSetLast(std::errc::invalid_argument, "Specified buffer is not large enough");
		return false;
	}

	CHANGE_HEADER* pChangeHeader = reinterpret_cast<CHANGE_HEADER*>(_pChangeBuffer);
	// ensure always the same bit pattern if there 
	// is any struct padding
	memset(pChangeHeader, 0, sizeof(CHANGE_HEADER)); 
	pChangeHeader->Type = 'COPY';
	pChangeHeader->Size = Desc.Size;

	// Copy the entire buffer
	memcpy(pChangeHeader + 1, Desc.BaseAddress, Desc.Size);
	return true;
}

bool oMirroredArena_Impl::DINERetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved)
{
	oASSERT(page_size() == ::detail::PAGE_SIZE, "Page size changed unexpectedly");

	ULONG pageSize = 0;
	ULONG_PTR nDirtyPages = DirtyPagesCapacity;
	DWORD Flags = (_pChangeBuffer && _SizeofChangeBuffer) ? WRITE_WATCH_FLAG_RESET : 0; // only reset if we're really going to write out diffs
	if (0 != GetWriteWatch(Flags, Desc.BaseAddress, Desc.Size, ppDirtyPages, &nDirtyPages, &pageSize))
		throw ouro::windows::error();

	oASSERT(page_size() == pageSize, "Page size is different from WriteWatch than from GetPageSize()");

	size_t requiredSize = nDirtyPages * (sizeof(DIFF_HEADER) + pageSize);

	if (_pSizeRetrieved)
		*_pSizeRetrieved = sizeof(CHANGE_HEADER) + requiredSize;

	if (_pChangeBuffer && _SizeofChangeBuffer)
	{
		CHANGE_HEADER* pChangeHeader = reinterpret_cast<CHANGE_HEADER*>(_pChangeBuffer);
		pChangeHeader->Type = 'DIFF';
		pChangeHeader->Size = requiredSize;

		void* pCurrent = pChangeHeader + 1;

		for (unsigned int i = 0; i < nDirtyPages; i++)
		{
			DIFF_HEADER* pDiffHeader = reinterpret_cast<DIFF_HEADER*>(pCurrent);
			pDiffHeader->PageNumber = static_cast<unsigned int>(byte_diff(ppDirtyPages[i], Desc.BaseAddress) / pageSize);
			pCurrent = byte_add(pCurrent, sizeof(DIFF_HEADER));
			memcpy(pCurrent, ppDirtyPages[i], pageSize);
			pCurrent = byte_add(pCurrent, pageSize);
		}
	}

	return true;
}

bool oMirroredArena_Impl::DIFFRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved)
{
	oASSERT(page_size() == ::detail::PAGE_SIZE, "Page size changed unexpectedly");

	::detail::BOOKKEEPING_HEADER* pBookkeepingHeader = 0;
	void* pVoidDirtyBits = 0;
	::detail::GetBookkeepingPointers(Desc.BaseAddress, &pBookkeepingHeader, &pVoidDirtyBits);

	// @tony: Do I need to lock here against other pages being dirtied?
	const size_t requiredSize = pBookkeepingHeader->NumDirtyPages * (sizeof(DIFF_HEADER) + ::detail::PAGE_SIZE);

	if (_pSizeRetrieved)
		*_pSizeRetrieved = sizeof(CHANGE_HEADER) + requiredSize;

	if (!_pChangeBuffer)
		return true;

	if (_SizeofChangeBuffer < requiredSize)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return false;
	}

	#ifdef _M_X64
		unsigned long long* pDirtyBits = reinterpret_cast<unsigned long long*>(pVoidDirtyBits);
	#else
		unsigned int* pDirtyBits = reinterpret_cast<unsigned int*>(pVoidDirtyBits);
	#endif

	CHANGE_HEADER* pChangeHeader = reinterpret_cast<CHANGE_HEADER*>(_pChangeBuffer);
	pChangeHeader->Type = 'DIFF';
	pChangeHeader->Size = requiredSize;

	void* pCurrent = pChangeHeader + 1;

	// @tony: TODO: This can be optimized by special-casing full-word, or
	// even half-word runs. For example, if this is 0xffff...ffff, then one big
	// memcpy could be done rather than 32 or 64 small memcpys.

	unsigned int pagesCopied = 0;
	const size_t nWords = ::detail::GetNumBookkeepingBytes(Desc.Size) / sizeof(uintptr_t);
	for (size_t i = 0; i < nWords; i++)
	{
		size_t word = pDirtyBits[i];

		int bitindex = firstbitlow(word);
		while (bitindex >= 0)
		{
			DIFF_HEADER* pDiffHeader = reinterpret_cast<DIFF_HEADER*>(pCurrent);
			pDiffHeader->PageNumber = static_cast<unsigned int>(i * ::detail::NUM_WORD_BITS + bitindex);
			pCurrent = byte_add(pCurrent, sizeof(DIFF_HEADER));
			memcpy(pCurrent, ::detail::GetPageBasePointer(Desc.BaseAddress, pDiffHeader->PageNumber), ::detail::PAGE_SIZE);
			pCurrent = byte_add(pCurrent, ::detail::PAGE_SIZE);
			pagesCopied++;
			word &=~ oBitShiftLeft(bitindex);
			bitindex = firstbitlow(word);
		}
	}

	oASSERT(pagesCopied == pBookkeepingHeader->NumDirtyPages, "");
	::detail::ResetBookkeeping(Desc.BaseAddress);
	
	return true;
}

bool oMirroredArena_Impl::RetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved) threadsafe
{
	oLockedPointer<::detail::oAccessViolationHandler> pLockedHandler(::detail::oAccessViolationHandler::Singleton());

	// 1. Lock the exception handler - no new pages get marked
	// 2. Mark all pages read-only so anyone has to go through the exception 
	// 3. While the mark is occurring, if a write sneaks in it's ok, either it will
	//    handler block on the handler Lock, or it's writing to a page about to be 
	//    marked as read-only.
	// 4. Do COPY or DIFF. If DIFF, we're already re-marked read-only, so we're 
	//    done. If COPY, remark the buffer read/write.

	switch (Desc.Usage)
	{
		// @oooii-Andrew: Since COPYRetrieveChanges and DIFFRetrieveChanges are just 
		// different implementations of this function, it is safe to use thread_cast 
		// because these will only be called from this function and we lock above.
		case READ_WRITE:
		{
			set_access(Desc.BaseAddress, Desc.Size, access::read_only);
			bool result = thread_cast<oMirroredArena_Impl*>(this)->COPYRetrieveChanges(_pChangeBuffer, _SizeofChangeBuffer, _pSizeRetrieved);
			set_access(Desc.BaseAddress, Desc.Size, access::read_write);
			return result;
		}

		case READ_WRITE_DIFF:
			set_access(Desc.BaseAddress, Desc.Size, access::read_only);
			return thread_cast<oMirroredArena_Impl*>(this)->DIFFRetrieveChanges(_pChangeBuffer, _SizeofChangeBuffer, _pSizeRetrieved);

		case READ_WRITE_DIFF_NO_EXCEPTIONS:
			return thread_cast<oMirroredArena_Impl*>(this)->DINERetrieveChanges(_pChangeBuffer, _SizeofChangeBuffer, _pSizeRetrieved);

		case READ:
			// Read-only arenas don't have diffs
			if (_pSizeRetrieved)
				*_pSizeRetrieved = 0;
			return false;

		oNODEFAULT;
	}
}

bool oMirroredArena_Impl::COPYApplyChanges(const void* _pChangeBuffer)
{
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);

	if (pChangeHeader->Size != Desc.Size)
	{
		oErrorSetLast(std::errc::invalid_argument, "Mismatched arena sizes");
		return false;
	}

	if (Desc.Usage == READ)
		set_access(Desc.BaseAddress, Desc.Size, access::read_write);

	memcpy(Desc.BaseAddress, pChangeHeader + 1, static_cast<size_t>(pChangeHeader->Size));

	if (Desc.Usage == READ)
		set_access(Desc.BaseAddress, Desc.Size, access::read_only);

	return true;
}

bool oMirroredArena_Impl::DIFFApplyChanges(const void* _pChangeBuffer)
{
	oASSERT(page_size() == ::detail::PAGE_SIZE, "Page size changed unexpectedly");
	
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);
	const void* pPageDiffs = pChangeHeader + 1;
	const void* pEnd = byte_add(pPageDiffs, (size_t)pChangeHeader->Size);

	// @tony: Is it more efficient to mark the whole arena, or just the pages
	// that changed? It's probably dependent on how much the arena has changed, so
	// really we should take a look at the page count and do both depending on some
	// magic trade-off threshold.
	
	if (Desc.Usage == READ)
		set_access(Desc.BaseAddress, Desc.Size, access::read_write);

	while (pPageDiffs < pEnd)
	{
		const DIFF_HEADER* pDiffHeader = reinterpret_cast<const DIFF_HEADER*>(pPageDiffs);
		oASSERT(pDiffHeader->PageNumber < ::detail::GetNumPages(Desc.Size), "Page number out of range");
		const void* pSource = pDiffHeader + 1;
		void* pDestination = byte_add(Desc.BaseAddress, pDiffHeader->PageNumber * ::detail::PAGE_SIZE);
		memcpy(pDestination, pSource, ::detail::PAGE_SIZE);
		pPageDiffs = byte_add(pPageDiffs, sizeof(DIFF_HEADER) + ::detail::PAGE_SIZE);
	}

	if (Desc.Usage == READ)
		set_access(Desc.BaseAddress, Desc.Size, access::read_only);
	
	return true;
}

bool oMirroredArena_Impl::ApplyChanges(const void* _pChangeBuffer)
{
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);
	switch (pChangeHeader->Type)
	{
		case 'DIFF': return DIFFApplyChanges(_pChangeBuffer);
		case 'COPY': return COPYApplyChanges(_pChangeBuffer);
		default:
			oErrorSetLast(std::errc::invalid_argument, "Invalid change buffer specified");
			return false;
	}
}

static size_t CalculateSizeInRange(const void* _pAddress, size_t _Size, const void* _pPage, size_t _PageSize)
{
	const void* pEnd = byte_add(_pAddress, _Size);
	const void* pPageEnd = byte_add(_pPage, _PageSize);

	// range doesn't overlap at all.
	if (pEnd < _pPage || _pAddress >= pPageEnd)
		return 0;

	// There are 4 types of comparisons:
	// 0. Range starts before this page and ends after this page
	// 1. Range starts in this page and continues
	// 2. Range starts before this page and ends in this page
	// 3. Range is 100% inside the specified page
	bool StartsInPage = (_pAddress >= _pPage) && (_pAddress < pPageEnd);
	bool EndsInPage = (pEnd >= _pPage) && (pEnd <= pPageEnd);

	switch ((static_cast<int>(EndsInPage)<<1) | static_cast<int>(StartsInPage))
	{
		case 0: return _PageSize;
		case 1: return static_cast<size_t>(_PageSize - byte_diff(_pAddress, _pPage));
		case 2: return static_cast<size_t>(_Size - byte_diff(_pPage, _pAddress));
		case 3: return _Size;
		oNODEFAULT;
	}
}

bool oMirroredArena_Impl::IsInChanges(const void* _pAddress, size_t _Size, const void* _pChangeBuffer) const threadsafe
{
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);

	switch (pChangeHeader->Type)
	{
		case 'COPY':
		{
			if (_pAddress < Desc.BaseAddress || _pAddress >= byte_add(Desc.BaseAddress, as_int(pChangeHeader->Size)))
				return false;

			size_t offset = byte_diff(_pAddress, Desc.BaseAddress);
			return _Size <= (pChangeHeader->Size - offset);
		}

		case 'DIFF':
		case 'DINE':
		{
			if (0 == CalculateSizeInRange(_pAddress, _Size, Desc.BaseAddress, Desc.Size))
				return false;

			const void* pPageDiffs = pChangeHeader + 1;
			const void* pEnd = byte_add(pPageDiffs, (size_t)pChangeHeader->Size);

			size_t sizeLeft = _Size;
			for (; (sizeLeft > 0) && (pPageDiffs < pEnd); pPageDiffs = byte_add(pPageDiffs, sizeof(DIFF_HEADER) + ::detail::PAGE_SIZE))
			{
				const DIFF_HEADER* pDiffHeader = reinterpret_cast<const DIFF_HEADER*>(pPageDiffs);
				const void* pPageBaseAddress = byte_add(Desc.BaseAddress, pDiffHeader->PageNumber * ::detail::PAGE_SIZE);

				size_t sizeInRange = CalculateSizeInRange(_pAddress, _Size, pPageBaseAddress, ::detail::PAGE_SIZE);
				oASSERT(sizeInRange <= sizeLeft, "Size in range is larger than we're looking for... there's a bug in the calculation somewhere");
				sizeLeft -= sizeInRange;
			}

			return sizeLeft == 0;
		}

		oNODEFAULT;
	}
}