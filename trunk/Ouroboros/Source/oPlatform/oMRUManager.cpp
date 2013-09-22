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
#include <oPlatform/oMRUManager.h>
#include <oPlatform/oGUIMenu.h>
#include <oPlatform/Windows/oWinRegistry.h>

using namespace ouro;

struct oMRUManagerRegistry : oMRUManager
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oMRUManagerRegistry(const oMRU_DESC& _Desc, bool* _pSuccess);

	void GetDesc(oMRU_DESC* _pDesc) override;
	void Add(const char* _Entry) override;
	char* Get(char* _StrDestination, size_t _SizeofStrDestination, int _ID) override;
	void RefreshMenu() override;

private:
	oMRU_DESC Desc;
	sstring MRUKeyFormat;
	oRefCount RefCount;
	int NumMRUs;

	void GetEntryName(sstring& _EntryName, int _Index);
	void GetEntries(std::vector<uri_string>& _Entries);
};

oMRUManagerRegistry::oMRUManagerRegistry(const oMRU_DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, NumMRUs(_Desc.LastID-_Desc.FirstID+1)
{
		*_pSuccess = false;

	if (_Desc.MRUEntryPrefix.empty())
	{
		oErrorSetLast(std::errc::invalid_argument, "An MRUEntryPrefix must be specified");
		return;
	}

	if (0 > snprintf(MRUKeyFormat, "%s%%02d", _Desc.MRUEntryPrefix))
	{
		oErrorSetLast(std::errc::invalid_argument, "MRUEntryPrefix is too long.");
		return;
	}

	*_pSuccess = true;
}

bool oMRUManagerCreate(const oMRU_DESC& _Desc, oMRUManager** _ppMRUManager)
{
	bool success = false;
	oCONSTRUCT(_ppMRUManager, oMRUManagerRegistry(_Desc, &success));
	return success;
}

void oMRUManagerRegistry::GetEntryName(sstring& _EntryName, int _Index)
{
	snprintf(_EntryName, "MRU%02d", _Index);
}

void oMRUManagerRegistry::GetEntries(std::vector<uri_string>& _Entries)
{
	uri_string Entry;
	_Entries.clear();
	_Entries.reserve(NumMRUs);
	for (int i = 0; i < NumMRUs; i++)
	{
		sstring EntryName;
		GetEntryName(EntryName, i);
		if (oWinRegistryGetValue(Entry, oHKEY_CURRENT_USER, Desc.MRURegistryKey, EntryName))
			_Entries.push_back(Entry);
	}
}

void oMRUManagerRegistry::GetDesc(oMRU_DESC* _pDesc)
{
	*_pDesc = Desc;
}

void oMRUManagerRegistry::Add(const char* _Entry)
{
	sstring EntryName;
	std::vector<uri_string> Entries;
	GetEntries(Entries);

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
	int i = 0;
	for (; i < oInt(Entries.size()); i++)
	{
		GetEntryName(EntryName, i);
		oVERIFY(oWinRegistrySetValue(oHKEY_CURRENT_USER, Desc.MRURegistryKey, EntryName, Entries[i]));
	}

	// and delete any extra stale entries that remain as a result of the Entries
	// list getting shorter
	const int NumMRUs = Desc.LastID-Desc.FirstID+1;
	for (; i < NumMRUs; i++)
	{
		GetEntryName(EntryName, i);
		oWinRegistryDeleteValue(oHKEY_CURRENT_USER, Desc.MRURegistryKey, EntryName);
	}

	RefreshMenu();
}

char* oMRUManagerRegistry::Get(char* _StrDestination, size_t _SizeofStrDestination, int _ID)
{
	if (_ID >= Desc.FirstID && _ID <= Desc.LastID)
	{
		sstring EntryName;
		GetEntryName(EntryName, _ID - Desc.FirstID);
		if (oWinRegistryGetValue(_StrDestination, _SizeofStrDestination, oHKEY_CURRENT_USER, Desc.MRURegistryKey, EntryName))
			return _StrDestination;
	}

	return nullptr;
}

void oMRUManagerRegistry::RefreshMenu()
{
	oGUIMenuRemoveAllItems(Desc.hMenu);

	sstring EntryName;
	uri_string Entry;
	for (int i = 0; i < NumMRUs; i++)
	{
		GetEntryName(EntryName, i);
		if (oWinRegistryGetValue(Entry, oHKEY_CURRENT_USER, Desc.MRURegistryKey, EntryName))
			oGUIMenuAppendItem(Desc.hMenu, Desc.FirstID + i, Entry);
	}
}
