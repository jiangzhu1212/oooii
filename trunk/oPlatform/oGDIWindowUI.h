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
#ifndef oGDIWindowUI_h
#define oGDIWindowUI_h

#include <oBasis/oFixedString.h>
#include "oWindowUIElementBaseMixin.h"

struct oGDIWindowUILine : oWindowUILine, oWindowUIElementBaseMixin<oWindowUILine, oGDIWindowUILine>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUILine);
	oDECLARE_ONEVENT();
};

struct oGDIWindowUIBox : oWindowUIBox, oWindowUIElementBaseMixin<oWindowUIBox, oGDIWindowUIBox>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIBox);
	oDECLARE_ONEVENT();
};

struct oGDIWindowUIFont : oWindowUIFont, oWindowUIElementBaseMixin<oWindowUIFont, oGDIWindowUIFont>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIFont);
	oDECLARE_ONEVENT();
	inline HFONT GetFont() const threadsafe { return hFont; }
protected:
	oGDIScopedObject<HFONT> hFont;
};

struct oGDIWindowUIText : oWindowUIText, oWindowUIElementBaseMixin<oWindowUIText, oGDIWindowUIText>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIText);
	oDECLARE_ONEVENT();
	void GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe override;
	void SetFont(threadsafe oWindowUIFont* _pFont) threadsafe override;
	void SetText(const char* _Text) threadsafe override;

protected:
	oSharedMutex FontStringMutex;
	oRef<threadsafe oWindowUIFont> Font;
	oStringXL Text;
};

struct oGDIWindowUIPicture : oWindowUIPicture, oWindowUIElementBaseMixin<oWindowUIPicture, oGDIWindowUIPicture>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIPicture);
	oDECLARE_ONEVENT();
	void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally = false, bool _FlipVertically = false) threadsafe override;

protected:
	oSharedMutex BitmapMutex;
	oGDIScopedObject<HBITMAP> hBitmap;
	oGDIScopedObject<HBITMAP> hFlipBitmap;
};

#endif
