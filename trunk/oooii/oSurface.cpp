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
#include <oooii/oWindows.h>
#include <oooii/oSurface.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oStddef.h>
#include <oooii/oMath.h>
#include <string.h>

#define LAST_FORMAT oSurface::BC7_UNORM_SRGB

struct FORMAT_DESC
{
	const char* String;
	unsigned int Size;
	unsigned short NumChannels;
	bool IsBlockCompressed;
	bool IsUNORM;
};

static const FORMAT_DESC sFormatDescs[] = 
{
	{ "UNKNOWN", 0, false, false },
	{ "R32G32B32A32_TYPELESS", 4 * sizeof(float), 4, false, false },
	{ "R32G32B32A32_FLOAT", 4 * sizeof(float), 4, false, false },
	{ "R32G32B32A32_UINT", 4 * sizeof(int), 4, false, false },
	{ "R32G32B32A32_SINT", 4 * sizeof(int), 4, false, false },
	{ "R32G32B32_TYPELESS", 3 * sizeof(int), 3, false, false },
	{ "R32G32B32_FLOAT", 3 * sizeof(float), 3, false, false },
	{ "R32G32B32_UINT", 4 * sizeof(int), 3, false, false },
	{ "R32G32B32_SINT", 4 * sizeof(int), 3, false, false },
	{ "R16G16B16A16_TYPELESS", 4 * sizeof(short), 4, false, false },
	{ "R16G16B16A16_FLOAT", 4 * sizeof(short), 4, false, false },
	{ "R16G16B16A16_UNORM", 4 * sizeof(short), 4, false, true },
	{ "R16G16B16A16_UINT", 4 * sizeof(short), 4, false, false },
	{ "R16G16B16A16_SNORM", 4 * sizeof(short), 4, false, false },
	{ "R16G16B16A16_SINT", 4 * sizeof(short), 4, false, false },
	{ "R32G32_TYPELESS", 2 * sizeof(float), 2, false, false },
	{ "R32G32_FLOAT", 2 * sizeof(float), 2, false, false },
	{ "R32G32_UINT", 2 * sizeof(int), 2, false, false },
	{ "R32G32_SINT", 2 * sizeof(int), 2, false, false },
	{ "R32G8X24_TYPELESS", 2 * sizeof(float), 3, false, false },
	{ "D32_FLOAT_S8X24_UINT", 2 * sizeof(int), 3, false, false },
	{ "R32_FLOAT_X8X24_TYPELESS", 2 * sizeof(float), 3, false, false },
	{ "X32_TYPELESS_G8X24_UINT", 2 * sizeof(int), 3, false, false },
	{ "R10G10B10A2_TYPELESS", sizeof(int), 4, false, false },
	{ "R10G10B10A2_UNORM", sizeof(int), 4, false, true },
	{ "R10G10B10A2_UINT", sizeof(int), 4, false, false },
	{ "R11G11B10_FLOAT", sizeof(float), 3, false, false },
	{ "R8G8B8A8_TYPELESS", sizeof(int), 4, false, false },
	{ "R8G8B8A8_UNORM", sizeof(int), 4, false, true },
	{ "R8G8B8A8_UNORM_SRGB", sizeof(int), 4, false, true },
	{ "R8G8B8A8_UINT", sizeof(int), 4, false, false },
	{ "R8G8B8A8_SNORM", sizeof(int), 4, false, false },
	{ "R8G8B8A8_SINT", sizeof(int), 4, false, false },
	{ "R16G16_TYPELESS", 2 * sizeof(short), 2, false, false },
	{ "R16G16_FLOAT", 2 * sizeof(short), 2, false, false },
	{ "R16G16_UNORM", 2 * sizeof(short), 2, false, true },
	{ "R16G16_UINT", 2 * sizeof(short), 2, false, false },
	{ "R16G16_SNORM", 2 * sizeof(short), 2, false, false },
	{ "R16G16_SINT", 2 * sizeof(short), 2, false, false },
	{ "R32_TYPELESS", sizeof(float), 1, false, false },
	{ "D32_FLOAT", sizeof(float), 1, false, false },
	{ "R32_FLOAT", sizeof(float), 1, false, false },
	{ "R32_UINT", sizeof(int), 1, false, false },
	{ "R32_SINT", sizeof(int), 1, false, false },
	{ "R24G8_TYPELESS", sizeof(float), 2, false, false },
	{ "D24_UNORM_S8_UINT", sizeof(float), 2, false, true },
	{ "R24_UNORM_X8_TYPELESS", sizeof(float), 2, false, true },
	{ "X24_TYPELESS_G8_UINT", sizeof(float), 2, false, false },
	{ "R8G8_TYPELESS", 2 * sizeof(char), 2, false, false },
	{ "R8G8_UNORM", 2 * sizeof(char), 2, false, true },
	{ "R8G8_UINT", 2 * sizeof(char), 2, false, false },
	{ "R8G8_SNORM", 2 * sizeof(char), 2, false, false },
	{ "R8G8_SINT", 2 * sizeof(char), 2, false, false },
	{ "R16_TYPELESS", sizeof(short), 1, false, false },
	{ "R16_FLOAT", sizeof(short), 1, false, false },
	{ "D16_UNORM", sizeof(short), 1, false, true },
	{ "R16_UNORM", sizeof(short), 1, false, true },
	{ "R16_UINT", sizeof(short), 1, false, false },
	{ "R16_SNORM", sizeof(short), 1, false, false },
	{ "R16_SINT", sizeof(short), 1, false, false },
	{ "R8_TYPELESS", sizeof(char), 1, false, false },
	{ "R8_UNORM", sizeof(char), 1, false, true },
	{ "R8_UINT", sizeof(char), 1, false, false },
	{ "R8_SNORM", sizeof(char), 1, false, false },
	{ "R8_SINT", sizeof(char), 1, false, false },
	{ "A8_UNORM", sizeof(char), 1, false, true },
	{ "R1_UNORM", 1, 1, false, true },
	{ "R9G9B9E5_SHAREDEXP", sizeof(int), 4, false, false },
	{ "R8G8_B8G8_UNORM", sizeof(int), 4, false, true },
	{ "G8R8_G8B8_UNORM", sizeof(int), 4, false, true },
	{ "BC1_TYPELESS", 8, 3, true, false },
	{ "BC1_UNORM", 8, 3, true, true },
	{ "BC1_UNORM_SRGB", 8, 3, true, true },
	{ "BC2_TYPELESS", 0, 4, true, false },
	{ "BC2_UNORM", 0, 4, true, true },
	{ "BC2_UNORM_SRGB", 0, 4, true, true },
	{ "BC3_TYPELESS", 0, 4, true, false },
	{ "BC3_UNORM", 0, 4, true, true },
	{ "BC3_UNORM_SRGB", 0, 4, true, true },
	{ "BC4_TYPELESS", 0, 1, true, false },
	{ "BC4_UNORM", 0, 1, true, true },
	{ "BC4_SNORM", 0, 1, true, false },
	{ "BC5_TYPELESS", 0, 2, true, false },
	{ "BC5_UNORM", 0, 2, true, true },
	{ "BC5_SNORM", 0, 2, true, false },
	{ "B5G6R5_UNORM", sizeof(short), 3, false, true },
	{ "B5G5R5A1_UNORM", sizeof(short), 4, false, true },
	{ "B8G8R8A8_UNORM", sizeof(int), 4, false, true },
	{ "B8G8R8X8_UNORM", sizeof(int), 4, false, true },
	{ "R10G10B10_XR_BIAS_A2_UNORM", sizeof(int), 4, false, true },
	{ "B8G8R8A8_TYPELESS", sizeof(int), 4, false, false },
	{ "B8G8R8A8_UNORM_SRGB", sizeof(int), 4, false, true },
	{ "B8G8R8X8_TYPELESS", sizeof(int), 4, false, false },
	{ "B8G8R8X8_UNORM_SRGB", sizeof(int), 4, false, true },
	{ "BC6H_TYPELESS", 0, 3, true, false }, // @oooii-tony: num channels on BC6 and BC7 
	{ "BC6H_UF16", 0, 3, true, false },     // formats are suspect. I can't find a real spec 
	{ "BC6H_SF16", 0, 3, true, false },     // on what's inside, and BC7 seems to support 3- 
	{ "BC7_TYPELESS", 0, 4, true, false },  // and 4-channel color.
	{ "BC7_UNORM", 0, 4, true, true },
	{ "BC7_UNORM_SRGB", 0, 4, true, true },
};
oSTATICASSERT(oCOUNTOF(sFormatDescs) == oSurface::NUM_FORMATS);

