// $(header)
#pragma once
#ifndef oImage_h
#define oImage_h

#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>

#include <oBasis/oBuffer.h>

interface oImage : oBuffer
{
  
  typedef enum Compression_e
  {
    NO_COMPRESSION,
    LOW_COMPRESSION,
    MEDIUM_COMPRESSION,
    HIGH_COMPRESSION,
  };

  struct ForceAlpha_t {};

	// If the source data is only RGB, this will pad out the data and fill in an 
	// opaque value for the alpha. If the data includes alpha, that data will be
	// respected and this flag will be a noop.
	static const ForceAlpha_t ForceAlpha;



	struct DESC
	{
		DESC()
			: Format(oSURFACE_B8G8R8A8_UNORM)
			, Pitch(oInvalid)
			, Dimensions(0,0)
		{}

		oSURFACE_FORMAT Format;
		unsigned int Pitch;
		int2 Dimensions;
	};


	virtual bool FlipHorizontal() = 0;
	virtual bool FlipVertical() = 0;

	// Resize the current image to the specified width and height
	virtual bool Resize(const int2& _NewSize) = 0;

	virtual void GetDesc(DESC* _pDesc) const = 0;
	virtual const DESC * GetDesc() const = 0;
	virtual void SetDesc(const DESC* _pDesc, const void * pImageData) = 0;
};

	// Saves the image to the specified path. The file extension is used to 
	// determine the format. Returns the number of bytes written, or 0 if failed.
	// Check oErrorGetLast().
	//virtual bool Save(const char* _Path, COMPRESSION _Compression = NONE) threadsafe = 0;
oAPI bool oImageSave(const oImage * pImage, const char* _Path, oImage::Compression_e _Compression = oImage::NO_COMPRESSION);
oAPI bool oImageSaveDDS(const oImage * pImage, const char* _Path, oImage::Compression_e _Compression  = oImage::NO_COMPRESSION);

	// Saves the image to a memory buffer as if it were a file, the file extension 
	// is used to determine the format. Returns the number of bytes written, or 0
	// if failed. Check oErrorGetLast(). If _pBuffer is NULL, this still returns
	// the size requires for a successful save though nothing would be written.
	//virtual size_t Save(const char* _Path, void* _pBuffer, size_t _SizeofBuffer, COMPRESSION _Compression = NONE) threadsafe = 0;
oAPI size_t oImageSave(const oImage * pImage, const char* _Path, void* _pBuffer, size_t _SizeofBuffer, oImage::Compression_e _Compression = oImage::NO_COMPRESSION);

oAPI struct HBITMAP__* AsBmp(const oImage * pImage); // Returns a copy (HBITMAP). Use DeleteObject() when finished with the HBITMAP.

// Creates an image from an in-memory image file.
oAPI bool oImageCreate(const char* pImageName, const void* _pBuffer, size_t _SizeOfBuffer, oImage** _ppImage);


oAPI bool oImageCreate(const char* pImageName, const oImage * pSrcImage, oImage** _ppImageCopy);

// Creates an image from an in-memory image file. If the source data is RGB,
// an alpha channel will be added with all values as opaque. if the source is
// RGBA, nothing differently is done and the data is preserved.
oAPI bool oImageCreate(const char* pImageName, const void* _pBuffer, size_t _SizeOfBuffer, const oImage::ForceAlpha_t&, oImage** _ppImage);


// Creates an uninitialized oImage from the specified DESC. Pitch is ignored 
// as a creation parameter and only RGB and RGBA formats are currently 
// supported.
oAPI bool oImageCreate(const char* pImageName, const oImage::DESC* _Desc, oImage** _ppImage);

// Populates the specified DESC by reading only the first few bytes of the 
// specified buffer. This means that the buffer specified need only contain the
// file's header information, not the entire file as well.
oAPI bool oImageCreateDesc(const void* _pBuffer, size_t _SizeofBuffer, oImage::DESC* _pDesc);


// Returns true if the specified file can be loaded as an oImage
oAPI bool oImageIsSupportedFormat(const char* _Path);

// Compare the two specified images according to the specified parameters, 
// including generating a 3rd image that is the difference between the two input 
// images. All operations in this function operate in a threadsafe manner and
// call only threadsafe API on the specified images.
oAPI bool oImageCompare(const oImage* _pImage1, const oImage* _pImage2, 
  unsigned int bitTolerance, 
  float & Root_Mean_Square, 
  oImage** _ppDiffImage = nullptr, unsigned int _DiffMultiplier = 1);


#include <oPlatform/oImage.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

// The image compressor takes images in one format and compresses them into
// more real-time friendly formats. 
interface oImageCompressor : oInterface
{
  enum DEVICE_TYPE
  {
    CPU,
    GPU
  };
  struct DESC
  {
    DESC(): deviceType(GPU), pDevice(nullptr), pContext(nullptr)
    {
    }
    DEVICE_TYPE deviceType;

    // optional when specifying GPU compression
    ID3D11Device* pDevice;
    ID3D11DeviceContext * pContext;
  };

  typedef oFUNCTION<void(const oImage* _pSrcImage, oImage* _pDestImage)> callback_t;


  virtual const oImage * GetCompressedImage() const = 0;

  virtual bool SupportsCompression(const oImage::DESC & _SrcImageDesc, oSURFACE_FORMAT _DestFormat) = 0;

  // Compresses the image calling the user back when the compression has completed or failed.  If
  // compression fails for any reason the _pDestImage will be nullptr and the error can be retrieved via
  // oErrorGetLast.
  virtual void CompressTexture(const oImage* _pSrc, oSURFACE_FORMAT _DestFormat, callback_t _Callback) = 0;

  virtual bool Decompress(const oImage* _pSrc, oImage ** _pDest) = 0;
};

oAPI bool oImageCompressorCreate(const oImageCompressor::DESC& _Desc, oImageCompressor** _ppTextureCompressor);
#endif

