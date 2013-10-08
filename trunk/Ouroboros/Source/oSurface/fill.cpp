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
#include <oBase/fixed_string.h>
#include <oBase/byte.h>

namespace ouro {
	namespace surface {

void fill_solid(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color& _Color)
{
	for (int y = 0; y < _Dimensions.y; y++)
	{
		color* pScanline = byte_add(_pColors, y * _RowPitch);
		for (int x = 0; x < _Dimensions.x; x++)
			pScanline[x] = _Color;
	}
}

void fill_solid_masked(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color& _Color, uint _Mask)
{
	for (int y = 0; y < _Dimensions.y; y++)
	{
		color* pScanline = byte_add(_pColors, y * _RowPitch);
		for (int x = 0; x < _Dimensions.x; x++)
			pScanline[x] = color(((int)_Color & _Mask) | ((int)pScanline[x] & ~_Mask));
	}
}

void fill_checkerboard(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const color& _Color0, const color& _Color1)
{
	color c[2];
	for (int y = 0; y < _Dimensions.y; y++)
	{
		color* pScanline = byte_add(_pColors, y * _RowPitch);

		int tileY = y / _GridDimensions.y;

		if (tileY & 0x1)
		{
			c[0] = _Color1;
			c[1] = _Color0;
		}

		else
		{
			c[0] = _Color0;
			c[1] = _Color1;
		}

		for (int x = 0; x < _Dimensions.x; x++)
		{
			int tileX = x / _GridDimensions.x;
			pScanline[x] = c[tileX & 0x1];
		}
	}
}

void fill_gradient(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const color _CornerColors[4])
{
	for (int y = 0; y < _Dimensions.y; y++)
	{
		color* pScanline = byte_add(_pColors, y * _RowPitch);
		float Ry = y / static_cast<float>(_Dimensions.y-1);
		for (int x = 0; x < _Dimensions.x; x++)
		{
			float Rx = x / static_cast<float>(_Dimensions.x-1);
			color top = lerp(_CornerColors[0], _CornerColors[1], Rx);
			color bottom = lerp(_CornerColors[2], _CornerColors[3], Rx);
			pScanline[x] = lerp(top, bottom, Ry);
		}
	}
}

void fill_grid_lines(color* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const color& _GridColor)
{
	for (int y = 0; y < _Dimensions.y; y++)
	{
		color* pScanline = byte_add(_pColors, y * _RowPitch);
		int modY = y % _GridDimensions.y;
		if (modY == 0 || modY == (_GridDimensions.y-1))
		{
			for (int x = 0; x < _Dimensions.x; x++)
				pScanline[x] = _GridColor;
		}

		else
		{
			for (int x = 0; x < _Dimensions.x; x++)
			{
				int modX = x % _GridDimensions.x;
				if (modX == 0 || modX == (_GridDimensions.x-1))
					pScanline[x] = _GridColor;
			}
		}
	}
}

bool fill_grid_numbers(const int2& _Dimensions, const int2& _GridDimensions, std::function<bool(const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)> _DrawText)
{
	int2 DBPosition;

	int i = 0;
	for (int y = 0; y < _Dimensions.y; y += _GridDimensions.y)
	{
		DBPosition.y = y;
		for (int x = 0; x < _Dimensions.x; x += _GridDimensions.x)
		{
			DBPosition.x = x;
			sstring buf;
			snprintf(buf, "%d", i++);
			if (!_DrawText(DBPosition, _GridDimensions, buf))
				return false; // pass through error
		}
	}

	return true;
}

	} // namespace surface
} // namespace ouro