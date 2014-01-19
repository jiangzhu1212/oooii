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

namespace ouro
{

void core_fill_grid_numbers(surface::buffer* _pBuffer, const int2& _GridDimensions, color _NumberColor)
{
	surface::info si = _pBuffer->get_info();

	if (si.format != surface::b8g8r8a8_unorm)
		throw std::invalid_argument("only b8g8r8a8_unorm currently supported");

	oGDIScopedObject<HBITMAP> hBmp = windows::gdi::make_bitmap(_pBuffer);
	oGDIScopedGetDC hScreenDC(0);
	oGDIScopedDC hDC(CreateCompatibleDC(hScreenDC));

	font_info fd;
	fd.name = "Tahoma";
	fd.point_size = (oGDILogicalHeightToPoint(hDC, _GridDimensions.y) * 0.33f);
	fd.bold = fd.point_size < 15;
	oGDIScopedObject<HFONT> hFont(oGDICreateFont(fd));

	oGDIScopedSelect ScopedSelectBmp(hDC, hBmp);
	oGDIScopedSelect ScopedSelectFont(hDC, hFont);

	text_info td;
	td.alignment = alignment::middle_center;
	td.shadow = color(0);
	td.single_line = true;

	surface::fill_grid_numbers(si.dimensions.xy(), _GridDimensions,
		[&](const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)->bool
	{
		td.position = _DrawBoxPosition;
		td.size = _DrawBoxSize;
		td.foreground = _NumberColor;
		return oGDIDrawText(hDC, td, _Text);
	});

	surface::lock_guard lock(_pBuffer);
	windows::gdi::memcpy2d(lock.mapped.data, lock.mapped.row_pitch, hBmp, si.dimensions.y, true);
}

} // namespace ouro
