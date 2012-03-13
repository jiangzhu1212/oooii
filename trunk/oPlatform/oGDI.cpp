/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oGDI.h>
#include <oBasis/oByte.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinWindowing.h>
#include "SoftLink/oWinMSIMG32.h"

int oGDIPointToLogicalHeight(HDC _hDC, int _Point)
{
	return -MulDiv(_Point, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
}

int oGDILogicalHeightToPoint(HDC _hDC, int _Height)
{
	return MulDiv(_Height, 72, GetDeviceCaps(_hDC, LOGPIXELSY));
}

float oGDILogicalHeightToPointF(HDC _hDC, int _Height)
{
	return (_Height * 72.0f) / (float)GetDeviceCaps(_hDC, LOGPIXELSY);
}

int oGDIPointToLogicalHeight(HDC _hDC, float _Point)
{
	return oGDIPointToLogicalHeight(_hDC, static_cast<int>(_Point + 0.5f));
}

bool oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow)
{
	RECT r;
	if (_pRect)
	{
		// Find offset into client area
		POINT p = {0,0};
		ClientToScreen(_hWnd, &p);

		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		p.x -= wr.left;
		p.y -= wr.top;

		r.left = _pRect->left + p.x;
		r.right = _pRect->right + p.x;
		r.top = _pRect->top + p.y;
		r.bottom = _pRect->bottom + p.y;
	}

	else
	{
		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		r.left = 0;
		r.top = 0;
		r.right = oWinRectW(wr);
		r.bottom = oWinRectH(wr);
	}

	int2 size = oWinRectSize(r);

	if (size.x == 0 || size.y == 0)
		return false;

	WORD bitdepth = 0;
	{
		oDISPLAY_DESC DDesc;
		oVERIFY(oDisplayEnum(oWinGetDisplayIndex(_hWnd), &DDesc));
		bitdepth = static_cast<WORD>(DDesc.Mode.Bitdepth);
		if (bitdepth == 32) bitdepth = 24;
	}

	if (!_pBitmapInfo)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	memset(_pBitmapInfo, 0, sizeof(BITMAPINFO));
	_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_pBitmapInfo->bmiHeader.biWidth = size.x;
	_pBitmapInfo->bmiHeader.biHeight = size.y;
	_pBitmapInfo->bmiHeader.biPlanes = 1;
	_pBitmapInfo->bmiHeader.biBitCount = bitdepth;
	_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	_pBitmapInfo->bmiHeader.biSizeImage = oByteAlign(size.x, 4) * size.y * bitdepth / 8;

	if (_pImageBuffer)
	{
		if (_SizeofImageBuffer < _pBitmapInfo->bmiHeader.biSizeImage)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Destination buffer too small");
			return false;
		}

		HDC hDC = GetWindowDC(_hWnd);
		HDC hMemDC = CreateCompatibleDC(hDC);

		HBITMAP hBMP = CreateCompatibleBitmap(hDC, size.x, size.y);
		HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBMP);
		if (_RedrawWindow)
			oVB(RedrawWindow(_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW));

		BitBlt(hMemDC, 0, 0, size.x, size.y, hDC, r.left, r.top, SRCCOPY);
		GetDIBits(hMemDC, hBMP, 0, size.y, _pImageBuffer, _pBitmapInfo, DIB_RGB_COLORS);

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		ReleaseDC(0, hDC);
	}

	return true;
}

bool oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, oFUNCTION<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow)
{
	if (!_Allocate || !_ppBuffer || !_pBufferSize)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}
	
	*_ppBuffer = nullptr;
	*_pBufferSize = 0;

	// Create a bmp in memory
	RECT r;
	GetClientRect(_hWnd, &r);
	BITMAPINFO bmi;

	RECT* pRect = &r;
	if (_IncludeBorder)
		pRect = nullptr;

	if (oGDIScreenCaptureWindow(_hWnd, pRect, nullptr, 0, &bmi, _RedrawWindow))
	{
		BITMAPFILEHEADER bmfh;
		memset(&bmfh, 0, sizeof(bmfh));
		bmfh.bfType = 'MB';
		bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
		bmfh.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO));

		*_pBufferSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
		*_ppBuffer = _Allocate(*_pBufferSize);
		memcpy(*_ppBuffer, &bmfh, sizeof(bmfh));
		memcpy(oByteAdd(*_ppBuffer, sizeof(bmfh)), &bmi, sizeof(bmi));
		return oGDIScreenCaptureWindow(_hWnd, pRect, oByteAdd(*_ppBuffer, sizeof(bmfh) + sizeof(bmi)), bmi.bmiHeader.biSizeImage, &bmi, _RedrawWindow);
	}

	return false;
}