#if oDXVER >= oDXVER_10
unsigned int oSurface::GetDXGIFormat(FORMAT _Format)
{
	// @oooii-tony: For now, these are the same exact thing
		return (unsigned int)(DXGI_FORMAT)_Format;
}
#endif

const char* oAsString(const oSurface::FORMAT& _Format)
{
	return (_Format <= LAST_FORMAT) ? sFormatDescs[_Format].String : "UNKNOWN";
}

oSurface::FORMAT oSurface::GetFromString(const char* _EnumString)
{
	for (size_t i = 0; i < oCOUNTOF(sFormatDescs); i++)
		if (!strcmp(_EnumString, sFormatDescs[i].String))
			return static_cast<oSurface::FORMAT>(i);
	return UNKNOWN;
}

bool oSurface::IsBlockCompressedFormat(FORMAT _Format)
{
	return (_Format <= LAST_FORMAT) ? sFormatDescs[_Format].IsBlockCompressed : false;
}

bool oSurface::IsUNORM( FORMAT _Format )
{
	return (_Format <= LAST_FORMAT) ? sFormatDescs[_Format].IsUNORM : false;
}

unsigned int oSurface::GetNumChannels(FORMAT _Format)
{
	return (_Format <= LAST_FORMAT) ? sFormatDescs[_Format].NumChannels : 0;
}

