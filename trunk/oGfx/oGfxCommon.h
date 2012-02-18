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
// Enables a degree of non-linear (multiple?) inheritance while
// presenting a very simple linear inheritance in the public API.
// This implements the details of oInterface, oGfxDeviceChild and
// oGfxResource that is typical amongst all oGfxResource types.
#pragma once
#ifndef oGfxDeviceChildBase_h
#define oGfxDeviceChildBase_h

#include <oGfx/oGfx.h>

#include <oBasis/oError.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oNoncopyable.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>

// Use this instead of "struct oMyDerivedClass" to enforce naming consistency and allow
// for any shared code changes to happen in a central place. If the type is derived from
// oGfxResource, use the other version below.
#define oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(_oAPI, _ShortTypeName) \
	struct _oAPI##_ShortTypeName : oGfx##_ShortTypeName, oGfxDeviceChildMixin<oGfx##_ShortTypeName, _oAPI##_ShortTypeName>

// Use this instead of "struct oMyDerivedClass" to enforce naming consistency and allow
// for any shared code changes to happen in a central place.
#define oDECLARE_GFXRESOURCE_IMPLEMENTATION(_oAPI, _ShortTypeName, _ResourceType) \
	struct _oAPI##_ShortTypeName : oGfx##_ShortTypeName, oGfxResourceMixin<oGfx##_ShortTypeName, _oAPI##_ShortTypeName, oGfxResource::_ResourceType>

// Place this macro in the implementation class of an oGfxDeviceChild
// If the derivation is also an oGfxResource, use the other macro below.
// Basically this defines the virtual interface in terms of the inline
// mixins, basically copy-pasting the code into place, but the MIXIN
// implementations used different-typed but similar structs. In this 
// way these macros link the templated base class to the generic virtual
// interface in a way that does not complicate the vtable.
#define oDEFINE_GFXDEVICECHILD_INTERFACE() \
	int Reference() threadsafe override { return MIXINReference(); } \
	void Release() threadsafe override { MIXINRelease(); } \
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override { return MIXINQueryInterface(_InterfaceID, _ppInterface); } \
	void GetDevice(threadsafe oGfxDevice** _ppDevice) const threadsafe { MIXINGetDevice(_ppDevice); } \
	const char* GetName() const threadsafe override { return MIXINGetURI(); }

// Place this macro in the implementation class of an oGfxResource
#define oDEFINE_GFXRESOURCE_INTERFACE() oDEFINE_GFXDEVICECHILD_INTERFACE() \
	oGfxResource::TYPE GetType() const threadsafe override { return MIXINGetType(); } \
	uint GetID() const threadsafe override { return MIXINGetID(); } \
	void GetDesc(interface_type::DESC* _pDesc) const threadsafe override { MIXINGetDesc(_pDesc); }

// The one true hash. This is a persistent hash that can be used at tool time 
// and at runtime and should be capable of uniquely identifying any resource 
// in the system.
inline uint oGfxDeviceResourceHash(const char* _SourceName, oGfxResource::TYPE _Type) { return oHash_superfasti(_SourceName, static_cast<uint>(strlen(_SourceName)), _Type); }

template<typename InterfaceT, typename ImplementationT>
struct oGfxDeviceChildMixinBase : oNoncopyable
{
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;

protected:
	oRef<threadsafe oGfxDevice> Device;
	oInitOnce<oStringURI> Name;
	oRefCount RefCount;

	// Because of the vtable and the desire to work on the actual class and not
	// really this epherial mixin, use This() rather than 'this' for most local
	// operations.
	ImplementationT* This() { return static_cast<ImplementationT*>(this); }
	const ImplementationT* This() const { return static_cast<ImplementationT*>(this); }
	threadsafe ImplementationT* This() threadsafe { return static_cast<threadsafe ImplementationT*>(this); }
	const threadsafe ImplementationT* This() threadsafe const { return static_cast<threadsafe ImplementationT*>(this); }

	oGfxDeviceChildMixinBase(threadsafe oGfxDevice* _pDevice, const char* _Name)
		: Device(_pDevice)
		, Name(_Name)
	{}

	inline int MIXINReference() threadsafe
	{
		return RefCount.Reference();
	}
	
	inline void MIXINRelease() threadsafe
	{
		if (RefCount.Release())
			delete This();
	}

	inline void MIXINGetDevice(threadsafe oGfxDevice** _ppDevice) const threadsafe
	{
		threadsafe oGfxDevice* pDevice = const_cast<threadsafe oGfxDevice*>(Device.c_ptr());
		pDevice->Reference();
		*_ppDevice = pDevice;
	}

