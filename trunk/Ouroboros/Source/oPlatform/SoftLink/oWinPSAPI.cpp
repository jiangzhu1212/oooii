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
#include "oWinPSAPI.h"
#include <oBasis/oAssert.h>

static const char* dll_procs[] = 
{
	"EnumProcesses",
	"EnumProcessModules",
	"GetModuleBaseNameA",
	"GetProcessMemoryInfo",
	"GetModuleInformation",
	"GetModuleFileNameExA",
};

oDEFINE_DLL_SINGLETON_CTOR(oWinPSAPI, "psapi.dll", EnumProcesses)

// {9ADA18F8-492F-48DA-B6D5-A6CCC0749ED4}
const oGUID oWinPSAPI::GUID = { 0x9ada18f8, 0x492f, 0x48da, { 0xb6, 0xd5, 0xa6, 0xcc, 0xc0, 0x74, 0x9e, 0xd4 } };
