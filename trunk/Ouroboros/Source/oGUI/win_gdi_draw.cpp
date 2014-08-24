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
#include <oGUI/Windows/win_gdi_draw.h>
#include <oGUI/windows/win_gdi_bitmap.h>
#include <oGUI/windows/oWinRect.h>
#include <oCore/windows/win_error.h>
//#include <oBase/byte.h>


namespace ouro {
	namespace windows {
		namespace gdi {

void draw_line(HDC _hDC, const int2& _P0, const int2& _P1)
{
	oVB(MoveToEx(_hDC, _P0.x, _P0.y, nullptr));
	oVB(LineTo(_hDC, _P1.x, _P1.y));
}

void draw_box(HDC _hDC, const RECT& _rBox, int _EdgeRoundness, float _Alpha)
{
	if (equal(_Alpha, 1.0f))
	{
		if (_EdgeRoundness)
			oVB(RoundRect(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom, _EdgeRoundness, _EdgeRoundness));
		else
			oVB(Rectangle(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom));
		return;
	}

	// Copy the contents out (is there no way to access things directly?)
	BITMAPINFO bmi = {0};
	surface::info si;
	si.dimensions = int3(oWinRectW(_rBox), oWinRectH(_rBox), 1);
	si.format = surface::format::b8g8r8a8_unorm;

	bmi.bmiHeader = make_header(si);
	if (!bmi.bmiHeader.biWidth || !bmi.bmiHeader.biHeight)
		return;
	
	HDC hDCBitmap = CreateCompatibleDC(_hDC);
	scoped_bitmap hBmp(CreateDIBSection(hDCBitmap, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, nullptr, nullptr, 0));

	scoped_select ScopedSelectBmp(hDCBitmap, hBmp);
	scoped_select ScopedSelectPen(hDCBitmap, current_pen(_hDC));
	scoped_select ScopedSelectBrush(hDCBitmap, current_brush(_hDC));

	// Resolve areas around the curved corners so we can to the blend directly
	// later on. But just do the minimum necessary, don't copy the interior that
	// we're going to re-draw with a RECT.
	if (_EdgeRoundness)
	{
		// Minimize rects...
		int dist = _EdgeRoundness;
		RECT TL = { _rBox.left, _rBox.top, _rBox.left + dist, _rBox.top + dist };
		RECT RT = { _rBox.right - dist, _rBox.top, _rBox.right, _rBox.top + dist };
		RECT BL = { _rBox.left, _rBox.bottom - dist, _rBox.left + dist, _rBox.bottom };
		RECT BR = { _rBox.right - dist, _rBox.bottom - dist, _rBox.right, _rBox.bottom };
		BitBlt(hDCBitmap, 0, 0, dist, dist, _hDC, _rBox.left, _rBox.top, SRCCOPY);
		BitBlt(hDCBitmap, bmi.bmiHeader.biWidth - dist, 0, dist, dist, _hDC, _rBox.right - dist, _rBox.top, SRCCOPY);
		BitBlt(hDCBitmap, 0, bmi.bmiHeader.biHeight - dist, dist, dist, _hDC, _rBox.left, _rBox.bottom - dist, SRCCOPY);
		BitBlt(hDCBitmap, bmi.bmiHeader.biWidth - dist, bmi.bmiHeader.biHeight - dist, dist, dist, _hDC, _rBox.right - dist, _rBox.bottom - dist, SRCCOPY);
		oVB(RoundRect(hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, _EdgeRoundness, _EdgeRoundness));
	}

	else
		oVB(Rectangle(hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight));

	BLENDFUNCTION blend = { AC_SRC_OVER, 0, (BYTE)(static_cast<int>(_Alpha * 255.0f) & 0xff), 0 };
	oVB(AlphaBlend(_hDC, _rBox.left, _rBox.top, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hDCBitmap, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, blend));
}

void draw_ellipse(HDC _hDC, const RECT& _rBox)
{
	oVB(Ellipse(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom));
}

RECT calc_text_rect(HDC _hDC, const char* _Text)
{
	RECT r = {0,0,0,0};
	oVB(DrawText(_hDC, _Text, -1, &r, DT_CALCRECT|DT_LEFT));
	return r;
}

static void draw_text(HDC _hDC, const text_info& _Desc, const char* _Text, RECT* _pActual)
{
	int r,g,b,a;
	_Desc.foreground.decompose(&r, &g, &b, &a);

	int br,bg,bb,ba;
	_Desc.background.decompose(&br, &bg, &bb, &ba);

	int sr,sg,sb,sa;
	_Desc.shadow.decompose(&sr, &sg, &sb, &sa);

	if (!a)
	{
		if (!ba) oTHROW_INVARG0();
		r = br;
		g = bg;
		b = bb;
	}

	UINT uFormat = DT_WORDBREAK;
	switch (_Desc.alignment % 3)
	{
		case 0: uFormat |= DT_LEFT; break;
		case 1: uFormat |= DT_CENTER; break;
		case 2: uFormat |= DT_RIGHT; break;
		oNODEFAULT;
	}

	switch (_Desc.alignment / 3)
	{
		case 0: uFormat |= DT_TOP; break;
		case 1: uFormat |= DT_VCENTER; break;
		case 2: uFormat |= DT_BOTTOM; break;
		oNODEFAULT;
	}

	bool forcedSingleLine = !_Desc.single_line && ((uFormat & DT_BOTTOM) || (uFormat & DT_VCENTER));
	if (forcedSingleLine || _Desc.single_line)
	{
		if (forcedSingleLine)
			oTRACE_ONCE("GDI doesn't support multi-line, vertically aligned text. See DrawText docs for more details. http://msdn.microsoft.com/en-us/library/ms901121.aspx");
		uFormat &=~ DT_WORDBREAK;
		uFormat |= DT_SINGLELINE;
	}

	if (_pActual)
		uFormat |= DT_CALCRECT;

	RECT rect = oWinRectWH(_Desc.position, _Desc.size);
	if (!_pActual)
		_pActual = &rect;
	else
	{
		_pActual->top = 0;
		_pActual->left = 0;
		_pActual->right = 1;
		_pActual->bottom = 1;
	}

	if (sa && any(_Desc.shadow_offset != int2(0,0)))
	{
		// If the background is opaque, cast an opaque shadow
		scoped_text_color ShadowState(_hDC, RGB(sr,sg,sb), RGB(sr,sg,sb), ba);
		RECT rShadow = oWinRectTranslate(*_pActual, _Desc.shadow_offset);
		DrawTextA(_hDC, _Text, -1, &rShadow, uFormat);
	}
	
	scoped_text_color TextState(_hDC, RGB(r,g,b), RGB(br,bg,bb), ba);
	DrawTextA(_hDC, _Text, -1, _pActual, uFormat);
}

RECT calc_text_rect(HDC _hDC, const text_info& _Desc, const char* _Text)
{
	RECT r = oWinRectWH(_Desc.position, _Desc.size);
	draw_text(_hDC, _Desc, _Text, &r);
	return r;
}

void draw_text(HDC _hDC, const text_info& _Desc, const char* _Text)
{
	draw_text(_hDC, _Desc, _Text, nullptr);
}

HPEN make_pen(color _Color, int _Width)
{
	int r,g,b,a;
	_Color.decompose(&r, &g, &b, &a);
	return CreatePen(a ? PS_SOLID : PS_NULL, _Width, RGB(r,g,b));
}

HBRUSH make_brush(color _Color)
{
	int r,g,b,a;
	_Color.decompose(&r, &g, &b, &a);
	return a ? CreateSolidBrush(RGB(r,g,b)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

COLORREF pen_color(HPEN _hPen, int* _pWidth)
{
	LOGPEN lp;
	if (!GetObject(_hPen, sizeof(LOGPEN), &lp))
	{
		EXTLOGPEN elp;
		GetObject(_hPen, sizeof(EXTLOGPEN), &elp);
		if (_pWidth)
			*_pWidth = static_cast<int>(elp.elpWidth);
		return elp.elpColor;
	}

	if (_pWidth)
		*_pWidth = static_cast<int>(lp.lopnWidth.x);
	return lp.lopnColor;
}

COLORREF brush_color(HBRUSH _hBrush)
{
	LOGBRUSH lb;
	GetObject(_hBrush, sizeof(LOGBRUSH), &lb);
	return lb.lbColor;
}

HFONT make_font(const font_info& _Desc)
{
	scoped_getdc hDC(GetDesktopWindow());
	HFONT hFont = CreateFont(
		point_to_logical_heightf(hDC, _Desc.point_size)
		, 0
		, 0
		, 0
		, _Desc.bold ? FW_BOLD : FW_NORMAL
		, _Desc.italic
		, _Desc.underline
		, _Desc.strikeout
		, DEFAULT_CHARSET
		, OUT_DEFAULT_PRECIS
		, CLIP_DEFAULT_PRECIS
		, _Desc.antialiased ? CLEARTYPE_QUALITY : NONANTIALIASED_QUALITY
		, DEFAULT_PITCH
		, _Desc.name);
	return hFont;
}

font_info get_font_info(HFONT _hFont)
{
	font_info fi;
	LOGFONT lf = {0};
	::GetObject(_hFont, sizeof(lf), &lf);
	fi.name = lf.lfFaceName;
	fi.bold = lf.lfWeight > FW_NORMAL;
	fi.italic = !!lf.lfItalic;
	fi.underline = !!lf.lfUnderline;
	fi.strikeout = !!lf.lfStrikeOut;
	fi.antialiased = lf.lfQuality != NONANTIALIASED_QUALITY;
	scoped_getdc hDC(GetDesktopWindow());
	fi.point_size = logical_height_to_pointf(hDC, lf.lfHeight);
	return fi;
}

const char* font_family(BYTE _tmPitchAndFamily)
{
	switch (_tmPitchAndFamily & 0xf0)
	{
		case FF_DECORATIVE: return "Decorative";
		case FF_DONTCARE: return "Don't care";
		case FF_MODERN: return "Modern";
		case FF_ROMAN: return "Roman";
		case FF_SCRIPT: return "Script";
		case FF_SWISS: return "Swiss";
		default: break;
	}

	return "?";
}

const char* char_set(BYTE _tmCharSet)
{
	switch (_tmCharSet)
	{
		case ANSI_CHARSET: return "ANSI";
		case BALTIC_CHARSET: return "Baltic";
		case CHINESEBIG5_CHARSET: return "Chinese Big5";
		case DEFAULT_CHARSET: return "Default";
		case EASTEUROPE_CHARSET: return "East Europe";
		case GB2312_CHARSET: return "GB2312";
		case GREEK_CHARSET: return "Greek";
		case HANGUL_CHARSET: return "Hangul";
		case MAC_CHARSET: return "Mac";
		case OEM_CHARSET: return "OEM";
		case RUSSIAN_CHARSET: return "Russian";
		case SHIFTJIS_CHARSET: return "Shift JIS";
		case SYMBOL_CHARSET: return "Symbol";
		case TURKISH_CHARSET: return "Turkish";
		case VIETNAMESE_CHARSET: return "Vietnamese";
		case JOHAB_CHARSET: return "Johab";
		case ARABIC_CHARSET: return "Arabic";
		case HEBREW_CHARSET: return "Hebrew";
		case THAI_CHARSET: return "Thai";
		default: break;
	}

	return "?";
}

		} // namespace gdi
	} // namespace windows
} // namespace ouro
