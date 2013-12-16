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
// Encapsulate the handling of a most-recently-used menu.
#pragma once
#ifndef oGUI_mru_h
#define oGUI_mru_h

#include <oBase/fixed_string.h>
#include <oBasis/oGUI.h>
#include <vector>

namespace ouro {
	namespace gui {

class mru
{
public:
	struct info
	{
		info()
			: menu(nullptr)
			, first_id(0)
			, last_id(-1)
		{}

		// Path to where the MRUs will be kept
		path_string registry_key;

		// Prefixes the MRUs so that several MRUs can be stored at the same key
		sstring prefix;

		// The MRU menu that will be populated with a call to RefreshMenu
		oGUI_MENU menu;

		// The menu ID to use for the first MRU file.
		int first_id;

		// The last menu ID to use for MRU files. All IDs in between will be used. 
		// The number of MRUs is determined by (LastID-FirstID+1).
		int last_id;
	};

	mru(const info& _Info);

	inline info get_info() const { return Info; }

	// Adds an entry as the most recently used (MRU) and thus truncates the 
	// existing list off the back of the list. This calls RefreshMenu()
	void add(const char* _Entry);

	// Retrieves the text of the entry at the specified _ID which must be between
	// FirstID and LastID in the current info.
	char* get(char* _StrDestination, size_t _SizeofStrDestination, int _ID);

	template<size_t size> char* get(char (&_StrDestination)[size], int _ID) { return get(_StrDestination, size, _ID); }
	template<size_t capacity> char* get(fixed_string<char, capacity>& _StrDestination, int _ID) { return get(_StrDestination, _StrDestination.capacity(), _ID); }

	// Deletes all entries in the menu specified in info and refills it with the 
	// latest data.
	void refresh();

private:
	info Info;
	sstring KeyFormat;
	int NumMRUs;

	sstring entry_name(int _Index);
	void get_entries(std::vector<uri_string>& _Entries);
};

	} // namespace gui
} // namespace ouro

#endif
