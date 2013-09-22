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
// http://en.wikipedia.org/wiki/Universally_Unique_Identifier
// Using <MSVSInstallDir>/Common7/Tools/guidgen.exe is the easiest
// way to generate these.
#pragma once
#ifndef oBase_guid_h
#define oBase_guid_h

#include <oBase/operators.h>

namespace ouro {

struct guid
{
private:
	typedef const unsigned long long* ptr;

public:
	unsigned int Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];

	bool operator<(const guid& _That) const { return ptr(this)[0] < ptr(&_That)[0] || (ptr(this)[0] == ptr(&_That)[0] && ptr(this)[1] < ptr(&_That)[1]); }
	inline bool operator==(const guid& _That) const { return ptr(this)[0] == ptr(&_That)[0] && ptr(this)[1] == ptr(&_That)[1]; }
	oOPERATORS_COMPARABLE(guid);
};

static const guid null_guid = { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

} // namespace ouro

#endif
