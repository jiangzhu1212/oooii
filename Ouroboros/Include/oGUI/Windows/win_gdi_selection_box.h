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
// A 2D screen space selection box implemented using Windows GDI.
#pragma once
#ifndef oGUI_win_gdi_selection_box_h
#define oGUI_win_gdi_selection_box_h

#include <oBase/color.h>
#include <oGUI/Windows/win_gdi.h>

namespace ouro {
	namespace windows {
		namespace gdi {

class selection_box
{
	// Like when you drag to select several files in Windows Explorer. This uses
	// SetCapture() and ReleaseCapture() on hParent to retain mouse control while
	// drawing the selection box.

	selection_box(const selection_box&);
	const selection_box& operator=(const selection_box&);

public:
	struct info
	{
		info()
			: hParent(nullptr)
			, Fill(color(dodger_blue, 0.33f))
			, Border(dodger_blue)
			, EdgeRoundness(0)
			, UseOffscreenRender(false)
		{}

		HWND hParent;
		color Fill; // can have alpha value
		color Border;
		int EdgeRoundness;

		// DXGI back-buffers don't like rounded edge rendering, so copy contents
		// to an offscreen HBITMAP, draw the rect and resolve it back to the main
		// target. Even if this is true, offscreen render only occurs if there is
		// edge roundness.
		bool UseOffscreenRender;
	};

	selection_box();
	selection_box(selection_box&& _That) { operator=(_That); }
	const selection_box& operator=(selection_box&& _That);

	void set_info(const info& _Info);
	info get_info() const;

	// returns true if drawing is required by the state (between mouse down and 
	// mouse up events). This is useful if there's non-trivial effort in 
	// extracting the HDC that draw will use.
	bool selecting() const;

	void draw(HDC _hDC);
	void on_resize(const int2& _NewParentSize);
	void on_mouse_down(const int2& _MousePosition);
	void on_mouse_move(const int2& _MousePosition);
	void on_mouse_up();

private:
	scoped_pen hPen;
	scoped_brush hBrush;
	scoped_brush hNullBrush;
	scoped_bitmap hOffscreenBMP;

	int2 MouseDownAt;
	int2 MouseAt;
	float Opacity;
	bool Selecting;

	info Info;
};

		} // namespace gdi
	} // namespace windows
} // namespace ouro

#endif
