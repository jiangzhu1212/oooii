// $(header)
#include <oooii/oStdio.h>
#include <oooii/oMath.h>
#include <oooii/oString.h>
#include <oooii/oSTL.h>
#include <oooii/oCPU.h>
#include "oWebmFile.h"

const unsigned int VIEW_SIZE = oMB(8); //frame should never be larger than this in the webm file, or even close. typically a frame is around 512K a frame for a 120Mb/s file.

oWebmFile::oWebmFile( const char* _pFileName, const oVideoContainer::DESC& _Desc, bool* _pSuccess) : Finished(false)
{
	*_pSuccess = false;
	// Create the streaming file
	{
		bool success = false;
		oWebmStreaming *tempWebmStreaming;
		oCONSTRUCT(&tempWebmStreaming, oWebmStreaming(_Desc, &success));
		*(&WebmStreaming) = tempWebmStreaming;
		if(!success)
			return;
	}

	WebmStreaming->SetParseOneFrame(true);	

	if(!oMemoryMappedFile::Create( _pFileName, &MemoryMappedFrame))
	{
		return; // Rely on oMemoryMappedFile to set the last error
	}

	unsigned int sizeToMap = static_cast<unsigned int>(__min(MemoryMappedFrame->GetFileSize(),VIEW_SIZE));
	void* view = MemoryMappedFrame->Map(0, sizeToMap);
	WebmStreaming->PushByteStream(view, sizeToMap);
	MemoryMappedFrame->Unmap();
	FileIndex = WebmStreaming->GetDataConsumed();

	DataStartIndex = WebmStreaming->GetLastFrameIndex();

	*_pSuccess = true;
}

oWebmFile::~oWebmFile()
{

}

bool oWebmFile::Map(MAPPED* _pMapped)
{
	if (FileIndex >= MemoryMappedFrame->GetFileSize())
	{
		Finished = true;
		return oVideoReturnEndOfFile(_pMapped);
	}

	if (!WebmStreaming->HasEncodedFrame())
	{
		unsigned int sizeToMap = static_cast<unsigned int>(__min(MemoryMappedFrame->GetFileSize()-FileIndex,VIEW_SIZE));
		void* view = MemoryMappedFrame->Map(FileIndex,sizeToMap);
		WebmStreaming->PushByteStream(view,sizeToMap);
		FileIndex += WebmStreaming->GetDataConsumed();
		MemoryMappedFrame->Unmap();
	}
	WebmStreaming->Map(_pMapped);
	return true;
}

void oWebmFile::Unmap()
{
	WebmStreaming->Unmap();
	if (FileIndex >= MemoryMappedFrame->GetFileSize() && !WebmStreaming->HasEncodedFrame()) // end of file
	{
		Finished = true;
	}
}