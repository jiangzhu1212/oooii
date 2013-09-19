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
#include <oPlatform/Windows/oWinControlSet.h>
#include <oStd/algorithm.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinRect.h>

bool oWinControlSet::Initialize(HWND _hParent, const int2& _ParentClientSize, const oStd::xml& _XML, const IDFROMSTRING& _IDFromString)
{
	if (!_hParent)
		return oErrorSetLast(std::errc::invalid_argument, "A valid HWND must be specified as the parent");

	oStd::xml::node hRoot = _XML.first_child(_XML.root(), "oUI");
	if (!hRoot)
		return oErrorSetLast(std::errc::invalid_argument, "XML root node \"oUI\" not found");

	fonts_t fonts;
	if (!CreateFontsSibling(_XML, &fonts))
		return false; // pass through error

	CONTROL_CONTEXT cc;
	cc.hParent = _hParent;
	cc.ParentSize = _ParentClientSize;
	cc.IDFromString = _IDFromString;

	XML_CONTEXT xc(fonts);
	xc.pXML = &_XML;
	xc.hNode = hRoot;

	if (!CreateControlsSibling(xc, cc, &Controls))
		return false; // pass through error

	return true;
}

void oWinControlSet::Deinitialize()
{
	oFOR(HWND hWnd, Controls)
		if (IsWindow(hWnd))
			DestroyWindow(hWnd);

	Controls.clear();
}

HWND oWinControlSet::GetControl(int _ID) const
{
	if (_ID < 0 || _ID >= oInt(Controls.size()))
		return (HWND)oErrorSetLast(std::errc::invalid_argument, "Invalid ID %d", _ID);
	return Controls[_ID];
}

bool oWinControlSet::CreateFontsSibling(const oStd::xml& _XML, fonts_t* _pFonts)
{
	oStd::xml::node hRoot = _XML.first_child(_XML.root(), "oUI");
	if (!hRoot)
		return oErrorSetLast(std::errc::invalid_argument, "XML root node \"oUI\" not found");

	oStd::xml::node hChild = _XML.first_child(hRoot, "Font");
	while (hChild)
	{
		const char* StrFont = _XML.find_attr_value(hChild, "Name");
		if (!StrFont)
			return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Name attribute in a Font node");

		if (_pFonts->end() != _pFonts->find(StrFont))
			return oErrorSetLast(std::errc::invalid_argument, "Font %s already defined", StrFont);

		oGUI_FONT_DESC fd;
		if (!ParseFontDesc(_XML, hChild, &fd))
			return false; // pass through error

		HFONT hFont = oGDICreateFont(fd);
		if (!hFont)
			return oErrorSetLast(std::errc::invalid_argument, "Could not create font %s: %s: %s", StrFont, oErrorAsString(oErrorGetLast()), oErrorGetLastString());

		(*_pFonts)[StrFont] = hFont;

		hChild = _XML.next_sibling(hChild, "Font");
	}

	return true;
}

// @oooii-tony: Should this be promoted to somewhere more generic?
#define oXML_GETVALUE(_XML, _hNode, _pDestStruct, _FieldName) _XML.find_attr_value(_hNode, #_FieldName, &_pDestStruct->_FieldName);

bool oWinControlSet::ParseFontDesc(const oStd::xml& _XML, oStd::xml::node _hNode, oGUI_FONT_DESC* _pDesc)
{
	oXML_GETVALUE(_XML, _hNode, _pDesc, FontName);
	oXML_GETVALUE(_XML, _hNode, _pDesc, PointSize);
	oXML_GETVALUE(_XML, _hNode, _pDesc, Bold);
	oXML_GETVALUE(_XML, _hNode, _pDesc, Italic);
	oXML_GETVALUE(_XML, _hNode, _pDesc, Underline);
	oXML_GETVALUE(_XML, _hNode, _pDesc, StrikeOut);
	return true;
}

