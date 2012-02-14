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
#include "oGDIWindowUI.h"
#include <oPlatform/oWinRect.h>

oGDIWindowUILine::oGDIWindowUILine(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oGDIWindowUILine);
	*_pSuccess = true;
}

oGDIWindowUILine::~oGDIWindowUILine()
{
	Window->Unhook(HookID);
}

bool oGDIWindowUILine::OnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale)
{
	switch (_Event)
	{
		case oWindow::DRAW_UIAA:
		{
			const DESC& d = MIXINGetDesc();
			if (!oIsTransparentColor(d.Color))
			{
				oGDIScopedObject<HPEN> hPen = oGDICreatePen(d.Color, d.Thickness * _SuperSampleScale);
				HDC hDC = nullptr;
				oVERIFY(Window->QueryInterface(oGetGUID<oHDCAA>(), &hDC));
				oGDIScopedSelect pen(hDC, hPen);
				POINT pt[2];
				pt[0] = oAsPOINT((int)_SuperSampleScale * d.P1);
				pt[1] = oAsPOINT((int)_SuperSampleScale * d.P2);
				Polyline(hDC, pt, 2);
			}
			break;
		}

		default:
			break;
	}

	return true;
}

oGDIWindowUIBox::oGDIWindowUIBox(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oGDIWindowUIBox);
	*_pSuccess = true;
}

oGDIWindowUIBox::~oGDIWindowUIBox()
{
	oUNHOOK_ONEVENT();
}

bool oGDIWindowUIBox::OnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale)
{
	switch (_Event)
	{
		case oWindow::DRAW_UIAA:
		{
			const DESC& d = MIXINGetDesc();
			oGDIScopedObject<HPEN> hPen = oGDICreatePen(d.BorderColor, _SuperSampleScale);
			oGDIScopedObject<HBRUSH> hBrush = oGDICreateBrush(d.Color);
			HDC hDC = nullptr;
			oVERIFY(Window->QueryInterface(oGetGUID<oHDCAA>(), &hDC));
			oGDIScopedSelect SelectPen(hDC, hPen);
			oGDIScopedSelect SelectBrush(hDC, hBrush);
			RECT rAdjusted = oWinRectScale(oWinRectResolve(Window, d), _SuperSampleScale);
			int radius = static_cast<int>(_SuperSampleScale * d.Roundness * 2.5f);
			RoundRect(hDC, rAdjusted.left, rAdjusted.top, rAdjusted.right, rAdjusted.bottom, radius, radius);
			break;
		}
		default:
			break;
	}

	return true;
}

oGDIWindowUIFont::oGDIWindowUIFont(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oGDIScopedGetDC hDC((HWND)Window->GetNativeHandle());
	hFont = oGDICreateFont(
		_Desc.FontName
		, static_cast<int>(_Desc.PointSize)
		, _Desc.Style == BOLD || _Desc.Style == BOLDITALIC
		, _Desc.Style == ITALIC || _Desc.Style == BOLDITALIC
		, false);

	#ifdef _DEBUG
		// Use this to get the actual font used
		oGDIScopedSelect font(hDC, hFont);
		TEXTMETRIC tm;
		oVB(GetTextMetrics(hDC, &tm));
		// Here's what's requested of a device context
		LOGFONT lf;
		GetObject(hFont, sizeof(LOGFONT), &lf);
		oTRACE("oGDIWindowUIFont Created: %s %s%s %s %s", lf.lfFaceName, tm.tmWeight == FW_NORMAL ? "" : "bold", tm.tmItalic ? "italic" : "", oGDIGetCharSet(tm.tmCharSet), oGDIGetFontFamily(tm.tmPitchAndFamily));
	#endif
	*_pSuccess = true;
}

oGDIWindowUIFont::~oGDIWindowUIFont()
{
}

oGDIWindowUIText::oGDIWindowUIText(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oGDIWindowUIText);
	*_pSuccess = true;
}

oGDIWindowUIText::~oGDIWindowUIText()
{
	Window->Unhook(HookID);
}

void oGDIWindowUIText::GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe
{
	oSharedLock<oSharedMutex> lock(FontStringMutex);
	Font->Reference();
	*_ppFont = Font;
}

void oGDIWindowUIText::SetFont(threadsafe oWindowUIFont* _pFont) threadsafe
{
	oLockGuard<oSharedMutex> lock(FontStringMutex);
	Font = _pFont;
	oRECT r = oRect(oWinRectResolve(Window, MIXINGetDesc()));
	Window->Refresh(false, &r);
}

void oGDIWindowUIText::SetText(const char* _Text) threadsafe
{
	oGDIWindowUIText* pThis = thread_cast<oGDIWindowUIText*>(this);
	pThis->Text = _Text;
	oAddTruncationElipse(pThis->Text.c_str(), pThis->Text.capacity());
	oRECT r = oRect(oWinRectResolve(Window, MIXINGetDesc()));
	Window->Refresh(false, &r);
}

