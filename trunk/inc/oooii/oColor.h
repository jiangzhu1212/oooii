// $(header)

// A simple way to pass colors around to APIs that need them along with some 
// common colors to avoid having poor programmers try to use mspaint to get a
// pretty color.
#pragma once
#ifndef oColor_h
#define oColor_h
// Stored in argb, same as a DirectX D3DCOLOR

struct oColor
{
	unsigned int c;

	oColor() : c(0) {}
	oColor(unsigned int _C) : c(_C) {}

	operator const unsigned int() const volatile { return c; }
	operator unsigned int&() { return c; }
};

inline oColor oComposeColor(unsigned int _R, unsigned int _G, unsigned int _B, unsigned int _A) { return ((_A&0xff)<<24)|((_R&0xff)<<16)|((_G&0xff)<<8)|(_B&0xff); }
inline oColor oComposeColor(float _R, float _G, float _B, float _A) { return oComposeColor(static_cast<unsigned int>(_R*255.0f), static_cast<unsigned int>(_G*255.0f), static_cast<unsigned int>(_B*255.0f), static_cast<unsigned int>(_A*255.0f)); }
inline void oDecomposeColor(oColor _Color, unsigned int* _R, unsigned int* _G, unsigned int* _B, unsigned int* _A) { *_B = _Color&0xff; *_G = (_Color>>8)&0xff; *_R = (_Color>>16)&0xff; *_A = (_Color>>24)&0xff; }
inline void oDecomposeColor(oColor _Color, float* _pR, float* _pG, float* _pB, float* _pA) { unsigned int r, g, b, a; oDecomposeColor(_Color, &r, &g, &b, &a); *_pR = r / 255.0f; *_pG = g / 255.0f; *_pB = b / 255.0f; *_pA = a / 255.0f; }
inline void oDecomposeColor(oColor _Color, float* _pRGBA) { oDecomposeColor(_Color, &_pRGBA[0], &_pRGBA[1], &_pRGBA[2], &_pRGBA[3]); }
inline void oDecomposeColorRGB(oColor _Color, float* _pRGB) { float a; oDecomposeColor(_Color, &_pRGB[0], &_pRGB[1], &_pRGB[2], &a); }
template<typename float4T> inline float4T oDecomposeColor(oColor _Color) { float4T c; oDecomposeColor(_Color, (float*)&c); return c; }
template<typename float3T> inline float3T oDecomposeColorRGB(oColor _Color) { float3T c; oDecomposeColorRGB(_Color, (float*)&c); return c; }
inline void oDecomposeColor(oColor _Color, unsigned int *_pRGBA) { oDecomposeColor(_Color, &_pRGBA[0], &_pRGBA[1], &_pRGBA[2], &_pRGBA[3]); }
inline void oDecomposeColorRGB(oColor _Color, unsigned int *_pRGB) { unsigned int a; oDecomposeColor(_Color, &_pRGB[0], &_pRGB[1], &_pRGB[2], &a); }
inline float oGetLuminance(oColor _Color) { float _R,_G,_B,_A; oDecomposeColor(_Color, &_R, &_G, &_B, &_A); return 0.2126f*_R + 0.7152f*_G + 0.0722f*_B; }
inline bool oIsOpaqueColor(oColor _Color) { return ((_Color>>24)&0xff) != 0; }
inline bool oIsTransparentColor(oColor _Color) { return (_Color&0xff000000) == 0; }
inline bool oIsTranslucentColor(oColor _Color) { return (_Color&0xff000000) != 0xff000000; }
inline unsigned int absdiff__(unsigned int a, unsigned int b) { return (a > b) ? (a-b) : (b-a); }

// lerp lerps each channel separately
inline unsigned int lerp__(unsigned int a, unsigned int b, float s) { return static_cast<unsigned int>(a + s * (b-a)); }
inline oColor lerp(const oColor& a, const oColor& b, float s)
{
	unsigned int ra,ga,ba,aa, rb,gb,bb,ab;
	oDecomposeColor(a, &ra, &ga, &ba, &aa);
	oDecomposeColor(b, &rb, &gb, &bb, &ab);
	return oComposeColor(lerp__(ra,rb,s), lerp__(ga,gb,s), lerp__(ba,bb,s), lerp__(aa,ab,s));
}

// Sometimes colors are generated from the same hardware, but there is precision
// variability. In those cases, use this to fuzzy-compare color values
inline bool oEqual(oColor _Color1, oColor _Color2, unsigned int _BitFuzziness)
{
	unsigned int r1,g1,b1,a1,r2,g2,b2,a2;
	oDecomposeColor(_Color1, &r1, &g1, &b1, &a1);
	oDecomposeColor(_Color2, &r2, &g2, &b2, &a2);
	return (absdiff__(r1, r2) <= _BitFuzziness) && (absdiff__(g1, g2) <= _BitFuzziness) && (absdiff__(b1, b2) <= _BitFuzziness) && (absdiff__(a1, a2) <= _BitFuzziness);
}

