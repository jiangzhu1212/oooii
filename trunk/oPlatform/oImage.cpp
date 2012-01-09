// $(header)
#include <oPlatform/oReporting.h>
#include <oBasis/oColor.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oError.h>
#include <oBasis/oMutex.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oImage.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oWindows.h>
#include <FreeImage.h>



const oImage::ForceAlpha_t oImage::ForceAlpha;

// returns true if the ordering is BGR, false if RGB. Alpha or no alpha result
// in the same interpretation: true if ABGR, false if ARGB (I haven't run across
// the RGBA or BGRA situation yet).
static bool FIIsBGR(FIBITMAP* _bmp)
{
  // BGRA (or ARGB) = 0xAARRGGBB, RGBA (or ABGR) = 0xRRGGBBAA
  unsigned int redMask = FreeImage_GetRedMask(_bmp);
  return redMask == 0x00FF0000;
}

static oSURFACE_FORMAT GetSurfaceFormat(FIBITMAP* _pBitmap)
{
  oSURFACE_FORMAT format = oSURFACE_UNKNOWN;
  if (_pBitmap)
  {
    FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(_pBitmap);
    unsigned int bpp = FreeImage_GetBPP(_pBitmap);

    // only rgb(a) formats supported
    if (!(colorType == FIC_RGB || colorType == FIC_RGBALPHA))
      return oSURFACE_UNKNOWN;

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
        format = isBGR ? oSURFACE_B8G8R8A8_UNORM : oSURFACE_R8G8B8A8_UNORM;
      else
        format = isBGR ? oSURFACE_B8G8R8_UNORM : oSURFACE_R8G8B8_UNORM;
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
  }

  return format;
}

static FIBITMAP* FICreate(const oImage::DESC* _Desc)
{
  int bpp = 0;
  unsigned int r = 0, g = 0, b = 0;

  bpp = static_cast<int>(oSurfaceGetBitSize(_Desc->Format));

  switch (_Desc->Format)
  {
  case oSURFACE_B8G8R8A8_UNORM:
    r = FI_RGBA_RED_MASK;
    g = FI_RGBA_GREEN_MASK;
    b = FI_RGBA_BLUE_MASK;
    break;

  case oSURFACE_B8G8R8_UNORM:
    r = FI_RGBA_RED_MASK;
    g = FI_RGBA_GREEN_MASK;
    b = FI_RGBA_BLUE_MASK;
    break;

  default:
    //oErrorSetLast(oERROR_INVALID_PARAMETER, "Unsupported format %s", oAsString(_Desc->Format));
    return nullptr;
  }

  return FreeImage_Allocate(_Desc->Dimensions.x, _Desc->Dimensions.y, bpp, r, g, b);
}

static void FIGetDesc(FIBITMAP* _bmp, oImage::DESC* _pDesc)
{
  oASSERT(_bmp && _pDesc, "");

  _pDesc->Pitch = FreeImage_GetPitch(_bmp);
  _pDesc->Dimensions.x = FreeImage_GetWidth(_bmp);
  _pDesc->Dimensions.y = FreeImage_GetHeight(_bmp);
  _pDesc->Format = GetSurfaceFormat(_bmp);

  oSURFACE_DESC sd;
  sd.RowPitch = _pDesc->Pitch;
  sd.Dimensions.x = _pDesc->Dimensions.x;
  sd.Dimensions.y = _pDesc->Dimensions.y;
  sd.Dimensions.z = 1;
  sd.NumMips = 1;
  sd.SlicePitch = 0;
  sd.Format = _pDesc->Format;
  //return oSurfaceCalcSize(sd);
}

