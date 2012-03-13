/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oPlatform/oWindowUI.h>
#include "oD2DWindowUI.h"
#include "oGDIWindowUI.h"

template<typename InterfaceT, typename GDIImplT, typename D2DImplT> bool oWindowUIElementCreate(const typename InterfaceT::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe InterfaceT** _ppElement)
{
	if (!_pWindow || !_ppElement)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	bool success = false;
	switch (_pWindow->GetDrawMode())
	{
		case oWindow::USE_GDI: oCONSTRUCT(_ppElement, GDIImplT(_Desc, _pWindow, &success)); break;
		case oWindow::USE_D2D: oCONSTRUCT(_ppElement, D2DImplT(_Desc, _pWindow, &success)); break;
		default:
			oErrorSetLast(oERROR_NOT_FOUND, "oWindow with an incompatible oWindow::DRAW_MODE specified.");
			return false;
	}

	return success;
}

#define oDEFINE_WINDOWUI_CREATE(_ElementType) bool oWindowUI##_ElementType##Create(const oWindowUI##_ElementType::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUI##_ElementType** _ppElement) { return oWindowUIElementCreate<oWindowUI##_ElementType, oGDIWindowUI##_ElementType, oD2DWindowUI##_ElementType>(_Desc, _pWindow, _ppElement); }
oDEFINE_WINDOWUI_CREATE(Line);
oDEFINE_WINDOWUI_CREATE(Box);
oDEFINE_WINDOWUI_CREATE(Font);
oDEFINE_WINDOWUI_CREATE(Text);
oDEFINE_WINDOWUI_CREATE(Picture);
