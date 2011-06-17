#pragma once
#ifndef oWebmFile_h
#define oWebmFile_h
#include <oooii/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oRef.h>
#include <oooii/oMemoryMappedFile.h>
#include "oWebmStreaming.h"

class oWebmFile : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oWebmFile();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	virtual void GetDesc(DESC* _pDesc) const threadsafe override
	{
		WebmStreaming->GetDesc(_pDesc);
	}

	virtual bool HasFinished() const override { return Finished;}

	virtual void MapFrame(void** _ppFrameData, size_t* _szData, bool* _pValid, size_t *_decodedFrameNumber) threadsafe override
	{
		Mutex.Lock();
		thread_cast<oWebmFile*>(this)->MapFrameInternal(_ppFrameData,_szData,_pValid, _decodedFrameNumber);
	}
	virtual void UnmapFrame() threadsafe override
	{
		thread_cast<oWebmFile*>(this)->UnmapFrameInternal();
		Mutex.Unlock();
	}
	void Restart() threadsafe override { oMutex::ScopedLock lock(Mutex); FileIndex = DataStartIndex; Finished = false;}

private:
	void MapFrameInternal(void** _ppFrameData, size_t* _szData, bool* _pValid, size_t *_decodedFrameNumber);
	void UnmapFrameInternal();

	oRefCount RefCount;
	char FileName[_MAX_PATH];

	bool Finished;
	oMutex Mutex;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	oRef<threadsafe oWebmStreaming> WebmStreaming;
	unsigned long long FileIndex;
	unsigned long long DataStartIndex;
};


#endif //oWebmFile_h