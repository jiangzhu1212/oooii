// $(header)
#pragma once
#ifndef oImageSequence_h
#define oImageSequence_h
#include <oooii/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include "oVideoHelpers.h"

class oImageSequence : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,threadsafe);
	oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(oImageSequence, Mutex);
	oImageSequence( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oImageSequence();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;
	virtual bool HasFinished() const override { return CurrentFrame >= ImageFiles.size();}
	void Restart() threadsafe override { oMutex::ScopedLock lock(Mutex); CurrentFrame = 0;}

private:
	oDECLARE_VIDEO_MAP_INTERFACE();

	struct ImageFile
	{
		ImageFile(char *_pFileName); //will modify the supplied string
		bool operator<(const ImageFile &_other)
		{
			return ImageNumber < _other.ImageNumber;
		}
		char FileName[_MAX_PATH];
		long long ImageNumber;
	};

	oRefCount RefCount;
	oVideoContainer::DESC Desc;
	unsigned int CurrentFrame; //The frame that will get decoded on the next call to Decode.
	oMutex Mutex;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	std::vector<ImageFile> ImageFiles;
};


#endif //oImageSequence_h
