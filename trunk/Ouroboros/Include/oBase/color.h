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
// Encapsulate simple RGB(A) color as an argb (aka bgra, same as D3DCOLOR). 
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
	color(const color& _That, float _NewAlpha) { c.as_int = _That.c.as_int; c.as_unsigned_char[3] = f32ton8(_NewAlpha);  }
	color(int _ARGB) { c.as_int = _ARGB; }
	color(long _ARGB) { c.as_long = _ARGB; }
	color(unsigned int _ARGB) { c.as_unsigned_int = _ARGB; }
	color(unsigned long _ARGB) { c.as_unsigned_long = _ARGB; }
	color(int _R, int _G, int _B, int _A) { c.as_unsigned_char[0] = _B & 0xff; c.as_unsigned_char[1] = _G & 0xff; c.as_unsigned_char[2] = _R & 0xff; c.as_unsigned_char[3] = _A & 0xff; }
	color(float _R, float _G, float _B, float _A) { c.as_unsigned_char[0] = f32ton8(_B); c.as_unsigned_char[1] = f32ton8(_G); c.as_unsigned_char[2] = f32ton8(_R); c.as_unsigned_char[3] = f32ton8(_A); }

	operator int() const volatile { return c.as_int; }
	operator int&() { return c.as_int; }

	void decompose(int* _R, int* _G, int* _B) const { *_B = c.as_unsigned_char[0]; *_G = c.as_unsigned_char[1]; *_R = c.as_unsigned_char[2]; }
	void decompose(int* _R, int* _G, int* _B, int* _A) const { decompose(_R, _G, _B); *_A = c.as_unsigned_char[3]; }
	void decompose(float* _R, float* _G, float* _B) const { *_B = n8tof32(c.as_unsigned_char[0]); *_G = n8tof32(c.as_unsigned_char[1]); *_R = n8tof32(c.as_unsigned_char[2]); }
	void decompose(float* _R, float* _G, float* _B, float* _A) const { decompose(_R, _G, _B); *_A = n8tof32(c.as_unsigned_char[3]); }

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

} // namespace ouro

// promote to same level as oHLSL and other lerps. This also gets around a C++ standard 
// vaguery/compiler bug where if this were in the ouro namespace, then it would forever 
// block matching to ::lerp().
inline ouro::color lerp(const ouro::color& a, const ouro::color& b, float s)
{
	int ra,ga,ba,aa, rb,gb,bb,ab;
	a.decompose(&ra, &ga, &ba, &aa); b.decompose(&rb, &gb, &bb, &ab);
	return ouro::color(ouro::detail::lerp(ra,rb,s), ouro::detail::lerp(ga,gb,s), ouro::detail::lerp(ba,bb,s), ouro::detail::lerp(aa,ab,s));
}

#endif