unsigned int oSurface::GetSize(FORMAT _Format)
{
	return (_Format <= LAST_FORMAT) ? sFormatDescs[_Format].Size : 0;
}

unsigned int oSurface::GetBitSize(FORMAT _Format)
{
	if (_Format == R1_UNORM) return 1;
	return 8 * GetSize(_Format);
}

unsigned int oSurface::CalcRowPitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _MipLevel)
{
	oASSERT(_Format != UNKNOWN, "Unknown surface format passed to GetRowPitch");
	unsigned int w = CalcMipDimension(_Format, _Mip0Width, _MipLevel);
	unsigned int s = GetSize(_Format);
	return static_cast<unsigned int>(oByteAlign(w * s, sizeof(int)));
}

unsigned int oSurface::CalcLevelPitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _MipLevel)
{
	return CalcRowPitch(_Format, _Mip0Height, _MipLevel) * CalcNumRows(_Format, _Mip0Height, _MipLevel);
}

unsigned int oSurface::CalcSlicePitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height)
{
	unsigned int pitch = 0;
	const unsigned int nMips = CalcMipCount(_Mip0Width, _Mip0Height);
	for (unsigned int i = 0; i < nMips; i++)
		pitch += CalcLevelPitch(_Format, _Mip0Width, _Mip0Height, i);
	return pitch;
}

unsigned int oSurface::CalcMipDimension(FORMAT _Format, unsigned int _Mip0Dimension, unsigned int _MipLevel)
{
	oASSERT(_Format != UNKNOWN, "Unknown surface format passed to CalcMipDimension");
	oASSERT(_MipLevel == 0 || oIsPow2(_Mip0Dimension), "Mipchain dimensions must be a power of 2");
	unsigned int d = __max(1, _Mip0Dimension >> _MipLevel);
	return IsBlockCompressedFormat(_Format) ? static_cast<unsigned int>(oByteAlign(d, 4)) : d;
}