bool oWinControlSet::ParseControlDesc(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, const controls_t& _Controls, oGUI_CONTROL_DESC* _pDesc)
{
	_pDesc->hParent = (oGUI_WINDOW)_ControlContext.hParent;
	_pDesc->hFont = (oGUI_FONT)_ControlContext.hFont;

	const char* StrID = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "ID");
	if (!oSTRVALID(StrID))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing ID attribute for the specified Control.");

	int ID = oInvalid;
	if (_ControlContext.IDFromString(&ID, StrID))
		_pDesc->ID = oUShort(ID);
	else
		return oErrorSetLast(std::errc::invalid_argument, "Undeclared ID %s. All IDs must be declared as an enum in code.", StrID);

	if (ID < oInt(_Controls.size()) && _Controls[ID])
		return oErrorSetLast(std::errc::invalid_argument, "ID %s has already been used", StrID);

	if (!_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Type", &_pDesc->Type))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Type attribute for Control %s.", StrID);

	_pDesc->Text = oSAFESTR(_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Text"));
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "StartsNewGroup", &_pDesc->StartsNewGroup); // allow this to default to false

	const char* StrFont = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Font"); // allow this to default to the last context's font
	if (StrFont)
	{
		auto it = _XmlContext.Fonts.find(StrFont);
		if (it == _XmlContext.Fonts.end())
			return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Font %s defined in Control %s", StrFont, StrID);
		_pDesc->hFont = (oGUI_FONT)it->second;
	}

	// Resolve size relative to parent/context
	int2 ControlPos(0, 0);
	if (!_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Pos", &ControlPos))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Pos attribute for Control %s.", StrID);

	int2 ControlSize(oDEFAULT, oDEFAULT);
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Size", &ControlSize); // allow oDEFAULT to fall through if not specified
	ControlSize = oWinControlGetInitialSize(_pDesc->Type, ControlSize);

	oGUI_ALIGNMENT alignment = oGUI_ALIGNMENT_TOP_LEFT;
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Align", &alignment); // allow TopLeft as default value

	RECT rParentClient;
	int2 ParentSize = oGUIResolveRectSize(_ControlContext.ParentSize, oWinRectSize(rParentClient));
	RECT rParent = oWinRectWH(int2(0,0), ParentSize);

	RECT rControl = oWinRect(oGUIResolveRect(oRect(rParent), ControlPos, ControlSize, alignment, false));
	_pDesc->Position = oWinRectPosition(rControl);
	_pDesc->Size = oWinRectSize(rControl);

	return true;
}

void oWinControlSet::AddControl(int _ID, HWND _hHandle)
{
	oStd::safe_set(Controls, _ID, _hHandle);
}

HWND oWinControlSet::CreateControl(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls, int2 _ParentOffset)
{
	oGUI_CONTROL_DESC d;
	if (!ParseControlDesc(_XmlContext, _ControlContext, *_pControls, &d))
		return nullptr; // pass through error

	bool enabled = _ControlContext.Enabled;
	bool visible = _ControlContext.Visible;
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Enabled", &enabled);
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Visible", &visible);

	d.Position += _ParentOffset;
	HWND hControl = oWinControlCreate(d);
	if (!hControl)
		return (HWND)oWinSetLastError();

	// @oooii-tony: there still might be some redraw issues if this is the last 
	// control that may justify moving visible and enabled into control creation
	// to take advantage of first-draw

	if (!enabled)
		oWinControlEnable(hControl, false);

	if (!visible)
		oWinControlSetVisible(hControl, false);

	oStd::safe_set(*_pControls, d.ID, hControl);
	return hControl;
}

bool oWinControlSet::CreateControlsSibling(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls)
{
	oStd::xml::node hChild = _XmlContext.pXML->first_child(_XmlContext.hNode);
	while (hChild)
	{
		XML_CONTEXT xc = _XmlContext;
		xc.hNode = hChild;
		if (!CreateControlsRecursive(xc, _ControlContext, _pControls))
			return false; // pass through error
		hChild = _XmlContext.pXML->next_sibling(hChild);
	}

	return true;
}

bool oWinControlSet::CreateControlsRecursive(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls)
{
	CONTROL_CONTEXT Current = _ControlContext;
	const char* NodeName = _XmlContext.pXML->node_name(_XmlContext.hNode);
	if (oSTRVALID(NodeName))
	{
		bool VisitChildren = false;
		int2 RelativePos;
		int2 Size(oInvalid, oInvalid);
		if (!_stricmp("RefPoint", NodeName))
		{
			//#ifdef _DEBUG
			//	oTRACE("<RefPoint Name=\"%s\" ... >", oSAFESTRN(_XmlContext.pXML->GetValue(_XmlContext.hNode, "Name")));
			//#endif
			  
			if (_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Pos", &RelativePos))
				Current.ParentPosition += RelativePos;

			if (_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Size", &Size))
				Current.ParentSize = Size; 

			const char* StrFont = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Font");
			if (StrFont)
				Current.hFont = ((fonts_t&)_XmlContext.Fonts)[StrFont];

			_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Visible", &Current.Visible);
			_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Enabled", &Current.Enabled);

			VisitChildren = true;
		}

		else if (!_stricmp("Control", NodeName))
		{
			Current.hParent = CreateControl(_XmlContext, Current, _pControls, Current.ParentPosition);
			if (!Current.hParent)
				return false; // pass through error

			RECT rParent = oWinGetRelativeRect(Current.hParent);
			Current.ParentPosition = oWinRectPosition(rParent);
			Current.ParentSize = oWinRectSize(rParent);
			VisitChildren = true;
		}

		if (VisitChildren)
		{
			if (!CreateControlsSibling(_XmlContext, Current, _pControls))
				return false; // pass through error
		}
	}
	return true;
}
