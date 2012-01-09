// $(header)
// Utilities for working with Windows RECTs
#pragma once
#ifndef oWinRect_h
#define oWinRect_h

#include <oPlatform/oWindowUI.h>
#include <oPlatform/oWindows.h>

inline oRECT oRect(const RECT& _Rect) { oRECT r; r.SetMin(int2(_Rect.left, _Rect.top)); r.SetMax(int2(_Rect.right, _Rect.bottom)); return r; }

inline RECT oWinRect(int _Left, int _Top, int _Right, int _Bottom) { RECT r; r.left = _Left; r.top = _Top; r.right = _Right; r.bottom = _Bottom; return r; }
inline RECT oWinRectWH(int _Left, int _Top, int _Width, int _Height) { RECT r; r.left = _Left; r.top = _Top; r.right = (_Left == oDEFAULT || _Width == oDEFAULT) ? oDEFAULT : (r.left + _Width);r.bottom = (_Top == oDEFAULT || _Height == oDEFAULT) ? oDEFAULT : (r.top + _Height); return r; }
inline RECT oWinRectWH(const int2& _Position, const int2& _Size) { return oWinRectWH(_Position.x, _Position.y, _Size.x, _Size.y); }
inline int oWinRectW(const RECT& _Rect) { return _Rect.right - _Rect.left; }
inline int oWinRectH(const RECT& _Rect) { return _Rect.bottom - _Rect.top; }
inline int2 oWinRectPosition(const RECT& _Rect) { return int2(_Rect.left, _Rect.top); }
inline int2 oWinRectSize(const RECT& _Rect) { return int2(oWinRectW(_Rect), oWinRectH(_Rect)); }
inline int2 oWinRectCenter(const RECT& _Rect) { return oWinRectPosition(_Rect) + (oWinRectSize(_Rect) / 2); }

inline RECT oWinRectTranslate(const RECT& _rRect, const int2& _Translation) { RECT r = _rRect; r.left += _Translation.x; r.top += _Translation.y; r.right += _Translation.x; r.bottom += _Translation.y; return r; }
inline RECT oWinRectTranslate(const RECT& _rRect, int _Translation) { return oWinRectTranslate(_rRect, int2(_Translation, _Translation)); }

inline RECT oWinRectScale(const RECT& _rRect, const int2& _Scale) { RECT r = _rRect; r.left *= _Scale.x; r.top *= _Scale.y; r.right *= _Scale.x; r.bottom *= _Scale.y; return r; }
inline RECT oWinRectScale(const RECT& _rRect, int _Scale) { return oWinRectScale(_rRect, int2(_Scale, _Scale)); }

inline RECT oWinClip(const RECT& _rContainer, const RECT& _ToBeClipped) { RECT r = _ToBeClipped; r.left = __max(r.left, _rContainer.left); r.top = __max(r.top, _rContainer.top); r.right = __max(r.right, _rContainer.right); r.bottom = __max(r.bottom, _rContainer.bottom); return r; }
inline bool oWinRectContains(const RECT& _rContainer, const int2& _Point) { return _Point.x >= _rContainer.left && _Point.x <= _rContainer.right && _Point.y >= _rContainer.top && _Point.y <= _rContainer.bottom; }

// Returns a RECT that is positioned to accomodate _Position, _Size and _Anchor
// inside _rParent. oDEFAULT sizes are resolved to _rParent's size. oDEFAULT 
// positions are resolved to 0,0-off-anchor. Any position value will be relative
// to _Anchor. _Clip will ensure dimensions are kept within the bounds of 
// _rParent and is evaluated at the very end, after positioning has occurred.
RECT oWinRectResolve(const RECT& _rParent, const int2& _Position, const int2& _Size, oANCHOR _Anchor, bool _Clip);

// Ensure the specified desc includes the specified fields
template<typename desc_t> static RECT oWinRectResolve(threadsafe oWindow* _pWindow, const desc_t& _Desc, bool _Clip = false)
{
	RECT rClient;
	oVB(GetClientRect((HWND)_pWindow->GetNativeHandle(), &rClient));
	return oWinRectResolve(rClient, _Desc.Position, _Desc.Size, _Desc.Anchor, _Clip);
}

#endif
