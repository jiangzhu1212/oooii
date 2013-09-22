/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
// Encapsulate simple RGB(A) color as an argb (same as DirectX D3DCOLOR). This 
// is also known as a bgra.
#pragma once
#ifndef oBase_color_h
#define oBase_color_h

#include <oBase/byte.h>
#include <oBase/operators.h>
#include <limits>

// These are defined as ouro::colors below, but the macro helps in switch 
// statements.
#define oColorMaskRed 0x00FF0000
#define oColorMaskGreen 0x0000FF00
#define oColorMaskBlue 0x000000FF
#define oColorMaskAlpha 0xFF000000

namespace ouro {

class color : public oComparable<color>
{
public:
	color() { c.as_int = 0; }
	color(const color& _That) { c.as_int = _That.c.as_int; }
	color(const color& _That, int _NewAlpha) { c.as_int = _That.c.as_int; c.as_unsigned_char[3] = _NewAlpha & 0xff;  }
	color(const color& _That, float _NewAlpha) { c.as_int = _That.c.as_int; c.as_unsigned_char[3] = static_cast<unsigned char>(floor(_NewAlpha * 255.0f + 0.5f));  }
	color(int _ARGB) { c.as_int = _ARGB; }
	color(long _ARGB) { c.as_long = _ARGB; }
	color(unsigned int _ARGB) { c.as_unsigned_int = _ARGB; }
	color(unsigned long _ARGB) { c.as_unsigned_long = _ARGB; }
	color(int _R, int _G, int _B, int _A) { c.as_unsigned_char[0] = _B & 0xff; c.as_unsigned_char[1] = _G & 0xff; c.as_unsigned_char[2] = _R & 0xff; c.as_unsigned_char[3] = _A & 0xff; }
	color(float _R, float _G, float _B, float _A) { c.as_unsigned_char[0] = static_cast<unsigned char>(floor(_B * 255.0f + 0.5f)); c.as_unsigned_char[1] = static_cast<unsigned char>(floor(_G * 255.0f + 0.5f)); c.as_unsigned_char[2] = static_cast<unsigned char>(floor(_R * 255.0f + 0.5f)); c.as_unsigned_char[3] = static_cast<unsigned char>(floor(_A * 255.0f + 0.5f)); }

	operator int() const volatile { return c.as_int; }
	operator int&() { return c.as_int; }

	void decompose(int* _R, int* _G, int* _B) const { *_B = c.as_unsigned_char[0]; *_G = c.as_unsigned_char[1]; *_R = c.as_unsigned_char[2]; }
	void decompose(int* _R, int* _G, int* _B, int* _A) const { decompose(_R, _G, _B); *_A = c.as_unsigned_char[3]; }
	void decompose(float* _R, float* _G, float* _B) const { *_B = c.as_unsigned_char[0] / 255.0f; *_G = c.as_unsigned_char[1] / 255.0f; *_R = c.as_unsigned_char[2] / 255.0f; }
	void decompose(float* _R, float* _G, float* _B, float* _A) const { decompose(_R, _G, _B); *_A = c.as_unsigned_char[3] / 255.0f; }

	bool opaque() const { return c.as_unsigned_char[3] == 255; }
	bool translucent() const { return !opaque(); }
	bool transparent() const { return c.as_unsigned_char[3] == 0; }

	float luminance() const { float R, G, B, A; decompose(&R, &G, &B, &A); return 0.2126f*R + 0.7152f*G + 0.0722f*B; }

	bool operator<(const color& _That) const { return (c.as_unsigned_short[0] != _That.c.as_unsigned_short[0] || c.as_unsigned_char[2] != _That.c.as_unsigned_char[3]) ? (luminance() < _That.luminance()) : (c.as_unsigned_char[3] < _That.c.as_unsigned_char[3]); }
	bool operator==(const color& _That) const { return c.as_int == _That.c.as_int; }

	const color& operator+=(const color& _That) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] += _That.c.as_unsigned_char[i]; return *this; }
	const color& operator-=(const color& _That) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] -= _That.c.as_unsigned_char[i]; return *this; }
	const color& operator*=(const color& _That) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] *= _That.c.as_unsigned_char[i]; return *this; }
	const color& operator/=(const color& _That) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] /= _That.c.as_unsigned_char[i]; return *this; }

	inline color operator*=(float x) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] = static_cast<unsigned char>(c.as_unsigned_char[i] * x + 0.5f); return *this; }
	inline color operator/=(float x) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] = static_cast<unsigned char>(c.as_unsigned_char[i] / x + 0.5f); return *this; }
	inline color operator*=(double x) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] = static_cast<unsigned char>(c.as_unsigned_char[i] * x + 0.5); return *this; }
	inline color operator/=(double x) { for (size_t i = 0; i < 4; i++) c.as_unsigned_char[i] = static_cast<unsigned char>(c.as_unsigned_char[i] / x + 0.5); return *this; }

	oOPERATORS_DERIVED(color, +) oOPERATORS_DERIVED(color, -) oOPERATORS_DERIVED(color, *) oOPERATORS_DERIVED(color, /)
	oOPERATORS_DERIVED_COMMUTATIVE2(color, float, *) oOPERATORS_DERIVED2(color, float, /)
	oOPERATORS_DERIVED_COMMUTATIVE2(color, double, *) oOPERATORS_DERIVED2(color, double, /)

