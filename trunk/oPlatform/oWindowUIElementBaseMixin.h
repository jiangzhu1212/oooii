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
#pragma once
#ifndef oGDIWindowUIElementBaseMixin_h
#define oGDIWindowUIElementBaseMixin_h

#include <oBasis/oStdAtomic.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <oPlatform/oWindowUI.h>
#include <oPlatform/oGDI.h>
#include "oSpinlockEx.h"
#include <oPlatform/oWindow.h>
#include <oBasis/oAssert.h>

#define oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE() \
	oDEFINE_REFCOUNT_INTERFACE(RefCount) \
	oDEFINE_NOOP_QUERYINTERFACE() \
	void GetWindow(threadsafe oWindow** _ppWindow) threadsafe override { Window->Reference(); *_ppWindow = Window; } \
	void GetDesc(DESC* _pDesc) threadsafe override { MIXINGetDesc(_pDesc); }

#define oDEFINE_WINDOWUIELEMENT_INTERFACE() \
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE() \
	DESC* Map() threadsafe override { return MIXINMap(); } \
	void Unmap() threadsafe override { MIXINUnmap(); }

#define oDECLARE_CTORDTOR(_ClassName) \
	_ClassName(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess); \
	~_ClassName();

#define oDECLARE_ONEVENT() bool OnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale);

// Can't put this in the mixin because it's ctor executes before the derived 
// one - fields might not be initialized at that time.
#define oHOOK_ONEVENT(_ImplementationT) HookID = Window->Hook(oBIND(&_ImplementationT::OnEvent, this, oBIND1, oBIND2, oBIND3));
#define oUNHOOK_ONEVENT() Window->Unhook(HookID)

template<typename InterfaceT, typename ImplementationT> struct oWindowUIElementBaseMixin
{
	static const unsigned int LOCKED = 0x80000000;

	typedef InterfaceT interface_type;
	typedef typename InterfaceT::DESC desc_type;

	ImplementationT* This() { return (ImplementationT*)this; }
	const ImplementationT* This() const { return (ImplementationT*)this; }
	threadsafe ImplementationT* This() threadsafe { return (threadsafe ImplementationT*)this; }
	const threadsafe ImplementationT* This() threadsafe const { return (threadsafe ImplementationT*)this; }

	oWindowUIElementBaseMixin(const desc_type& _Desc, threadsafe oWindow* _pWindow)
		: Window(_pWindow)
		, HookID(oInvalid)
		, DescIndex(0)
	{
		Desc[0] = Desc[1] = _Desc;
	}

	desc_type& MIXINGetDesc() const threadsafe
	{
		return thread_cast<desc_type&>(Desc[DescIndex &~ LOCKED]);
	}

	inline void MIXINGetDesc(desc_type* _pDesc) const threadsafe
	{
		*_pDesc = This()->MIXINGetDesc();
	}

	inline desc_type* MIXINMap() threadsafe
	{
		unsigned int i, newI;
		do { i = DescIndex &~ LOCKED; newI = LOCKED | i; } while (!oStd::atomic_compare_exchange(&DescIndex, newI, i));
		i = (i + 1) % oCOUNTOF(Desc);
		return thread_cast<desc_type*>(&Desc[i]);
	}

	inline void MIXINUnmap() threadsafe
	{
		unsigned int i, newI;
		do {
			i = DescIndex;
			oASSERT((i & LOCKED) == LOCKED, "DescIndex is already LOCKED.");
			i |= LOCKED;
			newI = (i + 1) % oCOUNTOF(Desc); 
		} while (!oStd::atomic_compare_exchange(&DescIndex, newI, i));
	}

	oRef<threadsafe oWindow> Window;
	oRefCount RefCount;
	unsigned int HookID;
private:
	desc_type Desc[2];
	unsigned int DescIndex;
};

#endif
