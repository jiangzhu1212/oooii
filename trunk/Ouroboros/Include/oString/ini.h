// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Parses a string as an INI document by replacing certain delimiters inline 
// with null terminators and caching indices into the Buffers where values
// begin for very fast access to contents.

#pragma once
#include <oString/stringize.h>
#include <oString/text_document.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ouro {

class ini
{
	ini(const ini&);
	const ini& operator=(const ini&);

public:
	typedef uint16_t index_type;
	
	typedef struct section__ {}* section;
	typedef struct key__ {}* key;

	ini() : size_(0) {}
	ini(const char* _uri, char* data, deallocate_fn deallocate, size_t est_num_sections = 10, size_t est_num_keys = 100)
		: buffer(_uri, data, deallocate)
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		entries.reserve(est_num_sections + est_num_keys);
		index_buffer();
		size_ += entries.capacity() * sizeof(index_type);
	}

	ini(const char* _uri, const char* data, const allocator& alloc = default_allocator, size_t est_num_sections = 10, size_t est_num_keys = 100)
		: buffer(_uri, data, alloc, "ini doc")
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		entries.reserve(est_num_sections + est_num_keys);
		index_buffer();
		size_ += entries.capacity() * sizeof(index_type);
	}

	ini(ini&& _That) { operator=(std::move(_That)); }
	ini& operator=(ini&& _That)
	{
		if (this != &_That)
		{
			buffer = std::move(_That.buffer);
			entries = std::move(_That.entries);
			size_ = std::move(_That.size_);
		}
		return *this;
	}

	inline operator bool() const { return (bool)buffer; }
	inline const char* name() const { return buffer.uri; }
	inline size_t size() const { return size_;  }
	inline const char* section_name(section s) const { return buffer.data + entry(s).name; }
	inline section first_section() const { return entries.size() > 1 ? section(1) : 0; }
	inline section next_section(section _Prior) const { return section(entry(_Prior).next); }
	inline key first_key(section s) const { return key(entry(s).value); }
	inline key next_key(key _Prior) const { return key(entry(_Prior).next); }
	inline const char* name(key k) const { return buffer.data + entry(k).name; }
	inline const char* value(key k) const { return buffer.data + entry(k).value; }

	inline section find_section(const char* _Name) const
	{
		section s = 0;
		if (_Name) for (s = first_section(); s && _stricmp(_Name, section_name(s)); s = next_section(s)) {}
		return s;
	}

	inline key find_key(section s, const char* _KeyName) const
	{
		key k = 0;
		if (s && _KeyName) for (k = first_key(s); k && _stricmp(_KeyName, name(k)); k = next_key(k)) {}
		return k;
	}

	inline key find_key(const char* _SectionName, const char* _KeyName) const { return find_key(find_section(_SectionName), _KeyName); }

	inline const char* find_value(section s, const char* _KeyName) const { return value(find_key(s, _KeyName)); }
	inline const char* find_value(const char* _SectionName, const char* _KeyName) const { return value(find_key(_SectionName, _KeyName)); }

	template<typename T> bool find_value(section s, const char* _KeyName, T* _pValue) const
	{
		key k = find_key(s, _KeyName);
		if (k) return from_string(_pValue, value(k));
		return false;
	}
	
	template<typename charT, size_t capacity>
	bool find_value(section s, const char* _EntryName, fixed_string<charT, capacity>& _pValue) const
	{
		key k = find_key(s, _KeyName);
		if (k) return _pValue = value(k);
		return false;
	}

private:
	detail::text_buffer buffer;
	struct entry_t
	{
		entry_t() : next(0), name(0), value(0) {}
		index_type next;
		index_type name;
		index_type value;
	};
	typedef std::vector<entry_t> entries_t;
	entries_t entries;
	size_t size_;

	inline const entry_t& entry(key k) const { return entries[(size_t)k]; }
	inline const entry_t& entry(section s) const { return entries[(size_t)s]; }
	inline void index_buffer();
};

void ini::index_buffer()
{
	char* c = buffer.data;
	index_type lastSectionIndex = 0;
	entry_t s, k;
	bool link = false;
	entries.push_back(entry_t()); // make 0 to be a null object

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
				while (--c >= buffer.data && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
			case '[': // start of section, record it
				s.name = static_cast<index_type>(std::distance(buffer.data, c+1));
				if (lastSectionIndex) entries[lastSectionIndex].next = static_cast<index_type>(entries.size());
				lastSectionIndex = static_cast<index_type>(entries.size());
				entries.push_back(s);
				c += strcspn(c, "]"), *c++ = 0;
				link = false;
				break;
			default:
				if (entries.size() <= 1) throw new text_document_error(text_document_errc::generic_parse_error);
				k.next = 0;
				k.name = static_cast<index_type>(std::distance(buffer.data, c));
				c += strcspn(c, "=" oWHITESPACE); // move to end of key
				bool atSep = *c == '=';
				*c++ = 0;
				if (!atSep) c += strcspn(c, "=") + 1; // if we moved to whitespace, continue beyond it
				c += strspn(c, oWHITESPACE); // move past whitespace
				k.value = static_cast<index_type>(std::distance(buffer.data, c));
				c += strcspn(c, oNEWLINE ";"); // move to end of line or a comment
				if (link) entries.back().next = static_cast<index_type>(entries.size());
				link = true;
				if (!entries[lastSectionIndex].value) entries[lastSectionIndex].value = static_cast<index_type>(entries.size());
				entries.push_back(k);
				while (--c >= buffer.data && strchr(oWHITESPACE, *c)); // trim right whitespace
				if (!*(++c)) break; // if there is no more, just exit
				*c = 0;
				c += 1 + strcspn(c, oNEWLINE); // move to end of line
				// Support for \r\n  If we have just \n we need to advance 1 (done above), if we have both we need to advance a second time
				if (*c == '\n')
					c++; // advance again
				break;
		}
	}
	*buffer.data = '\0'; // have all empty name/values point to 0 offset and now that offset will be the empty string
}

}
