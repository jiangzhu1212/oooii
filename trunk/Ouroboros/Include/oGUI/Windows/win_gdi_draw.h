/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// API to make working with Window's GDI easier.
#pragma once
#ifndef oGUI_win_gdi_draw_h
#define oGUI_win_gdi_draw_h

#include <oGUI/oGUI.h>

namespace ouro {
	namespace windows {
		namespace gdi {

void draw_line(HDC _hDC, const int2& _P0, const int2& _P1);

// Uses the currently bound pen and brush to draw a box (also can be used to 
// draw a circle if a high roundness is used). If Alpha is [0,1), then a uniform 
// alpha blend is done for the box.
void draw_box(HDC _hDC, const RECT& _rBox, int _EdgeRoundness = 0, float _Alpha = 1.0f);

// Uses the currently bound pen and brush to draw an ellipse
void draw_ellipse(HDC _hDC, const RECT& _rBox);

// Returns the rect required for a single line of text using the specified HDC's 
// font and other settings.
RECT calc_text_rect(HDC _hDC, const char* _Text);

// Draws text using GDI.
void draw_text(HDC _hDC, const text_info& _Desc, const char* _Text);

// If color alpha is true 0, then a null/empty objects is returned. Use 
// DeleteObject on the value returned from these functions when finish with the
// object. (width == 0 means "default")
HPEN make_pen(color _Color, int _Width = 0);
HBRUSH make_brush(color _Color);

// Returns the COLORREF of the specified pen and optionally its thickness/width
COLORREF pen_color(HPEN _hPen, int* _pWidth = nullptr);

// Returns the COLORREF of the specified brush
COLORREF brush_color(HBRUSH _hBrush);

HFONT make_font(const font_info& _Desc);
font_info get_font_info(HFONT _hFont);

const char* font_family(BYTE _tmPitchAndFamily);
const char* char_set(BYTE _tmCharSet);

		} // namespace gdi
	} // namespace windows
} // namespace ouro

#endif
