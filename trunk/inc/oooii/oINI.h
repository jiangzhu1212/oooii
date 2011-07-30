// $(header)
#pragma once
#ifndef oINI_h
#define oINI_h

#include <oooii/oInterface.h>

// Forward declaration to reduce dependencies, declared in oString.h, C++ 
// built-in types are implemented in oString.cpp, and the user can implement 
// more as desired.
template<typename T> errno_t oFromString(T* _pValue, const char* _StrSource);

interface oINI : oInterface
{
	oDECLARE_HANDLE(HSECTION);
	oDECLARE_HANDLE(HENTRY);

	static bool Create(const char* _DocumentName, const char* _INIString, threadsafe oINI** _ppINI);

	virtual size_t GetDocumentSize() const threadsafe = 0;
	virtual const char* GetDocumentName() const threadsafe = 0;

	virtual HSECTION GetSection(const char* _Name) const threadsafe = 0;
	virtual const char* GetSectionName(HSECTION _hSection) const threadsafe = 0;
	virtual HSECTION GetFirstSection() const threadsafe = 0;
	virtual HSECTION GetNextSection(HSECTION _hPriorSection) const threadsafe = 0;

	virtual HENTRY GetEntry(HSECTION _hSection, const char* _Key) const threadsafe = 0;
	virtual HENTRY GetFirstEntry(HSECTION _hSection) const threadsafe = 0;
	virtual HENTRY GetNextEntry(HENTRY _hPriorEntry) const threadsafe = 0;
	virtual const char* GetKey(HENTRY _hEntry) const threadsafe = 0;
	virtual const char* GetValue(HENTRY _hEntry) const threadsafe = 0;

	inline HENTRY GetEntry(const char* _SectionName, const char* _Key) const threadsafe { return GetEntry(GetSection(_SectionName), _Key); }
	inline const char* GetValue(HSECTION _hSection, const char* _Key) const threadsafe { return GetValue(GetEntry(_hSection, _Key)); }
	inline const char* GetValue(const char* _SectionName, const char* _Key) const threadsafe { return GetValue(GetEntry(GetSection(_SectionName), _Key)); }

	template<typename T> inline bool GetValue(HSECTION _hSection, const char* _EntryName, T* _pValue) const threadsafe
	{
		const char* s = GetValue(_hSection, _EntryName);
		return s ? (0 == oFromString(_pValue, s)) : false;
	}
};

#endif
