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
#include <oBasis/oGUI.h>
#include <oCompute/linear_algebra.h>
#include <oBasis/oInterface.h>
#include <oBasis/oRTTI.h>

namespace ouro {

const char* as_string(const input_device_type::value& _InputDeviceType)
{
	switch (_InputDeviceType)
	{
		case input_device_type::unknown: return "unknown";
		case input_device_type::keyboard: return "keyboard";
		case input_device_type::mouse: return "mouse";
		case input_device_type::joystick: return "joystick";
		case input_device_type::control: return "control";
		case input_device_type::skeleton: return "skeleton";
		case input_device_type::voice: return "voice";
		case input_device_type::touch: return "touch";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input_device_type::value);
oDEFINE_FROM_STRING(input_device_type::value, input_device_type::count);

const char* as_string(const input_device_status::value& _InputDeviceStatus)
{
	switch (_InputDeviceStatus)
	{
		case input_device_status::ready: return "ready";
		case input_device_status::initializing: return "initializing";
		case input_device_status::not_connected: return "not_connected";
		case input_device_status::is_clone: return "is_clone";
		case input_device_status::not_supported: return "not_supported";
		case input_device_status::insufficient_bandwidth: return "insufficient_bandwidth";
		case input_device_status::low_power: return "low_power";
		case input_device_status::not_powered: return "not_powered";
		case input_device_status::not_ready: return "not_ready";
		default: break;
	}
	return "?";
}
	
oDEFINE_TO_STRING(input_device_status::value);
oDEFINE_FROM_STRING(input_device_status::value, input_device_status::count);

const char* as_string(const input_key::value& _InputKey)
{
	switch (_InputKey)
	{
		case input_key::none: return "none";
		case input_key::mouse_left: return "mouse_left";
		case input_key::mouse_right: return "mouse_right";
		case input_key::mouse_middle: return "mouse_middle";
		case input_key::mouse_side1: return "mouse_side1";
		case input_key::mouse_side2: return "mouse_side2";
		case input_key::mouse_left_double: return "mouse_left_double";
		case input_key::mouse_right_double: return "mouse_right_double";
		case input_key::mouse_middle_double: return "mouse_middle_double";
		case input_key::mouse_side1_double: return "mouse_side1_double";
		case input_key::mouse_side2_double: return "mouse_side2_double";
		case input_key::joy_lleft: return "joy_lleft";
		case input_key::joy_lup: return "joy_lup";
		case input_key::joy_lright: return "joy_lright";
		case input_key::joy_ldown: return "joy_ldown";
		case input_key::joy_rleft: return "joy_rleft";
		case input_key::joy_rup: return "joy_rup";
		case input_key::joy_rright: return "joy_rright";
		case input_key::joy_rdown: return "joy_rdown";
		case input_key::joy_lshoulder1: return "joy_lshoulder1";
		case input_key::joy_lshoulder2: return "joy_lshoulder2";
		case input_key::joy_rshoulder1: return "joy_rshoulder1";
		case input_key::joy_rshoulder2: return "joy_rshoulder2";
		case input_key::joy_lthumb: return "joy_lthumb";
		case input_key::joy_rthumb: return "joy_rthumb";
		case input_key::joy_system: return "joy_system";
		case input_key::joy_start: return "joy_start";
		case input_key::joy_select: return "joy_select";
		case input_key::lctrl: return "lctrl";
		case input_key::rctrl: return "rctrl";
		case input_key::lalt: return "lalt";
		case input_key::ralt: return "ralt";
		case input_key::lshift: return "lshift";
		case input_key::rshift: return "rshift";
		case input_key::lwin: return "lwin";
		case input_key::rwin: return "rwin";
		case input_key::app_cycle: return "app_cycle";
		case input_key::app_context: return "app_context";
		case input_key::capslock: return "capslock";
		case input_key::scrolllock: return "scrolllock";
		case input_key::numlock: return "numlock";
		case input_key::space: return "space";
		case input_key::backtick: return "backtick";
		case input_key::dash: return "dash";
		case input_key::equal_: return "equal";
		case input_key::lbracket: return "lbracket";
		case input_key::rbracket: return "rbracket";
		case input_key::backslash: return "backslash";
		case input_key::semicolon: return "semicolon";
		case input_key::apostrophe: return "apostrophe";
		case input_key::comma: return "comma";
		case input_key::period: return "period";
		case input_key::slash: return "slash";
		case input_key::_0: return "0";
		case input_key::_1: return "1";
		case input_key::_2: return "2";
		case input_key::_3: return "3";
		case input_key::_4: return "4";
		case input_key::_5: return "5";
		case input_key::_6: return "6";
		case input_key::_7: return "7";
		case input_key::_8: return "8";
		case input_key::_9: return "9";
		case input_key::a: return "a";
		case input_key::b: return "b";
		case input_key::c: return "c";
		case input_key::d: return "d";
		case input_key::e: return "e";
		case input_key::f: return "f";
		case input_key::g: return "g";
		case input_key::h: return "h";
		case input_key::i: return "i";
		case input_key::j: return "j";
		case input_key::k: return "k";
		case input_key::l: return "l";
		case input_key::m: return "m";
		case input_key::n: return "n";
		case input_key::o: return "o";
		case input_key::p: return "p";
		case input_key::q: return "q";
		case input_key::r: return "r";
		case input_key::s: return "s";
		case input_key::t: return "t";
		case input_key::u: return "u";
		case input_key::v: return "v";
		case input_key::w: return "w";
		case input_key::x: return "x";
		case input_key::y: return "y";
		case input_key::z: return "z";
		case input_key::num0: return "num0";
		case input_key::num1: return "num1";
		case input_key::num2: return "num2";
		case input_key::num3: return "num3";
		case input_key::num4: return "num4";
		case input_key::num5: return "num5";
		case input_key::num6: return "num6";
		case input_key::num7: return "num7";
		case input_key::num8: return "num8";
		case input_key::num9: return "num9";
		case input_key::nummul: return "nummul";
		case input_key::numadd: return "numadd";
		case input_key::numsub: return "numsub";
		case input_key::numdecimal: return "numdecimal";
		case input_key::numdiv: return "numdiv";
		case input_key::esc: return "esc";
		case input_key::backspace: return "backspace";
		case input_key::tab: return "tab";
		case input_key::enter: return "enter";
		case input_key::ins: return "ins";
		case input_key::del: return "del";
		case input_key::home: return "home";
		case input_key::end: return "end";
		case input_key::pgup: return "pgup";
		case input_key::pgdn: return "pgdn";
		case input_key::f1: return "f1";
		case input_key::f2: return "f2";
		case input_key::f3: return "f3";
		case input_key::f4: return "f4";
		case input_key::f5: return "f5";
		case input_key::f6: return "f6";
		case input_key::f7: return "f7";
		case input_key::f8: return "f8";
		case input_key::f9: return "f9";
		case input_key::f10: return "f10";
		case input_key::f11: return "f11";
		case input_key::f12: return "f12";
		case input_key::f13: return "f13";
		case input_key::f14: return "f14";
		case input_key::f15: return "f15";
		case input_key::f16: return "f16";
		case input_key::f17: return "f17";
		case input_key::f18: return "f18";
		case input_key::f19: return "f19";
		case input_key::f20: return "f20";
		case input_key::f21: return "f21";
		case input_key::f22: return "f22";
		case input_key::f23: return "f23";
		case input_key::f24: return "f24";
		case input_key::pause: return "pause";
		case input_key::sleep: return "sleep";
		case input_key::printscreen: return "printscreen";
		case input_key::left: return "left";
		case input_key::up: return "up";
		case input_key::right: return "right";
		case input_key::down: return "down";
		case input_key::mail: return "mail";
		case input_key::back: return "back";
		case input_key::forward: return "forward";
		case input_key::refresh: return "refresh";
		case input_key::stop: return "stop";
		case input_key::search: return "search";
		case input_key::favs: return "favs";
		case input_key::media: return "media";
		case input_key::mute: return "mute";
		case input_key::volup: return "volup";
		case input_key::voldn: return "voldn";
		case input_key::prev_track: return "prev_track";
		case input_key::next_track: return "next_track";
		case input_key::stop_track: return "stop_track";
		case input_key::play_pause_track: return "play_pause_track";
		case input_key::app1: return "app1";
		case input_key::app2: return "app2";
		case input_key::touch1: return "touch1";
		case input_key::touch2: return "touch2";
		case input_key::touch3: return "touch3";
		case input_key::touch4: return "touch4";
		case input_key::touch5: return "touch5";
		case input_key::touch6: return "touch6";
		case input_key::touch7: return "touch7";
		case input_key::touch8: return "touch8";
		case input_key::touch9: return "touch9";
		case input_key::touch10: return "touch10";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input_key::value);
oDEFINE_FROM_STRING(input_key::value, input_key::count);

const char* as_string(const skeleton_bone::value& _Bone)
{
	switch (_Bone)
	{
		case skeleton_bone::hip_center: return "hip_center";
		case skeleton_bone::spine: return "spine";
		case skeleton_bone::shoulder_center: return "shoulder_center";
		case skeleton_bone::head: return "head";
		case skeleton_bone::shoulder_left: return "shoulder_left";
		case skeleton_bone::elbow_left: return "elbow_left";
		case skeleton_bone::wrist_left: return "wrist_left";
		case skeleton_bone::hand_left: return "hand_left";
		case skeleton_bone::shoulder_right: return "shoulder_right";
		case skeleton_bone::elbow_right: return "elbow_right";
		case skeleton_bone::wrist_right: return "wrist_right";
		case skeleton_bone::hand_right: return "hand_right";
		case skeleton_bone::hip_left: return "hip_left";
		case skeleton_bone::knee_left: return "knee_left";
		case skeleton_bone::ankle_left: return "ankle_left";
		case skeleton_bone::foot_left: return "foot_left";
		case skeleton_bone::hip_right: return "hip_right";
		case skeleton_bone::knee_right: return "knee_right";
		case skeleton_bone::ankle_right: return "ankle_right";
		case skeleton_bone::foot_right: return "foot_right";
		case skeleton_bone::invalid: return "invalid";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(skeleton_bone::value);
oDEFINE_FROM_STRING2(skeleton_bone::value, skeleton_bone::count, skeleton_bone::invalid);

const char* as_string(const alignment::value& _Alignment)
{
	switch (_Alignment)
	{
		case alignment::top_left: return "top_left";
		case alignment::top_center: return "top_center";
		case alignment::top_right: return "top_right";
		case alignment::middle_left: return "middle_left";
		case alignment::middle_center: return "middle_center";
		case alignment::middle_right: return "middle_right";
		case alignment::bottom_left: return "bottom_left";
		case alignment::bottom_center: return "bottom_center";
		case alignment::bottom_right: return "bottom_right";
		case alignment::fit_parent: return "fit_parent";
		case alignment::fit_largest_axis: return "fit_largest_axis";
		case alignment::fit_smallest_axis: return "fit_smallest_axis";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(alignment::value);
oDEFINE_FROM_STRING(alignment::value, alignment::count);

const char* as_string(const window_state::value& _State)
{
	switch (_State)
	{
		case window_state::invalid: return "invalid";
		case window_state::hidden: return "hidden";
		case window_state::minimized: return "minimized";
		case window_state::restored: return "restored";
		case window_state::maximized: return "maximized"; 
		case window_state::fullscreen: return "fullscreen";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(window_state::value);
oDEFINE_FROM_STRING(window_state::value, window_state::count);

const char* as_string(const window_style::value& _Style)
{
	switch (_Style)
	{
		case window_style::default_style: return "default_style";
		case window_style::borderless: return "borderless";
		case window_style::dialog: return "dialog";
		case window_style::fixed: return "fixed";
		case window_style::fixed_with_menu: return "fixed_with_menu";
		case window_style::fixed_with_statusbar: return "fixed_with_statusbar";
		case window_style::fixed_with_menu_and_statusbar: return "fixed_with_menu_and_statusbar";
		case window_style::sizable: return "sizable";
		case window_style::sizable_with_menu: return "sizable_with_menu";
		case window_style::sizable_with_statusbar: return "sizable_with_statusbar";
		case window_style::sizable_with_menu_and_statusbar: return "sizable_with_menu_and_statusbar";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(window_style::value);
oDEFINE_FROM_STRING(window_style::value, window_style::count);

const char* as_string(const window_sort_order::value& _SortOrder)
{
	switch (_SortOrder)
	{
		case window_sort_order::sorted: return "sorted";
		case window_sort_order::always_on_top: return "always_on_top";
		case window_sort_order::always_on_top_with_focus: return "always_on_top_with_focus";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(window_sort_order::value);
oDEFINE_FROM_STRING(window_sort_order::value, window_sort_order::count);

const char* as_string(const cursor_state::value& _CursorState)
{
	switch (_CursorState)
	{
		case cursor_state::none: return "none";
		case cursor_state::arrow: return "arrow";
		case cursor_state::hand: return "hand";
		case cursor_state::help: return "help";
		case cursor_state::not_allowed: return "not_allowed";
		case cursor_state::wait_foreground: return "wait_foreground";
		case cursor_state::wait_background: return "wait_background";
		case cursor_state::user: return "user";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(cursor_state::value);
oDEFINE_FROM_STRING(cursor_state::value, cursor_state::count);

const char* as_string(const control_type::value& _Control)
{
	switch (_Control)
	{
		case control_type::unknown: return "unknown";
		case control_type::groupbox: return "groupbox";
		case control_type::button: return "button";
		case control_type::checkbox: return "checkbox";
		case control_type::radiobutton: return "radiobutton";
		case control_type::label: return "label";
		case control_type::label_centered: return "label_centered";
		case control_type::hyperlabel: return "hyperlabel"; 
		case control_type::label_selectable: return "label_selectable";
		case control_type::icon: return "icon";
		case control_type::textbox: return "textbox";
		case control_type::textbox_scrollable: return "textbox_scrollable";
		case control_type::floatbox: return "floatbox"; 
		case control_type::floatbox_spinner: return "floatbox_spinner";
		case control_type::combobox: return "combobox"; 
		case control_type::combotextbox: return "combotextbox"; 
		case control_type::progressbar: return "progressbar";
		case control_type::progressbar_unknown: return "progressbar_unknown"; 
		case control_type::tab: return "tab"; 
		case control_type::slider: return "slider";
		case control_type::slider_selectable: return "slider_selectable"; 
		case control_type::slider_with_ticks: return "slider_with_ticks"; 
		case control_type::listbox: return "listbox"; 
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(control_type::value);
oDEFINE_FROM_STRING(control_type::value, control_type::count);

const char* as_string(const gui_event::value& _Event)
{
	switch (_Event)
	{
		case gui_event::timer: return "timer";
		case gui_event::activated: return "activated";
		case gui_event::deactivated: return "deactivated";
		case gui_event::creating: return "creating";
		case gui_event::paint: return "paint";
		case gui_event::display_changed: return "display_changed";
		case gui_event::moving: return "moving";
		case gui_event::moved: return "moved";
		case gui_event::sizing: return "sizing";
		case gui_event::sized: return "sized";
		case gui_event::closing: return "closing";
		case gui_event::closed: return "closed";
		case gui_event::to_fullscreen: return "to_fullscreen";
		case gui_event::from_fullscreen: return "from_fullscreen";
		case gui_event::lost_capture: return "lost_capture";
		case gui_event::drop_files: return "drop_files";
		case gui_event::input_device_changed: return "input_device_changed";
		case gui_event::custom_event: return "custom_event";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gui_event::value);
oDEFINE_FROM_STRING(gui_event::value, gui_event::count);

const char* as_string(const gui_action::value& _Action)
{
	switch (_Action)
	{
		case gui_action::unknown: return "unknown";
		case gui_action::menu: return "menu";
		case gui_action::control_activated: return "control_activated";
		case gui_action::control_deactivated: return "control_deactivated";
		case gui_action::control_selection_changing: return "control_selection_changing";
		case gui_action::control_selection_changed: return "control_selection_changed";
		case gui_action::hotkey: return "hotkey";
		case gui_action::key_down: return "key_down";
		case gui_action::key_up: return "key_up";
		case gui_action::pointer_move: return "pointer_move";
		case gui_action::skeleton: return "skeleton";
		case gui_action::skeleton_acquired: return "skeleton_acquired";
		case gui_action::skeleton_lost: return "skeleton_lost";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(gui_action::value);
oDEFINE_FROM_STRING(gui_action::value, gui_action::count);

} // namespace ouro

void oGUIRecordInputState(const ouro::action_info& _Action, const ouro::input_key::value* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition)
{
	oASSERT((_NumKeys % _NumKeyStates) == 0, "NumKeyStates must be a multiple of num keys");

	bool KeyDown = false;
	switch (_Action.action)
	{
		case ouro::gui_action::key_down:
		{
			KeyDown = true;
			break;
		}

		case ouro::gui_action::key_up:
		{
			KeyDown = false;
			break;
		}

		case ouro::gui_action::pointer_move:
			*_pPointerPosition = _Action.position.xyz();
			return;

		default:
			return;
	}

	ouro::input_key::value TestKey = _Action.key;
	for (size_t i = 0; i < _NumKeys; i++)
	{
		if (TestKey == _pKeys[i])
		{
			_pKeyStates[(i % _NumKeyStates)] = KeyDown;
			break;
		}
	}
}

oRECT oGUIResolveRect(const oRECT& _Parent, const oRECT& _UnadjustedChild, ouro::alignment::value _Alignment, bool _Clip)
{
	int2 cpos = oGUIResolveRectPosition(_UnadjustedChild.Min);

	int2 psz = _Parent.size();
	int2 csz = oGUIResolveRectSize(_UnadjustedChild.Max - cpos, psz);

	float2 ResizeRatios = (float2)psz / max((float2)csz, float2(0.0001f, 0.0001f));

	ouro::alignment::value internalAlignment = _Alignment;

	int2 offset(0, 0);

	switch (_Alignment)
	{
		case ouro::alignment::fit_largest_axis:
		{
			const float ResizeRatio = min(ResizeRatios);
			csz = round((float2)csz * ResizeRatio);
			cpos = 0;
			internalAlignment = ouro::alignment::middle_center;
			break;
		}

		case ouro::alignment::fit_smallest_axis:
		{
			const float ResizeRatio = max(ResizeRatios);
			csz = round((float2)csz * ResizeRatio);
			cpos = 0;
			internalAlignment = ouro::alignment::middle_center;
			break;
		}

		case ouro::alignment::fit_parent:
			return _Parent;

		default:
			// preserve user-specified offset if there was one separately from moving 
			// around the child position according to internalAlignment
			offset = _UnadjustedChild.Min;
			break;
	}

	int2 code = int2(internalAlignment % 3, internalAlignment / 3);

	if (offset.x == oDEFAULT || code.x == 0) offset.x = 0;
	if (offset.y == oDEFAULT || code.y == 0) offset.y = 0;

	// All this stuff is top-left by default, so adjust for center/middle and 
	// right/bottom

	// center/middle
	if (code.x == 1) cpos.x = (psz.x - csz.x) / 2;
	if (code.y == 1) cpos.y = (psz.y - csz.y) / 2;

	// right/bottom
	if (code.x == 2) cpos.x = _Parent.Max.x - csz.x;
	if (code.y == 2) cpos.y = _Parent.Max.y - csz.y;

	int2 FinalOffset = _Parent.Min + offset;

	oRECT resolved;
	resolved.Min = cpos;
	resolved.Max = resolved.Min + csz;

	resolved.Min += FinalOffset;
	resolved.Max += FinalOffset;

	if (_Clip)
		resolved = oGUIClipRect(_Parent, resolved);

	return resolved;
}
