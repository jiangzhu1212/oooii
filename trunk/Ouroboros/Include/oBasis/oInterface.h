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
// A COM-lite style base class for a monolithic inheritance hierarchy and a 
// poor-man's answer for garbage collection. This is the basis for intrusive 
// smart pointer support used in many of the more complex classes. It attempts
// to emulate COM where COM is useful, but tries to avoid the obfuscation 
// overly-generic code can bring.
#ifndef oInterface_h
#define oInterface_h

#include <oBasis/oAssert.h>
#include <oBasis/oGUID.h>
#include <oBasis/oError.h>
#include <oBasis/oThreadsafe.h>

interface oInterface
{
	//Note that this class does not have a virtual destructor. so you should never use delete on a pointer to any type in this hierarchy. delete must always occur from the 
	//	exact type of the actual implementation, i.e. it in general should always happen in the specific implementation of Release which is dynamic unlike the destructor. 
	
	// Returns the new refcount. NOTE: This can be negative, meaning the refcount
	// is invalid. It probably will be a very negative number, not -1 or -2, so
	// if testing for validity, only test for < 0. The refcount should never be
	// 0 either because the count is atomically invalidated so that no quick 
	// ref/rel's happen around a 0->1 transition. Generally the user will not need
	// to check this value and should MAKE ALL ATTEMPTS to avoid using this value,
	// as the assert below in intrusive_ptr_add_ref() indicates. However there are
	// rare cases such as the autolist pattern for resource objects that can be 
	// streaming in from disk, being rendered by dedicated hardware AND going
	// through the regular mult-threaded user lifetime management where it is 
	// possible to query a container for said resource and it could come out that
	// it's been marked for a deferred garbage collection. In that very 
	// specialized case it seems that doing an extra check on the single indicator 
	// of an object's validity is worth doing.
	virtual int Reference() threadsafe = 0;
	virtual void Release() threadsafe = 0;

	// Based on a GUID, return an interface of the associated type. It is expected
	// that client code that knows enough to specify a specific GUID will also 
	// know the type that is expected to be returned.
	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe = 0;
	template<typename T> inline bool QueryInterface(const oGUID& _InterfaceID, T** _ppInterface) threadsafe { return QueryInterface(_InterfaceID, (threadsafe void**)_ppInterface); }
	template<typename T> inline bool QueryInterface(T** _ppInterface) threadsafe { return QueryInterface(oGetGUID(_ppInterface), _ppInterface); }
};

inline void intrusive_ptr_add_ref(threadsafe oInterface* p)
{
	#ifdef oENABLE_ASSERTS
		int count = 
	#endif
	p->Reference();
	oASSERT(count > 0, "ref on invalid object");
}

inline void intrusive_ptr_release(threadsafe oInterface* p) { p->Release(); }

// Lockable interfaces manage thread safety at an object level.  These interfaces
// tend to have mostly non-threadsafe methods so threadsafe access is granted
// via an oLockedPointer that gives exclusive const and mutable access.
interface oLockableInterface : oInterface
{
	virtual void Lock() threadsafe = 0;
	virtual void LockRead() const threadsafe = 0;
	virtual void Unlock() threadsafe = 0;
	virtual void UnlockRead() const threadsafe = 0;
};

// oLockedPointer support
inline void intrusive_ptr_lock(threadsafe oLockableInterface* p) { p->Lock(); }
inline void intrusive_ptr_unlock(threadsafe oLockableInterface* p) { p->Unlock(); }
inline void intrusive_ptr_lock_shared(const threadsafe oLockableInterface* p) { p->LockRead(); }
inline void intrusive_ptr_unlock_shared(const threadsafe oLockableInterface* p) { p->UnlockRead(); }

// _____________________________________________________________________________
// Macros to help with common implementations.

#define oDEFINE_NOOP_REFCOUNT_INTERFACE() int Reference() threadsafe { return 1; } void Release() threadsafe {}
#define oDEFINE_REFCOUNT_INTERFACE(FieldName) int Reference() threadsafe override { return (FieldName).Reference(); } void Release() threadsafe override { if ((FieldName).Release()) delete this; }

#define oDEFINE_LOCKABLE_INTERFACE(mutex) \
	void Lock() threadsafe { (mutex).lock();} \
	void LockRead() const threadsafe { (mutex).lock_shared(); } \
	void Unlock() threadsafe { (mutex).unlock(); } \
	void UnlockRead() const threadsafe { (mutex).unlock_shared(); } 