private:
	ouro::byte_swizzle32 c;
};

// Returns the specified color as a readable string if it matches one of the 
// standard color definitions below. If not, then this returns nullptr.
const char* as_string(const color& _color);

// fills _pColor with a standard color name. This returns false if the string
// is not one of the standard colors. This does not convert to a numeric 
// string.
bool from_string(color* _pColor, const char* _String);

namespace detail
{
	// this gives more precision that using color math directly
	inline int lerp(int a, int b, float s) { return static_cast<int>(a + s * (b-a) + 0.5f); }
}

inline color lerp(const color& a, const color& b, float s)
{
	int ra,ga,ba,aa, rb,gb,bb,ab;
	a.decompose(&ra, &ga, &ba, &aa); b.decompose(&rb, &gb, &bb, &ab);
	return color(detail::lerp(ra,rb,s), detail::lerp(ga,gb,s), detail::lerp(ba,bb,s), detail::lerp(aa,ab,s));
}

// Returns the index into the specified palette of the color that most closely
// matches _Color.
inline size_t palettize(const color& _Color, const color* _pPalette, size_t _NumColorsInPalette)
{
	const float kTestLum = _Color.luminance();
	size_t closest = std::numeric_limits<size_t>::max();
	float minDelta = std::numeric_limits<float>::max();
	for (size_t i = 0; i < _NumColorsInPalette; i++)
	{
		float delta = abs(kTestLum - _pPalette[i].luminance());
		if (delta < minDelta)
		{
			minDelta = delta;
			closest = i;
		}
	}

	return closest;
}
template<size_t size> inline size_t palettize(color _TestColor, const color (&_pPaletteColors)[size]) { return palettize(_TestColor, _pPaletteColors, size); }

