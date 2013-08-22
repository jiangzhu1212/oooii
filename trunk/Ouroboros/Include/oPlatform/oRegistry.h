/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// A generic mapping from an oURI to some oInterface object that has O(1) access
// and supports performant concurrent access.
#pragma once
#ifndef oRegistry_h
#define oRegistry_h

#include <oStd/date.h>
#include <oBasis/oInterface.h>
#include <oBasis/oURI.h>

struct oREGISTRY_DESC
{
	oREGISTRY_DESC()
		: Status(oInvalid)
		, AccessEpoch(oInvalid)
		, AccessCount(0)
	{}

	int Status;
	int AccessEpoch;
	int AccessCount;
	oStd::ntp_timestamp Accessed;
};

interface oRegistry : oInterface
{
	// Sets a reference value for the registry. Each time Get() is called, the
	// entry is marked with this value.
	virtual void SetEpoch(int _Epoch) threadsafe = 0;

	// Returns the currently set epoch.
	virtual int GetEpoch() const threadsafe = 0;

	// Returns the number of entries in the registry
	virtual int GetNumEntries() threadsafe const = 0;
	
	// Clear all entries
	virtual void Clear() threadsafe = 0;
	
	// Returns true if there's an entry.
	virtual bool Exists(const oURI& _URIReference) const threadsafe = 0;

	// Returns false if there is a pre-existing entry. AccessEpoch is set to the 
	// current epoch, AccessCount is set to zero, and Accessed is also set.
	virtual bool Add(const oURI& _URIReference, oInterface* _pInterface, int _Status = oInvalid) threadsafe = 0;
	
	// Returns false if there is no pre-existing entry.
	virtual bool Remove(const oURI& _URIReference) threadsafe = 0;

	// Returns false if there is no pre-existing entry. Each time this is called,
	// The AccessEpoch is set to the current epoch, the AccessCount is 
	// incremented, and the Accessed timestamp is set. If _pDesc is valid, a copy
	// of the post-updated stats are copied.
	virtual bool Get(const oURI& _URIReference, oInterface** _ppInterface, oREGISTRY_DESC* _pDesc = nullptr) const threadsafe = 0;
	
	// Returns false if there is no pre-existing entry. This sets AccessEpoch to
	// the current epoch, AccessCount to zero, and sets the Accessed timestamp. 
	// This sets the _Status as well, by default to oInvalid.
	virtual bool Set(const oURI& _URIReference, oInterface* _pInterface, int _Status = oInvalid) threadsafe = 0;
	
	// Exclusive-locks the registry and traverses each entry. Operations that 
	// would affect stats such as AccessCount are not performed. The visitor 
	// should return true to continue enummeration, or false to exit early.
	virtual void Enum(oFUNCTION<bool(const oURI& _URIReference, oInterface* _pInterface, const oREGISTRY_DESC& _Desc)> _Visitor) threadsafe = 0;
};

bool oRegistryCreate(threadsafe oRegistry** _ppRegistry);

#endif