// The DirectX-style GetDesc() pattern is used frequently to reduce virtual
// accessors and boilerplate typing, so here's a macro that helps implement
// that interface
#define oDEFINE_CONST_GETDESC_INTERFACE(FieldName, FnThreadSafety) void GetDesc(DESC* _pDesc) const FnThreadSafety override { *_pDesc = thread_cast<DESC&>(FieldName); }

// Under the MyClassCreate() pattern, usually there's an instantiation of a 
// class and assignment to the output pointer. A common pattern of that 
// implementation is to have the ctor return a bool success by address. This
// macro encapsulates the common procedure that usually follows of checking
// the pointer itself and success.  It also hooks the memory allocation and
// asserts that the class author is properly setting oErrorSetLast on failure
// Example:
// bool oMyObjCreate(const MYOBJ_DESC& _Desc, threadsafe oMyObj** _ppObj)
// {
//		bool success = false;
//		oCONSTRUCT(_ppObj, oMyObjImpl(_Desc));
//		return !!*_ppObj;
// }
#define oCONSTRUCT_CLEAR(_PointerToInstancePointer) if(*_PointerToInstancePointer){ (*_PointerToInstancePointer)->Release(); *_PointerToInstancePointer = 0; }
#define oCONSTRUCT_NEW(_PointerToInstancePointer, object) *_PointerToInstancePointer = new object
#define oCONSTRUCT_PLACEMENT_NEW(_PointerToInstancePointer, memory, object) *_PointerToInstancePointer = new (memory) object
#define oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer) if (!*_PointerToInstancePointer) oErrorSetLast(oERROR_AT_CAPACITY, "out of memory"); else if (!success) { (*_PointerToInstancePointer)->Release(); *_PointerToInstancePointer = 0; }
#define oCONSTRUCT_BASE(_PointerToInstancePointer, object) oCONSTRUCT_CLEAR(_PointerToInstancePointer); oCONSTRUCT_NEW(_PointerToInstancePointer, object); oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer)
#define oCONSTRUCT_PLACEMENT_BASE(_PointerToInstancePointer, memory, object) oCONSTRUCT_CLEAR(_PointerToInstancePointer); oCONSTRUCT_PLACEMENT_NEW(_PointerToInstancePointer, memory, object); oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer)
#ifndef _DEBUG
	#define oCONSTRUCT oCONSTRUCT_BASE
	#define oCONSTRUCT_PLACEMENT oCONSTRUCT_PLACEMENT_BASE
#else
	#define oCONSTRUCT(_PointerToInstancePointer, object) do \
	{	::size_t ErrorCount = oErrorGetLastCount(); \
		oCONSTRUCT_BASE(_PointerToInstancePointer, object) \
		oASSERT(success || oErrorGetLastCount() > ErrorCount, "%s failed but didn't call oErrorSetLast", #object); \
	} while (false)
	#define oCONSTRUCT_PLACEMENT(_PointerToInstancePointer, memory, object) do \
	{	::size_t ErrorCount = oErrorGetLastCount(); \
		oCONSTRUCT_PLACEMENT_BASE(_PointerToInstancePointer, memory, object) \
		oASSERT(success || oErrorGetLastCount() > ErrorCount, "%s failed but didn't call oErrorSetLast", #object); \
	} while (false)
#endif

// _QI* helper macros are internal, use oDEFINE_TRIVIAL_QUERYINTERFACE*() below
#define _QIPRE() bool QueryInterface(const oGUID& iid, threadsafe void ** _ppInterface) threadsafe override {
#define _QIIF(TYPE) if (iid == (oGetGUID<TYPE>())) \
	{ \
		if (Reference()) *_ppInterface = this; else *_ppInterface = 0; return !!*_ppInterface; \
	}
#define _QIPOST() return false; }

// Quickly implement trivial QueryInterface interfaces
#define oDEFINE_NOOP_QUERYINTERFACE() _QIPRE() _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE(TYPE) _QIPRE() _QIIF(TYPE) _QIIF(oInterface) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE2(BASE_TYPE, TYPE) _QIPRE() _QIIF(TYPE) _QIIF(BASE_TYPE) _QIIF(oInterface) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE3(BASER_TYPE, BASE_TYPE, TYPE) _QIPRE() _QIIF(TYPE) _QIIF(BASE_TYPE) _QIIF(BASER_TYPE) _QIIF(oInterface) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE4(BASEST_TYPE, BASER_TYPE, BASE_TYPE, TYPE) _QIPRE() _QIIF(TYPE) _QIIF(BASE_TYPE) _QIIF(BASER_TYPE) _QIIF(BASEST_TYPE)_QIIF(oInterface) _QIPOST()

#endif
