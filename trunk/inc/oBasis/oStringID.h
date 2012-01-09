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
// Syntactic sugar for a hash
#pragma once
#ifndef oStringID_h
#define oStringID_h

#include <oBasis/oHash.h>

// If defined, an oStringID created from a string will retain the string. This
// is for debugging purposes only as the string might be longer than the debug
// space allocated.
#ifndef oSTRINGID_STORE_STRING
	#define oSTRINGID_STORE_STRING 0
#endif

template<typename id_t, class hash = std::hash<const char*> >
class oStringID
{
	unsigned int id;
	#if oSTRINGID_STORE_STRING == 1
		char s[512];
	#endif
public:
	oStringID()
		: id(0)
	{
		#if oSTRINGID_STORE_STRING == 1
			*s = 0;
		#endif
	}
	
	// required so 0 can be assigned to a handle to initialize it
	oStringID(int _id)
		: id(_id) 
	{
		#if oSTRINGID_STORE_STRING == 1
			*s = 0;
		#endif
	} 

	oStringID(const char* _String) 
		: id(oHash_stlpi(_String))
	{
		#if oSTRINGID_STORE_STRING == 1
			strcpy_s(s, _String ? _String : "");
		#endif
	}
	
	inline operator bool() const { return !!id; }
	inline operator unsigned int() const { return id; }
	inline bool operator==(const oStringID& other) const { return id == other.id; }
	inline bool operator!=(const oStringID& other) const { return !(*this == other); }
};

#endif
