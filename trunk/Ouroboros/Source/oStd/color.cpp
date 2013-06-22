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
#include <oStd/color.h>
#include <oStd/stringize.h>

namespace oStd {
	namespace detail {

struct MAPPING
{
	const char* Name;
	oStd::color Value;
};

static const MAPPING sMappedColors[] = 
{
	{ "AliceBlue", AliceBlue },
	{ "AntiqueWhite", AntiqueWhite, },
	{ "Aqua", Aqua, },
	{ "Aquamarine", Aquamarine, },
	{ "Azure", Azure, },
	{ "Beige", Beige, },
	{ "Bisque", Bisque, },
	{ "Black", Black, },
	{ "BlanchedAlmond", BlanchedAlmond, },
	{ "Blue", Blue, },
	{ "BlueViolet", BlueViolet, },
	{ "Brown", Brown, },
	{ "BurlyWood", BurlyWood, },
	{ "CadetBlue", CadetBlue, },
	{ "Chartreuse", Chartreuse, },
	{ "Chocolate", Chocolate, },
	{ "Coral", Coral, },
	{ "CornflowerBlue", CornflowerBlue, },
	{ "Cornsilk", Cornsilk, },
	{ "Crimson", Crimson, },
	{ "Cyan", Cyan, },
	{ "DarkBlue", DarkBlue, },
	{ "DarkCyan", DarkCyan, },
	{ "DarkGoldenRod", DarkGoldenRod, },
	{ "DarkGray", DarkGray, },
	{ "DarkGreen", DarkGreen, },
	{ "DarkKhaki", DarkKhaki, },
	{ "DarkMagenta", DarkMagenta, },
	{ "DarkOliveGreen", DarkOliveGreen, },
	{ "Darkorange", Darkorange, },
	{ "DarkOrchid", DarkOrchid, },
	{ "DarkRed", DarkRed, },
	{ "DarkSalmon", DarkSalmon, },
	{ "DarkSeaGreen", DarkSeaGreen, },
	{ "DarkSlateBlue", DarkSlateBlue, },
	{ "DarkSlateGray", DarkSlateGray, },
	{ "DarkTurquoise", DarkTurquoise, },
	{ "DarkViolet", DarkViolet, },
	{ "DeepPink", DeepPink, },
	{ "DeepSkyBlue", DeepSkyBlue, },
	{ "DimGray", DimGray, },
	{ "DodgerBlue", DodgerBlue, },
	{ "FireBrick", FireBrick, },
	{ "FloralWhite", FloralWhite, },
	{ "ForestGreen", ForestGreen, },
	{ "Fuchsia", Fuchsia, },
	{ "Gainsboro", Gainsboro, },
	{ "GhostWhite", GhostWhite, },
	{ "Gold", Gold, },
	{ "GoldenRod", GoldenRod, },
	{ "Gray", Gray, },
	{ "Green", Green, },
	{ "GreenYellow", GreenYellow, },
	{ "HoneyDew", HoneyDew, },
	{ "HotPink", HotPink, },
	{ "IndianRed", IndianRed, },
	{ "Indigo", Indigo, },
	{ "Ivory", Ivory, },
	{ "Khaki", Khaki, },
	{ "Lavender", Lavender, },
	{ "LavenderBlush", LavenderBlush, },
	{ "LawnGreen", LawnGreen, },
	{ "LemonChiffon", LemonChiffon, },
	{ "LightBlue", LightBlue, },
	{ "LightCoral", LightCoral, },
	{ "LightCyan", LightCyan, },
	{ "LightGoldenRodYellow", LightGoldenRodYellow, },
	{ "LightGrey", LightGrey, },
	{ "LightGreen", LightGreen, },
	{ "LightPink", LightPink, },
	{ "LightSalmon", LightSalmon, },
	{ "LightSeaGreen", LightSeaGreen, },
	{ "LightSkyBlue", LightSkyBlue, },
	{ "LightSlateGray", LightSlateGray, },
	{ "LightSteelBlue", LightSteelBlue, },
	{ "LightYellow", LightYellow, },
	{ "Lime", Lime, },
	{ "LimeGreen", LimeGreen, },
	{ "Linen", Linen, },
	{ "Magenta", Magenta, },
	{ "Maroon", Maroon, },
	{ "MediumAquaMarine", MediumAquaMarine, },
	{ "MediumBlue", MediumBlue, },
	{ "MediumOrchid", MediumOrchid, },
	{ "MediumPurple", MediumPurple, },
	{ "MediumSeaGreen", MediumSeaGreen, },
	{ "MediumSlateBlue", MediumSlateBlue, },
	{ "MediumSpringGreen", MediumSpringGreen, },
	{ "MediumTurquoise", MediumTurquoise, },
	{ "MediumVioletRed", MediumVioletRed, },
	{ "MidnightBlue", MidnightBlue, },
	{ "MintCream", MintCream, },
	{ "MistyRose", MistyRose, },
	{ "Moccasin", Moccasin, },
	{ "NavajoWhite", NavajoWhite, },
	{ "Navy", Navy, },
	{ "OldLace", OldLace, },
	{ "Olive", Olive, },
	{ "OliveDrab", OliveDrab, },
	{ "Orange", Orange, },
	{ "OrangeRed", OrangeRed, },
	{ "Orchid", Orchid, },
	{ "PaleGoldenRod", PaleGoldenRod, },
	{ "PaleGreen", PaleGreen, },
	{ "PaleTurquoise", PaleTurquoise, },
	{ "PaleVioletRed", PaleVioletRed, },
	{ "PapayaWhip", PapayaWhip, },
	{ "PeachPuff", PeachPuff, },
	{ "Peru", Peru, },
	{ "Pink", Pink, },
	{ "Plum", Plum, },
	{ "PowderBlue", PowderBlue, },
	{ "Purple", Purple, },
	{ "Red", Red, },
	{ "RosyBrown", RosyBrown, },
	{ "RoyalBlue", RoyalBlue, },
	{ "SaddleBrown", SaddleBrown, },
	{ "Salmon", Salmon, },
	{ "SandyBrown", SandyBrown, },
	{ "SeaGreen", SeaGreen, },
	{ "SeaShell", SeaShell, },
	{ "Sienna", Sienna, },
	{ "Silver", Silver, },
	{ "SkyBlue", SkyBlue, },
	{ "SlateBlue", SlateBlue, },
	{ "SlateGray", SlateGray, },
	{ "Snow", Snow, },
	{ "SpringGreen", SpringGreen, },
	{ "SteelBlue", SteelBlue, },
	{ "Tan", Tan, },
	{ "Teal", Teal, },
	{ "Thistle", Thistle, },
	{ "Tomato", Tomato, },
	{ "Turquoise", Turquoise, },
	{ "Violet", Violet, },
	{ "Wheat", Wheat, },
	{ "White", White, },
	{ "WhiteSmoke", WhiteSmoke, },
	{ "Yellow", Yellow, },
	{ "YellowGreen", YellowGreen, },
	{ "OOOiiGreen", OOOiiGreen, },
	{ "TangentSpaceNormalBlue", TangentSpaceNormalBlue, },
	{ "ObjectSpaceNormalGreen", ObjectSpaceNormalGreen, },
};

bool from_string(color* _pColor, const char* _String)
{
	oFORI(i, sMappedColors)
	{
		if (!_stricmp(sMappedColors[i].Name, _String))
		{
			*_pColor = sMappedColors[i].Value;
			return true;
		}
	}

	return false;
}

	} // namespace detail

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const color& _Value)
{
	const char* c = as_string(_Value);
	if (c) return strlcpy(_StrDestination, c, _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
	int r,g,b,a;
	_Value.decompose(&r, &g, &b, &a);
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%d %d %d %d", r, g, b, a) ? _StrDestination : nullptr;
}

bool from_string(color* _pValue, const char* _StrSource)
{
	if (!_pValue || !_StrSource) return false;
	char trimmed[64];
	trim(trimmed, _StrSource);

	if (strstr(trimmed, "."))
	{
		float c[4];
		if (!from_string_float_array(c, 4, trimmed))
			return false;
		*_pValue = color(c[0], c[1], c[2], c[3]);
		return true;
	}

	else
	{
		_StrSource += strcspn(_StrSource, oDIGIT_SIGNED);
		if (*_StrSource)
		{
			int r,g,b,a;
			if (4 == sscanf_s(_StrSource, "%d %d %d %d", &r, &g, &b, &a))
			{
				*_pValue = color(r, g, b, a);
				return true;
			}
		}
	}

	return detail::from_string(_pValue, trimmed);
}

const char* as_string(const color& _Color)
{
	oFORI(i, detail::sMappedColors)
		if (detail::sMappedColors[i].Value == _Color)
			return detail::sMappedColors[i].Name;
	return nullptr;
}

} // namespace oStd
