// Used for two pass encoding to avoid having to read a png, decode, and chromo downsample it twice. This just stores raw
//	YUV420 data in the cache file, the index into the file is held in memory. 
#pragma once
#ifndef oVideoCache_h
#define oVideoCache_h

interface oMemoryMappedFile;

class oVideoCache
{
public:
	oVideoCache(char *_file, unsigned int _height);
	~oVideoCache();

	void CacheFrame(oSurface::YUV420 &_data);
	void StartSecondPass();
	void ReadBackFrame(oSurface::YUV420 &_data);
	bool HasFinished() const {return CurrentFrame >= CacheFrameOffsets.size();}

private:
	void CompressFrame(size_t _index, char *_data[3], unsigned int _sizes[3]);
	void WriteCacheTask();
	void ReadCompressedTask();

	unsigned int Height;
	char CacheFileName[_MAX_PATH];
	FILE *CacheFile;
	struct FrameOffset
	{
		FrameOffset(long long _y, long long _u, long long _v) : Y(_y), U(_u), V(_v) {}
		long long Y, U, V;
	};
	std::vector<FrameOffset> CacheFrameOffsets;

	std::vector<unsigned char> WorkingMemory[3];
	unsigned int CompressedSizes[3];
	oRef<threadsafe oMemoryMappedFile> ReadbackMap;
	unsigned int CurrentFrame;
	oEvent WorkEvent;
};

#endif