static FIBITMAP* FILoadFromMemory(const void* _pBuffer, size_t _SizeofBuffer, bool _LoadBitmap = true)
{
  FIBITMAP* bmp = nullptr;
  if (_pBuffer && _SizeofBuffer)
  {
    FIMEMORY* m = FreeImage_OpenMemory((BYTE*)_pBuffer, (DWORD)_SizeofBuffer);
    if (m)
    {
      oASSERT(_LoadBitmap, "FIF_LOAD_NOPIXELS isn't supported in this version of FreeImage (upgrade!)");
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(m, 0);
      if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif))
        bmp = FreeImage_LoadFromMemory(fif, m, 0 /*_LoadBitmap ? 0 : FIF_LOAD_NOPIXELS*/);

      FreeImage_CloseMemory(m);
    }
  }

  return bmp;
}

int GetSaveFlags(FREE_IMAGE_FORMAT _Format, oImage::Compression_e _Compression)
{
  switch (_Format)
  {
  case FIF_DDS:
    {

      return 0;
    }
    break;

  case FIF_BMP:
    {
      switch (_Compression)
      {
      case oImage::NO_COMPRESSION: 
        return BMP_DEFAULT;

      case oImage::LOW_COMPRESSION:
      case oImage::MEDIUM_COMPRESSION:
      case oImage::HIGH_COMPRESSION: 
        return BMP_SAVE_RLE;
        oNODEFAULT;
      }

    }
    break;

  case FIF_JPEG:
    {
      switch (_Compression)
      {
      case oImage::NO_COMPRESSION: 
        return JPEG_QUALITYSUPERB;
      case oImage::LOW_COMPRESSION: 
        return JPEG_DEFAULT;
      case oImage::MEDIUM_COMPRESSION: 
        return JPEG_QUALITYNORMAL;
      case oImage::HIGH_COMPRESSION: 
        return JPEG_QUALITYAVERAGE;
        oNODEFAULT;
      }

    }
    break;

  case FIF_PNG:
    {
      switch (_Compression)
      {
      case oImage::NO_COMPRESSION: 
        return PNG_Z_NO_COMPRESSION;
      case oImage::LOW_COMPRESSION: 
        return PNG_Z_BEST_SPEED;
      case oImage::MEDIUM_COMPRESSION: 
        return PNG_Z_DEFAULT_COMPRESSION;
      case oImage::HIGH_COMPRESSION: 
        return PNG_Z_BEST_COMPRESSION;
        oNODEFAULT;
      }

    }
    break;

  default: 
    break;
  }

  return 0;
}

// create a FreeImage from the oImage
struct oFreeImage : oNoncopyable
{
public:
  oFreeImage(FIBITMAP* _pBmp)
    : pBmp(_pBmp)
  {}
  oFreeImage(const oImage * pImage)
  {
    pBmp = FICreate(pImage->GetDesc());
    if(!pBmp)
      return;

    void * pData = FreeImage_GetBits(pBmp);
    memcpy(pData, pImage->GetData(), pImage->GetSize());
  }
  ~oFreeImage()
  {
    if(pBmp)
      FreeImage_Unload(pBmp);
  }

  operator FIBITMAP * () { return pBmp; }

private:
  FIBITMAP* pBmp;
};

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
  oDEFINE_TRIVIAL_QUERYINTERFACE(oImage);
  oDEFINE_LOCKABLE_INTERFACE(Mutex);

  oRef<oBuffer> imageData;


  oImage::DESC m_desc;

  oImage_Impl(const char * pImageName, bool* _pSuccess) : Name(pImageName)
  {
    *_pSuccess = true;
  }
  oImage_Impl(const char * pImageName, const oImage::DESC * pDesc, bool* _pSuccess) : Name(pImageName)
  {
    SetDesc(pDesc, nullptr);

    *_pSuccess = true;
  }
  oImage_Impl(const char * pImageName, const oImage::DESC * pDesc, const void * pData, bool* _pSuccess)  : Name(pImageName)
  {
    SetDesc(pDesc, pData);

    *_pSuccess = true;
  }
  ~oImage_Impl();

  bool FlipHorizontal() override;
  bool FlipVertical() override;
  bool Resize(const int2& _NewSize) override;

  void GetDesc(DESC* _pDesc) const override;
  const DESC* GetDesc() const override;
  void SetDesc(const DESC* _pDesc, const void * pData) override;


  // oBuffer methods
  void* GetData() override;
  const void* GetData() const override;
  size_t GetSize() const override;
  const char* GetName() const threadsafe{ return thread_cast<oImage_Impl*>(this)->Name; /* thread_cast safe because name deosn't change*/}



  oRefCount RefCount;
  oSharedMutex Mutex;
  oStringM Name;
};


