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
#ifndef oMRUManager_h
#define oMRUManager_h

#include <oBase/fixed_string.h>
#include <oBasis/oGUI.h>
#include <oBasis/oInterface.h>

struct oMRU_DESC
{
	// Path to where the MRUs will be kept
	ouro::path_string MRURegistryKey;

	// Prefixes the MRUs so that several MRUs can be stored at the same key
	ouro::sstring MRUEntryPrefix;

	// The MRU menu that will be populated with a call to RefreshMenu
	oGUI_MENU hMenu;

	// The menu ID to use for the first MRU file.
	int FirstID;

	// The last menu ID to use for MRU files. All IDs in between will be used. The 
	// number of MRUs is determined by (LastID-FirstID+1).
	int LastID;
};

interface oMRUManager : oInterface
{
	virtual void GetDesc(oMRU_DESC* _pDesc) = 0;

	// Adds an entry as the most recently used (MRU) and thus truncates the 
	// existing list off the back of the list. This calls RefreshMenu()
	virtual void Add(const char* _Entry) = 0;

	// Retrieves the text of the entry at the specified _ID which must be between
	// FirstID and LastID in the current oMRU_DESC.
	virtual char* Get(char* _StrDestination, size_t _SizeofStrDestination, int _ID) = 0;

	template<size_t size> char* Get(char (&_StrDestination)[size], int _ID) { return Get(_StrDestination, size, _ID); }
	template<size_t capacity> char* Get(ouro::fixed_string<char, capacity>& _StrDestination, int _ID) { return Get(_StrDestination, _StrDestination.capacity(), _ID); }

	// Deletes all entries in the menu specified in oMRU_DESC and refills it with
	// the latest data.
	virtual void RefreshMenu() = 0;
};

bool oMRUManagerCreate(const oMRU_DESC& _Desc, oMRUManager** _ppMRUManager);

#endif
