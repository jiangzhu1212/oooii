/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
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
