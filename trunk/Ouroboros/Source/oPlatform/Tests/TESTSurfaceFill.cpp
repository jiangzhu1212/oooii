/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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
#include <oBasis/oSurfaceFill.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/oTest.h>
#include <oPlatform/Windows/oWinRect.h>

// Creates an image filled with a gradiant of 4 colors then draws a grid on it
// with numbers in each grid in order left to right, top to bottom.
bool oImageCreateNumberedGrid(const int2& _Dimensions, const int2& _GridDimensions, oStd::color _GridColor, oStd::color _NumberColor, oStd::color _CornerColors[4], oImage** _ppImage)
{
	oImage::DESC d;
	d.Dimensions = _Dimensions;
	d.Format = oImage::BGRA32;
	d.RowPitch = oImageCalcRowPitch(d.Format, d.Dimensions.x);
	if (!oImageCreate("GradientImage", d, _ppImage))
		return false; // pass through error
	(*_ppImage)->GetDesc(&d);

	oStd::color* c = (oStd::color*)(*_ppImage)->GetData();
	oSurfaceFillGradient(c, d.RowPitch, d.Dimensions, _CornerColors);
	oSurfaceFillGridLines(c, d.RowPitch, d.Dimensions, _GridDimensions, _GridColor);

	// Lots of code to create an HBITMAP usable by GDI, then draw text to it, then
	// copy it back to the image.
	{
		HBITMAP hBmp;
		if (!oImageCreateBitmap(*_ppImage, &hBmp))
			return false; // pass through error

		oGDIScopedGetDC hScreenDC(0);
		oGDIScopedDC hDC(CreateCompatibleDC(hScreenDC));

		oGUI_FONT_DESC fd;
		fd.FontName = "Tahoma";
		fd.PointSize = (oGDILogicalHeightToPoint(hDC, _GridDimensions.y) * 0.33f);
		fd.Bold = fd.PointSize < 15;

		oGDIScopedObject<HFONT> hFont(oGDICreateFont(fd));

		oGDIScopedSelect SelectBmp(hDC, hBmp);
		oGDIScopedSelect SelectFont(hDC, hFont);

		if (!oSurfaceFillGridNumbers(d.Dimensions, _GridDimensions,
			[&](const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)->bool
		{
			oGUI_TEXT_DESC td;
			td.Position = _DrawBoxPosition;
			td.Size = _DrawBoxSize;
			td.Alignment = oGUI_ALIGNMENT_MIDDLE_CENTER;
			td.Foreground = _NumberColor;
			td.Shadow = 0;
			td.SingleLine = true;
			return oGDIDrawText(hDC, td, _Text);
		}))
			return false; // pass through error

		(*_ppImage)->CopyData(hBmp);
	}

	return true;
}

bool oImageCreateCheckerboard(const int2& _Dimensions, const int2& _GridDimensions, oStd::color _Color0, oStd::color _Color1, oImage** _ppImage)
{
	oImage::DESC d;
	d.Dimensions = _Dimensions;
	d.Format = oImage::BGRA32;
	d.RowPitch = oImageCalcRowPitch(d.Format, d.Dimensions.x);
	if (!oImageCreate("CheckerImage", d, _ppImage))
		return false; // pass through error
	(*_ppImage)->GetDesc(&d);
	oSurfaceFillCheckerboard((oStd::color*)(*_ppImage)->GetData(), d.RowPitch, d.Dimensions, int2(64,64), _Color0, _Color1);
	return true;
}

bool oImageCreateSolid(const int2& _Dimensions, oStd::color _Color, oImage** _ppImage)
{
	oImage::DESC d;
	d.Dimensions = _Dimensions;
	d.Format = oImage::BGRA32;
	d.RowPitch = oImageCalcRowPitch(d.Format, d.Dimensions.x);
	if (!oImageCreate("CheckerImage", d, _ppImage))
		return false; // pass through error
	(*_ppImage)->GetDesc(&d);
	oSurfaceFillSolid((oStd::color*)(*_ppImage)->GetData(), d.RowPitch, d.Dimensions, _Color);
	return true;
}

struct TESTSurfaceFill : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oStd::intrusive_ptr<oImage> Image[4];
		oStd::color gradiantColors0[4] = { oStd::Blue, oStd::Purple, oStd::Lime, oStd::Orange};
		oTESTB0(oImageCreateNumberedGrid(int2(256,256), int2(64,64), oStd::Black, oStd::Black, gradiantColors0, &Image[0]));
		oTESTI2(Image[0], 0);
		oStd::color gradiantColors1[4] = { oStd::MidnightBlue, oStd::DarkSlateBlue, oStd::Green, oStd::Chocolate };
		oTESTB0(oImageCreateNumberedGrid(int2(512,512), int2(32,32), oStd::Gray, oStd::White, gradiantColors1, &Image[1]));
		oTESTI2(Image[1], 1);
		oTESTB0(oImageCreateCheckerboard(int2(256,256), int2(32,32), oStd::Cyan, oStd::Pink, &Image[2]));
		oTESTI2(Image[2], 2);
		oTESTB0(oImageCreateSolid(int2(256,256), oStd::TangentSpaceNormalBlue, &Image[3]));
		oTESTI2(Image[3], 3);

		return SUCCESS;
	}
};

oTEST_REGISTER(TESTSurfaceFill);
