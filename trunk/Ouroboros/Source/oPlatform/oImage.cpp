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
#include <oPlatform/oReporting.h>
#include <oBasis/oColor.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oError.h>
#include <oBasis/oMutex.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oStreamUtil.h>
#include <oPlatform/Windows/oWindows.h>
#include "FreeImageHelpers.h"

const char* oAsString(oImage::FORMAT _Format)
{
	switch (_Format)
	{
		case oImage::UNKNOWN: return "UNKNOWN";
		case oImage::RGBA32: return "RGBA32";
		case oImage::RGB24: return "RGB24";
		case oImage::BGRA32: return "BGRA32";
		case oImage::BGR24: return "BGR24";
		case oImage::R8: return "R8";
		oNODEFAULT;
	}
}

oImage::FORMAT oImageFormatFromSurfaceFormat(oSURFACE_FORMAT _Format)
{
	switch (_Format)
	{
		case oSURFACE_UNKNOWN: return oImage::UNKNOWN;
		case oSURFACE_R8G8B8A8_UNORM: return oImage::RGBA32;
		case oSURFACE_R8G8B8_UNORM: return oImage::RGB24;
		case oSURFACE_B8G8R8A8_UNORM: return oImage::BGRA32;
		case oSURFACE_B8G8R8_UNORM: return oImage::BGR24;
		case oSURFACE_R8_UNORM: return oImage::R8;
		oNODEFAULT;
	}
}

oSURFACE_FORMAT oImageFormatToSurfaceFormat(oImage::FORMAT _Format)
{
	switch (_Format)
	{
		case oImage::UNKNOWN: return oSURFACE_UNKNOWN;
		case oImage::RGBA32: return oSURFACE_R8G8B8A8_UNORM;
		case oImage::RGB24: return oSURFACE_R8G8B8_UNORM;
		case oImage::BGRA32: return oSURFACE_B8G8R8A8_UNORM;
		case oImage::BGR24: return oSURFACE_B8G8R8_UNORM;
		case oImage::R8: return oSURFACE_R8_UNORM;
		oNODEFAULT;
	}
}

oAPI oImage::FILE_FORMAT oImageFormatFromExtension(const char* _URIReference)
{
	oURIParts URIParts;
	if (!oURIDecompose(_URIReference, &URIParts))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return oImage::UNKNOWN_FILE;
	}

	const char* ext = oGetFileExtension(URIParts.Path);
	if (!ext)
	{
		oErrorSetLast(oERROR_NONE);
		return oImage::UNKNOWN_FILE;
	}

	static const char* sSupportedExts[] = 
	{
		".ico",
		".bmp",
		".jpg",
		".png",
		".dds",
	};

	for (size_t i = 0; i < oCOUNTOF(sSupportedExts); i++)
		if (!oStricmp(ext, sSupportedExts[i]))
			return static_cast<oImage::FILE_FORMAT>(i);

	oErrorSetLast(oERROR_NOT_FOUND, "Unrecognized extension %s", oSAFESTR(ext));
	return oImage::UNKNOWN_FILE;
}