// Pic of all the colors: http://www.codeproject.com/KB/GDI/XHtmlDraw/XHtmlDraw4.png
static const color AliceBlue(0xFFF0F8FF);
static const color AntiqueWhite(0xFFFAEBD7);
static const color Aqua(0xFF00FFFF);
static const color Aquamarine(0xFF7FFFD4);
static const color Azure(0xFFF0FFFF);
static const color Beige(0xFFF5F5DC);
static const color Bisque(0xFFFFE4C4);
static const color Black(0xFF000000);
static const color AlmostBlack(0xFF252525); // useful for a renderer's clear color so black doesn't hide unlit draws
static const color BlanchedAlmond(0xFFFFEBCD);
static const color Blue(0xFF0000FF);
static const color BlueViolet(0xFF8A2BE2);
static const color Brown(0xFFA52A2A);
static const color BurlyWood(0xFFDEB887);
static const color CadetBlue(0xFF5F9EA0);
static const color Chartreuse(0xFF7FFF00);
static const color Chocolate(0xFFD2691E);
static const color Coral(0xFFFF7F50);
static const color CornflowerBlue(0xFF6495ED);
static const color Cornsilk(0xFFFFF8DC);
static const color Crimson(0xFFDC143C);
static const color Cyan(0xFF00FFFF);
static const color DarkBlue(0xFF00008B);
static const color DarkCyan(0xFF008B8B);
static const color DarkGoldenRod(0xFFB8860B);
static const color DarkGray(0xFFA9A9A9);
static const color DarkGreen(0xFF006400);
static const color DarkKhaki(0xFFBDB76B);
static const color DarkMagenta(0xFF8B008B);
static const color DarkOliveGreen(0xFF556B2F);
static const color Darkorange(0xFFFF8C00);
static const color DarkOrchid(0xFF9932CC);
static const color DarkRed(0xFF8B0000);
static const color DarkSalmon(0xFFE9967A);
static const color DarkSeaGreen(0xFF8FBC8F);
static const color DarkSlateBlue(0xFF483D8B);
static const color DarkSlateGray(0xFF2F4F4F);
static const color DarkTurquoise(0xFF00CED1);
static const color DarkViolet(0xFF9400D3);
static const color DeepPink(0xFFFF1493);
static const color DeepSkyBlue(0xFF00BFFF);
static const color DimGray(0xFF696969);
static const color DodgerBlue(0xFF1E90FF);
static const color FireBrick(0xFFB22222);
static const color FloralWhite(0xFFFFFAF0);
static const color ForestGreen(0xFF228B22);
static const color Fuchsia(0xFFFF00FF);
static const color Gainsboro(0xFFDCDCDC);
static const color GhostWhite(0xFFF8F8FF);
static const color Gold(0xFFFFD700);
static const color GoldenRod(0xFFDAA520);
static const color Gray(0xFF808080);
static const color Green(0xFF008000);
static const color GreenYellow(0xFFADFF2F);
static const color HoneyDew(0xFFF0FFF0);
static const color HotPink(0xFFFF69B4);
static const color IndianRed (0xFFCD5C5C);
static const color Indigo (0xFF4B0082);
static const color Ivory(0xFFFFFFF0);
static const color Khaki(0xFFF0E68C);
static const color Lavender(0xFFE6E6FA);
static const color LavenderBlush(0xFFFFF0F5);
static const color LawnGreen(0xFF7CFC00);
static const color LemonChiffon(0xFFFFFACD);
static const color LightBlue(0xFFADD8E6);
static const color LightCoral(0xFFF08080);
static const color LightCyan(0xFFE0FFFF);
static const color LightGoldenRodYellow(0xFFFAFAD2);
static const color LightGrey(0xFFD3D3D3);
static const color LightGreen(0xFF90EE90);
static const color LightPink(0xFFFFB6C1);
static const color LightSalmon(0xFFFFA07A);
static const color LightSeaGreen(0xFF20B2AA);
static const color LightSkyBlue(0xFF87CEFA);
static const color LightSlateGray(0xFF778899);
static const color LightSteelBlue(0xFFB0C4DE);
static const color LightYellow(0xFFFFFFE0);
static const color Lime(0xFF00FF00);
static const color LimeGreen(0xFF32CD32);
static const color Linen(0xFFFAF0E6);
static const color Magenta(0xFFFF00FF);
static const color Maroon(0xFF800000);
static const color MediumAquaMarine(0xFF66CDAA);
static const color MediumBlue(0xFF0000CD);
static const color MediumOrchid(0xFFBA55D3);
static const color MediumPurple(0xFF9370D8);
static const color MediumSeaGreen(0xFF3CB371);
static const color MediumSlateBlue(0xFF7B68EE);
static const color MediumSpringGreen(0xFF00FA9A);
static const color MediumTurquoise(0xFF48D1CC);
static const color MediumVioletRed(0xFFC71585);
static const color MidnightBlue(0xFF191970);
static const color MintCream(0xFFF5FFFA);
static const color MistyRose(0xFFFFE4E1);
static const color Moccasin(0xFFFFE4B5);
static const color NavajoWhite(0xFFFFDEAD);
static const color Navy(0xFF000080);
static const color OldLace(0xFFFDF5E6);
static const color Olive(0xFF808000);
static const color OliveDrab(0xFF6B8E23);
static const color Orange(0xFFFFA500);
static const color OrangeRed(0xFFFF4500);
static const color Orchid(0xFFDA70D6);
static const color PaleGoldenRod(0xFFEEE8AA);
static const color PaleGreen(0xFF98FB98);
static const color PaleTurquoise(0xFFAFEEEE);
static const color PaleVioletRed(0xFFD87093);
static const color PapayaWhip(0xFFFFEFD5);
static const color PeachPuff(0xFFFFDAB9);
static const color Peru(0xFFCD853F);
static const color Pink(0xFFFFC0CB);
static const color Plum(0xFFDDA0DD);
static const color PowderBlue(0xFFB0E0E6);
static const color Purple(0xFF800080);
static const color Red(0xFFFF0000);
static const color RosyBrown(0xFFBC8F8F);
static const color RoyalBlue(0xFF4169E1);
static const color SaddleBrown(0xFF8B4513);
static const color Salmon(0xFFFA8072);
static const color SandyBrown(0xFFF4A460);
static const color SeaGreen(0xFF2E8B57);
static const color SeaShell(0xFFFFF5EE);
static const color Sienna(0xFFA0522D);
static const color Silver(0xFFC0C0C0);
static const color SkyBlue(0xFF87CEEB);
static const color SlateBlue(0xFF6A5ACD);
static const color SlateGray(0xFF708090);
static const color Snow(0xFFFFFAFA);
static const color SpringGreen(0xFF00FF7F);
static const color SteelBlue(0xFF4682B4);
static const color Tan(0xFFD2B48C);
static const color Teal(0xFF008080);
static const color Thistle(0xFFD8BFD8);
static const color Tomato(0xFFFF6347);
static const color Turquoise(0xFF40E0D0);
static const color Violet(0xFFEE82EE);
static const color Wheat(0xFFF5DEB3);
static const color White(0xFFFFFFFF);
static const color WhiteSmoke(0xFFF5F5F5);
static const color Yellow(0xFFFFFF00);
static const color YellowGreen(0xFF9ACD32);
static const color OOOiiGreen(0xFF8DC81D);
static const color MicrosoftBluePen(0xFFFF3232);
static const color MicrosoftBlueBrush(0xFFA08064);
static const color TangentSpaceNormalBlue(0xFF7F7FFF); // Z-Up
static const color ObjectSpaceNormalGreen(0xFF7FFF7F); // Y-Up

static const color MaskRed(oColorMaskRed);
static const color MaskGreen(oColorMaskGreen);
static const color MaskBlue(oColorMaskBlue);
static const color MaskAlpha(oColorMaskAlpha);

} // namespace ouro

using ouro::lerp; // promote to same level as oHLSL and other lerps.

#endif
