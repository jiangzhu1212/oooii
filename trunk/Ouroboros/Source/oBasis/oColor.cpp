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
#include <oBasis/oColor.h>
#include <oBasis/oEqual.h>
#include <oBasis/oLimits.h>
#include <oBasis/oMath.h>
#include <oBasis/oPlatformFeatures.h>
#include <oBasis/oString.h>
#include <cstdlib>
  
void oColorDecomposeToHSV(oColor _Color, float* _pH, float* _pS, float* _pV)
{
	float r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	float Max = __max(r, __max(g,b));
	float Min = __min(r, __min(g,b));
	float diff = Max - Min;
	*_pV = Max;

	if (oEqual(diff, 0.0f))
	{
		*_pH = 0.0f;
		*_pS = 0.0f;
	}

	else
	{
		*_pS = diff / Max;
		float R = 60.0f * (Max - r) / diff + 180.0f;
		float G = 60.0f * (Max - g) / diff + 180.0f;
		float B = 60.0f * (Max - b) / diff + 180.0f;
		if (r == Max)
			*_pH = B - G;
		else if (oEqual(g, Max))
			*_pH = 120.0f + R - B;
		else
			*_pH = 240.0f + G - R;
	}

	if (*_pH < 0.0f) *_pH += 360.0f;
	if (*_pH >= 360.0f) *_pH -= 360.0f;
}

void oColorRGBToYUV(int _R, int _G, int _B, int* _pY, int* _pU, int* _pV)
{
	// Using the float version of ITU-R BT.601 that jpeg uses. This is similar to 
	// the integer version, except this uses the full 0 - 255 range.
	const float3 oITU_R_BT_601_OffsetToYUV(0.0f, 128.0f, 128.0f);
	const float3 oITU_R_BT_601_YFactor(0.299f, 0.587f, 0.114f);
	const float3 oITU_R_BT_601_UFactor(-0.1687f, -0.3313f, 0.5f);
	const float3 oITU_R_BT_601_VFactor(0.5f, -0.4187f, -0.0813f);
	float3 RGB = oCastAsFloat(int3(_R, _G, _B));
	float3 YUV = float3(dot(RGB, oITU_R_BT_601_YFactor), dot(RGB, oITU_R_BT_601_UFactor), dot(RGB, oITU_R_BT_601_VFactor)) + oITU_R_BT_601_OffsetToYUV;
	int3 iYUV = clamp(oCastAsInt(YUV), 0, 255);
	*_pY = iYUV.x;
	*_pU = iYUV.y;
	*_pV = iYUV.z;
}

void oColorYUVToRGB(int _Y, int _U, int _V, int* _pR, int* _pG, int* _pB)
{
	// Using the float version of ITU-R BT.601 that jpeg uses. This is similar to 
	// the integer version, except this uses the full 0 - 255 range.
	const float3 oITU_R_BT_601_OffsetToRGB = float3(0.0f, -128.0f, -128.0f);
	const float3 oITU_R_BT_601_RFactor = float3(1.0f, 0.0f, 1.402f);
	const float3 oITU_R_BT_601_GFactor = float3(1.0f, -0.34414f, -0.71414f);
	const float3 oITU_R_BT_601_BFactor = float3(1.0f, 1.772f, 0.0f);
	float3 YUV = oCastAsFloat(int3(_Y, _U, _V)) + oITU_R_BT_601_OffsetToRGB;
	float3 RGB = float3(dot(YUV, oITU_R_BT_601_RFactor), dot(YUV, oITU_R_BT_601_GFactor), dot(YUV, oITU_R_BT_601_BFactor));
	int3 iRGB = clamp(oCastAsInt(RGB), 0, 255);
	*_pR = iRGB.x;
	*_pG = iRGB.y;
	*_pB = iRGB.z;
}

void oColorRGBToYCoCg(int _R, int _G, int _B, int* _pY, int* _pCo, int* _pCg)
{
	//note that if you want to pay for int-float and back conversions, the non -R variant can be computed as 3 dot products.
	int3 yCoCg;

	yCoCg.x = (_R + (_G<<1) + _B + 2) >> 2;
	yCoCg.y = ((_R - _B + 1) >> 1);
	yCoCg.z = (((_G<<1) - _R - _B + 2) >> 2);

	yCoCg.x = clamp(yCoCg.x, 0, 255);
	yCoCg.y = clamp(yCoCg.y, -128, 127);
	yCoCg.z = clamp(yCoCg.z, -128, 127);

	*_pY = yCoCg.x;
	*_pCo = yCoCg.y;
	*_pCg = yCoCg.z;
}

void oColorYCoCgToRGB(int _Y, int _Co, int _Cg, int* _pR, int* _pG, int* _pB)
{
	*_pR = clamp(_Y + _Co - _Cg, 0, 255);
	*_pG = clamp(_Y + _Cg, 0, 255);
	*_pB = clamp(_Y - _Co - _Cg, 0, 255);
}

