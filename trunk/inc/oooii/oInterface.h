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
#ifndef oInterface_h
#define oInterface_h

#include <oooii/oStddef.h>
#include <oooii/oGUID.h>
#include <oooii/oAssert.h>

extern const oGUID oIIDInterface;

interface oInterface
{
	virtual void Reference() threadsafe = 0;
	virtual void Release() threadsafe = 0;

	virtual bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe { return false; } // @oooii-tony: FIXME
	
	template<typename T> inline bool QueryInterface(const oGUID& _InterfaceID, T** _ppInterface) threadsafe { T* pInterface = 0; *_ppInterface = (QueryInterface(_InterfaceID, (threadsafe void**)&pInterface) ? pInterface : 0); return !!*_ppInterface; }
	//template<typename T> inline bool QueryInterface(const oGUID& _InterfaceID, threadsafe T** _ppInterface) threadsafe { T* pInterface = 0; *_ppInterface = (QueryInterface(_InterfaceID, (void**)&pInterface) ? pInterface : 0); return !!*_ppInterface; }
	template<typename T> inline bool QueryInterface( T** _ppInterface) threadsafe { return QueryInterface( oGetGUID( _ppInterface ), _ppInterface ); }
	//template<typename T> inline bool QueryInterface( threadsafe T** _ppInterface) threadsafe { return QueryInterface( oGetGUID( _ppInterface ), _ppInterface ); }
};

inline void intrusive_ptr_add_ref(threadsafe oInterface* p) { p->Reference(); }
inline void intrusive_ptr_release(threadsafe oInterface* p) { p->Release(); }

// _____________________________________________________________________________
// Common DirectX-style patterns

#define oDEFINE_GETINTERFACE_INTERFACE(FnThreadSafety, GetInterfaceFn, TypeThreadSafety, Type, FieldName) void GetInterfaceFn(TypeThreadSafety Type** _ppInterface) FnThreadSafety override { (FieldName)->Reference(); *_ppInterface = (FieldName); }

#define oDEFINE_NOOP_REFCOUNT_INTERFACE() void Reference() threadsafe {} void Release() threadsafe {}
#define oDEFINE_REFCOUNT_INTERFACE(FieldName) void Reference() threadsafe override { (FieldName).Reference(); } void Release() threadsafe override { if ((FieldName).Release()) delete this; }
#define oDEFINE_CONST_GETDESC_INTERFACE(FieldName, FnThreadSafety) void GetDesc(DESC* _pDesc) const FnThreadSafety override { *_pDesc = thread_cast<DESC&>(FieldName); }

// Under the MyClass::Create() pattern, usually there's an instantiation of a 
// class and assignment to the output pointer. A common pattern of that 
// implementation is to have the ctor return a bool success by address. This
// macro encapsulates the common procedure that usually follows of checking
// the pointer itself and success.  It also hooks the memory allocation and
// asserts that the class author is properly setting oSetLastError on failure
#define oCONSTRUCT_BASE(_PointerToInstancePointer, object) *_PointerToInstancePointer = new object; if (!*_PointerToInstancePointer) oSetLastError(ENOMEM); else if (!success){ (*_PointerToInstancePointer)->Release(); *_PointerToInstancePointer = 0; }
#ifndef _DEBUG
#define oCONSTRUCT oCONSTRUCT_BASE
#else
#define oCONSTRUCT(_PointerToInstancePointer, object) \
	::size_t ErrorCount = oGetLastErrorCount(); \
	oCONSTRUCT_BASE(_PointerToInstancePointer, object) \
	oASSERT( success || oGetLastErrorCount() > ErrorCount, "%s failed but didn't call oSetLastError", #object );
#endif

// _QI* helper macros are internal, use oDEFINE_TRIVIAL_QUERYINTERFACE*() below
#define _QIPRE() bool QueryInterface(const oGUID& iid, threadsafe void ** _ppInterface) threadsafe override
#define _QIIF(IID) { if (iid == (IID) || iid == oIIDInterface)
#define _QIIF2(baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == oIIDInterface)
#define _QIIF3(baserIID, baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == (baserIID) || iid == oIIDInterface)
#define _QIIF4(basestIID, baserIID, baseIID, IID) { if (iid == (IID) || iid == (baseIID) || iid == (baserIID) || iid == (basestIID) || iid == oIIDInterface)
#define _QIPOST()  Reference(), *_ppInterface = this; else *_ppInterface = 0; return !!*_ppInterface; }

#define oDEFINE_NOOP_QUERYINTERFACE() _QIPRE() { return false; }
#define oDEFINE_TRIVIAL_QUERYINTERFACE(IID) _QIPRE() _QIIF(IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE2(baseIID, IID) _QIPRE() _QIIF2(baseIID, IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE3(baserIID, baseIID, IID) _QIPRE() _QIIF3(baserIID, baseIID, IID) _QIPOST()
#define oDEFINE_TRIVIAL_QUERYINTERFACE4(basestIID, baserIID, baseIID, IID) _QIPRE() _QIIF4(basestIID baserIID, baseIID, IID) _QIPOST()

#endif