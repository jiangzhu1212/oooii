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
// oImage is a trivial load-without-textures-or-video simple utility to load
// things like icons and pngs for screen captures of non-rendering-related 
// things. This module has gone through quite a few cycles of adding not-image
// features and then having them pulled out as they grew in complexity. The
// feature set of oImage is constrained by the aging file formats it supports
// such as ICO and JPG. More complex formats require significantly more complex
// infrastructure from YUV formats that maintain 3 separate pointers to data and 
// BC7+ formats that require GPU processing to make the bake times rational. So
// if you think you should extend the API or feature set of oImage, ask 
// yourself: can it be saved to a JPG? If not, then don't.
#pragma once
#ifndef oImage_h
#define oImage_h

#include <oBasis/oBuffer.h>
#include <oBasis/oInterface.h>
#include <oBasis/oMathTypes.h>
#include <oBasis/oSurface.h>

#define oBug_1911

interface oImage : oBuffer
{
	// !!! DO NOT ADD API WITHOUT REVIEW TO THIS CLASS !!!
	// Add a separate c-style function until it can be reviewed as being an 
	// appropriate addition.

	enum FORMAT
	{
		UNKNOWN,
		RGBA32,
		RGB24,
		BGRA32,
		BGR24,
		R8,
	};

	enum FILE_FORMAT
	{
		UNKNOWN_FILE,
		ICO,
		BMP,
		JPG,
		PNG,
		// NOTE: DDS is save-only. When saving, only the BMP/JPG/PNG feature set is
		// supported. Loading would imply more robust feature support, which is out
		// of scope for oImage.
		DDS,
	};

  enum COMPRESSION_LEVEL
  {
    NO_COMPRESSION,
    LOW_COMPRESSION,
    MEDIUM_COMPRESSION,
    HIGH_COMPRESSION,
  };

	// If the source data is only RGB, this will pad out the data and fill in an 
	// opaque value for the alpha. If the data includes alpha, that data will be
	// respected and this flag will be a noop.
	oDECLARE_FLAG(ForceAlpha);

	// If the source data is RGBA, this will ignore/drop alpha and operate on RGB
	// data only.
	oDECLARE_FLAG(ForceNoAlpha);
	
	// If specified, the copy operation will copy a top-to-bottom source into this
	// as a bottom-to-top bitmap image
	oDECLARE_FLAG(FlipVertical);

	struct DESC
	{
		DESC()
			: Format(BGRA32)
			, RowPitch(oInvalid)
			, Dimensions(oInvalid, oInvalid)
		{}

		FORMAT Format;
		unsigned int RowPitch;
		int2 Dimensions;
	};

	virtual void GetDesc(DESC* _pDesc) const threadsafe = 0;

	// Assumes the source buffer is the same format as this oImage and copies its
	// bitmap data into this oImage's store without changing topology information.
	virtual void CopyData(const void* _pSourceBuffer, size_t _SourceRowPitch) threadsafe = 0;
	virtual void CopyData(const void* _pSourceBuffer, size_t _SourceRowPitch, const FlipVerticalFlag&) threadsafe = 0;
	
	// Assumes the destination buffer is the same format and sized correctly and 
	// copies this oImage's bitmap data into the destination buffer.
	virtual void CopyDataTo(void* _pDestinationBuffer, size_t _DestinationRowPitch) const threadsafe = 0;
	virtual void CopyDataTo(void* _pDestinationBuffer, size_t _DestinationRowPitch, const FlipVerticalFlag&) const threadsafe = 0;

#ifdef oBug_1911
	// @oooii-tony: Inclusion of this API is temporary AND SHOULD NOT BE TAKEN AS
	// A MODEL TO ADD FORMAT SWIZZLING OF ANY KIND TO THIS CLASS. This is a hack
	// to get around Window's flipped bitmaps while usage is scrubbed to have more
	// direct conversions. Prefer using CopyData with the FlipVertical flag to 
	// do the flip while moving data from another source into this oImage.
	virtual void HACKFlipVertical() threadsafe = 0;

#endif
	// @oooii-tony: Inclusion of this API is temporary AND SHOULD NOT BE TAKEN AS
	// A MODEL TO ADD FORMAT SWIZZLING OF ANY KIND TO THIS CLASS. This is a hack
	// to get around current usage but we'll be coming back to this to reevaluate
	// this API.
	virtual bool HACKResize(const int2& _NewSize) threadsafe = 0;

	// Removes alpha channel
	virtual void HACKTo24Bit() threadsafe = 0;

	// Adds an opaque alpha channel
	virtual void HACKTo32Bit() threadsafe = 0;
};

// _____________________________________________________________________________
// Type conversion code

// Conversions so oSurface API can be used while still retaining the notion that
// oImage is a small, small subset of oSurfaces, not the full wrapper for all 
// that oSurface can do.
oAPI oImage::FORMAT oImageFormatFromSurfaceFormat(oSURFACE_FORMAT _Format);
oAPI oSURFACE_FORMAT oImageFormatToSurfaceFormat(oImage::FORMAT _Format);

