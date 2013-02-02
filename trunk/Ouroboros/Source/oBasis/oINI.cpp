/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oBasis/oINI.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oRefCount.h>
#include <vector>

struct INI_ENTRY
{
	typedef unsigned short value_type;
	value_type Next;
	value_type Name;
	value_type Value;
};

typedef std::vector<INI_ENTRY> ENTRIES;

bool INIParse(ENTRIES& _Entries, char* _INIBuffer)
{
	char* c = _INIBuffer;
	INI_ENTRY::value_type lastSectionIndex = 0;
	INI_ENTRY s = {0}, k = {0};
	bool link = false;
	_Entries.reserve(100);
	_Entries.clear();
	_Entries.push_back(s); // make 0 to be a null object

	while (*c)
	{
		c += strspn(c, oWHITESPACE); // move past whitespace
		// Check that we have stepped to the end of the file (this can happen if the last line of the file is blank (meaning lastine\r\n\r\n)
		if (!*c)
			break;
		switch (*c)
		{
			case ';': // comment, move to end of line
				c += strcspn(c, oNEWLINE), c++;
				// If a comment is at the end of the file, we need to check to see if there is any file left to read
				while (--c >= _INIBuffer && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
			case '[': // start of section, record it
				s.Name = static_cast<INI_ENTRY::value_type>(std::distance(_INIBuffer, c+1));
				if (lastSectionIndex) _Entries[lastSectionIndex].Next = static_cast<INI_ENTRY::value_type>(_Entries.size());
				lastSectionIndex = static_cast<INI_ENTRY::value_type>(_Entries.size());
				_Entries.push_back(s);
				c += strcspn(c, "]"), *c++ = 0;
				link = false;
				break;
			default:
				if (_Entries.size() <= 1) return false; // key/val pairs outside of section context
				k.Next = 0;
				k.Name = static_cast<INI_ENTRY::value_type>(std::distance(_INIBuffer, c));
				c += strcspn(c, "=" oWHITESPACE); // move to end of key
				bool atSep = *c == '=';
				*c++ = 0;
				if (!atSep) c += strcspn(c, "=") + 1; // if we moved to whitespace, continue beyond it
				c += strspn(c, oWHITESPACE); // move past whitespace
				k.Value = static_cast<INI_ENTRY::value_type>(std::distance(_INIBuffer, c));
				c += strcspn(c, oNEWLINE ";"); // move to end of line or a comment
				if (link) _Entries.back().Next = static_cast<INI_ENTRY::value_type>(_Entries.size());
				link = true;
				if (!_Entries[lastSectionIndex].Value) _Entries[lastSectionIndex].Value = static_cast<INI_ENTRY::value_type>(_Entries.size());
				_Entries.push_back(k);
				while (--c >= _INIBuffer && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
		}
	}
	return true;
}

const oGUID& oGetGUID( threadsafe const oINI* threadsafe const * )
{
	// {2F7482EF-15E2-47a2-A553-F11FB9EFE53B}
	static const oGUID oIIDINI = { 0x2f7482ef, 0x15e2, 0x47a2, { 0xa5, 0x53, 0xf1, 0x1f, 0xb9, 0xef, 0xe5, 0x3b } };
	return oIIDINI;
}

struct oINI_Impl : public oINI
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oINI);

	oINI_Impl(const char* _DocumentName, const char* _INIString);
	~oINI_Impl();

	inline const INI_ENTRY& Entry(HENTRY _hEntry) const threadsafe { return (*Entries)[(size_t)_hEntry]; }
	inline const INI_ENTRY& Entry(HSECTION _hSection) const threadsafe { return (*Entries)[(size_t)_hSection]; }

	size_t GetDocumentSize() const threadsafe override;
	const char* GetDocumentName() const threadsafe override;
	HSECTION GetSection(const char* _Name) const threadsafe override;
	HENTRY GetEntry(HSECTION _hSection, const char* _Key) const threadsafe override;

	const char* GetSectionName(HSECTION _hSection) const threadsafe override { return _hSection ? (Data + Entry(_hSection).Name) : ""; }
	HSECTION GetFirstSection() const threadsafe override { return Entries->size() > 1 ? HSECTION(1) : 0; }
	HSECTION GetNextSection(HSECTION _hPriorSection) const threadsafe override { return _hPriorSection ? HSECTION(Entry(_hPriorSection).Next) : 0; }
	HENTRY GetFirstEntry(HSECTION _hSection) const threadsafe override { return HENTRY(Entry(_hSection).Value); }
	HENTRY GetNextEntry(HENTRY _hPriorEntry) const threadsafe override { return HENTRY(Entry(_hPriorEntry).Next); }
	const char* GetKey(HENTRY _hEntry) const threadsafe override { return _hEntry ? Data + Entry(_hEntry).Name : ""; }
	const char* GetValue(HENTRY _hEntry) const threadsafe override { return _hEntry ? Data + Entry(_hEntry).Value : ""; }

	char* Data;
	oInitOnce<oStringURI> DocumentName;
	oInitOnce<ENTRIES> Entries;
	oRefCount RefCount;
};

bool oINICreate(const char* _DocumentName, const char* _INIString, threadsafe oINI** _ppINI)
{
	if (!_INIString || !_ppINI) return false;
	*_ppINI = new oINI_Impl(_DocumentName, _INIString);
	return !!*_ppINI;
}

size_t oINI_Impl::GetDocumentSize() const threadsafe
{
	return sizeof(*this) + oStrlen(Data) + 1 + Entries->capacity() * sizeof(ENTRIES::value_type);
}

const char* oINI_Impl::GetDocumentName() const threadsafe
{
	return DocumentName->c_str();
}

oINI_Impl::oINI_Impl(const char* _DocumentName, const char* _INIString)
	: DocumentName(_DocumentName)
{
	size_t numberOfElements = oStrlen(oSAFESTR(_INIString))+1;
	Data = new char[numberOfElements];
	oStrcpy(Data, numberOfElements, oSAFESTR(_INIString));
	INIParse(Entries.Initialize(), Data);
}

oINI_Impl::~oINI_Impl()
{
	delete [] Data;
}

oINI::HSECTION oINI_Impl::GetSection(const char* _Name) const threadsafe
{
	HSECTION hCurrent = GetFirstSection();
	while (hCurrent && _Name && oStricmp(_Name, GetSectionName(hCurrent)))
		hCurrent = GetNextSection(hCurrent);
	return hCurrent;
}

oINI::HENTRY oINI_Impl::GetEntry(HSECTION _hSection, const char* _Key) const threadsafe
{
	HENTRY hCurrent = GetFirstEntry(_hSection);
	while (hCurrent && _Key && oStricmp(_Key, GetKey(hCurrent)))
		hCurrent = GetNextEntry(hCurrent);
	return hCurrent;
}
