/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// This function contains wrappers for the FreeImage middleware API to simplify
// common tasks and conversions to Ouroboros lib types.
// THIS SHOULD ONLY BE USE IN THE IMPLEMENTATION OF oImage, NEVER DIRECTLY.
#pragma once
#ifndef oFreeImageHelpers_h
#define oFreeImageHelpers_h

#include <oStd/macros.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSingleton.h>
#include <freeimage.h>

class StaticInit : public oModuleSingleton<StaticInit>
{
public:
	StaticInit();
	~StaticInit();

	static void FreeImageErrorHandler(FREE_IMAGE_FORMAT _fif, const char* _Message);

private:
	bool IsInitialized;
};

// returns true if the ordering is BGR, false if RGB. Alpha or no alpha result
// in the same interpretation: true if ABGR, false if ARGB (I haven't run across
// the RGBA or BGRA situation yet).
bool FIIsBGR(FIBITMAP* _bmp);

// Returns size of the buffer returned from FreeImage_GetBits()
size_t FICalculateSize(FIBITMAP* _bmp);

// Allocate an FIBITMAP according to an oImage DESC. Use FreeImage_Unload when
// finished with the FIBITMAP.
FIBITMAP* FIAllocate(const oImage::DESC& _Desc);

FIBITMAP* FILoad(const void* _pBuffer, size_t _SizeofBuffer, bool _LoadBitmap = true);

// Copies to a Windows-compatible DIB useful in GDI-type APIs. Use 
// DeleteObject() when finished with the HBITMAP.
HBITMAP FIAllocateBMP(FIBITMAP* _bmp);

// Copies to a FreeImage bitmap from a Windows-compatible DIB as used in 
// GDI-type APIs. Use FreeImage_Unload() when finished with the return bitmap.
// _hBmp remains valid after this call.
FIBITMAP* FIAllocateFIBITMAP(HBITMAP _hBmp);

// Copies the pixel data from an HBITMAP to an FIBITMAP of exactly the same
// topology. This isn't all that robust, so if you get it wrong there will 
// probably be memory corruption.
void FICopyBits(FIBITMAP* _DstFIBitmap, HBITMAP _SrcHBmp);

oImage::FORMAT FIGetImageFormat(FIBITMAP* _pBitmap);

// Returns the proper FreeImage save flags for the specified compression level
// for each FreeImage file format.
int FIGetSaveFlags(FREE_IMAGE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel);

FREE_IMAGE_FORMAT FIToFIF(oImage::FILE_FORMAT _Format);
oImage::FILE_FORMAT FIFromFIF(FREE_IMAGE_FORMAT _fif);

// Fills the specified desc with information about the specified FIBITMAP.
void FIGetDesc(FIBITMAP* _bmp, oImage::DESC* _pDesc);

#endif
