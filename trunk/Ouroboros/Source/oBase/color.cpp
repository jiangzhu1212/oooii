/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
#include <oBase/color.h>
#include <oBase/colors.h>
#include <oBase/stringize.h>

namespace ouro {
	namespace detail {

struct MAPPING
{
	const char* Name;
	color Value;
};

static const MAPPING sMappedColors[] = 
{
	{ "alice_blue", alice_blue },
	{ "almost_black", almost_black },
	{ "antique_white", antique_white },
	{ "aqua", aqua },
	{ "aquamarine", aquamarine },
	{ "azure", azure },
	{ "beige", beige },
	{ "bisque", bisque },
	{ "black", black },
	{ "blanched_almond", blanched_almond },
	{ "blue", blue },
	{ "blue_violet", blue_violet },
	{ "brown", brown },
	{ "burly_wood", burly_wood },
	{ "cadet_blue", cadet_blue },
	{ "chartreuse", chartreuse },
	{ "chocolate", chocolate },
	{ "coral", coral },
	{ "cornflower_blue", cornflower_blue },
	{ "cornsilk", cornsilk },
	{ "crimson", crimson },
	{ "cyan", cyan },
	{ "cark_blue", cark_blue },
	{ "dark_cyan", dark_cyan },
	{ "dark_goldenrod", dark_goldenrod },
	{ "dark_gray", dark_gray },
	{ "dark_green", dark_green },
	{ "dark_khaki", dark_khaki },
	{ "dark_magenta", dark_magenta },
	{ "dark_olive_green", dark_olive_green },
	{ "dark_orange", dark_orange },
	{ "dark_orchid", dark_orchid },
	{ "dark_red", dark_red },
	{ "dark_salmon", dark_salmon },
	{ "dark_sea_green", dark_sea_green },
	{ "dark_slate_blue", dark_slate_blue },
	{ "dark_slate_gray", dark_slate_gray },
	{ "dark_turquoise", dark_turquoise },
	{ "dark_violet", dark_violet },
	{ "deep_pink", deep_pink },
	{ "deep_sky_blue", deep_sky_blue },
	{ "dim_gray", dim_gray },
	{ "dodger_blue", dodger_blue },
	{ "fire_brick", fire_brick },
	{ "floral_white", floral_white },
	{ "forest_green", forest_green },
	{ "fuchsia", fuchsia },
	{ "gainsboro", gainsboro },
	{ "ghost_white", ghost_white },
	{ "gold", gold },
	{ "goldenRod", goldenRod },
	{ "gray", gray },
	{ "green", green },
	{ "green_yellow", green_yellow },
	{ "honey_dew", honey_dew },
	{ "hot_pink", hot_pink },
	{ "indian_red", indian_red },
	{ "indigo", indigo },
	{ "ivory", ivory },
	{ "khaki", khaki },
	{ "lavender", lavender },
	{ "lavender_lush", lavender_lush },
	{ "lawn_green", lawn_green },
	{ "lemon_chiffon", lemon_chiffon },
	{ "light_blue", light_blue },
	{ "light_coral", light_coral },
	{ "light_cyan", light_cyan },
	{ "light_goldenrod_yellow", light_goldenrod_yellow },
	{ "light_gray", light_gray },
	{ "light_green", light_green },
	{ "light_pink", light_pink },
	{ "light_salmon", light_salmon },
	{ "light_sea_green", light_sea_green },
	{ "light_sky_blue", light_sky_blue },
	{ "light_slate_gray", light_slate_gray },
	{ "light_steel_blue", light_steel_blue },
	{ "light_yellow", light_yellow },
	{ "lime", lime },
	{ "lime_green", lime_green },
	{ "linen", linen },
	{ "magenta", magenta },
	{ "maroon", maroon },
	{ "mask_red", mask_red },
	{ "mask_green", mask_green },
	{ "mask_blue", mask_blue },
	{ "mask_alpha", mask_alpha },
	{ "medium_aquamarine", medium_aquamarine },
	{ "medium_blue", medium_blue },
	{ "medium_orchid", medium_orchid },
	{ "medium_purple", medium_purple },
	{ "medium_sea_green", medium_sea_green },
	{ "medium_slate_blue", medium_slate_blue },
	{ "medium_spring_green", medium_spring_green },
	{ "medium_turquoise", medium_turquoise },
	{ "medium_violet_red", medium_violet_red },
	{ "midnight_blue", midnight_blue },
	{ "mint_cream", mint_cream },
	{ "misty_rose", misty_rose },
	{ "moccasin", moccasin },
	{ "navajo_white", navajo_white },
	{ "navy", navy },
	{ "old_lace", old_lace },
	{ "olive", olive },
	{ "olive_drab", olive_drab },
	{ "orange", orange },
	{ "orange_red", orange_red },
	{ "orchid", orchid },
	{ "pale_goldenrod", pale_goldenrod },
	{ "pale_green", pale_green },
	{ "pale_turquoise", pale_turquoise },
	{ "pale_violet_red", pale_violet_red },
	{ "papaya_whip", papaya_whip },
	{ "peach_puff", peach_puff },
	{ "peru", peru },
	{ "pink", pink },
	{ "plum", plum },
	{ "powder_blue", powder_blue },
	{ "purple", purple },
	{ "red", red },
	{ "rosy_brown", rosy_brown },
	{ "royal_blue", royal_blue },
	{ "saddle_brown", saddle_brown },
	{ "salmon", salmon },
	{ "sandy_brown", sandy_brown },
	{ "sea_green", sea_green },
	{ "sea_shell", sea_shell },
	{ "sienna", sienna },
	{ "silver", silver },
	{ "sky_blue", sky_blue },
	{ "slate_blue", slate_blue },
	{ "slate_gray", slate_gray },
	{ "snow", snow },
	{ "springGreen", springGreen },
	{ "steelBlue", steelBlue },
	{ "tan_", tan_ },
	{ "teal", teal },
	{ "thistle", thistle },
	{ "tomato", tomato },
	{ "turquoise", turquoise },
	{ "violet", violet },
	{ "wheat", wheat },
	{ "white", white },
	{ "white_smoke", white_smoke },
	{ "yellow", yellow },
	{ "yellow_green", yellow_green },
	{ "microsoft_blue_pen", microsoft_blue_pen },
	{ "microsoft_blue_brush", microsoft_blue_brush },
	{ "tangent_space_normal_blue", tangent_space_normal_blue },
	{ "object_space_normal_green", object_space_normal_green },
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

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const color& value)
{
	const char* c = as_string(value);
	if (c) return strlcpy(_StrDestination, c, _SizeofStrDestination) < _SizeofStrDestination ? _StrDestination : nullptr;
	int r,g,b,a;
	value.decompose(&r, &g, &b, &a);
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

}
