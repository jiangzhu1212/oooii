// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_xml_h
#define oString_xml_h

// Parses a string as an XML document by replacing certain delimiters inline 
// with null terminators and caching indices into the buffers where values
// begin for very fast access to contents.

// todo: add separate support for <?> and <!> nodes. Right now the parser skips
// over all <! nodes as comments and skips over all <?> nodes before the first
// non-<?> node. Thereafter nodes starting with ? will be parsed the same as
// regular nodes, which isn't consistent/correct/standard.

// todo: add support for text fragments: multiple node values interspersed with
// child nodes. I guess add a new handle "text" and have a first/next text.

#include <oString/fixed_string.h>
#include <oString/stringize.h>
#include <oString/text_document.h>
#include <cstdint>
#include <cstring>
#include <vector>

// Convenience macros for iterating through a list of child nodes and attrs
#define oBase_xml_foreach_child(child__, xml__, parent__) for (ouro::xml::node (child__) = (xml__).first_child(parent__); (child__); (child__) = (xml__).next_sibling(child__))
#define oBase_xml_foreach_attr(attr__, xml__, node__) for (ouro::xml::attr (attr__) = (xml__).first_attr(node__); (attr__); attr__ = (xml__).next_attr(attr__))

namespace ouro {

class xml
{
public:
	typedef uint32_t index_type;
	typedef char char_type;

	typedef struct node__ {}* node;
	typedef struct attr__ {}* attr;

	class visitor
	{
	public:
		// Used with the visit API. The XML document will be traversed depth-first
		// and events will be received in that order. NOTE: Currently only pre-text,
		// that is text that comes before a child node will be respected. This is a 
		// bug that will be fixed by calling an event for each section of text 
		// between child nodes - because of the parsing done by this object, a 
		// string of the entire block will never be returned. If any function 
		// returns false, traversal is aborted.

		typedef std::pair<const char_type*, const char_type*> attr_type; // name/value pair
		virtual bool node_begin(const char_type* _NodeName, const char_type* _XRef, const attr_type* _pAttributes, size_t _NumAttributes) = 0;
		virtual bool node_end(const char_type* _NodeName, const char_type* _XRef) = 0;
		virtual bool node_text(const char_type* _NodeName, const char_type* _XRef, const char_type* _NodeText) = 0;
	};

	xml() : Size(0) {}
	xml(const char_type* uri, char_type* data, deallocate_fn deallocate, size_t est_num_nodes = 100, size_t est_num_attrs = 500)
		: Buffer(uri, data, deallocate)
	{
		Size = sizeof(*this) + strlen(Buffer.data) + 1;
		Nodes.reserve(est_num_nodes);
		Attrs.reserve(est_num_attrs);
		index_buffer();
		Size += Nodes.capacity() * sizeof(index_type) + Attrs.capacity() * sizeof(index_type);
	}

	xml(const char_type* uri, const char_type* data, const allocator& alloc = default_allocator, size_t est_num_nodes = 100, size_t est_num_attrs = 500)
		: Buffer(uri, data, alloc, "xml doc")
	{
		Size = sizeof(*this) + strlen(Buffer.data) + 1;
		Nodes.reserve(est_num_nodes);
		Attrs.reserve(est_num_attrs);
		index_buffer();
		Size += Nodes.capacity() * sizeof(index_type) + Attrs.capacity() * sizeof(index_type);
	}

	xml(xml&& that) { operator=(std::move(that)); }
	xml& operator=(xml&& that)
	{
		if (this != &that)
		{
			Buffer = std::move(that.Buffer);
			Attrs = std::move(that.Attrs);
			Nodes = std::move(that.Nodes);
			Size = std::move(that.Size);
		}
		return *this;
	}

	inline operator bool() const { return (bool)Buffer; }
	inline const char_type* name() const { return Buffer.uri; }
	inline size_t size() const { return Size; }

	// Node API
	inline node root() const { return node(1); } 
	inline node parent(node n) const { return node(Node(n).up);  }
	inline node first_child(node parent_node) const { return node(Node(parent_node).down); }
	inline node next_sibling(node prior_sibling) const { return node(Node(prior_sibling).next); }
	inline const char_type* node_name(node n) const { return Buffer.data + Node(n).name; }
	inline const char_type* node_value(node n) const { return Buffer.data + Node(n).value; }
	