struct oImageImpl : public oImage
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oBuffer, oImage);
	oDEFINE_LOCKABLE_INTERFACE(Mutex);

	void* GetData() override { return FreeImage_GetBits(FIBitmap); }
	const void* GetData() const override { return FreeImage_GetBits(FIBitmap); }
	size_t GetSize() const override { return FICalculateSize(FIBitmap); }
	const char* GetName() const threadsafe override { return Name; }
	void GetDesc(DESC* _pDesc) const threadsafe override { FIGetDesc(FIBitmap, _pDesc); }
	FIBITMAP* Bitmap() { return FIBitmap; }
	const FIBITMAP* Bitmap() const { return FIBitmap; }

	// Load from an in-memory file
	oImageImpl(const char* _Name, const void* _pBuffer, size_t _SizeofBuffer, oImage::LOAD_FLAGS _Flags, bool* _pSuccess)
		: FIBitmap(FILoad(_pBuffer, _SizeofBuffer))
		, Name(_Name)
	{
		*_pSuccess = false;

		if (!FIBitmap)
		{
			oErrorSetLast(oERROR_GENERIC, "Failed to create oImage %s", Name);
			return;
		}

		if (FIC_RGB == FreeImage_GetColorType(FIBitmap) && _Flags == oImage::FORCE_ALPHA)
		{
			FIBITMAP* FIBitmap32 = FreeImage_ConvertTo32Bits(FIBitmap);
			FreeImage_Unload(FIBitmap);
			FIBitmap = FIBitmap32;
		}

		else if (FIC_RGBALPHA == FreeImage_GetColorType(FIBitmap) && _Flags == oImage::FORCE_NO_ALPHA)
		{
			FIBITMAP* FIBitmap24 = FreeImage_ConvertTo24Bits(FIBitmap);
			FreeImage_Unload(FIBitmap);
			FIBitmap = FIBitmap24;
		}

		*_pSuccess = !!FIBitmap;
	}

	// Allocate according to desc
	oImageImpl(const char* _Name, const DESC& _Desc, bool* _pSuccess)
		: FIBitmap(FIAllocate(_Desc))
		, Name(_Name)
	{
		if (!FIBitmap)
			oErrorSetLast(oERROR_GENERIC, "Failed to create oImage %s", Name);
		*_pSuccess = !!FIBitmap;
	}

	#if defined(_WIN32) || defined(_WIN64)
		oImageImpl(const char* _Name, HBITMAP _hBmp, bool* _pSuccess)
			: FIBitmap(FIAllocateFIBITMAP(_hBmp))
			, Name(_Name)
		{
			if (!FIBitmap)
				oErrorSetLast(oERROR_GENERIC, "Failed to create oImage %s", Name);
			*_pSuccess = !!FIBitmap;
		}
	#endif

	~oImageImpl()
	{
		FreeImage_Unload(FIBitmap);
	}

	void Put(const int2& _Coord, oColor _Color) override
	{
		FreeImage_SetPixelColor(FIBitmap, _Coord.x, _Coord.y, (RGBQUAD*)&_Color);
	}

	oColor Get(const int2& _Coord) const override
	{
		oColor c;
		FreeImage_GetPixelColor(FIBitmap, _Coord.x, _Coord.y, (RGBQUAD*)&c);
		return c;
	}

	void CopyData(const void* _pSourceBuffer, size_t _SourceRowPitch, bool _FlipVertically) threadsafe override
	{
		oLockGuard<oSharedMutex> Lock(Mutex);
		DESC d;
		GetDesc(&d);
		oMemcpy2d(FreeImage_GetBits(FIBitmap), d.RowPitch, _pSourceBuffer, _SourceRowPitch, d.Dimensions.x * oImageGetSize(d.Format), d.Dimensions.y, _FlipVertically);
	}
		
	void CopyData(struct HBITMAP__* _hBitmap) threadsafe override
	{
		#if defined(_WIN32) || defined(_WIN64)
		FICopyBits(FIBitmap, _hBitmap);

		#endif
	}

	void CopyDataTo(void* _pDestinationBuffer, size_t _DestinationRowPitch, bool _FlipVertically) const threadsafe
	{
		oSharedLock<oSharedMutex> Lock(Mutex);
		DESC d;
		GetDesc(&d);
		oMemcpy2d(_pDestinationBuffer, _DestinationRowPitch, FreeImage_GetBits(FIBitmap), d.RowPitch, d.Dimensions.x * oImageGetSize(d.Format), d.Dimensions.y, _FlipVertically);
	}

private:
	FIBITMAP* FIBitmap;
	oRefCount RefCount;
	oSharedMutex Mutex;
	oStringURI Name;
};

const oGUID& oGetGUID( threadsafe const oImage* threadsafe const * )
{
	// {83CECF1C-316F-4ed4-9B20-4180B2ED4B4E}
	static const oGUID oIIDImage = { 0x83cecf1c, 0x316f, 0x4ed4, { 0x9b, 0x20, 0x41, 0x80, 0xb2, 0xed, 0x4b, 0x4e } };
	return oIIDImage;
}

bool oImageGetDesc(const void* _pBuffer, size_t _SizeofBuffer, oImage::DESC* _pDesc)
{
	FIBITMAP* FIBitmap = FILoad(_pBuffer, _SizeofBuffer, false);
	if (!FIBitmap)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to load the specified buffer as a file format supported by oImage");
	FIGetDesc(FIBitmap, _pDesc);
	FreeImage_Unload(FIBitmap);
	return true;
}

bool oImageCreate(const char* _Name, const void* _pBuffer, size_t _SizeofBuffer, oImage::LOAD_FLAGS _Flags, oImage** _ppImage)
{
	if (!_pBuffer || !_SizeofBuffer || !_ppImage) 
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	bool success = false;
	oCONSTRUCT(_ppImage, oImageImpl(_Name, _pBuffer, _SizeofBuffer, _Flags, &success));
	return success;
}

bool oImageCreate(const char* _Name, const oImage::DESC& _Desc, oImage** _ppImage)
{
	if (!_ppImage)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	
	bool success = false;
	oCONSTRUCT(_ppImage, oImageImpl(_Name, _Desc, &success));
	return success;
}

bool oImageCreate(const char* _Name, const oSURFACE_DESC& _Desc, oImage** _ppImage)
{
	oImage::DESC id;
	id.Format = oImageFormatFromSurfaceFormat(_Desc.Format);
	id.Dimensions = oSurfaceCalcDimensions(_Desc);
	id.RowPitch = oSurfaceMipCalcRowPitch(_Desc, 0);
	return oImageCreate(_Name, id, _ppImage);
}

