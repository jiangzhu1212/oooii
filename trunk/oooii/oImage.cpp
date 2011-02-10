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
#include "pch.h"
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oImage.h>
#include <oooii/oPath.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oStdio.h>
#include <oooii/oThreading.h>
#include <oooii/oWindows.h>
#include <FreeImage.h>

// {83CECF1C-316F-4ed4-9B20-4180B2ED4B4E}
const oGUID oIIDImage = { 0x83cecf1c, 0x316f, 0x4ed4, { 0x9b, 0x20, 0x41, 0x80, 0xb2, 0xed, 0x4b, 0x4e } };

oSurface::FORMAT GetSurfaceFormat(FIBITMAP* _pBitmap)
{
	oSurface::FORMAT format = oSurface::UNKNOWN;

	FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(_pBitmap);
	unsigned int bpp = FreeImage_GetBPP(_pBitmap);

	// only rgb(a) formats supported
	if (!(colorType == FIC_RGB || colorType == FIC_RGBALPHA))
		return oSurface::UNKNOWN;

	unsigned int redMask = FreeImage_GetRedMask(_pBitmap);
	//unsigned int greenMask = FreeImage_GetRedMask(_pBitmap);
	//unsigned int blueMask = FreeImage_GetRedMask(_pBitmap);

	bool isBGRA = redMask == FI_RGBA_RED_MASK;

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
				format = isBGRA ? oSurface::B8G8R8A8_UNORM : oSurface::R8G8B8A8_UNORM;
			//else
			//	format = isBGRA ? oSurface::B8G8R8_UNORM : oSurface::R8G8B8_UNORM;
			break;

		case FIT_UINT16:
			break;
		case FIT_INT16:
			break;
		case FIT_UINT32:
			break;
		case FIT_INT32:
			//R10G10B10A2_TYPELESS,
			//R10G10B10A2_UNORM,
			//R10G10B10A2_UINT,
			//R10G10B10_XR_BIAS_A2_UNORM,
			break;
		case FIT_FLOAT:
			break;
		case FIT_RGB16:
			break;
		case FIT_RGBA16:
			//R16G16B16A16_TYPELESS,
			//R16G16B16A16_FLOAT,
			//R16G16B16A16_UNORM,
			//R16G16B16A16_UINT,
			//R16G16B16A16_SNORM,
			//R16G16B16A16_SINT,
			break;
		case FIT_RGBF:
			//R32G32B32_TYPELESS,
			//R32G32B32_FLOAT,
			//R32G32B32_UINT,
			//R32G32B32_SINT,
			break;
		case FIT_RGBAF:
			//R32G32B32A32_TYPELESS,
			//R32G32B32A32_FLOAT,
			//R32G32B32A32_UINT,
			//R32G32B32A32_SINT,
			break;

		default: break;
	}

	return format;
}

int GetSaveFlags(FREE_IMAGE_FORMAT _Format, oImage::COMPRESSION _Compression)
{
	switch (_Format)
	{
		case FIF_BMP:
		{
			switch (_Compression)
			{
				case oImage::NONE: return BMP_DEFAULT;
				case oImage::LOW:
				case oImage::MEDIUM:
				case oImage::HIGH: return BMP_SAVE_RLE;
				default: oASSUME(0);
			}

			break;
		}

		case FIF_JPEG:
		{
			switch (_Compression)
			{
				case oImage::NONE: return JPEG_QUALITYSUPERB;
				case oImage::LOW: return JPEG_DEFAULT;
				case oImage::MEDIUM: return JPEG_QUALITYNORMAL;
				case oImage::HIGH: return JPEG_QUALITYAVERAGE;
				default: oASSUME(0);
			}

			break;
		}

		case FIF_PNG:
		{
			switch (_Compression)
			{
				case oImage::NONE: return PNG_Z_NO_COMPRESSION;
				case oImage::LOW: return PNG_Z_BEST_SPEED;
				case oImage::MEDIUM: return PNG_Z_DEFAULT_COMPRESSION;
				case oImage::HIGH: return PNG_Z_BEST_COMPRESSION;
				default: oASSUME(0);
			}

			break;
		}

		default: break;
	}

	return 0;
}

struct FIStaticInitialization : public oSingleton<FIStaticInitialization>
{
	struct Run
	{
		Run() { FIStaticInitialization::Singleton(); }
	};