	// Attribute API
	inline attr first_attr(node n) const { return attr(Node(n).Attr); }
	inline attr next_attr(attr a) const { return Attr(a).name ? attr(a + 1) : 0; }
	inline const char_type* attr_name(attr a) const { return Buffer.data + Attr(a).name; }
	inline const char_type* attr_value(attr a) const { return Buffer.data + Attr(a).value; }
	
	// Convenience functions that use the above API
	inline node first_child(node parent_node, const char_type* _Name) const
	{
		node n = first_child(parent_node);
		while (n && _stricmp(node_name(n), _Name))
			n = next_sibling(n);
		return n;
	}

	inline node next_sibling(node _hPriorSibling, const char_type* _Name) const
	{
		node n = next_sibling(_hPriorSibling);
		while (n && _stricmp(node_name(n), _Name))
			n = next_sibling(n);
		return n;
	}

	inline attr find_attr(node _node, const char_type* _AttrName) const
	{
		for (attr a = first_attr(_node); a; a = next_attr(a))
			if (!_stricmp(attr_name(a), _AttrName))
				return a;
		return attr(0);
	}

	inline const char_type* find_attr_value(node _node, const char_type* _AttrName) const
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

	template<typename T> bool find_attr_value(node _Node, const char_type* _AttrName, T* _pValue) const
	{
		attr a = find_attr(_Node, _AttrName);
		return a ? from_string(_pValue, attr_value(a)) : false;
	}

	template<typename charT, size_t capacity>
	bool find_attr_value(node _Node, const char_type* _AttrName, fixed_string<charT, capacity>& _pValue) const
	{
		attr a = find_attr(_Node, _AttrName);
		if (a) _pValue = attr_value(a);
		return !!a;
	}

	inline bool visit(visitor& visitor) const
	{
		std::vector<visitor::attr_type> attrs;
		attrs.reserve(16);
		return visit(first_child(root()), 0, attrs, visitor);
	}

	// Helper function to be called from inside visitor functions
	static const char_type* find_attr_value(const visitor::attr_type* _pAttributes, size_t _NumAttributes, const char_type* _AttrName)
	{
		for (size_t i = 0; i < _NumAttributes; i++)
			if (!_stricmp(_pAttributes[i].first, _AttrName))
				return _pAttributes[i].second;
		return nullptr;
	}

	// Helper function to be called from inside visitor functions
	template<typename T>
	static bool find_attr_value(const visitor::attr_type* _pAttributes, size_t _NumAttributes, const char_type* _AttrName, T* _pValue)
	{
		const char_type* v = find_attr_value(_pAttributes, _NumAttributes, _AttrName);
		return v ? from_string(_pValue, v) : false;
	}

	template<typename charT, size_t capacity>
	static bool find_attr_value(const visitor::attr_type* _pAttributes, size_t _NumAttributes, const char_type* _AttrName, fixed_string<charT, capacity>& _pValue)
	{
		const char_type* v = find_attr_value(_pAttributes, _NumAttributes, _AttrName);
		if (v) _pValue = v;
		return !!v;
	}

	// It is common that ids for nodes are unique to the file, so find the 
	// specified string in the id or name attribute of the first node in depth-
	// first order.
	inline node find_id(const char_type* _ID) const { return find_id(root(), _ID); }