oImage_Impl::~oImage_Impl()
{
}








bool oImageCreateDesc(const void* _pBuffer, size_t _SizeofBuffer, oImage::DESC* _pDesc)
{
#if 1
  return oErrorSetLast(oERROR_NOT_FOUND, "This API is not yet supported (we're on a stale version of FreeImage)");
#else
  FIStaticInitialization::Singleton(); // Ensure FI is initialized
  if (!_pBuffer || !_SizeofBuffer || !_pDesc) return oErrorSetLast(oERROR_INVALID_PARAMETER);
  FIBITMAP* bmp = FILoadFromMemory(_pBuffer, _SizeofBuffer, false);
  if (!bmp) return oErrorSetLast(oERROR_GENERIC, "Failed to load buffer as an oImage");
  FIGetDesc(bmp, nullptr, _pDesc);
  FreeImage_Unload(bmp);
  return true;
#endif
}



bool oImageIsSupportedFormat(const char* _Path)
{
  FREE_IMAGE_FORMAT format = FIF_UNKNOWN;
  format = FreeImage_GetFileType(_Path);
  if (format != FIF_UNKNOWN)
    return true;
  else
  {
    format = FreeImage_GetFIFFromFilename(_Path);
    if (format != FIF_UNKNOWN)
      return true;
    else
      return false;
  }
}

// does not return if the compare was a success.  It returns if it was able to compare.
bool oImageCompare(const oImage* _pImage1, const oImage* _pImage2, 
  unsigned int bitTolerance,
  float & Root_Mean_Square,
  oImage** _ppDiffImage, unsigned int _DiffMultiplier)
{
  Root_Mean_Square = FLT_MAX;
  if (!_pImage1 || !_pImage2)
    return oErrorSetLast(oERROR_INVALID_PARAMETER, "Two valid images must be specified");

  // @oooii-tony: This is ripe for parallelism. Optimize this using oParallelFor..

  const oColor* c1 = (oColor*)_pImage1->GetData();
  const oColor* c2 = (oColor*)_pImage2->GetData();

  if (_pImage1->GetSize() != _pImage2->GetSize())
    return oErrorSetLast(oERROR_INVALID_PARAMETER, "Image sizes don't match");

  oImage::DESC ImgDesc;
  _pImage1->GetDesc(&ImgDesc);

  oImage::DESC descDiff;
  if (_ppDiffImage)
  {
    descDiff.Dimensions = ImgDesc.Dimensions;
    descDiff.Format = oSURFACE_B8G8R8A8_UNORM;
    descDiff.Pitch = oSurfaceCalcRowPitch(descDiff.Format, descDiff.Dimensions.x);

    if (!oImageCreate("Temp image", &descDiff, _ppDiffImage))
      return false;
  }
  oColor* diff = nullptr;
  if (_ppDiffImage)
  {
    diff = static_cast<oColor*>((*_ppDiffImage)->GetData());
  }

  //unsigned int nDifferences = 0;

  const size_t nPixels = (*_ppDiffImage)->GetSize() / sizeof(oColor);

  Root_Mean_Square = 0;
  
  for (size_t i = 0; i < nPixels; i++)
  {
    // exact for RMS
    if (!oEqual(c1[i], c2[i], bitTolerance))
    {
	    unsigned int r1,g1,b1,a1,r2,g2,b2,a2;
    	oColorDecompose(c1[i], &r1, &g1, &b1, &a1); 
      oColorDecompose(c2[i], &r2, &g2, &b2, &a2);

      unsigned int dr = r1 - r2;
      unsigned int dg = g1 - g2;
      unsigned int db = b1 - b2;
      unsigned int da = a1 - a2;
      
      Root_Mean_Square += dr * dr + dg * dg + db * db + da * da;

      if (diff)
        diff[i] = oColorDiffRGB(c1[i], c2[i], _DiffMultiplier);
    }
    else if (diff)
      diff[i] = std::Black;

  }
  Root_Mean_Square = sqrt( Root_Mean_Square / nPixels);

 

  //if (_pNumDifferingPixels)
  //  *_pNumDifferingPixels = nDifferences;

  return true;
}





