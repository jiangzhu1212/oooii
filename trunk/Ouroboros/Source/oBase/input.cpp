/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/input.h>
#include <oBase/assert.h>
#include <oBase/stringize.h>

namespace ouro {

const char* as_string(const input::type& _Type)
{
	switch (_Type)
	{
		case input::unknown: return "unknown";
		case input::keyboard: return "keyboard";
		case input::mouse: return "mouse";
		case input::joystick: return "joystick";
		case input::control: return "control";
		case input::skeleton: return "skeleton";
		case input::voice: return "voice";
		case input::touch: return "touch";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input::type);
oDEFINE_FROM_STRING(input::type, input::type_count);

const char* as_string(const input::status& _Status)
{
	switch (_Status)
	{
		case input::ready: return "ready";
		case input::initializing: return "initializing";
		case input::not_connected: return "not_connected";
		case input::is_clone: return "is_clone";
		case input::not_supported: return "not_supported";
		case input::insufficient_bandwidth: return "insufficient_bandwidth";
		case input::low_power: return "low_power";
		case input::not_powered: return "not_powered";
		case input::not_ready: return "not_ready";
		default: break;
	}
	return "?";
}
	
oDEFINE_TO_STRING(input::status);
oDEFINE_FROM_STRING(input::status, input::status_count);

const char* as_string(const input::key& _Key)
{
	switch (_Key)
	{
		case input::none: return "none";
		case input::mouse_left: return "mouse_left";
		case input::mouse_right: return "mouse_right";
		case input::mouse_middle: return "mouse_middle";
		case input::mouse_side1: return "mouse_side1";
		case input::mouse_side2: return "mouse_side2";
		case input::mouse_left_double: return "mouse_left_double";
		case input::mouse_right_double: return "mouse_right_double";
		case input::mouse_middle_double: return "mouse_middle_double";
		case input::mouse_side1_double: return "mouse_side1_double";
		case input::mouse_side2_double: return "mouse_side2_double";
		case input::joy_lleft: return "joy_lleft";
		case input::joy_lup: return "joy_lup";
		case input::joy_lright: return "joy_lright";
		case input::joy_ldown: return "joy_ldown";
		case input::joy_rleft: return "joy_rleft";
		case input::joy_rup: return "joy_rup";
		case input::joy_rright: return "joy_rright";
		case input::joy_rdown: return "joy_rdown";
		case input::joy_lshoulder1: return "joy_lshoulder1";
		case input::joy_lshoulder2: return "joy_lshoulder2";
		case input::joy_rshoulder1: return "joy_rshoulder1";
		case input::joy_rshoulder2: return "joy_rshoulder2";
		case input::joy_lthumb: return "joy_lthumb";
		case input::joy_rthumb: return "joy_rthumb";
		case input::joy_system: return "joy_system";
		case input::joy_start: return "joy_start";
		case input::joy_select: return "joy_select";
		case input::lctrl: return "lctrl";
		case input::rctrl: return "rctrl";
		case input::lalt: return "lalt";
		case input::ralt: return "ralt";
		case input::lshift: return "lshift";
		case input::rshift: return "rshift";
		case input::lwin: return "lwin";
		case input::rwin: return "rwin";
		case input::app_cycle: return "app_cycle";
		case input::app_context: return "app_context";
		case input::capslock: return "capslock";
		case input::scrolllock: return "scrolllock";
		case input::numlock: return "numlock";
		case input::space: return "space";
		case input::backtick: return "backtick";
		case input::dash: return "dash";
		case input::equal_: return "equal";
		case input::lbracket: return "lbracket";
		case input::rbracket: return "rbracket";
		case input::backslash: return "backslash";
		case input::semicolon: return "semicolon";
		case input::apostrophe: return "apostrophe";
		case input::comma: return "comma";
		case input::period: return "period";
		case input::slash: return "slash";
		case input::_0: return "0";
		case input::_1: return "1";
		case input::_2: return "2";
		case input::_3: return "3";
		case input::_4: return "4";
		case input::_5: return "5";
		case input::_6: return "6";
		case input::_7: return "7";
		case input::_8: return "8";
		case input::_9: return "9";
		case input::a: return "a";
		case input::b: return "b";
		case input::c: return "c";
		case input::d: return "d";
		case input::e: return "e";
		case input::f: return "f";
		case input::g: return "g";
		case input::h: return "h";
		case input::i: return "i";
		case input::j: return "j";
		case input::k: return "k";
		case input::l: return "l";
		case input::m: return "m";
		case input::n: return "n";
		case input::o: return "o";
		case input::p: return "p";
		case input::q: return "q";
		case input::r: return "r";
		case input::s: return "s";
		case input::t: return "t";
		case input::u: return "u";
		case input::v: return "v";
		case input::w: return "w";
		case input::x: return "x";
		case input::y: return "y";
		case input::z: return "z";
		case input::num0: return "num0";
		case input::num1: return "num1";
		case input::num2: return "num2";
		case input::num3: return "num3";
		case input::num4: return "num4";
		case input::num5: return "num5";
		case input::num6: return "num6";
		case input::num7: return "num7";
		case input::num8: return "num8";
		case input::num9: return "num9";
		case input::nummul: return "nummul";
		case input::numadd: return "numadd";
		case input::numsub: return "numsub";
		case input::numdecimal: return "numdecimal";
		case input::numdiv: return "numdiv";
		case input::esc: return "esc";
		case input::backspace: return "backspace";
		case input::tab: return "tab";
		case input::enter: return "enter";
		case input::ins: return "ins";
		case input::del: return "del";
		case input::home: return "home";
		case input::end: return "end";
		case input::pgup: return "pgup";
		case input::pgdn: return "pgdn";
		case input::f1: return "f1";
		case input::f2: return "f2";
		case input::f3: return "f3";
		case input::f4: return "f4";
		case input::f5: return "f5";
		case input::f6: return "f6";
		case input::f7: return "f7";
		case input::f8: return "f8";
		case input::f9: return "f9";
		case input::f10: return "f10";
		case input::f11: return "f11";
		case input::f12: return "f12";
		case input::f13: return "f13";
		case input::f14: return "f14";
		case input::f15: return "f15";
		case input::f16: return "f16";
		case input::f17: return "f17";
		case input::f18: return "f18";
		case input::f19: return "f19";
		case input::f20: return "f20";
		case input::f21: return "f21";
		case input::f22: return "f22";
		case input::f23: return "f23";
		case input::f24: return "f24";
		case input::pause: return "pause";
		case input::sleep: return "sleep";
		case input::printscreen: return "printscreen";
		case input::left: return "left";
		case input::up: return "up";
		case input::right: return "right";
		case input::down: return "down";
		case input::mail: return "mail";
		case input::back: return "back";
		case input::forward: return "forward";
		case input::refresh: return "refresh";
		case input::stop: return "stop";
		case input::search: return "search";
		case input::favs: return "favs";
		case input::media: return "media";
		case input::mute: return "mute";
		case input::volup: return "volup";
		case input::voldn: return "voldn";
		case input::prev_track: return "prev_track";
		case input::next_track: return "next_track";
		case input::stop_track: return "stop_track";
		case input::play_pause_track: return "play_pause_track";
		case input::app1: return "app1";
		case input::app2: return "app2";
		case input::touch1: return "touch1";
		case input::touch2: return "touch2";
		case input::touch3: return "touch3";
		case input::touch4: return "touch4";
		case input::touch5: return "touch5";
		case input::touch6: return "touch6";
		case input::touch7: return "touch7";
		case input::touch8: return "touch8";
		case input::touch9: return "touch9";
		case input::touch10: return "touch10";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input::key);
oDEFINE_FROM_STRING(input::key, input::key_count);

typedef resized_type<input::key, unsigned short> resized_key_t;
const char* as_string(const resized_key_t& _Key)
{
	return as_string((input::key)_Key);
}

oDEFINE_TO_STRING(resized_key_t);
oDEFINE_FROM_STRING(resized_key_t, input::key_count);

const char* as_string(const input::skeleton_bone& _Bone)
{
	switch (_Bone)
	{
		case input::hip_center: return "hip_center";
		case input::spine: return "spine";
		case input::shoulder_center: return "shoulder_center";
		case input::head: return "head";
		case input::shoulder_left: return "shoulder_left";
		case input::elbow_left: return "elbow_left";
		case input::wrist_left: return "wrist_left";
		case input::hand_left: return "hand_left";
		case input::shoulder_right: return "shoulder_right";
		case input::elbow_right: return "elbow_right";
		case input::wrist_right: return "wrist_right";
		case input::hand_right: return "hand_right";
		case input::hip_left: return "hip_left";
		case input::knee_left: return "knee_left";
		case input::ankle_left: return "ankle_left";
		case input::foot_left: return "foot_left";
		case input::hip_right: return "hip_right";
		case input::knee_right: return "knee_right";
		case input::ankle_right: return "ankle_right";
		case input::foot_right: return "foot_right";
		case input::invalid_bone: return "invalid_bone";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input::skeleton_bone);
oDEFINE_FROM_STRING2(input::skeleton_bone, input::bone_count, input::invalid_bone);

const char* as_string(const input::action_type& _ActionType)
{
	switch (_ActionType)
	{
		case input::unknown: return "unknown";
		case input::menu: return "menu";
		case input::control_activated: return "control_activated";
		case input::control_deactivated: return "control_deactivated";
		case input::control_selection_changing: return "control_selection_changing";
		case input::control_selection_changed: return "control_selection_changed";
		case input::hotkey: return "hotkey";
		case input::key_down: return "key_down";
		case input::key_up: return "key_up";
		case input::pointer_move: return "pointer_move";
		case input::skeleton_update: return "skeleton_update";
		case input::skeleton_acquired: return "skeleton_acquired";
		case input::skeleton_lost: return "skeleton_lost";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(input::action_type);
oDEFINE_FROM_STRING(input::action_type, input::action_type_count);

typedef resized_type<input::action_type, char> resized_action_type_t;
const char* as_string(const resized_action_type_t& _Key)
{
	return as_string((input::action_type)_Key);
}

oDEFINE_TO_STRING(resized_action_type_t);
oDEFINE_FROM_STRING(resized_action_type_t, input::action_type_count);

namespace input {

void record_state(const ouro::input::action& _Action, const ouro::input::key* _pKeys, size_t _NumKeys, bool* _pKeyStates, size_t _NumKeyStates, float3* _pPointerPosition)
{
	oASSERT((_NumKeys % _NumKeyStates) == 0, "NumKeyStates must be a multiple of num keys");

	bool KeyDown = false;
	switch (_Action.action_type)
	{
		case ouro::input::key_down:
		{
			KeyDown = true;
			break;
		}

		case ouro::input::key_up:
		{
			KeyDown = false;
			break;
		}

		case ouro::input::pointer_move:
			*_pPointerPosition = _Action.position().xyz();
			return;

		default:
			return;
	}

	ouro::input::key TestKey = _Action.key;
	for (size_t i = 0; i < _NumKeys; i++)
	{
		if (TestKey == _pKeys[i])
		{
			_pKeyStates[(i % _NumKeyStates)] = KeyDown;
			break;
		}
	}
}

	} // namespace input
}
