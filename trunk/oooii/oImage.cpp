// $(header)
#include <oooii/oAssert.h>
#include <oooii/oColor.h>
#include <oooii/oErrno.h>
#include <oooii/oFile.h>
#include <oooii/oImage.h>
#include <oooii/oPath.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oMutex.h>
#include <oooii/oWindows.h>
#include <oooii/oBuffer.h>
#include <FreeImage.h>
#include <oooii/oSTL.h>

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

// @oooii-Andrew: was oProcessSingleton.  This probably needs to be an oModuleSingleton, particularly
// to fix the ico free image crash where only one module (oOmek) runs the initialization but the exe GestureServer
// attempts to load the icon.
struct FIStaticInitialization : public oModuleSingleton<FIStaticInitialization>
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
		oASSERT(false, "%s format image file: %s", FreeImage_GetFormatFromFIF(fif), message);
	}
};

FIStaticInitialization::Run sInitializeFreeImage; // @oooii-tony: ok static, we need to run this code exactly once per process, and oSingleton guarantees that.

const oGUID& oGetGUID( threadsafe const oImage* threadsafe const * )
{
	// {83CECF1C-316F-4ed4-9B20-4180B2ED4B4E}
	static const oGUID oIIDImage = { 0x83cecf1c, 0x316f, 0x4ed4, { 0x9b, 0x20, 0x41, 0x80, 0xb2, 0xed, 0x4b, 0x4e } };
	return oIIDImage;
}

struct oImage_Impl : public oImage
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oImage>());

	oImage_Impl(FIBITMAP* _bmp, oSurface::FORMAT _Format);
	~oImage_Impl();

	void Cleanup();
	bool Create(FIBITMAP* _bmp, oSurface::FORMAT _Format);

	bool Update(const void* _pBuffer, size_t _SizeOfBuffer) threadsafe override;

	void FlipHorizontal() threadsafe override;
	void FlipVertical() threadsafe override;

	void GetDesc(DESC* _pDesc) const threadsafe override;

	void* Map() threadsafe override;
	const void* Map() const threadsafe override;

	void Unmap() threadsafe override;
	void Unmap() const threadsafe override;

	bool Compare(const oImage* _pOtherImage, unsigned int _BitFuzziness, unsigned int* _pNumDifferingPixels = 0, oImage** _ppDiffImage = 0, unsigned int _DiffMultiplier = 1) override;

	bool Save(const char* _Path, COMPRESSION _Compression = NONE) threadsafe override;
	size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe override;
	
	bool GetAsRGBA(BYTE* _pOutput, size_t _OutputSize, bool _topDown) threadsafe override;
	bool GetAsBGRA(unsigned char* _pOutput, size_t _OutputSize, bool _topDown) threadsafe override;
	
	#if defined(_WIN32) || defined(_WIN64)
		HBITMAP AsBmp() threadsafe override;
	#endif

	DESC Desc; //only used for YUV format.
	std::vector<unsigned char> YUVData; //Support texture arrays in YUV format.
	oSurface::YUV420 YUVFrame;

	FIBITMAP* bmp;
	oRefCount RefCount;
	oRWMutex Mutex;
};

oImage_Impl::oImage_Impl(FIBITMAP* _bmp, oSurface::FORMAT _Format)
	: bmp(_bmp)
{
	Create(_bmp, _Format);

	oASSERT(FIStaticInitialization::Singleton()->IsInitialized, "FreeImage has not been initialized.");
}

oImage_Impl::~oImage_Impl()
{
	Cleanup();

}

bool oImage_Impl::Create(FIBITMAP* _bmp, oSurface::FORMAT _Format) 
{
	bmp = _bmp;

	FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(_bmp);

	// @oooii-eric: TODO: Only a handful of conversions currently supported. should support all.
	if (colorType == FIC_RGB && !(_Format == oSurface::R8G8B8_UNORM || _Format == oSurface::YUV420_UNORM))
	{
		bmp = FreeImage_ConvertTo32Bits(_bmp);
		FreeImage_Unload(_bmp);
	}

	if (_Format == oSurface::YUV420_UNORM)
	{
		Desc.Width = FreeImage_GetWidth(bmp);
		Desc.Height = FreeImage_GetHeight(bmp);
		unsigned int pitch = FreeImage_GetPitch(bmp);
		unsigned int YSize = Desc.Width*Desc.Height;
		unsigned int UVSize = YSize/4;

		YUVData.resize(YSize+UVSize*2);
		YUVFrame.pY = &YUVData[0];
		YUVFrame.pU = &YUVData[YSize];
		YUVFrame.pV = &YUVData[YSize+UVSize];
		YUVFrame.YPitch = Desc.Width;
		YUVFrame.UVPitch = Desc.Width/2;

		Desc.Pitch = static_cast<unsigned int>(YUVFrame.YPitch);
		Desc.Format = oSurface::YUV420_UNORM;
		Desc.Size = static_cast<unsigned int>(YUVData.size());

		if (colorType == FIC_RGBALPHA)
		{
			oSurface::convert_B8G8R8A8_UNORM_to_YUV420(Desc.Width, Desc.Height, FreeImage_GetBits(bmp), pitch, &YUVFrame, true);
		}
		else if (colorType == FIC_RGB)
		{
			oSurface::convert_B8G8R8_UNORM_to_YUV420(Desc.Width, Desc.Height, FreeImage_GetBits(bmp), pitch, &YUVFrame, true);
		}
		FreeImage_Unload(bmp);
		bmp = nullptr;
	}

#ifdef _DEBUG
	for(int m = FIMD_COMMENTS; m <= FIMD_CUSTOM; m++)
		if(FreeImage_GetMetadataCount((FREE_IMAGE_MDMODEL)m, bmp))
		{
			oWARN("Image contains metadata. FreeImage will leak when unloading this image.");
			break;
		}
#endif // _DEBUG

		return true;
}