size_t oColorFindClosest(oColor _TestColor, const oColor* _pPaletteColors, size_t _NumPaletteColors)
{
	int test[4], p[4];
	oColorDecompose(_TestColor, test);

	size_t closest = oNumericLimits<size_t>::GetMax();
	int minDelta = oNumericLimits<int>::GetMax();
	for (size_t i = 0; i < _NumPaletteColors; i++)
	{
		oColorDecompose(_pPaletteColors[i], p);
		int delta = 0;
		for (size_t e = 0; e < 4; e++)
		{
			int eDelta = test[e] - p[e];
			eDelta *= eDelta;
			delta += eDelta; // sum the square of the per-element difference
		}

		if (delta < minDelta)
		{
			minDelta = delta;
			closest = i;
		}
	}

	return closest;
}

struct MAPPING
{
	const char* Name;
	oColor Value;
};

static const MAPPING sMappedColors[] = 
{
	{ "AliceBlue", std::AliceBlue },
	{ "AntiqueWhite", std::AntiqueWhite, },
	{ "Aqua", std::Aqua, },
	{ "Aquamarine", std::Aquamarine, },
	{ "Azure", std::Azure, },
	{ "Beige", std::Beige, },
	{ "Bisque", std::Bisque, },
	{ "Black", std::Black, },
	{ "BlanchedAlmond", std::BlanchedAlmond, },
	{ "Blue", std::Blue, },
	{ "BlueViolet", std::BlueViolet, },
	{ "Brown", std::Brown, },
	{ "BurlyWood", std::BurlyWood, },
	{ "CadetBlue", std::CadetBlue, },
	{ "Chartreuse", std::Chartreuse, },
	{ "Chocolate", std::Chocolate, },
	{ "Coral", std::Coral, },
	{ "CornflowerBlue", std::CornflowerBlue, },
	{ "Cornsilk", std::Cornsilk, },
	{ "Crimson", std::Crimson, },
	{ "Cyan", std::Cyan, },
	{ "DarkBlue", std::DarkBlue, },
	{ "DarkCyan", std::DarkCyan, },
	{ "DarkGoldenRod", std::DarkGoldenRod, },
	{ "DarkGray", std::DarkGray, },
	{ "DarkGreen", std::DarkGreen, },
	{ "DarkKhaki", std::DarkKhaki, },
	{ "DarkMagenta", std::DarkMagenta, },
	{ "DarkOliveGreen", std::DarkOliveGreen, },
	{ "Darkorange", std::Darkorange, },
	{ "DarkOrchid", std::DarkOrchid, },
	{ "DarkRed", std::DarkRed, },
	{ "DarkSalmon", std::DarkSalmon, },
	{ "DarkSeaGreen", std::DarkSeaGreen, },
	{ "DarkSlateBlue", std::DarkSlateBlue, },
	{ "DarkSlateGray", std::DarkSlateGray, },
	{ "DarkTurquoise", std::DarkTurquoise, },
	{ "DarkViolet", std::DarkViolet, },
	{ "DeepPink", std::DeepPink, },
	{ "DeepSkyBlue", std::DeepSkyBlue, },
	{ "DimGray", std::DimGray, },
	{ "DodgerBlue", std::DodgerBlue, },
	{ "FireBrick", std::FireBrick, },
	{ "FloralWhite", std::FloralWhite, },
	{ "ForestGreen", std::ForestGreen, },
	{ "Fuchsia", std::Fuchsia, },
	{ "Gainsboro", std::Gainsboro, },
	{ "GhostWhite", std::GhostWhite, },
	{ "Gold", std::Gold, },
	{ "GoldenRod", std::GoldenRod, },
	{ "Gray", std::Gray, },
	{ "Green", std::Green, },
	{ "GreenYellow", std::GreenYellow, },
	{ "HoneyDew", std::HoneyDew, },
	{ "HotPink", std::HotPink, },
	{ "IndianRed", std::IndianRed, },
	{ "Indigo", std::Indigo, },
	{ "Ivory", std::Ivory, },
	{ "Khaki", std::Khaki, },
	{ "Lavender", std::Lavender, },
	{ "LavenderBlush", std::LavenderBlush, },
	{ "LawnGreen", std::LawnGreen, },
	{ "LemonChiffon", std::LemonChiffon, },
	{ "LightBlue", std::LightBlue, },
	{ "LightCoral", std::LightCoral, },
	{ "LightCyan", std::LightCyan, },
	{ "LightGoldenRodYellow", std::LightGoldenRodYellow, },
	{ "LightGrey", std::LightGrey, },
	{ "LightGreen", std::LightGreen, },
	{ "LightPink", std::LightPink, },
	{ "LightSalmon", std::LightSalmon, },
	{ "LightSeaGreen", std::LightSeaGreen, },
	{ "LightSkyBlue", std::LightSkyBlue, },
	{ "LightSlateGray", std::LightSlateGray, },
	{ "LightSteelBlue", std::LightSteelBlue, },
	{ "LightYellow", std::LightYellow, },
	{ "Lime", std::Lime, },
	{ "LimeGreen", std::LimeGreen, },
	{ "Linen", std::Linen, },
	{ "Magenta", std::Magenta, },
	{ "Maroon", std::Maroon, },
	{ "MediumAquaMarine", std::MediumAquaMarine, },
	{ "MediumBlue", std::MediumBlue, },
	{ "MediumOrchid", std::MediumOrchid, },
	{ "MediumPurple", std::MediumPurple, },
	{ "MediumSeaGreen", std::MediumSeaGreen, },
	{ "MediumSlateBlue", std::MediumSlateBlue, },
	{ "MediumSpringGreen", std::MediumSpringGreen, },
	{ "MediumTurquoise", std::MediumTurquoise, },
	{ "MediumVioletRed", std::MediumVioletRed, },
	{ "MidnightBlue", std::MidnightBlue, },
	{ "MintCream", std::MintCream, },
	{ "MistyRose", std::MistyRose, },
	{ "Moccasin", std::Moccasin, },
	{ "NavajoWhite", std::NavajoWhite, },
	{ "Navy", std::Navy, },
	{ "OldLace", std::OldLace, },
	{ "Olive", std::Olive, },
	{ "OliveDrab", std::OliveDrab, },
	{ "Orange", std::Orange, },
	{ "OrangeRed", std::OrangeRed, },
	{ "Orchid", std::Orchid, },
	{ "PaleGoldenRod", std::PaleGoldenRod, },
	{ "PaleGreen", std::PaleGreen, },
	{ "PaleTurquoise", std::PaleTurquoise, },
	{ "PaleVioletRed", std::PaleVioletRed, },
	{ "PapayaWhip", std::PapayaWhip, },
	{ "PeachPuff", std::PeachPuff, },
	{ "Peru", std::Peru, },
	{ "Pink", std::Pink, },
	{ "Plum", std::Plum, },
	{ "PowderBlue", std::PowderBlue, },
	{ "Purple", std::Purple, },
	{ "Red", std::Red, },
	{ "RosyBrown", std::RosyBrown, },
	{ "RoyalBlue", std::RoyalBlue, },
	{ "SaddleBrown", std::SaddleBrown, },
	{ "Salmon", std::Salmon, },
	{ "SandyBrown", std::SandyBrown, },
	{ "SeaGreen", std::SeaGreen, },
	{ "SeaShell", std::SeaShell, },
	{ "Sienna", std::Sienna, },
	{ "Silver", std::Silver, },
	{ "SkyBlue", std::SkyBlue, },
	{ "SlateBlue", std::SlateBlue, },
	{ "SlateGray", std::SlateGray, },
	{ "Snow", std::Snow, },
	{ "SpringGreen", std::SpringGreen, },
	{ "SteelBlue", std::SteelBlue, },
	{ "Tan", std::Tan, },
	{ "Teal", std::Teal, },
	{ "Thistle", std::Thistle, },
	{ "Tomato", std::Tomato, },
	{ "Turquoise", std::Turquoise, },
	{ "Violet", std::Violet, },
	{ "Wheat", std::Wheat, },
	{ "White", std::White, },
	{ "WhiteSmoke", std::WhiteSmoke, },
	{ "Yellow", std::Yellow, },
	{ "YellowGreen", std::YellowGreen, },
	{ "OOOiiGreen", std::OOOiiGreen, },
	{ "TangentSpaceNormalBlue", std::TangentSpaceNormalBlue, },
	{ "ObjectSpaceNormalGreen", std::ObjectSpaceNormalGreen, },
};

const char* oAsString(oColor _Color)
{
	for (size_t i = 0; i < oCOUNTOF(sMappedColors); i++)
		if (sMappedColors[i].Value == _Color)
			return sMappedColors[i].Name;
	return "Unrecognized oColor";
}

bool oFromString(oColor* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	char trimmed[64];
	oTrim(trimmed, _StrSource);
	for (size_t i = 0; i < oCOUNTOF(sMappedColors); i++)
	{
		if (!oStricmp(sMappedColors[i].Name, trimmed))
		{
			*_pValue = sMappedColors[i].Value;
			return true;
		}
	}

	return false;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const int4& _Value);

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oColor& _Value)
{
	const char* c = oAsString(_Value);
	if (0 != oStrcmp("Unrec", c))
		return oStrcpy(_StrDestination, _SizeofStrDestination, c);

	int4 color;
	oColorDecompose(_Value, (int*)&color);
	return ::oToString(_StrDestination, _SizeofStrDestination, color);
}
