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
#pragma once
#ifndef oXML_h
#define oXML_h

#include <oBasis/oFunction.h>
#include <oBasis/oInterface.h>
#include <oBasis/oStringize.h>

interface oXML : oInterface
{
	struct DESC
	{
		DESC()
			: DocumentName(nullptr)
			, XMLString(nullptr)
			, EstimatedNumNodes(100)
			, EstimatedNumAttributes(1000)
			, CopyXMLString(true)
		{}

		// Name (often a file path) of the document. This is just a label and is not
		// used internally.
		const char* DocumentName;

		// The source string that will be parsed as XML.
		char* XMLString;

		// Preallocate memory ahead of parsing for this many nodes
		unsigned int EstimatedNumNodes;

		// Preallocate memory ahead of parsing for this many attributes
		unsigned int EstimatedNumAttributes;

		// If CopyXMLString is false, this function will be called on XMLString in the
		// destructor of the oXML object. This can be a noop function if the memory is
		// not to be owned by oXML, however note that oXML will modify the specified
		// buffer, so it won't be useful by anyone outside this class.
		oFUNCTION<void(const char* _XMLString)> FreeXMLString;

		// If CopyXMLString is true, a copy of XML String will be made and the created
		// oXML object has no further use of the buffer pointed to by XMLString. If 
		// this is false, then oXML will parse the XML and modify contents in the 
		// buffer and call FreeXMLString on the buffer in the oXML's destructor.
		bool CopyXMLString;
	};

	oDECLARE_HANDLE(HATTR);
	oDECLARE_HANDLE(HNODE);

	// If _hParentNode is 0, this returns the root node. If a name is specified,
	// this returns the first child that matches the specified name.
	virtual HNODE GetFirstChild(HNODE _hParentNode, const char* _Name = 0) const threadsafe = 0;

	// If _hParentNode is 0, this returns the root node. If a name is specified,
	// this returns the first child that matches the specified name.
	virtual HNODE GetNextSibling(HNODE _hPriorSibling, const char* _Name = 0) const threadsafe = 0;

	virtual HATTR GetFirstAttribute(HNODE _hNode) const threadsafe = 0;
	virtual HATTR GetNextAttribute(HATTR _hAttribute) const threadsafe = 0;

	virtual size_t GetDocumentSize() const threadsafe = 0;
	virtual const char* GetDocumentName() const threadsafe = 0;
	virtual const char* GetNodeName(HNODE _hNode) const threadsafe = 0;
	virtual const char* GetNodeValue(HNODE _hNode) const threadsafe = 0;
	virtual const char* GetAttributeName(HATTR _hAttribute) const threadsafe = 0;
	virtual const char* GetAttributeValue(HATTR _hAttribute) const threadsafe = 0;
	virtual const char* FindAttributeValue(HNODE _hNode, const char* _AttributeName) const threadsafe = 0;
	virtual bool GetAttributeValue(HNODE _hNode, const char* _AttributeName, char* _StrDestination, size_t _NumberOfElements) const threadsafe = 0;
	template<size_t size> inline bool GetAttributeValue(HNODE _hNode, const char* _AttributeName, char (&_StrDestination)[size]) const threadsafe { return GetAttributeValue(_hNode, _AttributeName, _StrDestination, size); }
	template<typename T> inline bool GetTypedAttributeValue(HNODE _hNode, const char* _AttributeName, T* _pValue) const threadsafe
	{
		char s[256];
		if (!GetAttributeValue(_hNode, _AttributeName, s, oCOUNTOF(s)))
			return false;
		return oFromString(_pValue, s);
	}
};

// If _FreeXMLString is non-null, oXML will maintain the lifetime of the buffer
// pointed to by _XMLString and use _FreeXMLString to free the buffer. If no
// Free function is specified, a copy of _XMLString 
bool oXMLCreate(const oXML::DESC& _Desc, threadsafe oXML** _ppXML);

#endif
