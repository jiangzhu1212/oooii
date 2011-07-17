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

bool oWebmFile::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	if (_InterfaceID == oGetGUID<oVideoContainer>())
	{
		Reference();
		*_ppInterface = this;
		return true;
	}

	oSetLastError(ENOENT);
	return false;
}

bool oWebmFile::MapNOLOCK(MAPPED* _pMapped)
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

void oWebmFile::UnmapNOLOCK()
{
	WebmStreaming->Unmap();
	if (FileIndex >= MemoryMappedFrame->GetFileSize() && !WebmStreaming->HasEncodedFrame()) // end of file
	{
		Finished = true;
	}
}