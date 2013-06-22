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
// Parses a string as an XML document by replacing certain delimiters inline 
// with null terminators and caching indices into the buffers where values
// begin for very fast access to contents.
#pragma once
#ifndef oStd_xml_h
#define oStd_xml_h

#include <oStd/fixed_string.h>
#include <oStd/macros.h>
#include <oStd/stringize.h>
#include <oStd/text_document.h>
#include <cstring>
#include <vector>

namespace oStd {

class xml
{
public:
	typedef unsigned int index_type;

	oDECLARE_HANDLE(node);
	oDECLARE_HANDLE(attr);

	xml() : Size(0) {}
	xml(const char* _URI, char* _pData, const text_document_deleter_t& _Delete, size_t _EstNumNodes = 100, size_t _EstNumAttrs = 500)
		: Buffer(_URI, _pData, _Delete)
	{
		Size = sizeof(*this) + strlen(Buffer.pData) + 1;
		Nodes.reserve(_EstNumNodes);
		Attrs.reserve(_EstNumAttrs);
		index_buffer();
		Size += Nodes.capacity() * sizeof(index_type) + Attrs.capacity() * sizeof(index_type);
	}

	xml(xml&& _That) { operator=(std::move(_That)); }
	xml& operator=(xml&& _That)
	{
		if (this != &_That)
		{
			Buffer = std::move(_That.Buffer);
			Attrs = std::move(_That.Attrs);
			Nodes = std::move(_That.Nodes);
			Size = std::move(_That.Size);
		}
		return *this;
	}

	inline operator bool() const { return (bool)Buffer; }
	inline const char* name() const { return Buffer.URI; }
	inline size_t size() const { return Size; }

	// Node API
	inline node root() const { return node(1); } 
	inline node first_child(node _ParentNode) const { if (_ParentNode) return node(Node(_ParentNode).Down); return node(1); }
	inline node next_sibling(node _PriorSibling) const { return node(Node(_PriorSibling).Next); }
	inline const char* node_name(node _Node) const { return Buffer.pData + Node(_Node).Name; }
	inline const char* node_value(node _Node) const { return Buffer.pData + Node(_Node).Value; }
	
	// Attribute API
	inline attr first_attr(node _Node) const { return attr(Node(_Node).Attr); }
	inline attr next_attr(attr _Attr) const { return Attr(_Attr).Name ? attr(_Attr + 1) : 0; }
	inline const char* attr_name(attr _Attr) const { return Buffer.pData + Attr(_Attr).Name; }
	inline const char* attr_value(attr _Attr) const { return Buffer.pData + Attr(_Attr).Value; }
	
	// Convenience functions that use the above API
	inline node first_child(node _ParentNode, const char* _Name) const
	{
		node n = first_child(_ParentNode);
		while (n && _stricmp(node_name(n), _Name))
			n = next_sibling(n);
		return n;
	}

	inline node next_sibling(node _hPriorSibling, const char* _Name) const
	{
		node n = next_sibling(_hPriorSibling);
		while (n && _stricmp(node_name(n), _Name))
			n = next_sibling(n);
		return n;
	}

	inline attr find_attr(node _node, const char* _AttrName) const
	{
		for (attr a = first_attr(_node); a; a = next_attr(a))
			if (!_stricmp(attr_name(a), _AttrName))
				return a;
		return attr(0);
	}

	inline const char* find_attr_value(node _node, const char* _AttrName) const
	{
		attr a = find_attr(_node, _AttrName);
		return a ? attr_value(a) : nullptr;
	}
	
	template<typename T> bool attr_value(attr _Attr, T* _pValue) const
	{
		return from_string(_pValue, attr_value(_Attr));
	}
	
	template<typename charT, size_t capacity> 
	bool attr_value(attr _Attr, fixed_string<charT, capacity>& _pValue) const
	{
		_pValue = attr_value(_Attr);
		return true;
	}

	template<typename T> bool find_attr_value(node _Node, const char* _AttrName, T* _pValue) const
	{
		attr a = find_attr(_Node, _AttrName);
		return a ? from_string(_pValue, attr_value(a)) : false;
	}

