// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_json_h
#define oString_json_h

// Parses a string as a JSON document by replacing certain delimiters inline 
// with null terminators and caching indices into the buffers where values
// begin for very fast access to contents.

#include <oString/text_document.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ouro {

enum class json_node_type { object, array, value };
enum class json_value_type { string, number, object, array, true_, false_, null };

class json
{
public:
	typedef uint32_t index_type;

	typedef struct node__ {}* node;

	json() : size_(0) {}
	json(const char* _URI, char* _pData, deallocate_fn _Delete, size_t _EstNumNodes = 100)
		: Buffer(_URI, _pData, _Delete)
	{
		size_ = sizeof(*this) + strlen(Buffer.data) + 1;
		nodes.reserve(_EstNumNodes);
		index_buffer();
		size_ += nodes.capacity() * sizeof(index_type) * 7;
	}

	json(const char* _URI, const char* _pData, const allocator& alloc = default_allocator, size_t _EstNumNodes = 100)
		: Buffer(_URI, _pData, alloc, "json doc")
	{
		size_ = sizeof(*this) + strlen(Buffer.data) + 1;
		nodes.reserve(_EstNumNodes);
		index_buffer();
		size_ += nodes.capacity() * sizeof(index_type) * 7;
	}

	json(json&& _That) { operator=(std::move(_That)); }
	json& operator=(json&& _That)
	{
		if (this != &_That)
		{
			Buffer = std::move(_That.Buffer);
			nodes = std::move(_That.nodes);
			size_ = std::move(_That.size_);
		}
		return *this;
	}

	inline operator bool() const { return (bool)Buffer; }
	inline const char* name() const { return Buffer.uri; }
	inline size_t size() const { return size_; }

	// Node API
	inline node root() const { return node(1); } 
	inline node first_child(node _ParentNode) const { return node(Node(_ParentNode).down); }
	inline node next_sibling(node _PriorSibling) const { return node(Node(_PriorSibling).next); }
	inline const char* node_name(node _Node) const { return Buffer.data + Node(_Node).name; }
	inline const char* node_value(node _Node) const { return Buffer.data + Node(_Node).value; }
	json_node_type node_type(node _Node) const { return Node(_Node).type; }
	json_value_type value_type(node _Node) const { return Node(_Node).value_type; }

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

	struct node_t
	{
		node_t() : next(0), down(0), attr(0), name(0), value(0), type(json_node_type::object), value_type(json_value_type::object) {}
		index_type next, down, attr, name, value;
		json_node_type type;
		json_value_type value_type;
	};

	typedef std::vector<node_t> nodes_t;

	nodes_t nodes;
	size_t size_;

	inline const node_t& Node(node _Node) const { return nodes[(size_t)_Node]; }

	inline node_t& Node(node _Node) { return nodes[(size_t)_Node]; }

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
	nodes.push_back(node_t()); // use up slot 0 so it can be used as a null handle
	nodes.push_back(node_t()); // add root node

	char* start = Buffer.data + strcspn(Buffer.data, "{[");
	bool isArray = *start++ == '[';
	*Buffer.data = 0; // make the first char nul so 0 offsets are the empty string

	int OpenTagCount = 0, CloseTagCount = 0; // these should be equal by the end
	make_next_node(start, root(), 0, isArray, OpenTagCount, CloseTagCount); // start recursing
	if (OpenTagCount != CloseTagCount) throw text_document_error(text_document_errc::unclosed_scope); // if not equal, something went wrong
}

json::node json::make_next_node(char*& _JSON, node _Parent, node _Previous, bool _IsArray, int& _OpenTagCount, int& _CloseTagCount)
{
	_OpenTagCount++;

	index_type newNode = static_cast<index_type>(nodes.size());
	if (_Parent) nodes[(size_t)_Parent].down = newNode;

	node hPreviousNode = _Previous;
	bool hasMoreSiblings = false;
	do 
	{
		newNode = static_cast<index_type>(nodes.size());
		if (hPreviousNode) nodes[(size_t)hPreviousNode].next = newNode;

		node_t n;
		n.type = json_node_type::value;

		if (_IsArray)
		{
			n.name = 0;

			// Skip whitespace
			_JSON += strspn(_JSON, " \t\v\r\n");
		}
		else
		{
			// Set name without the quotes, so zero terminate on the end quote
			_JSON += strcspn(_JSON, "\"");
			n.name = static_cast<index_type>(std::distance(Buffer.data, ++_JSON));
			if (!detail::skip_string(_JSON))
				return 0;
			*_JSON++ = 0;

			// Skip to value begin
			_JSON += strspn(_JSON, " \t\v\r\n:");
		}

		// Set value
		n.value = static_cast<index_type>(std::distance(Buffer.data, _JSON));

		// Push new node
		nodes.push_back(n);

		node parent = node(newNode);
		node prev = 0;

		// Determine node type and process it
		if (*_JSON == '{')
		{
			nodes[newNode].type = json_node_type::object;
			nodes[newNode].value = 0;
			nodes[newNode].value_type = json_value_type::object;

			// CreateNextNode sets down and next for newNode
			hPreviousNode = make_next_node(++_JSON, parent, prev, false, _OpenTagCount, _CloseTagCount);
			if (!hPreviousNode) return 0;
		}
		else if (*_JSON == '[')
		{
			nodes[newNode].type = json_node_type::array;
			nodes[newNode].value = 0;
			nodes[newNode].value_type = json_value_type::array;

			// CreateNextNode sets down and next for newNode
			hPreviousNode = make_next_node(++_JSON, parent, prev, true, _OpenTagCount, _CloseTagCount);
			if (!hPreviousNode) return 0;
		}
		else if (*_JSON == '\"')
		{
			nodes[newNode].value_type = json_value_type::string;
			detail::skip_string(++_JSON);
			_JSON++;
		}
		else if (*_JSON == 't')
		{
			nodes[newNode].value_type = json_value_type::true_;
			if (0 != memcmp(_JSON, "true", 4)) return 0;
			_JSON += 4;
		}
		else if (*_JSON == 'f')
		{
			nodes[newNode].value_type = json_value_type::false_;
			if (0 != memcmp(_JSON, "false", 5)) return 0;
			_JSON += 5;

		}
		else if (*_JSON == 'n')
		{
			nodes[newNode].value_type = json_value_type::null;
			if (0 != memcmp(_JSON, "null", 4)) return 0;
			_JSON += 4;
		}
		else if (*_JSON != 0)
		{
			nodes[newNode].value_type = json_value_type::number;
			_JSON += strspn(_JSON, "0123456789eE+-.");
		}
		else
		{
			return 0;
		}

		char* Marker = _JSON;

		// Skip whitespace to either the next name/value pair
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

}

#endif
