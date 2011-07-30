// $(header)
#pragma once
#ifndef oImageSequence_h
#define oImageSequence_h
#include <oVideo/oVideoCodec.h>
#include <oooii/oFile.h>
#include <oooii/oRefCount.h>
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include "oVideoHelpers.h"
#include <oooii/oEvent.h>

class oImageSequence : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);
	oDECLARE_VIDEO_MAP_INTERFACE();
	oImageSequence( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oImageSequence();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;
	virtual bool HasFinished() const override { return CurrentFrame >= ImageFiles.size();}
	void Restart() override;

private:
	void PrefetchTask(unsigned int _frame);
	unsigned int NextBuffer() const {return (CurrentBuffer+1)%2;}

	oRefCount RefCount;
	oVideoContainer::DESC Desc;
	unsigned int CurrentFrame; //The frame that will get decoded on the next call to Decode.

	struct ImageFile
	{
		ImageFile(const char* _Filename);
		bool operator<(const ImageFile &_other)
		{
			return ImageNumber < _other.ImageNumber;
		}
		char Filename[_MAX_PATH];
		long long ImageNumber;
		oSize64 FileSize;
	};

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	std::vector<ImageFile> ImageFiles;
	std::vector<unsigned char> PNGBuffer[2];
	int CurrentBuffer;

	oEvent IOEvent;

	static bool ListImageFiles(const char* _Path, const oFile::DESC& _Desc, const oImageSequence::DESC& _ISDesc, std::vector<ImageFile>& _ImageFiles);
};

#endif //oImageSequence_h
