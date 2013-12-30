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
#include <oGUI/mru.h>
#include <oGUI/oGUIMenu.h>
#include <oCore/windows/win_registry.h>

using namespace ouro::windows;

namespace ouro {
	namespace gui {

mru::mru(const info& _Info)
	: Info(_Info)
	, NumMRUs(_Info.last_id-_Info.first_id+1)
{
	if (_Info.prefix.empty())
		oTHROW_INVARG("A prefix must be specified");
	if (0 > snprintf(KeyFormat, "%s%%02d", _Info.prefix))
		oTHROW_INVARG("the prefix is too long");
}

sstring mru::entry_name(int _Index)
{
	sstring n;
	snprintf(n, "MRU%02d", _Index);
	return n;
}

void mru::get_entries(std::vector<uri_string>& _Entries)
{
	uri_string Entry;
	_Entries.clear();
	_Entries.reserve(NumMRUs);
	for (int i = 0; i < NumMRUs; i++)
	{
		sstring EntryName = entry_name(i);
		registry::get(Entry, registry::current_user, Info.registry_key, EntryName);
		_Entries.push_back(Entry);
	}
}

void mru::add(const char* _Entry)
{
	sstring EntryName;
	std::vector<uri_string> Entries;
	get_entries(Entries);

	// Remove any duplicates of the incoming URI
	auto it = find_if(Entries, [&](const uri_string& x)->bool { return !_stricmp(x, _Entry); });
	while (it != Entries.end())
	{
		Entries.erase(it);
		it = find_if(Entries, [&](const uri_string& x)->bool { return !_stricmp(x, _Entry); });
	}

	// Insert this as the front of the list
	Entries.insert(Entries.begin(), _Entry);

	// Write the list back out
	oCHECK_SIZE(int, Entries.size());
	int i = 0;
	for (; i < static_cast<int>(Entries.size()); i++)
		registry::set(registry::current_user, Info.registry_key, entry_name(i), Entries[i]);

	// and delete any extra stale entries that remain as a result of the Entries
	// list getting shorter
	const int NumIDs = Info.last_id-Info.first_id+1;
	for (; i < NumMRUs; i++)
		registry::delete_value(registry::current_user, Info.registry_key, entry_name(i));

	refresh();
}

char* mru::get(char* _StrDestination, size_t _SizeofStrDestination, int _ID)
{
	if (_ID >= Info.first_id && _ID <= Info.last_id)
		return registry::get(_StrDestination, _SizeofStrDestination, registry::current_user, Info.registry_key, entry_name(_ID - Info.first_id));
	return nullptr;
}

void mru::refresh()
{
	oGUIMenuRemoveAllItems(Info.menu);
	uri_string Entry;
	for (int i = 0; i < NumMRUs; i++)
	{
		registry::get(Entry, registry::current_user, Info.registry_key, entry_name(i));
		oGUIMenuAppendItem(Info.menu, Info.first_id + i, Entry);
	}
}

	} // namespace gui
} // namespace ouro
