// $(CHANGE_HEADER)
#include "pch.h"
#include <oooii/oMirroredArena.h>
#include <oooii/oAssert.h>
#include <oooii/oBuffer.h>
#include <oooii/oByte.h>
#include <oooii/oErrno.h>
#include <oooii/oHeap.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oMath.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oStdio.h>
#include <oooii/oSTL.h>
#include <oooii/oSwizzle.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>

// @oooii-tony: Note on future/potential cross-platform support. I admit it, I 
// don't know linux other than it does have virtual memory and it can do some
// interesting things. What I don't see from my 10 minutes of Googling, is if
// Linux has the concept of a GUARD page like Windows does. I think I'd have 
// used Guard pages in this implementation rather than manually marking read-
// only pages r/w myself, but when I was about to change it, I thought again and 
// decided to leave it because A. it works and B. I don't see that what I'd 
// change it to is supported by Linux. If someone knows, let me know!

uintptr_t oBitShiftLeft(unsigned int _BitIndex)
{
	// @oooii-tony: 1 << 63 == 1 << 31 :(   1LL << 63 is what you want.
	return uintptr_t(1) << _BitIndex;
}

namespace detail {

	static const size_t NUM_WORD_BITS = 8 * sizeof(void*);
	static const size_t PAGE_SIZE = (size_t)oHeap::GetPageSize();

	// @oooii-tony: We want to efficiently tell what arena we're in from the global 
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
		unsigned int NumDirtyPages;
	};

	static oHeap::ACCESS GetAccess(oMirroredArena::USAGE _Usage)
	{
		// READ_WRITE_DIFF marks all pages as READ and monitors access to those pages
		return _Usage == oMirroredArena::READ_WRITE ? oHeap::READ_WRITE : oHeap::READ;
	}

	static void* GetBasePointer(void* _pUserPointer)
	{
		return oByteAlignDown(_pUserPointer, REQUIRED_ALIGNMENT);
	}

	static unsigned int GetPageNumber(void* _pUserPointer)
	{
		void* pBase = GetBasePointer(_pUserPointer);
		return static_cast<unsigned int>(oByteDiff(_pUserPointer, pBase) / PAGE_SIZE);
	}

	static void* GetPageBasePointer(void* _pBasePointer, unsigned int _PageNumber)
	{
		return oByteAdd(_pBasePointer, _PageNumber * PAGE_SIZE);
	}

	static void* GetPageBasePointer(void* _pUserPointer)
	{
		return oByteAlignDown(_pUserPointer, PAGE_SIZE);
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

		// @oooii-tony: 
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
			oINC(&pBookkeepingHeader->NumDirtyPages);
	}

	static void UnlockPage(void* _pUserPointer)
	{
		void* pUserPage = GetPageBasePointer(_pUserPointer);
		oHeap::PagedSetProtection(pUserPage, PAGE_SIZE, detail::GetAccess(oMirroredArena::READ_WRITE));
	}

	static void* GetUserPointer(PEXCEPTION_POINTERS _pExceptionInfo)
	{
		// http://msdn.microsoft.com/en-us/library/aa363082(v=VS.85).aspx
		return (void*)_pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
	}

	struct oAccessViolationHandler : public oInterface
	{
		oDEFINE_REFCOUNT_INTERFACE(RefCount);

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
			oRWMutex::ScopedLock lock(Mutex);
			
			threadsafe ARENA* pFreeArena = 0;
			for (size_t i = 0; i < oCOUNTOF(RegisteredArenas); i++)
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
			oRWMutex::ScopedLock lock(Mutex);
			for (size_t i = 0; i < oCOUNTOF(RegisteredArenas); i++)
				if (_pBase == RegisteredArenas[i].pBase)
					RegisteredArenas[i].pBase = 0;
		}

		size_t GetArenaSize(void* _pUserPointer) const
		{
			void* pBase = GetBasePointer(_pUserPointer);
			for (size_t i = 0; i < oCOUNTOF(RegisteredArenas); i++)
				if (pBase == RegisteredArenas[i].pBase)
					return RegisteredArenas[i].Size;
			return 0;
		}

		inline void Lock() threadsafe { Mutex.Lock(); }
		inline void Unlock() threadsafe { Mutex.Unlock(); }
		inline void LockRead() const threadsafe { Mutex.LockRead(); }
		inline void UnlockRead() const threadsafe { Mutex.UnlockRead(); }

		static LONG CALLBACK HandleAccessViolation(PEXCEPTION_POINTERS _pExceptionInfo)
		{
			if (_pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				void* pUserPointer = GetUserPointer(_pExceptionInfo);

				// We lock not only for GetArenaSize() below, but also lock here to lock
				// out a RetrieveChanges() or ApplyChanges().
				oLockedPointer<oAccessViolationHandler> pLockedHandler(Singleton());

				#ifdef _DEBUG
					// @oooii-tony: In release, any unhandled exception is fatal, so favor 
					// performance over safety, but in debug make unexpected exceptions 
					// behave well.

					size_t arenaSize = pLockedHandler->GetArenaSize(pUserPointer);
					if (!arenaSize || (pUserPointer >= oByteAdd(GetBasePointer(pUserPointer), arenaSize)))
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
		oRWMutex Mutex;
	};

} // namespace detail