BOOL oGDIDrawBitmap(HDC _hDC, INT _X, INT _Y, HBITMAP _hBitmap, DWORD _dwROP)
{
	HDC hDCBitmap = 0;
	BITMAP Bitmap;
	BOOL bResult = false;

	if (_hDC && _hBitmap)
	{
		hDCBitmap = CreateCompatibleDC(_hDC);
		GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		HGDIOBJ hOld = SelectObject(hDCBitmap, _hBitmap);
		bResult = BitBlt(_hDC, _X, _Y, Bitmap.bmWidth, Bitmap.bmHeight, hDCBitmap, 0, 0, _dwROP);
		SelectObject(hDCBitmap, hOld);
		DeleteDC(hDCBitmap);
	}

	return bResult;
}

BOOL oGDIStretchBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap, DWORD _dwROP)
{
	HDC hDCBitmap = 0;
	BITMAP Bitmap;
	BOOL bResult = false;

	if (_hDC && _hBitmap)
	{
		hDCBitmap = CreateCompatibleDC(_hDC);
		GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		HGDIOBJ hOld = SelectObject(hDCBitmap, _hBitmap);
		bResult = StretchBlt(_hDC, _X, _Y, _Width, _Height, hDCBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, _dwROP);
		SelectObject(hDCBitmap, hOld);
		DeleteDC(hDCBitmap);
	}

	return bResult;
}

