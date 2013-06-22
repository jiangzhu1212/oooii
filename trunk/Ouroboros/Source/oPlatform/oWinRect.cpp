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
#include <oPlatform/Windows/oWinRect.h>

RECT oWinRectResolve(const RECT& _rParent, const int2& _Position, const int2& _Size, oGUI_ALIGNMENT _Alignment, bool _Clip)
{
	int2 psz = oWinRectSize(_rParent);
	int2 csz = oWinRectResolveSize(_Size, psz);

	int2 cpos = _Position;
	if (cpos.x == oDEFAULT) cpos.x = 0;
	if (cpos.y == oDEFAULT) cpos.y = 0;

	int2 code = int2(_Alignment % 3, _Alignment / 3);

	// preserve user-specified offset if there was one separately from moving 
	// around the child position according to _Alignment
	int2 offset = _Position;
	if (offset.x == oDEFAULT || code.x == 0) offset.x = 0;
	if (offset.y == oDEFAULT || code.y == 0) offset.y = 0;

	// All this stuff is top-left by default, so adjust for center/middle and 
	// right/bottom

	// center/middle
	if (code.x == 1) cpos.x = (psz.x - csz.x) / 2;
	if (code.y == 1) cpos.y = (psz.y - csz.y) / 2;

	// right/bottom
	if (code.x == 2) cpos.x = _rParent.right - csz.x;
	if (code.y == 2) cpos.y = _rParent.bottom - csz.y;

	RECT rResolved = oWinRectTranslate(oWinRectWH(cpos, csz), oWinRectPosition(_rParent) + offset);

	if (_Clip)
		rResolved = oWinClip(_rParent, rResolved);

	return rResolved;
}

RECT oWinRectResolve(const int2& _Anchor, const int2& _Size, oGUI_ALIGNMENT _Alignment)
{
	int2 offset(0,0);
	int2 code = int2(_Alignment % 3, _Alignment / 3);

	// center/middle
	if (code.x == 1) offset.x = -_Size.x / 2;
	if (code.y == 1) offset.y = -_Size.y / 2;

	// right/bottom
	if (code.x == 2) offset.x = -_Size.x;
	if (code.y == 2) offset.y = -_Size.y;

	return oWinRectTranslate(oWinRectWH(_Anchor, _Size), offset);
}
