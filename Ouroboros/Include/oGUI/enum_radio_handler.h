// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
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
