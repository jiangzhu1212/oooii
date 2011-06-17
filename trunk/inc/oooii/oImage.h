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
			: Format(oSurface::UNKNOWN)
			, Pitch(~0u)
			, Width(~0u)
			, Height(~0u)
			, Size(~0u)
		{}

		oSurface::FORMAT Format;
		unsigned int Pitch;
		unsigned int Width;
		unsigned int Height;
		unsigned int Size; // total size in bytes of the image bit data
	};

	// Creates an Image from an in-memory image file
	static bool Create(const void* _pBuffer, size_t _SizeOfBuffer, oImage** _ppImage);

	// Creates an uninitialized IMG from the specified DESC.
	// Pitch is ignored as a creation parameter.
	static bool Create(const DESC* _pDesc, oImage** _ppImage);

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
	
	virtual bool GetAsRGBA(unsigned char* _pOutput, size_t _OutputSize) threadsafe = 0;

	#if defined(_WIN32) || defined(_WIN64)
		virtual struct HBITMAP__* AsBmp() threadsafe = 0; // Returns a copy (HBITMAP). Use DeleteObject() when finished with the HBITMAP.
	#endif
};

#endif
