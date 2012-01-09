// $(header)
#include <oPlatform/oWinRect.h>

RECT oWinRectResolve(const RECT& _rParent, const int2& _Position, const int2& _Size, oANCHOR _Anchor, bool _Clip)
{
	int2 psz = oWinRectSize(_rParent);
	int2 csz = _Size;
	if (csz.x == oDEFAULT) csz.x = psz.y;
	if (csz.y == oDEFAULT) csz.y = psz.y;

	int2 cpos = _Position;
	if (cpos.x == oDEFAULT) cpos.x = 0;
	if (cpos.y == oDEFAULT) cpos.y = 0;

	// preserve user-specified offset if there was one separately from moving 
	// around the child position according to _Anchor
	int2 offset = _Position;
	if (offset.x == oDEFAULT) offset.x = 0;
	if (offset.y == oDEFAULT) offset.y = 0;

	int2 code = int2(_Anchor % 3, _Anchor / 3);

	// All this stuff is top-left by default, so adjust for center/middle and 
	// right/bottom

	// center/middle
	if (code.x == 1) cpos.x = (psz.x - csz.x) / 2;
	if (code.y == 1) cpos.y = (psz.y - csz.y) / 2;

	// right/bottom
	if (code.x == 2) cpos.x = _rParent.right - csz.x;
	if (code.y == 2) cpos.y = _rParent.bottom - csz.y;

	RECT rResolved = oWinRectTranslate(oWinRectWH(cpos, csz), offset);

	if (_Clip)
		rResolved = oWinClip(_rParent, rResolved);

	return rResolved;
}
