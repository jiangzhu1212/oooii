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
#include <oPlatform/oGUI.h>

const char* oAsString(const oGUI_WINDOW_STATE& _State)
{
	switch (_State)
	{
		case oGUI_WINDOW_NONEXISTANT: return "oGUI_WINDOW_NONEXISTANT";
		case oGUI_WINDOW_HIDDEN: return "oGUI_WINDOW_HIDDEN";
		case oGUI_WINDOW_MINIMIZED: return "oGUI_WINDOW_MINIMIZED";
		case oGUI_WINDOW_RESTORED: return "oGUI_WINDOW_RESTORED";
		case oGUI_WINDOW_MAXIMIZED: return "oGUI_WINDOW_MAXIMIZED";
		case oGUI_WINDOW_FULLSCREEN: return "oGUI_WINDOW_FULLSCREEN";
		oNODEFAULT;
	}
}

const char* oAsString(const oGUI_WINDOW_STYLE& _Style)
{
	switch (_Style)
	{
		case oGUI_WINDOW_EMBEDDED: return "oGUI_WINDOW_EMBEDDED";
		case oGUI_WINDOW_BORDERLESS: return "oGUI_WINDOW_BORDERLESS";
		case oGUI_WINDOW_FIXED: return "oGUI_WINDOW_FIXED";
		case oGUI_WINDOW_DIALOG: return "oGUI_WINDOW_DIALOG";
		case oGUI_WINDOW_SIZEABLE: return "oGUI_WINDOW_SIZEABLE";
		oNODEFAULT;
	}
}

const char* oAsString(const oGUI_CURSOR_STATE& _State)
{
	switch (_State)
	{
		case oGUI_CURSOR_NONE: return "oGUI_CURSOR_NONE";
		case oGUI_CURSOR_ARROW: return "oGUI_CURSOR_ARROW";
		case oGUI_CURSOR_HAND: return "oGUI_CURSOR_HAND";
		case oGUI_CURSOR_HELP: return "oGUI_CURSOR_HELP";
		case oGUI_CURSOR_NOTALLOWED: return "oGUI_CURSOR_NOTALLOWED";
		case oGUI_CURSOR_WAIT_FOREGROUND: return "oGUI_CURSOR_WAIT_FOREGROUND";
		case oGUI_CURSOR_WAIT_BACKGROUND: return "oGUI_CURSOR_WAIT_BACKGROUND";
		case oGUI_CURSOR_USER: return "oGUI_CURSOR_USER";
		oNODEFAULT;
	}
}

const char* oAsString(const oGUI_ALIGNMENT& _Alignment)
{
	switch (_Alignment)
	{
		case oGUI_ALIGNMENT_TOP_LEFT: return "oGUI_ALIGNMENT_TOP_LEFT";
		case oGUI_ALIGNMENT_TOP_CENTER: return "oGUI_ALIGNMENT_TOP_CENTER";
		case oGUI_ALIGNMENT_TOP_RIGHT: return "oGUI_ALIGNMENT_TOP_RIGHT";
		case oGUI_ALIGNMENT_MIDDLE_LEFT: return "oGUI_ALIGNMENT_MIDDLE_LEFT";
		case oGUI_ALIGNMENT_MIDDLE_CENTER: return "oGUI_ALIGNMENT_MIDDLE_CENTER";
		case oGUI_ALIGNMENT_MIDDLE_RIGHT: return "oGUI_ALIGNMENT_MIDDLE_RIGHT";
		case oGUI_ALIGNMENT_BOTTOM_LEFT: return "oGUI_ALIGNMENT_BOTTOM_LEFT";
		case oGUI_ALIGNMENT_BOTTOM_CENTER: return "oGUI_ALIGNMENT_BOTTOM_CENTER";
		case oGUI_ALIGNMENT_BOTTOM_RIGHT: return "oGUI_ALIGNMENT_BOTTOM_RIGHT";
		case oGUI_ALIGNMENT_FIT_PARENT: return "oGUI_ALIGNMENT_FIT_PARENT";
		oNODEFAULT;
	}
}

oAPI const char* oAsString(const oGUI_CONTROL_TYPE& _Type)
{
	switch (_Type)
	{
		case oGUI_CONTROL_UNKNOWN: return "oGUI_CONTROL_UNKNOWN";
		case oGUI_CONTROL_GROUPBOX: return "oGUI_CONTROL_GROUPBOX";
		case oGUI_CONTROL_BUTTON: return "oGUI_CONTROL_BUTTON";
		case oGUI_CONTROL_CHECKBOX: return "oGUI_CONTROL_CHECKBOX";
		case oGUI_CONTROL_RADIOBUTTON: return "oGUI_CONTROL_RADIOBUTTON";
		case oGUI_CONTROL_LABEL: return "oGUI_CONTROL_LABEL";
		case oGUI_CONTROL_LABEL_SELECTABLE: return "oGUI_CONTROL_LABEL_SELECTABLE";
		case oGUI_CONTROL_TEXTBOX: return "oGUI_CONTROL_TEXTBOX";
		case oGUI_CONTROL_COMBOBOX: return "oGUI_CONTROL_COMBOBOX";
		case oGUI_CONTROL_COMBOTEXTBOX: return "oGUI_CONTROL_COMBOTEXTBOX";
		case oGUI_CONTROL_PROGRESSBAR: return "oGUI_CONTROL_PROGRESSBAR";
		case oGUI_CONTROL_TAB: return "oGUI_CONTROL_TAB";
		oNODEFAULT;
	}
}

const char* oAsString(const oGUI_BORDER_STYLE& _Style)
{
	switch (_Style)
	{
		case oGUI_BORDER_SUNKEN: return "oGUI_BORDER_SUNKEN";
		case oGUI_BORDER_FLAT: return "oGUI_BORDER_FLAT";
		case oGUI_BORDER_RAISED: return "oGUI_BORDER_RAISED";
		oNODEFAULT;
	}
}

bool oFromString(oGUI_WINDOW_STYLE* _pStyle, const char* _StrSource)
{
	if (!_stricmp(_StrSource,"embedded"))
		*_pStyle = oGUI_WINDOW_EMBEDDED;
	if (!_stricmp(_StrSource,"borderless"))
		*_pStyle = oGUI_WINDOW_BORDERLESS;
	else if (!_stricmp(_StrSource,"fixed"))
		*_pStyle = oGUI_WINDOW_FIXED;
	else if (!_stricmp(_StrSource,"dialog"))
		*_pStyle = oGUI_WINDOW_DIALOG;
	else
		*_pStyle = oGUI_WINDOW_SIZEABLE;
	return true;
}
