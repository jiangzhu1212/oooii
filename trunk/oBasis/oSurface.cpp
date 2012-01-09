// $(header)
#include <oBasis/oSurface.h>
#include <oBasis/oAssert.h>
#include <oBasis/oByte.h>
#include <oBasis/oMacros.h>
#include <oBasis/oMemory.h>
#include <cstring>

struct BIT_SIZE
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

struct FORMAT_DESC
{
	const char* String;
	unsigned int Size;
	BIT_SIZE BitSize;
	unsigned short NumChannels;
	bool IsBlockCompressed;
	bool IsUNORM;
	bool HasAlpha;
};

static const FORMAT_DESC sFormatDescs[] = 
{
	{ "UNKNOWN", 0, {0,0,0,0}, 0, false, false, false },
	{ "R32G32B32A32_TYPELESS", 4 * sizeof(float), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_FLOAT", 4 * sizeof(float), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_UINT", 4 * sizeof(int), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32A32_SINT", 4 * sizeof(int), {32,32,32,32}, 4, false, false, true },
	{ "R32G32B32_TYPELESS", 3 * sizeof(int), {32,32,32,0}, 3, false, false, true },
	{ "R32G32B32_FLOAT", 3 * sizeof(float), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_UINT", 4 * sizeof(int), {32,32,32,0}, 3, false, false, false },
	{ "R32G32B32_SINT", 4 * sizeof(int), {32,32,32,0}, 3, false, false, false },
	{ "R16G16B16A16_TYPELESS", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_FLOAT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_UNORM", 4 * sizeof(short), {16,16,16,16}, 4, false, true, true },
	{ "R16G16B16A16_UINT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SNORM", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R16G16B16A16_SINT", 4 * sizeof(short), {16,16,16,16}, 4, false, false, true },
	{ "R32G32_TYPELESS", 2 * sizeof(float), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_FLOAT", 2 * sizeof(float), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_UINT", 2 * sizeof(int), {32,32,0,0}, 2, false, false, false },
	{ "R32G32_SINT", 2 * sizeof(int), {32,32,0,0}, 2, false, false, false },
	{ "R32G8X24_TYPELESS", 2 * sizeof(float), {32,8,0,24}, 3, false, false, false },
	{ "D32_FLOAT_S8X24_UINT", 2 * sizeof(int), {32,8,0,24}, 3, false, false, false },
	{ "R32_FLOAT_X8X24_TYPELESS", 2 * sizeof(float), {32,8,0,24}, 3, false, false, false },
	{ "X32_TYPELESS_G8X24_UINT", 2 * sizeof(int), {32,8,0,24}, 3, false, false, false },
	{ "R10G10B10A2_TYPELESS", sizeof(int), {10,10,10,2}, 4, false, false, true },
	{ "R10G10B10A2_UNORM", sizeof(int), {10,10,10,2}, 4, false, true, true },
	{ "R10G10B10A2_UINT", sizeof(int), {10,10,10,2}, 4, false, false, true },
	{ "R11G11B10_FLOAT", sizeof(float), {11,11,11,0}, 3, false, false, false },
	{ "R8G8B8A8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "R8G8B8A8_UINT", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SNORM", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R8G8B8A8_SINT", sizeof(int), {8,8,8,8}, 4, false, false, true },
	{ "R16G16_TYPELESS", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_FLOAT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_UNORM", 2 * sizeof(short), {16,16,0,0}, 2, false, true, false },
	{ "R16G16_UINT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SNORM", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R16G16_SINT", 2 * sizeof(short), {16,16,0,0}, 2, false, false, false },
	{ "R32_TYPELESS", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "D32_FLOAT", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "R32_FLOAT", sizeof(float), {32,0,0,0}, 1, false, false, false },
	{ "R32_UINT", sizeof(int), {32,0,0,0}, 1, false, false, false },
	{ "R32_SINT", sizeof(int), {32,0,0,0}, 1, false, false, false },
	{ "R24G8_TYPELESS", sizeof(float), {24,8,0,0}, 2, false, false, false },
	{ "D24_UNORM_S8_UINT", sizeof(float), {24,8,0,0}, 2, false, true, false },
	{ "R24_UNORM_X8_TYPELESS", sizeof(float), {24,8,0,0}, 2, false, true, false },
	{ "X24_TYPELESS_G8_UINT", sizeof(float), {24,8,0,0}, 2, false, false, false },
	{ "R8G8_TYPELESS", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_UNORM", 2 * sizeof(char), {8,8,0,0}, 2, false, true, false },
	{ "R8G8_UINT", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SNORM", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R8G8_SINT", 2 * sizeof(char), {8,8,0,0}, 2, false, false, false },
	{ "R16_TYPELESS", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_FLOAT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "D16_UNORM", sizeof(short), {16,0,0,0}, 1, false, true, false },
	{ "R16_UNORM", sizeof(short), {16,0,0,0}, 1, false, true, false },
	{ "R16_UINT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_SNORM", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R16_SINT", sizeof(short), {16,0,0,0}, 1, false, false, false },
	{ "R8_TYPELESS", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_UNORM", sizeof(char), {8,0,0,0}, 1, false, true, false },
	{ "R8_UINT", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_SNORM", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "R8_SINT", sizeof(char), {8,0,0,0}, 1, false, false, false },
	{ "A8_UNORM", sizeof(char), {8,0,0,0}, 1, false, true, true },
	{ "R1_UNORM", 1, {1,0,0,0}, 1, false, true, false },
	{ "R9G9B9E5_SHAREDEXP", sizeof(int), {9,9,9,5}, 4, false, false, false },
	{ "R8G8_B8G8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "G8R8_G8B8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "BC1_TYPELESS", 8, {5,6,5,0}, 3, true, false, false },
	{ "BC1_UNORM", 8, {5,6,5,0}, 3, true, true, false },
	{ "BC1_UNORM_SRGB", 8, {5,6,5,0}, 3, true, true, false },
	{ "BC2_TYPELESS", 16, {5,6,5,4}, 4, true, false, true },
	{ "BC2_UNORM", 16, {5,6,5,4}, 4, true, true, true },
	{ "BC2_UNORM_SRGB", 16, {5,6,5,4}, 4, true, true, true },
	{ "BC3_TYPELESS", 16, {5,6,5,8}, 4, true, false, true },
	{ "BC3_UNORM", 16, {5,6,5,8}, 4, true, true, true },
	{ "BC3_UNORM_SRGB", 16, {5,6,5,8}, 4, true, true, true },
	{ "BC4_TYPELESS", 8, {8,0,0,0}, 1, true, false, false },
	{ "BC4_UNORM", 8, {8,0,0,0}, 1, true, true, false },
	{ "BC4_SNORM", 8, {8,0,0,0}, 1, true, false, false },
	{ "BC5_TYPELESS", 16, {8,8,0,0}, 2, true, false, false },
	{ "BC5_UNORM", 16, {8,8,0,0}, 2, true, true, false },
	{ "BC5_SNORM", 16, {8,8,0,0}, 2, true, false, false },
	{ "B5G6R5_UNORM", sizeof(short), {5,6,5,0}, 3, false, true, false },
	{ "B5G5R5A1_UNORM", sizeof(short), {5,6,5,1}, 4, false, true, true },
	{ "B8G8R8A8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, true },
	{ "B8G8R8X8_UNORM", sizeof(int), {8,8,8,8}, 4, false, true, false },
	{ "R10G10B10_XR_BIAS_A2_UNORM", sizeof(int), 4, false, true, false },
	{ "B8G8R8A8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false },
	{ "B8G8R8A8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true },
	{ "B8G8R8X8_TYPELESS", sizeof(int), {8,8,8,8}, 4, false, false },
	{ "B8G8R8X8_UNORM_SRGB", sizeof(int), {8,8,8,8}, 4, false, true },
	{ "BC6H_TYPELESS", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC6H_UF16", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC6H_SF16", 16, {0,0,0,0}, 3, true, false, false },
	{ "BC7_TYPELESS", 16, {0,0,0,0}, 4, true, false, true },
	{ "BC7_UNORM", 16, {0,0,0,0}, 4, true, true, true },
	{ "BC7_UNORM_SRGB", 16, {0,0,0,0}, 4, true, true, true },
	{ "R8G8B8_UNORM", 3 * sizeof(char), {8,8,8,0}, 3, false, true, false },
	{ "B8G8R8_UNORM", 3 * sizeof(char), {8,8,8,0}, 3, false, true, false },
};
static_assert(oCOUNTOF(sFormatDescs) == oSURFACE_NUM_FORMATS, "");

const char* oAsString(const oSURFACE_FORMAT& _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].String : "UNKNOWN";
}


char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oSURFACE_FORMAT& _Format)
{
	return 0 == strcpy_s(_StrDestination, _SizeofStrDestination, oAsString(_Format)) ? _StrDestination : nullptr;
}

bool oFromString(oSURFACE_FORMAT* _pFormat, const char* _StrSource)
{
	*_pFormat = oSURFACE_UNKNOWN;
	for (size_t i = 0; i < oCOUNTOF(sFormatDescs); i++)
	{
		if (!_stricmp(_StrSource, sFormatDescs[i].String))
		{
			*_pFormat = (oSURFACE_FORMAT)i;
			return true;
		}
	}
	return false;
}

bool oSurfaceIsBlockCompressedFormat(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].IsBlockCompressed : false;
}

bool oSurfaceIsDepthFormat(oSURFACE_FORMAT _Format)
{
	bool result = false;

	switch (_Format)
	{
		case oSURFACE_D32_FLOAT:
		case oSURFACE_R32_TYPELESS:
		case oSURFACE_D24_UNORM_S8_UINT:
		case oSURFACE_R24_UNORM_X8_TYPELESS:
		case oSURFACE_D32_FLOAT_S8X24_UINT:
		case oSURFACE_R32_FLOAT_X8X24_TYPELESS:
		case oSURFACE_D16_UNORM:
		case oSURFACE_R16_TYPELESS:
			result = true;
		default:
			break;
	}

	return result;
}

bool oSurfaceIsAlphaFormat(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].HasAlpha : false;
}

bool oSurfaceIsUNORM(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].IsUNORM : false;
}

unsigned int oSurfaceGetNumChannels(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].NumChannels : 0;
}

unsigned int oSurfaceGetSize(oSURFACE_FORMAT _Format)
{
	return (_Format < oSURFACE_NUM_FORMATS) ? sFormatDescs[_Format].Size : 0;
}

unsigned int oSurfaceGetBitSize(oSURFACE_FORMAT _Format)
{
	if (_Format == oSURFACE_R1_UNORM) return 1;
	return 8 * oSurfaceGetSize(_Format);
}

void oSurfaceGetChannelBitSize(oSURFACE_FORMAT _Format, int* _pNBitsR, int* _pNBitsG, int* _pNBitsB, int* _pNBitsA)
{
	if (_Format < oSURFACE_NUM_FORMATS)
	{
		const BIT_SIZE& b = sFormatDescs[_Format].BitSize;
		*_pNBitsR = b.G; *_pNBitsG = b.G; *_pNBitsB = b.B; *_pNBitsA = b.A;
	}

	else
		*_pNBitsR = *_pNBitsG = *_pNBitsB = *_pNBitsA = 0;
}

unsigned int oSurfaceCalcRowPitch(oSURFACE_FORMAT _Format, unsigned int _Mip0Width, unsigned int _MipLevel)
{
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to GetRowPitch");
	unsigned int w = oSurfaceCalcMipDimension(_Format, _Mip0Width, _MipLevel);
	if (oSurfaceIsBlockCompressedFormat(_Format)) // because the atom is a 4x4 block
		w /= 4;
	unsigned int s = oSurfaceGetSize(_Format);
	return static_cast<unsigned int>(oByteAlign(w * s, sizeof(int)));
}

unsigned int oSurfaceCalcLevelPitch(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _MipLevel)
{
	return oSurfaceCalcRowPitch(_Format, _MipDimensions.x, _MipLevel) * oSurfaceCalcNumRows(_Format, _MipDimensions.y, _MipLevel);
}

unsigned int oSurfaceCalcSlicePitch(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumMips)
{
	unsigned int pitch = 0;
	const unsigned int nMips = 0 == _NumMips ? oSurfaceCalcMipCount(_MipDimensions) : _NumMips;
	for (unsigned int i = 0; i < nMips; i++)
		pitch += oSurfaceCalcLevelPitch(_Format, _MipDimensions, i);
	return pitch;
}

unsigned int oSurfaceCalcMipDimension(oSURFACE_FORMAT _Format, unsigned int _Mip0Dimension, unsigned int _MipLevel)
{
	oASSERT(_Format != oSURFACE_UNKNOWN, "Unknown surface format passed to CalcMipDimension");
	oASSERT(_MipLevel == 0 || oIsPow2(_Mip0Dimension), "Mipchain dimensions must be a power of 2");
	unsigned int d = __max(1, _Mip0Dimension >> _MipLevel);
	return oSurfaceIsBlockCompressedFormat(_Format) ? static_cast<unsigned int>(oByteAlign(d, 4)) : d;
}

unsigned int oSurfaceCalcNumRows(oSURFACE_FORMAT _Format, unsigned int _Mip0Height, unsigned int _MipLevel)
{
	unsigned int heightInPixels = oSurfaceCalcMipDimension(_Format, _Mip0Height, _MipLevel);
	return oSurfaceIsBlockCompressedFormat(_Format) ? __max(1, heightInPixels/4) : heightInPixels;
}

unsigned int oSurfaceCalcMipCount(int2 _MipDimensions)
{
	unsigned int n = 0, w, h;
	do
	{
		// For this calculation, format doesn't matter
		w = oSurfaceCalcMipDimension(oSURFACE_B8G8R8A8_TYPELESS, _MipDimensions.x, n);
		h = oSurfaceCalcMipDimension(oSURFACE_B8G8R8A8_TYPELESS, _MipDimensions.y, n);
		n++;
	} while (w != 1 || h != 1);

	return n;
}

unsigned int oSurfaceCalcBufferSize(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices)
{
	// http://www.cs.umbc.edu/~olano/s2006c03/ch02.pdf
	// "Texture3D behaves identically to a Texture2DArray with n array 
	// slices where n is the depth (3rd dimension) of the Texture3D."
	return oSurfaceCalcSlicePitch(_Format, _MipDimensions.x, _MipDimensions.y) * _NumSlices;
}

unsigned int oSurfaceCalcSubresourceSize(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices, unsigned int _Subresource)
{
	unsigned int nMips = oSurfaceCalcMipCount(_MipDimensions);
	unsigned int slicePitch = oSurfaceCalcSlicePitch(_Format, _MipDimensions);

	unsigned int slice0, mipLevel0, slice1, mipLevel1;
	oSurfaceUnpackSubresource(_Subresource, nMips, &mipLevel0, &slice0);
	oSurfaceUnpackSubresource(_Subresource + 1, nMips, &mipLevel1, &slice1);
	
	unsigned int offset0 = (slice0 * slicePitch) + oSurfaceCalcLevelOffset(_Format, _MipDimensions, mipLevel0);
	unsigned int offset1 = (slice1 * slicePitch) + oSurfaceCalcLevelOffset(_Format, _MipDimensions, mipLevel1);
	
	return offset1 - offset0;
}

unsigned int oSurfaceCalcLevelOffset(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _MipLevel)
{
	unsigned int offset = 0;
	for (unsigned int i = 0; i < _MipLevel; i++)
		offset += oSurfaceCalcLevelPitch(_Format, _MipDimensions, i);
	return offset;
}

unsigned int oSurfaceCalcBufferOffset(oSURFACE_FORMAT _Format, int2 _MipDimensions, unsigned int _NumSlices, unsigned int _Subresource)
{
	unsigned int nMips = oSurfaceCalcMipCount(_MipDimensions);
	unsigned int slice, mipLevel;
	oSurfaceUnpackSubresource(_Subresource, nMips, &mipLevel, &slice);
	unsigned int slicePitch = oSurfaceCalcSlicePitch(_Format, _MipDimensions.x, _MipDimensions.y);
	return (slice * slicePitch) + oSurfaceCalcLevelOffset(_Format, _MipDimensions, mipLevel);
}

unsigned int oSurfaceCalcSize(const oSURFACE_DESC& _Desc)
{
	// Mips and slices not yet supported.
	oASSERT(_Desc.NumMips <= 1, "");
	oASSERT(_Desc.Dimensions.z <= 1, "");

	unsigned int rowPitch = _Desc.RowPitch;
	
	// Calculate assuming the simple case of a contiguous set of word-aligned scanlines
	if (!rowPitch)
		rowPitch = oSurfaceCalcRowPitch(_Desc.Format, _Desc.Dimensions.x);

	// Block-compressed formats cover 4x4 blocks in their stride, so from a bytes 
	// perspective, the width seems wider, and the height should be shorter.
	unsigned int heightAdjustment = oSurfaceIsBlockCompressedFormat(_Desc.Format) ? 4 : 1;
	unsigned int blockAlignedHeight = oSurfaceIsBlockCompressedFormat(_Desc.Format) ? static_cast<unsigned int>(oByteAlign(_Desc.Dimensions.y, 4)) : _Desc.Dimensions.y;
	return rowPitch * (blockAlignedHeight / heightAdjustment);
}


unsigned int oSurfaceCalcSize(oSURFACE_FORMAT _Format, unsigned int _Width, unsigned int _Height)
{
	unsigned int rowPitch = oSurfaceCalcRowPitch(_Format, _Width);
	
	// Calculate assuming the simple case of a contiguous set of word-aligned scanlines
	if (!rowPitch)
		rowPitch = oSurfaceCalcRowPitch(_Format, _Width);

	// Block-compressed formats cover 4x4 blocks in their stride, so from a bytes 
	// perspective, the width seems wider, and the height should be shorter.
	unsigned int heightAdjustment = oSurfaceIsBlockCompressedFormat(_Format) ? 4 : 1;
	unsigned int blockAlignedHeight = oSurfaceIsBlockCompressedFormat(_Format) ? 
    static_cast<unsigned int>(oByteAlign(_Height, 4)) : _Height;
	return rowPitch * (blockAlignedHeight / heightAdjustment);
}
