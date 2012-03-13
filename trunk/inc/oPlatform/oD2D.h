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

// Soft-link D2D1 and some utility functions to address common challenges in 
// D2D and DirectWrite.
#pragma once
#ifndef oD2D_h
#define oD2D_h

#include <oBasis/oMath.h>
#include <oPlatform/oWindows.h>
#include <oPlatform/oWindowUI.h>

#if oDXVER >= oDXVER_10

	D2D1::ColorF oD2DColor(oColor _Color);

	inline D2D1_POINT_2F oD2DAsPOINT(const float2& _P) { D2D1_POINT_2F p; p.x = _P.x; p.y = _P.y; return p; }
	inline D2D1_POINT_2F oD2DAsPOINT(const int2& _P) { return oD2DAsPOINT(oCastAsFloat(_P)); }
	inline float2 oD2DAsFloat2(const D2D1_POINT_2F& _Point2F) { return float2(_Point2F.x, _Point2F.y); }
	inline float2 oD2DAsFloat2(const D2D1_SIZE_F& _SizeF) { return float2(_SizeF.width, _SizeF.height); }
	inline D2D1::ColorF oD2DAsColorF(oColor _Color) { float r,g,b,a; oColorDecompose(_Color, &r, &g, &b, &a); return D2D1::ColorF(r,g,b,a); }
	inline D2D1_RECT_F oD2DAsRect(const RECT& _Rect) { D2D1_RECT_F r; r.left = static_cast<float>(_Rect.left); r.top = static_cast<float>(_Rect.top); r.right = static_cast<float>(_Rect.right); r.bottom = static_cast<float>(_Rect.bottom); return r; }

	// Creates a threadsafe factory
	bool oD2DCreateFactory(ID2D1Factory** _ppFactory);

	// A shared factory is created at DWrite link time (first call of this function)
	// that all clients should use.
	IDWriteFactory* oD2DGetSharedDWriteFactory();

	// Creates a render target with the specified parameters, using basic defaults
	// for all other D2D1 parameters.
	bool oD2DCreateRenderTarget(ID2D1Factory* _pD2DFactory, IDXGISwapChain* _pSwapChain, oSURFACE_FORMAT _Format, bool _UseAntialiasing, ID2D1RenderTarget** _ppRenderTarget);

	// Creates a bitmap that is compatible blending-wise with a render target created with
	// oD2DCreateRenderTarget.
	bool oD2DCreateBitmap(ID2D1RenderTarget* _pRenderTarget, const int2& _Size, oSURFACE_FORMAT _Format, ID2D1Bitmap** _ppBitmap);

	// Optimally draws a rounded rectangle given the specified RECT and brushes. This draws
	// both the fill and the border if the brush is not 100% translucent. If either brush
	// is null, then that aspect won't be drawn.
	bool oD2DDrawRoundedRect(ID2D1RenderTarget* _pRenderTarget, const D2D1_ROUNDED_RECT& _Rect, ID2D1Brush* _pFillBrush, ID2D1Brush* _pBorderBrush);

	// Handles conversion to DWrite types that is two separate enums
	bool oD2DSetAlignment(IDWriteTextFormat* _pDWTextFormat, oGUI_ALIGNMENT _Alignment);

#endif
#endif
