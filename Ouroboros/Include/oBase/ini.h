// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_ini_h
#define oBase_ini_h

// Parses a string as an INI document by replacing certain delimiters inline 
// with null terminators and caching indices into the Buffers where values
// begin for very fast access to contents.

#include <oBase/text_document.h>
#include <oBase/macros.h>
#include <oString/stringize.h>
#include <cstring>
#include <vector>

namespace ouro {

class ini
{
	ini(const ini&);
	const ini& operator=(const ini&);

public:
	typedef unsigned short index_type;
	
	oDECLARE_HANDLE(section);
	oDECLARE_HANDLE(key);

	ini() : Size(0) {}
	ini(const char* _URI, char* _pData, const text_document_deleter_t& _Delete, size_t _EstNumSections = 10, size_t _EstNumKeys = 100)
		: Buffer(_URI, _pData, _Delete)
	{
		Size = sizeof(*this) + strlen(Buffer.pData) + 1;
		Entries.reserve(_EstNumSections + _EstNumKeys);
		index_buffer();
		Size += Entries.capacity() * sizeof(index_type);
	}

	ini(ini&& _That) { operator=(std::move(_That)); }
	ini& operator=(ini&& _That)
	{
		if (this != &_That)
		{
			Buffer = std::move(_That.Buffer);
			Entries = std::move(_That.Entries);
			Size = std::move(_That.Size);
		}
		return *this;
	}

	inline operator bool() const { return (bool)Buffer; }
	inline const char* name() const { return Buffer.URI; }
	inline size_t size() const { return Size;  }
	inline const char* section_name(section _Section) const { return Buffer.pData + Entry(_Section).Name; }
	inline section first_section() const { return Entries.size() > 1 ? section(1) : 0; }
	inline section next_section(section _Prior) const { return section(Entry(_Prior).Next); }
	inline key first_key(section _Section) const { return key(Entry(_Section).Value); }
	inline key next_key(key _Prior) const { return key(Entry(_Prior).Next); }
	inline const char* name(key _Key) const { return Buffer.pData + Entry(_Key).Name; }
	inline const char* value(key _Key) const { return Buffer.pData + Entry(_Key).Value; }

	inline section find_section(const char* _Name) const
	{
		section s = 0;
		if (_Name) for (s = first_section(); s && _stricmp(_Name, section_name(s)); s = next_section(s)) {}
		return s;
	}

	inline key find_key(section _Section, const char* _KeyName) const
	{
		key k = 0;
		if (_Section && _KeyName) for (k = first_key(_Section); k && _stricmp(_KeyName, name(k)); k = next_key(k)) {}
		return k;
	}

	inline key find_key(const char* _SectionName, const char* _KeyName) const { return find_key(find_section(_SectionName), _KeyName); }

	inline const char* find_value(section _Section, const char* _KeyName) const { return value(find_key(_Section, _KeyName)); }
	inline const char* find_value(const char* _SectionName, const char* _KeyName) const { return value(find_key(_SectionName, _KeyName)); }

	template<typename T> bool find_value(section _Section, const char* _KeyName, T* _pValue) const
	{
		key k = find_key(_Section, _KeyName);
		if (k) return from_string(_pValue, value(k));
		return false;
	}
	
	template<typename charT, size_t capacity>
	bool find_value(section _Section, const char* _EntryName, fixed_string<charT, capacity>& _pValue) const
	{
		key k = find_key(_Section, _KeyName);
		if (k) return _pValue = value(k);
		return false;
	}

private:
	detail::text_buffer Buffer;
	struct ENTRY
	{
		ENTRY() : Next(0), Name(0), Value(0) {}
		index_type Next;
		index_type Name;
		index_type Value;
	};
	typedef std::vector<ENTRY> entries_t;
	entries_t Entries;
	size_t Size;

	inline const ENTRY& Entry(key _Key) const { return Entries[(size_t)_Key]; }
	inline const ENTRY& Entry(section _Section) const { return Entries[(size_t)_Section]; }
	inline void index_buffer();
};

void ini::index_buffer()
{
	char* c = Buffer.pData;
	index_type lastSectionIndex = 0;
	ENTRY s, k;
	bool link = false;
	Entries.push_back(ENTRY()); // make 0 to be a null object

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
				while (--c >= Buffer.pData && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
			case '[': // start of section, record it
				s.Name = static_cast<index_type>(std::distance(Buffer.pData, c+1));
				if (lastSectionIndex) Entries[lastSectionIndex].Next = static_cast<index_type>(Entries.size());
				lastSectionIndex = static_cast<index_type>(Entries.size());
				Entries.push_back(s);
				c += strcspn(c, "]"), *c++ = 0;
				link = false;
				break;
			default:
				if (Entries.size() <= 1) throw new text_document_error(text_document_errc::generic_parse_error);
				k.Next = 0;
				k.Name = static_cast<index_type>(std::distance(Buffer.pData, c));
				c += strcspn(c, "=" oWHITESPACE); // move to end of key
				bool atSep = *c == '=';
				*c++ = 0;
				if (!atSep) c += strcspn(c, "=") + 1; // if we moved to whitespace, continue beyond it
				c += strspn(c, oWHITESPACE); // move past whitespace
				k.Value = static_cast<index_type>(std::distance(Buffer.pData, c));
				c += strcspn(c, oNEWLINE ";"); // move to end of line or a comment
				if (link) Entries.back().Next = static_cast<index_type>(Entries.size());
				link = true;
				if (!Entries[lastSectionIndex].Value) Entries[lastSectionIndex].Value = static_cast<index_type>(Entries.size());
				Entries.push_back(k);
				while (--c >= Buffer.pData && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
		}
	}
	*Buffer.pData = '\0'; // have all empty name/values point to 0 offset and now that offset will be the empty string
}

}

#endif