// Does a per-component diff and returns the results as an oColor
inline oColor oDiffColors(oColor _Color1, oColor _Color2, unsigned int _Multiplier = 1)
{
	unsigned int r1,g1,b1,a1,r2,g2,b2,a2;
	oDecomposeColor(_Color1, &r1, &g1, &b1, &a1);
	oDecomposeColor(_Color2, &r2, &g2, &b2, &a2);
	return oComposeColor(_Multiplier * absdiff__(r1, r2), _Multiplier * absdiff__(g1, g2), _Multiplier * absdiff__(b1, b2), _Multiplier * absdiff__(a1, a2));
}

inline oColor oDiffColorsRGB(oColor _Color1, oColor _Color2, unsigned int _Multiplier = 1)
{
	unsigned int r1,g1,b1,a1,r2,g2,b2,a2;
	oDecomposeColor(_Color1, &r1, &g1, &b1, &a1);
	oDecomposeColor(_Color2, &r2, &g2, &b2, &a2);
	return oComposeColor(_Multiplier * absdiff__(r1, r2), _Multiplier * absdiff__(g1, g2), _Multiplier * absdiff__(b1, b2), 0xff);
}

// Hue is in degrees, saturation and value are [0,1]
void oDecomposeToHSV(oColor _Color, float* _pH, float* _pS, float* _pV);

