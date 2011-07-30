// $(header)
#pragma once
#ifndef oXML_h
#define oXML_h

#include <oooii/oInterface.h>

// Forward declaration to reduce dependencies, declared in oString.h, C++ 
// built-in types are implemented in oString.cpp, and the user can implement 
// more as desired.
template<typename T> errno_t oFromString(T* _pValue, const char* _StrSource);

interface oBuffer;
interface oXML : oInterface
{
	oDECLARE_HANDLE(HATTR);
	oDECLARE_HANDLE(HNODE);

	// makes internal copies of strings
	static bool Create(const char* _DocumentName, const char* _XMLString, size_t _EstimatedNumNodes, size_t _EstimatedNumAttributes, threadsafe oXML** _ppXML);

	// Buffer is retained internally by refcount
	static bool Create(const char* _DocumentName, threadsafe oBuffer* _pXMLStringBuffer, size_t _EstimatedNumNodes, size_t _EstimatedNumAttributes, threadsafe oXML** _ppXML);

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
	template<size_t size> inline bool GetAttributeValue(HNODE _hNode, const char* _AttributeName, char (&_StrDestination)[size]) const { return GetAttributeValue(_hNode, _AttributeName, _StrDestination, size); }
	template<typename T> inline bool GetTypedAttributeValue(HNODE _hNode, const char* _AttributeName, T* _pValue) const threadsafe
	{
		char s[256];
		bool success = GetAttributeValue(_hNode, _AttributeName, s, oCOUNTOF(s));
		if (success)
			success = (0 == oFromString(_pValue, s));
		return success;
	}
};

#endif
