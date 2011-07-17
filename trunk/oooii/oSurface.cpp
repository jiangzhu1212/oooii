// $(header)
#include <oooii/oSurface.h>
#include <oooii/oAssert.h>
#include <oooii/oByte.h>
#include <oooii/oStddef.h>
#include <oooii/oMath.h>
#include <oooii/oMemory.h>
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
	{ "BC2_TYPELESS", 16, 4, true, false },
	{ "BC2_UNORM", 16, 4, true, true },
	{ "BC2_UNORM_SRGB", 16, 4, true, true },
	{ "BC3_TYPELESS", 16, 4, true, false },
	{ "BC3_UNORM", 16, 4, true, true },
	{ "BC3_UNORM_SRGB", 16, 4, true, true },
	{ "BC4_TYPELESS", 8, 1, true, false },
	{ "BC4_UNORM", 8, 1, true, true },
	{ "BC4_SNORM", 8, 1, true, false },
	{ "BC5_TYPELESS", 16, 2, true, false },
	{ "BC5_UNORM", 16, 2, true, true },
	{ "BC5_SNORM", 16, 2, true, false },
	{ "B5G6R5_UNORM", sizeof(short), 3, false, true },
	{ "B5G5R5A1_UNORM", sizeof(short), 4, false, true },
	{ "B8G8R8A8_UNORM", sizeof(int), 4, false, true },
	{ "B8G8R8X8_UNORM", sizeof(int), 4, false, true },
	{ "R10G10B10_XR_BIAS_A2_UNORM", sizeof(int), 4, false, true },
	{ "B8G8R8A8_TYPELESS", sizeof(int), 4, false, false },
	{ "B8G8R8A8_UNORM_SRGB", sizeof(int), 4, false, true },
	{ "B8G8R8X8_TYPELESS", sizeof(int), 4, false, false },
	{ "B8G8R8X8_UNORM_SRGB", sizeof(int), 4, false, true },
	{ "BC6H_TYPELESS", 16, 3, true, false },
	{ "BC6H_UF16", 16, 3, true, false },
	{ "BC6H_SF16", 16, 3, true, false },
	{ "BC7_TYPELESS", 16, 4, true, false },
	{ "BC7_UNORM", 16, 4, true, true },
	{ "BC7_UNORM_SRGB", 16, 4, true, true },
	{ "R8G8B8_UNORM", 3 * sizeof(char), 3, false, true },
	{ "YUV420_UNORM", 3 * sizeof(char), 3, false, true },
};
oSTATICASSERT(oCOUNTOF(sFormatDescs) == oSurface::NUM_FORMATS);

#if defined(_WIN32) || defined(_WIN64)
	int oSurface::GetPlatformFormat(FORMAT _Format)
	{
		// @oooii-tony: For now, oSurface::FORMAT and DXGI_FORMAT are the same thing.
		return static_cast<int>(_Format);
	}
#else
	#error Unsupported platform (oSurface::GetPlatformFormat())
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

bool oSurface::IsDepthFormat(FORMAT _Format)
{
	bool result = false;

	switch (_Format)
	{
		case D32_FLOAT:
		case R32_TYPELESS:
		case D24_UNORM_S8_UINT:
		case R24_UNORM_X8_TYPELESS:
		case D32_FLOAT_S8X24_UINT:
		case R32_FLOAT_X8X24_TYPELESS:
		case D16_UNORM:
		case R16_TYPELESS:
			result = true;
		default:
			break;
	}

	return result;
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
	if (IsBlockCompressedFormat(_Format)) // because the atom is a 4x4 block
		w /= 4;
	unsigned int s = GetSize(_Format);
	return static_cast<unsigned int>(oByteAlign(w * s, sizeof(int)));
}

unsigned int oSurface::CalcLevelPitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _MipLevel)
{
	return CalcRowPitch(_Format, _Mip0Width, _MipLevel) * CalcNumRows(_Format, _Mip0Height, _MipLevel);
}

