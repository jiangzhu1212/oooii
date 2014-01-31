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
// API to make working with Window's GDI easier.
#pragma once
#ifndef oGDI_h
#define oGDI_h

#include <oBase/color.h>
#include <oGUI/oGUI.h>
#include <oGUI/Windows/win_gdi.h>
#include <oSurface/buffer.h>
#include <atomic>

// Captures image data of the specified window and fills _pImageBuffer.
// pImageBuffer can be NULL, in which case only _pBitmapInfo is filled out.
// For RECT, either specify a client-space rectangle, or NULL to capture the 
// window including the frame.
void oGDIScreenCaptureWindow(HWND _hWnd, const RECT* _pRect, void* _pImageBuffer, size_t _SizeofImageBuffer, BITMAPINFO* _pBitmapInfo, bool _RedrawWindow, bool _FlipV);

// Given an allocator, this will allocate the properly-sized buffer and fill it
// with the captured window image using the above oGDIScreenCaptureWindow() API.
// If the buffer is fwritten to a file, it would be a .bmp. This function 
// internally set *_ppBuffer to nullptr. Later when the allocation happens, 
// there is still a chance of failure so if this throws an exception check 
// *_ppBuffer and if not nullptr then ensure it is freed to prevent a memory 
// leak.
void oGDIScreenCaptureWindow(HWND _hWnd, bool _IncludeBorder, std::function<void*(size_t _Size)> _Allocate, void** _ppBuffer, size_t* _pBufferSize, bool _RedrawWindow, bool _FlipV);

#endif