	inline const char* MIXINGetURI() const threadsafe
	{
		return *Name;
	}
};

template<typename InterfaceT, typename ImplementationT>
struct oGfxDeviceChildMixin : oGfxDeviceChildMixinBase<InterfaceT, ImplementationT>
{
	oGfxDeviceChildMixin(threadsafe oGfxDevice* _pDevice, const char* _Name)
		: oGfxDeviceChildMixinBase(_pDevice, _Name)
	{}

protected:

	inline bool MIXINQueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
	{
		*_ppInterface = nullptr;

		if (_InterfaceID == oGetGUID<oGfxDeviceChild>() || _InterfaceID == oGetGUID<InterfaceT>())
		{
			This()->Reference();
			*_ppInterface = This();
		}

		else if (_InterfaceID == oGetGUID<oGfxDevice>())
		{
			Device->Reference();
			*_ppInterface = Device;
		}

		return !!*_ppInterface;
	}
};

template<typename InterfaceT, typename ImplementationT, oGfxResource::TYPE Type>
struct oGfxResourceMixin : oGfxDeviceChildMixinBase<InterfaceT, ImplementationT>
{
	typedef typename InterfaceT::DESC desc_type;

	oGfxResourceMixin(threadsafe oGfxDevice* _pDevice, const char* _Name)
		: oGfxDeviceChildMixinBase(_pDevice, _Name)
		, ID(oGfxDeviceResourceHash(_Name, Type))
	{}

	inline threadsafe desc_type* GetDirectDesc() threadsafe { return &Desc; }

protected:

	desc_type Desc;
	uint ID;

	inline bool MIXINQueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
	{
		*_ppInterface = 0;

		if (_InterfaceID == oGetGUID<oGfxDeviceChild>() || _InterfaceID == oGetGUID<oGfxResource>() || _InterfaceID == oGetGUID<InterfaceT>())
		{
			This()->Reference();
			*_ppInterface = This();
		}

		else if (_InterfaceID == oGetGUID<oGfxDevice>())
		{
			Device->Reference();
			*_ppInterface = Device;
		}

		return !!*_ppInterface;
	}

	inline oGfxResource::TYPE MIXINGetType() const threadsafe
	{
		return Type;
	}

	inline uint MIXINGetID() const threadsafe
	{
		return ID;
	}

	inline void MIXINGetDesc(desc_type* _pDesc) const threadsafe
	{
		*_pDesc = thread_cast<desc_type&>(Desc); // safe because it's read-only
	}
};

// Macros to unify code and enforce uniformity

#define DEVICE(_API) static_cast<oCONCAT(oCONCAT(o, _API), Device)>(Device.c_ptr())->oCONCAT(_API, Device)

// Confirm that _Name is a valid string
#define oGFXCREATE_CHECK_NAME() do { \
	if (!oSTRVALID(_Name)) \
	{ return oErrorSetLast(oERROR_INVALID_PARAMETER, "A proper name must be specified"); \
	}} while(0)

// Confirm the output has been specified
#define oGFXCREATE_CHECK_OUTPUT(_ppOut) do { \
	if (!_ppOut) \
	{ return oErrorSetLast(oERROR_INVALID_PARAMETER, "A valid address for an output pointer must be specified"); \
	}} while(0)

// Check all things typical in Create<resource>() functions
#define oGFXCREATE_CHECK_PARAMETERS(_ppOut) oGFXCREATE_CHECK_NAME(); oGFXCREATE_CHECK_OUTPUT(_ppOut)

// Wrap the boilerplate Create implementations in case we decide to play around with where device
// children's memory comes from.
#define oDEFINE_GFXDEVICE_CREATE(_oAPI, _TypeShortName) \
	bool _oAPI##Device::Create##_TypeShortName(const char* _Name, const oGfx##_TypeShortName::DESC& _Desc, threadsafe oGfx##_TypeShortName** _pp##_TypeShortName) threadsafe \
	{	oGFXCREATE_CHECK_PARAMETERS(_pp##_TypeShortName); \
		bool success = false; \
		oCONSTRUCT(_pp##_TypeShortName, _oAPI##_TypeShortName(this, _Desc, _Name, &success)); \
		return success; \
	}

// Centralize the signature of the ctors for base types in case system-wide changes need to be made
#define oDECLARE_GFXDEVICECHILD_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GFXDEVICECHILD_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName::_oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGfxDeviceChildMixin(_pDevice, _Name)

#define oDECLARE_GFXRESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GFXRESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName::_oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGfxResourceMixin(_pDevice, _Name)

#endif
