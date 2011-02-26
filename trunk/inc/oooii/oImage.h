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
#pragma once
#ifndef oImage_h
#define oImage_h

#include <oooii/oInterface.h>
#include <oooii/oSurface.h>

#if defined(_WIN32) || defined(_WIN64)
// Forward declare windows symbols to avoid including <Windows.h>
typedef struct HBITMAP__ *HBITMAP;
typedef struct HICON__ *HICON;
#endif

interface oImage : public oInterface
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

	virtual void* GetData() threadsafe = 0;
	virtual const void* GetData() const threadsafe = 0;

	// Saves the image to the specified path. The file extension is used to 
	// determine the format. Returns the number of bytes written, or 0 if failed.
	// Check oGetLastError().
	virtual bool Save(const char* _Path, COMPRESSION _Compression = NONE) threadsafe = 0;

	// Saves the image to a memory buffer as if it were a file, the file extension 
	// is used to determine the format. Returns the number of bytes written, or 0
	// if failed. Check oGetLastError(). If _pBuffer is NULL, this still returns
	// the size requires for a successful save though nothing would be written.
	virtual size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe = 0;
	
	#if defined(_WIN32) || defined(_WIN64)
		virtual HBITMAP AsBmp() threadsafe = 0; // Returns a copy (typecast to HBITMAP). Use DeleteObject() when finished with the HBITMAP.
		virtual HICON AsIco() threadsafe = 0; // Returns a copy (typecast to HICON). Use DestroyIcon() when finished with the HICON.
	#endif
};

#endif