// Pic of all the colors: http://www.codeproject.com/KB/GDI/XHtmlDraw/XHtmlDraw4.png
namespace std {
	static const oColor AliceBlue = 0xFFF0F8FF;
	static const oColor AntiqueWhite = 0xFFFAEBD7;
	static const oColor Aqua = 0xFF00FFFF;
	static const oColor Aquamarine = 0xFF7FFFD4;
	static const oColor Azure = 0xFFF0FFFF;
	static const oColor Beige = 0xFFF5F5DC;
	static const oColor Bisque = 0xFFFFE4C4;
	static const oColor Black = 0xFF000000;
	static const oColor BlanchedAlmond = 0xFFFFEBCD;
	static const oColor Blue = 0xFF0000FF;
	static const oColor BlueViolet = 0xFF8A2BE2;
	static const oColor Brown = 0xFFA52A2A;
	static const oColor BurlyWood = 0xFFDEB887;
	static const oColor CadetBlue = 0xFF5F9EA0;
	static const oColor Chartreuse = 0xFF7FFF00;
	static const oColor Chocolate = 0xFFD2691E;
	static const oColor Coral = 0xFFFF7F50;
	static const oColor CornflowerBlue = 0xFF6495ED;
	static const oColor Cornsilk = 0xFFFFF8DC;
	static const oColor Crimson = 0xFFDC143C;
	static const oColor Cyan = 0xFF00FFFF;
	static const oColor DarkBlue = 0xFF00008B;
	static const oColor DarkCyan = 0xFF008B8B;
	static const oColor DarkGoldenRod = 0xFFB8860B;
	static const oColor DarkGray = 0xFFA9A9A9;
	static const oColor DarkGreen = 0xFF006400;
	static const oColor DarkKhaki = 0xFFBDB76B;
	static const oColor DarkMagenta = 0xFF8B008B;
	static const oColor DarkOliveGreen = 0xFF556B2F;
	static const oColor Darkorange = 0xFFFF8C00;
	static const oColor DarkOrchid = 0xFF9932CC;
	static const oColor DarkRed = 0xFF8B0000;
	static const oColor DarkSalmon = 0xFFE9967A;
	static const oColor DarkSeaGreen = 0xFF8FBC8F;
	static const oColor DarkSlateBlue = 0xFF483D8B;
	static const oColor DarkSlateGray = 0xFF2F4F4F;
	static const oColor DarkTurquoise = 0xFF00CED1;
	static const oColor DarkViolet = 0xFF9400D3;
	static const oColor DeepPink = 0xFFFF1493;
	static const oColor DeepSkyBlue = 0xFF00BFFF;
	static const oColor DimGray = 0xFF696969;
	static const oColor DodgerBlue = 0xFF1E90FF;
	static const oColor FireBrick = 0xFFB22222;
	static const oColor FloralWhite = 0xFFFFFAF0;
	static const oColor ForestGreen = 0xFF228B22;
	static const oColor Fuchsia = 0xFFFF00FF;
	static const oColor Gainsboro = 0xFFDCDCDC;
	static const oColor GhostWhite = 0xFFF8F8FF;
	static const oColor Gold = 0xFFFFD700;
	static const oColor GoldenRod = 0xFFDAA520;
	static const oColor Gray = 0xFF808080;
	static const oColor Green = 0xFF008000;
	static const oColor GreenYellow = 0xFFADFF2F;
	static const oColor HoneyDew = 0xFFF0FFF0;
	static const oColor HotPink = 0xFFFF69B4;
	static const oColor IndianRed  = 0xFFCD5C5C;
	static const oColor Indigo  = 0xFF4B0082;
	static const oColor Ivory = 0xFFFFFFF0;
	static const oColor Khaki = 0xFFF0E68C;
	static const oColor Lavender = 0xFFE6E6FA;
	static const oColor LavenderBlush = 0xFFFFF0F5;
	static const oColor LawnGreen = 0xFF7CFC00;
	static const oColor LemonChiffon = 0xFFFFFACD;
	static const oColor LightBlue = 0xFFADD8E6;
	static const oColor LightCoral = 0xFFF08080;
	static const oColor LightCyan = 0xFFE0FFFF;
	static const oColor LightGoldenRodYellow = 0xFFFAFAD2;
	static const oColor LightGrey = 0xFFD3D3D3;
	static const oColor LightGreen = 0xFF90EE90;
	static const oColor LightPink = 0xFFFFB6C1;
	static const oColor LightSalmon = 0xFFFFA07A;
	static const oColor LightSeaGreen = 0xFF20B2AA;
	static const oColor LightSkyBlue = 0xFF87CEFA;
	static const oColor LightSlateGray = 0xFF778899;
	static const oColor LightSteelBlue = 0xFFB0C4DE;
	static const oColor LightYellow = 0xFFFFFFE0;
	static const oColor Lime = 0xFF00FF00;
	static const oColor LimeGreen = 0xFF32CD32;
	static const oColor Linen = 0xFFFAF0E6;
	static const oColor Magenta = 0xFFFF00FF;
	static const oColor Maroon = 0xFF800000;
	static const oColor MediumAquaMarine = 0xFF66CDAA;
	static const oColor MediumBlue = 0xFF0000CD;
	static const oColor MediumOrchid = 0xFFBA55D3;
	static const oColor MediumPurple = 0xFF9370D8;
	static const oColor MediumSeaGreen = 0xFF3CB371;
	static const oColor MediumSlateBlue = 0xFF7B68EE;
	static const oColor MediumSpringGreen = 0xFF00FA9A;
	static const oColor MediumTurquoise = 0xFF48D1CC;
	static const oColor MediumVioletRed = 0xFFC71585;
	static const oColor MidnightBlue = 0xFF191970;
	static const oColor MintCream = 0xFFF5FFFA;
	static const oColor MistyRose = 0xFFFFE4E1;
	static const oColor Moccasin = 0xFFFFE4B5;
	static const oColor NavajoWhite = 0xFFFFDEAD;
	static const oColor Navy = 0xFF000080;
	static const oColor OldLace = 0xFFFDF5E6;
	static const oColor Olive = 0xFF808000;
	static const oColor OliveDrab = 0xFF6B8E23;
	static const oColor Orange = 0xFFFFA500;
	static const oColor OrangeRed = 0xFFFF4500;
	static const oColor Orchid = 0xFFDA70D6;
	static const oColor PaleGoldenRod = 0xFFEEE8AA;
	static const oColor PaleGreen = 0xFF98FB98;
	static const oColor PaleTurquoise = 0xFFAFEEEE;
	static const oColor PaleVioletRed = 0xFFD87093;
	static const oColor PapayaWhip = 0xFFFFEFD5;
	static const oColor PeachPuff = 0xFFFFDAB9;
	static const oColor Peru = 0xFFCD853F;
	static const oColor Pink = 0xFFFFC0CB;
	static const oColor Plum = 0xFFDDA0DD;
	static const oColor PowderBlue = 0xFFB0E0E6;
	static const oColor Purple = 0xFF800080;
	static const oColor Red = 0xFFFF0000;
	static const oColor RosyBrown = 0xFFBC8F8F;
	static const oColor RoyalBlue = 0xFF4169E1;
	static const oColor SaddleBrown = 0xFF8B4513;
	static const oColor Salmon = 0xFFFA8072;
	static const oColor SandyBrown = 0xFFF4A460;
	static const oColor SeaGreen = 0xFF2E8B57;
	static const oColor SeaShell = 0xFFFFF5EE;
	static const oColor Sienna = 0xFFA0522D;
	static const oColor Silver = 0xFFC0C0C0;
	static const oColor SkyBlue = 0xFF87CEEB;
	static const oColor SlateBlue = 0xFF6A5ACD;
	static const oColor SlateGray = 0xFF708090;
	static const oColor Snow = 0xFFFFFAFA;
	static const oColor SpringGreen = 0xFF00FF7F;
	static const oColor SteelBlue = 0xFF4682B4;
	static const oColor Tan = 0xFFD2B48C;
	static const oColor Teal = 0xFF008080;
	static const oColor Thistle = 0xFFD8BFD8;
	static const oColor Tomato = 0xFFFF6347;
	static const oColor Turquoise = 0xFF40E0D0;
	static const oColor Violet = 0xFFEE82EE;
	static const oColor Wheat = 0xFFF5DEB3;
	static const oColor White = 0xFFFFFFFF;
	static const oColor WhiteSmoke = 0xFFF5F5F5;
	static const oColor Yellow = 0xFFFFFF00;
	static const oColor YellowGreen = 0xFF9ACD32;
	static const oColor OOOiiGreen = 0xFF8DC81D;
	static const oColor TangentSpaceNormalBlue = 0xFF7F7FFF; // Z-Up
	static const oColor ObjectSpaceNormalGreen = 0xFF7FFF7F; // Y-Up
}

#endif
