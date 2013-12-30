/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// Enables a degree of non-linear (multiple?) inheritance while presenting a 
// very simple linear inheritance in the public API. This implements the details 
// of oInterface, oGPUDeviceChild and oGPUResource that is typical amongst all 
// oGPUResource types.
#pragma once
#ifndef oGPUCommon_h
#define oGPUCommon_h

#include <oGPU/oGPU.h>

#include <oBasis/oError.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oRefCount.h>
#include <oBase/fixed_string.h>
#include <oBase/fnv1a.h>

#if defined(WIN32) || defined(WIN64)
#undef interface
#include <oCore/windows/win_util.h>
#include <d3d11.h>
#endif

// Use this instead of "struct oMyDerivedClass" to enforce naming consistency 
// and allow for any shared code changes to happen in a central place. If the 
// type is derived from oGPUResource, use the other version below.
#define oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(_oAPI, _ShortTypeName, _32BitA, _16BitB, _16BitC, _8BitD, _8BitE, _8BitF, _8BitG, _8BitH, _8BitI, _8BitJ, _8BitK) \
	oDEFINE_GUID_S(oCONCAT(_oAPI, _ShortTypeName), _32BitA, _16BitB, _16BitC, _8BitD, _8BitE, _8BitF, _8BitG, _8BitH, _8BitI, _8BitJ, _8BitK); \
	struct _oAPI##_ShortTypeName : oGPU##_ShortTypeName, oGPUDeviceChildMixin<oGPU##_ShortTypeName, _oAPI##_ShortTypeName>

// Use this instead of "struct oMyDerivedClass" to enforce naming consistency 
// and allow for any shared code changes to happen in a central place.
#define oDECLARE_GPURESOURCE_IMPLEMENTATION(_oAPI, _ShortTypeName, _ResourceType, _32BitA, _16BitB, _16BitC, _8BitD, _8BitE, _8BitF, _8BitG, _8BitH, _8BitI, _8BitJ, _8BitK) \
	oDEFINE_GUID_S(oCONCAT(_oAPI, _ShortTypeName), _32BitA, _16BitB, _16BitC, _8BitD, _8BitE, _8BitF, _8BitG, _8BitH, _8BitI, _8BitJ, _8BitK); \
	struct _oAPI##_ShortTypeName : oGPU##_ShortTypeName, oGPUResourceMixin<oGPU##_ShortTypeName, _oAPI##_ShortTypeName, _ResourceType>

// Place this macro in the implementation class of an oGPUDeviceChild If the 
// derivation is also an oGPUResource, use the other macro below. Basically this 
// defines the virtual interface in terms of the inline mixins, basically copy-
// pasting the code into place, but the MIXIN implementations used different-
// typed but similar structs. In this way these macros link the templated base 
// class to the generic virtual interface in a way that does not complicate the 
// vtable.
#define oDEFINE_GPUDEVICECHILD_INTERFACE() \
	int Reference() threadsafe override { return MIXINReference(); } \
	void Release() threadsafe override { MIXINRelease(); } \
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override { return MIXINQueryInterface(_InterfaceID, _ppInterface); } \
	void GetDevice(oGPUDevice** _ppDevice) const threadsafe { MIXINGetDevice(_ppDevice); } \
	const char* GetName() const threadsafe override { return MIXINGetURI(); }

#define oDEFINE_GPUDEVICECHILD_INTERFACE_EXPLICIT_QI() \
	int Reference() threadsafe override { return MIXINReference(); } \
	void Release() threadsafe override { MIXINRelease(); } \
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override; \
	void GetDevice(oGPUDevice** _ppDevice) const threadsafe { MIXINGetDevice(_ppDevice); } \
	const char* GetName() const threadsafe override { return MIXINGetURI(); }

// Place this macro in the implementation class of an oGPUResource
#define oDEFINE_GPURESOURCE_INTERFACE() oDEFINE_GPUDEVICECHILD_INTERFACE() \
	ouro::gpu::resource_type::value GetType() const threadsafe override { return MIXINGetType(); } \
	unsigned int GetID() const threadsafe override { return MIXINGetID(); } \
	void GetDesc(interface_type::DESC* _pDesc) const threadsafe override { MIXINGetDesc(_pDesc); } \
	int2 GetByteDimensions(int _Subresource) const threadsafe override;

// The one true hash. This is a persistent hash that can be used at tool time 
// and at runtime and should be capable of uniquely identifying any resource 
// in the system.
inline unsigned int oGPUDeviceResourceHash(const char* _SourceName, ouro::gpu::resource_type::value _Type) { return ouro::fnv1a<unsigned int>(_SourceName, static_cast<unsigned int>(strlen(_SourceName)), _Type); }

template<typename InterfaceT, typename ImplementationT>
struct oGPUDeviceChildMixinBase
{
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;

	oGPUDeviceChildMixinBase(const oGPUDeviceChildMixinBase&)/* = delete*/;
	const oGPUDeviceChildMixinBase& operator=(const oGPUDeviceChildMixinBase&)/* = delete*/;

protected:
	ouro::intrusive_ptr<oGPUDevice> Device;
	oInitOnce<ouro::uri_string> Name;
	oRefCount RefCount;