	template<typename charT, size_t capacity>
	bool find_attr_value(node _Node, const char* _AttrName, oStd::fixed_string<charT, capacity>& _pValue) const
	{
		attr a = find_attr(_Node, _AttrName);
		if (a) _pValue = attr_value(a);
		return !!a
	}

private:
	detail::text_buffer Buffer;
	
	struct ATTR
	{
		ATTR() : Name(0), Value(0) {}
		index_type Name, Value;
	};

	struct NODE
	{
		NODE() : Next(0), Down(0), Attr(0), Name(0), Value(0) {}
		index_type Next, Down, Attr, Name, Value;
	};

	typedef std::vector<ATTR> attrs_t;
	typedef std::vector<NODE> nodes_t;

	attrs_t Attrs;
	nodes_t Nodes;
	size_t Size;

	inline const ATTR& Attr(attr _Attr) const { return Attrs[(size_t)_Attr]; }
	inline const NODE& Node(node _Node) const { return Nodes[(size_t)_Node]; }

	inline ATTR& Attr(attr _Attr) { return Attrs[(size_t)_Attr]; }
	inline NODE& Node(node _Node) { return Nodes[(size_t)_Node]; }

	// Parsing functions
	static inline ATTR make_attr(char* _XMLStart, char*&_XMLCurrent);
	inline void index_buffer();
	inline void make_node_attrs(char* _XMLStart, char*& _XML, NODE& _Node);
	inline node make_next_node(char*& _XML, node _Parent, node _Previous, int& _OpenTagCount, int& _CloseTagCount);
};

	namespace detail
	{
		// _XML must be pointing at a '<' char. This will leave _XML point at a '<'
		// to a non-reserved node.
		inline void skip_reserved_nodes(char*& _XML)
		{
			bool skipping = false;
			do
			{
				skipping = false;
				if (*(_XML+1) == '!') // skip comment
				{
					skipping = true;
					_XML = strstr(_XML+4, "-->");
					if (!_XML) throw new text_document_error(text_document_errc::unclosed_scope);
					_XML += strcspn(_XML, "<");
				}
				if (*(_XML+1) == '?') // skip meta tag
				{
					skipping = true;
					_XML += strcspn(_XML+2, ">");
					_XML += strcspn(_XML, "<");
				}
			} while (skipping);
		}

