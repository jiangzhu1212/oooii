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
#include <oPlatform/oD2D.h>
#include <oBasis/oAssert.h>
#include <oBasis/oFixedString.h>
#include <oPlatform/oModule.h>
#include <oPlatform/oSingleton.h>
#include <oPlatform/oDXGI.h>
#include "SoftLink/oWinD2D.h"
#include "SoftLink/oWinDWrite.h"

#if oDXVER >= oDXVER_10

D2D1::ColorF oD2DColor(oColor _Color)
{
	float r,g,b,a;
	oColorDecompose(_Color, &r, &g, &b, &a);
	return D2D1::ColorF(r, g, b, a);
}

bool oD2DCreateFactory(ID2D1Factory** _ppFactory)
{
	D2D1_FACTORY_OPTIONS opt;
	opt.debugLevel = 
	#ifdef _DEBUG
		D2D1_DEBUG_LEVEL_WARNING;
	#else
		D2D1_DEBUG_LEVEL_NONE;
	#endif
	oVB_RETURN2(oWinD2D::Singleton()->D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory), &opt, (void**)_ppFactory));
	return true;
}

IDWriteFactory* oD2DGetSharedDWriteFactory()
{
	return oWinDWrite::Singleton()->GetDWriteFactory();
}

D2D1_PIXEL_FORMAT GetFormat(oSURFACE_FORMAT _Format)
{
	D2D1_PIXEL_FORMAT f;
	f.format = oDXGIFromSurfaceFormat(_Format);
	f.alphaMode = oSurfaceFormatIsAlpha(_Format) ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE;
	return f;
}

bool oD2DCreateRenderTarget(ID2D1Factory* _pD2DFactory, IDXGISwapChain* _pSwapChain, oSURFACE_FORMAT _Format, bool _UseAntialiasing, ID2D1RenderTarget** _ppRenderTarget)
{
	if (!_pD2DFactory || !_ppRenderTarget)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	oRef<ID3D10Texture2D> RT;
	oVB_RETURN2(_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&RT));

	oRef<IDXGISurface> RTSurface;
	oVB_RETURN2(RT->QueryInterface(&RTSurface));

	D2D1_RENDER_TARGET_PROPERTIES rtp;
	rtp.pixelFormat = GetFormat(_Format);
	rtp.dpiX = 0.0f;
	rtp.dpiY = 0.0f;
	rtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	rtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	rtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;

	oVB_RETURN2(_pD2DFactory->CreateDxgiSurfaceRenderTarget(RTSurface, &rtp, _ppRenderTarget));
	(*_ppRenderTarget)->SetAntialiasMode(_UseAntialiasing ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
	return true;
}

bool oD2DCreateBitmap(ID2D1RenderTarget* _pRenderTarget, const int2& _Size, oSURFACE_FORMAT _Format, ID2D1Bitmap** _ppBitmap)
{
	if (!_pRenderTarget || !_ppBitmap || less_than_equal(_Size, int2(0,0)))
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	D2D1_SIZE_U size; size.width = _Size.x; size.height = _Size.y;
	D2D1_BITMAP_PROPERTIES properties;
	properties.pixelFormat = GetFormat(_Format);
	oRef<ID2D1Factory> D2DFactory;
	_pRenderTarget->GetFactory(&D2DFactory);
	D2DFactory->GetDesktopDpi(&properties.dpiX, &properties.dpiY);
	static const HRESULT oWINCODEC_ERR_UNSUPPORTEDPIXELFORMAT = 0x88982f80; // @oooii-tony: I'm not sure what header this is in...
	HRESULT hr = _pRenderTarget->CreateBitmap(size, 0, 0, &properties, _ppBitmap);
	if (hr == oWINCODEC_ERR_UNSUPPORTEDPIXELFORMAT)
	{
		oStringL err;
		sprintf_s(err, "%s%s is incompatible with the specified render target", oAsString(_Format), properties.pixelFormat.alphaMode == D2D1_ALPHA_MODE_PREMULTIPLIED ? " with pre-multipled alpha" : "");
		return oWinSetLastError(hr, err);
	}

	else if (hr)
		return oWinSetLastError(hr);

	return true;
}

bool oD2DDrawRoundedRect(ID2D1RenderTarget* _pRenderTarget, const D2D1_ROUNDED_RECT& _Rect, ID2D1Brush* _pFillBrush, ID2D1Brush* _pBorderBrush)
{
	if (!_pRenderTarget)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	if (oEqual(_Rect.radiusX, 0.0f) && oEqual(_Rect.radiusY, 0.0f))
	{
		if (_pFillBrush && !oEqual(_pFillBrush->GetOpacity(), 0.0f))
			_pRenderTarget->FillRectangle(&_Rect.rect, _pFillBrush);
		if (_pBorderBrush && !oEqual(_pBorderBrush->GetOpacity(), 0.0f))
			_pRenderTarget->DrawRectangle(&_Rect.rect, _pBorderBrush);
	}

	else
	{
		if (_pFillBrush && !oEqual(_pFillBrush->GetOpacity(), 0.0f))
			_pRenderTarget->FillRoundedRectangle(&_Rect, _pFillBrush);
		if (_pBorderBrush && !oEqual(_pBorderBrush->GetOpacity(), 0.0f))
			_pRenderTarget->DrawRoundedRectangle(&_Rect, _pBorderBrush);
	}

	return true;
}

bool oD2DSetAlignment(IDWriteTextFormat* _pDWTextFormat, oGUI_ALIGNMENT _Alignment)
{
	if (!_pDWTextFormat)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);

	DWRITE_TEXT_ALIGNMENT TAlign;
	switch (_Alignment % 3)
	{
		case 0: TAlign = DWRITE_TEXT_ALIGNMENT_LEADING; break;
		case 1: TAlign = DWRITE_TEXT_ALIGNMENT_CENTER; break;
		case 2: TAlign = DWRITE_TEXT_ALIGNMENT_TRAILING; break;
		oNODEFAULT;
	}

	DWRITE_PARAGRAPH_ALIGNMENT PAlign;
	switch (_Alignment / 3)
	{
		case 0: PAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR; break;
		case 1: PAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER; break;
		case 2: PAlign = DWRITE_PARAGRAPH_ALIGNMENT_FAR; break;
		oNODEFAULT;
	}

	_pDWTextFormat->SetTextAlignment(TAlign);
	_pDWTextFormat->SetParagraphAlignment(PAlign);
	return true;
}

#endif // oDXVER >= oDXVER_10
