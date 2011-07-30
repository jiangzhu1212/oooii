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
#include <oVideo/oVideoCodec.h>
#include <oooii/oFile.h>
#include <oooii/oMemoryMappedFile.h>
#include "oWebmStreaming.h"

const unsigned int VIEW_SIZE = oMB(8); //frame should never be larger than this in the webm file, or even close. typically a frame is around 512K a frame for a 120Mb/s file.

const static char WebXHeader[] = { 'o', 'w' ,'e', 'b', 'x'};
const static unsigned int WebXVersion = 1; // not really used currently, but there if we need it.

class oWebXStream : public oVideoFile
{
public:
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oGetGUID<oVideoFile>(), oGetGUID<oVideoContainer>() );
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDECLARE_VIDEO_MAP_INTERFACE();
	oWebXStream( const char* _pFileName, const oVideoContainer::DESC& _Desc, unsigned long long _Offset, unsigned long long _Size, bool* _pSuccess);
	
	virtual void GetDesc(DESC* _pDesc) const override
	{
		WebmStreaming->GetDesc(_pDesc);
	}

	bool HasFinished() const override { return Finished; }
	void Restart() override { FileIndex = DataStartIndex; Finished = false;}

private:

	oRefCount RefCount;

	bool Finished;

	oRef<threadsafe oMemoryMappedFile> MemoryMappedFrame;
	oRef<oWebmStreaming> WebmStreaming;
	unsigned long long FileIndex;
	unsigned long long DataStartIndex;
	unsigned long long StreamOffset;
	unsigned long long StreamSize;
};

oWebXStream::oWebXStream( const char* _pFileName,  const oVideoContainer::DESC& _Desc, unsigned long long _Offset,
	unsigned long long _Size, bool* _pSuccess) : Finished(false), StreamSize(_Size), StreamOffset(_Offset)
{
	*_pSuccess = false;
	bool success = false;
	oWebmStreaming *tempWebmStreaming;
	oCONSTRUCT(&tempWebmStreaming, oWebmStreaming(_Desc, &success));
	*(&WebmStreaming) = tempWebmStreaming;
	*_pSuccess = success;
	if(!success)
		return;

	WebmStreaming->SetParseOneFrame(true);	

	if(!oMemoryMappedFile::Create( _pFileName, &MemoryMappedFrame))
		return;

	unsigned int sizeToMap = static_cast<unsigned int>(__min(StreamSize,VIEW_SIZE));
	void* view = MemoryMappedFrame->Map(StreamOffset, sizeToMap);
	WebmStreaming->PushByteStream(view, sizeToMap);
	MemoryMappedFrame->Unmap();
	FileIndex = WebmStreaming->GetDataConsumed();

	DataStartIndex = WebmStreaming->GetLastFrameIndex();

	*_pSuccess = true;
}

bool oWebXStream::Map(MAPPED* _pMapped)
{
	if (FileIndex >= StreamSize)
	{
		Finished = true;
		return oVideoReturnEndOfFile(_pMapped);
	}

	if (!WebmStreaming->HasEncodedFrame())
	{
		unsigned int sizeToMap = static_cast<unsigned int>(__min(StreamSize-FileIndex,VIEW_SIZE));
		void* view = MemoryMappedFrame->Map(StreamOffset + FileIndex,sizeToMap);
		WebmStreaming->PushByteStream(view,sizeToMap);
		FileIndex += WebmStreaming->GetDataConsumed();
		MemoryMappedFrame->Unmap();
	}
	WebmStreaming->Map(_pMapped);
	return true;
}

void oWebXStream::Unmap()
{
	WebmStreaming->Unmap();
	if (FileIndex >= MemoryMappedFrame->GetFileSize() && !WebmStreaming->HasEncodedFrame()) // end of file
	{
		Finished = true;
	}
}

