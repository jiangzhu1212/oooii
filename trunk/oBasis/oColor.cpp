// $(header)
#include <oBasis/oColor.h>
#include <oBasis/oEqual.h>
#include <oBasis/oLimits.h>
#include <oBasis/oMathTypes.h>
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

size_t oColorFindClosest(oColor _TestColor, const oColor* _pPaletteColors, size_t _NumPaletteColors)
{
	unsigned int test[4], p[4];
	oColorDecompose(_TestColor, test);

	size_t closest = oNumericLimits<size_t>::GetMax();
	unsigned int minDelta = oNumericLimits<unsigned int>::GetMax();
	for (size_t i = 0; i < _NumPaletteColors; i++)
	{
		oColorDecompose(_pPaletteColors[i], p);
		unsigned int delta = 0;
		for (size_t e = 0; e < 4; e++)
		{
			unsigned int eDelta = test[e] - p[e];
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

const char* oAsString(const oColor& _Color)
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
		if (!_stricmp(sMappedColors[i].Name, trimmed))
		{
			*_pValue = sMappedColors[i].Value;
			return true;
		}
	}

	return false;
}

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const uint4& _Value);

char* oToString(char* _StrDestination, size_t _SizeofStrDestination, const oColor& _Value)
{
	const char* c = oAsString(_Value);
	if (!strcmp("Unrec", c))
		return 0 == strcpy_s(_StrDestination, _SizeofStrDestination, c) ? _StrDestination : nullptr;

	uint4 color;
	oColorDecompose(_Value, (unsigned int*)&color);
	return ::oToString(_StrDestination, _SizeofStrDestination, color);
}