bool oImageCreate(const char * pImageName, const void* _pBuffer, size_t _SizeofBuffer, oImage** ppImage)
{
  FIStaticInitialization::Singleton(); // Ensure FI is initialized
  if (!_pBuffer || !_SizeofBuffer || !ppImage) 
    return oErrorSetLast(oERROR_INVALID_PARAMETER);

  oFreeImage bmp (FILoadFromMemory(_pBuffer, _SizeofBuffer));

  bool success = false;

  oCONSTRUCT(ppImage, oImage_Impl(pImageName, &success));

  oImage* pImage = *ppImage;

  oImage::DESC desc;

  FIGetDesc(bmp, &desc);

  BYTE * pData = FreeImage_GetBits(bmp);

  pImage->SetDesc(&desc, pData);

  return !!*ppImage;
}

// from file in memory
// TODO: Need FreeImage fallbacks
bool oImageCreate(const char * pImageName, const void* _pBuffer, size_t _SizeOfBuffer, const oImage::ForceAlpha_t&, oImage** ppImage)
{
  FIStaticInitialization::Singleton(); // Ensure FI is initialized
  if (!_pBuffer || !_SizeOfBuffer || !ppImage) 
    return oErrorSetLast(oERROR_INVALID_PARAMETER);

  FIBITMAP* bmp = FILoadFromMemory(_pBuffer, _SizeOfBuffer);

  FREE_IMAGE_COLOR_TYPE colorType = FreeImage_GetColorType(bmp);
  if (colorType == FIC_RGB)
  {
    // convert to 32 bits
    FIBITMAP* tbmp = FreeImage_ConvertTo32Bits(bmp);
    FreeImage_Unload(bmp);
    bmp = tbmp;
  }

  bool success = false;

  oCONSTRUCT(ppImage, oImage_Impl(pImageName, &success));

  oImage* pImage = *ppImage;

  oImage::DESC desc;

  FIGetDesc(bmp, &desc);


  BYTE * pData = FreeImage_GetBits(bmp);

  pImage->SetDesc(&desc, pData);

  FreeImage_Unload(bmp);

  return !!*ppImage;

}

bool oImageCreate(const char * pImageName, const oImage::DESC* _Desc, oImage** ppImage)
{

  bool success = false;
  oCONSTRUCT(ppImage, oImage_Impl(pImageName, _Desc, &success));
  oImage* pImage = *ppImage;

  return success;
}

// create a copy of an image
oAPI bool oImageCreate(const char * pImageName, const oImage * pSrcImage, oImage** _ppImage)
{
  if (!_ppImage) 
    return oErrorSetLast(oERROR_INVALID_PARAMETER);

  bool success = false;
  oCONSTRUCT(_ppImage, oImage_Impl(pImageName, pSrcImage->GetDesc(), pSrcImage->GetData(), &success));


  return success;

}


void oImage_Impl::GetDesc(DESC* pDesc) const 
{
  memcpy_s(pDesc, sizeof(DESC), (void*)&m_desc, sizeof(DESC));
}


