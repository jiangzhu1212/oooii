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
#include <oGUI/Windows/oGDI.h>
#include <oCore/windows/win_error.h>
#include <oBase/byte.h>
#include <oGUI/windows/win_gdi_bitmap.h>

using namespace ouro;
using namespace ouro::windows::gdi;

void oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow, bool _FlipV)
{
	RECT r;
	if (_pRect)
	{
		// Find offset into client area
		POINT p = {0,0};
		ClientToScreen(_hWnd, &p);

		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		p.x -= wr.left;
		p.y -= wr.top;

		r.left = _pRect->left + p.x;
		r.right = _pRect->right + p.x;
		r.top = _pRect->top + p.y;
		r.bottom = _pRect->bottom + p.y;
	}

	else
	{
		RECT wr;
		oVB(GetWindowRect(_hWnd, &wr));
		r.left = 0;
		r.top = 0;
		r.right = oWinRectW(wr);
		r.bottom = oWinRectH(wr);
	}

	int2 size = oWinRectSize(r);

	if (size.x == 0 || size.y == 0)
		throw std::invalid_argument("invalid size");

	WORD BitDepth = 0;
	{
		ouro::display::info di = ouro::display::get_info(oWinGetDisplayId(_hWnd));
		BitDepth = static_cast<WORD>(di.mode.depth);
		if (BitDepth == 32) BitDepth = 24;
	}

	if (!_pBitmapInfo)
		oTHROW_INVARG0();
	const int BytesPerPixel = BitDepth / 8;
	const int AlignedWidth = byte_align(size.x, 4);
	const int RowPitch = AlignedWidth * BytesPerPixel;

	memset(_pBitmapInfo, 0, sizeof(BITMAPINFO));
	_pBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	_pBitmapInfo->bmiHeader.biWidth = size.x;
	_pBitmapInfo->bmiHeader.biHeight = size.y;
	_pBitmapInfo->bmiHeader.biPlanes = 1;
	_pBitmapInfo->bmiHeader.biBitCount = BitDepth;
	_pBitmapInfo->bmiHeader.biCompression = BI_RGB;
	_pBitmapInfo->bmiHeader.biSizeImage = RowPitch * size.y;

	if (_pImageBuffer)
	{
		if (_SizeofImageBuffer < _pBitmapInfo->bmiHeader.biSizeImage)
			oTHROW0(no_buffer_space);

		HDC hDC = GetWindowDC(_hWnd);
		HDC hMemDC = CreateCompatibleDC(hDC);

		HBITMAP hBMP = CreateCompatibleBitmap(hDC, size.x, size.y);
		HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBMP);
		if (_RedrawWindow)
			oVB(RedrawWindow(_hWnd, 0, 0, RDW_INVALIDATE|RDW_UPDATENOW));

		BitBlt(hMemDC, 0, 0, size.x, size.y, hDC, r.left, r.top, SRCCOPY);

		const int PadSize = AlignedWidth - size.x;
		for (int y = 0; y < size.y; y++)
		{
			GetDIBits(hMemDC, hBMP, _FlipV ? (size.y - 1 - y) : y, 1, byte_add(_pImageBuffer, y * RowPitch), _pBitmapInfo, DIB_RGB_COLORS);
			char* pad = (char*)byte_add(_pImageBuffer, (y*RowPitch) + (size.x*BytesPerPixel));
			switch (PadSize)
			{
				// Duff's device
				case 3: pad[2] = 0;
				case 2: pad[1] = 0;
				case 1: pad[0] = 0;
				default: break;
			}
		}

		SelectObject(hMemDC, hOld);
		DeleteDC(hMemDC);
		ReleaseDC(0, hDC);
	}
}

void oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, std::function<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow, bool _FlipV)
{
	if (!_Allocate || !_ppBuffer || !_pBufferSize)
		oTHROW_INVARG0();

	*_ppBuffer = nullptr;
	*_pBufferSize = 0;

	// Create a bmp in memory
	RECT r;
	GetClientRect(_hWnd, &r);
	BITMAPINFO bmi;

	RECT* pRect = &r;
	if (_IncludeBorder)
		pRect = nullptr;

	oGDIScreenCaptureWindow(_hWnd, pRect, nullptr, 0, &bmi, _RedrawWindow, _FlipV);
	BITMAPFILEHEADER bmfh;
	memset(&bmfh, 0, sizeof(bmfh));
	bmfh.bfType = 'MB';
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	bmfh.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO));

	*_pBufferSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bmi.bmiHeader.biSizeImage;
	*_ppBuffer = _Allocate(*_pBufferSize);
	memcpy(*_ppBuffer, &bmfh, sizeof(bmfh));
	memcpy(byte_add(*_ppBuffer, sizeof(bmfh)), &bmi, sizeof(bmi));
	oGDIScreenCaptureWindow(_hWnd, pRect, byte_add(*_ppBuffer, sizeof(bmfh) + sizeof(bmi)), bmi.bmiHeader.biSizeImage, &bmi, _RedrawWindow, _FlipV);
}
