// $(header)
#ifndef oVideoHelpers_h
#define oVideoHelpers_h

#include <oooii/oVideoCodec.h>
#include <oooii/oErrno.h>

#define oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(_Type, _Mutex) \
	bool Map(MAPPED* _pMapped) threadsafe override \
	{ (_Mutex).Lock(); \
	bool result = thread_cast<_Type*>(this)->MapNOLOCK(_pMapped); \
	if (!result) (_Mutex).Unlock(); \
	return result && (_pMapped->DataSize > 0); \
	} \
	void Unmap() threadsafe override \
	{	thread_cast<_Type*>(this)->UnmapNOLOCK(); \
		(_Mutex).Unlock(); \
	}

#define oDECLARE_VIDEO_MAP_INTERFACE() \
	bool MapNOLOCK(MAPPED* _pMapped); \
	void UnmapNOLOCK()

inline void oVideoNullMapped(oVideoContainer::MAPPED* _pMapped)
{
	_pMapped->pFrameData = nullptr;
	_pMapped->DataSize = 0;
	_pMapped->DecodedFrameNumber = oINVALID_SIZE_T;
}

inline bool oVideoReturnEndOfFile(oVideoContainer::MAPPED* _pMapped)
{
	oSetLastError(ENOENT); // @oooii-tony: I kinda wish there were an EOF, but is it work adding EEOF? or ESTREAMEND? or EENUMEND for FindFiles/EnumFiles/EnumGPUs/etc.?
	oVideoNullMapped(_pMapped);
	return false;
}

#endif