	// Because of the vtable and the desire to work on the actual class and not
	// really this epherial mixin, use This() rather than 'this' for most local
	// operations.
	ImplementationT* This() { return static_cast<ImplementationT*>(this); }
	const ImplementationT* This() const { return static_cast<ImplementationT*>(this); }
	threadsafe ImplementationT* This() threadsafe { return static_cast<threadsafe ImplementationT*>(this); }
	const threadsafe ImplementationT* This() threadsafe const { return static_cast<threadsafe ImplementationT*>(this); }

	oGPUDeviceChildMixinBase(oGPUDevice* _pDevice, const char* _Name)
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

	inline void MIXINGetDevice(oGPUDevice** _ppDevice) const threadsafe
	{
		*_ppDevice = thread_cast<oGPUDeviceChildMixinBase*>(this)->Device; // safe because pointer never changes, and Reference() is threadsafe
		(*_ppDevice)->Reference();
	}

	inline const char* MIXINGetURI() const threadsafe
	{
		return *Name;
	}
};

template<typename InterfaceT, typename ImplementationT>
struct oGPUDeviceChildMixin : oGPUDeviceChildMixinBase<InterfaceT, ImplementationT>
{
	oGPUDeviceChildMixin(oGPUDevice* _pDevice, const char* _Name)
		: oGPUDeviceChildMixinBase(_pDevice, _Name)
	{}

protected:

	inline bool MIXINQueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
	{
		*_ppInterface = nullptr;

		if (_InterfaceID == oGUID_oGPUDeviceChild || _InterfaceID == oGetGUID<InterfaceT>(0) || _InterfaceID == oGetGUID<ImplementationT>(0))
		{
			This()->Reference();
			*_ppInterface = This();
		}

		else if (_InterfaceID == oGUID_oGPUDevice)
		{
			Device->Reference();
			*_ppInterface = Device;
		}

		return !!*_ppInterface;
	}
};

template<typename InterfaceT, typename ImplementationT, ouro::gpu::resource_type::value Type>
struct oGPUResourceMixin : oGPUDeviceChildMixinBase<InterfaceT, ImplementationT>
{
	typedef typename InterfaceT::DESC desc_type;

	oGPUResourceMixin(oGPUDevice* _pDevice, const typename InterfaceT::DESC& _Desc, const char* _Name)
		: oGPUDeviceChildMixinBase(_pDevice, _Name)
		, Desc(_Desc)
		, ID(oGPUDeviceResourceHash(_Name, Type))
	{}

	inline desc_type* GetDirectDesc() { return &Desc; }

protected:

	desc_type Desc;
	unsigned int ID;

	inline bool MIXINQueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
	{
		*_ppInterface = 0;

		if (_InterfaceID == oGUID_oGPUDeviceChild || _InterfaceID == oGUID_oGPUResource || _InterfaceID == oGetGUID<InterfaceT>(0))
		{
			This()->Reference();
			*_ppInterface = This();
		}

		else if (_InterfaceID == oGUID_oGPUDevice)
		{
			Device->Reference();
			*_ppInterface = Device;
		}

		return !!*_ppInterface;
	}

	inline ouro::gpu::resource_type::value MIXINGetType() const threadsafe
	{
		return Type;
	}

	inline unsigned int MIXINGetID() const threadsafe
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
#define oGPU_CREATE_CHECK_NAME() do { \
	if (!oSTRVALID(_Name)) \
	{ return oErrorSetLast(std::errc::invalid_argument, "A proper name must be specified"); \
	}} while(0)

// Confirm the output has been specified
#define oGPU_CREATE_CHECK_OUTPUT(_ppOut) do { \
	if (!_ppOut) \
	{ return oErrorSetLast(std::errc::invalid_argument, "A valid address for an output pointer must be specified"); \
	}} while(0)

// Check all things typical in Create<resource>() functions
#define oGPU_CREATE_CHECK_PARAMETERS(_ppOut) oGPU_CREATE_CHECK_NAME(); oGPU_CREATE_CHECK_OUTPUT(_ppOut)

// Wrap the boilerplate Create implementations in case we decide to play around 
// with where device children's memory comes from.
#define oDEFINE_GPUDEVICE_CREATE(_oAPI, _TypeShortName) \
	bool _oAPI##Device::Create##_TypeShortName(const char* _Name, const oGPU##_TypeShortName::DESC& _Desc, oGPU##_TypeShortName** _pp##_TypeShortName) \
	{	oGPU_CREATE_CHECK_PARAMETERS(_pp##_TypeShortName); \
		bool success = false; \
		oCONSTRUCT(_pp##_TypeShortName, _oAPI##_TypeShortName(this, _Desc, _Name, &success)); \
		return success; \
	}

// Centralize the signature of the ctors for base types in case system-wide 
// changes need to be made
#define oDECLARE_GPUDEVICECHILD_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName::_oAPI##_TypeShortName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGPUDeviceChildMixin(_pDevice, _Name)

#define oDECLARE_GPURESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GPURESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName::_oAPI##_TypeShortName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGPUResourceMixin(_pDevice, _Desc, _Name)

#endif
