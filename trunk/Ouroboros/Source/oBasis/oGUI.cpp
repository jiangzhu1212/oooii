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
#include <oBasis/oGUI.h>

template<size_t EXPECTED_SIZE, size_t SIZE, typename T> bool oFromStringT(T* _pValue, const char* _StrSource, const char* _AsStrings[SIZE])
{
	static_assert(SIZE == EXPECTED_SIZE, "mismatched count");
	for (size_t i = 0; i < SIZE; i++)
	{
		if (!oStricmp(_StrSource, _AsStrings[i]))
		{
			*_pValue = (T)i;
			return true;
		}
	}
	return false;
}

const char* oAsString(oGUI_WINDOW_STATE _State)
{
	switch (_State)
	{
		case oGUI_WINDOW_NONEXISTANT: return "oGUI_WINDOW_NONEXISTANT";
		case oGUI_WINDOW_HIDDEN: return "oGUI_WINDOW_HIDDEN";
		case oGUI_WINDOW_MINIMIZED: return "oGUI_WINDOW_MINIMIZED";
		case oGUI_WINDOW_RESTORED: return "oGUI_WINDOW_RESTORED";
		case oGUI_WINDOW_MAXIMIZED: return "oGUI_WINDOW_MAXIMIZED";
		case oGUI_WINDOW_FULLSCREEN_COOPERATIVE: return "oGUI_WINDOW_FULLSCREEN_COOPERATIVE";
		case oGUI_WINDOW_FULLSCREEN_EXCLUSIVE: return "oGUI_WINDOW_FULLSCREEN_EXCLUSIVE";
		oNODEFAULT;
	}
}

const char* oAsString(oGUI_WINDOW_STYLE _Style)
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

const char* oAsString(oGUI_EVENT _Event)
{
	switch (_Event)
	{
		case oGUI_IDLE: return "oGUI_IDLE";
		case oGUI_ACTIVATED: return "oGUI_ACTIVATED";
		case oGUI_DEACTIVATED: return "oGUI_DEACTIVATED";
		case oGUI_CREATING: return "oGUI_CREATING";
		case oGUI_PAINT: return "oGUI_PAINT";
		case oGUI_DISPLAY_CHANGED: return "oGUI_DISPLAY_CHANGED";
		case oGUI_SETCURSOR: return "oGUI_SETCURSOR";
		case oGUI_MOVING: return "oGUI_MOVING";
		case oGUI_MOVED: return "oGUI_MOVED";
		case oGUI_SIZING: return "oGUI_SIZING";
		case oGUI_SIZED: return "oGUI_SIZED";
		case oGUI_CLOSING: return "oGUI_CLOSING";
		case oGUI_CLOSED: return "oGUI_CLOSED";
		case oGUI_TO_FULLSCREEN: return "oGUI_TO_FULLSCREEN";
		case oGUI_FROM_FULLSCREEN: return "oGUI_FROM_FULLSCREEN";
		case oGUI_LOST_CAPTURE: return "oGUI_LOST_CAPTURE";
		oNODEFAULT;
	}
}

const char* oAsString(oGUI_ACTION _Action)
{
	switch (_Action)
	{
		case oGUI_ACTION_UNKNOWN: return "oGUI_ACTION_UNKNOWN";
		case oGUI_ACTION_MENU: return "oGUI_ACTION_MENU";
		case oGUI_ACTION_CONTROL_ACTIVATED: return "oGUI_ACTION_CONTROL_ACTIVATED";
		case oGUI_ACTION_CONTROL_SELECTION_CHANGING: return "oGUI_ACTION_CONTROL_SELECTION_CHANGING";
		case oGUI_ACTION_CONTROL_SELECTION_CHANGED: return "oGUI_ACTION_CONTROL_SELECTION_CHANGED";
		case oGUI_ACTION_HOTKEY: return "oGUI_ACTION_HOTKEY";
		case oGUI_ACTION_KEY_DOWN: return "oGUI_ACTION_KEY_DOWN";
		case oGUI_ACTION_KEY_UP: return "oGUI_ACTION_KEY_UP";
		case oGUI_ACTION_POINTER_MOVE: return "oGUI_ACTION_POINTER_MOVE";
		oNODEFAULT;
	}
}