unsigned int oSurface::CalcSlicePitch(FORMAT _Format, unsigned int _Mip0Width, unsigned int _Mip0Height, unsigned int _NumMips)
{
	unsigned int pitch = 0;
	const unsigned int nMips = 0 == _NumMips ? CalcMipCount(_Mip0Width, _Mip0Height) : _NumMips;
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

struct SourceYModNormal
{
	unsigned int operator()(unsigned int _y, unsigned int _height) { return _y;}
};

struct SourceYModFlip
{
	unsigned int operator()(unsigned int _y, unsigned int _height) { return (_height - _y -1);}
};

struct ExtractRGBFromBGR
{
	void operator()(int &_r, int &_g, int &_b, const unsigned char* oRESTRICT _RGBLine, unsigned int _x) 
	{
		_r = _RGBLine[3*_x+2];
		_g = _RGBLine[3*_x+1];
		_b = _RGBLine[3*_x];
	}
};

struct ExtractRGBFromBGRA
{
	void operator()(int &_r, int &_g, int &_b, const unsigned char* oRESTRICT _RGBLine, unsigned int _x) 
	{
		_r = _RGBLine[4*_x+2];
		_g = _RGBLine[4*_x+1];
		_b = _RGBLine[4*_x];
	}
};

template<typename SourceYMod, typename ExtractRGB>
void convert_B8G8R8_UNORM_to_YUV420(const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBSrc,
	const size_t _pRGBPitch, oSurface::YUV420* _pYUVDst, SourceYMod _sourceYMod, ExtractRGB _extractRGB )
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
		const unsigned char* oRESTRICT RGBLine = oByteAdd( (const unsigned char* oRESTRICT)_pSrcRGBSrc, _sourceYMod(y, _Height) * _pRGBPitch );

		// Planar destinations
		unsigned char* oRESTRICT YLine = oByteAdd( pDestY, y * YPitch );

		for( unsigned int x = 0; x < _Width; ++x )
		{
			int R, G, B;
			_extractRGB(R, G, B, RGBLine, x);

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

void oSurface::convert_B8G8R8_UNORM_to_YUV420(const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBSrc, const size_t _pRGBPitch, YUV420* _pYUVDst, bool _yflip )
{
	if(_yflip)
		convert_B8G8R8_UNORM_to_YUV420(_Width, _Height, _pSrcRGBSrc, _pRGBPitch, _pYUVDst, SourceYModFlip(), ExtractRGBFromBGR());
	else
		convert_B8G8R8_UNORM_to_YUV420(_Width, _Height, _pSrcRGBSrc, _pRGBPitch, _pYUVDst, SourceYModNormal(), ExtractRGBFromBGR());
}

void oSurface::convert_B8G8R8A8_UNORM_to_YUV420( const unsigned int _Width, const unsigned int _Height, const unsigned char* _pSrcRGBASrc, const size_t _pRGBAPitch, YUV420* _pYUVDst, bool _yflip )
{
	if(_yflip)
		convert_B8G8R8_UNORM_to_YUV420(_Width, _Height, _pSrcRGBASrc, _pRGBAPitch, _pYUVDst, SourceYModFlip(), ExtractRGBFromBGRA());
	else
		convert_B8G8R8_UNORM_to_YUV420(_Width, _Height, _pSrcRGBASrc, _pRGBAPitch, _pYUVDst, SourceYModNormal(), ExtractRGBFromBGRA());
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
			pixel |= 0xff << 24;

			scanline[x] = pixel;
		}
	}
}






#include <oooii/oRef.h>
#include <oooii/oD3D11.h>
#include <oooii/oD3DX11.h>

