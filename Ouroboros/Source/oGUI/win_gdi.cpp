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
#include <oGUI/Windows/win_gdi.h>

namespace ouro {
	namespace windows {
		namespace gdi {
		
int point_to_logical_height(HDC _hDC, int _Point)
{
	return -MulDiv(_Point, GetDeviceCaps(_hDC, LOGPIXELSY), 72);
}

int point_to_logical_heightf(HDC _hDC, float _Point)
{
	return point_to_logical_height(_hDC, static_cast<int>(_Point + 0.5f));
}

int logical_height_to_point(HDC _hDC, int _Height)
{
	return MulDiv(_Height, 72, GetDeviceCaps(_hDC, LOGPIXELSY));
}

float logical_height_to_pointf(HDC _hDC, int _Height)
{
	return (_Height * 72.0f) / (float)GetDeviceCaps(_hDC, LOGPIXELSY);
}

float2 dpi_scale(HDC _hDC)
{
	return float2(GetDeviceCaps(_hDC, LOGPIXELSX) / 96.0f, GetDeviceCaps(_hDC, LOGPIXELSY) / 96.0f);
}

		} // namespace gdi
	} // namespace windows
} // namespace ouro
