#include "oVideoCache.h"
#include <oooii/oStdio.h>
#include <oooii/oMemoryMappedFile.h>
#include <snappy.h>

oVideoCache::oVideoCache(char *_file, unsigned int _height) : Height(_height)
{
	if(!oGetSysPath(CacheFileName, oSYSPATH_APP))
	{
		oSetLastError(EINVAL, "Failed to find the exe path");
	}
	strcat_s(CacheFileName, _file);
	
	if (0 != fopen_s(&CacheFile, CacheFileName, "wb"))
	{
		errno_t err = 0;
		_get_errno(&err);
		oSetLastError(err, "Failed to open %s", CacheFileName);
		CacheFile = nullptr;
	}
	WorkEvent.Set();
}

oVideoCache::~oVideoCache()
{
	WorkEvent.Wait();

	if (CacheFile)
		fclose(CacheFile);
	ReadbackMap = nullptr;
	oFile::Delete(CacheFileName);
}

void oVideoCache::CacheFrame(oSurface::YUV420 &_data)
{
	WorkEvent.Wait();

	unsigned int YSize = oSize32(_data.YPitch * Height);
	unsigned int UVSize = oSize32(_data.UVPitch * Height/2);
	
	char *data[3];
	unsigned int sizes[3];
	data[0] = (char*)_data.pY;
	data[1] = (char*)_data.pU;
	data[2] = (char*)_data.pV;
	sizes[0] = YSize;
	sizes[1] = UVSize;
	sizes[2] = UVSize;

	oParallelFor(oBIND(&oVideoCache::CompressFrame, this, oBIND1, data, sizes), 0, 3);

	WorkEvent.Reset();

	oIssueAsyncTask(oBIND(&oVideoCache::WriteCacheTask, this));
}

void oVideoCache::CompressFrame(size_t _index, char *_data[3], unsigned int _sizes[3])
{
	size_t compSize = snappy::MaxCompressedLength(_sizes[_index]);	
	WorkingMemory[_index].resize(compSize);
	snappy::RawCompress(_data[_index], _sizes[_index], (char*)&WorkingMemory[_index][0], &compSize);	
	CompressedSizes[_index] = oSize32(compSize);
}

void oVideoCache::WriteCacheTask()
{
	long long YFileOffset = _ftelli64(CacheFile);
	fwrite(&WorkingMemory[0][0], sizeof(unsigned char), CompressedSizes[0], CacheFile);

	long long UFileOffset = _ftelli64(CacheFile);
	fwrite(&WorkingMemory[1][0], sizeof(unsigned char), CompressedSizes[1], CacheFile);

	long long VFileOffset = _ftelli64(CacheFile);
	fwrite(&WorkingMemory[2][0], sizeof(unsigned char), CompressedSizes[2], CacheFile);

	CacheFrameOffsets.push_back(FrameOffset(YFileOffset, UFileOffset, VFileOffset));

	WorkEvent.Set();
}

void oVideoCache::StartSecondPass()
{
	WorkEvent.Wait();

	if (CacheFile)
		fclose(CacheFile);
	CacheFile = nullptr;
	CurrentFrame = 0;

	if(!oMemoryMappedFile::Create(CacheFileName, &ReadbackMap ))
	{
		oSetLastError(EINVAL, "Could not map the cache file %s", CacheFileName);
	}
	if(!HasFinished())
	{
		WorkEvent.Reset();
		oIssueAsyncTask(oBIND(&oVideoCache::ReadCompressedTask, this));
	}
}

void oVideoCache::ReadBackFrame(oSurface::YUV420 &_data)
{
	WorkEvent.Wait();
	
	if(HasFinished())
		return;

	unsigned int mapSize = static_cast<unsigned int>(ReadbackMap->GetFileSize() - CacheFrameOffsets[CurrentFrame].Y);
	if(CurrentFrame+1 < CacheFrameOffsets.size())
		mapSize = static_cast<unsigned int>(CacheFrameOffsets[CurrentFrame+1].Y - CacheFrameOffsets[CurrentFrame].Y);
	unsigned int compressedYSize = static_cast<unsigned int>(CacheFrameOffsets[CurrentFrame].U - CacheFrameOffsets[CurrentFrame].Y);
	unsigned int compressedUSize = static_cast<unsigned int>(CacheFrameOffsets[CurrentFrame].V - CacheFrameOffsets[CurrentFrame].U);
	unsigned int compressedVSize = static_cast<unsigned int>(mapSize - compressedYSize - compressedUSize);

	char *compressedY = (char*)&WorkingMemory[0][0];
	char *compressedU = compressedY + compressedYSize;
	char *compressedV = compressedU + compressedUSize;

	snappy::RawUncompress(compressedY, compressedYSize, (char*)_data.pY);
	snappy::RawUncompress(compressedU, compressedUSize, (char*)_data.pU);
	snappy::RawUncompress(compressedV, compressedVSize, (char*)_data.pV);
	
	++CurrentFrame;

	if(!HasFinished())
	{
		WorkEvent.Reset();
		oIssueAsyncTask(oBIND(&oVideoCache::ReadCompressedTask, this));
	}
}

void oVideoCache::ReadCompressedTask()
{
	unsigned int mapSize = static_cast<unsigned int>(ReadbackMap->GetFileSize() - CacheFrameOffsets[CurrentFrame].Y);
	if(CurrentFrame+1 < CacheFrameOffsets.size())
		mapSize = static_cast<unsigned int>(CacheFrameOffsets[CurrentFrame+1].Y - CacheFrameOffsets[CurrentFrame].Y);
	
	unsigned char *compressed = (unsigned char*)ReadbackMap->Map(CacheFrameOffsets[CurrentFrame].Y, mapSize);

	WorkingMemory[0].resize(mapSize);
	memcpy(&WorkingMemory[0][0], compressed, mapSize);

	ReadbackMap->Unmap();

	WorkEvent.Set();
}