const char* oAsString(oGUI_CURSOR_STATE _State)
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

const char* oAsString(oGUI_ALIGNMENT _Alignment)
{
	switch (_Alignment)
	{
		case oGUI_ALIGNMENT_TOP_LEFT: return "TopLeft";
		case oGUI_ALIGNMENT_TOP_CENTER: return "TopCenter";
		case oGUI_ALIGNMENT_TOP_RIGHT: return "TopRight";
		case oGUI_ALIGNMENT_MIDDLE_LEFT: return "MiddleLeft";
		case oGUI_ALIGNMENT_MIDDLE_CENTER: return "MiddleCenter";
		case oGUI_ALIGNMENT_MIDDLE_RIGHT: return "MiddleRight";
		case oGUI_ALIGNMENT_BOTTOM_LEFT: return "BottomLeft";
		case oGUI_ALIGNMENT_BOTTOM_CENTER: return "BottomCenter";
		case oGUI_ALIGNMENT_BOTTOM_RIGHT: return "BottomRight";
		case oGUI_ALIGNMENT_FIT_PARENT: return "FitParent";
		oNODEFAULT;
	}
}

bool oFromString(oGUI_ALIGNMENT* _pAlignment, const char* _StrSource)
{
	static const char* sAlignmentStrings[] = 
	{
		"TopLeft",
		"TopMiddle",
		"TopRight",
		"MiddleLeft",
		"MiddleCenter",
		"MiddleRight",
		"BottomLeft",
		"BottomCenter",
		"BottomRight",
		"FitParent",
	};

	return oFromStringT<10, oCOUNTOF(sAlignmentStrings)>(_pAlignment, _StrSource, sAlignmentStrings);
}

const char* oAsString(oGUI_CONTROL_TYPE _Type)
{
	switch (_Type)
	{
		case oGUI_CONTROL_UNKNOWN: return "oGUI_CONTROL_UNKNOWN";
		case oGUI_CONTROL_GROUPBOX: return "oGUI_CONTROL_GROUPBOX";
		case oGUI_CONTROL_BUTTON: return "oGUI_CONTROL_BUTTON";
		case oGUI_CONTROL_CHECKBOX: return "oGUI_CONTROL_CHECKBOX";
		case oGUI_CONTROL_RADIOBUTTON: return "oGUI_CONTROL_RADIOBUTTON";
		case oGUI_CONTROL_LABEL: return "oGUI_CONTROL_LABEL";
		case oGUI_CONTROL_HYPERLABEL: return "oGUI_CONTROL_HYPERLABEL";
		case oGUI_CONTROL_LABEL_SELECTABLE: return "oGUI_CONTROL_LABEL_SELECTABLE";
		case oGUI_CONTROL_ICON: return "oGUI_CONTROL_ICON";
		case oGUI_CONTROL_TEXTBOX: return "oGUI_CONTROL_TEXTBOX";
		case oGUI_CONTROL_TEXTBOX_SCROLLABLE: return "oGUI_CONTROL_TEXTBOX_SCROLLABLE";
		case oGUI_CONTROL_FLOATBOX: return "oGUI_CONTROL_FLOATBOX";
		case oGUI_CONTROL_FLOATBOX_SPINNER: return "oGUI_CONTROL_FLOATBOX_SPINNER";
		case oGUI_CONTROL_COMBOBOX: return "oGUI_CONTROL_COMBOBOX";
		case oGUI_CONTROL_COMBOTEXTBOX: return "oGUI_CONTROL_COMBOTEXTBOX";
		case oGUI_CONTROL_PROGRESSBAR: return "oGUI_CONTROL_PROGRESSBAR";
		case oGUI_CONTROL_PROGRESSBAR_UNKNOWN: return "oGUI_CONTROL_PROGRESSBAR_UNKNOWN";
		case oGUI_CONTROL_TAB: return "oGUI_CONTROL_TAB";
		case oGUI_CONTROL_SLIDER: return "oGUI_CONTROL_SLIDER";
		case oGUI_CONTROL_SLIDER_SELECTABLE: return "oGUI_CONTROL_SLIDER_SELECTABLE";
		case oGUI_CONTROL_SLIDER_WITH_TICKS: return "oGUI_CONTROL_SLIDER_WITH_TICKS";
		oNODEFAULT;
	}
}

