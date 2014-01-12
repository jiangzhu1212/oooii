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
#include <oCore/windows/win_version.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ouro {

const char* as_string(const windows::version::value& _Version)
{
	switch (_Version)
	{
		case windows::version::win2000: return "Windows 2000";
		case windows::version::xp: return "Windows XP";
		case windows::version::xp_pro_64bit: return "Windows XP Pro 64-bit";
		case windows::version::server_2003: return "Windows Server 2003";
		case windows::version::home_server: return "Windows Home Server";
		case windows::version::server_2003r2: return "Windows Server 2003R2";
		case windows::version::vista: return "Windows Vista";
		case windows::version::server_2008: return "Windows Server 2008";
		case windows::version::server_2008r2: return "Windows Server 2008R2";
		case windows::version::win7: return "Windows 7";
		case windows::version::win7_sp1: return "Windows 7 SP1";
		case windows::version::win8: return "Windows 8";
		case windows::version::server_2012: "Windows Server 2012";
		case windows::version::win8_1: return "Windows 8.1";
		case windows::version::server_2012_sp1: "Windows Server 2012 SP1";
		case windows::version::unknown:
		default: break;
	}

	return "?";
}

	namespace windows {

version::value get_version()
{
	OSVERSIONINFOEX osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		if (osvi.dwMajorVersion == 6)
		{
			if (osvi.dwMinorVersion == 2)
			{
				if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win8_1 : version::server_2012_sp1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win8 : version::server_2012;
			}

			else if (osvi.dwMinorVersion == 1)
			{
				if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win7 : version::server_2008r2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win7_sp1 : version::server_2008r2_sp1;
			}
			else if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wServicePackMajor == 2)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista_sp2 : version::server_2008_sp2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista_sp1 : version::server_2008_sp1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista : version::server_2008;
			}
		}

		else if (osvi.dwMajorVersion == 5)
		{
			if (osvi.dwMinorVersion == 2)
			{
				SYSTEM_INFO si;
				GetSystemInfo(&si);
				if ((osvi.wProductType == VER_NT_WORKSTATION) && (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
					return version::xp_pro_64bit;
				else if (osvi.wSuiteMask & 0x00008000 /*VER_SUITE_WH_SERVER*/)
					return version::home_server;
				else
					return GetSystemMetrics(SM_SERVERR2) ? version::server_2003r2 : version::server_2003;
			}

			else if (osvi.dwMinorVersion == 1)
				return version::xp;
			else if (osvi.dwMinorVersion == 0)
				return version::win2000;
		}
	}

	return version::unknown;
}

	} // namespace windows
} // namespace ouro