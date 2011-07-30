// $(header)
#pragma once
#ifndef oMOVFileImpl_h
#define oMOVFileImpl_h
#include <oVideo/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oByte.h>
#include <oooii/oFile.h>
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include "oVideoHelpers.h"

struct oAtomHeader
{
	unsigned long long Size;
	char Type[4];
	unsigned long long DataOffset;
	unsigned long long EndDataOffset;
};

// For a good reference to the MOV container format look here. http://developer.apple.com/library/mac/#documentation/QuickTime/QTFF/QTFFPreface/qtffPreface.html#//apple_ref/doc/uid/TP40000939-CH202-TPXREF101
// That link does not include information on how to decode the data itself though. For a description of the rle compression byte stream see this. http://wiki.multimedia.cx/index.php?title=Apple_QuickTime_RLE
class oMOVFileImpl : public oVideoFile
{
public:
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oGetGUID<oVideoFile>(), oGetGUID<oVideoContainer>() );
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);
	oDECLARE_VIDEO_MAP_INTERFACE();
	oMOVFileImpl( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oMOVFileImpl();

	virtual bool HasFinished() const override
	{ 
		if(Desc.EndingFrame == -1)
			return CurrentFrame >= NumberOfFrames;
		else
			return CurrentFrame >= static_cast<unsigned int>(Desc.EndingFrame);
	}

	void Restart() override { /*oMutex::ScopedLock lock(Mutex);*/ CurrentFrame = 0;}

private:
	//_searchToken is a '.' delimited list of fourcc codes for the atom types. strings don't need to be null terminated.
	void FindAtoms(std::vector<oAtomHeader> &_results,const char *_searchToken,const oAtomHeader * const _startAtom = NULL);
	void FindAtoms(std::vector<oAtomHeader> &_results,const std::vector<const char*> &_searchTokens,unsigned int _searchIndex);
	bool ReadHeader(oAtomHeader &_header);
	template<typename T> bool ReadAtom(const oAtomHeader &_atom,T &_result);
	
	// @oooii-tony: Careful of the semantics here... this returns true even for a 
	// no-read due to end-of-file.
	template<typename T> bool FRead(T& _item)
	{
		if (sizeof(_item) == oFile::FRead(&_item, sizeof(_item), sizeof(_item), File) || oGetLastError() == EEOF)
			return true;
		else
			return false;
	}

	oRefCount RefCount;
	char FileName[_MAX_PATH];
	oVideoContainer::DESC Desc;
	oFile::Handle File;
	//For rle compression, if a pixel hasn't changed from the previous frame, it may not be included in the next frame's data. So you need to keep
	//	the last frame around.
	bool Is64BitFile; //If the file is 4G are larger, the layout of somethings are different.
	unsigned long long FrameFileOffsets; //Offset into the file, for the table that contains the file offsets for each frame
	unsigned int NumberOfFrames; //Total number of frames in this file.
	unsigned int TimeScale; //Number of time units per second. For a 24fps test file this is 24000
	unsigned int Duration; //Duration of movie in time units (see TimeScale)
	unsigned int FrameDuration; //Number of Time units for a single frame. For a 24fps test file this is 1001 (i.e. 23.97FPS)
	unsigned int CurrentFrame; //The frame that will get decoded on the next call to Decode.
	
	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	std::vector<unsigned long long> FileOffsetTable; //the file offset to each frame, in frame order
	std::vector<unsigned long long> SortedFileOffsetTable; // FileOffsetTable sorted by offset
};


#endif //oMOVFileImpl_h