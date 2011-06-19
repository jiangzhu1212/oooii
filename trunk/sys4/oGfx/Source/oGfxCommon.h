// $(header)
// Enables a degree of non-linear (multiple?) inheritance while
// presenting a very simple linear inheritance in the public API.
// This implements the details of oInterface, oGfxDeviceChild and
// oGfxResource that is typical amongst all oGfxResource types.
#pragma once
#ifndef oGfxDeviceChildBase_h
#define oGfxDeviceChildBase_h

#include <oGfx/oGfx.h>

#include <oooii/oErrno.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>

#define oDECLARE_GFXDEVICECHILD_IMPLEMENTATION(_oAPI, _ShortTypeName, _ResourceType) \
	struct _oAPI##_ShortTypeName : oGfx##_ShortTypeName, oGfxDeviceChildMixin<oGfx##_ShortTypeName, _oAPI##_ShortTypeName, oGfxResource::_ResourceType>

#define oDECLARE_GFXRESOURCE_IMPLEMENTATION(_oAPI, _ShortTypeName, _ResourceType) \
	struct _oAPI##_ShortTypeName : oGfx##_ShortTypeName, oGfxResourceMixin<oGfx##_ShortTypeName, _oAPI##_ShortTypeName, oGfxResource::_ResourceType>

// Place this macro in the implementation class of an oGfxDeviceChild
// If the derivation is also an oGfxResource, use the other macro below.
#define oDEFINE_GFXDEVICECHILD_INTERFACE() \
	void Reference() threadsafe override { MIXINReference(); } \
	void Release() threadsafe override { MIXINRelease(); } \
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override { return MIXINQueryInterface(_InterfaceID, _ppInterface); } \
	void GetDevice(threadsafe oGfxDevice** _ppDevice) const threadsafe { MIXINGetDevice(_ppDevice); } \
	const char* GetName() const threadsafe override { return MIXINGetName(); }

// Place this macro in the implementation class of an oGfxResource
#define oDEFINE_GFXRESOURCE_INTERFACE() oDEFINE_GFXDEVICECHILD_INTERFACE() \
	oGfxResource::TYPE GetType() const threadsafe override { return MIXINGetType(); } \
	void GetDesc(interface_type::DESC* _pDesc) const threadsafe override { MIXINGetDesc(_pDesc); }

template<typename InterfaceT, typename ImplementationT>
struct oGfxDeviceChildMixinBase : oNoncopyable
{
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;

protected:
	oRef<threadsafe oGfxDevice> Device;
	char Name[oURI::MAX_URIREF];
	oRefCount RefCount;

	ImplementationT* This() { return static_cast<ImplementationT*>(this); }
	const ImplementationT* This() const { return static_cast<ImplementationT*>(this); }
	threadsafe ImplementationT* This() threadsafe { return static_cast<threadsafe ImplementationT*>(this); }
	const threadsafe ImplementationT* This() threadsafe const { return static_cast<threadsafe ImplementationT*>(this); }

	oGfxDeviceChildMixinBase(threadsafe oGfxDevice* _pDevice, const char* _Name)
		: Device(_pDevice)
	{
		strcpy_s(Name, oSAFESTRN(Name));
	}

	inline void MIXINReference() threadsafe
	{
		RefCount.Reference();
	}
	
	inline void MIXINRelease() threadsafe
	{
		if (RefCount.Release()) delete This();
	}

	inline void MIXINGetDevice(threadsafe oGfxDevice** _ppDevice) const threadsafe
	{
		threadsafe oGfxDevice* pDevice = const_cast<threadsafe oGfxDevice*>(Device.c_ptr());
		pDevice->Reference();
		*_ppDevice = pDevice;
	}

	inline const char* MIXINGetName() const threadsafe
	{
		return thread_cast<const char*>(Name); // safe because it's read-only
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
		*_ppInterface = 0;

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
	{}

protected:

	desc_type Desc;

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

	inline void MIXINGetDesc(desc_type* _pDesc) const threadsafe
	{
		*_pDesc = thread_cast<desc_type&>(Desc); // safe because it's read-only
	}
};

// Macros to unify code and enforce uniformity

#define DEVICE(_API) static_cast<oCONCAT(oCONCAT(o, _API), Device)>(Device.c_ptr())->oCONCAT(_API, Device)

#define oGFXCREATE_CHECK_NAME() do { \
	if (!_Name || !*_Name) \
	{ oSetLastError(EINVAL, "A proper name must be specified"); \
		return false; \
	}} while(0)

#define oGFXCREATE_CHECK_OUTPUT(_ppOut) do { \
	if (!_ppOut) \
	{ oSetLastError(EINVAL, "A valid address for a mesh pointer must be specified"); \
		return false; \
	}} while(0)

#define oGFXCREATE_CHECK_PARAMETERS(_ppOut) oGFXCREATE_CHECK_NAME(); oGFXCREATE_CHECK_OUTPUT(_ppOut)

#define oDEFINE_GFXDEVICE_CREATE(_oAPI, _TypeShortName) \
	bool _oAPI##Device::Create##_TypeShortName(const char* _Name, const oGfx##_TypeShortName::DESC& _Desc, oGfx##_TypeShortName** _pp##_TypeShortName) threadsafe \
	{	oGFXCREATE_CHECK_PARAMETERS(_pp##_TypeShortName); \
		bool success = false; \
		oCONSTRUCT(_pp##_TypeShortName, _oAPI##_TypeShortName(this, _Desc, _Name, &success)); \
		return success; \
	}

#define oDECLARE_GFXRESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess);
#define oBEGIN_DEFINE_GFXRESOURCE_CTOR(_oAPI, _TypeShortName) _oAPI##_TypeShortName::_oAPI##_TypeShortName(threadsafe oGfxDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess) : oGfxResourceMixin(_pDevice, _Name)

#endif
