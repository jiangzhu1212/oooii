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
#include <oSurface/fill.h>
#include <oGUI/Windows/oGDI.h>
#include <oPlatform/oTest.h>
#include <oGUI/Windows/oWinRect.h>

void PlatformFillGridNumbers(ouro::surface::buffer* _pBuffer
	, const int2& _GridDimensions, ouro::color _NumberColor)
{
	ouro::surface::info si = _pBuffer->get_info();

	if (si.format != ouro::surface::b8g8r8a8_unorm)
		throw std::invalid_argument("only b8g8r8a8_unorm currently supported");

	oGDIScopedObject<HBITMAP> hBmp = CreateBitmap(si.dimensions.x, si.dimensions.y, 1, 32, nullptr);

	oGDIScopedGetDC hScreenDC(0);
	oGDIScopedDC hDC(CreateCompatibleDC(hScreenDC));

	oGUI_FONT_DESC fd;
	fd.FontName = "Tahoma";
	fd.PointSize = (oGDILogicalHeightToPoint(hDC, _GridDimensions.y) * 0.33f);
	fd.Bold = fd.PointSize < 15;

	oGDIScopedObject<HFONT> hFont(oGDICreateFont(fd));

	oGDIScopedSelect ScopedSelectBmp(hDC, hBmp);
	oGDIScopedSelect ScopedSelectFont(hDC, hFont);

	oGUI_TEXT_DESC td;
	td.Alignment = ouro::alignment::middle_center;
	td.Shadow = ouro::color(0);
	td.SingleLine = true;

	ouro::surface::fill_grid_numbers(si.dimensions.xy(), _GridDimensions,
		[&](const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)->bool
	{
		td.Position = _DrawBoxPosition;
		td.Size = _DrawBoxSize;
		td.Foreground = _NumberColor;
		return oGDIDrawText(hDC, td, _Text);
	});

	oBMI_DESC bmid;
	bmid.Dimensions = si.dimensions.xy();
	bmid.Format = si.format;

	BITMAPINFO bmi;
	oGDIInitializeBMI(bmid, &bmi);

	std::vector<char> temp;
	temp.resize(bmi.bmiHeader.biSizeImage);

	if (!GetDIBits(hScreenDC, hBmp, 0, si.dimensions.y, temp.data(), &bmi, DIB_RGB_COLORS))
		oTHROW(io_error, "GetDIBits failed");

	ouro::surface::const_mapped_subresource msr;
	msr.data = temp.data();
	msr.row_pitch = bmi.bmiHeader.biSizeImage / bmi.bmiHeader.biHeight;
	_pBuffer->update_subresource(0, msr, true);
}
