/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "pch.h"
#include <oooii/oXML.h>
#include <oooii/oAssert.h>
#include <oooii/oBuffer.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oGUID.h>

struct ATTRIBUTE
{
	typedef unsigned int value_type;
	value_type Name, Value;
	ATTRIBUTE() : Name(0), Value(0) {}
};

struct NODE
{
	typedef unsigned int value_type;
	value_type Next, Down, Attr, Name, Value;
	NODE() : Next(0), Down(0), Attr(0), Name(0), Value(0) {}
};

typedef std::vector<ATTRIBUTE> ATTRIBUTES;
typedef std::vector<NODE> NODES;

static void XMLOverwriteSpecialCharacters(char* s)
{
	struct SYM { const char* res; unsigned short len; char c; };
	static SYM reserved[] = { { "lt;", 3, '<' }, { "gt;", 3, '>' }, { "amp;", 4, '&' }, { "apos;", 5, '\'' }, { "quot;", 5, '\"' }, };
	s = strchr(s, '&');
	if (!s) return;
	char* w = s;
	while (*s)
		if (*s == '&' && ++s)
			for (SYM* sym = reserved, *end = reserved + oCOUNTOF(reserved); sym != end; ++sym)
				{ if (!memcmp(sym->res, s, sym->len)) { *w++ = sym->c; s += sym->len; break; } }
		else *w++ = *s++;
	*w = 0;
}

static void XMLScanToData(char*& oXML)
{
	bool skipping = false;
	do
	{
		skipping = false;
		if (!memcmp("<!--", oXML, 4)) // skip comments
		{
			skipping = true;
			oXML += strcspn(oXML+3, ">");
			oXML += strcspn(oXML, "<");
		}
		if (*(oXML+1) == '?') // skip meta tag
		{
			skipping = true;
			oXML += strcspn(oXML+2, ">");
			oXML += strcspn(oXML, "<");
		}
	} while (skipping);
}

void XMLCreateAttributes(char* _XMLDocument, char*& _XMLCurrent, ATTRIBUTES& _Attributes, NODE& _Node)
{
	_Node.Attr = static_cast<NODE::value_type>(_Attributes.size());
	while (*_XMLCurrent != '>')
	{
		while (*_XMLCurrent && !strchr("/>", *_XMLCurrent) && !(isalnum(*_XMLCurrent) || *_XMLCurrent == '_')) _XMLCurrent++; // find next identifier
		if (*_XMLCurrent == '>' || *_XMLCurrent == '/') break;
		ATTRIBUTE a;
		a.Name = static_cast<ATTRIBUTE::value_type>(std::distance(_XMLDocument, _XMLCurrent));
		_XMLCurrent += strcspn(_XMLCurrent, " \t\r\n="), *_XMLCurrent++ = 0;
		_XMLCurrent += strcspn(_XMLCurrent, "\"");
		a.Value = static_cast<ATTRIBUTE::value_type>(std::distance(_XMLDocument, ++_XMLCurrent));
		_XMLCurrent += strcspn(_XMLCurrent, "\">"), *_XMLCurrent++ = 0;
		_Attributes.push_back(a);
		XMLOverwriteSpecialCharacters(_XMLDocument+a.Name), XMLOverwriteSpecialCharacters(_XMLDocument+a.Value);
	}
	(_Node.Attr == _Attributes.size()) ? _Node.Attr = 0 : _Attributes.push_back(ATTRIBUTE()); // either null the list if no elements added, or push a null terminator
}

const oGUID& oGetGUID( threadsafe const oXML* threadsafe const * )
{
	// {2F544EC3-FE5C-450e-B522-DC6AEC08068E}
	static const oGUID oIIDXML = { 0x2f544ec3, 0xfe5c, 0x450e, { 0xb5, 0x22, 0xdc, 0x6a, 0xec, 0x8, 0x6, 0x8e } };
	return oIIDXML;
}