bool oSurface::ConvertSurface(const DESC& _srcDesc, const unsigned char* _pSrcSurface, const DESC& _dstDesc, unsigned char* _pDstSurface)
{
	HRESULT hr;

	D3D_FEATURE_LEVEL FeatureLevel;
	oRef<ID3D11Device> D3DDevice;
	oRef<ID3D11DeviceContext> D3DDeviceContext;
	hr = oD3D11::Singleton()->D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&D3DDevice,
		&FeatureLevel,
		&D3DDeviceContext);

	if(FAILED(hr))
	{
		hr = oD3D11::Singleton()->D3D11CreateDevice(
			NULL,
			D3D_DRIVER_TYPE_REFERENCE,
			NULL,
			0,
			NULL,
			0,
			D3D11_SDK_VERSION,
			&D3DDevice,
			&FeatureLevel,
			&D3DDeviceContext);
	}

	if(FAILED(hr) || FeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		oSetLastError(EINVAL, "Failed to create DX11 device!");
		return false;
	}

	// unsigned int SrcSize = CalculateSize(&_srcDesc);

	oRef<ID3D11Texture2D> D3DSrcTexture;
	{
		D3D11_TEXTURE2D_DESC Desc;

		Desc.Width = _srcDesc.Width;
		Desc.Height = _srcDesc.Height;
		Desc.MipLevels = _srcDesc.NumMips;
		Desc.ArraySize = _srcDesc.NumSlices;
		Desc.Format = static_cast<DXGI_FORMAT>(GetPlatformFormat(_srcDesc.Format));
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Usage = D3D11_USAGE_STAGING;
		Desc.BindFlags = 0;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA SubresourceData;
		SubresourceData.pSysMem = _pSrcSurface;
		SubresourceData.SysMemPitch = _srcDesc.RowPitch;
		SubresourceData.SysMemSlicePitch = _srcDesc.SlicePitch;

		hr = D3DDevice->CreateTexture2D(&Desc, &SubresourceData, &D3DSrcTexture);
	}

	if(FAILED(hr))
	{
		oSetLastError(EINVAL, "Failed to create D3D source texture.");
		return false;
	}

	oRef<ID3D11Texture2D> D3DDstTexture;
	{
		D3D11_TEXTURE2D_DESC Desc;

		Desc.Width = _dstDesc.Width;
		Desc.Height = _dstDesc.Height;
		Desc.MipLevels = _dstDesc.NumMips;
		Desc.ArraySize = _dstDesc.NumSlices;
		Desc.Format = static_cast<DXGI_FORMAT>(GetPlatformFormat(_dstDesc.Format));
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Usage = D3D11_USAGE_STAGING;
		Desc.BindFlags = 0;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		Desc.MiscFlags = 0;

		D3DDevice->CreateTexture2D(&Desc, NULL, &D3DDstTexture);
	}

	if(FAILED(hr))
	{
		oSetLastError(EINVAL, "Failed to create D3D destination texture.");
		return false;
	}

	D3DX11_TEXTURE_LOAD_INFO loadInfo;
	loadInfo.pSrcBox = NULL;
	loadInfo.pDstBox = NULL;
	loadInfo.SrcFirstMip = 0;
	loadInfo.DstFirstMip = 0;
	loadInfo.NumMips = _srcDesc.NumMips;
	loadInfo.SrcFirstElement = 0;
	loadInfo.DstFirstElement = 0;
	loadInfo.NumElements = 1;
	loadInfo.Filter = D3DX11_DEFAULT;
	loadInfo.MipFilter = D3DX11_DEFAULT;

	oTRACE("D3DX11LoadTextureFromTexture begin 0x%p (can take a while)...", D3DDstTexture);
	hr = oD3DX11::Singleton()->D3DX11LoadTextureFromTexture(D3DDeviceContext, D3DSrcTexture, &loadInfo, D3DDstTexture);
	oTRACE("D3DX11LoadTextureFromTexture end 0x%p", D3DDstTexture);
	if(FAILED(hr))
	{
		oSetLastError(EINVAL, "Failed to convert texture from format %i to format %i.", _srcDesc.Format, _dstDesc.Format);
		return false;
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = D3DDeviceContext->Map(D3DDstTexture, 0, D3D11_MAP_READ, 0, &mapped);

	if(FAILED(hr))
	{
		oSetLastError(EINVAL, "Failed to map destination texture.");
		return false;
	}

	oASSERT(mapped.DepthPitch == _dstDesc.DepthPitch, "Destination Surface is incorrect size for conversion.");
	memcpy(_pDstSurface, mapped.pData, __min(mapped.DepthPitch, _srcDesc.DepthPitch));
	D3DDeviceContext->Unmap(D3DDstTexture, 0);

	return true;
}