// Returns the format as interpreted from the extension of the file
oAPI oImage::FILE_FORMAT oImageFormatFromExtension(const char* _URIReference);

inline unsigned int oImageCalcRowPitch(oImage::FORMAT _Format, int _Width) { return oSurfaceCalcRowPitch(oImageFormatToSurfaceFormat(_Format), _Width); }
inline unsigned int oImageGetBitSize(oImage::FORMAT _Format) { return oSurfaceGetBitSize(oImageFormatToSurfaceFormat(_Format)); }
inline unsigned int oImageGetSize(oImage::FORMAT _Format) { return oSurfaceGetSize(oImageFormatToSurfaceFormat(_Format)); }
inline unsigned int oImageCalcSize(oImage::FORMAT _Format, const int2& _Dimensions) { return oSurfaceCalcSize(oImageFormatToSurfaceFormat(_Format), _Dimensions); }
inline bool oImageIsAlphaFormat(oImage::FORMAT _Format) { return oSurfaceIsAlphaFormat(oImageFormatToSurfaceFormat(_Format)); }

// _____________________________________________________________________________
// Creation code (image format parsing)

// @oooii-tony: Ideally oImage would be oBasis code, but as long as it depends
// on FreeImage for file I/O it must be oPlatform code. Is it worth separating
// the I/O code out? Maybe it'll seem more worthwhile when we fill out 
// conversions, but a lot of those would make sense to GPU accelerate...

// Populates the specified DESC by reading only the first few bytes of the 
// specified buffer. This means that the buffer specified need only contain the
// file's header information, not all the color data. This is most useful in
// streaming systems that can quickly read the first 1k or so and start building
// system buffers, then stream directly into those buffers.
oAPI bool oImageGetDesc(const void* _pBuffer, size_t _SizeofBuffer, oImage::DESC* _pDesc);

// Creates an image from an in-memory file image. (i.e. _pBuffer is the result
// of a full fread of a file). _Name can be whatever label is appropriate, but
// is often the source URI. Typically if an RGB is loaded, it will remain RGB. 
// If RGBA is loaded it will remain RGBA. If ForceAlpha is specified it will 
// noop on RGBA data, but will force an opaque channel for RGB source data.
oAPI bool oImageCreate(const char* _Name, const void* _pBuffer, size_t _SizeOfBuffer, oImage** _ppImage);
oAPI bool oImageCreate(const char* _Name, const void* _pBuffer, size_t _SizeOfBuffer, const oImage::ForceAlphaFlag&, oImage** _ppImage);

// Creates an uninitialized oImage from the specified DESC. Ensure 
// oSurfaceCalcRowPitch is used to specify the pitch or this function will 
// return false in error.
oAPI bool oImageCreate(const char* _Name, const oImage::DESC& _Desc, oImage** _ppImage);

// Creates a copy of the specified _pSourceImage in the form of a Windows 
// platform HBITMAP. Use DeleteObject() when finished with the HBITMAP.
oAPI bool oImageCreateBitmap(const threadsafe oImage* _pSourceImage, struct HBITMAP__** _ppBitmap);

// _____________________________________________________________________________
// Load/Save (heavy on platform I/O)

// Saves the specified _pImage to the specified buffer in file format (as 
// opposed to runtime format). At this time, this function will fail if the 
// buffer is not large enough to hold the specified oImage.
oAPI size_t oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, void* _pBuffer, size_t _SizeofBuffer);
oAPI size_t oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, const oImage::ForceNoAlphaFlag&, void* _pBuffer, size_t _SizeofBuffer);

oAPI bool oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, const char* _Path);
oAPI bool oImageSave(const threadsafe oImage* _pImage, oImage::FILE_FORMAT _Format, oImage::COMPRESSION_LEVEL _CompressionLevel, const oImage::ForceNoAlphaFlag&, const char* _Path);
inline bool oImageSave(const threadsafe oImage* _pImage, const char* _Path) { return oImageSave(_pImage, oImage::UNKNOWN_FILE, oImage::NO_COMPRESSION, oImage::ForceNoAlpha, _Path); }

oAPI bool oImageLoad(const char* _Path, oImage** _ppImage);
oAPI bool oImageLoad(const char* _Path, const oImage::ForceAlphaFlag&, oImage** _ppImage);

// Compare the two specified images according to the specified parameters, 
// including generating a 3rd image that is the difference between the two input 
// images. This returns false if there are topological differences in the images
// that prevents a per-pixel compare (differing format/dimensions) and true if
// the pixel compare was able to complete. So images are the same if this 
// function returns true AND RootMeanSquare is below a threashold the client 
// code finds acceptable.
oAPI bool oImageCompare(const threadsafe oImage* _pImage1, const threadsafe oImage* _pImage2, unsigned int _BitTolerance, float* _pRootMeanSquare = nullptr, oImage** _ppDiffImage = nullptr, unsigned int _DiffMultiplier = 1);

#endif
