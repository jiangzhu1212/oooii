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
// Parses a string as a JSON document by replacing certain delimiters inline 
// with null terminators and caching indices into the buffers where values
// begin for very fast access to contents.
#pragma once
#ifndef oStd_json_h
#define oStd_json_h

#include <oStd/text_document.h>
#include <oStd/macros.h>
#include <cstring>
#include <vector>

namespace oStd {

namespace json_node_type { enum type { object, array, value }; }
namespace json_value_type { enum type { string, number, object, array, true_, false_, null }; }

class json
{
public:
	typedef unsigned int index_type;

	oDECLARE_HANDLE(node);

	json() : Size(0) {}
	json(const char* _URI, char* _pData, const text_document_deleter_t& _Delete, size_t _EstNumNodes = 100)
		: Buffer(_URI, _pData, _Delete)
	{
		Size = sizeof(*this) + strlen(Buffer.pData) + 1;
		Nodes.reserve(_EstNumNodes);
		index_buffer();
		Size += Nodes.capacity() * sizeof(index_type) * 7;
	}

	json(json&& _That) { operator=(std::move(_That)); }
	json& operator=(json&& _That)
	{
		if (this != &_That)
		{
			Buffer = std::move(_That.Buffer);
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
	inline node first_child(node _ParentNode) const { return node(Node(_ParentNode).Down); }
	inline node next_sibling(node _PriorSibling) const { return node(Node(_PriorSibling).Next); }
	inline const char* node_name(node _Node) const { return Buffer.pData + Node(_Node).Name; }
	inline const char* node_value(node _Node) const { return Buffer.pData + Node(_Node).Value; }
	json_node_type::type node_type(node _Node) const { return Node(_Node).Type; }
	json_value_type::type value_type(node _Node) const { return Node(_Node).ValueType; }

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

private:
	detail::text_buffer Buffer;

	struct NODE
	{
		NODE() : Next(0), Down(0), Attr(0), Name(0), Value(0), Type(json_node_type::object), ValueType(json_value_type::object) {}
		index_type Next, Down, Attr, Name, Value;
		json_node_type::type Type;
		json_value_type::type ValueType;
	};

	typedef std::vector<NODE> nodes_t;

	nodes_t Nodes;
	size_t Size;

	inline const NODE& Node(node _Node) const { return Nodes[(size_t)_Node]; }

	inline NODE& Node(node _Node) { return Nodes[(size_t)_Node]; }

	// Parsing functions
	inline void index_buffer();
	inline node make_next_node(char*& _JSON, node _Parent, node _Previous, bool _IsArray, int& _OpenTagCount, int& _CloseTagCount);
};

	namespace detail
	{
		inline bool skip_string(char*& _JSON)
		{
			// _JSON should point to the first character after the initial quote and 
			// will then skip to the terminating quote.
			do 
			{
				_JSON += strcspn(_JSON, "\\\"");
				if (*_JSON == '\\')
				{
					size_t numBackslashes = strspn(_JSON, "\\");
					_JSON += numBackslashes;
					if (*_JSON == '\"' && (numBackslashes & 1) != 0)
						++_JSON;
				}
			}
			while (*_JSON != '\"' && *_JSON != 0);
			return *_JSON == '\"';
		}
	} // namespace detail


void json::index_buffer()
{
	Nodes.push_back(NODE()); // use up slot 0 so it can be used as a null handle
	Nodes.push_back(NODE()); // add root node

	char* start = Buffer.pData + strcspn(Buffer.pData, "{[");
	bool isArray = *start++ == '[';
	*Buffer.pData = 0; // make the first char nul so 0 offsets are the empty string

	int OpenTagCount = 0, CloseTagCount = 0; // these should be equal by the end
	make_next_node(start, root(), 0, isArray, OpenTagCount, CloseTagCount); // start recursing
	if (OpenTagCount != CloseTagCount) throw text_document_error(text_document_errc::unclosed_scope); // if not equal, something went wrong
}

json::node json::make_next_node(char*& _JSON, node _Parent, node _Previous, bool _IsArray, int& _OpenTagCount, int& _CloseTagCount)
{
	_OpenTagCount++;

	index_type newNode = static_cast<index_type>(Nodes.size());
	if (_Parent) Nodes[(size_t)_Parent].Down = newNode;

	node hPreviousNode = _Previous;
	bool hasMoreSiblings = false;
	do 
	{
		newNode = static_cast<index_type>(Nodes.size());
		if (hPreviousNode) Nodes[(size_t)hPreviousNode].Next = newNode;

		NODE n;
		n.Type = json_node_type::value;

		if (_IsArray)
		{
			n.Name = 0;

			// Skip whitespace
			_JSON += strspn(_JSON, " \t\v\r\n");
		}
		else
		{
			// Set name without the quotes, so zero terminate on the end quote
			_JSON += strcspn(_JSON, "\"");
			n.Name = static_cast<index_type>(std::distance(Buffer.pData, ++_JSON));
			if (!detail::skip_string(_JSON))
				return 0;
			*_JSON++ = 0;

			// Skip to value begin
			_JSON += strspn(_JSON, " \t\v\r\n:");
		}

		// Set value
		n.Value = static_cast<index_type>(std::distance(Buffer.pData, _JSON));

		// Push new node
		Nodes.push_back(n);

		node parent = node(newNode);
		node prev = 0;

		// Determine node type and process it
		if (*_JSON == '{')
		{
			Nodes[newNode].Type = json_node_type::object;
			Nodes[newNode].Value = 0;
			Nodes[newNode].ValueType = json_value_type::object;

			// CreateNextNode sets Down and Next for newNode
			hPreviousNode = make_next_node(++_JSON, parent, prev, false, _OpenTagCount, _CloseTagCount);
			if (!hPreviousNode) return 0;
		}
		else if (*_JSON == '[')
		{
			Nodes[newNode].Type = json_node_type::array;
			Nodes[newNode].Value = 0;
			Nodes[newNode].ValueType = json_value_type::array;

			// CreateNextNode sets Down and Next for newNode
			hPreviousNode = make_next_node(++_JSON, parent, prev, true, _OpenTagCount, _CloseTagCount);
			if (!hPreviousNode) return 0;
		}
		else if (*_JSON == '\"')
		{
			Nodes[newNode].ValueType = json_value_type::string;
			detail::skip_string(++_JSON);
			_JSON++;
		}
		else if (*_JSON == 't')
		{
			Nodes[newNode].ValueType = json_value_type::true_;
			if (0 != memcmp(_JSON, "true", 4)) return 0;
			_JSON += 4;
		}
		else if (*_JSON == 'f')
		{
			Nodes[newNode].ValueType = json_value_type::false_;
			if (0 != memcmp(_JSON, "false", 5)) return 0;
			_JSON += 5;

		}
		else if (*_JSON == 'n')
		{
			Nodes[newNode].ValueType = json_value_type::null;
			if (0 != memcmp(_JSON, "null", 4)) return 0;
			_JSON += 4;
		}
		else if (*_JSON != 0)
		{
			Nodes[newNode].ValueType = json_value_type::number;
			_JSON += strspn(_JSON, "0123456789eE+-.");
		}
		else
		{
			return 0;
		}

		char* Marker = _JSON;

		// Skip whitespace to either the next Name/Value pair
		// or end of Object/Array marker
		if (_IsArray)
			_JSON += strcspn(_JSON, ",]");
		else
			_JSON += strcspn(_JSON, ",}");

		hasMoreSiblings = (*_JSON == ',');

		// Clear from the marker to current pos to make sure the value above
		// gets zero terminated. We have to do that here because there is not
		// guaranteed room for a zero terminator until we've parsed until here.
		memset(Marker, 0, std::distance(Marker, ++_JSON));

		hPreviousNode = node(newNode);

	} while (hasMoreSiblings);

	_CloseTagCount++;

	return (*_JSON != 0) ? node(newNode) : 0;
}

} // namespace oStd

#endif