const oImage::DESC* oImage_Impl::GetDesc() const 
{
  return &m_desc;
}

void oImage_Impl::SetDesc(const DESC* pDesc, const void * pData) 
{

  size_t size = oSurfaceCalcSize(pDesc->Format, pDesc->Dimensions.x, pDesc->Dimensions.y);

  memcpy_s((void *)&m_desc, sizeof(DESC), (void *)pDesc, sizeof(DESC));

  m_desc.Pitch = oSurfaceCalcRowPitch(m_desc.Format, m_desc.Dimensions.x);

  unsigned char* bufData = new unsigned char[size];

  if (pData)
    memcpy(bufData, pData, size);
  else
    memset(bufData, 0, size);

  oBufferCreate("Image Buffer", bufData, size, oBuffer::Delete, &imageData);

}

void * oImage_Impl::GetData() 
{

  return imageData->GetData();
}


size_t oImage_Impl::GetSize() const 
{
  return imageData->GetSize();
}



const void * oImage_Impl::GetData() const 
{

  return imageData->GetData();

}


bool oImageFromFreeImage(FIBITMAP * bmp, oImage * pImage)  
{
  FIStaticInitialization::Singleton(); // Ensure FI is initialized

  void * pData = FreeImage_GetBits(bmp);

  oImage::DESC desc;

  FIGetDesc(bmp, &desc);

  pImage->SetDesc(&desc, pData);

  return true;
}

bool oImage_Impl::FlipHorizontal() 
{
  oFreeImage bmp = oFreeImage(this);
  if (bmp)
  {
    BOOL successfulFlipHorizontal = FreeImage_FlipHorizontal(bmp);

    oImageFromFreeImage(bmp, this);
    return true;
  }

  oErrorSetLast(oERROR_INVALID_PARAMETER, "Unsupported format %s", oAsString(m_desc.Format));

  return false;
}

bool oImage_Impl::FlipVertical() 
{
#if 0
  oFreeImage bmp = oFreeImage(this);
  if (bmp)
  {
    BOOL successfulFlipHorizontal = FreeImage_FlipVertical(bmp);
    oImageFromFreeImage(bmp, this);
    return true;
  }

  oErrorSetLast(oERROR_INVALID_PARAMETER, "Unsupported format %s", oAsString(m_desc.Format));
  return false;

#endif

  // can't flip compressed formats
  if (oSurfaceIsBlockCompressedFormat(m_desc.Format))
  {
    oErrorSetLast(oERROR_INVALID_PARAMETER, "Unsupported format %s", oAsString(m_desc.Format));
    return false;
  }

  unsigned int pitch  = m_desc.Pitch;
  unsigned int height = m_desc.Dimensions.y;

  std::vector<BYTE> tempLine(pitch);

  BYTE * imageData = (BYTE*)GetData();
  if (!imageData)
    return false;

  unsigned int line_top = 0;
  unsigned int line_bottom = (height-1) * pitch;

  for(unsigned int y = 0; y < height/2; y++) 
  {
    memcpy(&tempLine[0], &imageData[line_top], pitch);
    memcpy(&imageData[line_top], &imageData[line_bottom], pitch);
    memcpy(&imageData[line_bottom], &tempLine[0], pitch);

    line_top += pitch;
    line_bottom -= pitch;

  }

  return true;

}



#if defined(_WIN32) || defined(_WIN64)
#include <oPlatform/oWindows.h>

HBITMAP AsBmp(const oImage * pImage) 
{
  oFreeImage bmp = oFreeImage(pImage);
  if (bmp)
  {
    //oSharedLock<oSharedMutex> lock(Mutex);
    HBITMAP hbm = CreateDIBitmap(GetDC(0), 
      FreeImage_GetInfoHeader(bmp), 
      CBM_INIT, 
      FreeImage_GetBits(bmp), 
      FreeImage_GetInfo(bmp), 
      DIB_RGB_COLORS);
    return hbm;

  }
  else
    return 0;
}
#endif