bool oGDIWindowUIText::OnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale)
{
	switch (_Event)
	{
		case oWindow::DRAW_UI:
		{
			const DESC& d = MIXINGetDesc();
			HDC hDC = nullptr;
			oVERIFY(Window->QueryInterface(oGetGUID<oHDC>(), &hDC));
			RECT rAdjusted = oWinRectResolve(Window, d);
			
			oLockGuard<oSharedMutex> lock(FontStringMutex);
			if (Font && *Text && !oIsTransparentColor(d.Color))
			{
				oGDIScopedSelect font(hDC, static_cast<threadsafe oGDIWindowUIFont*>(Font.c_ptr())->GetFont());
		
				if (!oIsTransparentColor(d.ShadowColor))
				{
					oWindowUIFont::DESC fdesc;
					Font->GetDesc(&fdesc);
					int offset = static_cast<int>(round(fdesc.ShadowOffset));
					if (offset)
					{
						RECT rShadow = oWinRectTranslate(rAdjusted, int2(offset, offset));
						oVB(oGDIDrawText(hDC, rShadow, d.Alignment, d.ShadowColor, 0, !d.MultiLine, Text));
					}
				}

				oVB(oGDIDrawText(hDC, rAdjusted, d.Alignment, d.Color, 0, !d.MultiLine, Text));
			}
			break;
		}
		default:
			break;
	}

	return true;
}

oGDIWindowUIPicture::oGDIWindowUIPicture(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oGDIScopedGetDC hDC((HWND)Window->GetNativeHandle());
	hBitmap = CreateCompatibleBitmap(hDC, _Desc.ImageDesc.Dimensions.x, _Desc.ImageDesc.Dimensions.y);
	// Anticipating continuous capture, that is usually flipped compared
	// to what Windows expects, allocate an extra buffer. The performance
	// benefits are worth the memory hit. If not, promote this action to
	// a flag/hint on the DESC.
	hFlipBitmap = CreateCompatibleBitmap(hDC, _Desc.ImageDesc.Dimensions.x, _Desc.ImageDesc.Dimensions.y);
	oHOOK_ONEVENT(oGDIWindowUIPicture);
	*_pSuccess = true;
}

oGDIWindowUIPicture::~oGDIWindowUIPicture()
{
	Window->Unhook(HookID);
}

void oGDIWindowUIPicture::Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally, bool _FlipVertically) threadsafe
{
	HWND hWnd = (HWND)Window->GetNativeHandle();
	// Always allocate enough memory for 8-bit formats and avoid a heap 
	// alloc below
	const DESC& d = MIXINGetDesc();
	BITMAPINFO* pBMI = (BITMAPINFO*)_alloca(oGetBMISize(d.ImageDesc.Format));
	oAllocateBMI(&pBMI, thread_cast<oImage::DESC&>(d.ImageDesc), 0, !_FlipVertically); // Windows is already flipping it, so if you want to flip it again, undo that
	oGDIScopedGetDC hdc(hWnd);
	HDC htemp = CreateCompatibleDC(hdc);

	int w = _FlipHorizontally ? -(int)d.ImageDesc.Dimensions.x : d.ImageDesc.Dimensions.x;
	int h = _FlipVertically ? d.ImageDesc.Dimensions.y : -(int)d.ImageDesc.Dimensions.y;

	oLockGuard<oSharedMutex> lock(BitmapMutex);
	if (!_FlipHorizontally && !_FlipVertically)
		SetDIBits(htemp, hBitmap, 0, d.ImageDesc.Dimensions.y, _pSourceData, pBMI, DIB_RGB_COLORS);
	else
	{
		SetDIBits(htemp, hFlipBitmap, 0, d.ImageDesc.Dimensions.y, _pSourceData, pBMI, DIB_RGB_COLORS);
		SelectObject(htemp, hBitmap);
		oGDIStretchBitmap(htemp, w < 0 ? d.ImageDesc.Dimensions.x : 0, h < 0 ? d.ImageDesc.Dimensions.y : 0, w, h, hFlipBitmap, SRCCOPY);
	}

	DeleteDC(htemp);
	oRECT r = oRect(oWinRectResolve(Window, d));
	Window->Refresh(false, &r);
}

bool oGDIWindowUIPicture::OnEvent(oWindow::EVENT _Event, const float3& _Position, int _SuperSampleScale)
{
	switch (_Event)
	{
		case oWindow::DRAW_UI:
		{
			HDC hDC = nullptr;
			oVERIFY(Window->QueryInterface(oGetGUID<oHDC>(), &hDC));
			RECT rAdjusted = oWinRectResolve(Window, MIXINGetDesc());
			oLockGuard<oSharedMutex> lock(BitmapMutex);
			oVB(oGDIStretchBlendBitmap(hDC, rAdjusted.left, rAdjusted.top, oWinRectW(rAdjusted), oWinRectH(rAdjusted), hBitmap));
			//oVB(oGDIStretchBitmap(hDC, rAdjusted.left, rAdjusted.top, oWinRectW(rAdjusted), oWinRectH(rAdjusted), hBitmap, SRCCOPY));
			break;
		}
		default:
			break;
	}

	return true;
}