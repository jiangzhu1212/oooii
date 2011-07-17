// $(header)
#pragma once
#ifndef oMemoryMappedFile_h
#define oMemoryMappedFile_h

#include <oooii/oInterface.h>
#include <memory.h>

//Currently only read only memory mapped files are supported. Any attempt to write
//	will throw an access violation. Also only one view can be mapped at a time. a second
//	call to MapView without unmapping the first will block until the first is unmapped.
//	these limitations could be fixed if needed.
interface oMemoryMappedFile : oInterface
{
	static bool Create(const char* _Path, threadsafe oMemoryMappedFile** _ppMappedFile);

	virtual unsigned long long GetFileSize() const threadsafe = 0;

	// If _Size is zero the entire file is mapped from _Offset to the end of the file
	virtual void* Map(unsigned long long _Offset, unsigned int _Size) threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
};

#endif
