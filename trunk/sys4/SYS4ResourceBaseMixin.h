// $(header)
// Enables a degree of non-linear (multiple?) inheritance while
// presenting a very simple linear inheritance in the public API.
// This implements the details of oInterface, oGPUDeviceChild and
// oGPUResource that is typical amongst all oGPUResource types.
#pragma once
#ifndef SYS4ResourceBaseMixin_h
#define SYS4ResourceBaseMixin_h

#include <SYS4/SYS4Render.h>

#include <oooii/oRef.h>
#include <oooii/oRefCount.h>

// Call this macro in each implementation of oGPUResource derivations
// to use a mixin-patterned object to handle the implementation of
// oGPUResource's virtual interface without introducing multiple
// inheritance.
#define SYS4_DEFINE_GPURESOURCE_INTERFACE() \
	void Reference() threadsafe override { MIXINReference(); } \
	void Release() threadsafe override { MIXINRelease(); } \
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override { return IMPLQueryInterface(_InterfaceID, _ppInterface); } \
	void GetDevice(threadsafe oGPUDevice** _ppDevice) const threadsafe { MIXINGetDevice(_ppDevice); } \
	oGPUResource::TYPE GetType() const threadsafe override { return MIXINGetType(); } \
	const char* GetName() const threadsafe override { return MIXINGetName(); } \
	const char* GetCacheName() const threadsafe override { return MIXINGetCacheName(); } \
	void GetDesc(interface_type::DESC* _pDesc) const threadsafe pverride { MIXINGetDesc(_pDesc); }

template<typename InterfaceT, typename ImplementationT, oGPUResource::TYPE Type>
struct SYS4ResourceBaseMixin
{
	typedef InterfaceT interface_type;
	typedef ImplementationT implementation_type;
	typedef typename InterfaceT::DESC desc_type;

protected:
	oRef<threadsafe oGPUDevice> Device;
	char Name[oURI::MAX_URIREF];
	char CacheName[oURI::MAX_URI];
	oRefCount RefCount;

	ImplementationT* This() { return static_cast<ImplementationT*>(this); }
	const ImplementationT* This() const { return static_cast<ImplementationT*>(this); }
	threadsafe ImplementationT* This() threadsafe { return static_cast<threadsafe ImplementationT*>(this); }
	const threadsafe ImplementationT* This() threadsafe const { return static_cast<threadsafe ImplementationT*>(this); }

	SYS4ResourceBaseMixin(threadsafe oGPUDevice* _pDevice, const desc_type& _Desc, const char* _Name, const char* _CacheName)
		: Device(_pDevice)
	{
		Desc = _Desc;
		strcpy_s(Name, oSAFESTRN(Name));
		strcpy_s(CacheName, oSAFESTRN(_CacheName));
	}

	inline void MIXINReference() threadsafe
	{
		RefCount.Reference();
	}
	
	inline void MIXINRelease() threadsafe
	{
		if (RefCount.Release()) delete This();
	}

	inline bool MIXINQueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
	{
		*_ppInterface = 0;

		if (_InterfaceID == oGetGUID<oGPUResource>() || _InterfaceID == oGetGUID<InterfaceT>())
		{
			This()->Reference();
			*_ppInterface = This();
		}

		else if (_InterfaceID == oGetGUID(oGPUDevice)
		{
			Device->Reference();
			*_ppInterface = Device;
		}

		return !!*_ppInterface;
	}

	inline void MIXINGetDevice(threadsafe oGPUDevice** _ppDevice) const threadsafe
	{
		Device->Reference();
		*_ppDevice = Device;
	}

	inline oGPUResource::TYPE MIXINGetType() const threadsafe
	{
		return Type;
	}
	
	inline const char* MIXINGetName() const threadsafe
	{
		return thread_cast<const char*>(Name); // safe because it's read-only
	}

	inline const char* MIXINGetCacheName() const threadsafe
	{
		return thread_cast<const char*>(CacheName); // safe because it's read-only
	}

	inline void MIXINGetDesc(desc_type* _pDesc) const threadsafe
	{
		*_pDesc = thread_cast<desc_type&>(Desc); // safe because it's read-only
	}
};

#define SYS4_CHECK_NAME() do { \
	if (!_Name || !*_Name) \
	{ oSetLastError(EINVAL, "A proper name must be specified"); \
		return false; \
	}} while(0)

#define SYS4_CHECK_OUTPUT(_ppOut) do { \
	if (!_ppOut) \
	{ oSetLastError(EINVAL, "A valid address for a mesh pointer must be specified"); \
		return false; \
	}} while(0)

#define SYS4_CHECK_PARAMETERS() SYS4_CHECK_NAME(); SYS4_CHECK_OUTPUT()

#define oSYMMERGE(a,b) a##b

#define SYS4_DEFINE_GPURESOURCE_CREATE(_API, _Resource) \
	bool oSYMMERGE(oSYMMERGE(o, _API), Device)::oSYMMERGE(Create,_Resource)(const char* _Name, const oGPU##_Resource::DESC& _Desc, threadsafe oSYMMERGE(oGPU, _Resource)** _ppResource) threadsafe \
	{	SYS4_CHECK_PARAMETERS(); \
		bool success = false; \
		oCONSTRUCT(_ppResource, oSYMMERGE(oSYMMERGE(o, _API), _Resource)(this, _Desc, &success)); \
		return success; \
	}

#endif