inline void intrusive_ptr_lock(threadsafe detail::oAccessViolationHandler* p) { p->Lock(); }
inline void intrusive_ptr_unlock(threadsafe detail::oAccessViolationHandler* p) { p->Unlock(); }
inline void intrusive_ptr_lock_read(threadsafe detail::oAccessViolationHandler* p) { p->LockRead(); }
inline void intrusive_ptr_unlock_read(threadsafe detail::oAccessViolationHandler* p) { p->UnlockRead(); }

struct oMirroredArena_Impl : public oMirroredArena
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);

	oMirroredArena_Impl(const DESC* _pDesc, bool* _pSuccess);
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

	bool COPYRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved);
	bool DIFFRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved);

	bool COPYApplyChanges(const void* _pChangeBuffer);
	bool DIFFApplyChanges(const void* _pChangeBuffer);

	DESC Desc;
	oRefCount RefCount;

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
	return detail::REQUIRED_ALIGNMENT;
}

size_t oMirroredArena::GetMaxSize()
{
	return detail::MAX_SIZE;
}

size_t oMirroredArena::GetChangeBufferSize(const void* _pChangeBuffer)
{
	// @oooii-tony: Hmm, this makes me think that the change buffer format, which
	// is a cross-platform concept, should be further isolated from the platform-
	// specific nature of VirtualAlloc().
	const oMirroredArena_Impl::CHANGE_HEADER* pChangeHeader = reinterpret_cast<const oMirroredArena_Impl::CHANGE_HEADER*>(_pChangeBuffer);
	return sizeof(oMirroredArena_Impl::CHANGE_HEADER) + (size_t)pChangeHeader->Size;
}

bool oMirroredArena::Create(const DESC* _pDesc, oMirroredArena** _ppMirroredArena)
{
	if (!_pDesc || !_ppMirroredArena)
	{
		oSetLastError(EINVAL);
		return false;
	}

	bool success = false;
	oCONSTRUCT( _ppMirroredArena, oMirroredArena_Impl(_pDesc, &success) );
	return success;
}

size_t oMirroredArena::GetHeaderSize()
{
	return sizeof(oMirroredArena_Impl::CHANGE_HEADER);
}