#if defined(_WIN32) || defined(_WIN64)
	#include <oPlatform/Windows/oWindows.h>

	bool oImageCreateBitmap(const threadsafe oImage* _pSourceImage, struct HBITMAP__** _ppBitmap)
	{
		if (!_pSourceImage || !_ppBitmap)
			return oErrorSetLast(oERROR_INVALID_PARAMETER);

		oConstLockedPointer<oImage> LockedSourceImage(_pSourceImage);
		*_ppBitmap = FIAllocateBMP(((oImageImpl*)LockedSourceImage.c_ptr())->Bitmap());
		return !!_ppBitmap;
	}

	bool oImageCreate(const char* _Name, struct HBITMAP__* _pBitmap, oImage** _ppImage)
	{
		if (!_pBitmap || !_ppImage)
			return oErrorSetLast(oERROR_INVALID_PARAMETER);

		bool success = false;
		oCONSTRUCT(_ppImage, oImageImpl(_Name, _pBitmap, &success));
		return success;
	}

#endif

static size_t oImageSave(FIBITMAP* _bmp, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, void* _pBuffer, size_t _SizeofBuffer)
{
	oASSERT((size_t)((DWORD)_SizeofBuffer) == _SizeofBuffer, "Size of buffer too large for underlying implementation.");
	FIMEMORY* pMemory = FreeImage_OpenMemory(nullptr, 0);
	oOnScopeExit CloseMem([&] { FreeImage_CloseMemory(pMemory); });

	FREE_IMAGE_FORMAT fif = FIToFIF(_Format);

	size_t written = 0;

	FIBITMAP* flipClone = FreeImage_Clone(_bmp);
	oOnScopeExit unloadClone([&](){ FreeImage_Unload(flipClone); });
	FreeImage_FlipVertical(flipClone);

	if (FreeImage_SaveToMemory(fif, flipClone, pMemory, FIGetSaveFlags(fif, _CompressionLevel)))
		written = FreeImage_TellMemory(pMemory);
	else
		oErrorSetLast(oERROR_IO, "Failed to save image to memory");

	if (written > _SizeofBuffer)
	{
		oErrorSetLast(oERROR_AT_CAPACITY, "The specified buffer is too small to receive image");
		return written;
	}
	
	if (_pBuffer)
		memcpy(_pBuffer, pMemory, written);

	return written;
}

size_t oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, void* _pBuffer, size_t _SizeofBuffer)
{
	if (!_pImage || _Format == oImage::UNKNOWN || _Format == oImage::DDS || !_pBuffer || !_SizeofBuffer)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oConstLockedPointer<oImage> LockedImage(_pImage);
	FIBITMAP* FIBitmap = ((oImageImpl*)LockedImage.c_ptr())->Bitmap();
	return oImageSave(FIBitmap, _Format, _CompressionLevel, _pBuffer, _SizeofBuffer); // pass through error
}

size_t oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, oImage::LOAD_FLAGS _Flags, void* _pBuffer, size_t _SizeofBuffer)
{
	if (!_pImage || _Format == oImage::UNKNOWN || _Format == oImage::DDS || !_pBuffer || !_SizeofBuffer)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oConstLockedPointer<oImage> LockedImage(_pImage);
	FIBITMAP* FIBitmap = ((oImageImpl*)LockedImage.c_ptr())->Bitmap();

	if (_Flags == oImage::FORCE_ALPHA && FIC_RGB == FreeImage_GetColorType(FIBitmap))
		FIBitmap = FreeImage_ConvertTo32Bits(FIBitmap);
	else if (_Flags == oImage::FORCE_NO_ALPHA && FIC_RGBALPHA == FreeImage_GetColorType(FIBitmap))
		FIBitmap = FreeImage_ConvertTo24Bits(FIBitmap);

	size_t written = oImageSave(FIBitmap, _Format, _CompressionLevel, _pBuffer, _SizeofBuffer);
	
	if (((oImageImpl*)LockedImage.c_ptr())->Bitmap() != FIBitmap)
		FreeImage_Unload(FIBitmap);

	return written; // pass through error
}