	bool IsInitialized;
	FIStaticInitialization()
	{
		FreeImage_Initialise();
		FreeImage_SetOutputMessage(FreeImageErrorHandler);
		IsInitialized = true;
	}

	~FIStaticInitialization()
	{
		IsInitialized = false;
		FreeImage_DeInitialise();
	}

	static void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
	{
		oMsgBox::printf(oMsgBox::ERR, "OOOii oImage Interface", "%s format image file: %s", FreeImage_GetFormatFromFIF(fif), message);
	}
};

FIStaticInitialization::Run sInitializeFreeImage; // @oooii-tony: ok static, we need to run this code exactly once per process, and oSingleton guarantees that.

struct oImage_Impl : public oImage
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oIIDImage);

	oImage_Impl(FIBITMAP* _bmp);
	~oImage_Impl();

	void FlipHorizontal() threadsafe override;
	void FlipVertical() threadsafe override;

	void GetDesc(DESC* _pDesc) const threadsafe override;

	void* GetData() threadsafe override;
	const void* GetData() threadsafe const override;

	bool Save(const char* _Path, COMPRESSION _Compression = NONE) threadsafe override;
	size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe override;
	
	#if defined(_WIN32) || defined(_WIN64)
		HBITMAP AsBmp() threadsafe override;
		HICON AsIco() threadsafe override;
	#endif

	FIBITMAP* bmp;
	oRefCount RefCount;
	oRWMutex Mutex;
};

oImage_Impl::oImage_Impl(FIBITMAP* _bmp)
	: bmp(_bmp)
{
	FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(_bmp);

	if (colorType == FIC_RGB)
	{
		bmp = FreeImage_ConvertTo32Bits(_bmp);
		FreeImage_Unload(_bmp);
	}

	oASSERT(FIStaticInitialization::Singleton()->IsInitialized, "FreeImage has not been initialized.");
}

oImage_Impl::~oImage_Impl()
{
	oASSERT(FIStaticInitialization::Singleton()->IsInitialized, "FreeImage has not been initialized.");
	if (bmp)
		FreeImage_Unload(bmp);
}

void oImage_Impl::FlipHorizontal() threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	#ifdef _DEBUG
		BOOL successfulFlipHorizontal = 
	#endif
	FreeImage_FlipHorizontal(bmp);

	oASSERT(successfulFlipHorizontal, "");
}

void oImage_Impl::FlipVertical() threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	#ifdef _DEBUG
		BOOL successfulFlipVertical = 
	#endif
	FreeImage_FlipVertical(bmp);
	oASSERT(successfulFlipVertical, "");
}

void oImage_Impl::GetDesc(DESC* _pDesc) const threadsafe
{
	oRWMutex::ScopedLockRead lock(Mutex);

	_pDesc->Pitch = FreeImage_GetPitch(bmp);
	_pDesc->Width = FreeImage_GetWidth(bmp);
	_pDesc->Height = FreeImage_GetHeight(bmp);
	_pDesc->Format = GetSurfaceFormat(bmp);

	oSurface::DESC desc;
	desc.RowPitch = _pDesc->Pitch;
	desc.Width = _pDesc->Width;
	desc.Height = _pDesc->Height;
	desc.NumSlices = 1;
	desc.NumMips = 1;
	desc.SlicePitch = 0;
	desc.Format = _pDesc->Format;
	_pDesc->Size = oSurface::CalculateSize(&desc);
}

void* oImage_Impl::GetData() threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	return FreeImage_GetBits(bmp);
}

const void* oImage_Impl::GetData() const threadsafe
{
	oRWMutex::ScopedLockRead lock(Mutex);
	return FreeImage_GetBits(bmp);
}

bool oImage_Impl::Save(const char* _Path, COMPRESSION _Compression) threadsafe
{
	if (!_Path || !*_Path)
	{
		oSetLastError(EINVAL);
		return false;
	}

	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(_Path);
	if (format == FIF_UNKNOWN)
	{
		oSetLastError(EINVAL);
		return false;
	}

	char folder[_MAX_PATH];
	strcpy_s(folder, _Path);
	oTrimFilename(folder);

	if (!oFile::CreateFolder(folder) && oGetLastError() != EEXIST)
	{
		oSetLastError(ENOENT, "Folder %s could not be created", folder);
		return false;
	}

	oRWMutex::ScopedLockRead lock(Mutex);
	if (!FreeImage_Save(format, bmp, _Path, GetSaveFlags(format, _Compression)))
	{
		oSetLastError(EIO, "Failed to save %s", _Path);
		return false;
	}

	return true;
}