struct oXML_Impl : public oXML
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oXML>());

	HNODE GetFirstChild(HNODE _hParentNode, const char* _Name = 0) const threadsafe override;
	HNODE GetNextSibling(HNODE _hPriorSibling, const char* _Name = 0) const threadsafe override;
	HATTR GetFirstAttribute(HNODE _hNode) const threadsafe override;
	HATTR GetNextAttribute(HATTR _hAttribute) const threadsafe override;
	size_t GetDocumentSize() const threadsafe override;
	const char* GetDocumentName() const threadsafe override;
	const char* GetNodeName(HNODE _hNode) const threadsafe override;
	const char* GetNodeValue(HNODE _hNode) const threadsafe override;
	const char* GetAttributeName(HATTR _hAttribute) const threadsafe override;
	const char* GetAttributeValue(HATTR _hAttribute) const threadsafe override;
	const char* FindAttributeValue(HNODE _hNode, const char* _AttributeName) const threadsafe override;
	bool GetAttributeValue(HNODE _hNode, const char* _AttributeName, char* _StrDestination, size_t _NumberOfElements) const threadsafe override;

	oXML_Impl(const char* _DocumentName, const char* _XMLString, size_t _ReserveNodes = 100, size_t _ReserveAttributes = 100);
	oXML_Impl(const char* _DocumentName, threadsafe oBuffer* _pBuffer, size_t _ReserveNodes = 100, size_t _ReserveAttributes = 100);
	~oXML_Impl();

	inline ATTRIBUTE& Attribute(HATTR _hAttribute) const threadsafe { return thread_cast<ATTRIBUTES&>(Attributes)[(size_t)_hAttribute]; }
	inline NODE& Node(HNODE _hNode) const threadsafe { return thread_cast<NODES&>(Nodes)[(size_t)_hNode]; }

	HNODE CreateNextNode(char*& _XML, HNODE _hParentNode, HNODE _hPreviousNode);
	void InternalConstruct(const char* _DocumentName, threadsafe oBuffer* _pBuffer, size_t _ReserveNodes, size_t _ReserveAttributes);

	char* Data() const threadsafe { return XMLData; }

	char DocumentName[_MAX_PATH];
	oLockedPointer<oBuffer> BufferLock;
	threadsafe oRef<oBuffer> XMLBuffer;
	char* XMLData;
	ATTRIBUTES Attributes;
	NODES Nodes;
	oRefCount RefCount;
};

oXML::HNODE oXML_Impl::CreateNextNode(char*& _XML, HNODE _hParentNode, HNODE _hPreviousNode)
{
	XMLScanToData(_XML);
	NODE n;
	n.Name = static_cast<NODE::value_type>(std::distance(Data(), ++_XML));
	_XML += strcspn(_XML, " /\t\r\n>");
	bool veryEarlyOut = *_XML == '/';
	bool process = *_XML != '>' && !veryEarlyOut;
	*_XML++ = 0;
	if (process) XMLCreateAttributes(Data(), _XML, Attributes, n);
	if (!veryEarlyOut)
	{
		if (*_XML != '/') n.Value = static_cast<NODE::value_type>(std::distance(Data(), _XML++)), _XML += strcspn(_XML, "<");
		if (*(_XML+1) == '/') *_XML++ = 0;
		else n.Value = 0;
	}
	else n.Value = 0;
	NODE::value_type newNode = static_cast<NODE::value_type>(Nodes.size());
	if (_hParentNode) Node(_hParentNode).Down = newNode;
	if (_hPreviousNode) Node(_hPreviousNode).Next = newNode;
	Nodes.push_back(n);
	XMLOverwriteSpecialCharacters(Data()+n.Name), XMLOverwriteSpecialCharacters(Data()+n.Value);
	if (!veryEarlyOut && *_XML != '/') // recurse on children nodes
	{
		HNODE parent = HNODE(newNode);
		HNODE prev = 0;
		while (*(_XML+1) != '/')
			prev = CreateNextNode(_XML, parent, prev), parent = 0; // assign parent to 0 so it isn't reassigned the head of the child list
		_XML += strcspn(_XML+2, ">") + 1;
		_XML += strcspn(_XML, "<");
	}
	else _XML += strcspn(_XML, "<");
	return HNODE(newNode);
}

void oXML_Impl::InternalConstruct(const char* _DocumentName, threadsafe oBuffer* _pBuffer, size_t _ReserveNodes, size_t _ReserveAttributes)
{
	XMLBuffer = _pBuffer;
	BufferLock = XMLBuffer;
	XMLData = BufferLock->GetData<char>();
	strcpy_s(DocumentName, oSAFESTR(_DocumentName));

	// Use up slot 0 so it can be used as a null handle
	Attributes.reserve(_ReserveAttributes);
	Attributes.push_back(ATTRIBUTE());

	Nodes.reserve(_ReserveNodes);
	Nodes.push_back(NODE());

	char* p = XMLData;
	char* s = p + strcspn(p, "<");
	*p = 0; // empty strings point to p, and with this assignment properly to the empty string
	CreateNextNode(s, 0, 0);
}

