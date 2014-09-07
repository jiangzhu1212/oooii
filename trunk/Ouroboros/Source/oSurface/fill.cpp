// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/fill.h>
#include <oString/fixed_string.h>
#include <oMemory/byte.h>

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

void fill_color_cube(color* _pColors, size_t _RowPitch, size_t _SlicePitch, const int3& _Dimensions)
{
	float3 fcolor(0.0f, 0.0f, 0.0f);
	float3 step = float3(1.0f) / float3(_Dimensions - 1);

	for (int z = 0; z < _Dimensions.z; z++)
	{
		color* pSlice = byte_add(_pColors, z * _SlicePitch);

		fcolor.y = 0.0f;
		for (int y = 0; y < _Dimensions.y; y++)
		{
			color* pScanline = byte_add(pSlice, y * _RowPitch);
			fcolor.x = 0.0f;
			for (int x = 0; x < _Dimensions.x; x++)
			{
				pScanline[x] = color(fcolor.x, fcolor.y, fcolor.z, 1.0f);
				fcolor.x += step.x;
			}

			fcolor.y += step.y;
		}

		fcolor.z += step.z;
	}
}

	} // namespace surface
} // namespace ouro
