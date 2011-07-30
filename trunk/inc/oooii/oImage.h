// $(header)
#pragma once
#ifndef oImage_h
#define oImage_h

#include <oooii/oInterface.h>
#include <oooii/oSurface.h>

struct oBuffer;

interface oImage : oInterface
{
	enum COMPRESSION
	{
		NONE,
		LOW,
		MEDIUM,
		HIGH,
	};

	struct DESC
	{
		DESC()
			: Format(oSurface::R8G8B8A8_UNORM)
			, Pitch(oINVALID)
			, Width(oINVALID)
			, Height(oINVALID)
			, Size(oINVALID)
		{}

		oSurface::FORMAT Format;
		unsigned int Pitch;
		unsigned int Width;
		unsigned int Height;
		unsigned int Size; // total size in bytes of the image bit data
	};

	// Creates an Image from an in-memory image file. If a valid format is 
	// specified that is different from the buffer, the image will be converted.
	// For whatever-the-file-specified use UNKNOWN. If _LoadBytes is false, then
	// only header information is loaded and no allocation is done until Map is
	// called. Thus, _pBuffer does not have to be the entire file, only enough of
	// the file to container the header.
	static bool Create(const void* _pBuffer, size_t _SizeOfBuffer, oSurface::FORMAT _Format, oImage** _ppImage);

	// Creates an uninitialized oImage from the specified DESC. Pitch is ignored 
	// as a creation parameter.
	static bool Create(const DESC* _pDesc, oImage** _ppImage);
		
	//replace the image data with an in-memory image file.
	virtual bool Update(const void* _pBuffer, size_t _SizeOfBuffer) threadsafe = 0;

	virtual void FlipHorizontal() threadsafe = 0;
	virtual void FlipVertical() threadsafe = 0;

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	virtual void* Map() threadsafe = 0;
	virtual const void* Map() const threadsafe = 0;

	virtual void Unmap() threadsafe = 0;
	virtual void Unmap() const threadsafe = 0;

	virtual bool Compare(const oImage* _pOtherImage, unsigned int _BitFuzziness, unsigned int* _pNumDifferingPixels = 0, oImage** _ppDiffImage = 0, unsigned int _DiffMultiplier = 1) = 0;

	// Saves the image to the specified path. The file extension is used to 
	// determine the format. Returns the number of bytes written, or 0 if failed.
	// Check oGetLastError().
	virtual bool Save(const char* _Path, COMPRESSION _Compression = NONE) threadsafe = 0;

	// Saves the image to a memory buffer as if it were a file, the file extension 
	// is used to determine the format. Returns the number of bytes written, or 0
	// if failed. Check oGetLastError(). If _pBuffer is NULL, this still returns
	// the size requires for a successful save though nothing would be written.
	virtual size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe = 0;
	
	virtual bool GetAsRGBA(unsigned char* _pOutput, size_t _OutputSize, bool _topDown = false) threadsafe = 0;

	virtual bool GetAsBGRA(unsigned char* _pOutput, size_t _OutputSize, bool _topDown = false) threadsafe = 0;

	static bool IsSupportedFileType(const char* _Path);

	#if defined(_WIN32) || defined(_WIN64)
		virtual struct HBITMAP__* AsBmp() threadsafe = 0; // Returns a copy (HBITMAP). Use DeleteObject() when finished with the HBITMAP.
	#endif
};

#endif