bool oVideo::CreateWebXFile(const char** _SourcePaths, unsigned int _NumSourcePaths, const char *_DestPath, bool _StitchVertically, bool _DeleteSourceFiles)
{
	std::vector<oRef<threadsafe oMemoryMappedFile>> SourceFiles;
	SourceFiles.resize(_NumSourcePaths);
	for (unsigned int i = 0;i < _NumSourcePaths; ++i)
	{
		oMemoryMappedFile::Create(_SourcePaths[i], &SourceFiles[i]);
	}

	oFile::ScopedFile OutputFile(_DestPath, "wb");
	if(!OutputFile)
	{
		oSetLastError(EINVAL, "webx file %s can't be opened for write", _DestPath);
		return false;
	}

	oFile::FWrite(WebXHeader, sizeof(WebXHeader), OutputFile);
	oFile::FWrite(&WebXVersion, sizeof(WebXVersion), OutputFile);
	oFile::FWrite(&_StitchVertically, sizeof(_StitchVertically), OutputFile);
	oFile::FWrite(&_NumSourcePaths, sizeof(_NumSourcePaths), OutputFile);
	unsigned long long fileOffset = oFile::Tell(OutputFile) + _NumSourcePaths*sizeof(fileOffset);
	for (unsigned int i = 0;i < _NumSourcePaths; ++i)
	{
		oFile::FWrite(&fileOffset, sizeof(fileOffset), OutputFile);
		fileOffset += SourceFiles[i]->GetFileSize();
	}

	oFOREACH(oRef<threadsafe oMemoryMappedFile> &_source, SourceFiles)
	{
		void *data = _source->Map(0, 0);
		unsigned long long dataLeft = _source->GetFileSize();
		while(dataLeft)
		{
			size_t dataToWrite = (size_t)__min(oNumericLimits<size_t>::GetMax(), dataLeft);
			oFile::FWrite(data, dataToWrite, OutputFile);
			dataLeft -= dataToWrite;
		}
		_source->Unmap();
	}

	SourceFiles.clear();

	if(_DeleteSourceFiles)
	{
		for (unsigned int i = 0;i < _NumSourcePaths; ++i)
		{
			oFile::Delete(_SourcePaths[i]);
		}
	}
	return true;
}

bool oVideo::CreateWebXContainers( const char *_SourcePath, const oVideoContainer::DESC& _Desc, oFUNCTION<void (oVideoFile *_container)> _functor, bool *_pStitchVertically)
{
	oFile::ScopedFile webxFile(_SourcePath, "rb");

	if(!webxFile)
	{
		oSetLastError(EINVAL, "webx file %s can't be opened", _SourcePath);
		return false;
	}
	char Header[sizeof(WebXHeader)];
	if(oFile::FRead(Header, sizeof(Header), sizeof(Header), webxFile) != sizeof(WebXHeader))
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt webx header", _SourcePath);
		return false;
	}
	if(strncmp(Header, WebXHeader, sizeof(WebXHeader)) != 0)
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt webx header", _SourcePath);
		return false;
	}

	unsigned int Version;
	if(oFile::FRead(&Version, sizeof(Version), sizeof(Version), webxFile) != sizeof(Version))
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt version", _SourcePath);
		return false;
	}

	if(Version != WebXVersion)
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. wrong version, only version 1 currently supported", _SourcePath);
		return false;
	}

	bool StitchVertically;
	if(oFile::FRead(&StitchVertically, sizeof(StitchVertically), sizeof(StitchVertically), webxFile) != sizeof(StitchVertically))
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt flags", _SourcePath);
		return false;
	}
	if(_pStitchVertically)
	{
		*_pStitchVertically = StitchVertically;
	}

	unsigned int NumStreams;
	if(oFile::FRead(&NumStreams, sizeof(NumStreams), sizeof(NumStreams), webxFile) != sizeof(NumStreams))
	{
		oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt stream count", _SourcePath);
		return false;
	}

	std::vector<unsigned long long> streamOffsets;
	streamOffsets.resize(NumStreams);
	for (unsigned int i = 0;i < NumStreams;++i)
	{
		if(oFile::FRead(&streamOffsets[i], sizeof(unsigned long long), sizeof(unsigned long long), webxFile) != sizeof(unsigned long long))
		{
			oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt stream offsets", _SourcePath);
			return false;
		}
	}

	unsigned long long fileSize = oFile::GetSize(webxFile);
	for (unsigned int i = 0;i < NumStreams;++i)
	{
		if(streamOffsets[i] > fileSize)
		{
			oSetLastError(EINVAL, "file %s was not a valid webx file. corrupt stream offsets", _SourcePath);
			return false;
		}
	}
	
	streamOffsets.push_back(fileSize);
	for (unsigned int i = 0;i < NumStreams;++i)
	{
		oVideoFile *container;
		bool success = false;
		oCONSTRUCT( &container, oWebXStream( _SourcePath, _Desc, streamOffsets[i], streamOffsets[i+1] - streamOffsets[i], &success) );
		if(!success)
		{
			oSetLastError(EINVAL, "file %s: failed to create a webx stream", _SourcePath);
			return false;
		}
		_functor(container);
	}

	return true;
}