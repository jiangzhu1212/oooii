// $(header)
#pragma once
#ifndef oWebmFile_h
#define oWebmFile_h
#include <oooii/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oRef.h>
#include <oooii/oMemoryMappedFile.h>
#include "oWebmStreaming.h"
#include "oVideoHelpers.h"

class oWebmFile : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(oWebmFile, Mutex);
	oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oWebmFile();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe;

	virtual void GetDesc(DESC* _pDesc) const threadsafe override
	{
		WebmStreaming->GetDesc(_pDesc);
	}

	bool HasFinished() const override { return Finished; }
	void Restart() threadsafe override { oMutex::ScopedLock lock(Mutex); FileIndex = DataStartIndex; Finished = false;}

private:
	oDECLARE_VIDEO_MAP_INTERFACE();

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