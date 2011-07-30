// $(header)
#include <oooii/oMemoryMappedFile.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oHeap.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oMutex.h>
#include <oooii/oWindows.h>

const oGUID& oGetGUID( threadsafe const oMemoryMappedFile* threadsafe const * )
{
	// {F5541DD3-848F-426f-84DD-B48DE4EE0804}
	static const oGUID oIIDMemoryMappedFile = { 0xf5541dd3, 0x848f, 0x426f, { 0x84, 0xdd, 0xb4, 0x8d, 0xe4, 0xee, 0x8, 0x4 } };
	return oIIDMemoryMappedFile;
}

class oMemoryMappedFileImpl : public oMemoryMappedFile
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oMemoryMappedFile>());

	oMemoryMappedFileImpl(const char* _fileName, bool* _pSuccess);
	~oMemoryMappedFileImpl();

	virtual void* Map(unsigned long long _Offset, unsigned int _Size) threadsafe;
	virtual void Unmap() threadsafe;
	virtual unsigned long long GetFileSize() const threadsafe override { return FileSize; }

private:
	oRefCount RefCount;
	oMutex Mutex;
	unsigned int AllocationGran;

	HANDLE MemoryMappedFileHandle;
	HANDLE MemoryMappedMappingHandle;
	void *pMemoryMappedView;
	unsigned __int64 FileSize;
};

static bool Create(const char* _fileName, threadsafe oMemoryMappedFile** _ppBuffer);

bool oMemoryMappedFile::Create(const char* _Path, threadsafe oMemoryMappedFile** _ppMappedFile)
{
	bool success = false;
	oCONSTRUCT(_ppMappedFile, oMemoryMappedFileImpl(_Path, &success));
	return success;
}

oMemoryMappedFileImpl::oMemoryMappedFileImpl(const char* _fileName, bool* _pSuccess) : 
	MemoryMappedFileHandle(INVALID_HANDLE_VALUE), MemoryMappedMappingHandle(INVALID_HANDLE_VALUE),
	pMemoryMappedView(0)
{
	*_pSuccess = false;
	MemoryMappedFileHandle = CreateFile(_fileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
	if(MemoryMappedFileHandle == INVALID_HANDLE_VALUE)
	{
		oSetLastError(EINVAL,"%s: Couldn't open the file for memory mapping",_fileName);
		return;
	}
	MemoryMappedMappingHandle = CreateFileMapping(MemoryMappedFileHandle,0,PAGE_READONLY,0,0,0);
	if(MemoryMappedMappingHandle == INVALID_HANDLE_VALUE)
	{
		oSetLastError(EINVAL,"%s: Couldn't open the file for memory mapping",_fileName);
		return;
	}

	AllocationGran = oHeap::GetSystemAllocationGranularity();

	GetFileSizeEx(MemoryMappedFileHandle,(LARGE_INTEGER*)(&FileSize));

	*_pSuccess = true;
}

oMemoryMappedFileImpl::~oMemoryMappedFileImpl()
{
	oASSERT(!pMemoryMappedView,"destroyed a memory mapped file while there was still a view mapped");
	if (pMemoryMappedView)
		Unmap();
	if (MemoryMappedMappingHandle != INVALID_HANDLE_VALUE)
		CloseHandle(MemoryMappedMappingHandle);
	if (MemoryMappedFileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(MemoryMappedFileHandle);
}

void* oMemoryMappedFileImpl::Map(unsigned long long _Offset, unsigned int _Size) threadsafe
{
	Mutex.Lock();
	oASSERT(!pMemoryMappedView,"tried to map a frame that was already mapped");
	if (pMemoryMappedView)
		return 0;

	unsigned long long alignedFileOffset = oByteAlignDown(_Offset, AllocationGran);
	unsigned int padding = static_cast<unsigned int>(_Offset - alignedFileOffset);
	unsigned int mapSize = _Size + padding;
	pMemoryMappedView = MapViewOfFile(MemoryMappedMappingHandle, FILE_MAP_READ, alignedFileOffset>>32, alignedFileOffset&0xffffffff, mapSize);

	if (!pMemoryMappedView)
	{
		oSetLastError(GetLastError(),"could not map a view of the file");
		return 0;
	}
	return oByteAdd(pMemoryMappedView,padding);
}

void oMemoryMappedFileImpl::Unmap() threadsafe
{
	oASSERT(pMemoryMappedView, "tried to unmap a frame that was never mapped");
	if (pMemoryMappedView)
	{
		UnmapViewOfFile(pMemoryMappedView);
		pMemoryMappedView = 0;
	}
	Mutex.Unlock();
}