void oImage_Impl::Cleanup() 
{
	oASSERT(FIStaticInitialization::Singleton()->IsInitialized, "FreeImage has not been initialized.");
	if (bmp)
		FreeImage_Unload(bmp);
}

void oImage_Impl::FlipHorizontal() threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	#ifdef oENABLE_ASSERTS
		BOOL successfulFlipHorizontal = 
	#endif
	FreeImage_FlipHorizontal(bmp);

	oASSERT(successfulFlipHorizontal, "");
}

void oImage_Impl::FlipVertical() threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	#ifdef oENABLE_ASSERTS
		BOOL successfulFlipVertical = 
	#endif
	FreeImage_FlipVertical(bmp);
	oASSERT(successfulFlipVertical, "");
}

void oImage_Impl::GetDesc(DESC* _pDesc) const threadsafe
{
	oRWMutex::ScopedLockRead lock(Mutex);

	if(bmp)
	{
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
	else
	{
		*_pDesc = thread_cast<oImage_Impl*>(this)->Desc;
	}
}

void* oImage_Impl::Map() threadsafe
{
	Mutex.Lock();
	if(bmp)
		return FreeImage_GetBits(bmp);
	else
		return &thread_cast<oImage_Impl*>(this)->YUVData[0];
}

const void* oImage_Impl::Map() const threadsafe
{
	Mutex.LockRead();
	if(bmp)
		return FreeImage_GetBits(bmp);
	else
		return &thread_cast<oImage_Impl*>(this)->YUVData[0];
}

void oImage_Impl::Unmap() threadsafe
{
	Mutex.Unlock();
}

void oImage_Impl::Unmap() const threadsafe
{
	Mutex.UnlockRead();
}

bool oImage_Impl::Compare(const oImage* _pOtherImage, unsigned int _BitFuzziness, unsigned int* _pNumDifferingPixels, oImage** _ppDiffImage, unsigned int _DiffMultiplier)
{
	oImage::DESC desc1, desc2;
	GetDesc(&desc1);
	_pOtherImage->GetDesc(&desc2);

	if (desc1.Size != desc2.Size)
	{
		oSetLastError(EINVAL, "Image sizes don't match.");
		return false;
	}

	if (_ppDiffImage)
	{
		oImage::DESC desc;
		desc.Width = desc1.Width;
		desc.Height = desc1.Height;
		desc.Format = oSurface::R8G8B8A8_UNORM;
		desc.Pitch = oSurface::CalcRowPitch(desc.Format, desc.Width);
		desc.Size = oSurface::CalcLevelPitch(desc.Format, desc.Width, desc.Height);
		if (!oImage::Create(&desc, _ppDiffImage))
			return false;
	}

	// @oooii-tony: This is ripe for parallelism. oParallelFor is still being
	// brought up at this time, but should be deployed here.

	const oColor* c1 = (oColor*)Map();
	const oColor* c2 = (oColor*)_pOtherImage->Map();

	unsigned int* diff = (unsigned int*)(_ppDiffImage ? (*_ppDiffImage)->Map() : 0);

	unsigned int nDifferences = 0;

	const size_t nPixels = desc1.Size / sizeof(oColor);
	for (size_t i = 0; i < nPixels; i++)
	{
		if (!oEqual(c1[i], c2[i], _BitFuzziness))
		{
			nDifferences++;

			if (diff)
				diff[i] = oDiffColorsRGB(c1[i], c2[i], _DiffMultiplier);
		}

		else if (diff)
			diff[i] = 0xff000000;
	}

	Unmap();
	_pOtherImage->Unmap();

	if (_ppDiffImage)
		(*_ppDiffImage)->Unmap();

	if (_pNumDifferingPixels)
		*_pNumDifferingPixels = nDifferences;

	return true;
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

bool oImage_Impl::GetAsRGBA(BYTE* _pOutput, size_t _OutputSize, bool _topDown) threadsafe
{
	size_t size = FreeImage_GetWidth(bmp) * FreeImage_GetHeight(bmp) * 4;
	if(!GetAsBGRA(_pOutput, _OutputSize, _topDown))
		return false;

	// @oooii-mike: FreeImage kindly converts colors to bgra format, ignoring the parameters in ConvertToRawBits, and they need to be converted back:
	for(size_t i = 0; i+3 < size; i+=4)
	{
		_pOutput[i] ^= _pOutput[i+2];
		_pOutput[i+2] ^= _pOutput[i];
		_pOutput[i] ^= _pOutput[i+2];
	}

	return true;
}

bool oImage_Impl::GetAsBGRA(unsigned char* _pOutput, size_t _OutputSize, bool _topDown) threadsafe
{
	size_t size = FreeImage_GetWidth(bmp) * FreeImage_GetHeight(bmp) * 4;
	if(size > _OutputSize)
	{
		oSetLastError(E2BIG, "Output buffer too small for converted image.");
		return false;
	}
	FreeImage_ConvertToRawBits(_pOutput, bmp, FreeImage_GetWidth(bmp) * 4, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, _topDown);

	return true;
};

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

	bpp = static_cast<int>(oSurface::GetBitSize(_pDesc->Format));

	switch (_pDesc->Format)
	{
		case oSurface::R8G8B8A8_UNORM:
			r = FI_RGBA_RED_MASK;
			g = FI_RGBA_GREEN_MASK;
			b = FI_RGBA_BLUE_MASK;
			break;

		case oSurface::R8G8B8_UNORM:
			r = FI_RGBA_RED_MASK;
			g = FI_RGBA_GREEN_MASK;
			b = FI_RGBA_BLUE_MASK;
			break;

		default:
			oSetLastError(EINVAL, "oImage::Create() failed: Unsupported format %s", oAsString(_pDesc->Format));
			return 0;
	}

	return FreeImage_Allocate(_pDesc->Width, _pDesc->Height, bpp, r, g, b);
}

bool oImage_Impl::Update(const void* _pBuffer, size_t _SizeOfBuffer) threadsafe
{
	oRWMutex::ScopedLock lock(Mutex);
	oImage_Impl *lockedThis = thread_cast<oImage_Impl*>(this);

	lockedThis->Cleanup();

	if (!_pBuffer || !_SizeOfBuffer) return false;

	oSurface::FORMAT format = bmp ? oSurface::R8G8B8A8_UNORM : oSurface::YUV420_UNORM;
	bmp = FILoadFromMemory(_pBuffer, _SizeOfBuffer);
	return lockedThis->Create(bmp, format);
};

bool oImage::Create(const void* _pBuffer, size_t _SizeofBuffer, oSurface::FORMAT _Format, oImage** _ppImage)
{
	if (!_pBuffer || !_SizeofBuffer || !_ppImage) return false;
	FIBITMAP* bmp = FILoadFromMemory(_pBuffer, _SizeofBuffer);
	*_ppImage = bmp ? new oImage_Impl(bmp, _Format) : 0;
	return !!*_ppImage;
}

bool oImage::Create(const DESC* _pDesc, oImage** _ppImage)
{
	if (!_pDesc || !_ppImage) return false;
	FIBITMAP* bmp = FICreate(_pDesc);
	*_ppImage = bmp ? new oImage_Impl(bmp, _pDesc->Format) : 0;
	return !!*_ppImage;
}

#if defined(_WIN32) || defined(_WIN64)
	#include <oooii/oWindows.h>

	HBITMAP oImage_Impl::AsBmp() threadsafe
	{
		oRWMutex::ScopedLockRead lock(Mutex);
		return CreateDIBitmap(GetDC(0), FreeImage_GetInfoHeader(bmp), CBM_INIT, FreeImage_GetBits(bmp), FreeImage_GetInfo(bmp), DIB_RGB_COLORS);
	}
#endif

void SetConsoleOOOiiIcon()
{
	#if defined(_WIN32) || defined (_WIN64)
		// Load OOOii lib icon
		extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);
		const char* BufferName = 0;
		const void* pBuffer = 0;
		size_t bufferSize = 0;
		GetDescoooii_ico(&BufferName, &pBuffer, &bufferSize);

		// This function might be called from static init, so ensure 
		// FreeImage is up and running.
		FIStaticInitialization::Singleton();

		oRef<oImage> ico;
		oVERIFY(oImage::Create(pBuffer, bufferSize, oSurface::UNKNOWN, &ico));

		HBITMAP hBmp = ico->AsBmp();
		HICON hIcon = oIconFromBitmap(hBmp);
		oSetIcon(GetConsoleWindow(), false, hIcon);
		DeleteObject(hIcon);
		DeleteObject(hBmp);
	#endif
}

bool oImage::IsSupportedFileType(const char* _Path)
{
	FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
	format = FreeImage_GetFileType(_Path);
	if(format != FIF_UNKNOWN)
		return true;
	else
	{
		format = FreeImage_GetFIFFromFilename(_Path);
		if(format != FIF_UNKNOWN)
			return true;
		else
			return false;
	}
}
