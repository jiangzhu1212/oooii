#pragma once
#ifndef oMOVFileImpl_h
#define oMOVFileImpl_h
#include <oooii/oVideoCodec.h>
#include <oooii/oRefCount.h>
#include <oooii/oByte.h>
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oMutex.h>
#include <oooii/oRef.h>
#include "oVideoHelpers.h"

struct oAtomHeader
{
	__int64 Size;
	char Type[4];
	__int64 DataOffset;
	__int64 EndDataOffset;
};

// For a good reference to the MOV container format look here. http://developer.apple.com/library/mac/#documentation/QuickTime/QTFF/QTFFPreface/qtffPreface.html#//apple_ref/doc/uid/TP40000939-CH202-TPXREF101
// That link does not include information on how to decode the data itself though. For a description of the rle compression byte stream see this. http://wiki.multimedia.cx/index.php?title=Apple_QuickTime_RLE
class oMOVFileImpl : public oVideoFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,threadsafe);
	oDEFINE_VIDEO_MUTEXED_MAP_INTERFACE(oMOVFileImpl, Mutex);
	oMOVFileImpl( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess);
	~oMOVFileImpl();

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	virtual bool HasFinished() const override
	{ 
		if(Desc.EndingFrame == -1)
			return CurrentFrame >= NumberOfFrames;
		else
			return CurrentFrame >= static_cast<unsigned int>(Desc.EndingFrame);
	}

	void Restart() threadsafe override { oMutex::ScopedLock lock(Mutex); CurrentFrame = 0;}

private:
	oDECLARE_VIDEO_MAP_INTERFACE();
	//_searchToken is a '.' delimited list of fourcc codes for the atom types. strings don't need to be null terminated.
	void FindAtoms(std::vector<oAtomHeader> &_results,const char *_searchToken,const oAtomHeader * const _startAtom = NULL);
	void FindAtoms(std::vector<oAtomHeader> &_results,const std::vector<const char*> &_searchTokens,unsigned int _searchIndex);
	bool ReadHeader(oAtomHeader &_header);
	template<typename T> bool ReadAtom(const oAtomHeader &_atom,T &_result);
	__int64 FTell() { return _ftelli64(File);}
	bool FSeek(__int64 _position, unsigned int _seekFrom = SEEK_SET)
	{ 
		if(_fseeki64(File,_position,_seekFrom) == 0)
			return true; 
		else 
			return false;
	}
	template<typename T> bool FRead(T& _item,unsigned int _numItems = 1)
	{
		if(fread_s(&_item,sizeof(T),1,_numItems*sizeof(T),File) == _numItems*sizeof(T) || feof(File))
			return true;
		else
			return false;
	}

	oRefCount RefCount;
	char FileName[_MAX_PATH];
	oVideoContainer::DESC Desc;
	FILE *File;
	//For rle compression, if a pixel hasn't changed from the previous frame, it may not be included in the next frame's data. So you need to keep
	//	the last frame around.
	bool Is64BitFile; //If the file is 4G are larger, the layout of somethings are different.
	__int64 FrameFileOffsets; //Offset into the file, for the table that contains the file offsets for each frame
	unsigned int NumberOfFrames; //Total number of frames in this file.
	unsigned int TimeScale; //Number of time units per second. For a 24fps test file this is 24000
	unsigned int Duration; //Duration of movie in time units (see TimeScale)
	unsigned int FrameDuration; //Number of Time units for a single frame. For a 24fps test file this is 1001 (i.e. 23.97FPS)
	unsigned int CurrentFrame; //The frame that will get decoded on the next call to Decode.
	oMutex Mutex;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	std::vector<unsigned long long> FileOffsetTable; //the file offset to each frame, in frame order
	std::vector<unsigned long long> SortedFileOffsetTable; // FileOffsetTable sorted by offset
};


#endif //oMOVFileImpl_h