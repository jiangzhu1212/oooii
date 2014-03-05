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
#ifndef oGPU_common_h
#define oGPU_common_h

#include <oGPU/oGPU.h>
#include <oBase/fixed_string.h>
#include <oBase/fnv1a.h>
#include <oBase/intrusive_ptr.h>
#include <oCore/windows/win_util.h>
#include <memory>
#include "d3d11_util.h"

// _____________________________________________________________________________
// The particular API being wrapped

#define oIMPLEMENTATION_NAME d3d11

// _____________________________________________________________________________
// Implementation detail macros (don't use these directly)

#define oIMPLEMENTATION_PREFIX oCONCAT(d3d11,_)

#define oDEVICE_CHILD_CLASS__(_APIPrefix, _BaseTypeName) class oCONCAT(_APIPrefix,_BaseTypeName) : public _BaseTypeName, device_child_mixin<oCONCAT(_BaseTypeName,_info), _BaseTypeName, oCONCAT(_APIPrefix,_BaseTypeName)>
#define oRESOURCE_CLASS__(_APIPrefix, _BaseTypeName) class oCONCAT(_APIPrefix,_BaseTypeName) : public _BaseTypeName, resource_mixin<oCONCAT(_BaseTypeName,_info), _BaseTypeName, oCONCAT(_APIPrefix,_BaseTypeName), resource_type::_BaseTypeName>

#define oDEVICE_CHILD_CTOR__(_APIPrefix, _BaseTypeName) oCONCAT(_APIPrefix,_BaseTypeName)(std::shared_ptr<device>& _Device, const char* _Name, const oCONCAT(_BaseTypeName,_info)& _Info);
#define oRESOURCE_CTOR__(_APIPrefix, _BaseTypeName) oDEVICE_CHILD_CTOR__(_APIPrefix, _BaseTypeName)

#define oDEVICE_CHILD_INTERFACE__ \
	inline std::shared_ptr<device> get_device() const override { return Device; } \
	inline const char* name() const override { return Name; }

#define oRESOURCE_INTERFACE__ oDEVICE_CHILD_INTERFACE__ \
	inline resource_type::value type() const override { return get_type(); } \
	inline uint id() const override { return ID; } \
	inline info_type get_info() const override { return Info; } \
	uint2 byte_dimensions(int _Subresource) const override;

#define oDEVICE_CHILD_CTOR_DEFINITION__(_APIPrefix, _BaseTypeName) oCONCAT(_APIPrefix,_BaseTypeName)::oCONCAT(_APIPrefix,_BaseTypeName)(std::shared_ptr<device>& _Device, const char* _Name, const oCONCAT(_BaseTypeName,_info)& _Info) : device_child_mixin(_Device, _Name)
#define oRESOURCE_CTOR_DEFINITION__(_APIPrefix, _BaseTypeName) oCONCAT(_APIPrefix,_BaseTypeName)::oCONCAT(_APIPrefix,_BaseTypeName)(std::shared_ptr<device>& _Device, const char* _Name, const oCONCAT(_BaseTypeName,_info)& _Info) : resource_mixin(_Device, _Name, _Info)

#define oDEFINE_DEVICE_MAKE__(_APIPrefix, _BaseTypeName) \
	std::shared_ptr<_BaseTypeName> oCONCAT(_APIPrefix,device)::oCONCAT(make_,_BaseTypeName)(const char* _Name, const oCONCAT(_BaseTypeName,_info)& _Info) \
	{ return std::make_shared<oCONCAT(_APIPrefix,_BaseTypeName)>(get_shared(), _Name, _Info); }

// _____________________________________________________________________________
// Device child/resource definition macros

// All declarations and definitions for GPU implemenations should be wrapped in these macros
#define oGPU_NAMESPACE_BEGIN namespace ouro { namespace gpu { namespace oIMPLEMENTATION_NAME {
#define oGPU_NAMESPACE_END }}}

#define oDEVICE_CHILD_CLASS(_BaseTypeName) oDEVICE_CHILD_CLASS__(oIMPLEMENTATION_PREFIX, _BaseTypeName)
#define oRESOURCE_CLASS(_BaseTypeName) oRESOURCE_CLASS__(oIMPLEMENTATION_PREFIX, _BaseTypeName)

#define oDEVICE_CHILD_DECLARATION(_BaseTypeName) public: oDEVICE_CHILD_INTERFACE__ oDEVICE_CHILD_CTOR__(oIMPLEMENTATION_PREFIX, _BaseTypeName)
#define oRESOURCE_DECLARATION(_BaseTypeName) public: oRESOURCE_INTERFACE__ oRESOURCE_CTOR__(oIMPLEMENTATION_PREFIX, _BaseTypeName)

#define oDEVICE_CHILD_CTOR(_BaseTypeName) oDEVICE_CHILD_CTOR_DEFINITION__(oIMPLEMENTATION_PREFIX, _BaseTypeName)
#define oRESOURCE_CTOR(_BaseTypeName) oRESOURCE_CTOR_DEFINITION__(oIMPLEMENTATION_PREFIX, _BaseTypeName)

