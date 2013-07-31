/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Utilities for working with Windows RECTs
#pragma once
#ifndef oWinRect_h
#define oWinRect_h

#include <oBasis/oGUI.h>
#include <oPlatform/Windows/oWindows.h>

inline oRECT oRect(const RECT& _Rect) { oRECT r; r.Min = int2(_Rect.left, _Rect.top); r.Max = int2(_Rect.right, _Rect.bottom); return r; }
inline RECT oWinRect(const oRECT& _Rect) { RECT r; r.left = _Rect.Min.x; r.top = _Rect.Min.y; r.right = _Rect.Max.x; r.bottom = _Rect.Max.y; return r; }
inline RECT oWinRect(int _Left, int _Top, int _Right, int _Bottom) { RECT r; r.left = __min(_Left, _Right); r.top = __min(_Top, _Bottom); r.right = __max(_Left, _Right); r.bottom = __max(_Top, _Bottom); return r; }
inline RECT oWinRect(const int2& _TopLeft, const int2 _BottomRight) { return oWinRect(_TopLeft.x, _TopLeft.y, _BottomRight.x, _BottomRight.y); }
inline RECT oWinRectWH(int _Left, int _Top, int _Width, int _Height) { return oWinRect(_Left, _Top, _Left + _Width, _Top + _Height); }
inline RECT oWinRectWH(const int2& _Position, const int2& _Size) { return oWinRectWH(_Position.x, _Position.y, _Size.x, _Size.y); }
inline int oWinRectW(const RECT& _Rect) { return _Rect.right - _Rect.left; }
inline int oWinRectH(const RECT& _Rect) { return _Rect.bottom - _Rect.top; }
inline int2 oWinRectPosition(const RECT& _Rect) { return int2(_Rect.left, _Rect.top); }
inline int2 oWinRectSize(const RECT& _Rect) { return int2(oWinRectW(_Rect), oWinRectH(_Rect)); }

inline RECT oWinRectTranslate(const RECT& _rRect, const int2& _Translation) { RECT r = _rRect; r.left += _Translation.x; r.top += _Translation.y; r.right += _Translation.x; r.bottom += _Translation.y; return r; }
inline RECT oWinRectTranslate(const RECT& _rRect, const POINT& _Translation) { return oWinRectTranslate(_rRect, (const int2&)_Translation); }
inline RECT oWinRectTranslate(const RECT& _rRect, int _Translation) { return oWinRectTranslate(_rRect, int2(_Translation, _Translation)); }

inline RECT oWinRectDilate(const RECT& _rRect, int _Dilation) { RECT r = _rRect; r.left -= _Dilation; r.top -= _Dilation; r.right += _Dilation; r.bottom += _Dilation; return r; }

inline RECT oWinClip(const RECT& _rContainer, const RECT& _ToBeClipped) { RECT r = _ToBeClipped; r.left = __max(r.left, _rContainer.left); r.top = __max(r.top, _rContainer.top); r.right = __min(r.right, _rContainer.right); r.bottom = __min(r.bottom, _rContainer.bottom); return r; }
inline int2 oWinClip(const RECT& _rContainer, const int2& _ToBeClipped) { int2 r = _ToBeClipped; r.x = __min(__max(r.x, _rContainer.left), _rContainer.right); r.y = __min(__max(r.y, _rContainer.top), _rContainer.bottom); return r; }
inline bool oWinRectContains(const RECT& _rContainer, const int2& _Point) { return _Point.x >= _rContainer.left && _Point.x <= _rContainer.right && _Point.y >= _rContainer.top && _Point.y <= _rContainer.bottom; }

// Returns a RECT that is position to accommodate _Size and _Alignment relative 
// to _Anchor according to _Alignment. For example, a middle center alignment
// would have a rect whose center was at _Anchor.
RECT oWinRectResolve(const int2& _Anchor, const int2& _Size, oGUI_ALIGNMENT _Alignment);

#endif
