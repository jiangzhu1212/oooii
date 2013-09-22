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
#include <oPlatform/oTest.h>
#include <oPlatform/Windows/oWinRegistry.h>

const char* KeyTestValue = "Value";
const char* KeyPath = "Software/OOOii-oUnitTests/PLATFORM_oWinRegistry";
const char* ValueName = "TestValue";

struct PLATFORM_oWinRegistry : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB(oWinRegistrySetValue(oHKEY_CURRENT_USER, KeyPath, ValueName, KeyTestValue), "Failed to set key %s", oErrorGetLastString());
		ouro::lstring data;
		oTESTB(!oWinRegistryGetValue(data, oHKEY_CURRENT_USER, KeyPath, "Non-existant-ValueName"), "Succeeded reading a non-existant key %s", oErrorGetLastString());
		oTESTB(oWinRegistryGetValue(data, oHKEY_CURRENT_USER, KeyPath, ValueName), "Failed to read key %s", oErrorGetLastString());

		oTESTB(strcmp(data, KeyTestValue) == 0, "Set/Read values do not match");
		oTESTB(oWinRegistryDeleteValue(oHKEY_CURRENT_USER, KeyPath, ValueName), "Failed to delete key %s", oErrorGetLastString());

		// Delete oUnitTest (if empty) and delete OOOii (if empty)
		oTESTB0(oWinRegistryDeleteKey(oHKEY_CURRENT_USER, "Software/OOOii-oUnitTests"));

		return SUCCESS;
	}
};

oTEST_REGISTER(PLATFORM_oWinRegistry);