BOOL oGDIStretchBlendBitmap(HDC _hDC, INT _X, INT _Y, INT _Width, INT _Height, HBITMAP _hBitmap)
{
	static const BLENDFUNCTION kBlend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	HDC hDCBitmap = 0;
	BITMAP Bitmap;
	BOOL bResult = false;

	if (_hDC && _hBitmap)
	{
		hDCBitmap = CreateCompatibleDC(_hDC);
		GetObject(_hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		HGDIOBJ hOld = SelectObject(hDCBitmap, _hBitmap);
		bResult = oWinMSIMG32::Singleton()->AlphaBlend(_hDC, _X, _Y, _Width, _Height, hDCBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, kBlend);
		SelectObject(hDCBitmap, hOld);
		DeleteDC(hDCBitmap);
	}

	return bResult;
}

bool oGDIDrawBox(HDC _hDC, const RECT& _rBox, int _EdgeRoundness)
{
	if (_EdgeRoundness)
		RoundRect(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom, _EdgeRoundness, _EdgeRoundness);
	else if (!Rectangle(_hDC, _rBox.left, _rBox.top, _rBox.right, _rBox.bottom))
		return oWinSetLastError();
	return true;
}

static bool oGDIDrawText(HDC _hDC, const RECT& _rTextBox, oGUI_ALIGNMENT _Alignment, oColor _Foreground, oColor _Background, bool _SingleLine, const char* _Text, RECT* _pActual)
{
	int r,g,b,a;
	oColorDecompose(_Foreground, &r, &g, &b, &a);

	int br,bg,bb,ba;
	oColorDecompose(_Background, &br, &bg, &bb, &ba);

	if (!a)
	{
		if (!ba) return false;
		r = br;
		g = bg;
		b = bb;
	}

	UINT uFormat = DT_WORDBREAK;
	switch (_Alignment % 3)
	{
		case 0: uFormat |= DT_LEFT; break;
		case 1: uFormat |= DT_CENTER; break;
		case 2: uFormat |= DT_RIGHT; break;
		default: oASSERT_NOEXECUTION;
	}

	switch (_Alignment / 3)
	{
		case 0: uFormat |= DT_TOP; break;
		case 1: uFormat |= DT_VCENTER; break;
		case 2: uFormat |= DT_BOTTOM; break;
		default: oASSERT_NOEXECUTION;
	}

	bool forcedSingleLine = !_SingleLine && ((uFormat & DT_BOTTOM) || (uFormat & DT_VCENTER));
	if (forcedSingleLine || _SingleLine)
	{
		if (forcedSingleLine)
			oTRACE_ONCE("GDI doesn't support multi-line, vertically aligned text. See DrawText docs for more details. http://msdn.microsoft.com/en-us/library/ms901121.aspx");
		uFormat &=~ DT_WORDBREAK;
		uFormat |= DT_SINGLELINE;
	}

	if (_pActual)
		uFormat |= DT_CALCRECT;

	int oldBkMode = 0;
	COLORREF oldBkColor = 0;

	if (ba)
		oldBkColor = SetBkColor(_hDC, RGB(br,bg,bb));
	else
		oldBkMode = SetBkMode(_hDC, TRANSPARENT);
	
	COLORREF oldColor = SetTextColor(_hDC, RGB(r,g,b));
	RECT rect = _rTextBox;
	if (!_pActual)
		_pActual = &rect;
	else
	{
		_pActual->top = 0;
		_pActual->left = 0;
		_pActual->right = 1;
		_pActual->bottom = 1;
	}

	DrawText(_hDC, _Text, -1, _pActual, uFormat);
	SetTextColor(_hDC, oldColor);

	if (ba)
		SetBkColor(_hDC, oldBkColor);
	else
		SetBkMode(_hDC, oldBkMode);

	return true;
}

bool oGDICalcTextBox(HDC _hDC, RECT* _prTextBox, oGUI_ALIGNMENT _Alignment, oColor _Foreground, oColor _Background, bool _SingleLine, const char* _Text)
{
	return oGDIDrawText(_hDC, *_prTextBox, _Alignment, _Foreground, _Background, _SingleLine, _Text, _prTextBox);
}

bool oGDIDrawText(HDC _hDC, const RECT& _rTextBox, oGUI_ALIGNMENT _Alignment, oColor _Foreground, oColor _Background, bool _SingleLine, const char* _Text)
{
	return oGDIDrawText(_hDC, _rTextBox, _Alignment, _Foreground, _Background, _SingleLine, _Text, nullptr);
}

HPEN oGDICreatePen(oColor _Color, int _Width)
{
	int r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	return CreatePen(a ? PS_SOLID : PS_NULL, _Width, RGB(r,g,b));
}

HBRUSH oGDICreateBrush(oColor _Color)
{
	int r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	return a ? CreateSolidBrush(RGB(r,g,b)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

COLORREF oGDIGetBrushColor(HBRUSH _hBrush)
{
	LOGBRUSH lb;
	GetObject(_hBrush, sizeof(LOGBRUSH), &lb);
	return lb.lbColor;
}

int2 oGDIGetIconSize(HICON _hIcon)
{
	ICONINFO ii;
	BITMAP b;
	if (GetIconInfo(_hIcon, &ii))
	{
		if (ii.hbmColor)
		{
			if (GetObject(ii.hbmColor, sizeof(b), &b))
				return int2(b.bmWidth, b.bmHeight);
		}

		else
		{
			if (GetObject(ii.hbmMask, sizeof(b), &b))
				return int2(b.bmWidth, b.bmHeight);
		}
	}

	return int2(-1,-1);
}

HFONT oGDICreateFont(const oGUI_FONT_DESC& _Desc)
{
	oGDIScopedGetDC hDC(GetDesktopWindow());
	HFONT hFont = CreateFont(
		oGDIPointToLogicalHeight(hDC, _Desc.PointSize)
		, 0
		, 0
		, 0
		, _Desc.Bold ? FW_BOLD : FW_NORMAL
		, _Desc.Italic
		, _Desc.Underline
		, _Desc.StrikeOut
		, DEFAULT_CHARSET
		, OUT_DEFAULT_PRECIS
		, CLIP_DEFAULT_PRECIS
		, CLEARTYPE_QUALITY
		, DEFAULT_PITCH
		, _Desc.FontName);
	return hFont;
}

void oGDIGetFontDesc(HFONT _hFont, oGUI_FONT_DESC* _pDesc)
{
	LOGFONT lf = {0};
	::GetObject(_hFont, sizeof(lf), &lf);
	_pDesc->FontName = lf.lfFaceName;
	_pDesc->Bold = lf.lfWeight > FW_NORMAL;
	_pDesc->Italic = !!lf.lfItalic;
	_pDesc->Underline = !!lf.lfUnderline;
	_pDesc->StrikeOut = !!lf.lfStrikeOut;
	oGDIScopedGetDC hDC(GetDesktopWindow());
	_pDesc->PointSize = oGDILogicalHeightToPointF(hDC, lf.lfHeight);
}

const char* oGDIGetFontFamily(BYTE _tmPitchAndFamily)
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

	return "Unknown";
}

const char* oGDIGetCharSet(BYTE _tmCharSet)
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

	return "Unknown";
}

int oGDIEstimatePointSize(int _PixelHeight)
{
	// http://reeddesign.co.uk/test/points-pixels.html
	switch (_PixelHeight)
	{
		case 8: return 6;
		case 9: return 7;
		case 11: return 8;
		case 12: return 9;
		case 13: return 10;
		case 15: return 11;
		case 16: return 12;
		case 17: return 13;
		case 19: return 14;
		case 21: return 15;
		case 22: return 16;
		case 23: return 17;
		case 24: return 18;
		case 26: return 20;
		case 29: return 22;
		case 32: return 24;
		case 35: return 26;
		case 36: return 27;
		case 37: return 28;
		case 38: return 29;
		case 40: return 30;
		case 42: return 32;
		case 45: return 34;
		case 48: return 36;
		default: break;
	}

	if (_PixelHeight < 8)
		return 4;
	
	return static_cast<int>((_PixelHeight * 36.0f / 48.0f) + 0.5f);
}
