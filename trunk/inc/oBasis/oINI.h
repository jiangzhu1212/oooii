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
#ifndef oINI_h
#define oINI_h

#include <oBasis/oInterface.h>
#include <oBasis/oMacros.h>
#include <oBasis/oStringize.h>

interface oINI : oInterface
{
	oDECLARE_HANDLE(HSECTION);
	oDECLARE_HANDLE(HENTRY);

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
		return oFromString(_pValue, GetValue(_hSection, _EntryName));
	}
};

bool oINICreate(const char* _DocumentName, const char* _INIString, threadsafe oINI** _ppINI);

#endif