oXML_Impl::oXML_Impl(const char* _DocumentName, const char* _XMLString, size_t _ReserveNodes, size_t _ReserveAttributes)
{
	size_t numberOfElements = strlen(oSAFESTR(_XMLString))+1;
	char* p = new char[numberOfElements];

	threadsafe oRef<oBuffer> buffer;
	if (!oBuffer::Create(_DocumentName, p, numberOfElements, oBuffer::Delete, &buffer))
		oASSERT(false, "Failed to create buffer for %s", _DocumentName);

	{
		oLockedPointer<oBuffer> pLocked(buffer);
		strcpy_s(pLocked->GetData<char>(), buffer->GetSize(), _XMLString);
	}
	
	InternalConstruct(_DocumentName, buffer, _ReserveNodes, _ReserveAttributes);
}

oXML_Impl::oXML_Impl(const char* _DocumentName, threadsafe oBuffer* _pBuffer, size_t _ReserveNodes, size_t _ReserveAttributes)
{
	InternalConstruct(_DocumentName, _pBuffer, _ReserveNodes, _ReserveAttributes);
}

oXML_Impl::~oXML_Impl()
{
	// Ensure things are unlocked in case the oBuffer gets freed during dtor
	BufferLock = 0;
}

bool oXML::Create(const char* _DocumentName, const char* _XMLString, size_t _EstimatedNumNodes, size_t _EstimatedNumAttributes, threadsafe oXML** _ppXML)
{
	if (!_XMLString || !_ppXML) return false;
	*_ppXML = new oXML_Impl(_DocumentName, _XMLString, _EstimatedNumNodes, _EstimatedNumAttributes);
	return !!*_ppXML;
}

bool oXML::Create(const char* _DocumentName, threadsafe oBuffer* _pBuffer, size_t _EstimatedNumNodes, size_t _EstimatedNumAttributes, threadsafe oXML** _ppXML)
{
	if (!_pBuffer || !_ppXML) return false;
	*_ppXML = new oXML_Impl(_DocumentName, _pBuffer, _EstimatedNumNodes, _EstimatedNumAttributes);
	return !!*_ppXML;
}

oXML::HNODE oXML_Impl::GetFirstChild(HNODE _hParentNode, const char* _Name) const threadsafe
{
	if (!_hParentNode) return HNODE(1);
	HNODE c = HNODE(Node(_hParentNode).Down);
	while (c && _Name && _stricmp(_Name, Data()+Node(c).Name))
		c = HNODE(Node(c).Next);
	return c;
}

oXML::HNODE oXML_Impl::GetNextSibling(HNODE _hPriorSibling, const char* _Name) const threadsafe
{
	HNODE s = HNODE(Node(_hPriorSibling).Next);
	while (s && _Name && _stricmp(_Name, Data()+Node(s).Name))
		s = HNODE(Node(s).Next);
	return s;
}

oXML::HATTR oXML_Impl::GetFirstAttribute(HNODE _hNode) const threadsafe
{
	return HATTR(Node(_hNode).Attr);
}

oXML::HATTR oXML_Impl::GetNextAttribute(HATTR _hAttribute) const threadsafe
{
	return Attribute(_hAttribute).Name ? HATTR(_hAttribute+1) : 0;
}

size_t oXML_Impl::GetDocumentSize() const threadsafe
{
	return sizeof(*this) + strlen(Data()) + 1 + thread_cast<NODES&>(Nodes).capacity() * sizeof(NODES::value_type) + thread_cast<ATTRIBUTES&>(Attributes).capacity() * sizeof(ATTRIBUTES::value_type);
}

const char* oXML_Impl::GetDocumentName() const threadsafe
{
	return thread_cast<const char*>(DocumentName);
}

const char* oXML_Impl::GetNodeName(HNODE _hNode) const threadsafe
{
	return Data()+Node(_hNode).Name;
}

const char* oXML_Impl::GetNodeValue(HNODE _hNode) const threadsafe
{
	return Data()+Node(_hNode).Value;
}

const char* oXML_Impl::GetAttributeName(HATTR _hAttribute) const threadsafe
{
	return Data()+Attribute(_hAttribute).Name;
}

const char* oXML_Impl::GetAttributeValue(HATTR _hAttribute) const threadsafe
{
	return Data()+Attribute(_hAttribute).Value;
}

const char* oXML_Impl::FindAttributeValue(HNODE _hNode, const char* _AttributeName) const threadsafe
{
	for (HATTR a = GetFirstAttribute(_hNode); a; a = GetNextAttribute(a))
		if (!_stricmp(_AttributeName, GetAttributeName(a)))
			return GetAttributeValue(a);
	return 0;
}

bool oXML_Impl::GetAttributeValue(HNODE _hNode, const char* _AttributeName, char* _StrDestination, size_t _NumberOfElements) const threadsafe
{
	const char* v = FindAttributeValue(_hNode, _AttributeName);
	if (v) strcpy_s(_StrDestination, _NumberOfElements, v);
	return !!v;
}