// Saves the image to the specified path. The file extension is used to 
// determine the format. Returns the number of bytes written, or 0 if failed.
// Check oErrorGetLast().
oAPI bool oImageSave(const oImage * pImage, const char* _Path, oImage::Compression_e _Compression)
{
  oFreeImage bmp = oFreeImage(pImage);
  if (!_Path || !*_Path)
    return oErrorSetLast(oERROR_INVALID_PARAMETER);

  FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(_Path);
  if (format == FIF_UNKNOWN)
    return oErrorSetLast(oERROR_INVALID_PARAMETER);

  if (format == FIF_DDS)
		return oErrorSetLast(oERROR_NOT_FOUND, "DDS Save is not currently supported");

  char folder[_MAX_PATH];
  strcpy_s(folder, _Path);
  oTrimFilename(folder);

  if (!oFileCreateFolder(folder) && oErrorGetLast() != oERROR_REDUNDANT)
    return oErrorSetLast(oERROR_NOT_FOUND, "Folder %s could not be created", folder);

  if (!FreeImage_Save(format, bmp, _Path, GetSaveFlags(format, _Compression)))
  {

    return oErrorSetLast(oERROR_IO, "Failed to save %s", _Path);
  }

  return true;
}

// Saves the image to a memory buffer as if it were a file, the file extension 
// is used to determine the format. Returns the number of bytes written, or 0
// if failed. Check oErrorGetLast(). If _pBuffer is NULL, this still returns
// the size requires for a successful save though nothing would be written.
//virtual size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe = 0;
oAPI size_t oImageSave(const oImage * pImage, const char* _Path, void* _pBuffer, size_t _SizeofBuffer, oImage::Compression_e _Compression )
{
  oFreeImage bmp = oFreeImage(pImage);
  if (!_Path || !*_Path)
  {
    oErrorSetLast(oERROR_INVALID_PARAMETER);
    return 0;
  }

  FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(_Path);
  if (format == FIF_UNKNOWN)
  {
    oErrorSetLast(oERROR_INVALID_PARAMETER);
    return 0;
  }

  oASSERT((size_t)((DWORD)_SizeofBuffer) == _SizeofBuffer, "Size of buffer too large for underlying implementation.");
  FIMEMORY* pMemory = FreeImage_OpenMemory(static_cast<BYTE*>(_pBuffer), static_cast<DWORD>(_SizeofBuffer));

  size_t written = 0;

  //oSharedLock<oSharedMutex> lock(Mutex);

  if (FreeImage_SaveToMemory(format, bmp, pMemory, GetSaveFlags(format, _Compression)))
    written = FreeImage_TellMemory(pMemory);
  else
    oErrorSetLast(oERROR_IO, "Failed to save %s to memory", _Path);

  FreeImage_CloseMemory(pMemory);

  return written;
}




bool oImage_Impl::Resize(const int2& _NewSize) 
{
  DESC desc;
  GetDesc(&desc);

  if (_NewSize.x == desc.Dimensions.x && _NewSize.y == desc.Dimensions.y)
    return true; // already the correct size


  oFreeImage bmp = oFreeImage(this);
  if (bmp)
  {
    if(_NewSize.x > 0 && _NewSize.y > 0 && (FreeImage_GetWidth(bmp) != static_cast<unsigned int>(_NewSize.x) || FreeImage_GetHeight(bmp) != static_cast<unsigned int>(_NewSize.y)))
    {
      oFreeImage scaledBmp( FreeImage_Rescale(bmp, _NewSize.x, _NewSize.y, FILTER_BICUBIC) );
      if (scaledBmp)
      {
        // get scaled image
        oImageFromFreeImage(bmp, this);
      }
    } 
    return true;
  }

  oErrorSetLast(oERROR_INVALID_PARAMETER, "Unsupported format %s", oAsString(m_desc.Format));

  return false;
}
