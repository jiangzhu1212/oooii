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
#include <oGUI/enum_radio_handler.h>
#include <oBase/assert.h>
#include <oCore/windows/win_error.h>

namespace ouro { namespace gui { namespace menu { 

void enum_radio_handler::add(menu_handle m, int first_item, int last_item, const callback_t& callback)
{
	entry_t e;
	e.handle = m;
	e.first = first_item;
	e.last = last_item;
	e.callback = callback;

	auto it = std::find_if(callbacks.begin(), callbacks.end(), [&](const entry_t& i)
	{
		return (e.handle == i.handle)
			|| (e.first >= i.first && e.first <= i.last) 
			|| (e.last >= i.first && e.last <= i.last);
	});

	if (it != callbacks.end())
		throw std::invalid_argument("The specified menu/range has already been registered or overlaps a previously registered range");

	callbacks.push_back(e);
}

void enum_radio_handler::on_action(const input::action& a)
{
	if (a.action_type == input::menu)
	{
		auto it = std::find_if(callbacks.begin(), callbacks.end(), [&](const entry_t& e)
		{
			return (int)a.device_id >= e.first && (int)a.device_id <= e.last;
		});

		if (it != callbacks.end())
		{
			check_radio(it->handle, it->first, it->last, a.device_id);
			it->callback(a.device_id - it->first);
		}
	}
}

}}}