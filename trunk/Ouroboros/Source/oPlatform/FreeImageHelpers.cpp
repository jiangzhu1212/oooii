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
#include "FreeImageHelpers.h"

StaticInit::StaticInit()
{
	FreeImage_Initialise();
	FreeImage_SetOutputMessage(FreeImageErrorHandler);
	IsInitialized = true;
}

StaticInit::~StaticInit()
{
	IsInitialized = false;
	FreeImage_DeInitialise();
}

void StaticInit::FreeImageErrorHandler(FREE_IMAGE_FORMAT _fif, const char* _Message)
{
	oASSERT(false, "%s format image file: %s", FreeImage_GetFormatFromFIF(_fif), _Message);
}

// Call this to ensure FreeImage is initialized. Some image code may be called
// during static init, so this function needs to be called from every 
// FreeImage-using function as well as during static init explicitly for the 
// case where static init image code is not called.
static void FIEnsureInit()
{
	StaticInit::Singleton();
}

struct FIRunAtStaticInitTime { FIRunAtStaticInitTime() { FIEnsureInit(); } };

static FIRunAtStaticInitTime FIInit; // gets around false-positive leak reports by incurring the singleton init at app boot time rather than on first use

bool FIIsBGR(FIBITMAP* _bmp)
{
	FIEnsureInit();
	// BGRA (or ARGB) = 0xAARRGGBB, RGBA (or ABGR) = 0xRRGGBBAA
	//unsigned int redMask = FreeImage_GetRedMask(_bmp);
	//return redMask == 0x00FF0000;

	//When upgrading from from free image 3.15.1 to 3.15.4, FreeImage_GetRedMask now always returns 0 for all images except 16 bit 555 565 types.
	//so for now always return true here until we think of something better.
	return true;
}

size_t FICalculateSize(FIBITMAP* _bmp)
{
	FIEnsureInit();
	return FreeImage_GetPitch(_bmp) * FreeImage_GetHeight(_bmp);
}

static void FIGetAllocateParams(const oImage::DESC& _Desc, int* pBpp, unsigned int* pRedMask, unsigned int* pGreenMask, unsigned int* pBlueMask)
{
	*pBpp = oImageGetBitSize(_Desc.Format);
	*pRedMask = *pGreenMask = *pBlueMask = 0;
	switch (_Desc.Format)
	{
		case oImage::BGR24:
		case oImage::BGRA32:
			*pRedMask = FI_RGBA_RED_MASK;
			*pGreenMask = FI_RGBA_GREEN_MASK;
			*pBlueMask = FI_RGBA_BLUE_MASK;
			break;

		default:
			break;
	}
}

FIBITMAP* FIAllocate(const oImage::DESC& _Desc)
{
	int bpp;
	unsigned int r,g,b;
	FIGetAllocateParams(_Desc, &bpp, &r, &g, &b);
	FIEnsureInit();
	return FreeImage_Allocate(_Desc.Dimensions.x, _Desc.Dimensions.y, bpp, r, g, b);
}

FIBITMAP* FILoad(const void* _pBuffer, size_t _SizeofBuffer, bool _LoadBitmap)
{
	FIBITMAP* bmp = nullptr;
	if (_pBuffer && _SizeofBuffer)
	{
		FIEnsureInit();
		FIMEMORY* m = FreeImage_OpenMemory((BYTE*)_pBuffer, (DWORD)_SizeofBuffer);
		if (m)
		{
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(m, 0);
			if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
				bmp = FreeImage_LoadFromMemory(fif, m, _LoadBitmap ? 0 : FIF_LOAD_NOPIXELS);

			if(_LoadBitmap)
			{
				FreeImage_FlipVertical(bmp);
			}

			// NOTE: This oWARN below should not trigger anymore because FreeImage 
			// source has been modified to not absorb metadata. Maybe newer FreeImage
			// versions will have fixed the leak, but for now we're not using metadata
			// so losing this load functionality won't affect OOOii code.
			#ifdef _DEBUG
				for (int i = FIMD_COMMENTS; i <= FIMD_CUSTOM; i++)
					if(FreeImage_GetMetadataCount((FREE_IMAGE_MDMODEL)i, bmp))
					{
						oWARN_ONCE("An oImage contains metadata. FreeImage will leak when unloading this image.");
						break;
					}
			#endif

			FreeImage_CloseMemory(m);
		}
	}

	return bmp;
}

HBITMAP FIAllocateBMP(FIBITMAP* _bmp)
{
	HDC hDC = GetDC(0);
	FIBITMAP* clone = FreeImage_Clone(_bmp);
	FreeImage_FlipVertical(clone); //HBitmaps are upside down.
	HBITMAP hBmp = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(clone), CBM_INIT, FreeImage_GetBits(clone), FreeImage_GetInfo(clone), DIB_RGB_COLORS);
	FreeImage_Unload(clone);
	ReleaseDC(NULL, hDC);
	return hBmp;
}

