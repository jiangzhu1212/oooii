/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#ifndef oVersion_h
#define oVersion_h

#include <oBasis/oPlatformFeatures.h>
#include <oStd/operators.h>

struct oVersion : oComparable<oVersion>
{
	oVersion()
		: Major(0)
		, Minor(0)
		, Build(0)
		, Revision(0)
	{}

	/*constexpr*/ oVersion(unsigned short _Major, unsigned short _Minor, unsigned short _Build = 0, unsigned short _Revision = 0)
		: Major(_Major)
		, Minor(_Minor)
		, Build(_Build)
		, Revision(_Revision)
	{}

	unsigned short Major;
	unsigned short Minor;
	unsigned short Build;
	unsigned short Revision;

	bool IsValid() const { return Major || Minor || Build || Revision; }

	bool operator<(const oVersion& _That) const { return IsValid() && _That.IsValid() && ((Major < _That.Major) || (Major == _That.Major && Minor < _That.Minor) || (Major == _That.Major && Minor == _That.Minor && Build < _That.Build) || (Major == _That.Major && Minor == _That.Minor && Build == _That.Build) && Revision < _That.Revision); }
	bool operator==(const oVersion& _That) const { return IsValid() && _That.IsValid() && Major == _That.Major && Minor == _That.Minor && Build == _That.Build && Revision == _That.Revision; }
};

#endif
