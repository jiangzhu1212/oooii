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
// Primitive GUI elements useful when doing simple applications with oWindow.
#pragma once
#ifndef oWindowUI_h
#define oWindowUI_h

#include <oBasis/oColor.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInterface.h>
#include <oBasis/oMath.h>
#include <oPlatform/oImage.h>

enum oANCHOR
{
	oTOPLEFT,
	oTOPCENTER,
	oTOPRIGHT,
	oMIDDLELEFT,
	oMIDDLECENTER,
	oMIDDLERIGHT,
	oBOTTOMLEFT,
	oBOTTOMCENTER,
	oBOTTOMRIGHT,
	oFITPARENT,
};

oAPI const char* oAsString(const oANCHOR& _Anchor);

interface oWindow;
interface oWindowUIElement : oInterface
{
	virtual void GetWindow(threadsafe oWindow** _ppWindow) threadsafe = 0;
};

interface oWindowUILine : oWindowUIElement
{
	struct DESC
	{
		DESC()
			: P1(0, 0)
			, P2(0, 0)
			, Thickness(1)
			, Color(std::White)
		{}

		int2 P1;
		int2 P2;
		int Thickness;
		oColor Color;
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;
	virtual DESC* Map() threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
};

interface oWindowUIBox : oWindowUIElement
{
	struct DESC
	{
		DESC()
			: Position(oDEFAULT, oDEFAULT)
			, Size(oDEFAULT, oDEFAULT)
			, Anchor(oMIDDLECENTER)
			, Color(std::White)
			, BorderColor(std::Black)
			, Roundness(10.0f)
		{}
		
		int2 Position; // Relative to the anchor
		int2 Size; // oDEFAULT will use parent's Size
		oANCHOR Anchor; // Relative to parent window's client area
		oColor Color;
		oColor BorderColor;
		float Roundness;
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;
	virtual DESC* Map() threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
};

interface oWindowUIFont : oWindowUIElement
{
	enum STYLE
	{
		NORMAL,
		BOLD,
		ITALIC,
		BOLDITALIC,
	};

	struct DESC
	{
		DESC()
			: FontName("Tahoma")
			, Style(NORMAL)
			, PointSize(10.0f)
			, ShadowOffset(1.0f)
		{}

		oStringS FontName;
		STYLE Style;
		float PointSize; // Like in MS Word and such.
		float ShadowOffset; // In points. Bigger fonts will require bigger offsets.
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;
};

interface oWindowUIText : oWindowUIElement
{
	struct DESC
	{
		DESC()
			: Position(oDEFAULT, oDEFAULT)
			, Size(oDEFAULT, oDEFAULT)
			, Anchor(oMIDDLECENTER)
			, Alignment(oMIDDLECENTER)
			, Color(std::White)
			, ShadowColor(std::Black)
			, MultiLine(false)
		{}

		int2 Position; // Relative to the anchor
		int2 Size; // oDEFAULT will use parent's Size
		oANCHOR Anchor; // Relative to parent window's client area

		// Alignment within the logical rect defined above. For text that
		// is centered no matter the window size, specify MIDDLE_CENTER for
		// both Anchor and Alignment.
		oANCHOR Alignment;

		oColor Color;
		oColor ShadowColor; // Specify 0 to have unshadowed text
		bool MultiLine;
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;
	virtual DESC* Map() threadsafe = 0;
	virtual void Unmap() threadsafe = 0;
	virtual void SetFont(threadsafe oWindowUIFont* _pFont) threadsafe = 0;
	virtual void GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe = 0;
	virtual void SetText(const char* _Text) threadsafe = 0;
	inline void VPrint(const char* _Format, va_list _Args) threadsafe { oStringL s; vsprintf_s(s, _Format, _Args); SetText(s.c_str()); }

	inline void Print(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); VPrint(_Format, args); }
};

interface oWindowUIPicture : oWindowUIElement
{
	struct DESC
	{
		DESC()
			: Position(oDEFAULT, oDEFAULT)
			, Size(oDEFAULT, oDEFAULT)
			, Anchor(oMIDDLECENTER)
		{}

		int2 Position; // Relative to the anchor
		int2 Size; // oDEFAULT will use parent's Size
		oANCHOR Anchor; // Relative to parent window's client area
		oImage::DESC ImageDesc; // Desc of underlying bitmap data. Copy expects input in this format.
	};

	virtual void GetDesc(DESC* _pDesc) threadsafe = 0;
	virtual void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontal = false, bool _FlipVertical = false) threadsafe = 0;
};

bool oWindowUILineCreate(const oWindowUILine::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUILine** _ppLine);
bool oWindowUIBoxCreate(const oWindowUIBox::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUIBox** _ppBox);
bool oWindowUIFontCreate(const oWindowUIFont::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUIFont** _ppFont);
bool oWindowUITextCreate(const oWindowUIText::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUIText** _ppText);
bool oWindowUIPictureCreate(const oWindowUIPicture::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUIPicture** _ppPicture);

#endif