bool oFromString(oGUI_CONTROL_TYPE* _pType, const char* _StrSource)
{
	static const char* sControlTypeStrings[] = 
	{
		"Unknown",
		"Group",
		"Button",
		"Checkbox",
		"Radio",
		"Label",
		"Hyperlabel",
		"SelectableLabel",
		"Icon",
		"Textbox",
		"TextBoxScrollable",
		"Floatbox",
		"FloatboxSpinner",
		"Combobox",
		"ComboTextbox",
		"ProgressBar",
		"ProgressBarUnknown",
		"Tab",
		"Slider",
		"SelectableSlider",
		"SliderWithTicks",
	};

	return oFromStringT<oGUI_NUM_CONTROLS, oCOUNTOF(sControlTypeStrings)>(_pType, _StrSource, sControlTypeStrings);
}

const char* oAsString(oGUI_BORDER_STYLE _Style)
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
	*_pStyle = oGUI_WINDOW_SIZEABLE;

	static const char* sStrings[] = { "embedded", "borderless", "fixed", "dialog" };
	for (int i = 0; i < oCOUNTOF(sStrings); i++)
	{
		if (!oStricmp(_StrSource, sStrings[i]))
		{
			*_pStyle = (oGUI_WINDOW_STYLE)i;
			return true;
		}
	}

	// default to sizeable
	return true;
}

const oGUID& oGetGUID(const threadsafe oGUI_WINDOW* const threadsafe*)
{
	// {A68BBF18-4781-4FA1-87FD-5B6E2250441D}
	static const oGUID guid = { 0xa68bbf18, 0x4781, 0x4fa1, { 0x87, 0xfd, 0x5b, 0x6e, 0x22, 0x50, 0x44, 0x1d } };
	return guid;
}

const oGUID& oGetGUID(const threadsafe oGUI_MENU* const threadsafe*)
{
	// {913C9CA3-BF07-41DF-931A-57E974497FD5}
	static const oGUID guid = { 0x913c9ca3, 0xbf07, 0x41df, { 0x93, 0x1a, 0x57, 0xe9, 0x74, 0x49, 0x7f, 0xd5 } };
	return guid;
}

const oGUID& oGetGUID(const threadsafe oGUI_STATUSBAR* const threadsafe*)
{
	// {6D169E76-5A6F-4A03-B7CB-232B2AE92D2C}
	static const oGUID guid = { 0x6d169e76, 0x5a6f, 0x4a03, { 0xb7, 0xcb, 0x23, 0x2b, 0x2a, 0xe9, 0x2d, 0x2c } };
	return guid;
}

void oGUIRecordInputState(const oGUI_ACTION_DESC& _Action, const oKEYBOARD_KEY* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition, bool _ForceLowerCase)
{
	oASSERT((_NumKeys % _NumKeyStates) == 0, "NumKeyStates must be a multiple of num keys");

	bool KeyDown = false;
	switch (_Action.Action)
	{
		case oGUI_ACTION_KEY_DOWN:
		{
			KeyDown = true;
			break;
		}

		case oGUI_ACTION_KEY_UP:
		{
			KeyDown = false;
			break;
		}

		case oGUI_ACTION_POINTER_MOVE:
			*_pPointerPosition = _Action.PointerPosition;
			return;

		default:
			return;
	}

	oKEYBOARD_KEY TestKey = _Action.Key;
	if (_ForceLowerCase && TestKey >= oKB_A && TestKey <= oKB_Z)
		TestKey = oKEYBOARD_KEY(TestKey - oKB_A + oKB_a);

	for (size_t i = 0; i < _NumKeys; i++)
	{
		if (TestKey == _pKeys[i])
		{
			_pKeyStates[(i % _NumKeyStates)] = KeyDown;
			break;
		}
	}
}