oMirroredArena_Impl::oMirroredArena_Impl(const DESC* _pDesc, bool* _pSuccess)
	: Desc(*_pDesc)
{
	*_pSuccess = false;

	oASSERT(oHeap::GetPageSize() == detail::PAGE_SIZE, "Page size is not what's assumed in oMirroredArena");

	if (!oIsByteAligned(_pDesc->BaseAddress, oMirroredArena::GetRequiredAlignment()))
	{
		oSetLastError(EINVAL, "Base address alignment not correct. It must be aligned to 0x%p", oMirroredArena::GetRequiredAlignment());
		goto error;
	}

	if (_pDesc->Size > oMirroredArena::GetMaxSize())
	{
		oSetLastError(EINVAL, "Size larger than the maximum %u(user) > %u(max)", _pDesc->Size, oMirroredArena::GetMaxSize());
		goto error;
	}

	// Allocate the user space, and a known/fixed offset for the bookkeeping.
	void* p = oHeap::PagedAllocate(Desc.BaseAddress, Desc.Size, detail::GetAccess(Desc.Usage));
	if (p != Desc.BaseAddress)
		goto error; // respect last error from PagedAllocate;

	if (Desc.Usage == READ_WRITE_DIFF)
	{
		void* pBookkeeping = oHeap::PagedAllocate(detail::GetBookkeepingBasePointer(Desc.BaseAddress), detail::PAGE_SIZE, detail::GetAccess(READ_WRITE));
		oVB(pBookkeeping);
		oASSERT(pBookkeeping == detail::GetBookkeepingBasePointer(Desc.BaseAddress), "Did not allocate bookkeeping memory correctly.");
		if (!pBookkeeping || pBookkeeping != detail::GetBookkeepingBasePointer(Desc.BaseAddress))
		{
			// back out allocations and fail out
			oSetLastError(ENOMEM, "Failed to allocate bookkeeping pages");
			if (pBookkeeping)
				oHeap::PagedDeallocate(pBookkeeping);
			oHeap::PagedDeallocate(Desc.BaseAddress);
			goto error;
		}

		detail::ResetBookkeeping(Desc.BaseAddress);
	}

	// Ensure access violation handler is instantiated
	detail::oAccessViolationHandler::Singleton();

	#ifdef _DEBUG
		// Register with a double-checker
		{
			detail::oAccessViolationHandler::Singleton()->RegisterArena(Desc.BaseAddress, Desc.Size);
		}
	#endif

	*_pSuccess = true;
	return;

error:
	Desc.BaseAddress = NULL;
}

oMirroredArena_Impl::~oMirroredArena_Impl()
{
	// If we failed to allocate the base address just return
	if( !Desc.BaseAddress )
		return;

	#ifdef _DEBUG
		// Unregister with a double-checker
		{
			detail::oAccessViolationHandler::Singleton()->UnregisterArena(Desc.BaseAddress);
		}
	#endif

	if (Desc.Usage == READ_WRITE_DIFF)
		oHeap::PagedDeallocate(detail::GetBookkeepingBasePointer(Desc.BaseAddress));

	oHeap::PagedDeallocate(Desc.BaseAddress);
}

size_t oMirroredArena_Impl::GetNumDirtyPages() const threadsafe
{
	size_t nDirtyPages = 0;

	switch (Desc.Usage)
	{
		case READ_WRITE_DIFF:
		{
			detail::BOOKKEEPING_HEADER* pBookkeepingHeader = 0;
			void* pVoidDirtyBits = 0;
			detail::GetBookkeepingPointers(Desc.BaseAddress, &pBookkeepingHeader, &pVoidDirtyBits);
			oASSERT(pBookkeepingHeader->MagicNumber == 'OOii', "Arena bookkeeping corrupt");
			nDirtyPages = pBookkeepingHeader->NumDirtyPages;
			break;
		}

		case READ_WRITE:
			// No way to know, assume worst-case.
			nDirtyPages = Desc.Size / detail::PAGE_SIZE;
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
		oSetLastError(EINVAL, "Specified buffer is not large enough");
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

bool oMirroredArena_Impl::DIFFRetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved)
{
	oASSERT(oHeap::GetPageSize() == detail::PAGE_SIZE, "Page size changed unexpectedly");

	detail::BOOKKEEPING_HEADER* pBookkeepingHeader = 0;
	void* pVoidDirtyBits = 0;
	detail::GetBookkeepingPointers(Desc.BaseAddress, &pBookkeepingHeader, &pVoidDirtyBits);

	// @oooii-tony: Do I need to lock here against other pages being dirtied?
	const size_t requiredSize = pBookkeepingHeader->NumDirtyPages * (sizeof(DIFF_HEADER) + detail::PAGE_SIZE);

	if (_pSizeRetrieved)
		*_pSizeRetrieved = sizeof(CHANGE_HEADER) + requiredSize;

	if (!_pChangeBuffer)
		return true;

	if (_SizeofChangeBuffer < requiredSize)
	{
		oSetLastError(EINVAL);
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

	// @oooii-tony: TODO: This can be optimized by special-casing full-word, or
	// even half-word runs. For example, if this is 0xffff...ffff, then one big
	// memcpy could be done rather than 32 or 64 small memcpys.

	unsigned int pagesCopied = 0;
	const size_t nWords = detail::GetNumBookkeepingBytes(Desc.Size) / sizeof(uintptr_t);
	for (size_t i = 0; i < nWords; i++)
	{
		size_t word = pDirtyBits[i];

		int bitindex = firstbitlow(word);
		while (bitindex >= 0)
		{
			DIFF_HEADER* pDiffHeader = reinterpret_cast<DIFF_HEADER*>(pCurrent);
			pDiffHeader->PageNumber = static_cast<unsigned int>(i * detail::NUM_WORD_BITS + bitindex);
			pCurrent = oByteAdd(pCurrent, sizeof(DIFF_HEADER));
			memcpy(pCurrent, detail::GetPageBasePointer(Desc.BaseAddress, pDiffHeader->PageNumber), detail::PAGE_SIZE);
			pCurrent = oByteAdd(pCurrent, detail::PAGE_SIZE);
			pagesCopied++;
			word &=~ oBitShiftLeft(bitindex);
			bitindex = firstbitlow(word);
		}
	}

	oASSERT(pagesCopied == pBookkeepingHeader->NumDirtyPages, "");
	detail::ResetBookkeeping(Desc.BaseAddress);
	
	return true;
}

bool oMirroredArena_Impl::RetrieveChanges(void* _pChangeBuffer, size_t _SizeofChangeBuffer, size_t* _pSizeRetrieved) threadsafe
{
	oLockedPointer<detail::oAccessViolationHandler> pLockedHandler(detail::oAccessViolationHandler::Singleton());

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
			oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ));
			bool result = thread_cast<oMirroredArena_Impl*>(this)->COPYRetrieveChanges(_pChangeBuffer, _SizeofChangeBuffer, _pSizeRetrieved);
			oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ_WRITE));
			return result;
		}

		case READ_WRITE_DIFF:
		{
			oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ));
			return thread_cast<oMirroredArena_Impl*>(this)->DIFFRetrieveChanges(_pChangeBuffer, _SizeofChangeBuffer, _pSizeRetrieved);
		}

		case READ:
		{
			// Read-only arenas don't have diffs
			if (_pSizeRetrieved)
				*_pSizeRetrieved = 0;
			return false;
		}

		default:
			break;
	}

	oASSUME(0);
}

