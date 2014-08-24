/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oPlatform/oTest.h>
#include <oCore/windows/win_registry.h>

using namespace ouro::windows;

namespace ouro {
	namespace tests {

void TESTwin_registry()
{
	static const registry::hkey hKey = registry::current_user;
	static const char* KeyPath = "Software/Ouroboros/oUnitTests/TESTwin_registry";
	static const char* ValueName = "TestValue";
	static const char* TestValue = "Value";
	registry::set(hKey, KeyPath, ValueName, TestValue);
	lstring data;
	bool failed = false;
	try { registry::get(data, hKey, KeyPath, "Non-existant-ValueName"); }
	catch (std::exception&) { failed = true; }
	oCHECK(failed, "succeeded reading a non-existant key %s/%s", KeyPath, ValueName);
	oCHECK0(registry::get(data, hKey, KeyPath, ValueName));
	oCHECK(strcmp(data, TestValue) == 0, "Set/Read values do not match");
	registry::delete_value(hKey, KeyPath, ValueName);
	registry::delete_key(hKey, "Software/Ouroboros/oUnitTests");
}

	} // namespace tests
} // namespace ouro
