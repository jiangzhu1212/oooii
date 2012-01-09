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
#pragma once
#ifndef oD2DWindowUI_h
#define oD2DWindowUI_h

#include "oWindowUIElementBaseMixin.h"

struct oD2DWindowUILine : oWindowUILine, oWindowUIElementBaseMixin<oWindowUILine, oD2DWindowUILine>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oD2DWindowUILine);
	oDECLARE_ONEVENT();

protected:
	oRef<ID2D1SolidColorBrush> Brush;
};

struct oD2DWindowUIBox : oWindowUIBox, oWindowUIElementBaseMixin<oWindowUIBox, oD2DWindowUIBox>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oD2DWindowUIBox);
	oDECLARE_ONEVENT();

protected:
	oRef<ID2D1SolidColorBrush> Brush;
	oRef<ID2D1SolidColorBrush> BorderBrush;
};

struct oD2DWindowUIFont : oWindowUIFont, oWindowUIElementBaseMixin<oWindowUIFont, oD2DWindowUIFont>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oD2DWindowUIFont);
	oDECLARE_ONEVENT();
	inline IDWriteTextFormat* GetFormat() threadsafe { return thread_cast<IDWriteTextFormat*>(Format.c_ptr()); }

protected:
	oRef<IDWriteTextFormat> Format;
};

struct oD2DWindowUIText : oWindowUIText, oWindowUIElementBaseMixin<oWindowUIText, oD2DWindowUIText>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oD2DWindowUIText);
	oDECLARE_ONEVENT();
	void GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe override;
	void SetFont(threadsafe oWindowUIFont* _pFont) threadsafe override;
	void SetText(const char* _Text) threadsafe override;

protected:
	oSharedMutex FontTextMutex;
	oRef<threadsafe oWindowUIFont> Font;
	oRef<ID2D1SolidColorBrush> Brush;
	oRef<ID2D1SolidColorBrush> ShadowBrush;
	oWStringL Text;
};

struct oD2DWindowUIPicture : oWindowUIPicture, oWindowUIElementBaseMixin<oWindowUIPicture, oD2DWindowUIPicture>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oD2DWindowUIPicture);
	oDECLARE_ONEVENT();
	void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally = false, bool _FlipVertically = false) threadsafe override;

protected:
	void ApplyCPUBitmap() threadsafe;
	void CalculateTransform(const D2D1_RECT_F& _Rect, D2D1::Matrix3x2F* _pMatrix);

	oRef<ID2D1Bitmap> Bitmap;
	void* CPUBitmap; // required for when D2DRenderTarget is recreated (DX9-style, that's kinda lame!)
	unsigned int CPUBitmapRowPitch;
	oSharedMutex BitmapMutex;
	bool BitmapDirty;
	bool FlippedHorizontally;
	bool FlippedVertically;
};

#endif
