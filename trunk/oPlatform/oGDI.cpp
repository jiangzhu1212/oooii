// $(header)
#include <oPlatform/oGDI.h>
#include <oBasis/oByte.h>
#include <oPlatform/oDisplay.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinWindowing.h>

static const char* oWinMSIMG32_exports[] = 
{
	"AlphaBlend",
};

struct oWinMSIMG32 : oModuleSingleton<oWinMSIMG32>
{
	oWinMSIMG32() { hModule = oModuleLink("msimg32.dll", oWinMSIMG32_exports, (void**)&AlphaBlend); oASSERT(hModule, ""); }
	~oWinMSIMG32() { oModuleUnlink(hModule); }

public:
	BOOL (__stdcall *AlphaBlend)(HDC hdcDest, int xoriginDest, int yoriginDest, int wDest, int hDest, HDC hdcSrc, int xoriginSrc, int yoriginSrc, int wSrc, int hSrc, BLENDFUNCTION ftn);

protected:
	oHMODULE hModule;
};

int oPointToLogicalHeight(HDC _hDC, int _Point)
{
	return -MulDiv(_Point, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
}

int oPointToLogicalHeight(HDC _hDC, float _Point)
{
	return oPointToLogicalHeight(_hDC, static_cast<int>(_Point));
}

bool oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo)
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

		oVB(RedrawWindow(_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW));
		BitBlt(hMemDC, 0, 0, size.x, size.y, hDC, r.left, r.top, SRCCOPY);
		GetDIBits(hMemDC, hBMP, 0, size.y, _pImageBuffer, _pBitmapInfo, DIB_RGB_COLORS);

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		ReleaseDC(0, hDC);
	}

	return true;
}

bool oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, oFUNCTION<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize)
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

	if (oGDIScreenCaptureWindow(_hWnd, pRect, nullptr, 0, &bmi))
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
		return oGDIScreenCaptureWindow(_hWnd, pRect, oByteAdd(*_ppBuffer, sizeof(bmfh) + sizeof(bmi)), bmi.bmiHeader.biSizeImage, &bmi);
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

bool oGDIDrawText(HDC _hDC, const RECT& _rTextBox, oANCHOR _Alignment, oColor _Foreground, oColor _Background, bool _SingleLine, const char* _Text)
{
	unsigned int r,g,b,a;
	oColorDecompose(_Foreground, &r, &g, &b, &a);

	unsigned int br,bg,bb,ba;
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

	int oldBkMode = 0;
	COLORREF oldBkColor = 0;

	if (ba)
		oldBkColor = SetBkColor(_hDC, RGB(br,bg,bb));
	else
		oldBkMode = SetBkMode(_hDC, TRANSPARENT);
	
	COLORREF oldColor = SetTextColor(_hDC, RGB(r,g,b));
	RECT rect = _rTextBox;
	DrawText(_hDC, _Text, -1, &rect, uFormat);
	SetTextColor(_hDC, oldColor);

	if (ba)
		SetBkColor(_hDC, oldBkColor);
	else
		SetBkMode(_hDC, oldBkMode);

	return true;
}

HPEN oGDICreatePen(oColor _Color, int _Width)
{
	unsigned int r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	return CreatePen(a ? PS_SOLID : PS_NULL, _Width, RGB(r,g,b));
}

HBRUSH oGDICreateBrush(oColor _Color)
{
	unsigned int r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	return a ? CreateSolidBrush(RGB(r,g,b)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
}

HFONT oGDICreateFont(const char* _FontName, int _PointSize, bool _Bold, bool _Italics, bool _Underline)
{
		HDC hDC = GetDC(GetDesktopWindow());
		HFONT hFont = CreateFont(
			oPointToLogicalHeight(hDC, _PointSize)
			, 0
			, 0
			, 0
			, _Bold ? FW_BOLD : FW_NORMAL
			, _Italics
			, _Underline
			, FALSE
			, DEFAULT_CHARSET
			, OUT_DEFAULT_PRECIS
			, CLIP_DEFAULT_PRECIS
			, CLEARTYPE_QUALITY
			, DEFAULT_PITCH
			, _FontName);

	ReleaseDC(GetDesktopWindow(), hDC);
	return hFont;
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