void FICopyBits(FIBITMAP* _DstFIBitmap, HBITMAP _SrcHBmp)
{
	// The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (don't know why) 
	// So we save these infos below. This is needed for palletized images only. 
	int nColors = FreeImage_GetColorsUsed(_DstFIBitmap);
	HDC hDC = GetDC(NULL);
	int Success = GetDIBits(hDC, _SrcHBmp, 0, FreeImage_GetHeight(_DstFIBitmap), FreeImage_GetBits(_DstFIBitmap), FreeImage_GetInfo(_DstFIBitmap), DIB_RGB_COLORS);
	ReleaseDC(NULL, hDC);
	// restore BITMAPINFO members
	FreeImage_GetInfoHeader(_DstFIBitmap)->biClrUsed = nColors;
	FreeImage_GetInfoHeader(_DstFIBitmap)->biClrImportant = nColors;

	FreeImage_FlipVertical(_DstFIBitmap); //HBitmaps are upside down.
}

FIBITMAP* FIAllocateFIBITMAP(HBITMAP _hBmp)
{
	BITMAP bm;
	GetObject(_hBmp, sizeof(BITMAP), (LPSTR)&bm);
	FIBITMAP* bmp = FreeImage_Allocate(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel);
	FICopyBits(bmp, _hBmp);
	return bmp;
}

oImage::FORMAT FIGetImageFormat(FIBITMAP* _pBitmap)
{
	oImage::FORMAT format = oImage::UNKNOWN;
	if (_pBitmap)
	{
		FIEnsureInit();
		FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(_pBitmap);
		unsigned int bpp = FreeImage_GetBPP(_pBitmap);

		// only rgb(a) formats supported
		if (!(colorType == FIC_RGB || colorType == FIC_RGBALPHA))
			return oImage::UNKNOWN;

		bool isBGR = FIIsBGR(_pBitmap);
		bool hasAlpha = bpp == 32 || colorType == FIC_RGBALPHA;
		FREE_IMAGE_TYPE type = FreeImage_GetImageType(_pBitmap);
		//unsigned int bpp = FreeImage_GetBPP(_pBitmap);
		//unsigned int used = FreeImage_GetColorsUsed(_pBitmap);

		// @oooii-tony: this will be incrementally added to as we have real-world 
		// test cases to work against.
		switch (type)
		{
		case FIT_BITMAP:
			if (hasAlpha)
				format = isBGR ? oImage::BGRA32 : oImage::RGBA32;
			else
				format = isBGR ? oImage::BGR24 : oImage::RGB24;
			break;

		default: break;
		}
	}

	return format;
}

int FIGetSaveFlags(FREE_IMAGE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel)
{
	switch (_Format)
	{
		case FIF_DDS:
			return 0;

		case FIF_BMP:
		{
			switch (_CompressionLevel)
			{
				case oImage::NO_COMPRESSION: return BMP_DEFAULT;
				case oImage::LOW_COMPRESSION:
				case oImage::MEDIUM_COMPRESSION:
				case oImage::HIGH_COMPRESSION: return BMP_SAVE_RLE;
				oNODEFAULT;
			}
			break;
		}

		case FIF_JPEG:
		{
			switch (_CompressionLevel)
			{
				case oImage::NO_COMPRESSION: return JPEG_QUALITYSUPERB;
				case oImage::LOW_COMPRESSION: return JPEG_DEFAULT;
				case oImage::MEDIUM_COMPRESSION: return JPEG_QUALITYNORMAL;
				case oImage::HIGH_COMPRESSION: return JPEG_QUALITYAVERAGE;
				oNODEFAULT;
			}
			break;
		}

		case FIF_PNG:
		{
			switch (_CompressionLevel)
			{
				case oImage::NO_COMPRESSION: return PNG_Z_NO_COMPRESSION;
				case oImage::LOW_COMPRESSION: return PNG_Z_BEST_SPEED;
				case oImage::MEDIUM_COMPRESSION: return PNG_Z_DEFAULT_COMPRESSION;
				case oImage::HIGH_COMPRESSION: return PNG_Z_BEST_COMPRESSION;
				oNODEFAULT;
			}
			break;
		}

		default:
			break;
	}

	return 0;
}

FREE_IMAGE_FORMAT FIToFIF(oImage::FILE_FORMAT _Format)
{
	switch (_Format)
	{
		case oImage::ICO: return FIF_ICO;
		case oImage::BMP: return FIF_BMP;
		case oImage::JPG: return FIF_JPEG;
		case oImage::PNG: return FIF_PNG;
		case oImage::DDS: return FIF_DDS;
		default: return FIF_UNKNOWN;
	}
}

oImage::FILE_FORMAT FIFromFIF(FREE_IMAGE_FORMAT _fif)
{
	switch (_fif)
	{
		case FIF_ICO: return oImage::ICO;
		case FIF_BMP: return oImage::BMP;
		case FIF_JPEG: return oImage::JPG;
		case FIF_PNG: return oImage::PNG;
		case FIF_DDS: return oImage::DDS;
		default: return oImage::UNKNOWN_FILE;
	}
}

void FIGetDesc(FIBITMAP* _bmp, oImage::DESC* _pDesc)
{
	oASSERT(_bmp && _pDesc, "");
	FIEnsureInit();
	_pDesc->RowPitch = FreeImage_GetPitch(_bmp);
	_pDesc->Dimensions.x = FreeImage_GetWidth(_bmp);
	_pDesc->Dimensions.y = FreeImage_GetHeight(_bmp);
	_pDesc->Format = FIGetImageFormat(_bmp);
}