	// xref
	// This is like xpointer, but not as robust or verbose. If an xref does not 
	// begin with a '/', then this follows the typical convention of expecting 
	// that there is exactly one node with an id or name attribute that has the 
	// value and this returns the first match as found by a depth-first
	// search. If there is a slash, then this is an xref path. It is '/' delimited
	// and each section can either be the value of an id or name attr (id is looked
	// for first, then if not there then name is looked for) or it can be a 1-based
	// index of the sibling nodes.
	inline char_type* make_xref(char_type* _StrDestination, size_t _SizeofStrDestination, node _Node) const
	{
		std::vector<node> path;
		path.reserve(16);
		
		path.push_back(_Node);
		node p = parent(_Node);
		while (p != root()) // while not the root or invalid
		{
			path.push_back(p);
			p = parent(p);
		}

		if (path.empty())
		{
			if (_SizeofStrDestination < 2) return nullptr;
			_StrDestination[0] = '/';
			_StrDestination[1] = '\0';
			return _StrDestination;
		}
		else
			*_StrDestination = '\0';

		for (std::vector<node>::const_reverse_iterator it = path.rbegin(); it != path.rend(); ++it)
		{
			size_t len = strlcat(_StrDestination, "/", _SizeofStrDestination);
			if (len >= _SizeofStrDestination) return nullptr;
			
			const char_type* id = get_id(*it);
			char_type buf[10];
			if (!id)
			{
				int SiblingIndex = find_sibling_offset(*it) + 1;
				to_string(buf, SiblingIndex);
				id = buf;
			}

			len = strlcat(_StrDestination, id, _SizeofStrDestination);
			if (len >= _SizeofStrDestination) return nullptr;
		}

		return _StrDestination;
	}

	template<size_t size>
	char_type* make_xref(char_type (&_StrDestination)[size], node _Node) const { return make_xref(_StrDestination, size, _Node); }

	template<typename charT, size_t capacity> 
	char_type* make_xref(fixed_string<charT, capacity>& _StrDestination, node _Node) const { return make_xref(_StrDestination, _StrDestination.capacity(), _Node); }

	node find_xref(const char_type* _XRef) const
	{
		if (!_XRef || !_XRef[0]) return node(0);
		if (_XRef[0] == '/')
		{
			if (_XRef[1] == '\0') return root();
			return find_xref(first_child(root()), _XRef+1);
		}
		return find_id(root(), _XRef);
	}

private:
	detail::text_buffer Buffer;
	
	struct ATTR
	{
		ATTR() : name(0), value(0) {}
		index_type name, value;
	};

	struct NODE
	{
		NODE() : next(0), up(0), down(0), Attr(0), name(0), value(0) {}
		index_type next, up, down, Attr, name, value;
	};

	typedef std::vector<ATTR> attrs_t;
	typedef std::vector<NODE> nodes_t;

	attrs_t Attrs;
	nodes_t Nodes;
	size_t Size;

	inline const ATTR& Attr(attr a) const { return Attrs[(size_t)a]; }
	inline const NODE& Node(node n) const { return Nodes[(size_t)n]; }

	inline ATTR& Attr(attr a) { return Attrs[(size_t)a]; }
	inline NODE& Node(node n) { return Nodes[(size_t)n]; }

	// Parsing functions
	static inline ATTR make_attr(char_type* xml_start, char_type*& xml_current, bool& at_end);
	inline void index_buffer();
	inline void make_node_attrs(char_type* xml_start, char_type*& _XML, NODE& _Node);
	inline node make_next_node(char_type*& _XML, node _Parent, node _Previous, int& open_tag_count, int& close_tag_count);
	inline void make_next_node_children(char_type*& _XML, node _Parent, int& open_tag_count, int& close_tag_count);

	inline bool visit(node _Node, int _SiblingIndex, std::vector<visitor::attr_type>& _Attrs, visitor& visitor) const
	{
		const char_type* nname = node_name(_Node);
		const char_type* nval = node_value(_Node);
		_Attrs.clear();
		for (attr a = first_attr(_Node); a; a = next_attr(a))
			_Attrs.push_back(visitor::attr_type(attr_name(a), attr_value(a)));

		mstring xref;
		make_xref(xref, _Node);

		if (!visitor.node_begin(nname, xref, _Attrs.data(), _Attrs.size()))
			return false;

		// note: this should become interspersed with children for cases like this:
		// <node>
		//   some text
		//   <child />
		//   more text
		//   <child />
		//   final text
		// </node>
		if (nval && nval[0] && !visitor.node_text(nname, xref, nval))
			return false;

		int i = 0;
		for (node c = first_child(_Node); c; c = next_sibling(c), i++)
			if (!visit(c, i, _Attrs, visitor))
				return false;

		if (!visitor.node_end(nname, xref))
			return false;

		return true;
	}

	inline const char_type* get_id(node _Node) const
	{
		const char_type* id = find_attr_value(_Node, "id");
		if (!id) id = find_attr_value(_Node, "name");
		return id;
	}

