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
// Helper for the common case of wanting a radio-select list based on an
// enum (or partial range within a larger enum). This object can register a 
// callback for several ranges. Call its on_action in a default case since
// it's a PITA to register all enums in the range as cases that redirect to
// the same function. The on_action will route the in-range value to the 
// registered callback.
#pragma once
#ifndef oGUI_enum_radio_handler_h
#define oGUI_enum_radio_handler_h

#include <oGUI/menu.h>
#include <vector>

namespace ouro { namespace gui { namespace menu {

class enum_radio_handler
{
public:
	// rebased_item = (inputvalue - first_item), so the first item will
	// have a rebased value of 0.
	typedef std::function<void(int rebased_item)> callback_t;

	void add(menu_handle m, int first_item, int last_item, const callback_t& callback);
	
	// in an on_action handler:
	/*	switch (action.action_type)
			{
				// radio groups are a pain to list as separate cases, so use
				// this handler to pick them all up as a default handler.
				case input::menu:
				{
					case MY_APP_MENUITEM1:
						[...]
					case MY_APP_MENUITEM2:
						[...]
					default:
						my_enum_radio_handler.on_action(a);
					break;
				}
			}
	*/
	void on_action(const input::action& a);

private:
	struct entry_t
	{
		menu_handle handle;
		int first;
		int last;
		callback_t callback;
	};

	std::vector<entry_t> callbacks;
};

}}}

#endif
