// $(header)
#ifndef oInterface_h
#define oInterface_h

#include <oooii/oStddef.h>
#include <oooii/oGUID.h>
#include <oooii/oAssert.h>

interface oInterface
{
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

	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe = 0;
	
	template<typename T> inline bool QueryInterface(const oGUID& _InterfaceID, T** _ppInterface) threadsafe { T* pInterface = 0; *_ppInterface = (QueryInterface(_InterfaceID, (threadsafe void**)&pInterface) ? pInterface : 0); return !!*_ppInterface; }
	//template<typename T> inline bool QueryInterface(const oGUID& _InterfaceID, threadsafe T** _ppInterface) threadsafe { T* pInterface = 0; *_ppInterface = (QueryInterface(_InterfaceID, (void**)&pInterface) ? pInterface : 0); return !!*_ppInterface; }
	template<typename T> inline bool QueryInterface( T** _ppInterface) threadsafe { return QueryInterface( oGetGUID( _ppInterface ), _ppInterface ); }
	//template<typename T> inline bool QueryInterface( threadsafe T** _ppInterface) threadsafe { return QueryInterface( oGetGUID( _ppInterface ), _ppInterface ); }
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

// _____________________________________________________________________________
// Common DirectX-style patterns

#define oDEFINE_GETINTERFACE_INTERFACE(FnThreadSafety, GetInterfaceFn, TypeThreadSafety, Type, FieldName) void GetInterfaceFn(TypeThreadSafety Type** _ppInterface) FnThreadSafety override { (FieldName)->Reference(); *_ppInterface = (FieldName); }

#define oDEFINE_NOOP_REFCOUNT_INTERFACE() int Reference() threadsafe { return 1; } void Release() threadsafe {}
#define oDEFINE_REFCOUNT_INTERFACE(FieldName) int Reference() threadsafe override { return (FieldName).Reference(); } void Release() threadsafe override { if ((FieldName).Release()) delete this; }
#define oDEFINE_CONST_GETDESC_INTERFACE(FieldName, FnThreadSafety) void GetDesc(DESC* _pDesc) const FnThreadSafety override { *_pDesc = thread_cast<DESC&>(FieldName); }

// Under the MyClass::Create() pattern, usually there's an instantiation of a 
// class and assignment to the output pointer. A common pattern of that 
// implementation is to have the ctor return a bool success by address. This
// macro encapsulates the common procedure that usually follows of checking
// the pointer itself and success.  It also hooks the memory allocation and
// asserts that the class author is properly setting oSetLastError on failure

#define oCONSTRUCT_NEW(_PointerToInstancePointer, object) *_PointerToInstancePointer = new object
#define oCONSTRUCT_PLACEMENT_NEW(_PointerToInstancePointer, memory, object) *_PointerToInstancePointer = new (memory) object
#define oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer) if (!*_PointerToInstancePointer) oSetLastError(ENOMEM); else if (!success) { (*_PointerToInstancePointer)->Release(); *_PointerToInstancePointer = 0; }
#define oCONSTRUCT_BASE(_PointerToInstancePointer, object) oCONSTRUCT_NEW(_PointerToInstancePointer, object); oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer)
#define oCONSTRUCT_PLACEMENT_BASE(_PointerToInstancePointer, memory, object) oCONSTRUCT_PLACEMENT_NEW(_PointerToInstancePointer, memory, object); oCONSTRUCT_BASE_CHECK(_PointerToInstancePointer)
#ifndef _DEBUG
	#define oCONSTRUCT oCONSTRUCT_BASE
	#define oCONSTRUCT_PLACEMENT oCONSTRUCT_PLACEMENT_BASE
#else
	#define oCONSTRUCT(_PointerToInstancePointer, object) \
		::size_t ErrorCount = oGetLastErrorCount(); \
		oCONSTRUCT_BASE(_PointerToInstancePointer, object) \
		oASSERT(success || oGetLastErrorCount() > ErrorCount, "%s failed but didn't call oSetLastError", #object);
	#define oCONSTRUCT_PLACEMENT(_PointerToInstancePointer, memory, object) \
		::size_t ErrorCount = oGetLastErrorCount(); \
		oCONSTRUCT_PLACEMENT_BASE(_PointerToInstancePointer, memory, object) \
		oASSERT(success || oGetLastErrorCount() > ErrorCount, "%s failed but didn't call oSetLastError", #object);
#endif

// _QI* helper macros are internal, use oDEFINE_TRIVIAL_QUERYINTERFACE*() below
#define _QIPRE() bool QueryInterface(const oGUID& iid, threadsafe void ** _ppInterface) threadsafe override
#define _QIIF(IID) { if (iid == (IID) || iid == oGetGUID<oInterface>())
#define _QIIF2(baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == oGetGUID<oInterface>())
#define _QIIF3(baserIID, baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == (baserIID) || iid == oGetGUID<oInterface>())
#define _QIIF4(basestIID, baserIID, baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == (baserIID) || iid == (basestIID) || iid == oGetGUID<oInterface>())
#define _QIPOST() if (Reference()) *_ppInterface = this; else *_ppInterface = 0; return !!*_ppInterface; }

#define oDEFINE_NOOP_QUERYINTERFACE() _QIPRE() { return false; }
#define oDEFINE_TRIVIAL_QUERYINTERFACE(IID) _QIPRE() _QIIF(IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE2(baseIID, IID) _QIPRE() _QIIF2(baseIID, IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE3(baserIID, baseIID, IID) _QIPRE() _QIIF3(baserIID, baseIID, IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE4(basestIID, baserIID, baseIID, IID) _QIPRE() _QIIF4(basestIID baserIID, baseIID, IID) _QIPOST()

#endif