unsigned int oSurface::CalcNumRows(FORMAT _Format, unsigned int _Mip0Height, unsigned int _MipLevel)
{
	unsigned int heightInPixels = CalcMipDimension(_Format, _Mip0Height, _MipLevel);
	return IsBlockCompressedFormat(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

unsigned int oSurface::CalcMipCount(unsigned int _Mip0Width, unsigned int _Mip0Height)
{
	unsigned int n = 0, w, h;
	do
	{
		// For this calculation, format doesn't matter
		w = CalcMipDimension(R8G8B8A8_TYPELESS, _Mip0Width, n);
		h = CalcMipDimension(R8G8B8A8_TYPELESS, _Mip0Height, n);
		n++;
	} while (w != 1 || h != 1);

	return n;
}

unsigned int oSurface::CalcBufferSize(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices)
{
	// http://www.cs.umbc.edu/~olano/s2006c03/ch02.pdf
	// "Texture3D behaves identically to a Texture2DArray with n array 
	// slices where n is the depth (3rd dimension) of the Texture3D."
	return CalcSlicePitch(_Format, _Mip0Width, _Mip0Height) * _NumSlices;
}

unsigned int oSurface::CalcSubresourceSize(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices, unsigned int _Subresource)
{
	unsigned int nMips = CalcMipCount(_Mip0Width, _Mip0Height);
	unsigned int slicePitch = CalcSlicePitch(_Format, _Mip0Width, _Mip0Height);

	unsigned int slice0, mipLevel0, slice1, mipLevel1;
	UnpackSubresource(_Subresource, nMips, &mipLevel0, &slice0);
	UnpackSubresource(_Subresource + 1, nMips, &mipLevel1, &slice1);
	
	unsigned int offset0 = (slice0 * slicePitch) + CalcLevelOffset(_Format, _Mip0Width, _Mip0Height, mipLevel0);
	unsigned int offset1 = (slice1 * slicePitch) + CalcLevelOffset(_Format, _Mip0Width, _Mip0Height, mipLevel1);
	
	return offset1 - offset0;
}

unsigned int oSurface::CalcLevelOffset(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _MipLevel)
{
	unsigned int offset = 0;
	for (unsigned int i = 0; i < _MipLevel; i++)
		offset += CalcLevelPitch(_Format, _Mip0Width, _Mip0Height, i);
	return offset;
}

unsigned int oSurface::CalcBufferOffset(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumSlices, unsigned int _Subresource)
{
	unsigned int nMips = CalcMipCount(_Mip0Width, _Mip0Height);
	unsigned int slice, mipLevel;
	UnpackSubresource(_Subresource, nMips, &mipLevel, &slice);
	unsigned int slicePitch = CalcSlicePitch(_Format, _Mip0Width, _Mip0Height);
	return (slice * slicePitch) + CalcLevelOffset(_Format, _Mip0Width, _Mip0Height, mipLevel);
}

unsigned int oSurface::CalculateSize(const DESC* _pDesc)
{
	// Mips and slices not yet supported.
	oASSERT(_pDesc->NumMips <= 1, "");
	oASSERT(_pDesc->NumSlices <= 1, "");

	if (!_pDesc) return 0;
	
	unsigned int rowPitch = _pDesc->RowPitch;
	
	// Calculate assuming the simple case of a contiguous set of word-aligned scanlines
	if (!rowPitch)
		rowPitch = CalcRowPitch(_pDesc->Format, _pDesc->Width);

	// Block-compressed formats cover 4x4 blocks in their stride, so from a bytes 
	// perspective, the width seems wider, and the height should be shorter.
	unsigned int heightAdjustment = IsBlockCompressedFormat(_pDesc->Format) ? 4 : 1;
	unsigned int blockAlignedHeight = IsBlockCompressedFormat(_pDesc->Format) ? static_cast<unsigned int>(oByteAlign(_pDesc->Height, 4)) : _pDesc->Height;
	return rowPitch * (blockAlignedHeight / heightAdjustment);
}

#if defined (_WIN32) || defined (_WIN64)

void oSurface::GetBMI(void** _ppBMIVoid, const DESC* _pDesc, void* (*_Allocate)(size_t _Size), bool _FlipVertically, oColor _Monochrome8Zero, oColor _Monochrome8One)
{
	BITMAPINFO** _ppBMI = (BITMAPINFO**)_ppBMIVoid;

	if (_pDesc && _ppBMI)
	{
		const WORD bmiBitCount = (WORD)GetBitSize(_pDesc->Format);

		oASSERT(*_ppBMI || _Allocate, "If no pre-existing BITMAPINFO is specified, then an _Allocate function is required.");

		oASSERT(!IsBlockCompressedFormat(_pDesc->Format), "block compressed formats not supported by BITMAPINFO");
		const unsigned int pitch = _pDesc->RowPitch ? _pDesc->RowPitch : CalcRowPitch(_pDesc->Format, _pDesc->Width);

		size_t bmiSize = GetBMISize(_pDesc->Format);

		if (!*_ppBMI)
			*_ppBMI = (BITMAPINFO*)_Allocate(bmiSize);
		BITMAPINFO* pBMI = *_ppBMI;

		pBMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		pBMI->bmiHeader.biBitCount = bmiBitCount;
		pBMI->bmiHeader.biClrImportant = 0;
		pBMI->bmiHeader.biClrUsed = 0;
		pBMI->bmiHeader.biCompression = BI_RGB;
		pBMI->bmiHeader.biHeight = (_FlipVertically ? -1 : 1) * (LONG)_pDesc->Height;
		pBMI->bmiHeader.biWidth = _pDesc->Width;
		pBMI->bmiHeader.biPlanes = 1;
		pBMI->bmiHeader.biSizeImage = pitch * _pDesc->Height;
		pBMI->bmiHeader.biXPelsPerMeter = 0;
		pBMI->bmiHeader.biYPelsPerMeter = 0;

		if (bmiBitCount == 8)
		{
			// BMI doesn't understand 8-bit monochrome, so create a monochrome palette
			unsigned int r,g,b,a;
			oDecomposeColor(_Monochrome8Zero, &r, &g, &b, &a);
			float4 c0(oUBYTEAsUNORM(r), oUBYTEAsUNORM(g), oUBYTEAsUNORM(b), oUBYTEAsUNORM(a));

			oDecomposeColor(_Monochrome8One, &r, &g, &b, &a);
			float4 c1(oUBYTEAsUNORM(r), oUBYTEAsUNORM(g), oUBYTEAsUNORM(b), oUBYTEAsUNORM(a));

			for (size_t i = 0; i < 256; i++)
			{
				float4 c = lerp(c0, c1, oUBYTEAsUNORM(i));
				RGBQUAD& q = pBMI->bmiColors[i];
				q.rgbRed = oUNORMAsUBYTE(c.x);
				q.rgbGreen = oUNORMAsUBYTE(c.y);
				q.rgbBlue = oUNORMAsUBYTE(c.z);
				q.rgbReserved = oUNORMAsUBYTE(c.w);
			}
		}
	}
}

size_t oSurface::GetBMISize(FORMAT _Format)
{
	return GetBitSize(_Format) == 8 ? (sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 255) : sizeof(BITMAPINFO);
}

void oSurface::convert_B8G8R8A8_UNORM_to_YUV420( const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBASrc, const size_t _pRGBAPitch, YUV420* _pYUVDst )
{
	// Convert from RGB to YUV
	const unsigned int HalfWidth = _Width / 2;
	size_t TempSz = sizeof( int ) * _Width;
	int* uTemp = (int*)oSTACK_ALLOC( TempSz, 16 );
	int* vTemp = uTemp + HalfWidth;

	memset( uTemp, 0, TempSz );

	unsigned char* oRESTRICT pDestY = _pYUVDst->pY;
	unsigned char* oRESTRICT pDestU = _pYUVDst->pU;
	unsigned char* oRESTRICT pDestV = _pYUVDst->pV;

	size_t YPitch = _pYUVDst->YPitch;
	size_t UVPitch = _pYUVDst->UVPitch;

	for( unsigned int y = 0; y < _Height; ++y )
	{
		// RGB src
		const unsigned int* oRESTRICT RGBLine = oByteAdd( (const unsigned int* oRESTRICT)_pSrcRGBASrc, y * _pRGBAPitch );

		// Planar destinations
		unsigned char* oRESTRICT YLine = oByteAdd( pDestY, y * YPitch );
		
		for( unsigned int x = 0; x < _Width; ++x )
		{
			unsigned int pixel = RGBLine[x];
			int R = ( pixel >> 16 ) & 0x000000FF;
			int G = ( pixel >> 8 ) & 0x000000FF;
			int B = ( pixel & 0x000000FF );

			int Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
			int U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
			int V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;

			YLine[x] = (unsigned char)( clamp( Y, 0, 255 ) );

			int xHalf = x / 2;
			uTemp[xHalf] += U / 4;
			vTemp[xHalf] += V / 4;
		}

		if( y % 2 )
		{
			unsigned int yHalf = y / 2;
			unsigned char* ULine = oByteAdd( pDestU, yHalf * UVPitch );
			unsigned char* VLine = oByteAdd( pDestV, yHalf * UVPitch );

			for( unsigned int x = 0; x < HalfWidth; ++x )
			{
				ULine[x] = (unsigned char) clamp( uTemp[x], 0, 255 );
				VLine[x] = (unsigned char) clamp( vTemp[x], 0, 255 );
			}

			memset( uTemp, 0, TempSz );
		}
	}
}

void oSurface::convert_YUV420_to_B8G8R8A8_UNORM( const unsigned int _Width, const unsigned int _Height, const YUV420& _YUVSrc, unsigned char* _pSrcRGBADst, const size_t _pRGBAPitch )
{
	const unsigned char* oRESTRICT pDestY = _YUVSrc.pY;
	const unsigned char* oRESTRICT pDestU = _YUVSrc.pU;
	const unsigned char* oRESTRICT pDestV = _YUVSrc.pV;

	size_t YPitch = _YUVSrc.YPitch;
	size_t UVPitch = _YUVSrc.UVPitch;

	for( unsigned int y = 0; y < _Height; ++y )
	{
		unsigned int yHalf = y / 2;

		unsigned int* oRESTRICT scanline = oByteAdd( (unsigned int* oRESTRICT)_pSrcRGBADst, y * _pRGBAPitch );
		const unsigned char* oRESTRICT YScanline = oByteAdd( pDestY, YPitch* y );

		const unsigned char* oRESTRICT UScanline = oByteAdd( pDestU, + UVPitch * yHalf );
		const unsigned char* oRESTRICT VScanline =  oByteAdd( pDestV, + UVPitch * yHalf );;

		for( unsigned int x = 0; x < _Width; ++x )
		{
			unsigned int xHalf = x / 2;

			int Y = YScanline[x];
			int U = UScanline[xHalf];
			int V = VScanline[xHalf];

			int C = Y - 16;
			int	D = U - 128;
			int	E = V - 128;

			int R = clamp(( 298 * C           + 409 * E + 128) >> 8, 0, 255 );
			int G = clamp(( 298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255 );
			int B = clamp(( 298 * C + 516 * D           + 128) >> 8, 0, 255 );

			unsigned int pixel = B;
			pixel |= G << 8;
			pixel |= R << 16;

			scanline[x] = pixel;
		}
	}
}


#endif
