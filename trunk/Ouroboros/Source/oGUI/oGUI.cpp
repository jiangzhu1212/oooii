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
#include <oGUI/oGUI.h>
#include <oCompute/linear_algebra.h>

namespace ouro {

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
		case control_type::group: return "group";
		case control_type::button: return "button";
		case control_type::checkbox: return "checkbox";
		case control_type::radio: return "radio";
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

const char* as_string(const event_type::value& _Event)
{
	switch (_Event)
	{
		case event_type::timer: return "timer";
		case event_type::activated: return "activated";
		case event_type::deactivated: return "deactivated";
		case event_type::creating: return "creating";
		case event_type::paint: return "paint";
		case event_type::display_changed: return "display_changed";
		case event_type::moving: return "moving";
		case event_type::moved: return "moved";
		case event_type::sizing: return "sizing";
		case event_type::sized: return "sized";
		case event_type::closing: return "closing";
		case event_type::closed: return "closed";
		case event_type::to_fullscreen: return "to_fullscreen";
		case event_type::from_fullscreen: return "from_fullscreen";
		case event_type::lost_capture: return "lost_capture";
		case event_type::drop_files: return "drop_files";
		case event_type::input_device_changed: return "input_device_changed";
		case event_type::custom_event: return "custom_event";
		default: break;
	}
	return "?";
}

oDEFINE_TO_STRING(event_type::value);
oDEFINE_FROM_STRING(event_type::value, event_type::count);

rect resolve_rect(const rect& _Parent, const rect& _UnadjustedChild, alignment::value _Alignment, bool _Clip)
{
	int2 cpos = resolve_rect_position(_UnadjustedChild.Min);

	int2 psz = _Parent.size();
	int2 csz = resolve_rect_size(_UnadjustedChild.Max - cpos, psz);

	float2 ResizeRatios = (float2)psz / max((float2)csz, float2(0.0001f, 0.0001f));

	alignment::value internalAlignment = _Alignment;

	int2 offset(0, 0);

	switch (_Alignment)
	{
		case alignment::fit_largest_axis:
		{
			const float ResizeRatio = min(ResizeRatios);
			csz = round((float2)csz * ResizeRatio);
			cpos = 0;
			internalAlignment = alignment::middle_center;
			break;
		}

		case alignment::fit_smallest_axis:
		{
			const float ResizeRatio = max(ResizeRatios);
			csz = round((float2)csz * ResizeRatio);
			cpos = 0;
			internalAlignment = alignment::middle_center;
			break;
		}

		case alignment::fit_parent:
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

	rect resolved;
	resolved.Min = cpos;
	resolved.Max = resolved.Min + csz;

	resolved.Min += FinalOffset;
	resolved.Max += FinalOffset;

	if (_Clip)
		resolved = clip_rect(_Parent, resolved);

	return resolved;
}

} // namespace ouro