size_t oImage_Impl::Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression) threadsafe
{
	if (!_Path || !*_Path)
	{
		oSetLastError(EINVAL);
		return 0;
	}

	FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(_Path);
	if (format == FIF_UNKNOWN)
	{
		oSetLastError(EINVAL);
		return 0;
	}

	oASSERT((size_t)((DWORD)_SizeofBuffer) == _SizeofBuffer, "Size of buffer too large for underlying implementation.");
	FIMEMORY* pMemory = FreeImage_OpenMemory(static_cast<BYTE*>(_pBuffer), static_cast<DWORD>(_SizeofBuffer));

	size_t written = 0;

	oRWMutex::ScopedLockRead lock(Mutex);

	if (FreeImage_SaveToMemory(format, bmp, pMemory, GetSaveFlags(format, _Compression)))
		written = FreeImage_TellMemory(pMemory);
	else
		oSetLastError(EIO, "Failed to save %s to memory", _Path);

	FreeImage_CloseMemory(pMemory);

	return written;
}

#if defined(_WIN32) || defined(_WIN64)
	#include <oooii/oWindows.h>

	HBITMAP oImage_Impl::AsBmp() threadsafe
	{
		oRWMutex::ScopedLockRead lock(Mutex);
		return CreateDIBitmap(GetDC(0), FreeImage_GetInfoHeader(bmp), CBM_INIT, FreeImage_GetBits(bmp), FreeImage_GetInfo(bmp), DIB_RGB_COLORS);
	}

	HICON oImage_Impl::AsIco() threadsafe
	{
		oRWMutex::ScopedLockRead lock(Mutex);
		HBITMAP hBmp = CreateDIBitmap(GetDC(0), FreeImage_GetInfoHeader(bmp), CBM_INIT, FreeImage_GetBits(bmp), FreeImage_GetInfo(bmp), DIB_RGB_COLORS);
		HICON hIcon = oIconFromBitmap(hBmp);
		DeleteObject(hBmp);
		return hIcon;
	}
#endif

static FIBITMAP* FILoadFromMemory(const void* _pBuffer, size_t _SizeofBuffer)
{
	FIBITMAP* bmp = 0;
	if (_pBuffer && _SizeofBuffer)
	{
		FIMEMORY* m = FreeImage_OpenMemory((BYTE*)_pBuffer, (DWORD)_SizeofBuffer);
		if (m)
		{
			FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(m, 0);
			if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
				bmp = FreeImage_LoadFromMemory(fif, m, 0);

			FreeImage_CloseMemory(m);
		}
	}

	return bmp;
}
	
static FIBITMAP* FICreate(const oImage::DESC* _pDesc)
{
	int bpp = 0;
	unsigned int r = 0, g = 0, b = 0;

	switch (_pDesc->Format)
	{
		case oSurface::R8G8B8A8_UNORM:
			bpp = static_cast<int>(oSurface::GetBitSize(_pDesc->Format));
			r = FI_RGBA_RED_MASK;
			g = FI_RGBA_GREEN_MASK;
			b = FI_RGBA_BLUE_MASK;
			break;

		default:
			return 0;
	}

	return FreeImage_Allocate(_pDesc->Width, _pDesc->Height, bpp, r, g, b);
}

bool oImage::Create(const void* _pBuffer, size_t _SizeofBuffer, oImage** _ppImage)
{
	if (!_pBuffer || !_SizeofBuffer || !_ppImage) return false;
	FIBITMAP* bmp = FILoadFromMemory(_pBuffer, _SizeofBuffer);
	*_ppImage = bmp ? new oImage_Impl(bmp) : 0;
	return !!*_ppImage;
}

bool oImage::Create(const DESC* _pDesc, oImage** _ppImage)
{
	if (!_pDesc || !_ppImage) return false;
	FIBITMAP* bmp = FICreate(_pDesc);
	*_ppImage = bmp ? new oImage_Impl(bmp) : 0;
	return !!*_ppImage;
}