bool oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, oImage::LOAD_FLAGS _Flags, const char* _Path)
{
	if (!_pImage || !oSTRVALID(_Path))
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	if (_Format == oImage::UNKNOWN)
		_Format = FIFromFIF(FreeImage_GetFIFFromFilename(_Path));

	if (_Format == oImage::UNKNOWN)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "could not decode file format from path");

	if (_Format == oImage::DDS)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	FREE_IMAGE_FORMAT fif = FIToFIF(_Format);

	oConstLockedPointer<oImage> LockedImage(_pImage);
	FIBITMAP* FIBitmap = ((oImageImpl*)LockedImage.c_ptr())->Bitmap();

	if (_Flags == oImage::FORCE_ALPHA && FIC_RGB == FreeImage_GetColorType(FIBitmap))
		FIBitmap = FreeImage_ConvertTo32Bits(FIBitmap);
	else if (_Flags == oImage::FORCE_NO_ALPHA && FIC_RGBALPHA == FreeImage_GetColorType(FIBitmap))
		FIBitmap = FreeImage_ConvertTo24Bits(FIBitmap);

	if (!oFileEnsureParentFolderExists(_Path))
		return false; // pass through error

	FIBITMAP* flipClone = FreeImage_Clone(FIBitmap);
	oOnScopeExit unloadClone([&](){ FreeImage_Unload(flipClone); });
	FreeImage_FlipVertical(flipClone);

	bool success = !!FreeImage_Save(fif, flipClone, _Path, FIGetSaveFlags(fif, _CompressionLevel));
		
	if (((oImageImpl*)LockedImage.c_ptr())->Bitmap() != FIBitmap)
		FreeImage_Unload(FIBitmap);

	if (!success)
		return oErrorSetLast(oERROR_IO, "Failed to save oImage \"%s\" to path \"%s\"", _pImage->GetName(), _Path);

	return true;
}

bool oImageLoad(const char* _Path, oImage::LOAD_FLAGS _Flags, oImage** _ppImage)
{
	if (!oSTRVALID(_Path) || !_ppImage)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oRef<oBuffer> FileData;
	if (!oBufferLoad(_Path, &FileData))
		return false; // pass error through

	return oImageCreate(_Path, FileData->GetData(), FileData->GetSize(), _Flags, _ppImage); // pass through error
}

bool oImageCompare(const threadsafe oImage* _pImage1, const threadsafe oImage* _pImage2, unsigned int _BitTolerance, float* _pRootMeanSquare, oImage** _ppDiffImage, unsigned int _DiffMultiplier)
{
	if (!_pImage1 || !_pImage2)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Two valid images must be specified");

	oConstLockedPointer<oImage> LockedImage1(_pImage1);
	oConstLockedPointer<oImage> LockedImage2(_pImage2);
	
	oImage::DESC ImgDesc1, ImgDesc2;
	_pImage1->GetDesc(&ImgDesc1);
	_pImage2->GetDesc(&ImgDesc2);

	if (ImgDesc1.Format != ImgDesc2.Format)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "oImages must be in in the same format to be compared. (%s v. %s)", oAsString(ImgDesc1.Format), oAsString(ImgDesc2.Format));

	if (ImgDesc1.Dimensions != ImgDesc2.Dimensions)
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "oImages differ in dimensions. ([%dx%d] v. [%dx%d])", ImgDesc1.Dimensions.x, ImgDesc1.Dimensions.y, ImgDesc2.Dimensions.x, ImgDesc2.Dimensions.y);

	if (LockedImage1->GetSize() != LockedImage2->GetSize())
		return oErrorSetLast(oERROR_INVALID_PARAMETER, "Image sizes don't match");

	oImage::DESC descDiff;
	if (_ppDiffImage)
	{
		descDiff.Dimensions = ImgDesc1.Dimensions;
		descDiff.Format = ImgDesc1.Format;
		descDiff.RowPitch = oImageCalcRowPitch(descDiff.Format, descDiff.Dimensions.x);
		if (!oImageCreate("Temp image", descDiff, _ppDiffImage))
			return false; // pass through error
	}

	unsigned char* diff = nullptr;
	if (_ppDiffImage)
		diff = static_cast<unsigned char*>((*_ppDiffImage)->GetData());

	int elementSize = oSurfaceFormatGetSize(oImageFormatToSurfaceFormat(ImgDesc1.Format));
	// @oooii-tony: This is ripe for parallelism. Optimize this using oParallelFor..
	const unsigned char* c1 = static_cast<const unsigned char*>(LockedImage1->GetData());
	const unsigned char* c2 = static_cast<const unsigned char*>(LockedImage2->GetData());

	const size_t nPixels = (*_ppDiffImage)->GetSize() / elementSize;

	float RMSAccum = 0.0f;

	for (size_t i = 0; i < nPixels; i++)
	{
		for (int j = 0; j < elementSize; j++)
		{
			int sample1 = c1[i*elementSize+j];
			int sample2 = c2[i*elementSize+j];
			int d = abs(sample1 - sample2);
			RMSAccum += d*d;
			diff[i*elementSize+j] = static_cast<unsigned char>(clamp(d, 0, 255));
		}
	}

	if (_pRootMeanSquare)
		*_pRootMeanSquare = sqrt(RMSAccum / nPixels);

	return true;
}
