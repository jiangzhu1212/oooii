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
// Interface for working with display adapters (video cards)
#pragma once
#ifndef oCore_adapter_h
#define oCore_adapter_h

#include <oStd/function.h>
#include <oStd/macros.h>
#include <oStd/vendor.h>
#include <oStd/version.h>

namespace oCore {
	namespace adapter {

class id
{
public:
	id() : Handle(-1) {}

	bool operator==(const id& _That) const { return Handle == _That.Handle; }
	bool operator!=(const id& _That) const { return !(*this == _That); }
	operator bool() const { return Handle != -1; }

private:
	int Handle;
};

struct info
{
	class id id;
	oStd::mstring description;
	oStd::mstring plugnplay_id;
	struct oStd::version version;
	oStd::vendor::value vendor;
};

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator);

// Ouroboros requires a minimum adapter driver version. This returns that 
// version.
oStd::version minimum_version(oStd::vendor::value _Vendor);

// Checks that all adapters meet or exceed the minimum version
inline bool all_up_to_date()
{
	bool IsUpToDate = true;
	enumerate([&](const info& _Info)->bool
	{
		if (_Info.version < minimum_version(_Info.vendor))
			IsUpToDate = false;
		return IsUpToDate;
	});
	return IsUpToDate;
}

	} // namespace adapter
} // namespace oCore

#endif
