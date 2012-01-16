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
// When dealing with drivers and APIs, it is often useful to confirm versions,
// so here's a tupple for the common case of a major/minor version number.
#pragma once
#ifndef oVersion_h
#define oVersion_h

#include <oBasis/oInvalid.h>
#include <oBasis/oOperators.h>

struct oVersion : oCompareable<oVersion>
{
	oVersion()
		: Major(oInvalid)
		, Minor(oInvalid)
	{}

	/*constexpr*/ oVersion(int _Major, int _Minor)
		: Major(_Major)
		, Minor(_Minor)
	{}

	int Major;
	int Minor;

	bool IsValid() const { return Major != oInvalid && Minor != oInvalid; }
	bool operator<(const oVersion& _That) const { return IsValid() && _That.IsValid() && ((Major < _That.Major) || (Major == _That.Major && Minor < _That.Minor)); }
	bool operator==(const oVersion& _That) const { return IsValid() && _That.IsValid() && Major == _That.Major && Minor == _That.Minor; }
};

#endif
