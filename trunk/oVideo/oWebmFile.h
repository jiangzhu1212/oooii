// $(header)
#pragma once
#ifndef oWebmFile_h
#define oWebmFile_h
#include <oVideo/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oThreading.h>
#include <oooii/oRef.h>
#include <oooii/oMemoryMappedFile.h>
#include "oWebmStreaming.h"
#include "oVideoHelpers.h"

class oWebmFile : public oVideoFile
{
public:
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oGetGUID<oVideoFile>(), oGetGUID<oVideoContainer>() );
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDECLARE_VIDEO_MAP_INTERFACE();
	oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oWebmFile();

	virtual void GetDesc(DESC* _pDesc) const override
	{
		WebmStreaming->GetDesc(_pDesc);
	}

	bool HasFinished() const override { return Finished; }
	void Restart() override { FileIndex = DataStartIndex; Finished = false;}

private:
	oRefCount RefCount;
	char FileName[_MAX_PATH];

	bool Finished;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	oRef<oWebmStreaming> WebmStreaming;
	unsigned long long FileIndex;
	unsigned long long DataStartIndex;
};


#endif //oWebmFile_h