	inline node find_id(node _Node, const char_type* _ID) const
	{
		const char_type* id = get_id(_Node);
		if (!_stricmp(id, _ID))
			return _Node;

		for (node c = first_child(_Node); c; c = next_sibling(c))
		{
			node n = find_id(c, _ID);
			if (n) return n;
		}

		return node(0);
	}

	// Return the 0-based offset from the first child of this node's parent.
	inline int find_sibling_offset(node _Node) const
	{
		int offset = 0;
		if (_Node)
		{
			node p = parent(_Node);
			node n = first_child(p);
			while (n != _Node)
			{
				offset++;
				n = next_sibling(n);
			}
		}
		return offset;
	}

	inline node find_xref(node _Node, const char_type* _XRef) const
	{
		sstring tmp;
		const char_type* slash = strchr(_XRef, '/');
		if (slash)
			tmp.assign(_XRef, slash);
		else 
			tmp = _XRef;

		node n = _Node;
		int Index = -1;
		if (from_string(&Index, tmp))
		{ // by index
			int i = 1;
			while (n)
			{
				if (Index == i)
					break;
				i++;
				n = next_sibling(n);
			}
		}

		else
		{ // by name
			while (n)
			{
				const char_type* id = get_id(n);
				if (id && !_stricmp(id, tmp))
					break;
				n = next_sibling(n);
			}
		}

		if (n)
		{
			if (slash)
				return find_xref(first_child(n), slash+1);
			return n;
		}
		return node(0);
	}
};

	namespace detail
	{
		// _XML must be pointing at a '<' char_type. This will leave _XML point at a '<'
		// to a non-reserved node.
		template<typename charT>
		inline void skip_reserved_nodes(charT*& _XML)
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