#define oDEFINE_DEVICE_MAKE(_BaseTypeName) oDEFINE_DEVICE_MAKE__(oIMPLEMENTATION_PREFIX, _BaseTypeName)

// _____________________________________________________________________________
// Base class static mixins

oGPU_NAMESPACE_BEGIN

// The one true hash. This is a persistent hash that can be used at tool time 
// and at runtime and should be capable of uniquely identifying any resource 
// in the system.
inline uint resource_hash(const char* _SourceName, resource_type::value _Type) { return ouro::fnv1a<uint>(_SourceName, static_cast<uint>(strlen(_SourceName)), _Type); }

template<typename InfoT, typename InterfaceT, typename ImplementationT>
class device_child_mixin_base
{
public:
	typedef InfoT info_type;
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;

protected:
	std::shared_ptr<device> Device;
	uri_string Name;

	// Because of the vtable and the desire to work on the actual class and not
	// this epherial mixin, use This() rather than 'this' for most local operations.
	ImplementationT* This() { return static_cast<ImplementationT*>(this); }
	const ImplementationT* This() const { return static_cast<ImplementationT*>(this); }

	device_child_mixin_base(std::shared_ptr<device>& _Device, const char* _Name)
		: Device(_Device)
		, Name(_Name)
	{}

private:
	device_child_mixin_base(const device_child_mixin_base&)/* = delete*/;
	const device_child_mixin_base& operator=(const device_child_mixin_base&)/* = delete*/;
};

template<typename InfoT, typename InterfaceT, typename ImplementationT>
class device_child_mixin : public device_child_mixin_base<InfoT, InterfaceT, ImplementationT>
{
public:
	device_child_mixin(std::shared_ptr<device>& _Device, const char* _Name)
		: device_child_mixin_base(_Device, _Name)
	{}
};

template<typename InfoT, typename InterfaceT, typename ImplementationT, resource_type::value TypeT>
class resource_mixin : public device_child_mixin_base<InfoT, InterfaceT, ImplementationT>
{
public:
	typedef InfoT info_type;
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;

	resource_mixin(std::shared_ptr<device>& _Device, const char* _Name, const InfoT& _Info)
		: device_child_mixin_base(_Device, _Name)
		, Info(_Info)
		, ID(resource_hash(_Name, get_type()))
	{}

	inline resource_type::value get_type() const { return TypeT; }
	inline InfoT& get_info_direct() { return Info; }

protected:
	InfoT Info;
	uint ID;
};

oGPU_NAMESPACE_END


// declare and define the ctor interface. Opening brackets and the body must still be defined.
#define oDEFINE_RESOURCE_CTOR(_APIPrefix, _BaseTypeName) _APIPrefix##_BaseTypeName::_APIPrefix##_BaseTypeName(std::shared_ptr<device>& _Device, const char* _Name, const _BaseTypeName##_info& _Info) : resource_mixin(_Device, _Name, _Info)

// declare and define the ctor interface. Opening brackets and the body must still be defined.
#define oDECLARE_RESOURCE_CTOR(_APIPrefix, _BaseTypeName) _APIPrefix##_BaseTypeName(std::shared_ptr<device>& _Device, const char* _Name, const _BaseTypeName##_info& _Info)
#define oDEFINE_RESOURCE_CTOR(_APIPrefix, _BaseTypeName) _APIPrefix##_BaseTypeName::_APIPrefix##_BaseTypeName(std::shared_ptr<device>& _Device, const char* _Name, const _BaseTypeName##_info& _Info) : resource_mixin(_Device, _Name, _Info)

// D3D11 simplifications of the above macros
#define oDEFINE_D3D11_MAKE(_BaseTypeName) oDEFINE_MAKE(d3d11_, _BaseTypeName)



#if 0

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
#define oDEFINE_GPUDEVICE_CREATE(_oAPI, _BaseTypeName) \
	bool _oAPI##Device::Create##_BaseTypeName(const char* _Name, const oGPU##_BaseTypeName::DESC& _Desc, oGPU##_BaseTypeName** _pp##_BaseTypeName) \
	{	oGPU_CREATE_CHECK_PARAMETERS(_pp##_BaseTypeName); \
		bool success = false; \
		oCONSTRUCT(_pp##_BaseTypeName, _oAPI##_BaseTypeName(this, _Desc, _Name, &success)); \
		return success; \
	}

// Centralize the signature of the ctors for base types in case system-wide 
// changes need to be made
#define oDECLARE_GPUDEVICECHILD_CTOR(_oAPI, _BaseTypeName) _oAPI##_BaseTypeName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(_oAPI, _BaseTypeName) _oAPI##_BaseTypeName::_oAPI##_BaseTypeName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGPUDeviceChildMixin(_pDevice, _Name)

#define oDECLARE_GPURESOURCE_CTOR(_oAPI, _BaseTypeName) _oAPI##_BaseTypeName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GPURESOURCE_CTOR(_oAPI, _BaseTypeName) _oAPI##_BaseTypeName::_oAPI##_BaseTypeName(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGPUResourceMixin(_pDevice, _Desc, _Name)

#endif
#endif