		inline void ampersand_decode(char* s)
		{
			static const char* code[] = { "lt;", "gt;", "amp;", "apos;", "quot;" };
			static const unsigned int codelen[] = { 3, 3, 4, 5, 5 };
			static const char decode[] = { '<', '>', '&', '\'', '\"' };
			s = strchr(s, '&'); // find encoding marker
			if (s)
			{
				char* w = s; // set up a write head to write in-place
				while (*s)
				{
					if (*s == '&' && ++s) // process a full symbol at a time
					{
						for (size_t i = 0; i < oCOUNTOF(code); i++)
						{
							if (!memcmp(code[i], s, codelen[i])) // if this is an encoding, decode
							{
								*w++ = decode[i];
								s += codelen[i];
								break;
							}
						}
					}
					else
						*w++ = *s++; // copy character normally
				}
				*w = 0; // ensure the final string is nul-terminated
			}
		}
	} // namespace detail

void xml::index_buffer()
{
	char* start = Buffer.pData + strcspn(Buffer.pData, "<"); // find first opening tag
	// offsets of 0 must point to the empty string, so assign first byte as empty
	// string... but ensure we've moved past where it's important to have that
	// char be meaningful.
	if (start == Buffer.pData) detail::skip_reserved_nodes(start);
	*Buffer.pData = 0; // make the first char nul so 0 offsets are the empty string
	Nodes.push_back(NODE()); // init 0 to point to 0 offset from start
	Attrs.push_back(ATTR()); // init 0 to 0 offset from start
	int OpenTagCount = 0, CloseTagCount = 0; // these should be equal by the end
	make_next_node(start, 0, 0, OpenTagCount, CloseTagCount); // start recursing
	if (OpenTagCount != CloseTagCount) throw text_document_error(text_document_errc::unclosed_scope); // if not equal, something went wrong
}

// This expects _XMLCurrent to be pointing after a node name and before an
// attribute's name. This can end with false if a new attribute could not be
// found and thus _XMLCurrent will be pointing to either a '/' for the one-line
// node definition (<mynode attr="val" />) or a '>' for the multi-line node
// definition (<mynode attr="val"></mynode>).
xml::ATTR xml::make_attr(char* _XMLStart, char*&_XMLCurrent)
{
	ATTR a;
	while (*_XMLCurrent && !strchr("_/>", *_XMLCurrent) && !isalnum(*_XMLCurrent)) _XMLCurrent++; // move to next identifier ([_0-9a-zA-Z), but not an end tag ("/>" or ">")
	if (*_XMLCurrent != '>' && *_XMLCurrent != '/') // if not at the end of the node
	{
		a.Name = static_cast<index_type>(std::distance(_XMLStart, _XMLCurrent)); // assign the offset to the start of the name
		_XMLCurrent += strcspn(_XMLCurrent, " \t\r\n="), *_XMLCurrent++ = 0; // move to end of identifier and make the name a nul-terminated string
		_XMLCurrent += strcspn(_XMLCurrent, "\""); // move to start of value
		a.Value = static_cast<index_type>(std::distance(_XMLStart, ++_XMLCurrent)); // assign the offset to the start of the value (past quote)
		_XMLCurrent += strcspn(_XMLCurrent, "\""), *_XMLCurrent++ = 0; // find closing quote and nul-terminate
		detail::ampersand_decode(_XMLStart + a.Name), detail::ampersand_decode(_XMLStart + a.Value); // decode ampersand-encoding for the values (always makes string shorter, so this is ok to do inline)
	}
	return a;
}

void xml::make_node_attrs(char* _XMLStart, char*& _XML, NODE& _Node)
{
	_Node.Attr = static_cast<index_type>(Attrs.size());
	while (*_XML != '>' && *_XML != '/')
	{
		ATTR a = make_attr(_XMLStart, _XML);
		if (a.Name)
			Attrs.push_back(a);
	}
	if (*_XML == '>') _XML++;
	(_Node.Attr == Attrs.size()) ? _Node.Attr = 0 : Attrs.push_back(ATTR()); // either null the list if no elements added, or push a null terminator
}

xml::node xml::make_next_node(char*& _XML, node _Parent, node _Previous, int& _OpenTagCount, int& _CloseTagCount)
{
	_OpenTagCount++;
	NODE n;
	if (_Parent || *_XML == '<') detail::skip_reserved_nodes(_XML); // base-case where the first char of file is '<' and that got nulled for the 0 offset empty value
	n.Name = static_cast<index_type>(std::distance(Buffer.pData, ++_XML));
	_XML += strcspn(_XML, " /\t\r\n>");
	bool veryEarlyOut = *_XML == '/';
	bool process = *_XML != '>' && !veryEarlyOut;
	*_XML++ = 0;
	if (process) make_node_attrs(Buffer.pData, _XML, n);
	if (!veryEarlyOut)
	{
		if (*_XML != '/' && *_XML != '<') n.Value = static_cast<index_type>(std::distance(Buffer.pData, _XML++)), _XML += strcspn(_XML, "<");
		if (*(_XML+1) == '/') *_XML++ = 0;
		else n.Value = 0;
	}
	else n.Value = 0;
	index_type newNode = static_cast<index_type>(Nodes.size());
	if (_Parent) Node(_Parent).Down = newNode;
	if (_Previous) Node(_Previous).Next = newNode;
	Nodes.push_back(n);
	detail::ampersand_decode(Buffer.pData + n.Name), detail::ampersand_decode(Buffer.pData + n.Value);
	if (!veryEarlyOut && *_XML != '/') // recurse on children nodes
	{
		node parent = node(newNode);
		node prev = 0;
		while (*(_XML+1) != '/' && *(_XML+1) != 0) // Check for end of the buffer as well
		{
			if (*(_XML+1) == '!') detail::skip_reserved_nodes(_XML);
			else prev = make_next_node(_XML, parent, prev, _OpenTagCount, _CloseTagCount), parent = 0; // assign parent to 0 so it isn't reassigned the head of the child list
		}
		if (*(_XML+1) == '/') // Validate this is an end tag
			_CloseTagCount++;
		_XML += strcspn(_XML+2, ">") + 1;
		_XML += strcspn(_XML, "<");
	}
	else 
	{
		_CloseTagCount++;
		_XML += strcspn(_XML, "<");
	}
	return node(newNode);
}

} // namespace oStd

#endif