bool oMirroredArena_Impl::COPYApplyChanges(const void* _pChangeBuffer)
{
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);

	if (pChangeHeader->Size != Desc.Size)
	{
		oSetLastError(EINVAL, "Mismatched arena sizes");
		return false;
	}

	if (Desc.Usage == READ)
		oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ_WRITE));

	memcpy(Desc.BaseAddress, pChangeHeader + 1, static_cast<size_t>(pChangeHeader->Size));

	if (Desc.Usage == READ)
		oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ));

	return true;
}

bool oMirroredArena_Impl::DIFFApplyChanges(const void* _pChangeBuffer)
{
	oASSERT(oHeap::GetPageSize() == detail::PAGE_SIZE, "Page size changed unexpectedly");
	
	const CHANGE_HEADER* pChangeHeader = reinterpret_cast<const CHANGE_HEADER*>(_pChangeBuffer);
	const void* pPageDiffs = pChangeHeader + 1;
	const void* pEnd = oByteAdd(pPageDiffs, (size_t)pChangeHeader->Size);

	// @oooii-tony: Is it more efficient to mark the whole arena, or just the pages
	// that changed? It's probably dependent on how much the arena has changed, so
	// really we should take a look at the page count and do both depending on some
	// magic trade-off threshold.
	
	if (Desc.Usage == READ)
		oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ_WRITE));

	while (pPageDiffs < pEnd)
	{
		const DIFF_HEADER* pDiffHeader = reinterpret_cast<const DIFF_HEADER*>(pPageDiffs);
		oASSERT(pDiffHeader->PageNumber < detail::GetNumPages(Desc.Size), "Page number out of range");
		const void* pSource = pDiffHeader + 1;
		void* pDestination = oByteAdd(Desc.BaseAddress, pDiffHeader->PageNumber * detail::PAGE_SIZE);
		memcpy(pDestination, pSource, detail::PAGE_SIZE);
		pPageDiffs = oByteAdd(pPageDiffs, sizeof(DIFF_HEADER) + detail::PAGE_SIZE);
	}

	if (Desc.Usage == READ)
		oHeap::PagedSetProtection(Desc.BaseAddress, Desc.Size, detail::GetAccess(READ));
	
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
			oSetLastError(EINVAL, "Invalid change buffer specified");
			return false;
	}
}