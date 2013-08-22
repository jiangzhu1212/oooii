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
#include <oBasis/oVersion.h>
#include <oBasis/oString.h>
#include <stdio.h>

namespace oStd {

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oVersion& _Version)
{
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u.%u.%u.%u", _Version.Major, _Version.Minor, _Version.Build, _Version.Revision) ? _StrDestination : nullptr;
}

bool from_string(oVersion* _pType, const char* _StrSource)
{
	unsigned int Major=0, Minor=0, Build=0, Revision=0;
	int nScanned = sscanf_s(_StrSource, "%u.%u.%u.%u", &Major, &Minor, &Build, &Revision);
	if (nScanned <= 0) return false;
	#define CHK(x) do { _pType->x = static_cast<unsigned short>(x); if (_pType->x != x) return false; } while(false)
	CHK(Major); CHK(Minor); CHK(Build); CHK(Revision);
	return true;
}

} // namespace oStd