		template<typename charT>
		inline void ampersand_decode(charT* s)
		{
			static const struct { const char* code; unsigned char len; char c; } reserved[] = { { "lt;", 3, '<' }, { "gt;", 3, '>' }, { "amp;", 4, '&' }, {"apos;", 5, '\'' }, { "quot;", 5, '\"' }, };

			s = strchr(s, '&'); // find encoding marker
			if (s)
			{
				charT* w = s; // set up a write head to write in-place
				while (*s)
				{
					if (*s == '&' && ++s) // process a full symbol at a time
					{
						for (const auto& sym : reserved)
						{
							if (!memcmp(sym.code, s, sym.len)) // if this is an encoding, decode
							{
								*w++ = sym.c;
								s += sym.len;
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
	char_type* start = Buffer.data + strcspn(Buffer.data, "<"); // find first opening tag
	// offsets of 0 must point to the empty string, so assign first byte as empty
	// string... but ensure we've moved past where it's important to have that
	// char_type be meaningful.
	if (start == Buffer.data) detail::skip_reserved_nodes(start);
	Attrs.push_back(ATTR()); // init 0 to 0 offset from start
	Nodes.push_back(NODE()); // init 0 to point to 0 offset from start
	Nodes.push_back(NODE()); // init 1 to be the root - the parent of the first node
	int OpenTagCount = 0, CloseTagCount = 0; // these should be equal by the end
	make_next_node_children(start, root(), OpenTagCount, CloseTagCount); // start recursing
	*Buffer.data = 0; // make the first char_type nul so 0 offsets are the empty string
	if (OpenTagCount != CloseTagCount) throw text_document_error(text_document_errc::unclosed_scope); // if not equal, something went wrong
}

// This expects xml_current to be pointing after a node name and before an
// attribute's name. This can end with false if a new attribute could not be
// found and thus xml_current will be pointing to either a '/' for the one-line
// node definition (<mynode attr="val" />) or a '>' for the multi-line node
// definition (<mynode attr="val"></mynode>).
xml::ATTR xml::make_attr(xml::char_type* xml_start, xml::char_type*& xml_current, bool& at_end)
{
	ATTR a;
	while (*xml_current && !strchr("_/>", *xml_current) && !isalnum(*xml_current)) xml_current++; // move to next identifier ([_0-9a-zA-Z), but not an end tag ("/>" or ">")
	if (*xml_current != '>' && *xml_current != '/') // if not at the end of the node
	{
		a.name = static_cast<index_type>(std::distance(xml_start, xml_current)); // assign the offset to the start of the name
		xml_current += strcspn(xml_current, "/> \t\r\n="); // move to end of identifier 
		const char_type* checkEq = xml_current + strspn(xml_current, " \t\r\n"); // move to next non-whitespace

		if (*checkEq == '=')
		{
			*xml_current++ = 0;
			xml_current += strcspn(xml_current, "\"'"); // move to start of value
			a.value = static_cast<index_type>(std::distance(xml_start, ++xml_current)); // assign the offset to the start of the value (past quote)
			xml_current += strcspn(xml_current, "\"'"), *xml_current++ = 0; // find closing quote and nul-terminate
			detail::ampersand_decode(xml_start + a.name), detail::ampersand_decode(xml_start + a.value); // decode ampersand-encoding for the values (always makes string shorter, so this is ok to do inline)
		}

		else
		{
			if (*checkEq == '>' || *checkEq == '/')
				at_end = true;
			*xml_current++ = 0;
			a.value = a.name; 
		}
	}
	return a;
}

void xml::make_node_attrs(char_type* xml_start, char_type*& _XML, NODE& _Node)
{
	_Node.Attr = static_cast<index_type>(Attrs.size());
	bool AtEnd = false;
	while (*_XML != '>' && *_XML != '/' && !AtEnd)
	{
		ATTR a = make_attr(xml_start, _XML, AtEnd);
		if (a.name)
			Attrs.push_back(a);
	}
	if (*_XML == '>') _XML++;
	(_Node.Attr == Attrs.size()) ? _Node.Attr = 0 : Attrs.push_back(ATTR()); // either null the list if no elements added, or push a null terminator
}

void xml::make_next_node_children(char_type*& _XML, node _Parent, int& open_tag_count, int& close_tag_count)
{
	node prev = 0;
	while (*_XML != 0 && *(_XML+1) != '/' && *(_XML+1) != 0) // Check for end of the buffer as well
	{
		if (*(_XML+1) == '!') detail::skip_reserved_nodes(_XML);
		else prev = make_next_node(_XML, _Parent, prev, open_tag_count, close_tag_count);
	}
	if (*(_XML+1) == '/') // Validate this is an end tag
		close_tag_count++;
	if (_XML[0] && _XML[1] && _XML[2])
	{
		_XML += strcspn(_XML+2, ">") + 1;
		_XML += strcspn(_XML, "<");
	}
}

xml::node xml::make_next_node(char_type*& _XML, node _Parent, node _Previous, int& open_tag_count, int& close_tag_count)
{
	open_tag_count++;
	NODE n;
	n.up = (index_type)_Parent;
	if (_Parent || *_XML == '<') detail::skip_reserved_nodes(_XML); // base-case where the first char_type of file is '<' and that got nulled for the 0 offset empty value
	n.name = static_cast<index_type>(std::distance(Buffer.data, ++_XML));
	_XML += strcspn(_XML, " /\t\r\n>");
	bool veryEarlyOut = *_XML == '/';
	bool process = *_XML != '>' && !veryEarlyOut;
	*_XML++ = 0;
	if (process) make_node_attrs(Buffer.data, _XML, n);
	if (!veryEarlyOut)
	{
		if (*_XML != '/' && *_XML != '<') n.value = static_cast<index_type>(std::distance(Buffer.data, _XML++)), _XML += strcspn(_XML, "<");
		if (*(_XML+1) == '/') *_XML++ = 0;
		else n.value = 0;
	}
	else n.value = 0;
	index_type newNode = static_cast<index_type>(Nodes.size());
	if (_Previous) Node(_Previous).next = newNode;
	else if (_Parent) Node(_Parent).down = newNode; // only assign down for first child - all other siblings don't reassign their parent's down.
	Nodes.push_back(n);
	detail::ampersand_decode(Buffer.data + n.name), detail::ampersand_decode(Buffer.data + n.value);
	if (!veryEarlyOut && *_XML != '/') // recurse on children nodes
	{
		node p = node(newNode);
		make_next_node_children(_XML, p, open_tag_count, close_tag_count);
	}
	else 
	{
		close_tag_count++;
		_XML += strcspn(_XML, "<");
	}
	return node(newNode);
}

}

#endif
